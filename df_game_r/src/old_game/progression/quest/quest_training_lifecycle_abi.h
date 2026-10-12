#ifndef OLDSRV_GLOBAL_QUEST_TRAINING_LIFECYCLE_ABI_H_
#define OLDSRV_GLOBAL_QUEST_TRAINING_LIFECYCLE_ABI_H_


// Declaration-only A/public bridge for CDataManager embedded quest/training
// PVF lifecycle. C/08 owns the value bodies and importers.
struct QuestParameterScript;
struct TrainingQuestScript;

// IDA_EA: 0x836F1C2 / 0x837D3CA.
// CDataManager embeds QuestParameterScript at old offset +0x8CC8.
// IDA_EA: 0x8371CC8 / 0x837D488.
// CDataManager embeds TrainingQuestScript at old offset +0x8D34.

#endif  // OLDSRV_GLOBAL_QUEST_TRAINING_LIFECYCLE_ABI_H_
