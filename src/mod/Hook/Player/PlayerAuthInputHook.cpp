#include "mod/Hook/Hook.h"

#include <cstddef>
#include <ll/api/memory/Hook.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/ecs/WeakEntityRef.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/PlayerAuthInputPacket.h>

#include "mod/Events/PlayerEventHandle.h"

namespace stats::hook::player {

LL_TYPE_INSTANCE_HOOK(
    PlayerAuthInputHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    ::NetworkIdentifier const&     source,
    ::PlayerAuthInputPacket const& packet
) {
    origin(source, packet);
    auto handle = thisFor<NetEventCallback>();
    auto player = handle->_getServerPlayer(source, packet.mSenderSubId);
    if (!player) return;
    event::player::onAuthInput(*player, packet);
    return;
}

void hookPlayerAuthInput() { PlayerAuthInputHook::hook(); }

} // namespace stats::hook::player