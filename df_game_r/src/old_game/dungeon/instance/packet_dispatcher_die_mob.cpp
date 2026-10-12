#include "old_game/dungeon/instance/packet_dispatcher_die_mob.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hack_log_overdamage_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/log/format/log_manager.h"

namespace {

const char kDieMobSignature[] =
    "virtual int DisPatcher_DieMob::dispatch_sig(CUser*, PacketBuf&)";

struct DieMobTail {
    // These are parsed after the MSG_MONSTER_DIE body in the exact client
    // order.  The two flags are dispatcher controls, not persistent monster
    // fields: one reports a forced movement and the other destroys a passive
    // object before ordinary monster resolution.
    uint8_t passive_object;
    uint8_t monster_move_hack;
    uint16_t unknown_b;
    uint8_t unknown_c;
    int32_t unknown_d;
    int32_t unknown_e;
    int16_t unknown_f;
    int16_t unknown_g;
    int16_t unknown_h;
    int16_t unknown_i;
    int16_t unknown_j;
    int16_t unknown_k;
    int16_t unknown_l;
};

int SendKillRejected(CUser* user, uint16_t monster_unique_id) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_DIE_MONSTER);
    response.put_byte(0);
    response.put_byte(5);
    response.put_short(monster_unique_id);
    response.finalize(true);
    user->Send(response);
    return 0;
}

