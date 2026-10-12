#include "old_data/db/npc/db_update_npc_relationship.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

namespace {

enum {
    kSigUpdateNPCRelationshipSize = 608,
    kNPCRelationshipEntrySize = 12,
};

#define DB_UPDATE_NPC_REL_I32(payload, offset) \
    (*reinterpret_cast<int*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844EF6E
// IDA_NAME: _ZN24DB_UpdateNPCRelationshipC2Ev
// DEMANGLED: DB_UpdateNPCRelationship::DB_UpdateNPCRelationship(void)
DB_UpdateNPCRelationship::DB_UpdateNPCRelationship() {}

// IDA_EA: 0x845D486
// IDA_NAME: _ZN24DB_UpdateNPCRelationshipD2Ev
// DEMANGLED: DB_UpdateNPCRelationship::~DB_UpdateNPCRelationship()
// IDA_EA: 0x845D4B6
// IDA_NAME: _ZN24DB_UpdateNPCRelationshipD0Ev
// DEMANGLED: DB_UpdateNPCRelationship::~DB_UpdateNPCRelationship() deleting destructor
DB_UpdateNPCRelationship::~DB_UpdateNPCRelationship() {}

// IDA_EA: 0x8435E1E
// IDA_NAME: _ZN24DB_UpdateNPCRelationship8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateNPCRelationship::dispatch(int,int,Stream *)
bool DB_UpdateNPCRelationship::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned char payload[kSigUpdateNPCRelationshipSize];
    if (stream->get_binary(payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    int charac_no = DB_UPDATE_NPC_REL_I32(payload, 0);
    int npc_count = DB_UPDATE_NPC_REL_I32(payload, 4);
    char* npc_data = db_handle->blob_to_str(
        0,
        payload + 8,
        npc_count * kNPCRelationshipEntrySize);
    db_handle->set_query(
        "upDate charac_npc set npc_cnt=%d,npc_data='%s' where charac_no=%d",
        npc_count,
        npc_data,
        charac_no);
    db_handle->exec(true);
    return true;
}
