#include <cstring>

#include "infra/log/format/trace.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// These are the original const member symbols.  CDungeon's complete C-owned
// layout is not needed by this party-side admission transaction.
// IDA_EA: 0x85ABC80
// IDA_NAME: _ZN6CParty26checkInoutConditionDungeonEPK8CDungeon17ENUM_DUNGEON_MODE14ENUM_CMDPACKET
// DEMANGLED: CParty::checkInoutConditionDungeon(
//     CDungeon const*,ENUM_DUNGEON_MODE,ENUM_CMDPACKET)
int CParty::checkInoutConditionDungeon(const CDungeon* dungeon,
                                       ENUM_DUNGEON_MODE mode,
                                       ENUM_CMDPACKET command) {
    if (!dungeon) {
        return 1;
    }

    PacketGuard packet;
    unsigned char failed_member = 0;

    // The original constructs four Inven_Item objects, then deliberately
    // clears their serialised state before CheckEnterAncientDungeon fills it.
    // Keep both the concrete object lifetime and that post-C2 zeroing.
    Inven_Item required_items[4];
    std::memset(required_items, 0, sizeof(required_items));
    int required_item_slots[4];
    std::memset(required_item_slots, 0, sizeof(required_item_slots));

    if (command == ENUM_CMDPACKET_REGISTER_QUICK_PARTY) {
        // IDA executes this preliminary four-seat validation pass before the
        // stamina rejection pass; retain it even though its result is unused.
        for (int seat = 0; seat <= 3; ++seat) {
            (void)checkValidUser(seat);
        }

        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const user = get_user(seat);
            if (checkValidUser(seat) == 1 && user &&
                user->getCurCharacStamina() != 100) {
                packet.clear();
                packet.put_header(1, ENUM_CMDPACKET_REGISTER_QUICK_PARTY);
                packet.put_byte(0);
                packet.put_byte(234);
                packet.put_byte(failed_member);
                packet.finalize(true);
                send_to_party(packet);
                return 234;
            }
        }
    }

    if (!CheckEnterAncientDungeon(dungeon, failed_member, required_items,
                                  required_item_slots)) {
        packet.clear();
        packet.put_header(1, command);
        packet.put_byte(0);
        packet.put_byte(17);
        packet.put_byte(failed_member);
        packet.finalize(true);
        send_to_party(packet);
        return 17;
    }

    if (reinterpret_cast<const CDungeon*>(dungeon)->get_index() == 100 &&
        UseAncientDungeonItems(dungeon, required_items, required_item_slots) !=
            1) {
        PacketGuard select_dungeon_packet;
        select_dungeon_packet.put_header(1, ENUM_CMDPACKET_SELECT_DUNGEON);
        select_dungeon_packet.put_byte(0);
        select_dungeon_packet.put_int(1);
        select_dungeon_packet.finalize(true);
        send_to_party(select_dungeon_packet);
        return 17;
    }

    if (reinterpret_cast<const CDungeon*>(dungeon)->get_dimension_possible() > 0) {
        failed_member = 0;
        const int dimension_result =
            CheckEnterDimensionDungeon(dungeon, failed_member, true);
        if (dimension_result == 172 || dimension_result == 173) {
            packet.clear();
            packet.put_header(1, command);
            packet.put_byte(0);
            packet.put_byte(dimension_result);
            packet.put_byte(failed_member);
            packet.finalize(true);
            send_to_party(packet);
            return dimension_result;
        }
        if (dimension_result == 171 || setStandardDimensionLevel(dungeon) != 1) {
            packet.clear();
            packet.put_header(1, command);
            packet.put_byte(0);
            packet.put_byte(171);
            packet.finalize(true);
            send_to_party(packet);
            return 171;
        }
    }

    int start_failed_member = 0;
    int start_result = 0;
    if (mode != static_cast<ENUM_DUNGEON_MODE>(1)) {
        start_result = check_dungeon_start(dungeon, start_failed_member);
    }
    if (mode == static_cast<ENUM_DUNGEON_MODE>(1) || start_result == 0) {
        const int dungeon_index = reinterpret_cast<const CDungeon*>(dungeon)->get_index();
        const bool is_tournament_dungeon =
            dungeon_index == 120 || dungeon_index == 121;
        if (is_tournament_dungeon &&
            (get_member_count() > 1 ||
             mode == static_cast<ENUM_DUNGEON_MODE>(1))) {
            for (int seat = 0; seat <= 3; ++seat) {
                if (_checkValidUser(seat) == 1) {
                    CUser* const user = get_user(seat);
                    if (user) {
                        LogManager::logFormat(
                            1, "party.cpp",
                            "ENUM_ERROR CParty::checkInoutConditionDungeon(const CDungeon*, ENUM_DUNGEON_MODE, ENUM_CMDPACKET)",
                            0x25EB,
                            "TOURNAMENT_DUNGEON_ERROR_LOG : CHARAC_NO (%d) MODE (%d)",
                            user->getCurCharacNo(), static_cast<int>(mode));
                    }
                }
            }
            packet.clear();
            packet.put_header(1, command);
            packet.put_byte(0);
            packet.put_byte(243);
            packet.finalize(true);
            send_to_party(packet);
            return 1;
        }
        return 0;
    }

    const bool include_failed_member =
        start_result == 10 || start_result == 95 || start_result == 243 ||
        start_result == 244 || start_result == 245 || start_result == 248;
    packet.clear();
    packet.put_header(1, command);
    packet.put_byte(0);
    packet.put_byte(start_result);
    if (include_failed_member) {
        packet.put_byte(start_failed_member);
    }
    packet.finalize(true);
    send_to_party(packet);
    return start_result;
}
