#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_PARAM_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_PARAM_RUNTIME_H_

#include "infra/network/packet/message_base.h"

// IDA units:
// 10_mail_trade_auction_cash/stMailboxSendParam,
// 10_mail_trade_auction_cash/stMailboxOpenParam, and
// 10_mail_trade_auction_cash/stOpenCeraPackage.
//
// These dispatcher parameters are complete, polymorphic value types.  Their
// derived fields are written by the old-server constructors; the virtual
// destructor slots are emitted by the old i386 C++ ABI as D0/D1/D2 variants.
struct stMailboxSendParam : public ParamBase {
    stMailboxSendParam();
    virtual ~stMailboxSendParam();

    int result;
};

struct stMailboxOpenParam : public ParamBase {
    stMailboxOpenParam();
    virtual ~stMailboxOpenParam();

    int result;
    int not_loaded_mail_count;
};

struct stMailboxExtractParam : public ParamBase {
    stMailboxExtractParam();
    virtual ~stMailboxExtractParam();

    int result;
    // The old constructor stores zero at +0x08.  No old-server consumer in
    // this closure establishes a stronger semantic name for this integer.
    int unobserved_08;
};

struct stOpenCeraPackage : public ParamBase {
    stOpenCeraPackage();
    virtual ~stOpenCeraPackage();

    int result;
};

typedef char stMailboxSendParam_Runtime_Size_Must_Be_0x08[
    (sizeof(stMailboxSendParam) == 0x08) ? 1 : -1];
typedef char stMailboxOpenParam_Runtime_Size_Must_Be_0x0c[
    (sizeof(stMailboxOpenParam) == 0x0c) ? 1 : -1];
typedef char stMailboxExtractParam_Runtime_Size_Must_Be_0x0c[
    (sizeof(stMailboxExtractParam) == 0x0c) ? 1 : -1];
typedef char stOpenCeraPackage_Runtime_Size_Must_Be_0x08[
    (sizeof(stOpenCeraPackage) == 0x08) ? 1 : -1];

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAILBOX_PARAM_RUNTIME_H_
