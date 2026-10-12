#include "old_game/social/party/packet_dispatcher_set_play_result.h"

#include <stdint.h>

#include "common/config.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/character/load_charac_payload.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kSetPlayResultSignature[] =
    "virtual int DisPatcher_SetPlayResult::dispatch_sig(CUser*, PacketBuf&)";

void AddSetPlayResultHack(CUser* user,
                          int hack_type,
                          int count,
                          int value1,
                          int value2) {
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
        static_cast<unsigned int>(count),
        static_cast<unsigned int>(value1),
        static_cast<unsigned int>(value2));
}

bool IsHighLevel(CUser* user) {
    SIG_LOAD_CHARAC payload;
    return user->GetCharacInfo(&payload) == 1 &&
           payload.charac_data.level > 29;
}

bool HasRequiredClearEquipment(CParty* party) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const member = party->get_user(seat);
        CInventory* const inventory = member ? member->getCurCharacInvenR() : 0;
        if (!inventory) {
            continue;
        }

        // The original sret calls read the equipment slots whose item-key
        // word is compared with 10964 and 12958.  Preserve the two equipment
        // slots as typed inventory reads instead of retaining stack aliases.
        if (inventory->GetInvenSlot(0, 0).getKey() == 10964 ||
            inventory->GetInvenSlot(
                0, oldsrv::config::kEquipmentTypeWeapon).getKey() == 12958) {
            return true;
        }
    }
    return false;
}

}  // namespace

template class IPacketDispatcher<MSG_BASE,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595580
// IDA_NAME: _ZN24DisPatcher_SetPlayResultC2Ev
// DEMANGLED: DisPatcher_SetPlayResult::DisPatcher_SetPlayResult(void)
DisPatcher_SetPlayResult::DisPatcher_SetPlayResult()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823303E
// IDA_NAME: _ZN24DisPatcher_SetPlayResultD2Ev
// DEMANGLED: DisPatcher_SetPlayResult::~DisPatcher_SetPlayResult()
// IDA_EA: 0x823306E
// IDA_NAME: _ZN24DisPatcher_SetPlayResultD0Ev
// DEMANGLED: DisPatcher_SetPlayResult::~DisPatcher_SetPlayResult()
DisPatcher_SetPlayResult::~DisPatcher_SetPlayResult() {}

// IDA_EA: 0x81F5DB6
// IDA_NAME: _ZN24DisPatcher_SetPlayResult10SendResultEP6CPartyR13GameResultSeti
// DEMANGLED: DisPatcher_SetPlayResult::SendResult(CParty *,GameResultSet &,int)
int DisPatcher_SetPlayResult::SendResult(CParty* party, GameResultSet& result) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PLAY_RESULT);
    packet.put_byte(result.raw_48);
    packet.put_int(result.raw_0c);
    packet.put_byte(result.raw_10);
    packet.put_byte(result.raw_09);
    party->makeBestClearTimePacket(packet, result.raw_0c);
    packet.finalize(true);
    party->send_to_party(packet);
    return 1;
}