int ReadDieMobPrefix(PacketBuf& packet,
                     MSG_MONSTER_DIE& message,
                     DieMobTail& tail) {
    std::memset(&message, 0, sizeof(message));
    std::memset(&tail, 0, sizeof(tail));

    if (packet.get_short(message.monster_id_) != 1) {
        return LineFunc(27913, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(message.monster_type_) != 1) {
        return LineFunc(27914, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(message.field_11_) != 1) {
        return LineFunc(27916, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(message.field_15_) != 1) {
        return LineFunc(27917, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(message.field_a1a_) != 1) {
        return LineFunc(27918, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(message.field_a1e_) != 1) {
        return LineFunc(27919, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(message.entry_count_) != 1) {
        return LineFunc(27921, kDieMobSignature, 0, 0);
    }
    for (unsigned int index = 0; index < message.entry_count_; ++index) {
        if (packet.get_short(message.entry_u16_a_[index]) != 1) {
            return LineFunc(27926, kDieMobSignature, 0, 0);
        }
        if (packet.get_short(message.entry_u16_b_[index]) != 1) {
            return LineFunc(27927, kDieMobSignature, 0, 0);
        }
        if (packet.get_int(message.entry_u32_[index]) != 1) {
            return LineFunc(27928, kDieMobSignature, 0, 0);
        }
        if (packet.get_short(message.entry_u16_c_[index]) != 1) {
            return LineFunc(27929, kDieMobSignature, 0, 0);
        }
    }
    if (packet.get_short(message.field_a24_) != 1) {
        return LineFunc(27941, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(message.field_a26_) != 1) {
        return LineFunc(27942, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(message.field_a22_) != 1) {
        return LineFunc(27944, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(message.field_a23_) != 1) {
        return LineFunc(27947, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(tail.passive_object) != 1) {
        return LineFunc(27951, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(tail.monster_move_hack) != 1) {
        return LineFunc(27954, kDieMobSignature, 0, 0);
    }
    return 0;
}

int ReadDieMobSuffix(PacketBuf& packet, DieMobTail& tail) {
    if (packet.get_short(tail.unknown_b) != 1) {
        return LineFunc(27966, kDieMobSignature, 0, 0);
    }
    if (packet.get_byte(tail.unknown_c) != 1) {
        return LineFunc(27969, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(tail.unknown_d) != 1) {
        return LineFunc(27970, kDieMobSignature, 0, 0);
    }
    if (packet.get_int(tail.unknown_e) != 1) {
        return LineFunc(27971, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_f) != 1) {
        return LineFunc(27972, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_g) != 1) {
        return LineFunc(27973, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_h) != 1) {
        return LineFunc(27974, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_i) != 1) {
        return LineFunc(27975, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_j) != 1) {
        return LineFunc(27976, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_k) != 1) {
        return LineFunc(27977, kDieMobSignature, 0, 0);
    }
    if (packet.get_short(tail.unknown_l) != 1) {
        return LineFunc(27978, kDieMobSignature, 0, 0);
    }
    return 0;
}

void AddDieMobHack(CUser* user, unsigned int hack_type) {
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
        1u,
        0u,
        0u);
}

// IDA 0x81EB0C4 executes these report-integrity guards only after the
// battlefield accepts the ordinary-dungeon kill.  They deliberately use the
// client supplied hit records as a report check rather than game damage.
void CheckNormalDieMobReport(CUser* user,
                             CParty* party,
                             const MSG_MONSTER_DIE& message) {
    int zero_or_large_hit_count = message.field_a1e_;
    int missing_hit_marker = message.field_a1a_;
    for (unsigned int index = 0; index < message.entry_count_; ++index) {
        if (message.entry_u16_a_[index] != 273u) {
            continue;
        }
        CUser* const reporter =
            G_GameWorld()->find_from_world(message.entry_u16_b_[index]);
        if (!reporter || party->isMyMember(message.entry_u16_b_[index]) != 1) {
            continue;
        }

        uint32_t limit_damage = 0u;
        WongWork::ENUM_HACKTYPE over_damage_hack =
            static_cast<WongWork::ENUM_HACKTYPE>(107u);
        CHackLog_OverDamage::GetDungeonLimitDamageAndHackType(
            reporter, limit_damage, over_damage_hack);

        if (zero_or_large_hit_count > 0x5dcu) {
            AddDieMobHack(user, 0x6fu);
            zero_or_large_hit_count = 1;
        }
        if (zero_or_large_hit_count == 1 &&
            party->battle_field().get_dungeon_index() == 11006) {
            AddDieMobHack(reporter, 0x7fu);
        }
        if (missing_hit_marker == 0) {
            AddDieMobHack(user, 0x6du);
            missing_hit_marker = 1;
        }
        if (message.entry_u16_c_[index] == 0) {
            AddDieMobHack(user, 0x6eu);
        }

        const uint16_t reported_hits =
            message.entry_u16_c_[index] == 0u ? 1u
                                               : message.entry_u16_c_[index];
        const uint32_t average_damage = message.entry_u32_[index] /
                                        reported_hits;
        const bool special_dungeon =
            party->battle_field().GetDungeon() != 0 &&
            party->battle_field().get_dungeon_index() == 11006;
        if (average_damage > limit_damage && !special_dungeon) {
            reporter->getHackAnalyzer()->addServerHackCnt(
                reporter, over_damage_hack, 1u, 0u, 0u);
            if (average_damage > static_cast<uint32_t>(
                                     CHackLog_OverDamage::
                                         GetDungeonLimitMaxDamage())) {
                AddDieMobHack(
                    reporter,
                    static_cast<unsigned int>(CHackLog_OverDamage::
                                                  GetHackTypeOverLimitMaxDamage()));
            }
        }
    }

    // The old body uses the client counters after the whole report traversal,
    // rather than only when a member record was accepted.
    if ((zero_or_large_hit_count % 10) == 0 &&
        (missing_hit_marker % 10) == 0) {
        AddDieMobHack(user, 0xcau);
    }
}

void CheckTowerDieMobReport(CUser* user, const MSG_MONSTER_DIE& message) {
    int zero_or_large_hit_count = message.field_a1e_;
    int missing_hit_marker = message.field_a1a_;
    for (unsigned int index = 0; index < message.entry_count_; ++index) {
        if (message.entry_u16_a_[index] != 273u) {
            continue;
        }
        CUser* const reporter =
            G_GameWorld()->find_from_world(message.entry_u16_b_[index]);
        if (!reporter) continue;
        if (zero_or_large_hit_count > 0x5dcu) {
            AddDieMobHack(user, 0x6fu);
            zero_or_large_hit_count = 1;
        }
        uint16_t reported_hits = message.entry_u16_c_[index];
        if (reported_hits == 0u) {
            AddDieMobHack(user, 0x6eu);
            // The original computes the average after first recording the
            // zero-count violation.  The reconstructed form uses the same
            // repaired denominator so a malformed report cannot divide by
            // zero before it is logged.
            reported_hits = 1u;
        }

        const uint32_t average_damage = message.entry_u32_[index] /
                                        reported_hits;
        if (average_damage > static_cast<uint32_t>(
                                 CHackLog_OverDamage::GetBossTowerLimitDamage())) {
            AddDieMobHack(
                reporter,
                static_cast<unsigned int>(CHackLog_OverDamage::
                                              GetBossTowerOverDamageHackType()));
            if (average_damage > static_cast<uint32_t>(
                                     CHackLog_OverDamage::
                                         GetDungeonLimitMaxDamage())) {
                AddDieMobHack(
                    reporter,
                    static_cast<unsigned int>(CHackLog_OverDamage::
                                                  GetHackTypeOverLimitMaxDamage()));
            }
        }
        if (missing_hit_marker == 0) {
            AddDieMobHack(user, 0x6du);
            missing_hit_marker = 1;
        }
    }
}

// CMD42 state-10 has a distinct tower threshold set: report-zero is charged
// to the killer, the 0x32c7 per-hit ceiling is charged to the killer, and the
// fixed 700000 hard cap is charged to the reporting party member.
void CheckDeathTowerDieMobReport(CUser* user,
                                 const MSG_MONSTER_DIE& message) {
    for (unsigned int index = 0; index < message.entry_count_; ++index) {
        if (message.entry_u16_a_[index] != 273u) continue;
        CUser* const reporter =
            G_GameWorld()->find_from_world(message.entry_u16_b_[index]);
        if (!reporter) continue;

        uint16_t reported_hits = message.entry_u16_c_[index];
        if (reported_hits == 0u) {
            AddDieMobHack(user, 0x6eu);
            reported_hits = 1u;
        }
        const uint32_t average_damage = message.entry_u32_[index] /
                                        reported_hits;
        if (average_damage > 0x32c7u) {
            AddDieMobHack(user, 0x6cu);
        }
        if (average_damage > static_cast<uint32_t>(
                                 CHackLog_OverDamage::
                                     GetDeathTowerLimitDamage())) {
            AddDieMobHack(
                reporter,
                static_cast<unsigned int>(CHackLog_OverDamage::
                                              GetDeathTowerOverDamageHackType()));
        }
    }
}

}  // namespace

template class IPacketDispatcher<MSG_BASE,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595414
// IDA_NAME: _ZN17DisPatcher_DieMobC2Ev
// DEMANGLED: DisPatcher_DieMob::DisPatcher_DieMob(void)
DisPatcher_DieMob::DisPatcher_DieMob()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8232B08
// IDA_NAME: _ZN17DisPatcher_DieMobD2Ev
// DEMANGLED: DisPatcher_DieMob::~DisPatcher_DieMob()
// IDA_EA: 0x8232B38
// IDA_NAME: _ZN17DisPatcher_DieMobD0Ev
// DEMANGLED: DisPatcher_DieMob::~DisPatcher_DieMob()
DisPatcher_DieMob::~DisPatcher_DieMob() {}

// IDA_EA: 0x81EB0C4
// IDA_NAME: _ZN17DisPatcher_DieMob12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DieMob::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_DieMob::dispatch_sig(CUser* user, PacketBuf& packet) {
    MSG_MONSTER_DIE message;
    DieMobTail tail;
    const int read_result = ReadDieMobPrefix(packet, message, tail);
    if (read_result != 0) {
        return read_result;
    }

    const int state = user->get_state();
    CParty* normal_party = 0;
    // The old dispatcher applies this state-5 consequence immediately after
    // packet byte 27954, before reading the remaining tail.  In particular,
    // a malformed tail must not suppress an already-observed forced move.
    if (tail.monster_move_hack != 0 && state == 5) {
        normal_party = G_CGameManager()->GetParty(user->GetPartyIndex());
        if (!normal_party) {
            return LineFunc(27960, kDieMobSignature, 0, 0);
        }
        normal_party->setMonsterMoveHack();
    }

    const int tail_result = ReadDieMobSuffix(packet, tail);
    if (tail_result != 0) {
        return tail_result;
    }

    if (state == 5) {
        CParty* const party = normal_party
                                  ? normal_party
                                  : G_CGameManager()->GetParty(
                                        user->GetPartyIndex());
        if (!party) {
            return LineFunc(27988, kDieMobSignature, 0, 0);
        }
        // 140828 client ownership: NOTI31 tears down the current monster
        // objects before it can consume later NOTI38 packets from the same
        // TCP kill batch.  ClearDungeon() latches CParty+0x011a before the
        // dispatcher can process the next 0828 CMD39, so reject that stale
        // report
        // at the authoritative party boundary instead of broadcasting a
        // monster UID that no longer has a client-side object.
        if (party->isTournamentVictory() != 0) {
            LogManager::logFormat(
                1,
                "dungeon42_diag.cpp",
                kDieMobSignature,
                0,
                "[DUNGEON42][POST_CLEAR_REJECT] uid=%d monster_uid=%u monster_type=%u",
                user->GetUID(),
                static_cast<unsigned int>(message.monster_id_),
                static_cast<unsigned int>(message.monster_type_));
            return SendKillRejected(user, message.monster_id_);
        }
        party->recordDieMobReport(message.monster_id_);
        if (tail.passive_object == 1) {
            party->DestroyPassiveObject(static_cast<int>(message.monster_id_));
            return 0;
        }

        int monster_result = 0;
        const int kill_result = party->kill_monster(
            user,
            static_cast<int>(message.monster_id_),
            message.monster_type_,
            message,
            &monster_result);
        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "virtual int DisPatcher_DieMob::dispatch_sig(CUser*, PacketBuf&)",
            0,
            "[DUNGEON42][RESULT] uid=%d monster_uid=%u monster_type=%u no_reward=%u complete=%u kill_result=%d monster_index=%d",
            user->GetUID(),
            static_cast<unsigned int>(message.monster_id_),
            static_cast<unsigned int>(message.monster_type_),
            static_cast<unsigned int>(message.field_a22_),
            static_cast<unsigned int>(message.field_a23_),
            kill_result,
            monster_result);
        if (kill_result != 1) {
            return SendKillRejected(user, message.monster_id_);
        }
        user->GetHackCheck()->DieMob(&message);

        if (message.monster_type_ == 0xffffu) {
            if (++user->special_monster_kill_counter_ > 4u) {
                user->special_monster_kill_counter_ = 0;
                const unsigned int special_dungeon_value =
                    party->battle_field().get_dungeon_index() == 11006
                        ? 2u
                        : 0u;
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0xcdu),
                    1u,
                    special_dungeon_value,
                    0u);
                if (!party->checkBossRoom()) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(0xceu),
                        1u,
                        special_dungeon_value,
                        0u);
                }
            }
        } else {
            user->special_monster_kill_counter_ = 0;
            const uint32_t now = static_cast<uint32_t>(
                GlobalData::s_systemTime_.getCurTickCount());
            if (user->monster_kill_position_.Check(
                    now,
                    message.field_a24_,
                    message.field_a26_,
                    0x258u,
                    0x258u,
                    0x3e8u)) {
                AddDieMobHack(user, 0xd1u);
            }
        }
        CheckNormalDieMobReport(user, party, message);
        WongWork::CHackAnalyzer::ValidateFixMobDiePos(
            user,
            static_cast<uint16_t>(monster_result),
            message.field_a24_,
            message.field_a26_);
        return 0;
    }

    if (state == 10) {
        WongWork::CDeathTower* const tower = user->getDeathTower();
        if (!tower) {
            return 0;
        }
        if (message.monster_type_ == 0xffffu) {
            CParty* const party = G_CGameManager()->GetParty(
                user->GetPartyIndex());
            if (!party) {
                return LineFunc(28287, kDieMobSignature, 0, 0);
            }
            if (++user->special_monster_kill_counter_ > 4u) {
                user->special_monster_kill_counter_ = 0;
                user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<WongWork::ENUM_HACKTYPE>(0xcdu),
                    1u, 1u, 0u);
                user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<WongWork::ENUM_HACKTYPE>(0xceu),
                    1u, 1u, 0u);
            }
        } else {
            user->special_monster_kill_counter_ = 0;
            const uint32_t now = static_cast<uint32_t>(
                GlobalData::s_systemTime_.getCurTickCount());
            if (user->monster_kill_position_.Check(
                    now,
                    message.field_a24_,
                    message.field_a26_,
                    0x258u,
                    0x258u,
                    0x3e8u)) {
                AddDieMobHack(user, 0xd4u);
            }
        }

        CheckDeathTowerDieMobReport(user, message);

        int monster_result = 0;
        tower->handleKillMonster(user,
                                 message.monster_id_,
                                 message.monster_type_,
                                 message,
                                 &monster_result);
        WongWork::CHackAnalyzer::ValidateFixMobDiePos(
            user,
            static_cast<uint16_t>(monster_result),
            message.field_a24_,
            message.field_a26_);
        user->GetHackCheck()->DieMob(&message);
        return 0;
    }

    if (state == 12) {
        if (message.monster_type_ == 0xffffu) {
            AddDieMobHack(user, 0xcdu);
            AddDieMobHack(user, 0xceu);
        }
        CheckTowerDieMobReport(user, message);
        int monster_result = 0;
        Global::CBossTowerFunc::DieMob(user,
                                       message.monster_id_,
                                       message.monster_type_,
                                       message,
                                       &monster_result);
        return 0;
    }

    if (state == 13) {
        advancealtar::Manager::dieMob(user,
                                      message.monster_id_,
                                      message.monster_type_,
                                      message);
        return 0;
    }

    if (state == 8) {
        WarRoom* const room =
            G_CGameManager()->GetWarRoom(user->GetWarRoomIndex());
        if (!room) {
            return LineFunc(28368, kDieMobSignature, 0, 0);
        }

        CUser* target = 0;
        if (message.monster_type_ != 0xffffu) {
            target = G_GameWorld()->find_from_world(message.monster_type_);
            if (!target) {
                return LineFunc(28379, kDieMobSignature, 0, 0);
            }
        }
        if (room->HandleMonsterKill(user,
                                    static_cast<int>(message.monster_id_),
                                    target) != 1) {
            return SendKillRejected(user, message.monster_id_);
        }
    }
    return 0;
}

// IDA_EA: 0x8232AF4
// IDA_NAME: _ZN17DisPatcher_DieMob4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DieMob::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DieMob::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232AFE
// IDA_NAME: _ZN17DisPatcher_DieMob7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DieMob::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DieMob::process(CUser* user,
                               MSG_BASE& message,
                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
