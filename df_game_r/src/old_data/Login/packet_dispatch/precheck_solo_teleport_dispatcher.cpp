#include "old_data/Login/packet_dispatch/precheck_solo_teleport_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/character_packets.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kPrecheckSoloTeleportReadSignature[] =
    "virtual int Dispatcher_PrecheckSoloTeleport::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// The retail CMD470/471 state-0 specialization is one shared template
// provider.  The actual CMD471 handler is still a separate lifecycle closure;
// emitting this template here restores CMD470's raw object ownership only.
// IDA_EA: 0x824B65E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824B7BA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824B7C4
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B70A / 0x825B73A
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EED2Ev
// IDA_EA: 0x825D5E8
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598FD2
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SOLO_TELEPORT19stSoloTeleportParamL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_SOLO_TELEPORT,
                                 stSoloTeleportParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x822D764
// IDA_NAME: _ZN15stPartyTelePort5resetEv
// DEMANGLED: stPartyTelePort::reset(void)
void stPartyTelePort::reset() {
    // The original C2 clears exactly the first dword; position and direction
    // are populated only by CMD471 when a concrete destination is supplied.
    // The first wire byte is village at +1; teleport_kind at +0 remains zero.
    std::memset(this, 0, 0x04u);
}

// IDA_EA: 0x822E10C
// IDA_NAME: _ZN17MSG_SOLO_TELEPORTC2Ev
// DEMANGLED: MSG_SOLO_TELEPORT::MSG_SOLO_TELEPORT(void)
MSG_SOLO_TELEPORT::MSG_SOLO_TELEPORT()
    : MSG_BASE(),
      target_charac_name(),
      target_name_length(0),
      target_charac_no(0),
      teleport_data_index(-1),
      resolved_teleport(0) {
    party_teleport.reset();
}

// IDA_EA: 0x8231B56
// IDA_NAME: _ZN19stSoloTeleportParamC2Ev
// DEMANGLED: stSoloTeleportParam::stSoloTeleportParam(void)
stSoloTeleportParam::stSoloTeleportParam()
    : ParamBase(), result_code(0), target_charac_no(0), resolved_teleport(0) {}

// IDA_EA: 0x82468B4
// IDA_NAME: _ZN19stSoloTeleportParamD2Ev
// IDA_EA: 0x82468E4
// IDA_NAME: _ZN19stSoloTeleportParamD0Ev
// DEMANGLED: stSoloTeleportParam::~stSoloTeleportParam()
stSoloTeleportParam::~stSoloTeleportParam() {}

// IDA_EA: 0x8283C5A
// IDA_NAME: _ZN8Teleport10check_ruleEP5CUser
// DEMANGLED: Teleport::check_rule(CUser *)
int Teleport::check_rule(CUser* user) {
    if (!user) {
        return 1;
    }
    if (user->get_state() != 3) {
        return 19;
    }

    GameWorld* const world = G_GameWorld();
    if (world->IsPVPChannel() || world->IsPvPSkilTreeChannel()) {
        return 19;
    }
    if (static_cast<uint8_t>(user->getCurCharacVill()) == 7u) {
        return 190;
    }

    if (static_cast<uint8_t>(user->isGMUser()) != 1u) {
        int aura_option_value = 0;
        const int equip_result =
            user->is_equip_aura_avatar(2, aura_option_value);
        if (equip_result > 0) {
            return equip_result;
        }
    }

    const int aura_cooldown = user->get_aura_avatar_option_value(2);
    if (static_cast<uint8_t>(user->isGMUser()) != 1u &&
        GlobalData::s_systemTime_.getCurSec() < aura_cooldown) {
        return 22;
    }
    return 0;
}

// IDA_EA: 0x8597B7A
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleportC2Ev
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::Dispatcher_PrecheckSoloTeleport(void)
Dispatcher_PrecheckSoloTeleport::Dispatcher_PrecheckSoloTeleport()
    : IPacketDispatcher<MSG_SOLO_TELEPORT,
                        stSoloTeleportParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824421E
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleportD2Ev
// IDA_EA: 0x824424E
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleportD0Ev
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::~Dispatcher_PrecheckSoloTeleport()
Dispatcher_PrecheckSoloTeleport::~Dispatcher_PrecheckSoloTeleport() {}

// IDA_EA: 0x81E1920
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleport4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PrecheckSoloTeleport::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SOLO_TELEPORT& request = static_cast<MSG_SOLO_TELEPORT&>(message);
    if (packet.get_int(request.target_name_length) != 1) {
        return LineFunc(22317, kPrecheckSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_str(request.target_charac_name,
                       30,
                       request.target_name_length) == 1) {
        return 0;
    }
    return LineFunc(22320, kPrecheckSoloTeleportReadSignature, 0, 0);
}

// IDA_EA: 0x81E1A9E
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleport11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PrecheckSoloTeleport::check_error(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)message;
    (void)param;
    if (!user) {
        return -1;
    }
    return Teleport::check_rule(user);
}

// IDA_EA: 0x81E19CA
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleport7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PrecheckSoloTeleport::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    MSG_SOLO_TELEPORT& request = static_cast<MSG_SOLO_TELEPORT&>(message);
    stSoloTeleportParam& result = static_cast<stSoloTeleportParam&>(param);
    const int error = check_error(user, message, param);
    if (error > 0) {
        result.result_code = error;
    } else if (error < 0) {
        return -1;
    }

    if (error == 0) {
        Packet_Monitor_Request_Charac_Info monitor_packet;
        // The old packet is a 0x23-byte UDP request: target name begins at
        // +0x0a.  Its original strncpy uses the client-provided length
        // directly; retain that behavior rather than silently truncating it.
        std::strncpy(monitor_packet.target_charac_name,
                     request.target_charac_name,
                     request.target_name_length);
        monitor_packet.requester_charac_no =
            static_cast<uint32_t>(user->getCurCharacNo());
        GlobalData::s_monitor_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&monitor_packet), 0x23);
    }
    return 0;
}

// IDA_EA: 0x81E1ACC
// IDA_NAME: _ZN31Dispatcher_PrecheckSoloTeleport4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PrecheckSoloTeleport::send(CUser *,ParamBase &)
void Dispatcher_PrecheckSoloTeleport::send(CUser* user, ParamBase& param) {
    const stSoloTeleportParam& result =
        static_cast<const stSoloTeleportParam&>(param);
    if (result.result_code == 0) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_PRECHECK_SOLO_TELEPOART);
    packet.put_byte(0);
    packet.put_byte(static_cast<int8_t>(result.result_code));
    packet.finalize(true);
    user->Send(packet);
}

// Deliberately not registered in the root 606-slot PacketDispatcher here.
// The complete constructor/teardown registration transaction owns activation.
