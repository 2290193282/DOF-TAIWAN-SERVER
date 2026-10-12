#include "old_game/player/core/cuser.h"

#include "common/manager/global_tools.h"
#include "infra/network/transport/ip_address_split_abi.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/db/anti_cheat/db_hack_ip_save.h"
#include "old_data/db/anti_cheat/db_insert_auto_punish_first_user.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstdio>
#include <cstring>


// IDA_EA: 0x8685D9C
// IDA_NAME: _ZN15cUserHistoryLog21IPCounterLog_InsertIpEiiPc
// DEMANGLED: cUserHistoryLog::IPCounterLog_InsertIp(int,int,char *)
int cUserHistoryLog::IPCounterLog_InsertIp(int hack_type,
                                           int hack_sub_type,
                                           char* peer_ip) {
    return owner_->LogHistory("IPCountInt,%d,%d,%s",
                              hack_type,
                              hack_sub_type,
                              peer_ip);
}

// IDA_EA: 0x8681368
// IDA_NAME: _ZN5CUser12OnPunishUserEv
// DEMANGLED: CUser::OnPunishUser(void)
char CUser::OnPunishUser() {
    if (!getCurCharacR()) {
        return 0;
    }

    CInventory* const inventory = getCurCharacInvenR();
    const int money = inventory->get_money();
    return static_cast<char>(
        GetInstanceCirculationStatistic()->AddCirculationStatistic(
            static_cast<CIRCULATION_STATISTIC_FIELD>(28),
            this,
            static_cast<unsigned int>(money)));
}

// IDA_EA: 0x8681312
// IDA_NAME: _ZN5CUser15OnRestrictTradeEv
// DEMANGLED: CUser::OnRestrictTrade(void)
char CUser::OnRestrictTrade() {
    if (!getCurCharacR()) {
        return 0;
    }

    CInventory* const inventory = getCurCharacInvenR();
    const int money = inventory->get_money();
    return static_cast<char>(
        GetInstanceCirculationStatistic()->AddCirculationStatistic(
            static_cast<CIRCULATION_STATISTIC_FIELD>(29),
            this,
            static_cast<unsigned int>(money)));
}

// IDA_EA: 0x867F748
// IDA_NAME: _ZN5CUser11setHackUserEjjjjtj
// DEMANGLED: CUser::setHackUser(unsigned int,unsigned int,unsigned int,unsigned int,unsigned short,unsigned int)
void CUser::setHackUser(unsigned int hack_type,
                        unsigned int accumulated_count,
                        unsigned int hack_sub_type,
                        unsigned int sub_accumulated_count,
                        unsigned short apply_flag,
                        unsigned int etc_low) {
    unsigned short stored_apply_flag = apply_flag;

    const char* const account_id_text =
        NumberToString(static_cast<unsigned int>(get_acc_id()), 0);
    cMyTrace trace(__PRETTY_FUNCTION__, 32341, 0);
    trace("m_id : %s, SetHackUser : %d(%d)\t  %d(%d)   ApplyFlag = %d\n",
          account_id_text,
          hack_type,
          accumulated_count,
          hack_sub_type,
          sub_accumulated_count,
          static_cast<unsigned int>(apply_flag));

    switch (apply_flag) {
    case 2:
        lookouts_.push_back(stLookOuts(hack_type, hack_sub_type));
        break;

    case 3:
        if (hack_ip_counter_saved_ != 1u) {
            char peer_ip[16];
            std::memset(peer_ip, 0, sizeof(peer_ip));
            network_.GetPeerIP2(peer_ip, sizeof(peer_ip));

            char c_class_ip[16];
            std::memset(c_class_ip, 0, sizeof(c_class_ip));
            Divide_C_ClassIP_EndIP(c_class_ip, 0, peer_ip);

            ip_counters_.push_back(tyIPCounters(hack_type, hack_sub_type));
            hack_ip_counter_saved_ = 1u;
            history_log_.IPCounterLog_InsertIp(
                static_cast<int>(hack_type),
                static_cast<int>(hack_sub_type),
                peer_ip);
            DB_HackIPSave::makeRequest(c_class_ip,
                                       peer_ip,
                                       hack_type,
                                       hack_sub_type);
        }
        return;

    case 10: {
        lookout_history_.push_back(stLookOuts(hack_type, hack_sub_type));
        if (isTradePunishType(4)) {
            return;
        }

        const int delay_seconds =
            static_cast<int>(get_rand_int(290)) + 10;
        std::printf("ARS TRADE PUNISH USER - Rand Sec : %d\n",
                    delay_seconds);
        const unsigned int unique_id =
            static_cast<unsigned short>(get_unique_id());
        const int uid = GetUID();
        TimerQueue* const timer_queue = G_TimerQueue();
        timer_queue->InsertTimer(
            0, uid, 151, delay_seconds, unique_id, -1);
        break;
    }

    default: {
        int delay_seconds = static_cast<int>(get_rand_int(290)) + 10;
        hack_user_timer_ = 1u;
        if (apply_flag == 8u) {
            ars_user_kick_ = 1u;
            delay_seconds = 1;
        }
        if (apply_flag == 7u) {
            ars_user_kick_ = 1u;
        }
        if (apply_flag == 9u) {
            ars_user_kick_ = 1u;
        }
        if (apply_flag == 12u) {
            ars_user_kick_ = 1u;
        }
        if (apply_flag == 5u) {
            ars_user_kick_ = 1u;
        }

        const unsigned int unique_id =
            static_cast<unsigned short>(get_unique_id());
        const int uid = GetUID();
        TimerQueue* const timer_queue = G_TimerQueue();
        timer_queue->InsertTimer(
            0, uid, 129, delay_seconds, unique_id, -1);
        OnPunishUser();

        if (pu_user_ && GetTradePunishType()) {
            return;
        }
        if (pu_user_ &&
            (apply_flag == 1u || apply_flag == 4u || apply_flag == 9u)) {
            stored_apply_flag = 10u;
        }
        break;
    }
    }

    char peer_ip[16];
    std::memset(peer_ip, 0, sizeof(peer_ip));
    char* const request_ip = network_.GetPeerIP2(peer_ip, sizeof(peer_ip));
    DB_InsertAutoPunishFirstUser::makeRequest(
        static_cast<unsigned int>(get_acc_id()),
        request_ip,
        hack_type,
        accumulated_count,
        hack_sub_type,
        sub_accumulated_count,
        stored_apply_flag,
        etc_low);
}
