#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"

#include <cstring>
#include <new>
#include <utility>
#include <vector>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_parameter_script_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/fair_pvp_score_stream.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

ServerParameterScript* ServerParameterAt(CDataManager* data_manager) {
    // CDataManager::C2 @0x8355B30 constructs this exact embedded object at
    // +0x68.  The B-owned accessor exposes that same object without reopening
    // the manager layout in this C-owned score path.
    return data_manager->get_server_parameter_script();
}

uint16_t TrainingSealBonusCountAt(CDataManager* data_manager) {
    // CFairPvPScore::loadData @0x84A4E85 reads [training seal bonus count],
    // which is not ServerParameterScript::GetDailyMatchCount(+0xf8).
    return data_manager->get_server_parameter_script()
        ->get_training_seal_bonus_count();
}

inline __attribute__((always_inline))
void MakeFairPvPItem(const CItem* prototype, Inven_Item& item) {
    // The old CItem vptr has D2/D0 in slots 0/1 and make_item in slot 2
    // (_ZNK14CStackableItem9make_itemER10Inven_Item or the CEquipItem
    // override).  Keep the dispatch indirect: virtual-call lowering through
    // CItem itself is not available from its public ABI facade.
    const CItemVTable* vtable = 0;
    std::memcpy(&vtable, prototype, sizeof(vtable));
    vtable->make_item(prototype, item);
}

void SendTrainingRewardInventoryUpdate(CUser* user,
                                       const std::vector<int>& slots) {
    if (slots.size() <= 2u) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 0x0e);
    packet.put_byte(0);
    int count_index = packet.get_index();
    int item_count = 0;
    packet.put_short(0);
    for (size_t index = 0; index < slots.size(); index += 3u) {
        if (user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(1), slots[index], packet)) {
            ++item_count;
        }
    }
    packet.put_short(count_index, item_count);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

