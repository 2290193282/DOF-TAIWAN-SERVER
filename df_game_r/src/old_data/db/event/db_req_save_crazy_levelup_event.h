#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_REQ_SAVE_CRAZY_LEVELUP_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_REQ_SAVE_CRAZY_LEVELUP_EVENT_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

// Old-service Stream ABI used by DB_ReqSaveCrazyLevelupEvent.
//
// IDA_EA: Stream::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT> @0x827E518,
// CStreamGuard::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT> @0x827DFA2, and
// Stream::GetOutBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT> @0x8451F42.
// All three helpers use exactly 13 bytes.  The two observed producers copy
// five bytes including the terminating NUL into event_id, then write m_id and
// charac_no at +5 and +9 respectively; there is no tail padding.
#pragma pack(push, 1)
struct SIG_SAVE_CRAZY_LEVELUP_EVENT {
    char event_id[5];       // +0x00, observed values: "0612" / "0707"
    uint32_t m_id;          // +0x05
    uint32_t charac_no;     // +0x09
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_CRAZY_LEVELUP_EVENT) == 13u,
              "SIG_SAVE_CRAZY_LEVELUP_EVENT IDA Stream ABI must stay 13 bytes");
static_assert(offsetof(SIG_SAVE_CRAZY_LEVELUP_EVENT, m_id) == 5u,
              "SIG_SAVE_CRAZY_LEVELUP_EVENT m_id offset must stay +5");
static_assert(offsetof(SIG_SAVE_CRAZY_LEVELUP_EVENT, charac_no) == 9u,
              "SIG_SAVE_CRAZY_LEVELUP_EVENT charac_no offset must stay +9");

// IDA unit: 11_event_runtime/DB_ReqSaveCrazyLevelupEvent.
class DB_ReqSaveCrazyLevelupEvent : public DBDispatcher {
public:
    DB_ReqSaveCrazyLevelupEvent();
    virtual ~DB_ReqSaveCrazyLevelupEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_REQ_SAVE_CRAZY_LEVELUP_EVENT_H_
