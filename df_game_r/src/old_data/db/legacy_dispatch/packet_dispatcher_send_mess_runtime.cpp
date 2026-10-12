#include "old_game/social/party/packet_dispatcher_group_business_043.h"

#include <cstring>
#include <string>

#include "common/manager/community_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/slang_filter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

void BuildOtherChannelChat(Packet_Monitor_Other_Channel_Chat& packet,
                           unsigned char chat_type,
                           uint32_t account_id,
                           unsigned char sender_marker,
                           uint32_t charac_no,
                           uint32_t item_position,
                           const char* target_name,
                           int target_name_length,
                           const char* message,
                           int message_length) {
    packet.chat_type = chat_type;
    packet.account_id = account_id;
    packet.sender_marker = sender_marker;
    packet.charac_no = charac_no;
    packet.item_position = item_position;
    packet.target_name_length = static_cast<unsigned char>(target_name_length);
    std::memcpy(packet.target_name, target_name, target_name_length);
    packet.message_length = static_cast<unsigned char>(message_length);
    std::memcpy(packet.message, message, message_length);
}

int OtherChannelChatWireSize(int message_length) {
    return static_cast<int>(static_cast<unsigned char>(message_length)) +
           static_cast<int>(offsetof(Packet_Monitor_Other_Channel_Chat,
                                     message));
}

bool IsChatUseItemId(int item_id) {
    return item_id == 36 || item_id == 898 || item_id == 2675024 ||
           item_id == 690000093 || item_id == 690000184;
}

}  // namespace