namespace fair_pvp {

// IDA_EA: 0x8234F64
// IDA_NAME: _ZNK8fair_pvp13CFairPvPScore11GetGiveItemEv
// DEMANGLED: fair_pvp::CFairPvPScore::GetGiveItem(void)const
int CFairPvPScore::GetGiveItem() const {
    return give_item_;
}

// IDA_EA: 0x8234F74
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore11SetGiveItemEi
// DEMANGLED: fair_pvp::CFairPvPScore::SetGiveItem(int)
void CFairPvPScore::SetGiveItem(int give_item) {
    give_item_ = static_cast<uint8_t>(give_item);
    alter();
}

// IDA_EA: 0x8234F92
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore17SetDailyPlayCountEi
// DEMANGLED: fair_pvp::CFairPvPScore::SetDailyPlayCount(int)
void CFairPvPScore::SetDailyPlayCount(int play_count) {
    if (play_count < 0) {
        play_count = 0;
    }
    daily_play_count_ = play_count;
}

// IDA_EA: 0x8234FAE
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore19SetRemainBonusCountEi
// DEMANGLED: fair_pvp::CFairPvPScore::SetRemainBonusCount(int)
void CFairPvPScore::SetRemainBonusCount(int remain_count) {
    remain_bonus_count_ = static_cast<uint8_t>(remain_count);
}

// IDA_EA: 0x84A47DC
// IDA_NAME: _ZN8fair_pvp13CFairPvPScoreC2Ev
// DEMANGLED: fair_pvp::CFairPvPScore::CFairPvPScore(void)
CFairPvPScore::CFairPvPScore() {}

// IDA_EA: 0x84A4850
// IDA_NAME: _ZN8fair_pvp13CFairPvPScoreD2Ev
// DEMANGLED: fair_pvp::CFairPvPScore::~CFairPvPScore()
// IDA_EA: 0x84A48DA
// IDA_NAME: _ZN8fair_pvp13CFairPvPScoreD0Ev
// DEMANGLED: fair_pvp::CFairPvPScore::~CFairPvPScore()
CFairPvPScore::~CFairPvPScore() {}

// IDA_EA: 0x84A48F8
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore8loadDataEP5CUserPc
// DEMANGLED: fair_pvp::CFairPvPScore::loadData(CUser *,char *)
bool CFairPvPScore::loadData(CUser* user, char* data) {
    const SIG_FAIR_PVP_SCORE& payload =
        *reinterpret_cast<const SIG_FAIR_PVP_SCORE*>(data);

    for (unsigned int index = 0;
         index < payload.job_score.count && index <= 99u;
         ++index) {
        const SIG_FAIR_PVP_SCORE::JobScoreRecord& source =
            payload.job_score.records[index];
        const JobKey key(static_cast<ENUM_CHARACTERJOB>(source.job),
                         source.grow_type);
        const std::pair<JobScoreMap::iterator, bool> inserted =
            job_score_.insert(std::make_pair(key, source.score));
        if (!inserted.second) {
            LogManager::logFormat(
                1,
                "FairPvPScore.cpp",
                "virtual bool fair_pvp::CFairPvPScore::loadData(CUser*, char*)",
                0x20,
                "job_score_.insert error(%d,%d)",
                source.job,
                source.grow_type);
        }
    }

    for (unsigned int index = 0;
         index <= 9u && payload.job_score.recent_results[index] != 0u;
         ++index) {
        recent_results_.push_back(payload.job_score.recent_results[index]);
    }

    for (unsigned int mode = 0; mode <= 1u; ++mode) {
        mode_totals_[mode] = payload.mode_totals[mode];
    }
    relay_two_kill_ = static_cast<int>(payload.relay_two_kill);
    relay_three_kill_ = static_cast<int>(payload.relay_three_kill);
    successive_win_ = static_cast<int>(payload.successive_win);
    maximum_successive_win_ =
        static_cast<int>(payload.maximum_successive_win);
    daily_play_count_ = static_cast<int>(payload.daily_play_count);
    last_play_time_ = payload.last_play_time;

    if ((G_GameWorld()->GetChannelType() == 15 ||
         G_GameWorld()->GetChannelType() == 16) &&
        payload.last_play_time == 0) {
        std::vector<std::pair<int, int> > rewards;
        const int seal_count =
            ServerParameterAt(G_CDataManager())
                ->GetTraningSealGive(user->getPVPResultRefR()->values[5]);
        if (seal_count > 0) {
            rewards.push_back(std::make_pair(3312, seal_count));
        }
        ServerParameterAt(G_CDataManager())
            ->GetTraningFirstGive(user->get_charac_job(),
                                  user->getCurCharFirstGrowType(),
                                  user->get_pvp_grade(),
                                  rewards);
        if (!rewards.empty()) {
            std::vector<int> inserted_slots;
            const int inserted = user->getCurCharacInvenW()->insert_event_items(
                rewards,
                inserted_slots,
                static_cast<eMoneyAddReason>(3),
                static_cast<eItemAddReason>(8),
                true,
                "game_server_msg_171",
                "game_server_msg_172");
            if (inserted > 0 && inserted_slots.size() > 2u) {
                SendTrainingRewardInventoryUpdate(user, inserted_slots);
            }
        }
        last_play_time_ = GlobalData::s_systemTime_.getCurSec();
        alter();
    }

    const int daily_limit = TrainingSealBonusCountAt(G_CDataManager());
    if (static_cast<unsigned int>(daily_limit) >
        static_cast<unsigned int>(daily_play_count_)) {
        remain_bonus_count_ = static_cast<uint8_t>(
            daily_limit - daily_play_count_);
    } else {
        remain_bonus_count_ = 0;
    }

    PacketGuard packet;
    packet.put_header(0, 0x152);
    packet.put_short(remain_bonus_count_);
    packet.finalize(true);
    user->Send(packet);

    give_item_ = payload.give_item;
    if (G_GameWorld()->GetChannelType() == 15 ||
        G_GameWorld()->GetChannelType() == 16) {
        if (give_item_ <=
            ServerParameterAt(G_CDataManager())
                ->get_pvp_item_give(user->get_pvp_grade())) {
            packet.clear();
            packet.put_header(0, 0x161);
            packet.finalize(true);
            user->Send(packet);
        }
    }

    if (last_play_time_ == 0 ||
        static_cast<uint32_t>(last_play_time_) > 0x4DDD89EFu) {
        return 1;
    }

    int legacy_score =
        2 * mode_totals_[0].win +
        mode_totals_[0].lose + mode_totals_[0].draw +
        3 * mode_totals_[1].win +
        mode_totals_[1].lose + mode_totals_[1].draw;

    int match_count = mode_totals_[0].win +
                      mode_totals_[0].lose +
                      mode_totals_[0].draw;
    if (match_count > 9 && match_count <= 49) {
        legacy_score += 10;
    }
    if (match_count > 49) {
        legacy_score += 60;
    }
    if (static_cast<unsigned int>(mode_totals_[0].win) > 9u &&
        static_cast<unsigned int>(mode_totals_[0].win) <= 49u) {
        legacy_score += 15;
    }
    if (static_cast<unsigned int>(mode_totals_[0].win) > 49u) {
        legacy_score += 90;
    }

    match_count = mode_totals_[1].win +
                  mode_totals_[1].lose +
                  mode_totals_[1].draw;
    if (match_count > 9 && match_count <= 49) {
        legacy_score += 10;
    }
    if (match_count > 49) {
        legacy_score += 60;
    }
    if (static_cast<unsigned int>(mode_totals_[1].win) > 9u &&
        static_cast<unsigned int>(mode_totals_[1].win) <= 49u) {
        legacy_score += 15;
    }
    if (static_cast<unsigned int>(mode_totals_[1].win) > 49u) {
        legacy_score += 90;
    }
    legacy_score += 2 * mode_totals_[0].win;
    legacy_score += 2 * mode_totals_[1].win;

    // The old body constructs one Inven_Item and intentionally has no D2
    // call on any exit path.  Constructor-only storage preserves that exact
    // lifetime topology.
    unsigned char item_storage[sizeof(Inven_Item)];
    Inven_Item* const item = new (item_storage) Inven_Item;

    item->item_id_ = 3312;
    CItem* prototype = G_CDataManager()->find_item(3312);
    if (!prototype) {
        return 0;
    }
    item->add_info_ = legacy_score + 200;
    MakeFairPvPItem(prototype, *item);

    char title[21] = {};
    char text[256] = {};
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_171", 0),
                 0x14u);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_177", 0),
                 0xFFu);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        *item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        true,
        false);

    item->item_id_ = 2660232;
    prototype = G_CDataManager()->find_item(2660232);
    if (!prototype) {
        return 0;
    }
    item->add_info_ = 1;
    MakeFairPvPItem(prototype, *item);

    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_171", 0),
                 0x14u);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_178", 0),
                 0xFFu);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        *item,
        0,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        true,
        false);
    last_play_time_ = GlobalData::s_systemTime_.getCurSec();
    alter();
    return 1;
}

