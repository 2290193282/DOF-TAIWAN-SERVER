#include "infra/network/packet/stream_guard.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_pool.h"


// IDA_EA: 0x80C8C26
// IDA_NAME: _ZN12CStreamGuardC2EP6Streamb
// DEMANGLED: CStreamGuard::CStreamGuard(Stream *,bool)
CStreamGuard::CStreamGuard(Stream* stream, bool auto_free)
    : stream_(stream), auto_free_(auto_free) {}

// IDA_EA: 0x80DE62C
// IDA_NAME: _ZN12CStreamGuardC2Eb
// DEMANGLED: CStreamGuard::CStreamGuard(bool)
CStreamGuard::CStreamGuard(bool auto_free) : stream_(0), auto_free_(auto_free) {}

// IDA_EA: 0x861C8D2
// IDA_NAME: _ZN12CStreamGuardD2Ev
// DEMANGLED: CStreamGuard::~CStreamGuard()
CStreamGuard::~CStreamGuard() {
    if (!stream_ || !auto_free_ || !stream_->pool_live()) {
        return;
    }
    switch (stream_->pool_kind()) {
        case 1:
            if (GlobalData::small_stream_pool()) {
                GlobalData::small_stream_pool()->Free(stream_);
            }
            break;
        case 2:
            if (GlobalData::big_stream_pool()) {
                GlobalData::big_stream_pool()->Free(stream_);
            }
            break;
        default:
            if (GlobalData::stream_pool()) {
                GlobalData::stream_pool()->Free(stream_);
            }
            break;
    }
    stream_ = 0;
}

// IDA_EA: 0x80C8C46
// IDA_NAME: _ZN12CStreamGuarddeEv
// DEMANGLED: CStreamGuard::operator*(void)
CStreamGuard& CStreamGuard::operator*() {
    return *this;
}

// IDA_EA: 0x80C8C4E
// IDA_NAME: _ZN12CStreamGuardptEv
// DEMANGLED: CStreamGuard::operator->(void)
CStreamGuard* CStreamGuard::operator->() {
    return this;
}

// IDA_EA: 0x80DE644
// IDA_NAME: _ZN12CStreamGuard3SetEP6Stream
// DEMANGLED: CStreamGuard::Set(Stream *)
CStreamGuard* CStreamGuard::Set(Stream* stream) {
    stream_ = stream;
    return this;
}

// IDA_EA: 0x8100FFA
// IDA_NAME: _ZN12CStreamGuard3GetEv
// DEMANGLED: CStreamGuard::Get(void)
Stream* CStreamGuard::Get() const {
    return stream_;
}

// IDA_EA: 0x844D430
// IDA_NAME: _ZN12CStreamGuardlsEb
// DEMANGLED: CStreamGuard::operator<<(bool)
CStreamGuard& CStreamGuard::operator<<(bool value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x8101004
// IDA_NAME: _ZN12CStreamGuardlsEc
// DEMANGLED: CStreamGuard::operator<<(char)
CStreamGuard& CStreamGuard::operator<<(char value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x822AD66
// IDA_NAME: _ZN12CStreamGuardlsEs
// DEMANGLED: CStreamGuard::operator<<(short)
CStreamGuard& CStreamGuard::operator<<(short value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x80C8C56
// IDA_NAME: _ZN12CStreamGuardlsEi
// DEMANGLED: CStreamGuard::operator<<(int)
CStreamGuard& CStreamGuard::operator<<(int value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x80E2276
// IDA_NAME: _ZN12CStreamGuardlsEl
// DEMANGLED: CStreamGuard::operator<<(long)
CStreamGuard& CStreamGuard::operator<<(long value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x822AD8A
// IDA_NAME: _ZN12CStreamGuardlsEPc
// DEMANGLED: CStreamGuard::operator<<(char *)
CStreamGuard& CStreamGuard::operator<<(char* value) {
    if (stream_) *stream_ << value;
    return *this;
}

CStreamGuard& CStreamGuard::operator<<(const char* value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x844D454
// IDA_NAME: _ZN12CStreamGuardlsEh
// DEMANGLED: CStreamGuard::operator<<(unsigned char)
CStreamGuard& CStreamGuard::operator<<(unsigned char value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x847DCFA
// IDA_NAME: _ZN12CStreamGuardlsEt
// DEMANGLED: CStreamGuard::operator<<(unsigned short)
CStreamGuard& CStreamGuard::operator<<(unsigned short value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x80E2292
// IDA_NAME: _ZN12CStreamGuardlsEj
// DEMANGLED: CStreamGuard::operator<<(unsigned int)
CStreamGuard& CStreamGuard::operator<<(unsigned int value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x822ADA6
// IDA_NAME: _ZN12CStreamGuardlsEm
// DEMANGLED: CStreamGuard::operator<<(unsigned long)
CStreamGuard& CStreamGuard::operator<<(unsigned long value) {
    if (stream_) *stream_ << value;
    return *this;
}

// IDA_EA: 0x80DE652
// IDA_NAME: _ZN12CStreamGuard10put_binaryEPvi
// DEMANGLED: CStreamGuard::put_binary(void *,int)
int CStreamGuard::put_binary(void* src, int bytes) {
    return stream_ ? stream_->put_binary(src, bytes) : 0;
}

void* CStreamGuard::GetInBufferUntyped(int bytes) {
    return stream_ ? stream_->GetInBufferUntyped(bytes) : 0;
}
