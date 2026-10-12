#include "old_game/player/core/cuser.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// Generated from IDA 9.1 Hex-Rays export.
// Keep one IDA function as one source function during cleanup.

// IDA_EA: 0x808A966
// IDA_NAME: _ZN20CodeHackCheckRangeExC2Ev
// DEMANGLED: CodeHackCheckRangeEx::CodeHackCheckRangeEx(void)
CodeHackCheckRangeEx::CodeHackCheckRangeEx() {
    std::memset(file_name_, 0, 0x20u);
    range_start_ = 0;
    range_size_ = 0;
}

// IDA_EA: 0x808A9AA
// IDA_NAME: _ZN20CodeHackCheckRangeEx11SetFileNameEPKc
// DEMANGLED: CodeHackCheckRangeEx::SetFileName(char const*)
char* CodeHackCheckRangeEx::SetFileName(const char* source) {
    std::memset(file_name_, 0, sizeof(file_name_));
    return std::strncpy(file_name_, source, sizeof(file_name_) - 1u);
}

// IDA_EA: 0x808A9C4
// IDA_NAME: _ZNK20CodeHackCheckRangeEx11GetFileNameEv
// DEMANGLED: CodeHackCheckRangeEx::GetFileName(void) const
const char* CodeHackCheckRangeEx::GetFileName() const {
    return file_name_;
}

// IDA_EA: 0x808A9CC
// IDA_NAME: _ZN20CodeHackCheckRangeEx7setDataEPKc
// DEMANGLED: CodeHackCheckRangeEx::setData(char const*)
char* CodeHackCheckRangeEx::setData(const char* source) {
    const unsigned char file_name_length =
        static_cast<unsigned char>(source[0]);
    const char* cursor = source + 4;
    std::strncpy(file_name_, cursor, file_name_length);
    cursor += file_name_length;
    std::memcpy(&range_start_, cursor, sizeof(range_start_) + sizeof(range_size_));
    return const_cast<char*>(cursor + sizeof(range_start_) + sizeof(range_size_));
}

// IDA_EA: 0x8575C9E
// IDA_NAME: _ZN20CodeHackCheckStorage7IsEmptyEv
// DEMANGLED: CodeHackCheckStorage::IsEmpty(void)
bool CodeHackCheckStorage::IsEmpty() {
    return checksums_.size() == 0u;
}

// IDA_EA: 0x8575CB6
// IDA_NAME: _ZN20CodeHackCheckStorage6IsFullEv
// DEMANGLED: CodeHackCheckStorage::IsFull(void)
bool CodeHackCheckStorage::IsFull() {
    return checksums_.size() == 48u;
}

// IDA_EA: 0x8575CD0
// IDA_NAME: _ZN20CodeHackCheckStorage8PushDataEPK18CodeHackChecksumEx
// DEMANGLED: CodeHackCheckStorage::PushData(CodeHackChecksumEx const*)
int CodeHackCheckStorage::PushData(const CodeHackChecksumEx* value) {
    if (IsFull()) {
        return 0;
    }
    checksums_.push(*value);
    return 1;
}

// IDA_EA: 0x8575D06
// IDA_NAME: _ZN20CodeHackCheckStorage7PopDataEPc
// DEMANGLED: CodeHackCheckStorage::PopData(char *)
int CodeHackCheckStorage::PopData(char* file_name) {
    if (IsEmpty()) {
        return 0;
    }

    CodeHackChecksumEx value(checksums_.front());
    checksums_.pop();
    std::strcpy(file_name, value.GetFileName());
    return 1;
}

// IDA_EA: 0x8575DAC
// IDA_NAME: _ZN20CodeHackCheckStorage7PopDataEv
// DEMANGLED: CodeHackCheckStorage::PopData(void)
int CodeHackCheckStorage::PopData() {
    checksums_.pop();
    return 1;
}

// IDA_EA: 0x8575DC4
// IDA_NAME: _ZN20CodeHackCheckStorage15IsRightChecksumEPK16CodeHackChecksum
// DEMANGLED: CodeHackCheckStorage::IsRightChecksum(CodeHackChecksum const*)
int CodeHackCheckStorage::IsRightChecksum(const CodeHackChecksum* checksum) {
    if (IsEmpty()) {
        return 0;
    }
    return checksums_.front().IsRightChecksum(checksum);
}

// IDA_EA: 0x867EA5C
// IDA_NAME: _ZN5CUser22checkCodeChecksumRangeERK18CodeHackChecksumExRK20CodeHackCheckRangeEx
// DEMANGLED: CUser::checkCodeChecksumRange(CodeHackChecksumEx const&,CodeHackCheckRangeEx const&)
int CUser::checkCodeChecksumRange(const CodeHackChecksumEx& checksum,
                                  const CodeHackCheckRangeEx& range) {
    if (GetResumeChecksumTime() < GlobalData::s_systemTime_.getCurSec()) {
        if (code_hack_storage_.PushData(&checksum) != 1) {
            return 0;
        }

        PacketGuard packet;
        packet.put_header(0, 111);
        const char* const file_name = range.GetFileName();
        const std::size_t file_name_length = std::strlen(file_name);
        packet.put_int(static_cast<int>(file_name_length));
        packet.put_str(file_name, file_name_length);
        packet.put_int(static_cast<int>(range.range_start_));
        packet.put_int(static_cast<int>(range.range_size_));
        packet.finalize(true);
        Send(packet);
    }
    return 1;
}

// IDA_EA: 0x867EBBA
// IDA_NAME: _ZN5CUser17checkCodeChecksumERK16CodeHackChecksumPc
// DEMANGLED: CUser::checkCodeChecksum(CodeHackChecksum const&,char *)
int CUser::checkCodeChecksum(const CodeHackChecksum& checksum, char* dest) {
    if (code_hack_storage_.IsRightChecksum(&checksum)) {
        code_hack_storage_.PopData();
        return 1;
    }

    code_hack_storage_.PopData(dest);
    return 0;
}
