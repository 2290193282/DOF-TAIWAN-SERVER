#ifndef OLDSRV_11_EVENT_RUNTIME_EVENT_TABLE_H_
#define OLDSRV_11_EVENT_RUNTIME_EVENT_TABLE_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x811A0C8 in the companion source.

#include <vector>

class CEventBase;

// IDA unit: 11_event_runtime/CEventTable.
class CEventTable {
public:
    CEventTable();
    ~CEventTable();

    static int GetEventTableCount();
    // IDA_EA: 0x811A0DE
    // IDA_NAME: _ZN11CEventTable12GetEventBaseEi
    // DEMANGLED: CEventTable::GetEventBase(int)
    static CEventBase* GetEventBase(int index);

    // IDA_EA: 0x811A11E
    // IDA_NAME: _ZN11CEventTable10GetEventIdEi
    // DEMANGLED: CEventTable::GetEventId(int)
    static int GetEventId(int index);

    // IDA_EA: 0x811A1E0
    // IDA_NAME: _ZN11CEventTable16VerifyEventTableERSt6vectorIiSaIiEE
    // DEMANGLED: CEventTable::VerifyEventTable(std::vector<int>&)
    static bool VerifyEventTable(std::vector<int>& missing_indexes);

    // IDA_EA: 0x811A4D4
    // IDA_NAME: _ZN11CEventTable16InsertEventTableERSt6vectorIiSaIiEE
    // DEMANGLED: CEventTable::InsertEventTable(std::vector<int>&)
    static bool InsertEventTable(std::vector<int>& missing_indexes);
};


#endif  // OLDSRV_11_EVENT_RUNTIME_EVENT_TABLE_H_
