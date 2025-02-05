#include "mod/Events/BlockEventHandle.h"
#include "mod/Hook/Hook.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/level/block/NoteBlock.h"


namespace stats::hook::block {
LL_TYPE_INSTANCE_HOOK(
    NoteBlockAttackHook,
    HookPriority::Normal,
    NoteBlock,
    &NoteBlock::$attack,
    bool,
    ::Player*         player,
    ::BlockPos const& pos
) {
    auto res = origin(player, pos);
    // logger.info("NoteBlockAttackHook {} {} {}", player->getRealName(), pos, res);
    if (!res) return res;
    event::block::onNoteBlockAttacked(player);
    return res;
}

void hookNoteBlockAttack() { NoteBlockAttackHook::hook(); }

} // namespace stats::hook::block