#include "old_data/db/taiwan/inter_slots_331_353.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstring>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x84D9CBE
// IDA_NAME: _ZN30Inter_ResposncePCRoomPlyerList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ResposncePCRoomPlyerList::dispatch_sig(CUser *,char *,int)
int Inter_ResposncePCRoomPlyerList::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        PacketGuard packet;
        if (user->GetPCRoomNo()) {
            const SIG_PCROOM_PLAYER_LIST_RESPONSE* const reply =
                reinterpret_cast<const SIG_PCROOM_PLAYER_LIST_RESPONSE*>(
                    payload);
            const int character_id = user->get_charac_no(-1);
            if (character_id != static_cast<int>(reply->prefix.character_id)) {
                LogManager::logFormat(
                    1, "InterDispatcher.cpp",
                    "virtual int Inter_ResposncePCRoomPlyerList::dispatch_sig(CUser*, char*, int)",
                    0x4192,
                    "Inter_ResposncePCRoomPlyerList::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                    user->get_charac_no(-1),
                    static_cast<int>(reply->prefix.character_id));
                return 0;
            }
            if (reply->count <= 0x28u) {
                packet.put_header(1, ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_LIST);
                packet.put_byte(1);
                packet.put_byte(reply->count);
                for (int index = 0; index < reply->count; ++index) {
                    const SIG_PCROOM_PLAYER_LIST_ENTRY& entry =
                        reply->entries[index];
                    const size_t name_length = std::strlen(entry.character_name);
                    packet.put_int(static_cast<int>(name_length));
                    packet.put_str(entry.character_name, name_length);
                    packet.put_short(static_cast<short>(entry.field_02));
                    packet.put_byte(entry.field_00);
                    packet.put_byte(entry.field_01);
                    packet.put_byte(entry.field_22);
                    packet.put_byte(entry.field_23);
                    packet.put_byte(entry.field_24);
                }
                packet.finalize(true);
                user->Send(packet);
                return 0;
            }
        }

        packet.put_header(1, ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_LIST);
        packet.put_byte(0);
        packet.put_byte(3);
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}


// IDA_EA: 0x84EE030
// IDA_NAME: _ZN30Inter_ResposncePCRoomPlyerListC2Ev
// DEMANGLED: Inter_ResposncePCRoomPlyerList::Inter_ResposncePCRoomPlyerList(void)
Inter_ResposncePCRoomPlyerList::Inter_ResposncePCRoomPlyerList() {}

// IDA_EA: 0x84F819C (D2), 0x84F81CC (D0)
Inter_ResposncePCRoomPlyerList::~Inter_ResposncePCRoomPlyerList() {}


// IDA_EA: 0x84DA0AA
// IDA_NAME: _ZN31Inter_ResposncePCRoomPlyerCount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ResposncePCRoomPlyerCount::dispatch_sig(CUser *,char *,int)
int Inter_ResposncePCRoomPlyerCount::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    (void)payload_size;

    if (user->get_state()) {
        const SIG_PCROOM_PLAYER_COUNT_RESPONSE* const reply =
            reinterpret_cast<const SIG_PCROOM_PLAYER_COUNT_RESPONSE*>(payload);
        if (user->get_charac_no(-1) ==
            static_cast<int>(reply->prefix.character_id)) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_COUNT);
            packet.put_byte(1);
            packet.put_byte(reply->player_count);
            packet.finalize(true);
            user->Send(packet);
        } else {
            LogManager::logFormat(
                1, "InterDispatcher.cpp",
                "virtual int Inter_ResposncePCRoomPlyerCount::dispatch_sig(CUser*, char*, int)",
                0x41cc,
                "Inter_ResponsePCRoomPlayerCount::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharID(%d)",
                user->get_charac_no(-1),
                static_cast<int>(reply->prefix.character_id));
        }
    }
    return 0;
}


// IDA_EA: 0x84EE04C
// IDA_NAME: _ZN31Inter_ResposncePCRoomPlyerCountC2Ev
// DEMANGLED: Inter_ResposncePCRoomPlyerCount::Inter_ResposncePCRoomPlyerCount(void)
Inter_ResposncePCRoomPlyerCount::Inter_ResposncePCRoomPlyerCount() {}

// IDA_EA: 0x84F814E (D2), 0x84F817E (D0)
Inter_ResposncePCRoomPlyerCount::~Inter_ResposncePCRoomPlyerCount() {}
