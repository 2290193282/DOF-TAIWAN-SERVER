#include "old_game/pvp/battlefield/inter_power_war_dialogue2_returns.h"

#include <cstring>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/log_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

void UpdateGuildPowerSide(CUser* user, uint8_t power_side) {
    STGuildDBInfoOnly* const guild_info = user->GetGuildDBInfo();
    STGuildProxy proxy_info(guild_info->guild_name,
                            guild_info->guild_level,
                            power_side);
    GlobalData::s_guild_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->ModifyGuildInfoProxy(
            static_cast<uint32_t>(user->get_charac_guildkey()), proxy_info);
}

void SendJoinOrSecedeReply(CUser* user,
                           uint32_t result_code,
                           uint8_t power_side,
                           ENUM_CMDPACKET packet_id) {
    PacketGuard packet;
    packet.put_header(1, packet_id);
    if (result_code) {
        packet.put_byte(0);
        packet.put_byte(static_cast<uint8_t>(result_code));
    } else {
        packet.put_byte(1);
        packet.put_byte(power_side);
    }
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x84D46FA
// IDA_NAME: _ZN24Inter_ReplyPowerWarScore12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReplyPowerWarScore::dispatch_sig(CUser *,char *,int)
int Inter_ReplyPowerWarScore::dispatch_sig(CUser*, char* payload, int) {
    const int power_a = static_cast<int>(ReadU32(payload, 10));
    const int power_b = static_cast<int>(ReadU32(payload, 14));
    PacketGuard packet;
    packet.put_header(0, 155);
    packet.put_int(power_a);
    packet.put_int(power_b);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    GlobalData::s_power_manager->SetPowerWarPoint(power_a, power_b);
    return 0;
}

// IDA_EA: 0x84EDBD0 / 0x84F8E68 / 0x84F8E98
// IDA_NAME: _ZN24Inter_ReplyPowerWarScoreC2Ev /
//           _ZN24Inter_ReplyPowerWarScoreD2Ev /
//           _ZN24Inter_ReplyPowerWarScoreD0Ev
Inter_ReplyPowerWarScore::Inter_ReplyPowerWarScore() : ISigDispatcher() {}
Inter_ReplyPowerWarScore::~Inter_ReplyPowerWarScore() {}

// IDA_EA: 0x84D4FCE
// IDA_NAME: _ZN27Inter_NoticePowerWarProlong12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePowerWarProlong::dispatch_sig(CUser *,char *,int)
int Inter_NoticePowerWarProlong::dispatch_sig(CUser*, char* payload, int) {
    PacketGuard packet;
    packet.put_header(0, 163);
    packet.put_int(static_cast<int>(ReadU32(payload, 10)));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EDC78 / 0x84F8C46 / 0x84F8C76
// IDA_NAME: _ZN27Inter_NoticePowerWarProlongC2Ev /
//           _ZN27Inter_NoticePowerWarProlongD2Ev /
//           _ZN27Inter_NoticePowerWarProlongD0Ev
Inter_NoticePowerWarProlong::Inter_NoticePowerWarProlong()
    : ISigDispatcher() {}
Inter_NoticePowerWarProlong::~Inter_NoticePowerWarProlong() {}

// IDA_EA: 0x84D47D4
// IDA_NAME: _ZN20Inter_ReplyJoinPower12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReplyJoinPower::dispatch_sig(CUser *,char *,int)
int Inter_ReplyJoinPower::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state() > 2) {
        const uint32_t charac_no = ReadU32(payload, 18);
        if (user->get_charac_no(-1) == static_cast<int>(charac_no)) {
            const uint32_t result_code = ReadU32(payload, 10);
            const uint8_t power_side =
                static_cast<uint8_t>(payload[22]);
            SendJoinOrSecedeReply(
                user, result_code, power_side, ENUM_CMDPACKET_JOIN_POWER);
            UpdateGuildPowerSide(user, power_side);
        } else {
            LogManager::logFormat(
                1, "InterDispatcher.cpp",
                "virtual int Inter_ReplyJoinPower::dispatch_sig(CUser*, char*, int)",
                0x35FC,
                "Inter_ReplyJoinPower::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharNo(%d)",
                user->get_charac_no(-1), charac_no);
        }
    }
    return 0;
}

// IDA_EA: 0x84EDBEC / 0x84F8E1A / 0x84F8E4A
// IDA_NAME: _ZN20Inter_ReplyJoinPowerC2Ev /
//           _ZN20Inter_ReplyJoinPowerD2Ev /
//           _ZN20Inter_ReplyJoinPowerD0Ev
Inter_ReplyJoinPower::Inter_ReplyJoinPower() : ISigDispatcher() {}
Inter_ReplyJoinPower::~Inter_ReplyJoinPower() {}