// IDA_EA: 0x84A55A8
// IDA_NAME: _ZNK8fair_pvp13CFairPvPScore7getDataEPc
// DEMANGLED: fair_pvp::CFairPvPScore::getData(char *)const
void CFairPvPScore::getData(char* data) const {
    SIG_FAIR_PVP_SCORE& payload =
        *reinterpret_cast<SIG_FAIR_PVP_SCORE*>(data);

    unsigned int index = 0;
    for (JobScoreMap::const_iterator it = job_score_.begin();
         it != job_score_.end() && index <= 99u;
         ++it, ++index) {
        SIG_FAIR_PVP_SCORE::JobScoreRecord& destination =
            payload.job_score.records[index];
        destination.job = static_cast<uint8_t>(it->first.job);
        destination.grow_type = it->first.grow_type;
        destination.score = it->second;
        ++payload.job_score.count;
    }

    index = 0;
    for (std::list<uint8_t>::const_iterator it = recent_results_.begin();
         it != recent_results_.end() && index <= 9u;
         ++it, ++index) {
        payload.job_score.recent_results[index] = *it;
    }

    for (unsigned int mode = 0; mode <= 1u; ++mode) {
        payload.mode_totals[mode] = mode_totals_[mode];
    }
    payload.relay_two_kill = static_cast<uint32_t>(relay_two_kill_);
    payload.relay_three_kill = static_cast<uint32_t>(relay_three_kill_);
    payload.successive_win = static_cast<uint32_t>(successive_win_);
    payload.maximum_successive_win =
        static_cast<uint32_t>(maximum_successive_win_);
    payload.daily_play_count = static_cast<uint32_t>(daily_play_count_);
    payload.last_play_time = last_play_time_;
    payload.give_item = give_item_;
}