// IDA_EA: 0x81F4CE6
// IDA_NAME: _ZN24DisPatcher_SetPlayResult12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetPlayResult::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetPlayResult::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 5) {
        return 0;
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return LineFunc(33109, kSetPlayResultSignature, 0, 0);
    }

    CDungeon* const dungeon = party->battle_field().get_dungeon();
    PvP_Room* const zombie_room = user->GetPVPRoom();
    if ((dungeon && dungeon->isZombieDungeon()) ||
        (zombie_room &&
         (zombie_mode::IsActiveRoom(zombie_room) ||
          zombie_mode::IsFinishedRoom(zombie_room)))) {
        return 0;
    }

    // Raw CParty+0xcd8 is CBattle_Field+0x1b4: CMD16's instance mode,
    // not CParty::IsAutoCreated().  This report is suppressed only for
    // mode 1, as in the original direct dispatcher.
    if (party->battle_field().instance_dungeon_mode() == 1) {
        return 0;
    }

    if (dungeon && dungeon->get_blood_dungeon_type() > 0) {
        return 0;
    }

    GameResultSet parsed_result;
    // 140828 sender sub_AD8A40 writes:
    //   u8, u16, u8 count, count * {u16,u32,u8}, u8 aggregate_max,
    //   two u32 report values, five u32 integrity values, u8 mode.
    // The restored 130627 reader expected an obsolete u8 between the first
    // byte and u16 and omitted both new u8 fields.
    if (packet.get_byte(parsed_result.raw_09) != 1) {
        return LineFunc(33133, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_short(parsed_result.raw_4a) != 1) {
        return LineFunc(33140, kSetPlayResultSignature, 0, 0);
    }

    uint8_t member_result_count = 0;
    if (packet.get_byte(member_result_count) != 1) {
        return LineFunc(33145, kSetPlayResultSignature, 0, 0);
    }
    // Ordinary CParty owns exactly four result slots. Silently truncating a
    // larger client count leaves its remaining six-byte records in the cursor
    // and misreads the first one as claimed_boss_clear, so reject the frame at
    // the count boundary instead.
    if (member_result_count > 4u) {
        return LineFunc(33146, kSetPlayResultSignature, 0, 0);
    }
    for (int seat = 0; seat < member_result_count; ++seat) {
        if (packet.get_short(parsed_result.shot_counts[seat].shot_count) != 1) {
            return LineFunc(33150, kSetPlayResultSignature, 0, 0);
        }
        if (packet.get_int(parsed_result.shot_counts[seat].raw_04) != 1) {
            return LineFunc(33153, kSetPlayResultSignature, 0, 0);
        }
        uint8_t member_auxiliary = 0;
        if (packet.get_byte(member_auxiliary) != 1) {
            return LineFunc(33154, kSetPlayResultSignature, 0, 0);
        }
        // 140828 emits this per-member value before computing the aggregate
        // maximum below.  It belongs to the seven-byte record and must still
        // be consumed, but it is not the party rank input.
        (void)member_auxiliary;
    }
    if (packet.get_byte(parsed_result.raw_0a) != 1) {
        return LineFunc(33155, kSetPlayResultSignature, 0, 0);
    }
    // CLIENT_IDA_EVIDENCE: sub_AD8A40 writes each member byte at 0xAD8C75,
    // updates EBX with max(EBX, ESI) at 0xAD8C7A..0xAD8C7E, then emits that
    // aggregate byte at 0xAD8C92.  The old Release adapter's first-nonzero
    // member approximation is therefore not authoritative.  Feed the native
    // aggregate to the old raw_09 rank owner unconditionally, including zero,
    // while retaining raw_0a for its native downstream consumer.
    parsed_result.raw_09 = parsed_result.raw_0a;

    int claimed_boss_clear = 0;
    if (packet.get_int(claimed_boss_clear) != 1) {
        return LineFunc(33163, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_int(parsed_result.raw_4c) != 1) {
        return LineFunc(33173, kSetPlayResultSignature, 0, 0);
    }

    int unknown_0 = 0;
    int damage_numerator_0 = 0;
    int damage_numerator_1 = 0;
    int damage_denominator_0 = 0;
    int damage_denominator_1 = 0;
    if (packet.get_int(unknown_0) != 1) {
        return LineFunc(33294, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_int(damage_numerator_0) != 1) {
        return LineFunc(33295, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_int(damage_numerator_1) != 1) {
        return LineFunc(33296, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_int(damage_denominator_0) != 1) {
        return LineFunc(33297, kSetPlayResultSignature, 0, 0);
    }
    if (packet.get_int(damage_denominator_1) != 1) {
        return LineFunc(33298, kSetPlayResultSignature, 0, 0);
    }
    uint8_t report_mode = 0;
    if (packet.get_byte(report_mode) != 1) {
        return LineFunc(33299, kSetPlayResultSignature, 0, 0);
    }
    (void)report_mode;

    GameResultSet& result = party->game_result_set();
    result.Clear();
    result.raw_09 = parsed_result.raw_09;
    result.raw_4a = parsed_result.raw_4a;
    result.raw_0a = parsed_result.raw_0a;
    result.raw_4c = parsed_result.raw_4c;
    for (int seat = 0; seat < member_result_count; ++seat) {
        result.shot_counts[seat].shot_count =
            parsed_result.shot_counts[seat].shot_count;
        result.shot_counts[seat].raw_04 =
            parsed_result.shot_counts[seat].raw_04;
    }

    user->history_log().MapClear(
        result.raw_4a,
        static_cast<int>(party->GetMapPlayingTime() / 1000u),
        0);

    if (!party->checkBossRoom() && claimed_boss_clear != 0) {
        AddSetPlayResultHack(user, 507, claimed_boss_clear, 0, 0);
    }

    if (result.raw_4c > 10000) {
        const int quotient = result.raw_4c / 10000;
        result.raw_4c %= 10000;
        AddSetPlayResultHack(user, 120, 1, quotient, 0);
    }

    const bool half_scale =
        user->get_charac_job() == oldsrv::config::kCharacterJobMage ||
        (user->get_charac_job() == oldsrv::config::kCharacterJobPriest &&
                             user->getCurCharFirstGrowType() == 2);
    result.raw_4c = static_cast<int>(
        static_cast<long double>(result.raw_4c) *
        (half_scale ? 0.5L : 0.8L));
    if (static_cast<uint32_t>(result.raw_4c) >
        party->GetMapPlayingTime() / 1000u) {
        AddSetPlayResultHack(user, 117, 1, result.raw_4c, 0);
    }

    result.raw_04 = party->battle_field().get_dungeon_index();
    result.raw_08 = static_cast<uint8_t>(
        party->battle_field().get_dungeon_diff());
    party->IncreaseSetPlayResultCount(user);

    if (party->get_dungeon_clear_state() == 0) {
        if (party->battle_field().IsClearDungeon() != 1) {
            AddSetPlayResultHack(user, 406, 1, 0, 0);
            return 0;
        }
        party->insert_time_set_play_result_msg(15);
        party->set_dungeon_clear_state(1);
    }

    MapInfo* const current_map = party->battle_field().GetCurrentMapInfo();
    if (current_map &&
        current_map->normal_kill_check_.getUseSkillMaterialTime() == 0 &&
        IsHighLevel(user)) {
        AddSetPlayResultHack(user, 226, 1, 0, 0);
    }

    const uint32_t map_time_sec = static_cast<uint32_t>(
        party->GetMapPlayingTime() / 1000u);
    const uint32_t required_material_uses = (map_time_sec + 9u) / 10u;
    if (current_map &&
        static_cast<uint32_t>(
            current_map->normal_kill_check_.getUseSkillMaterialCount()) <
            required_material_uses &&
        IsHighLevel(user)) {
        AddSetPlayResultHack(user, 227, 1, 0, 0);
    }

    if (map_time_sec <= 9999u && user->get_charac_level() > 10) {
        AddSetPlayResultHack(user,
                              229,
                              1 - static_cast<int>(map_time_sec) + 10,
                              0,
                              0);
    }
    if (map_time_sec <= 19999u && user->get_charac_level() > 10) {
        AddSetPlayResultHack(user, 228, 1, 0, 0);
    }

    if (unknown_0 == 0 && party->GetUsedCoinCount() == 0 &&
        party->battle_field().GetTotalSkillMaterial() == 0 &&
        damage_denominator_0 != 0 &&
        static_cast<long double>(damage_numerator_0) /
                static_cast<long double>(damage_denominator_0) * 100.0L >=
            99.989998L &&
        damage_denominator_1 != 0 &&
        static_cast<long double>(damage_numerator_1) /
                static_cast<long double>(damage_denominator_1) * 100.0L >=
            99.989998L &&
        !HasRequiredClearEquipment(party)) {
        AddSetPlayResultHack(user, 418, 1, 0, 0);
    }

    if (party->CheckRecvFromAllMember(user) != 1) {
        return 0;
    }
    if (party->SetPlayResult(user) == 1) {
        return 0;
    }
    return LineFunc(33333, kSetPlayResultSignature, 0, 0);
}

// IDA_EA: 0x823302A
// IDA_NAME: _ZN24DisPatcher_SetPlayResult4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPlayResult::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPlayResult::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233034
// IDA_NAME: _ZN24DisPatcher_SetPlayResult7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPlayResult::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPlayResult::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
