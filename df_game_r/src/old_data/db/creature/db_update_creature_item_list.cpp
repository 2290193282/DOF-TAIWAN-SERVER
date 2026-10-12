#include "old_data/db/creature/db_update_creature_item_list.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



// IDA_EA: 0x844E366
// IDA_NAME: _ZN25DB_UpdateCreatureItemListC2Ev
// DEMANGLED: DB_UpdateCreatureItemList::DB_UpdateCreatureItemList(void)
DB_UpdateCreatureItemList::DB_UpdateCreatureItemList() {}

// IDA_EA: 0x845F3E8
// IDA_NAME: _ZN25DB_UpdateCreatureItemListD2Ev
// DEMANGLED: DB_UpdateCreatureItemList::~DB_UpdateCreatureItemList()
// IDA_EA: 0x845F418
// IDA_NAME: _ZN25DB_UpdateCreatureItemListD0Ev
// DEMANGLED: DB_UpdateCreatureItemList::~DB_UpdateCreatureItemList() deleting destructor
DB_UpdateCreatureItemList::~DB_UpdateCreatureItemList() {}

// IDA_EA: 0x841FD12
// IDA_NAME: _ZN25DB_UpdateCreatureItemList8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCreatureItemList::dispatch(int,int,Stream *)
bool DB_UpdateCreatureItemList::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_CREATURE_ITEM_LIST* const payload =
        stream->GetOutBuffer<SIG_CREATURE_ITEM_LIST>();
    if (!payload) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    const int count = static_cast<int>(payload->count);
    for (int index = 0; count > index; ++index) {
        const SIG_CREATURE_ITEM_DETAIL_INFO& creature =
            payload->details[index];
        db_handle->set_query(
            "upDate creature_items set exp=%d,stomach=%d where ui_id=%d",
            creature.experience,
            creature.stomach,
            creature.ui_id);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

