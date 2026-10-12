#include "old_game/pvp/battlefield/power_war_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/db/taiwan/inter_slots_357_382.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x847EBEC
// IDA_NAME: _ZN13CPowerManager26SetPowerWarUserRankingInfoEPj
// DEMANGLED: CPowerManager::SetPowerWarUserRankingInfo(unsigned int *)
bool CPowerManager::SetPowerWarUserRankingInfo(unsigned int* charac_no) {
    for (unsigned int index = 0; index <= 2; ++index) {
        STPowerWarStatueRankerInfo& ranker = ranker_[index];
        if (charac_no[index]) {
            ranker.charac_no = charac_no[index];

            CStreamGuard guard(
                GlobalData::s_stream_pool->Acquire("DNF_PowerManager.cpp",
                                                    125),
                true);
            guard << 358;
            guard << -1;
            guard << static_cast<int>(ranker.charac_no);
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
        } else {
            ranker.load_complete = 1;
        }
    }
    // IDA evaluates (n2 <= 2) once more for n2 == 3 before returning, so
    // this historical bool return is always false after the three updates.
    return false;
}

// IDA_EA: 0x847ED4E
// IDA_NAME: _ZN13CPowerManager25SetPowerWarUserStatueInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFO
// DEMANGLED: CPowerManager::SetPowerWarUserStatueInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *)
void CPowerManager::SetPowerWarUserStatueInfo(
    SIG_LOAD_POWER_WAR_STATUE_INFO* info) {
    for (unsigned int index = 0; index <= 2; ++index) {
        STPowerWarStatueRankerInfo& ranker = ranker_[index];
        if (ranker.charac_no == info->charac_no) {
            ranker.job = info->job;
            ranker.level = info->level;
            ranker.grow_type = info->grow_type;
            ranker.guild_id = info->guild_id;
            std::strncpy(ranker.charac_name, info->charac_name,
                         sizeof(ranker.charac_name));
            std::strncpy(ranker.guild_name, info->guild_name,
                         sizeof(ranker.guild_name));
            std::memcpy(ranker.equip_item_id, info->equip_item_id,
                        sizeof(ranker.equip_item_id));
            ranker.load_complete = 1;

            cMyTrace trace(
                "void CPowerManager::SetPowerWarUserStatueInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*)",
                164, 0);
            trace(
                "Set Statue Ranker %s(%u), job:%d, lev:%d, grow:%d, guild_name:%s, guild_id:%u, EQUP [head:%d][hair:%d][face:%d][jacket:%d][pants:%d][shoes:%d][breast:%d][waist:%d][skin:%d][weapon:%d]",
                ranker.charac_name, ranker.charac_no, ranker.job,
                ranker.level, ranker.grow_type, ranker.guild_name,
                ranker.guild_id, ranker.equip_item_id[0],
                ranker.equip_item_id[1], ranker.equip_item_id[2],
                ranker.equip_item_id[3], ranker.equip_item_id[4],
                ranker.equip_item_id[5], ranker.equip_item_id[6],
                ranker.equip_item_id[7], ranker.equip_item_id[8],
                ranker.equip_item_id[10]);
            break;
        }
    }

    if (CheckCompleteDBLoadStatueInfo()) {
        SendPowerWarUserStatueInfo(0);
    }
}

// IDA_EA: 0x847F1B0
// IDA_NAME: _ZN13CPowerManager29CheckCompleteDBLoadStatueInfoEv
// DEMANGLED: CPowerManager::CheckCompleteDBLoadStatueInfo(void)
int CPowerManager::CheckCompleteDBLoadStatueInfo() {
    for (unsigned int index = 0; index <= 2; ++index) {
        if (ranker_[index].load_complete != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x847F200
// IDA_NAME: _ZN13CPowerManager26SendPowerWarUserStatueInfoEP5CUser
// DEMANGLED: CPowerManager::SendPowerWarUserStatueInfo(CUser *)
void CPowerManager::SendPowerWarUserStatueInfo(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 182);

    int ranker_count = 0;
    for (unsigned int index = 0; index <= 2; ++index) {
        if (ranker_[index].charac_no) {
            ++ranker_count;
        }
    }
    packet.put_byte(static_cast<char>(ranker_count));

    for (int index = 0; index < ranker_count; ++index) {
        STPowerWarStatueRankerInfo& ranker = ranker_[index];
        const int charac_name_len = static_cast<int>(std::strlen(ranker.charac_name));
        packet.put_int(charac_name_len);
        packet.put_str(ranker.charac_name, charac_name_len);
        packet.put_byte(static_cast<char>(ranker.job));
        packet.put_byte(static_cast<char>(ranker.level));
        packet.put_byte(static_cast<char>(ranker.grow_type));
        const int guild_name_len = static_cast<int>(std::strlen(ranker.guild_name));
        packet.put_int(guild_name_len);
        packet.put_str(ranker.guild_name, guild_name_len);
        packet.put_int(static_cast<int>(ranker.guild_id));
        for (int equip_index = 0; equip_index <= 10; ++equip_index) {
            if (ranker.equip_item_id[equip_index] && equip_index != 9) {
                packet.put_int(ranker.equip_item_id[equip_index]);
            } else {
                packet.put_int(-1);
            }
        }
    }

    packet.finalize(true);
    if (user) {
        user->Send(packet);
    } else {
        G_GameWorld()->send_all(packet);
    }
}
