#ifndef OLDSRV_GLOBAL_BINGO_BROADCAST_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_BINGO_BROADCAST_SCRIPT_ABI_H_


// Declaration-only A/public bridge to B-owned Bingo/BroadCast script values.
// B owns the RDAR importers; C/11 owns Bingo runtime/event consumers.
class stBingoScript;
class stBroadCastItemScript;

// IDA_EA: 0x83715B6 / 0x837DDFC
// CDataManager embeds this object at old offset +0xB568.
// IDA_EA: 0x8371694 / 0x837DE92
// CDataManager embeds this object at old offset +0xB5BC.

#endif  // OLDSRV_GLOBAL_BINGO_BROADCAST_SCRIPT_ABI_H_