// IDA_EA: 0x84A57FE
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore9_saveDataEP5CUser
// DEMANGLED: fair_pvp::CFairPvPScore::_saveData(CUser *)
int CFairPvPScore::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "FairPvPScore.cpp", 0x10a);
    CStreamGuard guard(stream, true);
    *guard << 0x261;
    *guard << user->GetUID();
    SIG_FAIR_PVP_SCORE* const payload =
        guard->GetInBuffer<SIG_FAIR_PVP_SCORE>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    getData(reinterpret_cast<char*>(payload));
    payload->last_play_time = GlobalData::s_systemTime_.getCurSec();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x84A5942
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore6_resetEv
// DEMANGLED: fair_pvp::CFairPvPScore::_reset(void)
int CFairPvPScore::_reset() {
    job_score_.clear();
    recent_results_.clear();
    for (unsigned int mode = 0; mode <= 1u; ++mode) {
        mode_totals_[mode].win = 0;
        mode_totals_[mode].lose = 0;
        mode_totals_[mode].draw = 0;
    }
    relay_two_kill_ = 0;
    relay_three_kill_ = 0;
    successive_win_ = 0;
    maximum_successive_win_ = 0;
    daily_play_count_ = 0;
    last_play_time_ = 0;
    last_training_seal_count_ = 0;
    last_training_seal_bonus_count_ = 0;
    remain_bonus_count_ = 0;
    give_item_ = 0;
    return reinterpret_cast<int>(this);
}

// IDA_EA: 0x84A5A28
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore11UpdateScoreEiiiiiii
// DEMANGLED: fair_pvp::CFairPvPScore::UpdateScore(int,int,int,int,int,int,int)
void CFairPvPScore::UpdateScore(int job,
                               int grow_type,
                               int mode,
                               int win,
                               int lose,
                               int draw,
                               int relay_kill_count) {
    if (mode == 1) {
        const JobKey key(static_cast<ENUM_CHARACTERJOB>(job),
                         static_cast<uint8_t>(grow_type));
        JobScoreMap::iterator it = job_score_.find(key);
        if (it != job_score_.end()) {
            it->second.win += win;
            it->second.lose += lose;
            it->second.draw += draw;
        } else {
            FAIR_PVP_SCORE_MODE_COUNT score = {win, lose, draw};
            job_score_.insert(std::make_pair(key, score));
        }
        mode_totals_[0].win += win;
        mode_totals_[0].lose += lose;
        mode_totals_[0].draw += draw;
    } else {
        if (relay_kill_count == 2) {
            ++relay_two_kill_;
        } else if (relay_kill_count == 3) {
            ++relay_three_kill_;
        }
        mode_totals_[1].win += win;
        mode_totals_[1].lose += lose;
        mode_totals_[1].draw += draw;
    }

    if (win) {
        recent_results_.push_back(1);
    } else if (lose) {
        recent_results_.push_back(2);
    } else if (draw) {
        recent_results_.push_back(3);
    }
    if (recent_results_.size() > 10u) {
        recent_results_.pop_front();
    }

    if (lose) {
        successive_win_ = 0;
    } else if (win) {
        ++successive_win_;
        if (maximum_successive_win_ < successive_win_) {
            maximum_successive_win_ = successive_win_;
        }
    }
    ++daily_play_count_;
    alter();
}

