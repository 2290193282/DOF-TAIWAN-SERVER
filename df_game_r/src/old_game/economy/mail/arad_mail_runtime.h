#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_ARAD_MAIL_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_ARAD_MAIL_RUNTIME_H_

#include "old_game/economy/mail/mail_trade_value_types.h"

namespace ARAD {

// IDA_EA: 0x8196B03
// IDA_NAME: _ZN4ARAD9send_mailERN10ARAD_DATAS12AradMailInfoE
// DEMANGLED: ARAD::send_mail(ARAD_DATAS::AradMailInfo &)
int send_mail(ARAD_DATAS::AradMailInfo& info);

}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_ARAD_MAIL_RUNTIME_H_
