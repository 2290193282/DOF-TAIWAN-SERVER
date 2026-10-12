#ifndef OLDSRV_GLOBAL_LEVEL_REWARD_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_LEVEL_REWARD_SCRIPT_ABI_H_


// Declaration-only A/public bridge for B/C-owned reward script values embedded
// in CDataManager. B owns the importers; C/11 owns shared event value bodies.
struct stLevelUpRewardItemScript;
struct stGrowthCapsuleScript;
struct stReturnUserRewardScript;
struct stNewAccountLevelUpToJobScript;

// IDA_EA: 0x8371194 / 0x8371530
// CDataManager embeds this object at old offset +0x50AC.
// IDA_EA: 0x837114A
// CDataManager embeds this plain value at old offset +0x50C4.
// IDA_EA: 0x8371544 / 0x837DDE6
// CDataManager embeds this object at old offset +0xB548.
// IDA_EA: 0x837DED8 / 0x837DF44
// CDataManager embeds this object at old offset +0xB5E0.

#endif  // OLDSRV_GLOBAL_LEVEL_REWARD_SCRIPT_ABI_H_
