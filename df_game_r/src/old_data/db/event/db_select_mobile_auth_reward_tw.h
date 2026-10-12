#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_MOBILE_AUTH_REWARD_TW_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_MOBILE_AUTH_REWARD_TW_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class CUser;
class Stream;

// Taiwan internal DB route 12.  Its four-byte payload is the Garena account
// identifier used as mobile_auth_reward_tw.m_id.
class DBSelectMobileAuthRewardTw : public TaiwanInterfaceDBDispatcher {
public:
    struct MSG {
        unsigned int account_id;
    };

    // IDA_EA: 0x815E3FA
    // IDA_NAME: _ZN26DBSelectMobileAuthRewardTwC2Ev
    DBSelectMobileAuthRewardTw();

    // IDA_EA: 0x815E41E (D2), 0x815E44E (D0)
    // IDA_NAME: _ZN26DBSelectMobileAuthRewardTwD2Ev / D0Ev
    virtual ~DBSelectMobileAuthRewardTw();

    // IDA_EA: 0x815C586
    // IDA_NAME: _ZN26DBSelectMobileAuthRewardTw11makeRequestEP5CUser
    static void makeRequest(CUser* user);

    // IDA_EA: 0x815C6A8
    // IDA_NAME: _ZN26DBSelectMobileAuthRewardTw8dispatchEiiP6Stream
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DBSelectMobileAuthRewardTw::MSG) == 0x04u,
              "DBSelectMobileAuthRewardTw::MSG must retain its wire ABI");
static_assert(sizeof(DBSelectMobileAuthRewardTw) == 0x08u,
              "DBSelectMobileAuthRewardTw must retain the Taiwan DB ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SELECT_MOBILE_AUTH_REWARD_TW_H_