// IDA_EA: 0x81F3540
// IDA_NAME: _ZN19DisPatcher_SendMess12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SendMess::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SendMess::dispatch_sig(CUser* user, PacketBuf& packet) {
    static const char kSignature[] =
        "virtual int DisPatcher_SendMess::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2) {
        return 0;
    }

    unsigned char chat_type = 0;
    uint16_t item_space = 0;
    uint32_t item_position = 0;
    int message_length = 0;
    char message[256];
    if (packet.get_byte(chat_type) != 1) {
        return LineFunc(32501, kSignature, 0, 0);
    }
    if (packet.get_short(item_space) != 1) {
        return LineFunc(32502, kSignature, 0, 0);
    }
    if (packet.get_int(item_position) != 1) {
        return LineFunc(32503, kSignature, 0, 0);
    }
    if (packet.get_int(message_length) != 1) {
        return LineFunc(32504, kSignature, 0, 0);
    }
    if (packet.get_str(message, sizeof(message), message_length) != 1) {
        return LineFunc(32505, kSignature, 0, 0);
    }

    G_CDataManager()->rewriteUsableEmoticon(*user, message);
    if (user->IsCurCharacGhost() &&
        (chat_type > 8 || ((1u << chat_type) & 0x144u) == 0)) {
        return 0;
    }

    int target_name_length = 0;
    char target_name[30];
    if (chat_type == 36 || chat_type == 37 || chat_type == 38) {
        unsigned char sender_marker = 0;
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(32587, kSignature, 0, 0);
        }
        if (packet.get_str(target_name, sizeof(target_name),
                           target_name_length) != 1) {
            return LineFunc(32589, kSignature, 0, 0);
        }
        if (packet.get_byte(sender_marker) != 1) {
            return LineFunc(32591, kSignature, 0, 0);
        }

        Packet_Monitor_Other_Channel_Chat out_packet;
        const unsigned char monitor_chat_type =
            chat_type == 36 ? 7 : (chat_type == 37 ? 33 : 1);
        BuildOtherChannelChat(out_packet,
                              monitor_chat_type,
                              static_cast<uint32_t>(user->get_acc_id()),
                              sender_marker,
                              static_cast<uint32_t>(user->getCurCharacNo()),
                              item_position,
                              target_name,
                              target_name_length,
                              message,
                              message_length);
        GlobalData::s_community_proxy->SendTcpPacket(
            reinterpret_cast<char*>(&out_packet),
            OtherChannelChatWireSize(message_length));
        return 0;
    }

    if (chat_type == 7 || chat_type == 1 || chat_type == 33) {
        unsigned char ignored_sender_marker = 0;
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(32625, kSignature, 0, 0);
        }
        if (packet.get_str(target_name, sizeof(target_name),
                           target_name_length) != 1) {
            return LineFunc(32627, kSignature, 0, 0);
        }
        if (packet.get_byte(ignored_sender_marker) != 1) {
            return LineFunc(32632, kSignature, 0, 0);
        }

        if (chat_type == 7) {
            Packet_Monitor_Other_Channel_Chat out_packet;
            out_packet.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            out_packet.target_name_length =
                static_cast<unsigned char>(target_name_length);
            std::memcpy(out_packet.target_name,
                        target_name,
                        target_name_length);
            out_packet.message_length =
                static_cast<unsigned char>(message_length);
            std::memcpy(out_packet.message, message, message_length);

            const std::string filtered =
                CSyncSlangFilter::GetInstance()->Filter(std::string(message));
            if (!filtered.empty()) {
                std::strncpy(out_packet.message,
                             filtered.c_str(),
                             filtered.size());
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
                    ->SendPacket(reinterpret_cast<char*>(&out_packet),
                                 OtherChannelChatWireSize(message_length));
            }
            return 0;
        }
    } else {
        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(item_space));
        const Inven_Item* const inventory_item =
            user->getCurCharacInvenR()->GetInvenRef(
                inventory_type, static_cast<int>(item_position));

        switch (chat_type) {
        case 11:
        case 12:
        case 35:
            if (inventory_item &&
                IsChatUseItemId(inventory_item->getKey())) {
                const int security_error =
                    GlobalData::s_pSecuProtectionField->Check(
                        user, static_cast<SECURITY_PROTCTION>(36));
                if (security_error) {
                    user->SendCmdErrorPacket(
                        ENUM_CMDPACKET_SEND_MESSAGE,
                        static_cast<unsigned char>(security_error));
                    return 0;
                }
                if (user->CheckItemLock(inventory_type,
                                        static_cast<int>(item_position))) {
                    user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 213);
                    return 0;
                }
                if (user->use_stackable(static_cast<short>(item_position),
                                        static_cast<int>(item_space),
                                        -1,
                                        -1) == 1) {
                    goto send_chat;
                }
            }
            break;

        case 13:
        case 15:
            if (!inventory_item) {
                return LineFunc(32800, kSignature, 0, 0);
            }
            if ((inventory_item->getKey() != 37 ||
                 chat_type != 13) &&
                (inventory_item->getKey() != 2660655 ||
                 chat_type != 15)) {
                return LineFunc(32805, kSignature, 0, 0);
            }
            {
                const int security_error =
                    GlobalData::s_pSecuProtectionField->Check(
                        user, static_cast<SECURITY_PROTCTION>(36));
                if (security_error) {
                    user->SendCmdErrorPacket(
                        ENUM_CMDPACKET_SEND_MESSAGE,
                        static_cast<unsigned char>(security_error));
                    return 0;
                }
            }
            if (user->CheckItemLock(inventory_type,
                                    static_cast<int>(item_position))) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 213);
                return 0;
            }
            if (user->use_stackable(static_cast<short>(item_position),
                                    static_cast<int>(item_space),
                                    -1,
                                    -1) == 1) {
                goto send_chat;
            }
            break;

        case 14:
            if (!inventory_item || inventory_item->getKey() != 1120) {
                return LineFunc(32851, kSignature, 0, 0);
            }
            if (!GlobalData::s_pGMAccounts_->isGM(
                    static_cast<uint32_t>(user->get_acc_id()))) {
                return LineFunc(32855, kSignature, 0, 0);
            }
            if (user->use_stackable(static_cast<short>(item_position),
                                    static_cast<int>(item_space),
                                    -1,
                                    -1) == 1) {
                goto send_chat;
            }
            break;

        case 34:
            if (user->get_state() != 3) {
                return 0;
            }
            if (!inventory_item || inventory_item->getKey() != 7582) {
                return LineFunc(32874, kSignature, 0, 0);
            }
            {
                CItem* const item = G_CDataManager()->find_item(
                    inventory_item->getKey());
                if (item && item->is_stackable() == 1 &&
                    user->getCurCharacInvenW()->delete_item(
                        static_cast<INVEN_TYPE>(inventory_type),
                        static_cast<int>(item_position),
                        1,
                        static_cast<eItemDelReason>(3),
                        true) == 1) {
                    goto send_chat;
                }
            }
            break;

        default:
            goto send_chat;
        }

        user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 17);
        return 0;
    }

send_chat:
    if (GlobalData::s_pGMAccounts_->isGM(
            static_cast<uint32_t>(user->get_acc_id()))) {
        DB_InsertPoliceSaveChatting::makeRequest(
            static_cast<uint32_t>(user->getCurCharacNo()),
            static_cast<char>(chat_type),
            message);
    }
    G_GameWorld()->send_chat_msg(user,
                                 static_cast<char>(chat_type),
                                 item_space,
                                 item_position,
                                 message,
                                 message_length,
                                 target_name,
                                 target_name_length,
                                 static_cast<ENUM_NOTIPACKET>(12));
    return 0;
}
