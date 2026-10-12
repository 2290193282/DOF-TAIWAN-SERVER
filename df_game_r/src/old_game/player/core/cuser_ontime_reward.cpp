#include "old_game/player/core/cuser.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x868C170
// IDA_NAME: _ZN5CUser31send_ontime_reward_start_notifyEv
// DEMANGLED: CUser::send_ontime_reward_start_notify(void)
int CUser::send_ontime_reward_start_notify() {
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(51);
    if (!repeat_event) {
        cMyTrace trace("bool CUser::send_ontime_reward_start_notify()",
                       42999,
                       5);
        trace("[ON_TIME_EVENT] send_ontime_reward_start_notify() error");
        return 0;
    }
    if (repeat_event->IsEventing(0) != 1) {
        return 0;
    }

    COnTimeEvent* const ontime_event =
        static_cast<COnTimeEvent*>(repeat_event);
    if (ontime_event->get_reward_state()) {
        return 0;
    }

    char log_prefix[256];
    std::memset(log_prefix, 0, sizeof(log_prefix));
    std::sprintf(log_prefix,
                 "./log/%s/OnTimeEvent",
                 G_CEnvironment()->get_file_name());
    const int charac_no = get_charac_no(-1);
    const char* const account_id =
        NumberToString(static_cast<unsigned int>(get_acc_id()), 0);
    CMyFileLog log("bool CUser::send_ontime_reward_start_notify()", 43011);
    log(log_prefix,
        "On Time Event : On Charac Login(%s, %u)",
        account_id,
        charac_no);

    const int ontime_id = ontime_event->get_ontime_id();
    if (ontime_id > get_ontime_last_recv_idx()) {
        const std::time_t now = std::time(0);
        const int reward_end_time = ontime_event->get_reward_end_time();
        const int remaining_seconds = reward_end_time - static_cast<int>(now);
        if (remaining_seconds < 0) {
            return 0;
        }

        PacketGuard packet;
        packet.put_header(0, 261);
        packet.put_int(remaining_seconds);
        packet.finalize(true);
        Send(packet);
    }
    return 1;
}

// IDA_EA: 0x868C3BC
// IDA_NAME: _ZN5CUser25delete_ontime_reward_listEv
// DEMANGLED: CUser::delete_ontime_reward_list(void)
int CUser::delete_ontime_reward_list() {
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(51);
    if (repeat_event) {
        if (repeat_event->IsEventing(0) == 1) {
            static_cast<COnTimeEvent*>(repeat_event)->delete_reward_user(this);
            return 1;
        }
        return 0;
    }

    cMyTrace trace("bool CUser::delete_ontime_reward_list()", 43048, 5);
    trace("[ON_TIME_EVENT]delete_ontime_reward_list() error");
    return 0;
}

// IDA_EA: 0x81BC11E
// IDA_NAME: _ZN5CUser24set_ontime_last_recv_idxEi
// DEMANGLED: CUser::set_ontime_last_recv_idx(int)
CUser* CUser::set_ontime_last_recv_idx(int ontime_id) {
    ontime_last_recv_idx_ = ontime_id;
    return this;
}

// IDA_EA: 0x81BC130
// IDA_NAME: _ZNK5CUser24get_ontime_last_recv_idxEv
// DEMANGLED: CUser::get_ontime_last_recv_idx(void)const
int CUser::get_ontime_last_recv_idx() const {
    return ontime_last_recv_idx_;
}

// IDA_EA: 0x81BC13E
// IDA_NAME: _ZN5CUser31set_update_ontime_last_recv_idxEv
// DEMANGLED: CUser::set_update_ontime_last_recv_idx(void)
CUser* CUser::set_update_ontime_last_recv_idx() {
    update_ontime_last_recv_idx_ = 1;
    return this;
}
