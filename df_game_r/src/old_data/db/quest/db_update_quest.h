#ifndef OLDSRV_DB_DB_UPDATE_QUEST_H_
#define OLDSRV_DB_DB_UPDATE_QUEST_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)

// IDA_EA: Stream::GetOutBuffer<SIG_UPDATE_QUEST> @0x8450ECC and
// DB_UpdateQuest::_updateQuests @0x8418C1C.  The clear-quest payload is a
// fixed 0x7530-byte bit/data region; the two 20-entry integer tables precede
// it in the SQL column order used by the old dispatcher.
struct SIG_UPDATE_QUEST {
    uint32_t charac_no;             // +0x0000
    int32_t play[20];               // +0x0004
    int32_t play_trigger[20];       // +0x0054
    char clear_quest[0x7530];       // +0x00A4
    uint8_t update_clear_quest;     // +0x75D4
    int32_t urgent_quest_index;     // +0x75D5
    char quest_notify[0x28];        // +0x75D9
};

#pragma pack(pop)

static_assert(sizeof(SIG_UPDATE_QUEST) == 30209u,
              "SIG_UPDATE_QUEST IDA ABI size must stay 30209");
static_assert(offsetof(SIG_UPDATE_QUEST, play_trigger) == 0x54u,
              "SIG_UPDATE_QUEST trigger table must stay at +0x54");
static_assert(offsetof(SIG_UPDATE_QUEST, clear_quest) == 0xA4u,
              "SIG_UPDATE_QUEST clear data must stay at +0xA4");
static_assert(offsetof(SIG_UPDATE_QUEST, update_clear_quest) == 0x75D4u,
              "SIG_UPDATE_QUEST clear flag must stay at +0x75D4");
static_assert(offsetof(SIG_UPDATE_QUEST, urgent_quest_index) == 0x75D5u,
              "SIG_UPDATE_QUEST urgent quest must stay at +0x75D5");
static_assert(offsetof(SIG_UPDATE_QUEST, quest_notify) == 0x75D9u,
              "SIG_UPDATE_QUEST notify data must stay at +0x75D9");


// IDA_EA: 0x8418B74/0x8418B90/0x8418BC0/0x8418BDE/0x8418C1C.
// IDA unit: 13_database_dispatch/DB_UpdateQuest.
class DB_UpdateQuest : public DBDispatcher {
public:
    DB_UpdateQuest();
    virtual ~DB_UpdateQuest();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool _updateQuests(SIG_UPDATE_QUEST* payload);
};

#endif  // OLDSRV_DB_DB_UPDATE_QUEST_H_
