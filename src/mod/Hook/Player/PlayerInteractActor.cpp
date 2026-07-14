#include "ll/api/base/StdInt.h"
#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/entity/components_json_legacy/BreedableComponent.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/gamemode/InteractionResult.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/BlockPos.h>


#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

#include <chrono>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace stats::hook::player {
namespace {
using PendingCatchClock = std::chrono::steady_clock;

constexpr auto        PendingCatchLifetime = std::chrono::seconds{30};
constexpr std::size_t MaxPendingCatches    = 512;

std::unordered_map<ActorUniqueID, PendingCatchClock::time_point> pendingFishCatches;

void prunePendingFishCatches(PendingCatchClock::time_point now) {
    for (auto entry = pendingFishCatches.begin(); entry != pendingFishCatches.end();) {
        if (entry->second <= now) {
            entry = pendingFishCatches.erase(entry);
        } else {
            ++entry;
        }
    }
    if (pendingFishCatches.size() < MaxPendingCatches) return;

    auto oldest = pendingFishCatches.begin();
    for (auto entry = std::next(pendingFishCatches.begin()); entry != pendingFishCatches.end(); ++entry) {
        if (entry->second < oldest->second) oldest = entry;
    }
    pendingFishCatches.erase(oldest);
}

void rememberPendingFishCatch(ActorUniqueID uniqueId) {
    auto const now = PendingCatchClock::now();
    prunePendingFishCatches(now);
    pendingFishCatches.insert_or_assign(uniqueId, now + PendingCatchLifetime);
}

bool consumePendingFishCatch(ActorUniqueID uniqueId) {
    auto const pending = pendingFishCatches.find(uniqueId);
    if (pending == pendingFishCatches.end()) return false;

    auto const valid = pending->second > PendingCatchClock::now();
    pendingFishCatches.erase(pending);
    return valid;
}

Player* resolveLoveCausePlayer(ActorUniqueID const& loveCause) {
    if (loveCause == ActorUniqueID::INVALID_ID()) return nullptr;

    auto* level = ll::service::getLevel().as_ptr();
    return level ? level->getPlayer(loveCause) : nullptr;
}
} // namespace
LL_TYPE_INSTANCE_HOOK(
    InteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    ::InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    auto uniqueId = actor.getOrCreateUniqueID();
    auto uuid     = getUuid();
    auto r        = origin(actor, location);
    if (!r.mSuccess) return r; //后续可能还需要修改
    if (actor.hasCategory(::ActorCategory::WaterAnimal) || actor.isType(::ActorType::Axolotl)) {
        if (consumePendingFishCatch(uniqueId)) {
            handler::onPlayerFishCaught(uuid);
        }
    }
    return r;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerBredAnimalsHook,
    HookPriority::Normal,
    BreedableComponent,
    &BreedableComponent::mate,
    std::vector<WeakEntityRef>,
    Actor& owner,
    Actor& partner
) {
    auto const loveCause = mLoveCause.get();
    auto       result    = origin(owner, partner);
    if (result.empty()) return result;

    auto* player = resolveLoveCausePlayer(loveCause);
    if (!player) return result;

    handler::onPlayerBreedAnimal(player->getUuid());
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    BucketItemUseOnHook,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::$_useOn,
    ::InteractionResult,
    ::ItemStack&  instance,
    ::Actor&      entity,
    ::BlockPos    pos,
    uchar         face,
    ::Vec3 const& clickPos
) {
    auto typeId  = static_cast<int>(entity.getEntityTypeId());
    auto canFill = (typeId >= 9068 && typeId <= 9072) || typeId == 4994 || typeId == 9093;
    if (!canFill) return origin(instance, entity, pos, face, clickPos);
    auto uniqueId = entity.getOrCreateUniqueID();
    auto r        = origin(instance, entity, pos, face, clickPos);
    if (r.mSwing) {
        rememberPendingFishCatch(uniqueId);
    }
    return r;
}

void hookPlayerInteractActor() { InteractEntityHook::hook(); }
void unhookPlayerInteractActor() { InteractEntityHook::unhook(); }
void hookPlayerBreedAnimal() { PlayerBredAnimalsHook::hook(); }
void unhookPlayerBreedAnimal() { PlayerBredAnimalsHook::unhook(); }
void hookPlayerUseBucketItemOnFish() { BucketItemUseOnHook::hook(); }
void unhookPlayerUseBucketItemOnFish() {
    BucketItemUseOnHook::unhook();
    pendingFishCatches.clear();
}

} // namespace stats::hook::player
