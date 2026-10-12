#include "old_game/activity/schedule/advance_altar_runtime.h"

#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/player/core/character_messages.h"
#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>

namespace advancealtar {

// IDA_EA: 0x8133784
// IDA_NAME: _ZN12advancealtar23DBAndExchangeServerProc22requestLoadFromDBAndESER5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: advancealtar::DBAndExchangeServerProc::requestLoadFromDBAndES(CUser &,MSG_CHARAC_SELECT &)
int DBAndExchangeServerProc::requestLoadFromDBAndES(
    CUser& user,
    MSG_CHARAC_SELECT& select) {
    user.advance_altar_manager_.reset();

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire(
            "localglobal/global_AdvanceAltar.cpp", 3130),
        true);
    *guard << 25;
    *guard << user.GetUID();
    user.DisableCharacInfo(static_cast<CACHE_SECTION>(10));

    if (user.IsValidLastLoginCharac()) {
        *guard << user.GetLastLoginChannelNo();
        *guard << user.GetLastLoginCharacNo();
    }

    // A owns CStreamGuard.  This direct template call deliberately retains
    // the original IDA symbol instead of the retired C-side global wrapper.
    _CharacAdvanceAltarDbData* const payload =
        guard.GetInBuffer<_CharacAdvanceAltarDbData>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user.get_charac_no(select.character_slot));

    GlobalData::msgq_mgr()->put(
        user.IsValidLastLoginCharac()
            ? static_cast<MsgQueueMgr::QUEUE_IDX>(5)
            : MsgQueueMgr::DB_Q,
        guard);
    return 1;
}

// IDA_EA: 0x813394C
// IDA_NAME: _ZN12advancealtar23DBAndExchangeServerProc13getFromMemoryERK5CUserRNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::DBAndExchangeServerProc::getFromMemory(CUser const&,advancealtar::_CharacAdvanceAltarDbData &)
void DBAndExchangeServerProc::getFromMemory(
    const CUser& user,
    _CharacAdvanceAltarDbData& data) {
    data.charac_no = static_cast<uint32_t>(user.getCurCharacNo());
    user.advance_altar_manager_.getCharacAdvanceAltarDbData(data);
    if (user.advance_altar_manager_.isUpdated()) {
        DB_Update::makeRequest(user.GetUID(), data);
    }
}

}  // namespace advancealtar
