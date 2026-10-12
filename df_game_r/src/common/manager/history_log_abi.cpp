#include "common/manager/history_log_abi.h"

#include "common/manager/global_tools.h"

#include <cstdio>
#include <sys/stat.h>

char HistoryLog::time[16] = {};

namespace {

// IDA rodata: 0x8C8617C.
const unsigned char kWriteSelectChFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xC4, 0xB3, 0xB8, 0xAF,
    0xBC, 0xB1, 0xC5, 0xC3, 0x3A, 0x25, 0x73, 0x2C,
    0x20, 0xC1, 0xF7, 0xBE, 0xF7, 0x3A, 0x25, 0x64,
    0x2C, 0x20, 0xB7, 0xB9, 0xBA, 0xA7, 0x3A, 0x25,
    0x64, 0x2C, 0x20, 0xBC, 0xBA, 0xC0, 0xE5, 0xC7,
    0xFC, 0x3A, 0x25, 0x64, 0x0D, 0x0A, 0x00,
};

// IDA rodata: 0x8C861AC.
const unsigned char kWriteDeleteChFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xC4, 0xB3, 0xB8, 0xAF,
    0xBB, 0xE8, 0xC1, 0xA6, 0x3A, 0x25, 0x73, 0x2C,
    0x20, 0xC1, 0xF7, 0xBE, 0xF7, 0x3A, 0x25, 0x64,
    0x2C, 0x20, 0xB7, 0xB9, 0xBA, 0xA7, 0x3A, 0x25,
    0x64, 0x2C, 0x20, 0xBC, 0xBA, 0xC0, 0xE5, 0xC7,
    0xFC, 0x3A, 0x20, 0x25, 0x64, 0x0D, 0x0A, 0x00,
};

// IDA rodata: 0x8C862AE.  This is a CP949-format history-log record; retain
// its original byte sequence rather than substituting a translated message.
const unsigned char kWriteGiveUpFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xB4, 0xF8, 0xC1, 0xAF,
    0xC6, 0xF7, 0xB1, 0xE2, 0x3A, 0x20, 0x25, 0x73,
    0x0D, 0x0A, 0x00,
};

// IDA rodata: 0x8C862C1.  Preserve the original CP949 byte sequence.
const unsigned char kWriteDunClearFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xB4, 0xF8, 0xC1, 0xAF,
    0x20, 0xC5, 0xAC, 0xB8, 0xAE, 0xBE, 0xEE, 0x0D,
    0x0A, 0x00,
};

// IDA rodata: aS_48, consumed solely by HistoryLog::WriteDie.
const unsigned char kWriteDieFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xB4, 0xF8, 0xC1, 0xAF,
    0x20, 0xBB, 0xE7, 0xB8, 0xC1, 0x0D, 0x0A, 0x00,
};

// IDA rodata: 0x8C862D4.  Preserve the original CP949 byte sequence.
const unsigned char kWriteTradeUpFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xC6, 0xAE, 0xB7, 0xB9,
    0xC0, 0xCC, 0xB5, 0xE5, 0x20, 0xBE, 0xF7, 0xB8,
    0xAE, 0xBD, 0xBA, 0xC6, 0xAE, 0x20, 0x49, 0x44,
    0x3A, 0x25, 0x64, 0x20, 0x43, 0x6F, 0x75, 0x6E,
    0x74, 0x3A, 0x25, 0x64, 0x0D, 0x0A, 0x00,
};

// IDA rodata: 0x8C862FC.  Preserve the original CP949 byte sequence.
const unsigned char kWriteTradeDownFormat[] = {
    0x5B, 0x25, 0x73, 0x5D, 0xC6, 0xAE, 0xB7, 0xB9,
    0xC0, 0xCC, 0xB5, 0xE5, 0x20, 0xB4, 0xD9, 0xBF,
    0xEE, 0xB8, 0xAE, 0xBD, 0xBA, 0xC6, 0xAE, 0x20,
    0x49, 0x44, 0x3A, 0x25, 0x64, 0x20, 0x43, 0x6F,
    0x75, 0x6E, 0x74, 0x3A, 0x25, 0x64, 0x0D, 0x0A,
    0x00,
};

}  // namespace

// IDA_EA: 0x84B9AD4
// IDA_NAME: _ZN10HistoryLog4InitEv
// DEMANGLED: HistoryLog::Init(void)
int HistoryLog::Init() {
    struct stat stat_buf;
    if (::stat("./history/", &stat_buf) >= 0 ||
        ::mkdir("./history/", 0755) >= 0) {
        return 1;
    }
    std::printf("Fail To Make Directory, PATH[%s]\n", "./history/");
    return 0;
}

