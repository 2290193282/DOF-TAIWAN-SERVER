#include "old_game/world/portal/solo_teleport_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/invalid_area_move.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kSoloTeleportReadSignature[] =
    "virtual int Dispatcher_SoloTeleport::read(PacketBuf&, MSG_BASE&)";
const char kSoloTeleportSendSignature[] =
    "virtual void Dispatcher_SoloTeleport::send(CUser*, ParamBase&)";

}  // namespace

// IDA_DATA: unnamed BSS dword at 0x941F81C.
User_Teleport* g_user_solo_teleport __asm__("dword_941F81C") = 0;

// IDA_EA: 0x822D788
// IDA_NAME: _ZN15STCheckRuleDataC2Ev
STCheckRuleData::STCheckRuleData() {}

// IDA_EA: 0x822D78E
// IDA_NAME: _ZN14STTeleportDataC2Ev
STTeleportData::STTeleportData() {
    std::memset(this, 0, 0x04u);
}

// IDA_EA: 0x8283C30
// IDA_NAME: _ZN8TeleportC2Ev
Teleport::Teleport() : end_cool_time_(0) {
    party_teleport_.reset();
}

// IDA_EA: 0x8283D82
// IDA_NAME: _ZN8Teleport10check_ruleERK15STCheckRuleData
int Teleport::check_rule(const STCheckRuleData& rule) {
    return Teleport::check_rule(rule.requesting_user);
}

// IDA_EA: 0x8283D98
// IDA_NAME: _ZN8Teleport18set_party_teleportERK15stPartyTelePort
Teleport* Teleport::set_party_teleport(
    const stPartyTelePort& destination) {
    std::memcpy(&party_teleport_, &destination, sizeof(party_teleport_));
    return this;
}

// IDA_EA: 0x8283DB6
// IDA_NAME: _ZN8Teleport18get_party_teleportEv
stPartyTelePort* Teleport::get_party_teleport() {
    return &party_teleport_;
}

// IDA_EA: 0x8283DC2
// IDA_NAME: _ZN8Teleport17set_end_cool_timeEP5CUser
void Teleport::set_end_cool_time(CUser* user) {
    if (!user) {
        return;
    }
    end_cool_time_ = user->get_aura_avatar_option_value(2);
    const int now = GlobalData::s_systemTime_.getCurSec();
    if (end_cool_time_ <= now) {
        end_cool_time_ = 0;
    } else {
        end_cool_time_ -= now;
    }
}

// IDA_EA: 0x8283E24
// IDA_NAME: _ZN8Teleport17get_end_cool_timeEv
int Teleport::get_end_cool_time() const {
    return end_cool_time_;
}

// IDA_EA: 0x8283E30
// IDA_NAME: _ZN8Teleport25update_next_end_cool_timeEP5CUser
void Teleport::update_next_end_cool_time(CUser* user) {
    if (!user) {
        return;
    }
    int cooldown_seconds = 0;
    user->is_equip_aura_avatar(2, cooldown_seconds);
    const int expiry = GlobalData::s_systemTime_.getCurSec() +
                       cooldown_seconds;
    user->set_aura_avatar_option_value(2, expiry);
    user->UpdateAuraAvatarOption(
        2, user->get_aura_avatar_option_value(2));
}

// IDA_EA: 0x8283EBA
// IDA_NAME: _ZN8Teleport20send_teleport_effectEP5CUseri
void Teleport::send_teleport_effect(CUser* user, int phase) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 379);
    packet.put_byte(phase);
    packet.put_byte(1);
    packet.put_short(user->GetUID());
    packet.finalize(true);
    G_GameWorld()->send_to_area(user->getCurCharacVill(),
                                user->get_area(false), packet);
}

// IDA_EA: 0x8283FB8
// IDA_NAME: _ZN8Teleport9move_areaEP5CUser
int Teleport::move_area(CUser* user) {
    if (!user) {
        return 0;
    }
    send_teleport_effect(user, 1);
    if (G_GameWorld()->move_area(
            user, party_teleport_.teleport_village,
            party_teleport_.teleport_area_index,
            party_teleport_.teleport_posx, party_teleport_.teleport_posy,
            party_teleport_.teleport_direction, true, 2, 0, 0) != 1) {
        return 0;
    }
    send_teleport_effect(user, 2);
    update_next_end_cool_time(user);
    return 1;
}

// IDA_EA: 0x82843B0 / 0x82843D6
// IDA_NAME: _ZN8TeleportD2Ev / _ZN8TeleportD0Ev
Teleport::~Teleport() {}

