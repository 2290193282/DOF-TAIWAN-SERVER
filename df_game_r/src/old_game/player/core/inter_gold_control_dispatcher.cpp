#include "old_game/player/core/inter_gold_control_dispatcher.h"

#include "infra/security/policy/secu_gold_control_abi.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84E4FE8
// IDA_NAME: _ZN34Inter_GoldControlMoneyActionCancel12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GoldControlMoneyActionCancel::dispatch_sig(CUser *,char *,int)
int Inter_GoldControlMoneyActionCancel::dispatch_sig(CUser* user,
                                                     char* payload,
                                                     int payload_size) {
    if (!user || !payload || payload_size < 2) {
        return 0;
    }

    Secu_GoldControl* const gold_control =
        static_cast<Secu_GoldControl*>(user->GetGoldControl());
    if (payload[0]) {
        gold_control->MailSameMid();
    } else {
        gold_control->MailComplete(payload[1] != 0);
    }
    return 0;
}

// IDA_EA: 0x84EEC00 / 0x84F6066 / 0x84F6096
Inter_GoldControlMoneyActionCancel::Inter_GoldControlMoneyActionCancel()
    : ISigDispatcher() {}
Inter_GoldControlMoneyActionCancel::~Inter_GoldControlMoneyActionCancel() {}
