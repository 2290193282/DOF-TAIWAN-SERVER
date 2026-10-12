#include "infra/network/packet/stream_guard.h"
#include "old_data/db/core/db_runtime_payloads.h"

// IDA_EA: 0x8113194
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_OBJECT_BRING_UPEEPT_v
// DEMANGLED: SIG_OBJECT_BRING_UP * Stream::GetInBuffer<SIG_OBJECT_BRING_UP>(void)
template <>
SIG_OBJECT_BRING_UP* Stream::GetInBuffer<SIG_OBJECT_BRING_UP>() {
    const int bytes = 16;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_OBJECT_BRING_UP* result =
        reinterpret_cast<SIG_OBJECT_BRING_UP*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8111F9A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_OBJECT_BRING_UPEEPT_v
// DEMANGLED: SIG_OBJECT_BRING_UP * CStreamGuard::GetInBuffer<SIG_OBJECT_BRING_UP>(void)
template <>
SIG_OBJECT_BRING_UP* CStreamGuard::GetInBuffer<SIG_OBJECT_BRING_UP>() {
    return stream_->GetInBuffer<SIG_OBJECT_BRING_UP>();
}

// IDA_EA: 0x8454f88
// IDA_NAME: _ZN6Stream11GetInBufferIN13private_store17SIG_PRIVATE_STOREEEEPT_v
// DEMANGLED: private_store::SIG_PRIVATE_STORE * Stream::GetInBuffer<private_store::SIG_PRIVATE_STORE>(void)
template <>
private_store::SIG_PRIVATE_STORE* Stream::GetInBuffer<private_store::SIG_PRIVATE_STORE>() {
    const int bytes = 1076;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    private_store::SIG_PRIVATE_STORE* result =
        reinterpret_cast<private_store::SIG_PRIVATE_STORE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845139C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN13private_store17SIG_PRIVATE_STOREEEEPT_v
// DEMANGLED: private_store::SIG_PRIVATE_STORE * CStreamGuard::GetInBuffer<private_store::SIG_PRIVATE_STORE>(void)
template <>
private_store::SIG_PRIVATE_STORE* CStreamGuard::GetInBuffer<private_store::SIG_PRIVATE_STORE>() {
    return stream_->GetInBuffer<private_store::SIG_PRIVATE_STORE>();
}

// IDA_EA: 0x86a26f8
// IDA_NAME: _ZN6Stream11GetInBufferI21SIG_MERCENARY_DB_DATAEEPT_v
// DEMANGLED: SIG_MERCENARY_DB_DATA * Stream::GetInBuffer<SIG_MERCENARY_DB_DATA>(void)
template <>
SIG_MERCENARY_DB_DATA* Stream::GetInBuffer<SIG_MERCENARY_DB_DATA>() {
    const int bytes = 3028;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MERCENARY_DB_DATA* result =
        reinterpret_cast<SIG_MERCENARY_DB_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869CB8A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_MERCENARY_DB_DATAEEPT_v
// DEMANGLED: SIG_MERCENARY_DB_DATA * CStreamGuard::GetInBuffer<SIG_MERCENARY_DB_DATA>(void)
template <>
SIG_MERCENARY_DB_DATA* CStreamGuard::GetInBuffer<SIG_MERCENARY_DB_DATA>() {
    return stream_->GetInBuffer<SIG_MERCENARY_DB_DATA>();
}
