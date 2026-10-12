#include "infra/network/packet/stream_guard.h"
#include "old_data/db/dungeon/db_load_kill_monster_info.h"

// IDA_EA: 0x8236DB4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_LOAD_KILL_MONSTER_INFOEEPT_v
// DEMANGLED: SIG_LOAD_KILL_MONSTER_INFO * CStreamGuard::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>(void)
template <>
SIG_LOAD_KILL_MONSTER_INFO* CStreamGuard::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>() {
    return stream_->GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>();
}
