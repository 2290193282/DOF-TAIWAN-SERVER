#ifndef OLDSRV_GLOBAL_IN_GAME_EVENT_RUNTIME_ABI_H_
#define OLDSRV_GLOBAL_IN_GAME_EVENT_RUNTIME_ABI_H_


// Declaration-only A/public bridge to C-owned 11_event_runtime values used by
// the old CDataManager/PVF in-game-event slots.
class CLottery_NeedMoney;
struct EventEtcScript;

// IDA_EA: 0x837DD28 / 0x837DD3C
// CDataManager embeds CLottery_NeedMoney at old offset +0xB4D4.
// IDA_EA: 0x8372E66 / 0x837DDA0
// CDataManager embeds EventEtcScript at old offset +0xB520.

#endif  // OLDSRV_GLOBAL_IN_GAME_EVENT_RUNTIME_ABI_H_
