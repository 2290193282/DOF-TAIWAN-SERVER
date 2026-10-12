#include "infra/security/policy/codehack_checksum.h"

#include <string.h>


namespace {

char* StrCpyMaxPath(char* dest, const char* src) {
    if (!src) {
        dest[0] = 0;
        return dest;
    }
    strncpy(dest, src, 0x103u);
    dest[0x103] = 0;
    return dest;
}

uint32_t read_u32(const char* src) {
    uint32_t value = 0;
    memcpy(&value, src, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x808A700
// IDA_NAME: _ZN18CodeHackChecksumExC2Ev
// DEMANGLED: CodeHackChecksumEx::CodeHackChecksumEx(void)
CodeHackChecksumEx::CodeHackChecksumEx()
    : CodeHackChecksumData(),
      file_name_(),
      range_() {
    memset(file_name_, 0, sizeof(file_name_));
    memset(&range_, 0, sizeof(range_));
}

// IDA_EA: 0x8575F10
// IDA_NAME: _ZN18CodeHackChecksumExC2ERKS_
// DEMANGLED: CodeHackChecksumEx::CodeHackChecksumEx(CodeHackChecksumEx const&)
CodeHackChecksumEx::CodeHackChecksumEx(const CodeHackChecksumEx& rhs)
    : CodeHackChecksumData(rhs),
      file_name_(),
      range_(rhs.range_) {
    memcpy(file_name_, rhs.file_name_, sizeof(file_name_));
}

// IDA_EA: 0x808A76C
// IDA_NAME: _ZN18CodeHackChecksumExD2Ev
// DEMANGLED: CodeHackChecksumEx::~CodeHackChecksumEx()
CodeHackChecksumEx::~CodeHackChecksumEx() {
    checksums_.clear();
}

// IDA_EA: 0x808A7AE
// IDA_NAME: _ZN18CodeHackChecksumEx11SetFileNameEPKc
// DEMANGLED: CodeHackChecksumEx::SetFileName(char const*)
char* CodeHackChecksumEx::SetFileName(const char* src) {
    return StrCpyMaxPath(file_name_, src);
}

// IDA_EA: 0x808A7CC
// IDA_NAME: _ZN18CodeHackChecksumEx11GetFileNameEv
// DEMANGLED: CodeHackChecksumEx::GetFileName(void)
char* CodeHackChecksumEx::GetFileName() {
    return file_name_;
}

// IDA_EA: 0x808A7D8
// IDA_NAME: _ZN18CodeHackChecksumEx21SetCodeHackCheckRangeERK18CodeHackCheckRange
// DEMANGLED: CodeHackChecksumEx::SetCodeHackCheckRange(CodeHackCheckRange const&)
int CodeHackChecksumEx::SetCodeHackCheckRange(
    const CodeHackCheckRange& range) {
    range_ = range;
    return static_cast<int>(range_.start);
}

// IDA_EA: 0x808A7F4
// IDA_NAME: _ZN18CodeHackChecksumEx21GetCodeHackCheckRangeEv
// DEMANGLED: CodeHackChecksumEx::GetCodeHackCheckRange(void)
CodeHackCheckRange* CodeHackChecksumEx::GetCodeHackCheckRange() {
    return &range_;
}

// IDA_EA: 0x808A802
// IDA_NAME: _ZN18CodeHackChecksumEx7setDataEPKc
// DEMANGLED: CodeHackChecksumEx::setData(char const*)
bool CodeHackChecksumEx::setData(const char* src) {
    uint8_t name_len = static_cast<uint8_t>(src[0]);
    const char* cursor = src + 4;
    memset(file_name_, 0, sizeof(file_name_));
    memcpy(file_name_, cursor, name_len);
    cursor += name_len;

    memcpy(&range_, cursor, sizeof(range_));
    cursor += sizeof(range_);

    uint32_t checksum_count = read_u32(cursor);
    cursor += 4;
    for (uint32_t index = 0; index < checksum_count; ++index) {
        CodeHackChecksum checksum;
        memcpy(&checksum, cursor, sizeof(checksum));
        cursor += sizeof(checksum);
        AddCodeHackChecksum(checksum);
    }
    return false;
}

// IDA_EA: 0x808A8C8
// IDA_NAME: _ZN18CodeHackChecksumEx15IsRightChecksumEPK16CodeHackChecksum
// DEMANGLED: CodeHackChecksumEx::IsRightChecksum(CodeHackChecksum const*)
int CodeHackChecksumEx::IsRightChecksum(
    const CodeHackChecksum* checksum) const {
    return CodeHackChecksumData::IsRightChecksum(checksum);
}
