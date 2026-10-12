#include "old_data/db/pvp_party/db_save_rank.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



#pragma pack(push, 1)
struct SIG_SAVE_RANK_RECORD {
    uint32_t reserved_00;      // +0x00
    uint32_t dungeon_no;       // +0x04
    uint8_t difficulty;        // +0x08
    uint8_t reserved_09[55];   // +0x09
    uint16_t style;            // +0x40
    uint16_t technic;          // +0x42
    uint32_t attacked;         // +0x44
    uint16_t rank;             // +0x48
    uint8_t reserved_4A[6];    // +0x4A
};

struct SIG_SAVE_RANK {
    uint32_t charac_no;             // +0
    uint32_t count;                 // +4
    SIG_SAVE_RANK_RECORD records[100];  // +8
};
#pragma pack(pop)

typedef char SIG_SAVE_RANK_RECORD_Size_Must_Be_80[
    (sizeof(SIG_SAVE_RANK_RECORD) == 80) ? 1 : -1];
typedef char SIG_SAVE_RANK_Size_Must_Be_0x1F48[
    (sizeof(SIG_SAVE_RANK) == 0x1F48) ? 1 : -1];

// IDA_EA: 0x844D99A
// IDA_NAME: _ZN11DB_SaveRankC2Ev
// DEMANGLED: DB_SaveRank::DB_SaveRank(void)
DB_SaveRank::DB_SaveRank() {}

// IDA_EA: 0x844D9B6
// IDA_NAME: _ZN11DB_SaveRankD2Ev
// DEMANGLED: DB_SaveRank::~DB_SaveRank()
// IDA_EA: 0x844D9E6
// IDA_NAME: _ZN11DB_SaveRankD0Ev
// DEMANGLED: DB_SaveRank::~DB_SaveRank() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveRank::~DB_SaveRank() {}

// IDA_EA: 0x84174C0
// IDA_NAME: _ZN11DB_SaveRank8dispatchEiiP6Stream
// DEMANGLED: DB_SaveRank::dispatch(int,int,Stream *)
bool DB_SaveRank::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_RANK rank;
    if (stream->get_binary(&rank, sizeof(rank)) != 1) {
        return false;
    }
    return SaveRankAll(&rank) == true;
}

// IDA_EA: 0x8417510
// IDA_NAME: _ZN11DB_SaveRank11SaveRankAllEP13SIG_SAVE_RANK
// DEMANGLED: DB_SaveRank::SaveRankAll(SIG_SAVE_RANK *)
bool DB_SaveRank::SaveRankAll(SIG_SAVE_RANK* rank) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    for (uint32_t i = 0; rank->count > i; ++i) {
        const SIG_SAVE_RANK_RECORD& record = rank->records[i];
        db_handle->set_query(
            "inSert into charac_best_record (charac_no,dungeon_no,difficulty,style,technic,attacked,rank) values(%d,%d,%d,%d,%d,%d,%d)",
            rank->charac_no,
            record.dungeon_no,
            record.difficulty,
            record.style,
            record.technic,
            record.attacked,
            record.rank);
        if (db_handle->exec(true) != true) {
            db_handle->set_query(
                "upDate charac_best_record set style=%d, technic=%d ,attacked=%d, rank=%d where charac_no=%d and dungeon_no=%d and difficulty=%d",
                record.style,
                record.technic,
                record.attacked,
                record.rank,
                rank->charac_no,
                record.dungeon_no,
                record.difficulty);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    }
    return true;
}
