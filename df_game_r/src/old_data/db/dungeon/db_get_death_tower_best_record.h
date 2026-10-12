#ifndef OLDSRV_DB_DB_GET_DEATH_TOWER_BEST_RECORD_H_
#define OLDSRV_DB_DB_GET_DEATH_TOWER_BEST_RECORD_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA evidence: WongWork::CDeathTower writes one 23-byte member record at
// 0x82CD5D8..0x82CD63B (20-byte name, level, job, grow type).
struct stDeathTowerRecordMemberInfo_t {
    char charac_name[20];
    uint8_t level;
    uint8_t job;
    uint8_t grow_type;
};

// IDA_EA: 0x8455D1A (SIG_DEATH_TOWER_UPDATE_BEST_RECORD input buffer).
// The same 108-byte slot is repeated by SIG_DEATH_TOWER_GET_BEST_RECORD.
struct SIG_DEATH_TOWER_BEST_RECORD_SLOT {
    uint32_t charac_no;
    uint16_t tower_index;
    uint16_t member_count;
    stDeathTowerRecordMemberInfo_t member_info[4];
    uint32_t stage;
    uint32_t play_time;
};

// IDA_EA: 0x84560AC
// IDA_NAME: _ZN6Stream11GetInBufferI31SIG_DEATH_TOWER_GET_BEST_RECORDEEPT_v
// Stream advances exactly 2168 bytes: two u32 values plus 5*4 slots.
struct SIG_DEATH_TOWER_GET_BEST_RECORD {
    uint32_t record_count;
    uint32_t charac_no;
    SIG_DEATH_TOWER_BEST_RECORD_SLOT records[5][4];
};

static_assert(sizeof(stDeathTowerRecordMemberInfo_t) == 23,
              "death-tower member ABI drift");
static_assert(sizeof(SIG_DEATH_TOWER_BEST_RECORD_SLOT) == 108,
              "death-tower slot ABI drift");
static_assert(sizeof(SIG_DEATH_TOWER_GET_BEST_RECORD) == 2168,
              "death-tower result ABI drift");


// IDA unit: 09_dungeon_pvp_party/DB_GetDeathTowerBestRecord.
// IDA_EA: dispatch @0x8429AD0; makeRequest @0x842A048; ctor/dtor
// @0x844E81A/@0x845E632/@0x845E662.
class DB_GetDeathTowerBestRecord : public DBDispatcher {
public:
    DB_GetDeathTowerBestRecord();
    virtual ~DB_GetDeathTowerBestRecord();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_GET_DEATH_TOWER_BEST_RECORD_H_