// IDA_EA: 0x84A5CC2
// IDA_NAME: _ZNK8fair_pvp13CFairPvPScore15MakePacketSocreER11PacketGuardi
// DEMANGLED: fair_pvp::CFairPvPScore::MakePacketSocre(PacketGuard &,int)const
void CFairPvPScore::MakePacketSocre(PacketGuard& packet,
                                    int detail_mode) const {
    packet.put_byte(detail_mode);
    packet.put_int(mode_totals_[0].win);
    packet.put_int(mode_totals_[1].win);

    int recent_win = 0;
    int recent_lose = 0;
    int recent_draw = 0;
    for (std::list<uint8_t>::const_iterator it = recent_results_.begin();
         it != recent_results_.end();
         ++it) {
        if (*it == 1u) {
            ++recent_win;
        } else if (*it == 2u) {
            ++recent_lose;
        } else if (*it == 3u) {
            ++recent_draw;
        }
    }
    packet.put_byte(recent_win);
    packet.put_byte(recent_lose);
    packet.put_byte(recent_draw);
    packet.put_int(successive_win_);
    packet.put_int(relay_two_kill_);
    packet.put_int(relay_three_kill_);
    packet.put_int(maximum_successive_win_);

    if (detail_mode != 1) {
        return;
    }
    packet.put_int(mode_totals_[0].lose);
    packet.put_int(mode_totals_[0].draw);
    packet.put_int(mode_totals_[1].lose);
    packet.put_int(mode_totals_[1].draw);
    packet.put_byte(static_cast<int>(job_score_.size()));
    for (JobScoreMap::const_iterator it = job_score_.begin();
         it != job_score_.end();
         ++it) {
        packet.put_byte(it->first.job);
        packet.put_byte(it->first.grow_type);
        packet.put_int(it->second.win);
        packet.put_int(it->second.lose);
        packet.put_int(it->second.draw);
    }
}

// IDA_EA: 0x84A645A
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore10ResetDailyEv
// DEMANGLED: fair_pvp::CFairPvPScore::ResetDaily(void)
void CFairPvPScore::ResetDaily() {
    daily_play_count_ = 0;
}

// IDA_EA: 0x8568EC2
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore17GetDailyPlayCountEv
// DEMANGLED: fair_pvp::CFairPvPScore::GetDailyPlayCount(void)
int CFairPvPScore::GetDailyPlayCount() {
    return daily_play_count_;
}

// IDA_EA: 0x8568ECE
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore24SetLastTrainingSealCountEi
// DEMANGLED: fair_pvp::CFairPvPScore::SetLastTrainingSealCount(int)
void CFairPvPScore::SetLastTrainingSealCount(int count) {
    last_training_seal_count_ = static_cast<uint8_t>(count);
}

// IDA_EA: 0x8568EDE
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore29SetLastTrainingSealBonusCountEi
// DEMANGLED: fair_pvp::CFairPvPScore::SetLastTrainingSealBonusCount(int)
void CFairPvPScore::SetLastTrainingSealBonusCount(int count) {
    last_training_seal_bonus_count_ = static_cast<uint8_t>(count);
}

// IDA_EA: 0x85DFDAC
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore24GetLastTrainingSealCountEv
// DEMANGLED: fair_pvp::CFairPvPScore::GetLastTrainingSealCount(void)
int CFairPvPScore::GetLastTrainingSealCount() {
    return last_training_seal_count_;
}

// IDA_EA: 0x85DFDBC
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore29GetLastTrainingSealBonusCountEv
// DEMANGLED: fair_pvp::CFairPvPScore::GetLastTrainingSealBonusCount(void)
int CFairPvPScore::GetLastTrainingSealBonusCount() {
    return last_training_seal_bonus_count_;
}

// IDA_EA: 0x85DFDCC
// IDA_NAME: _ZN8fair_pvp13CFairPvPScore19GetRemainBonusCountEv
// DEMANGLED: fair_pvp::CFairPvPScore::GetRemainBonusCount(void)
int CFairPvPScore::GetRemainBonusCount() {
    return remain_bonus_count_;
}

}  // namespace fair_pvp
