#include "ll/api/base/StdInt.h"
#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/platform/UUID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/actor/ai/goal/BreedGoal.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/gamemode/InteractionResult.h>
#include <mc/world/item/BucketItem.h>
#include <mc/world/level/BlockPos.h>


#include "mod/Events/PlayerEventHandle.h"

#include <unordered_map>
#include <unordered_set>

namespace stats::hook::player {
namespace {
std::unordered_map<uint64, mce::UUID> breedCacheMap;
std::unordered_set<uint64>            fishCaughtSet;
} // namespace
LL_TYPE_INSTANCE_HOOK(
    InteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    bool,
    Actor&      actor,
    Vec3 const& location
) {
    auto uniqueId = actor.getOrCreateUniqueID().getHash();
    auto text     = getInteractText();
    auto uuid     = getUuid();
    auto r        = origin(actor, location);
    if (!r) return r;
    if (actor.hasCategory(::ActorCategory::WaterAnimal) || actor.hasType(::ActorType::Axolotl)) {
        auto it = fishCaughtSet.find(uniqueId);
        if (it != fishCaughtSet.end()) {
            event::player::onFishCaught(uuid);
            fishCaughtSet.erase(it);
        }
    }
    if (text != "Feed") return r;
    breedCacheMap[uniqueId] = uuid;
    return r;
}

LL_TYPE_INSTANCE_HOOK(BreedGoalStopHook, HookPriority::Normal, BreedGoal, &BreedGoal::$stop, void) {
    Mob& mob     = mOwner;
    Actor* partner = mPartner->tryUnwrap<>();
    if (!partner) return origin();
    auto mob1UniqueId = mob.getOrCreateUniqueID().getHash();
    auto mob2UniqueId = partner->getOrCreateUniqueID().getHash();
    auto find1        = breedCacheMap.find(mob1UniqueId);
    auto find2        = breedCacheMap.find(mob2UniqueId);
    if (find1 == breedCacheMap.end() && find2 == breedCacheMap.end()) return origin();
    if (find1->second == find2->second) {
        event::player::onBreedAnimal(find1->second);
        breedCacheMap.erase(find1);
        breedCacheMap.erase(find2);
    } else {
        if (find1 != breedCacheMap.end()) {
            event::player::onBreedAnimal(find1->second);
            breedCacheMap.erase(find1);
        }
        if (find2 != breedCacheMap.end()) {
            event::player::onBreedAnimal(find1->second);
            breedCacheMap.erase(find2);
        }
    }
    origin();
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
    if (r.mResult == InteractionResult::Result::Swing) {
        fishCaughtSet.insert(uniqueId);
    }
    return r;
}

void hookPlayerInteractActor() { InteractEntityHook::hook(); }
void hookPlayerBreedAnimal() { BreedGoalStopHook::hook(); }
void hookPlayerUseBucketItemOnFish() { BucketItemUseOnHook::hook(); }

} // namespace stats::hook::player