// IDA_EA: 0x82840DA
// IDA_NAME: _ZN12NPC_TeleportC2Ev
NPC_Teleport::NPC_Teleport() : Teleport() {}

// IDA_EA: 0x82840F6
// IDA_NAME: _ZN12NPC_Teleport10check_ruleERK15STCheckRuleData
int NPC_Teleport::check_rule(const STCheckRuleData& rule) {
    return Teleport::check_rule(rule);
}

// IDA_EA: 0x8284124
// IDA_NAME: _ZN12NPC_Teleport17set_teleport_dataERK14STTeleportData
Teleport* NPC_Teleport::set_teleport_data(const STTeleportData& data) {
    return set_party_teleport(data.party_teleport);
}

// IDA_EA: 0x82843F4 / 0x8284424
// IDA_NAME: _ZN12NPC_TeleportD2Ev / _ZN12NPC_TeleportD0Ev
NPC_Teleport::~NPC_Teleport() {}

// IDA_EA: 0x8284142
// IDA_NAME: _ZN13User_TeleportC2Ev
User_Teleport::User_Teleport() : Teleport() {}

// IDA_EA: 0x828415E
// IDA_NAME: _ZN13User_Teleport10check_ruleERK15STCheckRuleData
int User_Teleport::check_rule(const STCheckRuleData& rule) {
    const int error = Teleport::check_rule(rule);
    if (error) {
        return error;
    }

    CUser* const target = G_GameWorld()->find_from_world(
        static_cast<uint16_t>(rule.target_charac_no));
    if (!target) {
        return 21;
    }
    if (target->get_state() != 3) {
        return 4;
    }
    const int target_village = target->getCurCharacVill();
    if (target_village == 7 || target_village == 8 || target_village == 10) {
        return 4;
    }
    CHackLog_InvalidAreaMove* const invalid_area =
        reinterpret_cast<CHackLog_InvalidAreaMove*>(
            static_cast<uintptr_t>(target_village));
    if (invalid_area->checkSeriaRoom(target->get_area(false), 0)) {
        return 4;
    }
    if (target->GetAssaultPlace()) {
        return 156;
    }

    CParty* const party = target->GetParty();
    if (!party) {
        return 0;
    }
    return static_cast<uint8_t>(
               party->GetPartyTelePort()->get_teleport_state()) == 0xffu
               ? 0
               : 158;
}

// IDA_EA: 0x82842B8
// IDA_NAME: _ZN13User_Teleport17set_teleport_dataERK14STTeleportData
Teleport* User_Teleport::set_teleport_data(const STTeleportData& data) {
    CUser* const target = G_GameWorld()->find_from_world(
        static_cast<uint16_t>(data.target_charac_no));
    if (!target) {
        return 0;
    }

    stPartyTelePort destination;
    std::memset(&destination, 0, sizeof(destination));
    destination.teleport_village =
        static_cast<uint8_t>(target->getCurCharacVill());
    destination.teleport_area_index =
        static_cast<uint8_t>(target->get_area(false));
    destination.teleport_posx = static_cast<uint16_t>(target->get_posX());
    destination.teleport_posy = static_cast<uint16_t>(target->get_posY());
    destination.teleport_direction =
        static_cast<uint8_t>(target->get_direction());
    return set_party_teleport(destination);
}

// IDA_EA: 0x8284442 / 0x8284472
// IDA_NAME: _ZN13User_TeleportD2Ev / _ZN13User_TeleportD0Ev
User_Teleport::~User_Teleport() {}

