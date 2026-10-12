#include "old_game/player/core/cuser.h"

#include "old_data/db/game_option/db_save_game_option_chatting_emoticon.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


// IDA_EA: 0x868BF98
// IDA_NAME: _ZN5CUser24send_charac_visible_dataEv
// DEMANGLED: CUser::send_charac_visible_data(void)
void CUser::send_charac_visible_data() {
    // 0828 client reader for notification 446 consumes one byte and refreshes
    // the current actor's job/awakening appearance.  Keep this serialization
    // in the existing CUser notification owner; no separate compatibility
    // helper is introduced for the newer reader.
    PacketGuard grow_avatar_packet;
    grow_avatar_packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_CHARAC_QUEST_PIECE);
    grow_avatar_packet.put_byte(static_cast<unsigned char>(
        get_charac_visible_grow_avatar() != 0));
    grow_avatar_packet.finalize(true);
    Send(grow_avatar_packet);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 356);
    packet.put_short(get_unique_id());
    packet.put_byte(get_charac_visible_values());
    packet.finalize(true);

    GameWorld* const world = G_GameWorld();
    if (world->IsPVPChannel()) {
        world->send_all(packet);
        return;
    }

    Village* const village = world->getUserVillage(this);
    if (!village) {
        return;
    }

    if (get_area(false) == village->get_gate_area()) {
        Send(packet);
        return;
    }

    const int state = get_state();
    if (state != 5) {
        if (state <= 5) {
            if (state == 3) {
                world->send_to_area(
                    static_cast<signed char>(getCurCharacVill()),
                    get_area(false),
                    packet);
                return;
            }
            world->send_all(packet);
            return;
        }
        if (static_cast<unsigned int>(state - 8) > 2u) {
            world->send_all(packet);
            return;
        }
    }

    CParty* const party = GetParty();
    if (party) {
        party->send_to_party(packet);
    }
}

// IDA_EA: 0x8689A22
// IDA_NAME: _ZN5CUser22ReCalcChattingEmoticonEv
// DEMANGLED: CUser::ReCalcChattingEmoticon(void)
void CUser::ReCalcChattingEmoticon() {
    CGameOption* const game_option = GetGameOptionRef();
    CChattingEmoticonConfig config = game_option->getEmoticonInfo();
    G_CDataManager()->ReCalcEmoticonConf(config, *this);
    if (game_option->SetEmoticonInfo(config) != 1) {
        return;
    }

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("user.cpp", 41078),
        true);
    guard << 727;
    guard << GetUID();
    SIG_SAVE_CHATTING_EMOTICON* const saved =
        guard.GetInBuffer<SIG_SAVE_CHATTING_EMOTICON>();
    saved->m_id = get_acc_id();
    std::memcpy(saved->shortcut_emoticon,
                &config,
                sizeof(saved->shortcut_emoticon));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8689B90
// IDA_NAME: _ZN5CUser20SendChattingEmoticonEv
// DEMANGLED: CUser::SendChattingEmoticon(void)
void CUser::SendChattingEmoticon() {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_EMOTICON_CONF_INFO);
    packet.put_short(24);

    const CChattingEmoticonConfig config =
        GetGameOptionRef()->getEmoticonInfo();
    for (int index = 0; index <= 23; ++index) {
        const CChattingEmoticonConfigRow& row = config.rows[index];
        packet.put_short(index);
        packet.put_short(row.icon_id);
        const int text_length = static_cast<int>(std::strlen(row.text));
        packet.put_int(text_length);
        packet.put_str(row.text, text_length);
    }
    packet.finalize(true);
    Send(packet);
}
