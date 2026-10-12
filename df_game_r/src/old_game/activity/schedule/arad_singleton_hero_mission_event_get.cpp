// Migration marker only.  The former raw 60-byte singleton storage was a
// pseudo-provider and remains removed.  The typed Get provider is in
// hero_mission_lifecycle_runtime.cpp; the direct GetPtr forwarding ABI is in
// hero_mission_gm_controls_runtime.cpp.
//
// IDA_EA: 0x811C9CE
// IDA_NAME: _ZN4ARAD9SingletonI16HeroMissionEventE3GetEv
// DEMANGLED: ARAD::Singleton<HeroMissionEvent>::Get(void)
//
// The original local-static construction/destruction lifetime is represented
// by the canonical typed provider.  This file intentionally has no body.
