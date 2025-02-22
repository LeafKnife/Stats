#include "mc/platform/UUID.h"
#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/ai/goal/BreedGoal.h"
#include "mc/world/actor/player/Player.h"

#include "mod/Events/PlayerEventHandle.h"

#include <unordered_map>

namespace stats::hook::player {
namespace {
std::unordered_map<int64, mce::UUID> breedCacheMap;
}
LL_TYPE_INSTANCE_HOOK(
    InteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    bool,
    Actor&      actor,
    Vec3 const& location
) {
    auto text     = getInteractText();
    auto uniqueId = actor.getOrCreateUniqueID().rawID;
    auto uuid     = getUuid();
    auto r        = origin(actor, location);
    if (!r) return r;
    if (text != "Feed") return r;
    breedCacheMap[uniqueId] = uuid;
    return r;
}

LL_TYPE_INSTANCE_HOOK(BreedGoalStopHook, HookPriority::Normal, BreedGoal, &BreedGoal::$stop, void) {
    Mob& mob          = mUnk76ece7.as<Mob&>();
    Mob& partner      = mUnk828cc9.as<::WeakEntityRef>().mWeakEntity.tryUnwrap<Mob>();
    auto mob1UniqueId = mob.getOrCreateUniqueID().rawID;
    auto mob2UniqueId = partner.getOrCreateUniqueID().rawID;
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

void hookPlayerBreedAnimal() {
    InteractEntityHook::hook();
    BreedGoalStopHook::hook();
}

} // namespace stats::hook::player