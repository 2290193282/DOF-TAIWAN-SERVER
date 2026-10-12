#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_INSERT_GIVE_BOX_EVENT_REWARD_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_INSERT_GIVE_BOX_EVENT_REWARD_H_

#include <stdint.h>

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"


class CUser;
class Stream;


// Taiwan DB internal code 16.  The old concrete object has no state beyond
// TaiwanInterfaceDBDispatcher's [vptr, code] pair.
class DBInsertGiveBoxEventReward : public TaiwanInterfaceDBDispatcher {
public:
    // IDA: CStreamGuard::GetInBuffer<MSG> @0x815ED08,
    // Stream::GetOutBuffer<MSG> @0x815ED1E, and
    // Stream::GetInBuffer<MSG> @0x815F552 all use this exact 8-byte signal.
    struct MSG {
        uint32_t account_id;  // +0x00: event_1306_account_reward_2nd.m_id
        uint32_t charac_no;   // +0x04: selected character number
    };

    // IDA_EA: 0x815E550
    // IDA_NAME: _ZN26DBInsertGiveBoxEventRewardC2Ev
    // DEMANGLED: DBInsertGiveBoxEventReward::DBInsertGiveBoxEventReward(void)
    DBInsertGiveBoxEventReward();

    // IDA_EA: 0x815E574 (D2), 0x815E5A4 (D0)
    // IDA_NAME: _ZN26DBInsertGiveBoxEventRewardD2Ev / D0Ev
    virtual ~DBInsertGiveBoxEventReward();

    // IDA_EA: 0x815D2C8
    // IDA_NAME: _ZN26DBInsertGiveBoxEventReward11makeRequestEP5CUser
    // DEMANGLED: DBInsertGiveBoxEventReward::makeRequest(CUser *)
    static void makeRequest(CUser* user);

    // IDA_EA: 0x815D3EE
    // IDA_NAME: _ZN26DBInsertGiveBoxEventReward8dispatchEiiP6Stream
    // DEMANGLED: DBInsertGiveBoxEventReward::dispatch(int,int,Stream *)
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};


static_assert(sizeof(DBInsertGiveBoxEventReward::MSG) == 0x08u,
              "GiveMeBox Taiwan DB payload must remain 8 bytes");

#if defined(__i386__)
static_assert(sizeof(DBInsertGiveBoxEventReward) == 0x08u,
              "GiveMeBox Taiwan DB dispatcher must retain the i386 8-byte ABI");
#endif

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_INSERT_GIVE_BOX_EVENT_REWARD_H_
