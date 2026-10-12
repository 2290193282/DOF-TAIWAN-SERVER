#include "common/manager/in_game_advertisement_inter_receivers.h"

#include "common/manager/global_data.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x84C8C4E
// IDA_NAME: _ZN34Inter_WebNoticeIngameAdvertisement12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_WebNoticeIngameAdvertisement::dispatch_sig(CUser *,char *,int)
int Inter_WebNoticeIngameAdvertisement::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;

    Stream* stream = GlobalData::stream_pool()->Acquire("InterDispatcher.cpp",
                                                         8823);
    CStreamGuard guard(stream, true);
    // The raw assembly calls CStreamGuard::operator<<(int) with 760.  The
    // 248 shown by the decompiler is only the low byte of 0x2f8.
    *guard << 760;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x84EA2EC
// IDA_NAME: _ZN34Inter_WebNoticeIngameAdvertisementC2Ev
// DEMANGLED: Inter_WebNoticeIngameAdvertisement::Inter_WebNoticeIngameAdvertisement(void)
Inter_WebNoticeIngameAdvertisement::Inter_WebNoticeIngameAdvertisement() {
}

// IDA_EA: 0x84EA308
// IDA_NAME: _ZN34Inter_WebNoticeIngameAdvertisementD2Ev
// DEMANGLED: Inter_WebNoticeIngameAdvertisement::~Inter_WebNoticeIngameAdvertisement()
// IDA_EA: 0x84EA338
// IDA_NAME: _ZN34Inter_WebNoticeIngameAdvertisementD0Ev
// DEMANGLED: Inter_WebNoticeIngameAdvertisement::~Inter_WebNoticeIngameAdvertisement()
Inter_WebNoticeIngameAdvertisement::~Inter_WebNoticeIngameAdvertisement() {
}


// IDA_EA: 0x84C8D10
// IDA_NAME: _ZN31Inter_ReloadIngameAdvertisement12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReloadIngameAdvertisement::dispatch_sig(CUser *,char *,int)
int Inter_ReloadIngameAdvertisement::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    (void)user;
    (void)payload_size;

    SIG_RELOAD_IN_GAME_ADVERTISEMENT* const reload =
        reinterpret_cast<SIG_RELOAD_IN_GAME_ADVERTISEMENT*>(payload);
    InGameADManagerInstance()->InitADExposeInfo();

    int index = 0;
    for (; index <= 199 &&
                    reload->entries[index].notify.banner_id !=
                        static_cast<uint32_t>(-1);
         ++index) {
        InGameADManagerInstance()->InsertADExposeInfo(reload->entries[index]);
    }

    cMyTrace trace(
        "virtual int Inter_ReloadIngameAdvertisement::dispatch_sig(CUser*, char*, int)",
        8848, 0);
    trace("In Game Advertisement condition Reload OK, count(%d)", index);
    return 0;
}

// IDA_EA: 0x84EA356
// IDA_NAME: _ZN31Inter_ReloadIngameAdvertisementC2Ev
// DEMANGLED: Inter_ReloadIngameAdvertisement::Inter_ReloadIngameAdvertisement(void)
Inter_ReloadIngameAdvertisement::Inter_ReloadIngameAdvertisement() {
}

// IDA_EA: 0x84EA372
// IDA_NAME: _ZN31Inter_ReloadIngameAdvertisementD2Ev
// DEMANGLED: Inter_ReloadIngameAdvertisement::~Inter_ReloadIngameAdvertisement()
// IDA_EA: 0x84EA3A2
// IDA_NAME: _ZN31Inter_ReloadIngameAdvertisementD0Ev
// DEMANGLED: Inter_ReloadIngameAdvertisement::~Inter_ReloadIngameAdvertisement()
Inter_ReloadIngameAdvertisement::~Inter_ReloadIngameAdvertisement() {
}
