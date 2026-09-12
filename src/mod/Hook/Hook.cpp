#include "mod/Hook/Hook.h"

#include <array>
#include <memory>

namespace stats::hook {
namespace {
using HookCallback = void (*)();

struct HookBinding {
    HookCallback install;
    HookCallback remove;
};

constexpr std::array bindings{
    HookBinding{player::hookPlayerAddEffect, player::unhookPlayerAddEffect},
    HookBinding{player::hookPlayerBlockUsingShield, player::unhookPlayerBlockUsingShield},
    HookBinding{player::hookPlayerDropItem, player::unhookPlayerDropItem},
    HookBinding{player::hookPlayerStartSleep, player::unhookPlayerStartSleep},
    HookBinding{player::hookServerPlayerOpenTrading, player::unhookServerPlayerOpenTrading},
    HookBinding{player::hookPlayerUseItem, player::unhookPlayerUseItem},
    HookBinding{player::hookPlayerInteractActor, player::unhookPlayerInteractActor},
    HookBinding{player::hookPlayerBreedAnimal, player::unhookPlayerBreedAnimal},
    HookBinding{player::hookPlayerUseBucketItemOnFish, player::unhookPlayerUseBucketItemOnFish},
    HookBinding{player::hookPlayerRiding, player::unhookPlayerRiding},
    HookBinding{player::hookPlayerAuthInput, player::unhookPlayerAuthInput},
    HookBinding{mob::hookMobGetDamageAfterEnchantReduction, mob::unhookMobGetDamageAfterEnchantReduction},
    HookBinding{block::hookBlockUse, block::unhookBlockUse},
    HookBinding{block::hookBlockOnFallOn, block::unhookBlockOnFallOn},
    HookBinding{block::hookCakeRemoveSlice, block::unhookCakeRemoveSlice},
    HookBinding{block::hookCauldronBlockUseInventory, block::unhookCauldronBlockUseInventory},
    HookBinding{block::hookCauldronBlockClean, block::unhookCauldronBlockClean},
    HookBinding{block::hookFlowerPotBlockPlaceFlower, block::unhookFlowerPotBlockPlaceFlower},
    HookBinding{block::hookNoteBlockAttack, block::unhookNoteBlockAttack},
    HookBinding{block::hookProjectileHitTargetBlock, block::unhookProjectileHitTargetBlock},
    HookBinding{item::hookItemStackBaseHurtAndBroken, item::unhookItemStackBaseHurtAndBroken},
    HookBinding{container::hookLevelContainerChanged, container::unhookLevelContainerChanged},
    HookBinding{
        inventory::hookItemStackRequestActionHandlerTransfer,
        inventory::unhookItemStackRequestActionHandlerTransfer
    },
};

class HookSession {
public:
    HookSession() {
        for (auto const& binding : bindings) {
            binding.install();
            ++mInstalledCount;
        }
    }

    ~HookSession() {
        while (mInstalledCount > 0) {
            bindings[--mInstalledCount].remove();
        }
    }

    HookSession(HookSession const&)            = delete;
    HookSession& operator=(HookSession const&) = delete;

private:
    size_t mInstalledCount{0};
};

std::unique_ptr<HookSession> activeSession;
} // namespace

void hook() {
    if (!activeSession) activeSession = std::make_unique<HookSession>();
}

void unhook() { activeSession.reset(); }
} // namespace stats::hook
