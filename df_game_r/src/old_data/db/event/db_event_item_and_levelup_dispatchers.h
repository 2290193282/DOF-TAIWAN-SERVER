#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ITEM_AND_LEVELUP_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ITEM_AND_LEVELUP_DISPATCHERS_H_

#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

struct SIG_EVENT_STAGE_LOG;

// IDA stream specialization `SIG_SAVE_EVENTITEM` reserves exactly 25 bytes.
// All offsets below are read directly by DB_SaveEventItem @0x841A7E4 and by
// CWebEvent::ReqSaveEventItem @0x86C1910.
#pragma pack(push, 1)
struct SIG_SAVE_EVENTITEM {
    uint32_t event_context;  // +0x00: forwarded unchanged to load reply
    uint32_t charac_no;      // +0x04
    uint32_t event_code;     // +0x08
    uint32_t item_id;        // +0x0c
    uint32_t stack_count;    // +0x10
    uint32_t event_source;   // +0x14: event-table entry/account discriminator
    uint8_t no_persist;      // +0x18
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_EVENTITEM) == 25,
              "SIG_SAVE_EVENTITEM must retain the old 25-byte wire ABI");

// IDA units: 11_event_runtime/DB_SaveEventItem and DB_RemoveEventItem.
// DBTask slots 76 and 77 use their original signal numbers directly.
class DB_SaveEventItem : public DBDispatcher {
public:
    DB_SaveEventItem();
    virtual ~DB_SaveEventItem();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

private:
    // IDA_EA: 0x841A51A
    // IDA_NAME: _ZN16DB_SaveEventItem25_CheckAlreadyGetYegamItemEii
    // DEMANGLED: DB_SaveEventItem::_CheckAlreadyGetYegamItem(int,int)
    bool _CheckAlreadyGetYegamItem(int entry_id, int item_id);

    // IDA_EA: 0x841A5EA
    // IDA_NAME: _ZN16DB_SaveEventItem21_UpdateEventYegamInfoEii
    // DEMANGLED: DB_SaveEventItem::_UpdateEventYegamInfo(int,int)
    int _UpdateEventYegamInfo(int entry_id, int item_id);

    // IDA_EA: 0x841A662
    // IDA_NAME: _ZN16DB_SaveEventItem19_HardCodeEventItemsEP18SIG_SAVE_EVENTITEM
    // DEMANGLED: DB_SaveEventItem::_HardCodeEventItems(SIG_SAVE_EVENTITEM *)
    int _HardCodeEventItems(SIG_SAVE_EVENTITEM* request);
};

class DB_RemoveEventItem : public DBDispatcher {
public:
    DB_RemoveEventItem();
    virtual ~DB_RemoveEventItem();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

// IDA unit: 11_event_runtime/DB_InsertEventStageLog, DBTask slot 819.
class DB_InsertEventStageLog : public DBDispatcher {
public:
    DB_InsertEventStageLog();
    virtual ~DB_InsertEventStageLog();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x844920E
    // IDA_NAME: _ZN22DB_InsertEventStageLog11makeRequestER19SIG_EVENT_STAGE_LOG
    // DEMANGLED: DB_InsertEventStageLog::makeRequest(SIG_EVENT_STAGE_LOG &)
    static void makeRequest(SIG_EVENT_STAGE_LOG& payload);
};

// IDA unit: 11_event_runtime/DB_LoadEventLevelupSupport, DBTask slot 848.
class DB_LoadEventLevelupSupport : public DBDispatcher {
public:
    DB_LoadEventLevelupSupport();
    virtual ~DB_LoadEventLevelupSupport();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, int charac_no);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ITEM_AND_LEVELUP_DISPATCHERS_H_
