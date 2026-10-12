#include "infra/network/packet/stream_guard.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"

// IDA_EA: 0x84560ac
// IDA_NAME: _ZN6Stream11GetInBufferI31SIG_DEATH_TOWER_GET_BEST_RECORDEEPT_v
// DEMANGLED: SIG_DEATH_TOWER_GET_BEST_RECORD * Stream::GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>(void)
template <>
SIG_DEATH_TOWER_GET_BEST_RECORD* Stream::GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>() {
    const int bytes = 2168;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_DEATH_TOWER_GET_BEST_RECORD* result =
        reinterpret_cast<SIG_DEATH_TOWER_GET_BEST_RECORD*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845240C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI31SIG_DEATH_TOWER_GET_BEST_RECORDEEPT_v
// DEMANGLED: SIG_DEATH_TOWER_GET_BEST_RECORD * CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>(void)
template <>
SIG_DEATH_TOWER_GET_BEST_RECORD* CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>() {
    return stream_->GetInBuffer<SIG_DEATH_TOWER_GET_BEST_RECORD>();
}
