#ifndef OLDSRV_PROTOCOL_STREAM_ITEM_LOCK_H_
#define OLDSRV_PROTOCOL_STREAM_ITEM_LOCK_H_

#include "infra/network/packet/stream_guard.h"


struct SIG_ITEM_LOCK_UPDATE;

template <>
SIG_ITEM_LOCK_UPDATE*
Stream::GetInBuffer<SIG_ITEM_LOCK_UPDATE>();

template <>
SIG_ITEM_LOCK_UPDATE*
Stream::GetOutBuffer<SIG_ITEM_LOCK_UPDATE>();

template <>
SIG_ITEM_LOCK_UPDATE*
CStreamGuard::GetInBuffer<SIG_ITEM_LOCK_UPDATE>();

#endif  // OLDSRV_PROTOCOL_STREAM_ITEM_LOCK_H_
