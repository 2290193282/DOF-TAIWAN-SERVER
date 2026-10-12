#include "old_game/pvp/season/packet_dispatcher_res_pvp_rank.h"

#include <stdint.h>

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x81F0282
// IDA_NAME: _ZN21DisPatcher_ResPvpRank12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_ResPvpRank::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_ResPvpRank::dispatch_sig(CUser* user, PacketBuf& packet) {
    const int state = user->get_state();
    PvP_Room* const zombie_room = user->GetPVPRoom();
    const bool zombie_rank_barrier =
        state == 5 && zombie_room &&
        (zombie_mode::IsActiveRoom(zombie_room) ||
         zombie_mode::IsFinishedRoom(zombie_room));
    if (state != 7 && !zombie_rank_barrier) {
        return 0;
    }

    MSG_RES_PVP_RANK message;
    if (ExtractPacket(message, packet) != 1) {
        return LineFunc(
            30518,
            "virtual int DisPatcher_ResPvpRank::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (SavePvpRank(user, message) == 1) {
        return 0;
    }
    return LineFunc(
        30521,
        "virtual int DisPatcher_ResPvpRank::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x81F039E
// IDA_NAME: _ZN21DisPatcher_ResPvpRank13ExtractPacketER16MSG_RES_PVP_RANKR9PacketBuf
// DEMANGLED: DisPatcher_ResPvpRank::ExtractPacket(MSG_RES_PVP_RANK &,PacketBuf &)
int DisPatcher_ResPvpRank::ExtractPacket(MSG_RES_PVP_RANK& message,
                                          PacketBuf& packet) {
    short v4 = 0;
    short v5 = 0;
    short v6 = 0;
    short v7 = 0;
    short v8 = 0;
    short v9 = 0;
    short v10 = 0;
    short v11 = 0;
    short v12 = 0;
    short v13 = 0;
    short v14 = 0;
    short native_tail_0 = 0;
    short native_tail_1 = 0;

    message.pvp_result.Clear();
    int* const values = message.pvp_result.values;
    if (packet.get_short(v14) != 1 ||
        packet.get_short(v13) != 1 ||
        packet.get_short(v12) != 1 ||
        packet.get_short(v11) != 1 ||
        packet.get_short(v10) != 1 ||
        packet.get_short(v9) != 1 ||
        packet.get_short(v8) != 1 ||
        packet.get_short(v7) != 1 ||
        packet.get_short(v6) != 1 ||
        packet.get_short(v5) != 1 ||
        packet.get_short(v4) != 1 ||
        packet.get_int(values[17]) != 1 ||
        packet.get_int(values[27]) != 1 ||
        packet.get_int(values[28]) != 1 ||
        packet.get_int(values[29]) != 1 ||
        packet.get_int(values[30]) != 1 ||
        packet.get_int(values[18]) != 1 ||
        packet.get_int(values[19]) != 1 ||
        packet.get_int(values[20]) != 1 ||
        packet.get_int(values[31]) != 1 ||
        packet.get_int(values[32]) != 1 ||
        packet.get_short(native_tail_0) != 1 ||
        packet.get_short(native_tail_1) != 1) {
        return 0;
    }
    values[6] = v14;
    values[7] = v13;
    values[8] = v12;
    values[9] = v11;
    values[14] = v10;
    values[15] = v9;
    values[12] = v8;
    values[13] = v7;
    values[10] = v6;
    values[11] = v5;
    values[16] = v4;
    return 1;
}

// IDA_EA: 0x81F05EA
// IDA_NAME: _ZN21DisPatcher_ResPvpRank11SavePvpRankEP5CUserR16MSG_RES_PVP_RANK
// DEMANGLED: DisPatcher_ResPvpRank::SavePvpRank(CUser *,MSG_RES_PVP_RANK &)
int DisPatcher_ResPvpRank::SavePvpRank(CUser* user,
                                        MSG_RES_PVP_RANK& message) {
    PvP_Room* const zombie_room = user->GetPVPRoom();
    if (zombie_room &&
        (zombie_mode::IsActiveRoom(zombie_room) ||
         zombie_mode::IsFinishedRoom(zombie_room))) {
        // NOTI49/CMD58 remains the client end-of-battle receive barrier for
        // Zombie mode, but its ordinary PvP rank payload is not authoritative
        // and must not reach update_pvp_rank, SetPVPResult, or pvp_reward.
        if (zombie_room->set_recv_pvp_rank_flag(user) != 1) {
            return 0;
        }
        if (!zombie_room->recv_pvp_rank_all() &&
            zombie_room->get_recv_pvp_rank_count() == 1) {
            zombie_room->insert_timer_recv_pvp_rank();
        }
        return 1;
    }

    unsigned int pvp_point =
        static_cast<unsigned int>(message.pvp_result.values[31]);
    const unsigned int reported_value =
        static_cast<unsigned int>(message.pvp_result.values[32]) / 1000u;
    if (pvp_point > 10000u) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x76u),
            1u,
            pvp_point / 10000u,
            reported_value);
        pvp_point %= 10000u;
    }

    const int job = user->get_charac_job();
    const long double allowed_point =
        (job == oldsrv::config::kCharacterJobMage ||
         (job == oldsrv::config::kCharacterJobPriest &&
          user->getCurCharFirstGrowType() == 2))
            ? static_cast<long double>(pvp_point) * 0.5L
            : static_cast<long double>(pvp_point) * 0.8L;
    if (static_cast<unsigned int>(allowed_point) > reported_value) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x73u),
            1u,
            static_cast<unsigned int>(allowed_point),
            reported_value);
    }

    if (user->update_pvp_rank(message.pvp_result) != 1) {
        return 0;
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return 0;
    }

    const int user_seat = room->get_user_seat(user);
    room->SetPVPResult(user_seat, message.pvp_result);
    if (room->set_recv_pvp_rank_flag(user) != 1) {
        return 0;
    }
    if (room->recv_pvp_rank_all()) {
        room->send_pvp_end();
    } else if (room->get_recv_pvp_rank_count() == 1) {
        room->insert_timer_recv_pvp_rank();
    }
    return 1;
}

// IDA_EA: 0x8232E8E
// IDA_NAME: _ZN21DisPatcher_ResPvpRank4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_ResPvpRank::read(PacketBuf &,MSG_BASE &)
int DisPatcher_ResPvpRank::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232E98
// IDA_NAME: _ZN21DisPatcher_ResPvpRank7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_ResPvpRank::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_ResPvpRank::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8232E40
// IDA_NAME: _ZN21DisPatcher_ResPvpRankD2Ev
// DEMANGLED: DisPatcher_ResPvpRank::~DisPatcher_ResPvpRank()
// IDA_EA: 0x8232E70
// IDA_NAME: _ZN21DisPatcher_ResPvpRankD0Ev
// DEMANGLED: DisPatcher_ResPvpRank::~DisPatcher_ResPvpRank()
DisPatcher_ResPvpRank::~DisPatcher_ResPvpRank() {}

// IDA_EA: 0x85954F4
// IDA_NAME: _ZN21DisPatcher_ResPvpRankC2Ev
// DEMANGLED: DisPatcher_ResPvpRank::DisPatcher_ResPvpRank(void)
DisPatcher_ResPvpRank::DisPatcher_ResPvpRank()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
