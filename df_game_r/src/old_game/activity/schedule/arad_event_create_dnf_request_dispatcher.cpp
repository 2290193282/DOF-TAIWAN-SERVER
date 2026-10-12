#include "old_game/activity/schedule/arad_event_create_dnf_request_dispatcher.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/event/db_create_dnf_user_info.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


namespace ARAD {
namespace DISPATCHER {

namespace {

const char kSourceFile[] = "localjapan/Arad_PacketDispatcher.cpp";
const char kDispatchSignature[] =
    "virtual int ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::"
    "dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8597F4E
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::Dispatcher_EventCreateDnfRequest(void)
Dispatcher_EventCreateDnfRequest::Dispatcher_EventCreateDnfRequest()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0C1C
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::~Dispatcher_EventCreateDnfRequest()
// IDA_EA: 0x81A0C4C
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::~Dispatcher_EventCreateDnfRequest()
Dispatcher_EventCreateDnfRequest::~Dispatcher_EventCreateDnfRequest() {}

// IDA_EA: 0x81A0362
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_EventCreateDnfRequest::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    (void)packet;

    if (user == 0) {
        return 1971;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4) {
        return 0;
    }

    CEventCreateDnf* const repeat_event = static_cast<CEventCreateDnf*>(
        GlobalData::s_event_manager->GetRepeatEvent(160));
    // Old 0x81A0421 reaches the logging path by invoking IsEventing through a
    // null RepeatEvent.  Keep every non-null branch and return/log result,
    // but do not deliberately reproduce that crash in the rebuilt server.
    const int eventing = repeat_event ? repeat_event->IsEventing(0) : 0;
    if (!repeat_event || eventing != 1) {
        LogManager::logFormat(
            1,
            kSourceFile,
            kDispatchSignature,
            1985,
            "[Taiwan, Event] stayEvent=%s eventing:%d",
            repeat_event ? "ok" : "nullptr",
            eventing);
        return 0;
    }

    if (static_cast<unsigned char>(user->GetEventCreateDnfReward()) != 1u &&
        static_cast<unsigned char>(repeat_event->IsRewardCondition()) != 0u) {
        user->SetEventCreateDnfReward(true);
        DBInsertCreateDnfUserInfo::makeRequest(
            user->GetUID(),
            static_cast<unsigned int>(user->get_acc_id()));
        repeat_event->SendReward(user);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EVENT_CREATE_DNF_REQUEST, 1);
        LogManager::logFormat(
            1,
            kSourceFile,
            kDispatchSignature,
            2006,
            "[Taiwan, Event] condition error m_id:%d",
            user->get_acc_id());
    }

    PacketGuard guard;
    guard.put_header(0, ENUM_NOTIPACKET_EVENT_CREATE_DNF);
    guard.put_short(repeat_event->GetRate());
    guard.put_byte(0);
    guard.finalize(true);
    user->Send(guard);
    return 0;
}

// IDA_EA: 0x81A0B72
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EventCreateDnfRequest::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    // The old body is exactly `xor eax,eax; ret`: CMD600 has no generic
    // MSG_BASE payload fields for the state-0 template to consume.
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B7C
// IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EventCreateDnfRequest::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    // The old body is exactly `xor eax,eax; ret`; the business path is the
    // concrete dispatch_sig override above.
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
