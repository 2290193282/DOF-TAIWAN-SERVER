#include "old_game/economy/mail/inter_mailbox_req_list.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84CF7FA
// IDA_NAME: _ZN22Inter_MailBox_Req_List12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MailBox_Req_List::dispatch_sig(CUser *,char *,int)
int Inter_MailBox_Req_List::dispatch_sig(CUser* user,
                                         char* payload,
                                         int /*payload_size*/) {
    if (user->get_state() <= 1 || !user->getCurCharacR()) {
        return 0;
    }

    // dispatch_sig's erased char* comes from IPacketDispatcher.  Signal 127
    // is nevertheless the complete typed DB reply written at 0x841DE4A.
    const SIG_MAILBOX_LIST* const reply =
        reinterpret_cast<const SIG_MAILBOX_LIST*>(payload);
    const int reply_charac_no = reply->response.charac_no;
    if (reply_charac_no != user->getCurCharacNo()) {
        return 11424;
    }

    WongWork::CMailBox* const mailbox = user->GetMailBox();
    if (!mailbox) {
        return 11428;
    }

    const int reply_load_count = reply->response.mail_load_count;
    if (reply_load_count != mailbox->getMailLoadCount()) {
        cMyTrace trace(
            "virtual int Inter_MailBox_Req_List::dispatch_sig(CUser*, char*, int)",
            11433,
            5);
        trace("Postal loaded twice. %s(%d)(%d->%d)",
              NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
              user->getCurCharacNo(),
              mailbox->getMailLoadCount(),
              reply_load_count);
        return 11434;
    }

    WongWork::CMailBoxHelper::SetMailBoxInfo(user, reply);
    return 0;
}

// IDA_EA: 0x84ED604 / 0x84F9D08 / 0x84F9D38
// IDA_NAME: _ZN22Inter_MailBox_Req_ListC2Ev /
//           _ZN22Inter_MailBox_Req_ListD2Ev /
//           _ZN22Inter_MailBox_Req_ListD0Ev
Inter_MailBox_Req_List::Inter_MailBox_Req_List() : ISigDispatcher() {}
Inter_MailBox_Req_List::~Inter_MailBox_Req_List() {}
