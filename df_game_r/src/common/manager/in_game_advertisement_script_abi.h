#ifndef OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_SCRIPT_ABI_H_


// Declaration-only A/public bridge to the B-owned InGameAdvertisementScript.
// IDA_EA: 0x83732F0
// IDA_NAME: _ZN25InGameAdvertisementScriptC2Ev
// CDataManager embeds this 0x97-byte value at old offset +0x4A5C.
// The old CDataManager destructor has no matching destructor call for it.
class InGameAdvertisementScript;


#endif  // OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_SCRIPT_ABI_H_
