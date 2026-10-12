#ifndef OLDSRV_OLD_GAME_EVENT_EVENT_LOGIN_ITEM_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_EVENT_LOGIN_ITEM_RUNTIME_H_

class MySQL;

namespace WongWork {

class CWebEvent {
public:
    // IDA_EA: 0x86C11D0
    // IDA_NAME: _ZN8WongWork9CWebEvent15RecvInfoFromWebEjjjjjj
    // The first unsigned value is the opaque old event-context value.  It is
    // persisted verbatim by ReqSaveEventItem and is never dereferenced by the
    // DB worker.
    static int RecvInfoFromWeb(unsigned int event_context,
                               unsigned int charac_no,
                               unsigned int item_id,
                               unsigned int stack_count,
                               unsigned int event_code,
                               unsigned int event_source);

    // IDA_EA: 0x86C121E
    // IDA_NAME: _ZN8WongWork9CWebEvent14RecvInfoFromDBEP5CUserjjjj
    static int RecvInfoFromDB(class CUser* user,
                              unsigned int item_id,
                              unsigned int stack_count,
                              unsigned int event_code,
                              unsigned int event_item_id);

    // IDA_EA: 0x86C1910
    // IDA_NAME: _ZN8WongWork9CWebEvent16ReqSaveEventItemEjjjjjjb
    static int ReqSaveEventItem(unsigned int event_context,
                                unsigned int charac_no,
                                unsigned int item_id,
                                unsigned int stack_count,
                                unsigned int event_code,
                                unsigned int event_source,
                                bool no_persist);

    // IDA_EA: 0x86C1A2E / 0x86C1BA8 / 0x86C1BD4.
    static int _GetATIAvatarAbility(int item_id);
    static const char* _GetEventString(int event_code);
    static const char* _GetEventIPGCode(int event_code);

    static int GetItemType(int event_kind, int item_no);
};

class EventCommon {
public:
    static unsigned int InsertEventItem2EventTable(unsigned int charac_no,
                                                   unsigned int item_no,
                                                   unsigned int stack_count,
                                                   unsigned int event_code);
    static void LoadCharacEventItem(unsigned int account_id,
                                    unsigned int charac_no,
                                    unsigned int event_item_id);
};

class EventATI {
public:
    static bool CheckAlreadyGetATIItem(unsigned int entry_id, int item_no);
    static int UpdateEventATIInfo(unsigned int entry_id, int item_no);
    static unsigned char ArrangeATIEventItem(unsigned int account_id,
                                             unsigned int event_id);
};

class EventSigong {
public:
    static bool CheckAlreadyGetSigongItem(unsigned int entry_id, int item_no);
    static int UpdateEventSigongInfo(unsigned int entry_id, int item_no);
    static unsigned char ArrangeSigongEventItem(unsigned int account_id,
                                                unsigned int event_id);
};

class EventEverland {
public:
    // IDA_EA: 0x83FAC0D
    // IDA_NAME: _ZN8WongWork13EventEverland27CheckAlreadyGetEverlandItemEji
    // DEMANGLED: WongWork::EventEverland::CheckAlreadyGetEverlandItem(unsigned int,int)
    static bool CheckAlreadyGetEverlandItem(unsigned int entry_id,
                                            int item_no);

    // IDA_EA: 0x83FACE3
    // IDA_NAME: _ZN8WongWork13EventEverland23UpdateEventEverlandInfoEji
    // DEMANGLED: WongWork::EventEverland::UpdateEventEverlandInfo(unsigned int,int)
    static int UpdateEventEverlandInfo(unsigned int entry_id, int item_no);

    // IDA_EA: 0x83FAD60
    // IDA_NAME: _ZN8WongWork13EventEverland24ArrangeEverlandEventItemEjj
    // DEMANGLED: WongWork::EventEverland::ArrangeEverlandEventItem(unsigned int,unsigned int)
    static unsigned char ArrangeEverlandEventItem(unsigned int account_id,
                                                  unsigned int event_id);
};

class EventMuz {
public:
    static bool CheckAlreadyGetMuzItem(unsigned int entry_id, int item_no);
    static int UpdateEventMuzInfo(unsigned int entry_id, int item_no);
    static unsigned char ArrangeMuzEventItem(unsigned int account_id,
                                             unsigned int event_id);
};

class EventAlttle {
public:
    static bool CheckAlreadyGetAlttleItem(unsigned int entry_id, int item_no);
    static int UpdateEventAlttleInfo(unsigned int entry_id, int item_no);
    static unsigned char ArrangeAlttleEventItem(unsigned int account_id,
                                                unsigned int event_id);
};

class EventFCCombo {
public:
    static bool CheckAlreadyFCComboEventItem(unsigned int entry_id,
                                             int item_no);
    static int UpdateFCComboEventInfo(unsigned int entry_id, int item_no);
    static unsigned char ArrangeFCComboEventItem(unsigned int account_id,
                                                 unsigned int event_id);
    static MySQL* GetDBHandle();
};

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_EVENT_EVENT_LOGIN_ITEM_RUNTIME_H_
