#include "old_game/economy/mail/inter_monitor_notify_new_mail.h"

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x84D017C
// IDA_NAME: _ZN29Inter_Monitor_Notify_New_Mail12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Monitor_Notify_New_Mail::dispatch_sig(CUser *,char *,int)
int Inter_Monitor_Notify_New_Mail::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    if (!user || !payload ||
        payload_size !=
            static_cast<int>(sizeof(Packet_Monitor_Notify_New_Mail))) {
        return 0;
    }
    const Packet_Monitor_Notify_New_Mail& notification =
        *reinterpret_cast<const Packet_Monitor_Notify_New_Mail*>(payload);
    if (user->getCurCharacR() &&
        user->getCurCharacNo() ==
            static_cast<int>(notification.receiver_charac_no)) {
        WongWork::CMailBox* const mailbox = user->GetMailBox();
        if (!mailbox) {
            return 0;
        }
        mailbox->IncNotLoadedMailCount();

        PacketGuard packet;
        packet.put_header(0, 99);
        packet.put_short(mailbox->GetNotLoadedMailCount());
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84ED658 / 0x84F9C1E / 0x84F9C4E
// IDA_NAME: _ZN29Inter_Monitor_Notify_New_MailC2Ev /
//           _ZN29Inter_Monitor_Notify_New_MailD2Ev /
//           _ZN29Inter_Monitor_Notify_New_MailD0Ev
Inter_Monitor_Notify_New_Mail::Inter_Monitor_Notify_New_Mail()
    : ISigDispatcher() {}
Inter_Monitor_Notify_New_Mail::~Inter_Monitor_Notify_New_Mail() {}
