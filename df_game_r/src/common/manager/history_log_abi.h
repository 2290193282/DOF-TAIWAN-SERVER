#ifndef OLDSRV_GLOBAL_HISTORY_LOG_ABI_H_
#define OLDSRV_GLOBAL_HISTORY_LOG_ABI_H_

#include <cstdio>

// IDA unit: 99_misc_global/HistoryLog.
// A owns the public global HistoryLog boundary. Keep one IDA function as one
// source function; do not fold HistoryLog writers into CUser call sites.
struct HistoryLog {
    // IDA_EA: 0x84B9AD4
    // IDA_NAME: _ZN10HistoryLog4InitEv
    // DEMANGLED: HistoryLog::Init(void)
    static int Init();

    // IDA_EA: 0x84B9D34
    // IDA_NAME: _ZN10HistoryLog8LogCloseEP8_IO_FILE
    // DEMANGLED: HistoryLog::LogClose(_IO_FILE *)
    static int LogClose(FILE* stream);

    // IDA_EA: 0x84B9D06
    // IDA_NAME: _ZN10HistoryLog7CheckFDEP8_IO_FILE
    static int CheckFD(FILE* stream);

    // IDA_EA: 0x84B9DB6
    // IDA_NAME: _ZN10HistoryLog13WriteSelectChEP8_IO_FILEPKciii
    static int WriteSelectCh(FILE* stream,
                             const char* charac_name,
                             int charac_job,
                             int charac_level,
                             int grow_type);

    // IDA_EA: 0x84B9E0A
    // IDA_NAME: _ZN10HistoryLog13WriteDeleteChEP8_IO_FILEPciii
    static int WriteDeleteCh(FILE* stream,
                             char* charac_name,
                             int charac_job,
                             int charac_level,
                             int grow_type);

    // IDA_EA: 0x84B9E5E
    // IDA_NAME: _ZN10HistoryLog12WriteLevelUpEP8_IO_FILEi
    // DEMANGLED: HistoryLog::WriteLevelUp(_IO_FILE *,int)
    static int WriteLevelUp(FILE* stream, int charac_exp);

    // IDA_EA: 0x84B9EE4
    // IDA_NAME: _ZN10HistoryLog14WriteFineQuestEP8_IO_FILEi
    // DEMANGLED: HistoryLog::WriteFineQuest(_IO_FILE *,int)
    static int WriteFineQuest(FILE* stream, int quest_index);

    // IDA_EA: 0x84B9F24
    // IDA_NAME: _ZN10HistoryLog15WriteLearnSkillEP8_IO_FILEii
    // DEMANGLED: HistoryLog::WriteLearnSkill(_IO_FILE *,int,int)
    static int WriteLearnSkill(FILE* stream, int skill_index, int skill_level);

    // IDA_EA: 0x84B9FF8
    // IDA_NAME: _ZN10HistoryLog8WriteDieEP8_IO_FILE
    // DEMANGLED: HistoryLog::WriteDie(_IO_FILE *)
    static int WriteDie(FILE* stream);

    // IDA_EA: 0x84BA030
    // IDA_NAME: _ZN10HistoryLog12WriteUseCoinEP8_IO_FILEPKc
    static int WriteUseCoin(FILE* stream, const char* charac_name);

    // IDA_EA: 0x84BA070
    // IDA_NAME: _ZN10HistoryLog11WriteGiveUpEP8_IO_FILEPKc
    // DEMANGLED: HistoryLog::WriteGiveUp(_IO_FILE *,char const*)
    static int WriteGiveUp(FILE* stream, const char* charac_name);

    // IDA_EA: 0x84BA0B0
    // IDA_NAME: _ZN10HistoryLog13WriteDunClearEP8_IO_FILE
    // DEMANGLED: HistoryLog::WriteDunClear(_IO_FILE *)
    static int WriteDunClear(FILE* stream);

    // IDA_EA: 0x84BA0E8
    // IDA_NAME: _ZN10HistoryLog12WriteTradeUpEP8_IO_FILEii
    // DEMANGLED: HistoryLog::WriteTradeUp(_IO_FILE *,int,int)
    static int WriteTradeUp(FILE* stream, int item_id, int item_value);

    // IDA_EA: 0x84BA12E
    // IDA_NAME: _ZN10HistoryLog14WriteTradeDownEP8_IO_FILEii
    // DEMANGLED: HistoryLog::WriteTradeDown(_IO_FILE *,int,int)
    static int WriteTradeDown(FILE* stream, int item_id, int item_value);

    // IDA_EA: 0x84BA1B4
    // IDA_NAME: _ZN10HistoryLog13WriteDisJointEP8_IO_FILEi
    // DEMANGLED: HistoryLog::WriteDisJoint(_IO_FILE *,int)
    static int WriteDisJoint(FILE* stream, int item_id);

    // IDA_EA: 0x84BA1F4
    // IDA_NAME: _ZN10HistoryLog12WriteBuyItemEP8_IO_FILEii
    // DEMANGLED: HistoryLog::WriteBuyItem(_IO_FILE *,int,int)
    static int WriteBuyItem(FILE* stream, int item_id, int item_value);

    // IDA_EA: 0x84BA23A
    // IDA_NAME: _ZN10HistoryLog13WriteSellItemEP8_IO_FILEii
    // DEMANGLED: HistoryLog::WriteSellItem(_IO_FILE *,int,int)
    static int WriteSellItem(FILE* stream, int item_id, int item_value);

    // IDA_EA: 0x84BA174
    // IDA_NAME: _ZN10HistoryLog16WriteCompositionEP8_IO_FILEi
    // DEMANGLED: HistoryLog::WriteComposition(_IO_FILE *,int)
    static int WriteComposition(FILE* stream, int item_index);

    static char time[16];
};

#endif  // OLDSRV_GLOBAL_HISTORY_LOG_ABI_H_
