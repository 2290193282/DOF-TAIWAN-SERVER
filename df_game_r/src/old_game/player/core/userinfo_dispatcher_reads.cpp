#include "old_game/player/core/userinfo_dispatcher_reads.h"

#include <cstring>

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace Taiwan {
// IDA_EA: 0x8163950
// IDA_NAME: _ZN6Taiwan15internal_streamER12CStreamGuardN18TaiwanInternalPack1TEi
int internal_stream(CStreamGuard& guard, int code, int src);
}

// IDA_EA: 0x8161012
// IDA_NAME: _ZN28InterSelectCreateDnfUserInfo12dispatch_sigEP5CUserPci
// DEMANGLED: InterSelectCreateDnfUserInfo::dispatch_sig(CUser *,char *,int)
int InterSelectCreateDnfUserInfo::dispatch_sig(CUser* user,
                                               char* msg,
                                               int length) {
    (void)length;
    if (!msg) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectCreateDnfUserInfo::dispatch_sig(CUser*, char*, int)",
            302,
            "[Taiwan, Event] interMsg is null.");
        return 303;
    }

    CEventCreateDnf* repeat_event =
        GlobalData::s_event_manager
            ? static_cast<CEventCreateDnf*>(
                  GlobalData::s_event_manager->GetRepeatEvent(160))
            : 0;
    const int eventing = repeat_event ? repeat_event->IsEventing(0) : 0;
    if (!repeat_event || eventing != 1) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectCreateDnfUserInfo::dispatch_sig(CUser*, char*, int)",
            312,
            "[Taiwan, Event] stayEvent=%s eventing:%d",
            repeat_event ? "ok" : "nullptr",
            eventing);
        return 313;
    }

    Msg* inter_msg = reinterpret_cast<Msg*>(msg);
    unsigned char rewardable = 0;
    if (inter_msg->received != 1 && repeat_event->IsRewardCondition()) {
        rewardable = 1;
    }

    user->SetEventCreateDnfReward(inter_msg->received == 1);
    PacketGuard guard;
    guard.put_header(0, ENUM_NOTIPACKET_EVENT_CREATE_DNF);
    guard.put_short(repeat_event->GetRate());
    guard.put_byte(rewardable);
    guard.finalize(true);
    user->Send(guard);
    return 0;
}

// IDA_EA: 0x8161218
// IDA_NAME: _ZN28InterSelectCreateDnfUserInfo11makeRequestEijb
// DEMANGLED: InterSelectCreateDnfUserInfo::makeRequest(int,unsigned int,bool)
void InterSelectCreateDnfUserInfo::makeRequest(int uid,
                                               unsigned int account_id,
                                               bool received) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
        333);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 10, uid);
    Msg* msg = guard.GetInBuffer<Msg>();
    if (msg) {
        msg->account_id = account_id;
        msg->received = received ? 1 : 0;
        msg->reserved_05[0] = 0;
        msg->reserved_05[1] = 0;
        msg->reserved_05[2] = 0;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "static void InterSelectCreateDnfUserInfo::makeRequest(int, memberIdentificationNumber_t, bool)",
            338,
            "[Taiwan, Event] m_id:%d isReceived:%d",
            account_id,
            received ? 1 : 0);
    }
}

// IDA_EA: 0x816208C
// IDA_NAME: _ZN28InterSelectCreateDnfUserInfoC2Ev
// DEMANGLED: InterSelectCreateDnfUserInfo::InterSelectCreateDnfUserInfo(void)
InterSelectCreateDnfUserInfo::InterSelectCreateDnfUserInfo()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::SELECT_CREATE_DNF_USER_INFO) {}

// IDA_EA: 0x81620B0
// IDA_NAME: _ZN28InterSelectCreateDnfUserInfoD2Ev
// DEMANGLED: InterSelectCreateDnfUserInfo::~InterSelectCreateDnfUserInfo()
// IDA_EA: 0x81620E0
// IDA_NAME: _ZN28InterSelectCreateDnfUserInfoD0Ev
// DEMANGLED: InterSelectCreateDnfUserInfo::~InterSelectCreateDnfUserInfo()
InterSelectCreateDnfUserInfo::~InterSelectCreateDnfUserInfo() {}