// IDA_EA: 0x81E1B9C
// IDA_NAME: _ZN23Dispatcher_SoloTeleport4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SoloTeleport::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SoloTeleport::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SOLO_TELEPORT& request = static_cast<MSG_SOLO_TELEPORT&>(message);
    if (packet.get_int(reinterpret_cast<uint32_t&>(
            request.target_charac_no)) != 1) {
        return LineFunc(22392, kSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_int(reinterpret_cast<uint32_t&>(
            request.teleport_data_index)) != 1) {
        return LineFunc(22395, kSoloTeleportReadSignature, 0, 0);
    }
    if (request.teleport_data_index == -1) {
        return 0;
    }
    // Retail writes the two C2S destination bytes at MSG offsets +0x39/+0x3a
    // (stPartyTelePort +1/+2): village followed by area.  teleport_kind at
    // +0x38 is reset locally and is not present on the wire.
    if (packet.get_byte(request.party_teleport.teleport_village) != 1) {
        return LineFunc(22399, kSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_byte(request.party_teleport.teleport_area_index) != 1) {
        return LineFunc(22400, kSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_short(request.party_teleport.teleport_posx) != 1) {
        return LineFunc(22401, kSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_short(request.party_teleport.teleport_posy) != 1) {
        return LineFunc(22402, kSoloTeleportReadSignature, 0, 0);
    }
    if (packet.get_byte(request.party_teleport.teleport_direction) != 1) {
        return LineFunc(22403, kSoloTeleportReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81E1D9C
// IDA_NAME: _ZN23Dispatcher_SoloTeleport7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SoloTeleport::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SoloTeleport::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    MSG_SOLO_TELEPORT& request = static_cast<MSG_SOLO_TELEPORT&>(message);
    stSoloTeleportParam& result = static_cast<stSoloTeleportParam&>(param);

    request.resolved_teleport =
        request.teleport_data_index == -1
            ? static_cast<Teleport*>(g_user_solo_teleport)
            : static_cast<Teleport*>(GlobalData::g_pTeleport);

    STTeleportData data;
    data.target_charac_no = request.target_charac_no;
    std::memcpy(&data.party_teleport, &request.party_teleport,
                sizeof(data.party_teleport));
    request.resolved_teleport->set_teleport_data(data);

    const int error = check_error(user, message, param);
    if (error < 0) {
        return -1;
    }
    if (error > 0) {
        result.result_code = error;
    } else {
        request.resolved_teleport->set_end_cool_time(user);
        result.target_charac_no = request.target_charac_no;
    }
    result.resolved_teleport = request.resolved_teleport;
    return 0;
}

// IDA_EA: 0x81E1E98
// IDA_NAME: _ZN23Dispatcher_SoloTeleport11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SoloTeleport::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SoloTeleport::check_error(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)param;
    if (!user) {
        return -1;
    }
    MSG_SOLO_TELEPORT& request = static_cast<MSG_SOLO_TELEPORT&>(message);
    if (request.target_charac_no == -1 &&
        request.teleport_data_index == -1) {
        return -1;
    }

    STCheckRuleData rule;
    rule.target_charac_no = request.target_charac_no;
    rule.requesting_user = user;
    const int rule_error = request.resolved_teleport->check_rule(rule);
    if (rule_error) {
        return rule_error;
    }

    const stPartyTelePort* const destination =
        request.resolved_teleport->get_party_teleport();
    return G_GameWorld()->check_move_area(
        user, destination->teleport_village, destination->teleport_area_index,
        destination->teleport_posx, destination->teleport_posy,
        destination->teleport_direction, true, 2);
}

// IDA_EA: 0x81E1FF6
// IDA_NAME: _ZN23Dispatcher_SoloTeleport4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SoloTeleport::send(CUser *,ParamBase &)
void Dispatcher_SoloTeleport::send(CUser* user, ParamBase& param) {
    stSoloTeleportParam& result = static_cast<stSoloTeleportParam&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SOLO_TELEPOART);
    if (result.result_code) {
        packet.put_byte(0);
        packet.put_byte(static_cast<int8_t>(result.result_code));
        packet.put_int(result.resolved_teleport->get_end_cool_time());
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    if (result.resolved_teleport->move_area(user) == 1) {
        cMyTrace trace(kSoloTeleportSendSignature, 22493, 0);
        trace("SOLO_TELEPORT : teleport_success:(char_no:%d)",
              user->getCurCharacNo());
    } else {
        cMyTrace trace(kSoloTeleportSendSignature, 22489, 5);
        trace("SOLO_TELEPORT : teleport_error:(char_no:%d)",
              user->getCurCharacNo());
    }
}

// IDA_EA: 0x82441D0
// IDA_NAME: _ZN23Dispatcher_SoloTeleportD2Ev
// DEMANGLED: Dispatcher_SoloTeleport::~Dispatcher_SoloTeleport()
// IDA_EA: 0x8244200
// IDA_NAME: _ZN23Dispatcher_SoloTeleportD0Ev
// DEMANGLED: Dispatcher_SoloTeleport::~Dispatcher_SoloTeleport()
Dispatcher_SoloTeleport::~Dispatcher_SoloTeleport() {}

// IDA_EA: 0x8597B96
// IDA_NAME: _ZN23Dispatcher_SoloTeleportC2Ev
// DEMANGLED: Dispatcher_SoloTeleport::Dispatcher_SoloTeleport(void)
Dispatcher_SoloTeleport::Dispatcher_SoloTeleport()
    : IPacketDispatcher<MSG_SOLO_TELEPORT,
                        stSoloTeleportParam,
                        static_cast<ch_state>(0)>() {}
