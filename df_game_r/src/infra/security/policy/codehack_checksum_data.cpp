#include "infra/security/policy/codehack_checksum.h"

#include <string.h>


// IDA_EA: 0x808AC84
// IDA_NAME: _ZN20CodeHackChecksumDataC2Ev
// DEMANGLED: CodeHackChecksumData::CodeHackChecksumData(void)
CodeHackChecksumData::CodeHackChecksumData() : checksums_() {}

// IDA_EA: 0x8575EF6
// IDA_NAME: _ZN20CodeHackChecksumDataC2ERKS_
// DEMANGLED: CodeHackChecksumData::CodeHackChecksumData(CodeHackChecksumData const&)
CodeHackChecksumData::CodeHackChecksumData(const CodeHackChecksumData& rhs)
    : checksums_(rhs.checksums_) {}

// IDA_EA: 0x808AC98
// IDA_NAME: _ZN20CodeHackChecksumDataD2Ev
// DEMANGLED: CodeHackChecksumData::~CodeHackChecksumData()
CodeHackChecksumData::~CodeHackChecksumData() {
    checksums_.clear();
}

// IDA_EA: 0x808A61A
// IDA_NAME: _ZN20CodeHackChecksumData5ClearEv
// DEMANGLED: CodeHackChecksumData::Clear(void)
int CodeHackChecksumData::Clear() {
    checksums_.clear();
    return 0;
}

// IDA_EA: 0x808A62E
// IDA_NAME: _ZN20CodeHackChecksumData7ReserveEl
// DEMANGLED: CodeHackChecksumData::Reserve(long)
int CodeHackChecksumData::Reserve(long count) {
    checksums_.reserve(static_cast<size_t>(count));
    return 0;
}

// IDA_EA: 0x808A648
// IDA_NAME: _ZN20CodeHackChecksumData19AddCodeHackChecksumERK16CodeHackChecksum
// DEMANGLED: CodeHackChecksumData::AddCodeHackChecksum(CodeHackChecksum const&)
void CodeHackChecksumData::AddCodeHackChecksum(
    const CodeHackChecksum& checksum) {
    checksums_.push_back(checksum);
}

// IDA_EA: 0x808A662
// IDA_NAME: _ZN20CodeHackChecksumData15IsRightChecksumEPK16CodeHackChecksum
// DEMANGLED: CodeHackChecksumData::IsRightChecksum(CodeHackChecksum const*)
int CodeHackChecksumData::IsRightChecksum(
    const CodeHackChecksum* checksum) const {
    for (std::vector<CodeHackChecksum>::const_iterator it = checksums_.begin();
         it != checksums_.end(); ++it) {
        if (memcmp(&(*it), checksum, 0x20u) == 0) {
            return 1;
        }
    }
    return 0;
}
