#include "old_game/economy/mail/inter_multimailbox_send_return.h"
#include "infra/network/packet/PacketEnum.h"

#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

// IDA_EA: 0x84EE880
Inter_MultiMailBoxReqSend::Inter_MultiMailBoxReqSend() {}

// IDA_EA: 0x84F6B10 / 0x84F6B40
Inter_MultiMailBoxReqSend::~Inter_MultiMailBoxReqSend() {}

// IDA_EA: 0x84E27B8
// IDA_NAME: _ZN25Inter_MultiMailBoxReqSend12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MultiMailBoxReqSend::dispatch_sig(CUser *,char *,int)
int Inter_MultiMailBoxReqSend::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_MAILBOX_CHECK_RECEIVER)) ||
        user->get_state() <= 2) {
        return 0;
    }

    SIG_MAILBOX_CHECK_RECEIVER& reply =
        *reinterpret_cast<SIG_MAILBOX_CHECK_RECEIVER*>(payload);
    MSG_MULTI_MAILBOX_SEND& multi = reply.message.payload.multi_mailbox_send;
    if (multi.item_count > 10u) {
        return 0;
    }

    for (uint8_t index = 0; index < multi.item_count; ++index) {
        MSG_MAILBOX_SEND request;
        std::memset(&request, 0, sizeof(request));

        const uint32_t name_length =
            multi.receiver_name_length < sizeof(request.receiver_name)
                ? multi.receiver_name_length
                : sizeof(request.receiver_name);
        std::strncpy(request.receiver_name, multi.receiver_name, name_length);
        request.money = multi.money;
        // The original handler transfers the gold with the first package only.
        multi.money = 0;
        request.inventory_type = multi.items[index].inventory_type;
        request.inventory_slot = multi.items[index].inventory_slot;
        request.item_id = multi.items[index].item_id;
        request.item_add_info = multi.items[index].item_add_info;

        const int result = WongWork::CMailBoxHelper::ReqDBSendNewMail(
            user,
            &request,
            static_cast<unsigned int>(reply.receiver_charac_no),
            false,
            reply.receiver_account_id,
            reply.multi_mail_text_length);
        if (result) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND,
                                     static_cast<unsigned char>(result));
            return 0;
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MULTI_MAILBOX_SEND);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}