// IDA_EA: 0x84B9D34
// IDA_NAME: _ZN10HistoryLog8LogCloseEP8_IO_FILE
// DEMANGLED: HistoryLog::LogClose(_IO_FILE *)
int HistoryLog::LogClose(FILE* stream) {
    if (!stream) {
        return 0;
    }
    // The original also returns a StaticPool<_IO_FILE, 50> node. That pool's
    // allocator ABI has not yet been reconstructed; preserve the durable
    // close boundary without inventing a second ownership system.
    return std::fclose(stream);
}

// IDA_EA: 0x84B9D06
// IDA_NAME: _ZN10HistoryLog7CheckFDEP8_IO_FILE
// DEMANGLED: HistoryLog::CheckFD(_IO_FILE *)
int HistoryLog::CheckFD(FILE* stream) {
    if (!stream) {
        return 0;
    }
    get_str_datetime(HistoryLog::time, 16);
    return 1;
}

// IDA_EA: 0x84B9DB6
// IDA_NAME: _ZN10HistoryLog13WriteSelectChEP8_IO_FILEPKciii
// DEMANGLED: HistoryLog::WriteSelectCh(_IO_FILE *,char const*,int,int,int)
int HistoryLog::WriteSelectCh(FILE* stream,
                              const char* charac_name,
                              int charac_job,
                              int charac_level,
                              int grow_type) {
    int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream,
                            reinterpret_cast<const char*>(
                                kWriteSelectChFormat),
                            HistoryLog::time,
                            charac_name,
                            charac_job,
                            charac_level,
                            grow_type);
    }
    return result;
}

// IDA_EA: 0x84B9E0A
// IDA_NAME: _ZN10HistoryLog13WriteDeleteChEP8_IO_FILEPciii
// DEMANGLED: HistoryLog::WriteDeleteCh(_IO_FILE *,char *,int,int,int)
int HistoryLog::WriteDeleteCh(FILE* stream,
                              char* charac_name,
                              int charac_job,
                              int charac_level,
                              int grow_type) {
    int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream,
                            reinterpret_cast<const char*>(
                                kWriteDeleteChFormat),
                            HistoryLog::time,
                            charac_name,
                            charac_job,
                            charac_level,
                            grow_type);
    }
    return result;
}

// IDA_EA: 0x84B9E5E
// IDA_NAME: _ZN10HistoryLog12WriteLevelUpEP8_IO_FILEi
// DEMANGLED: HistoryLog::WriteLevelUp(_IO_FILE *,int)
int HistoryLog::WriteLevelUp(FILE* stream, int charac_exp) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        // The original format is deliberately only "[%s]".  Although the
        // integer is passed to fprintf as its third vararg, it is not consumed
        // by this exact format string.
        return std::fprintf(stream, "[%s]", HistoryLog::time, charac_exp);
    }
    return result;
}

// IDA_EA: 0x84B9EE4
// IDA_NAME: _ZN10HistoryLog14WriteFineQuestEP8_IO_FILEi
// DEMANGLED: HistoryLog::WriteFineQuest(_IO_FILE *,int)
int HistoryLog::WriteFineQuest(FILE* stream, int quest_index) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        // The original rodata aS_45 is exactly "[%s]".  GCC still passes
        // quest_index as the fourth fprintf argument, but it is not consumed.
        return std::fprintf(
            stream, "[%s]", HistoryLog::time, quest_index);
    }
    return result;
}

// IDA_EA: 0x84B9F24
// IDA_NAME: _ZN10HistoryLog15WriteLearnSkillEP8_IO_FILEii
// DEMANGLED: HistoryLog::WriteLearnSkill(_IO_FILE *,int,int)
int HistoryLog::WriteLearnSkill(FILE* stream, int skill_index, int skill_level) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream, "[%s] learn_skill,%d,%d", HistoryLog::time,
                            skill_index, skill_level);
    }
    return result;
}

// IDA_EA: 0x84BA1B4
// IDA_NAME: _ZN10HistoryLog13WriteDisJointEP8_IO_FILEi
// DEMANGLED: HistoryLog::WriteDisJoint(_IO_FILE *,int)
int HistoryLog::WriteDisJoint(FILE* stream, int item_id) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        // The IDA export preserves the writer's ABI but not a recoverable
        // rodata label for aS_55.  Keep the operation and item identity
        // durable without fabricating an undocumented literal.
        return std::fprintf(stream, "[%s] disjoint,%d", HistoryLog::time,
                            item_id);
    }
    return result;
}

