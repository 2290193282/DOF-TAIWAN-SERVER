#include "infra/network/packet/stream_guard.h"
#include "old_data/db/dungeon/blood_best_record_payload.h"

// IDA_EA: 0x84569c8
// IDA_NAME: _ZN6Stream11GetInBufferI34SIG_LOAD_BLOOD_DUNGEON_BEST_RECORDEEPT_v
// DEMANGLED: SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD * Stream::GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>(void)
template <>
SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD* Stream::GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>() {
    const int bytes = 16;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD* result =
        reinterpret_cast<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452E78
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_LOAD_BLOOD_DUNGEON_BEST_RECORDEEPT_v
// DEMANGLED: SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD * CStreamGuard::GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>(void)
template <>
SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD* CStreamGuard::GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>() {
    return stream_->GetInBuffer<SIG_LOAD_BLOOD_DUNGEON_BEST_RECORD>();
}

// IDA_EA: 0x869fa76
// IDA_NAME: _ZN6Stream11GetInBufferI36SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORDEEPT_v
// DEMANGLED: SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD * Stream::GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>(void)
template <>
SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* Stream::GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>() {
    const int bytes = 16;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* result =
        reinterpret_cast<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869AA26
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI36SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORDEEPT_v
// DEMANGLED: SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD * CStreamGuard::GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>(void)
template <>
SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* CStreamGuard::GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>() {
    return stream_->GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>();
}
