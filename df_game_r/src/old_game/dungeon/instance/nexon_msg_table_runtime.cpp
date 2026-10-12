#include "old_game/dungeon/instance/nexon_msg_table_runtime.h"

#include <utility>

namespace {

struct NexonMessageEntry {
    int message_code;
    const char* script_key;
};

// Original df_game_r C2 evidence: 0x858001C walks VA 0x93ABB80.  The table
// deliberately has holes; they must remain lookup misses.
const NexonMessageEntry kNexonMessageEntries[] = {
    {0, "nexon_pcroom_msg_00"},
    {1, "nexon_pcroom_msg_01"},
    {2, "nexon_pcroom_msg_02"},
    {3, "nexon_pcroom_msg_03"},
    {4, "nexon_pcroom_msg_04"},
    {5, "nexon_pcroom_msg_05"},
    {6, "nexon_pcroom_msg_06"},
    {10, "nexon_pcroom_msg_10"},
    {11, "nexon_pcroom_msg_11"},
    {13, "nexon_pcroom_msg_13"},
    {14, "nexon_pcroom_msg_14"},
    {15, "nexon_pcroom_msg_15"},
    {16, "nexon_pcroom_msg_16"},
    {17, "nexon_pcroom_msg_17"},
    {18, "nexon_pcroom_msg_18"},
    {19, "nexon_pcroom_msg_19"},
    {22, "nexon_pcroom_msg_22"},
    {23, "nexon_pcroom_msg_23"},
    {24, "nexon_pcroom_msg_24"},
    {25, "nexon_pcroom_msg_25"},
    {26, "nexon_pcroom_msg_26"},
    {27, "nexon_pcroom_msg_27"},
    {90, "nexon_pcroom_msg_90"},
    {91, "nexon_pcroom_msg_91"},
    {92, "nexon_pcroom_msg_92"},
    {93, "nexon_pcroom_msg_93"},
    {94, "nexon_pcroom_msg_94"},
    {95, "nexon_pcroom_msg_95"},
};

const char kEmptyMessageString[] = "";

}  // namespace

// IDA_EA: 0x858001C
// IDA_NAME: _ZN14CNexonMsgTableC2Ev
// DEMANGLED: CNexonMsgTable::CNexonMsgTable(void)
CNexonMsgTable::CNexonMsgTable() {
    for (unsigned int index = 0;
         index < sizeof(kNexonMessageEntries) / sizeof(kNexonMessageEntries[0]);
         ++index) {
        const NexonMessageEntry& entry = kNexonMessageEntries[index];
        message_strings_.insert(std::make_pair(
            entry.message_code, std::string(entry.script_key)));
    }
}

// IDA_EA: 0x85800F8
// IDA_NAME: _ZN14CNexonMsgTableD2Ev
// DEMANGLED: CNexonMsgTable::~CNexonMsgTable()
CNexonMsgTable::~CNexonMsgTable() {}

// IDA_EA: 0x858010C
// IDA_NAME: _ZN14CNexonMsgTable16GetMessageStringEi
// DEMANGLED: CNexonMsgTable::GetMessageString(int)
const char* CNexonMsgTable::GetMessageString(int message_code) {
    const std::map<int, std::string>::iterator found =
        message_strings_.find(message_code);
    if (found == message_strings_.end()) {
        return kEmptyMessageString;
    }
    return found->second.c_str();
}

CNexonMsgTable g_nexonMsgTable;