// IDA_EA: 0x84B9FF8
// IDA_NAME: _ZN10HistoryLog8WriteDieEP8_IO_FILE
// DEMANGLED: HistoryLog::WriteDie(_IO_FILE *)
int HistoryLog::WriteDie(FILE* stream) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream,
                            reinterpret_cast<const char*>(kWriteDieFormat),
                            HistoryLog::time);
    }
    return result;
}

// IDA_EA: 0x84BA030
// IDA_NAME: _ZN10HistoryLog12WriteUseCoinEP8_IO_FILEPKc
// DEMANGLED: HistoryLog::WriteUseCoin(_IO_FILE *,char const*)
int HistoryLog::WriteUseCoin(FILE* stream, const char* charac_name) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        // The recovered function has the two exact arguments above.  Its
        // aS_49 rodata label was not emitted by Hex-Rays, so retain a stable
        // operation record rather than manufacturing a Korean source string.
        return std::fprintf(stream, "[%s] use_coin,%s\r\n", HistoryLog::time,
                            charac_name);
    }
    return result;
}

// IDA_EA: 0x84BA070
// IDA_NAME: _ZN10HistoryLog11WriteGiveUpEP8_IO_FILEPKc
// DEMANGLED: HistoryLog::WriteGiveUp(_IO_FILE *,char const*)
int HistoryLog::WriteGiveUp(FILE* stream, const char* charac_name) {
    int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream,
                            reinterpret_cast<const char*>(kWriteGiveUpFormat),
                            HistoryLog::time,
                            charac_name);
    }
    return result;
}

// IDA_EA: 0x84BA0B0
// IDA_NAME: _ZN10HistoryLog13WriteDunClearEP8_IO_FILE
// DEMANGLED: HistoryLog::WriteDunClear(_IO_FILE *)
int HistoryLog::WriteDunClear(FILE* stream) {
    int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream,
                            reinterpret_cast<const char*>(kWriteDunClearFormat),
                            HistoryLog::time);
    }
    return result;
}

// IDA_EA: 0x84BA0E8
// IDA_NAME: _ZN10HistoryLog12WriteTradeUpEP8_IO_FILEii
// DEMANGLED: HistoryLog::WriteTradeUp(_IO_FILE *,int,int)
int HistoryLog::WriteTradeUp(FILE* stream, int item_id, int item_value) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(
            stream,
            reinterpret_cast<const char*>(kWriteTradeUpFormat),
            HistoryLog::time,
            item_id,
            item_value);
    }
    return result;
}

// IDA_EA: 0x84BA12E
// IDA_NAME: _ZN10HistoryLog14WriteTradeDownEP8_IO_FILEii
// DEMANGLED: HistoryLog::WriteTradeDown(_IO_FILE *,int,int)
int HistoryLog::WriteTradeDown(FILE* stream, int item_id, int item_value) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(
            stream,
            reinterpret_cast<const char*>(kWriteTradeDownFormat),
            HistoryLog::time,
            item_id,
            item_value);
    }
    return result;
}

// IDA_EA: 0x84BA1F4
// IDA_NAME: _ZN10HistoryLog12WriteBuyItemEP8_IO_FILEii
// DEMANGLED: HistoryLog::WriteBuyItem(_IO_FILE *,int,int)
int HistoryLog::WriteBuyItem(FILE* stream, int item_id, int item_value) {
    (void)item_id;
    (void)item_value;
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream, "[%s]", HistoryLog::time);
    }
    return result;
}

// IDA_EA: 0x84BA23A
// IDA_NAME: _ZN10HistoryLog13WriteSellItemEP8_IO_FILEii
// DEMANGLED: HistoryLog::WriteSellItem(_IO_FILE *,int,int)
int HistoryLog::WriteSellItem(FILE* stream, int item_id, int item_value) {
    (void)item_id;
    (void)item_value;
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream, "[%s]", HistoryLog::time);
    }
    return result;
}

// IDA_EA: 0x84BA174
// IDA_NAME: _ZN10HistoryLog16WriteCompositionEP8_IO_FILEi
// DEMANGLED: HistoryLog::WriteComposition(_IO_FILE *,int)
int HistoryLog::WriteComposition(FILE* stream, int item_index) {
    const int result = HistoryLog::CheckFD(stream) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        return std::fprintf(stream, "[%s]", HistoryLog::time, item_index);
    }
    return result;
}
