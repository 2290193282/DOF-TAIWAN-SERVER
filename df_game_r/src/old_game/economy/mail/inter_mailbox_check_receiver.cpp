#include "old_game/economy/mail/inter_mailbox_check_receiver.h"

#include <cstring>

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/log/format/log_manager.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const ENUM_CMDPACKET kMailboxSendCommand = ENUM_CMDPACKET_MAILBOX_SEND;
const ENUM_CMDPACKET kCeraShopBuyCommand =
    ENUM_CMDPACKET_BUY_CERASHOP_ITEM;
const ENUM_CMDPACKET kMultiMailboxSendCommand =
    ENUM_CMDPACKET_MULTI_MAILBOX_SEND;
const int kDBSignalMultiMailboxSend = 501;  // 0x1f5
const int kCeraShopBuyItemFailureReturn = 11538;

ENUM_CMDPACKET GetCommandForCheckType(uint32_t check_type) {
    if (check_type == MSG_CHECK_MAIL_RECEIVER::kBuyCeraShopItem) {
        return kCeraShopBuyCommand;
    }
    if (check_type == MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend) {
        return kMultiMailboxSendCommand;
    }
    return kMailboxSendCommand;
}

bool IsRentalMailItem(unsigned int item_id) {
    return GlobalData::s_rentalSystem->CheckRentableItem(1, item_id) != 0 ||
           GlobalData::s_rentalSystem->CheckRentableItem(2, item_id) != 0;
}

}  // namespace

// IDA_EA: 0x84ED620
Inter_MailBox_Req_CheckReceiver::Inter_MailBox_Req_CheckReceiver() {}

// IDA_EA: 0x84F9CBA / 0x84F9CEA
Inter_MailBox_Req_CheckReceiver::~Inter_MailBox_Req_CheckReceiver() {}

// IDA_EA: 0x84CF95A
// IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MailBox_Req_CheckReceiver::dispatch_sig(CUser *,char *,int)
int Inter_MailBox_Req_CheckReceiver::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_MAILBOX_CHECK_RECEIVER)) ||
        user->get_state() <= 2) {
        return 0;
    }

    SIG_MAILBOX_CHECK_RECEIVER& reply =
        *reinterpret_cast<SIG_MAILBOX_CHECK_RECEIVER*>(payload);
    const uint32_t check_type = reply.message.check_type;
    const ENUM_CMDPACKET command = GetCommandForCheckType(check_type);

    if (!reply.receiver_charac_no) {
        SendErrorPacket(user, command, 3);
        return 0;
    }
    if (reply.receiver_delete_flag) {
        SendErrorPacket(user, command, 24);
        return 0;
    }
    if (GlobalData::s_pGMAccounts_->isGM(reply.receiver_account_id)) {
        SendErrorPacket(user, command, 90);
        return 0;
    }
    if (reply.black_list_result == 77) {
        SendErrorPacket(user, command, 77);
        return 0;
    }

    if (check_type == MSG_CHECK_MAIL_RECEIVER::kBuyCeraShopItem) {
        MSG_BUY_CERASHOP_ITEM& purchase =
            reply.message.payload.buy_cera_shop_item;

        // The original stores the checked character number at request +0x32
        // and overwrites the final four bytes of receiver_name (+0x2e) with
        // the checked account id before entering ItemVendingMachine.
        purchase.post_recipient_wire_word =
            static_cast<uint32_t>(reply.receiver_charac_no);
        std::memcpy(purchase.receiver_name + 26,
                    &reply.receiver_account_id,
                    sizeof(reply.receiver_account_id));
        if (GlobalData::s_pItemVendingMachine->BuyItem(user, purchase) != 0) {
            return 0;
        }
        return kCeraShopBuyItemFailureReturn;
    }

    if (check_type == MSG_CHECK_MAIL_RECEIVER::kMailboxSend) {
        MSG_MAILBOX_SEND& message = reply.message.payload.mailbox_send;
        if (message.item_id) {
            CItem* const item = G_CDataManager()->find_item(message.item_id);
            if (item) {
                if (IsRentalMailItem(message.item_id)) {
                    return 17;
                }
                if (item->GetAttachType() == 5 &&
                    static_cast<uint32_t>(user->get_acc_id()) !=
                        reply.receiver_account_id) {
                    user->SendCmdErrorPacket(
                        kMailboxSendCommand,
                        216);
                    return 0;
                }
            }
        }
        if (message.money &&
            static_cast<uint32_t>(G_CDataManager()->GetMoneyLimitPerLevel(
                reply.receiver_level, message.receiver_name)) < message.money) {
            SendErrorPacket(user, command, 14);
            return 0;
        }

        const int result = WongWork::CMailBoxHelper::ReqDBSendNewMail(
            user,
            &message,
            static_cast<unsigned int>(reply.receiver_charac_no),
            false,
            reply.receiver_account_id,
            0);
        if (result) {
            SendErrorPacket(user, command, static_cast<uint16_t>(result));
            return 0;
        }

        PacketGuard packet;
        packet.put_header(1, kMailboxSendCommand);
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (check_type != MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend) {
        return 0;
    }

    MSG_MULTI_MAILBOX_SEND& multi =
        reply.message.payload.multi_mailbox_send;
    // The original parser establishes this before DB submission.  Retain the
    // wire maximum here so a malformed DB reply cannot walk past payload.
    if (multi.item_count > 10u) {
        return 0;
    }
    for (uint8_t index = 0; index < multi.item_count; ++index) {
        const unsigned int item_id = multi.items[index].item_id;
        if (!item_id) {
            continue;
        }
        CItem* const item = G_CDataManager()->find_item(item_id);
        if (!item) {
            continue;
        }
        if (IsRentalMailItem(item_id)) {
            return 17;
        }
        if (item->GetAttachType() == 5 &&
            static_cast<uint32_t>(user->get_acc_id()) !=
                reply.receiver_account_id) {
            user->SendCmdErrorPacket(
                kMultiMailboxSendCommand, 216);
            return 0;
        }
    }
    if (multi.money &&
        static_cast<uint32_t>(G_CDataManager()->GetMoneyLimitPerLevel(
            reply.receiver_level, multi.receiver_name)) < multi.money) {
        user->SendCmdErrorPacket(
            kMultiMailboxSendCommand, 14);
        return 0;
    }

    SendDBThread_forchina(user, &reply);
    return 0;
}

