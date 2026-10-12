#ifndef OLDSRV_PROTOCOL_BLOOD_DUNGEON_STREAM_H_
#define OLDSRV_PROTOCOL_BLOOD_DUNGEON_STREAM_H_

#include "old_data/db/dungeon/blood_dungeon_payload.h"
#include "infra/network/packet/stream_guard.h"

// IDA_EA: 0x832A696
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * CStreamGuard::GetInBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* CStreamGuard::GetInBuffer<SIG_BLOOD_DUNGEON>();

// IDA_EA: 0x832A6AC
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * Stream::GetInBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* Stream::GetInBuffer<SIG_BLOOD_DUNGEON>();

// IDA_EA: 0x845352A
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_BLOOD_DUNGEONEEPT_v
// DEMANGLED: SIG_BLOOD_DUNGEON * Stream::GetOutBuffer<SIG_BLOOD_DUNGEON>(void)
template <>
SIG_BLOOD_DUNGEON* Stream::GetOutBuffer<SIG_BLOOD_DUNGEON>();

#endif  // OLDSRV_PROTOCOL_BLOOD_DUNGEON_STREAM_H_