// IDA_EA: 0x84D49D6
// IDA_NAME: _ZN17Inter_SecedePower12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SecedePower::dispatch_sig(CUser *,char *,int)
int Inter_SecedePower::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state() > 2) {
        const uint32_t charac_no = ReadU32(payload, 18);
        if (user->get_charac_no(-1) == static_cast<int>(charac_no)) {
            const uint32_t result_code = ReadU32(payload, 10);
            const uint8_t power_side =
                static_cast<uint8_t>(payload[22]);
            SendJoinOrSecedeReply(
                user, result_code, power_side, ENUM_CMDPACKET_SECEDE_POWER);
            // The old constructor receives a literal 0 for the cached side;
            // it deliberately does not reuse the reply's payload +22 byte.
            UpdateGuildPowerSide(user, 0);
        } else {
            LogManager::logFormat(
                1, "InterDispatcher.cpp",
                "virtual int Inter_SecedePower::dispatch_sig(CUser*, char*, int)",
                0x3620,
                "Inter_SecedePower::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharNo(%d)",
                user->get_charac_no(-1), charac_no);
        }
    }
    return 0;
}

// IDA_EA: 0x84EDC08 / 0x84F8DCC / 0x84F8DFC
// IDA_NAME: _ZN17Inter_SecedePowerC2Ev /
//           _ZN17Inter_SecedePowerD2Ev /
//           _ZN17Inter_SecedePowerD0Ev
Inter_SecedePower::Inter_SecedePower() : ISigDispatcher() {}
Inter_SecedePower::~Inter_SecedePower() {}

// IDA_EA: 0x84D4BD2
// IDA_NAME: _ZN29Inter_NoticePowerWarStartTime12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePowerWarStartTime::dispatch_sig(CUser *,char *,int)
int Inter_NoticePowerWarStartTime::dispatch_sig(CUser*, char* payload, int) {
    const int server_type = G_CEnvironment()->get_server_type();
    if (server_type != 1 && server_type != 2) {
        const uint8_t hour = static_cast<uint8_t>(payload[10]);
        const uint8_t minute = static_cast<uint8_t>(payload[11]);
        const uint8_t second = static_cast<uint8_t>(payload[12]);
        const uint8_t day = static_cast<uint8_t>(payload[13]);
        GlobalData::s_power_manager->SetPowerWarStartTime(hour, minute,
                                                           second, day);

        PacketGuard packet;
        packet.put_header(0, 161);
        packet.put_byte(hour);
        packet.put_byte(minute);
        packet.put_byte(second);
        packet.put_byte(day);
        packet.finalize(true);
        G_GameWorld()->send_all(packet);
    }
    return 0;
}

// IDA_EA: 0x84EDC24 / 0x84F8D7E / 0x84F8DAE
// IDA_NAME: _ZN29Inter_NoticePowerWarStartTimeC2Ev /
//           _ZN29Inter_NoticePowerWarStartTimeD2Ev /
//           _ZN29Inter_NoticePowerWarStartTimeD0Ev
Inter_NoticePowerWarStartTime::Inter_NoticePowerWarStartTime()
    : ISigDispatcher() {}
Inter_NoticePowerWarStartTime::~Inter_NoticePowerWarStartTime() {}

// IDA_EA: 0x84D4D46
// IDA_NAME: _ZN27Inter_NoticePowerWarEndTime12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePowerWarEndTime::dispatch_sig(CUser *,char *,int)
int Inter_NoticePowerWarEndTime::dispatch_sig(CUser*, char* payload, int) {
    PacketGuard packet;
    packet.put_header(0, 162);
    packet.put_byte(static_cast<uint8_t>(payload[10]));
    packet.put_int(static_cast<int>(ReadU32(payload, 11)));
    packet.put_int(static_cast<int>(ReadU32(payload, 15)));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EDC40 / 0x84F8D30 / 0x84F8D60
// IDA_NAME: _ZN27Inter_NoticePowerWarEndTimeC2Ev /
//           _ZN27Inter_NoticePowerWarEndTimeD2Ev /
//           _ZN27Inter_NoticePowerWarEndTimeD0Ev
Inter_NoticePowerWarEndTime::Inter_NoticePowerWarEndTime()
    : ISigDispatcher() {}
Inter_NoticePowerWarEndTime::~Inter_NoticePowerWarEndTime() {}

// IDA_EA: 0x84D4E18
// IDA_NAME: _ZN24Inter_NoticePowerWarInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePowerWarInfo::dispatch_sig(CUser *,char *,int)
int Inter_NoticePowerWarInfo::dispatch_sig(CUser*, char* payload, int) {
    const uint8_t winner_side = static_cast<uint8_t>(payload[18]);
    GlobalData::s_power_manager->SetPowerInfo(
        static_cast<char>(winner_side), static_cast<int>(ReadU32(payload, 10)),
        static_cast<int>(ReadU32(payload, 14)));

    GameWorld* const world = G_GameWorld();
    const int server_type = G_CEnvironment()->get_server_type();
    if (!world->IsIntegratedPvPBaseChannel() && server_type != 1 &&
        server_type != 2) {
        PacketGuard packet;
        packet.put_header(0, 188);
        packet.put_byte(winner_side);
        packet.finalize(true);
        G_GameWorld()->send_all(packet);
    }
    return 0;
}

// IDA_EA: 0x84EDC5C / 0x84F8CE2 / 0x84F8D12
// IDA_NAME: _ZN24Inter_NoticePowerWarInfoC2Ev /
//           _ZN24Inter_NoticePowerWarInfoD2Ev /
//           _ZN24Inter_NoticePowerWarInfoD0Ev
Inter_NoticePowerWarInfo::Inter_NoticePowerWarInfo() : ISigDispatcher() {}
Inter_NoticePowerWarInfo::~Inter_NoticePowerWarInfo() {}
