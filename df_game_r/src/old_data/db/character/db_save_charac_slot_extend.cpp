#include "old_data/db/character/db_save_charac_slot_extend.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



// IDA_EA: 0x844F89E
// IDA_NAME: _ZN23DB_SaveCharacSlotExtendC2Ev
// DEMANGLED: DB_SaveCharacSlotExtend::DB_SaveCharacSlotExtend(void)
DB_SaveCharacSlotExtend::DB_SaveCharacSlotExtend() {}

// IDA_EA: 0x845BAA0
// IDA_NAME: _ZN23DB_SaveCharacSlotExtendD2Ev
// DEMANGLED: DB_SaveCharacSlotExtend::~DB_SaveCharacSlotExtend()
// IDA_EA: 0x845BAD0
// IDA_NAME: _ZN23DB_SaveCharacSlotExtendD0Ev
// DEMANGLED: DB_SaveCharacSlotExtend::~DB_SaveCharacSlotExtend() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveCharacSlotExtend::~DB_SaveCharacSlotExtend() {}

// IDA_EA: 0x84420EA
// IDA_NAME: _ZN23DB_SaveCharacSlotExtend8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCharacSlotExtend::dispatch(int,int,Stream *)
bool DB_SaveCharacSlotExtend::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    SIG_EXTEND_CHARAC_SLOT* const payload =
        stream->GetOutBuffer<SIG_EXTEND_CHARAC_SLOT>();

    const char* account_id = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "upDate charac_view set slot_effect_count=%u, charac_slot_limit=%u where m_id=%s",
        payload->slot_effect_count,
        payload->charac_slot_limit,
        account_id);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_SaveCharacSlotExtend::dispatch(int, int, Stream*)",
        41461,
        5);
    trace(
        "upDate charac_view set slot_effect_count(%u), charac_slot_limit(%u) failed at m_id(%u)",
        payload->slot_effect_count,
        payload->charac_slot_limit,
        payload->account_id);
    return false;
}
