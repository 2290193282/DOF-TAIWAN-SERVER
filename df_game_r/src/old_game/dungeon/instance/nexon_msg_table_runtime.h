#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_NEXON_MSG_TABLE_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_NEXON_MSG_TABLE_RUNTIME_H_

#include <map>
#include <string>

// IDA C2 @ 0x858001C builds a std::map<int, std::string> directly in this
// object from the 28-entry table at VA 0x93ABB80.  The receiver only asks for
// the raw script-string key; an absent key maps to the original shared empty
// string rather than an invented fallback.
class CNexonMsgTable {
public:
    CNexonMsgTable();
    ~CNexonMsgTable();

    // IDA_EA: 0x858010C
    // IDA_NAME: _ZN14CNexonMsgTable16GetMessageStringEi
    // DEMANGLED: CNexonMsgTable::GetMessageString(int)
    const char* GetMessageString(int message_code);

private:
    std::map<int, std::string> message_strings_;
};

extern CNexonMsgTable g_nexonMsgTable;

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_NEXON_MSG_TABLE_RUNTIME_H_
