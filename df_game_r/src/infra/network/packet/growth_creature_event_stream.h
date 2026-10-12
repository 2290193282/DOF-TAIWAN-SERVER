#ifndef OLDSRV_PROTOCOL_GROWTH_CREATURE_EVENT_STREAM_H_
#define OLDSRV_PROTOCOL_GROWTH_CREATURE_EVENT_STREAM_H_

#include "old_game/activity/schedule/character_event_records.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x84546E4
// IDA_NAME: _ZN6Stream12GetOutBufferIN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoEEEPT_v
// DEMANGLED: SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo *
//            Stream::GetOutBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>(void)
template <>
SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*
Stream::GetOutBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>();

#endif  // OLDSRV_PROTOCOL_GROWTH_CREATURE_EVENT_STREAM_H_