// IDA_EA: 0x84CFF18
// IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver21SendDBThread_forchinaEP5CUserP26SIG_MAILBOX_CHECK_RECEIVER
// DEMANGLED: Inter_MailBox_Req_CheckReceiver::SendDBThread_forchina(CUser *,SIG_MAILBOX_CHECK_RECEIVER *)
int Inter_MailBox_Req_CheckReceiver::SendDBThread_forchina(
    CUser* user,
    SIG_MAILBOX_CHECK_RECEIVER* request) {
    CStreamGuard guard(
        GlobalData::big_stream_pool()->Acquire("InterDispatcher.cpp", 11770),
        true);
    // IDA 0x84CFF5F: the complete immediate is 0x1f5, not decompiler's 245.
    guard << kDBSignalMultiMailboxSend;
    // IDA 0x84CFFCC calls the int overload; DBTask reads the 501 UID as int.
    guard << user->GetUID();

    SIG_MAILBOX_CHECK_RECEIVER* const destination =
        guard.GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>();
    std::memcpy(destination, request, sizeof(*destination));
    std::strncpy(destination->sender_charac_name,
                 user->getCurCharacName(),
                 sizeof(destination->sender_charac_name));
    destination->sender_charac_no = user->getCurCharacNo();

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x84D008A
// IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver15SendErrorPacketEP5CUser14ENUM_CMDPACKETt
// DEMANGLED: Inter_MailBox_Req_CheckReceiver::SendErrorPacket(CUser *,ENUM_CMDPACKET,unsigned short)
int Inter_MailBox_Req_CheckReceiver::SendErrorPacket(CUser* user,
                                                      ENUM_CMDPACKET command,
                                                      uint16_t error) {
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "void Inter_MailBox_Req_CheckReceiver::SendErrorPacket(CUser*, "
        "ENUM_CMDPACKET, short unsigned int)",
        11794,
        "[ITEM_GIFT] Fail Command[%d] charac_no[%u] error_code[%d]",
        static_cast<int>(command),
        static_cast<unsigned int>(user->get_charac_no(-1)),
        static_cast<unsigned int>(error));
    if (command == kCeraShopBuyCommand) {
        return user->SendCmdErrorPacket2Byte(command, error) ? 1 : 0;
    }
    return user->SendCmdErrorPacket(command, static_cast<unsigned char>(error))
               ? 1
               : 0;
}
