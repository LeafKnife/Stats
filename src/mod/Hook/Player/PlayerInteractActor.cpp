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


#include "mod/Events/PlayerEventHandle.h"

#include <unordered_set>
#include <vector>

namespace stats::hook::player {
namespace {
std::unordered_set<uint64> fishCaughtSet;

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
    auto uniqueId = actor.getOrCreateUniqueID().getHash();
    auto uuid     = getUuid();
    auto r        = origin(actor, location);
    if (!r.mSuccess) return r; //后续可能还需要修改
    if (actor.hasCategory(::ActorCategory::WaterAnimal) || actor.isType(::ActorType::Axolotl)) {
        auto it = fishCaughtSet.find(uniqueId);
        if (it != fishCaughtSet.end()) {
            event::player::onFishCaught(uuid);
            fishCaughtSet.erase(it);
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

    event::player::onBreedAnimal(player->getUuid());
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
    auto uniqueId = entity.getOrCreateUniqueID().getHash();
    auto r        = origin(instance, entity, pos, face, clickPos);
    if (r.mSwing) {
        fishCaughtSet.insert(uniqueId);
    }
    return r;
}

void hookPlayerInteractActor() { InteractEntityHook::hook(); }
void unhookPlayerInteractActor() { InteractEntityHook::unhook(); }
void hookPlayerBreedAnimal() { PlayerBredAnimalsHook::hook(); }
void unhookPlayerBreedAnimal() { PlayerBredAnimalsHook::unhook(); }
void hookPlayerUseBucketItemOnFish() { BucketItemUseOnHook::hook(); }
void unhookPlayerUseBucketItemOnFish() { BucketItemUseOnHook::unhook(); }

} // namespace stats::hook::player
