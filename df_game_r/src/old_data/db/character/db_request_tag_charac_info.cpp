#include "old_data/db/character/db_request_tag_charac_info.h"

#include "old_data/db/character/db_load_charac.h"
#include "old_data/db/character/db_load_etc.h"
#include "old_data/db/inventory/db_load_inventory.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/global_data.h"

#include <string.h>


namespace {

enum {
    kRequestTagCharacInfoInterSignal = 766,
    kRequestTagCharacInfoLineCharac = 45595,
    kRequestTagCharacInfoLineInven = 45603,
    kRequestTagCharacInfoLineAvatar = 45612,
    kRequestTagCharacInfoLineQuestShop = 45620,
    kRequestTagCharacInfoLineSend = 45631,
};

}  // namespace

// IDA_EA: 0x844FCC6
// IDA_NAME: _ZN23DB_RequestTagCharacInfoC2Ev
// DEMANGLED: DB_RequestTagCharacInfo::DB_RequestTagCharacInfo(void)
DB_RequestTagCharacInfo::DB_RequestTagCharacInfo() {}

// IDA_EA: 0x845B0E0
// IDA_NAME: _ZN23DB_RequestTagCharacInfoD2Ev
// DEMANGLED: DB_RequestTagCharacInfo::~DB_RequestTagCharacInfo()
// IDA_EA: 0x845B110
// IDA_NAME: _ZN23DB_RequestTagCharacInfoD0Ev
// DEMANGLED: DB_RequestTagCharacInfo::~DB_RequestTagCharacInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_RequestTagCharacInfo::~DB_RequestTagCharacInfo() {}

// IDA_EA: 0x844768C
// IDA_NAME: _ZN23DB_RequestTagCharacInfo8dispatchEiiP6Stream
// DEMANGLED: DB_RequestTagCharacInfo::dispatch(int,int,Stream *)
bool DB_RequestTagCharacInfo::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;

    SIG_REQUEST_TAG_CHARAC_INFO* payload =
        stream->GetOutBuffer<SIG_REQUEST_TAG_CHARAC_INFO>();
    if (!payload) {
        return false;
    }

    const unsigned int charac_no = payload->charac_no;
    if (DB_LoadCharac::_GetTagCharacInfo(
            charac_no, &payload->tag_charac_data) != true) {
        cMyTrace trace(
            "virtual bool DB_RequestTagCharacInfo::dispatch(int, int, Stream*)",
            kRequestTagCharacInfoLineCharac,
            5);
        trace("DB_RequestTagChracInfo::_GetTagCharacInfo fail, characNo(%d)",
              charac_no);
        return false;
    }
    if (DB_LoadInventory::_GetTagCharacInven(
            charac_no, &payload->inventory) != true) {
        cMyTrace trace(
            "virtual bool DB_RequestTagCharacInfo::dispatch(int, int, Stream*)",
            kRequestTagCharacInfoLineInven,
            5);
        trace("DB_RequestTagChracInfo::_GetTagCharacInven fail, characNo(%d)",
              charac_no);
        return false;
    }
    if (DB_LoadInventory::_GetUserAvatar(
            charac_no,
            payload->inventory.avatar_detail,
            payload->inventory.avatar_equipment,
            payload->inventory.avatar_inventory,
            payload->inventory.avatar_count) != true) {
        cMyTrace trace(
            "virtual bool DB_RequestTagCharacInfo::dispatch(int, int, Stream*)",
            kRequestTagCharacInfoLineAvatar,
            5);
        trace("DB_RequestTagCharacInfo::_GetUserAvatar fail, characNo(%d)",
              charac_no);
        return false;
    }
    if (DB_LoadEtc::_LoadQuestShop(charac_no, &payload->quest_shop) != true) {
        cMyTrace trace(
            "virtual bool DB_RequestTagCharacInfo::dispatch(int, int, Stream*)",
            kRequestTagCharacInfoLineQuestShop,
            5);
        trace("DB_RequestTagCharacInfo::_LoadQuestShop fail, characNo(%d)",
              charac_no);
        return false;
    }

    sendResult(user_idx, payload);
    return true;
}

// IDA_EA: 0x84478A6
// IDA_NAME: _ZN23DB_RequestTagCharacInfo10sendResultEiP27SIG_REQUEST_TAG_CHARAC_INFO
// DEMANGLED: DB_RequestTagCharacInfo::sendResult(int,SIG_REQUEST_TAG_CHARAC_INFO *)
void DB_RequestTagCharacInfo::sendResult(int user_idx,
                                         SIG_REQUEST_TAG_CHARAC_INFO* payload_data) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        kRequestTagCharacInfoLineSend);
    CStreamGuard guard(stream, true);
    *guard << kRequestTagCharacInfoInterSignal;
    *guard << user_idx;
    SIG_REQUEST_TAG_CHARAC_INFO* dest =
        guard->GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>();
    if (dest) {
        memcpy(dest, payload_data, sizeof(SIG_REQUEST_TAG_CHARAC_INFO));
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}
