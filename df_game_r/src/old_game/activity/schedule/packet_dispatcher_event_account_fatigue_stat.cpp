#include "old_game/activity/schedule/packet_dispatcher_event_account_fatigue_stat.h"

#include "old_data/db/event/db_event_account_fatigue_stat.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>


namespace {

// CLIENT_IDA_EVIDENCE: DNF.exe sub_6041A0 reads u32 current and u32 target
// only after the standard success byte.  Its callback sub_604280 renders the
// values with "%d/%d" in UI/Event/AccountFatigueEvent.  The 0828 UI text
// explicitly defines the account-wide accumulated-fatigue target as 777.
const uint32_t kRequiredAccountFatigue = 777u;

void SendAccountFatigueStat(CUser* user, uint32_t fatigue_quantity) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_EVENT_ACCOUNT_FATIGUE_STAT);
    response.put_byte(1);
    response.put_int(static_cast<int>(fatigue_quantity));
    response.put_int(static_cast<int>(kRequiredAccountFatigue));
    response.finalize(true);
    user->Send(response);
}

void SendAccountFatigueError(CUser* user) {
    user->SendCmdErrorPacket(
        ENUM_CMDPACKET_EVENT_ACCOUNT_FATIGUE_STAT,
        event_account_fatigue_stat::kErrorOperationFailed);
}

}  // namespace


Dispatcher_EventAccountFatigueStat::Dispatcher_EventAccountFatigueStat()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_EventAccountFatigueStat::~Dispatcher_EventAccountFatigueStat() {}

int Dispatcher_EventAccountFatigueStat::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_6041E0 calls BeginCmd(710) and immediately
    // finalizes the packet, with no writer primitive in between.
    (void)packet;

    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    const int account_id = user->get_acc_id();
    if (account_id <= 0 ||
        !event_account_fatigue_stat::DB_EventAccountFatigueStat::makeRequest(
            user->GetUID(), static_cast<uint32_t>(account_id))) {
        SendAccountFatigueError(user);
        return 0;
    }
    return 0;
}

int Dispatcher_EventAccountFatigueStat::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}


Inter_EventAccountFatigueStat::Inter_EventAccountFatigueStat()
    : ISigDispatcher() {}

Inter_EventAccountFatigueStat::~Inter_EventAccountFatigueStat() {}

int Inter_EventAccountFatigueStat::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int payload_size) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    typedef event_account_fatigue_stat::Result Result;
    if (!payload || payload_size != static_cast<int>(sizeof(Result))) {
        SendAccountFatigueError(user);
        return 0;
    }

    const Result* const result = reinterpret_cast<const Result*>(payload);
    const int account_id = user->get_acc_id();
    // GetUID already prevents a stale reply from being routed to a reused
    // user slot.  Retain the account-id check as a second trust boundary for
    // this account-scoped statistic.
    if (account_id <= 0 ||
        result->account_id != static_cast<uint32_t>(account_id)) {
        return 0;
    }

    if (result->result != 0) {
        SendAccountFatigueError(user);
        return 0;
    }

    SendAccountFatigueStat(user, result->fatigue_quantity);
    return 0;
}

int Dispatcher_EventAccountFatigueStat::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
