#ifndef OLDSRV_OLD_GAME_EVENT_EVENT_STAGE_LOG_STREAM_H_
#define OLDSRV_OLD_GAME_EVENT_EVENT_STAGE_LOG_STREAM_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/packet/stream_guard.h"

// IDA_EA: 0x8457EF0 / 0x84546CE
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_EVENT_STAGE_LOGEEPT_v /
//           _ZN12CStreamGuard11GetInBufferI19SIG_EVENT_STAGE_LOGEEPT_v
// The canonical specializations are provided by
// stream_memberization_batch_001_58.cpp in manifest_A.mk.
template <>
SIG_EVENT_STAGE_LOG* Stream::GetInBuffer<SIG_EVENT_STAGE_LOG>();

template <>
SIG_EVENT_STAGE_LOG* CStreamGuard::GetInBuffer<SIG_EVENT_STAGE_LOG>();

#endif  // OLDSRV_OLD_GAME_EVENT_EVENT_STAGE_LOG_STREAM_H_
