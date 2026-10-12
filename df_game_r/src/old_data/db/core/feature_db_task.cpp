#include "old_data/db/core/feature_db_task.h"

#include "common/manager/global_data.h"
#include "common/manager/os_api.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/character/db_charac_link.h"
#include "old_data/db/character/db_clone_title.h"
#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/core/db_shutdown_quiescence.h"
#include "old_data/db/event/db_dragon_timeattack_rank.h"
#include "old_data/db/event/db_decide_levelup_event.h"
#include "old_data/db/event/db_event_all_user_gift.h"
#include "old_data/db/event/db_event_attendance.h"
#include "old_data/db/event/db_event_account_fatigue_stat.h"
#include "old_data/db/event/db_facebook_like.h"
#include "old_data/db/event/db_fatigue_acceleration.h"
#include "old_data/db/event/db_king_of_pick.h"
#include "old_data/db/event/db_levelup_support_3rd.h"
#include "old_data/db/event/db_collect_box.h"
#include "old_data/db/event/db_satisfaction_survey.h"
#include "old_data/db/event/db_skill_damage_rank_event.h"
#include "old_data/db/event/db_take_magic_lamp.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_data/db/pvp_party/db_integrated_match_pvp_score.h"
#include "old_data/db/pvp_party/db_pvp_practice_combo_score.h"
#include "old_data/db/taiwan/taiwan_db_dispatcher.h"

#include <cstring>
#include <new>


FeatureDBTask::FeatureDBTask() : dispatchers_(), ready_(false) {
    for (int index = 0; index < kDispatcherCount; ++index) {
        dispatchers_[index] = 0;
    }
    ready_ = RegistDispatcher();
}

FeatureDBTask::~FeatureDBTask() {
    UnregistDispatcher();
}

bool FeatureDBTask::ready() const {
    return ready_;
}

bool FeatureDBTask::RegistDispatcher() {
    return RegisterDispatcher(
               clone_title_db::kDbSignal,
               new (std::nothrow) clone_title_db::DB_CloneTitle()) &&
           RegisterDispatcher(
               charac_link_db::kDbSignal,
               new (std::nothrow) charac_link_db::DB_CharacLink()) &&
           RegisterDispatcher(
               king_of_pick_db::kDbSignal,
               new (std::nothrow) king_of_pick_db::DB_KingOfPick()) &&
           RegisterDispatcher(
               facebook_like_db::kDbSignal,
               new (std::nothrow)
                   facebook_like_db::DB_FacebookLike()) &&
           RegisterDispatcher(
               event_attendance_db::kDbSignal,
               new (std::nothrow)
                   event_attendance_db::DB_EventAttendance()) &&
           RegisterDispatcher(
               event_all_user_gift::kDbSignal,
               new (std::nothrow)
                   event_all_user_gift::DB_EventAllUserGift()) &&
           RegisterDispatcher(
               magic_lamp::kDbSignal,
               new (std::nothrow) magic_lamp::DB_TakeMagicLamp()) &&
           RegisterDispatcher(
               pvp_practice_combo_score::kDbSignal,
               new (std::nothrow)
                   pvp_practice_combo_score::
                       DB_PvpPracticeComboScore()) &&
           RegisterDispatcher(
               levelup_support_3rd_db::kDbSignal,
               new (std::nothrow)
                   levelup_support_3rd_db::DB_LevelupSupport3rd()) &&
           RegisterDispatcher(
               dragon_timeattack_rank::kDbSignal,
               new (std::nothrow)
                   dragon_timeattack_rank::DB_DragonTimeAttackRank()) &&
           RegisterDispatcher(
               skill_damage_rank_event::kDbSignal,
               new (std::nothrow)
                   skill_damage_rank_event::DB_SkillDamageRankEvent()) &&
           RegisterDispatcher(
               satisfaction_survey::kDbSignal,
               new (std::nothrow)
                   satisfaction_survey::DB_SaveSatisfactionSurvey()) &&
           RegisterDispatcher(
               fatigue_acceleration_db::kDbSignal,
               new (std::nothrow)
                   fatigue_acceleration_db::DB_FatigueAcceleration()) &&
           RegisterDispatcher(
               event_account_fatigue_stat::kDbSignal,
               new (std::nothrow)
                   event_account_fatigue_stat::DB_EventAccountFatigueStat()) &&
           RegisterDispatcher(
               item_growth_db::kDbSignal,
               new (std::nothrow)
                   item_growth_db::DB_ItemGrowthState()) &&
           RegisterDispatcher(
               decide_levelup_db::kDbSignal,
               new (std::nothrow)
                   decide_levelup_db::DB_DecideLevelupEvent()) &&
           RegisterDispatcher(
               collect_box_db::kDbSignal,
               new (std::nothrow) collect_box_db::DB_CollectBox()) &&
           RegisterDispatcher(
               integrated_match_pvp_score_db::kDbSignal,
               new (std::nothrow)
                   integrated_match_pvp_score_db::
                       DB_IntegratedMatchPvpScore()) &&
           RegisterDispatcher(
               db_shutdown_quiescence::kNativeSignal,
               new (std::nothrow)
                   db_shutdown_quiescence::
                       DB_ShutdownQuiescence()) &&
           RegisterDispatcher(
               846,
               new (std::nothrow) DB_DispatcherTaiwan(
                   TaiwanDBDispatcher::INIT_MODE_FEATURE_DB));
}

bool FeatureDBTask::RegisterDispatcher(int signal,
                                          DBDispatcher* dispatcher) {
    if (!dispatcher || signal < 0 || signal >= kDispatcherCount ||
        dispatchers_[signal]) {
        delete dispatcher;
        return false;
    }
    dispatchers_[signal] = dispatcher;
    return true;
}

void FeatureDBTask::UnregistDispatcher() {
    for (int index = 0; index < kDispatcherCount; ++index) {
        delete dispatchers_[index];
        dispatchers_[index] = 0;
    }
}

int FeatureDBTask::HandleEvents(bool& had_event) {
    had_event = false;
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    Stream* const stream = queues
                               ? queues->get(MsgQueueMgr::FEATURE_DB_Q)
                               : 0;
    if (!stream) {
        OS_API::uSleep(100);
        return 1;
    }
    had_event = true;

    int signal = 0;
    const int stream_size = stream->length();
    const int pool_kind = stream->pool_kind();
    int activity_signal = 0;
    if (stream_size >= static_cast<int>(sizeof(activity_signal))) {
        std::memcpy(&activity_signal,
                    stream->get_buffer(),
                    sizeof(activity_signal));
    }
    db_shutdown_quiescence::BeginActivity(
        db_shutdown_quiescence::kActivityNativeDb,
        activity_signal);
    DBProcess(stream, signal);
    db_shutdown_quiescence::EndActivity(
        db_shutdown_quiescence::kActivityNativeDb);
    FreeStream(stream, stream_size, pool_kind, signal);
    return 1;
}

int FeatureDBTask::DBProcess(Stream* stream, int& signal) {
    signal = 0;
    if (!stream || (*stream >> signal) != 1) {
        return 0;
    }
    if (signal < 0 || signal >= kDispatcherCount) {
        LogManager::logFormat(1,
                              "Native0828DBTask.cpp",
                              "Native0828DBTask::DBProcess",
                              __LINE__,
                              "Invalid native 0828 DB signal(%d)",
                              signal);
        return 0;
    }

    int user_idx = 0;
    if ((*stream >> user_idx) != 1) {
        return 0;
    }

    DBDispatcher* const dispatcher = dispatchers_[signal];
    if (!dispatcher) {
        LogManager::logFormat(1,
                              "Native0828DBTask.cpp",
                              "Native0828DBTask::DBProcess",
                              __LINE__,
                              "Can't find native 0828 DB dispatcher: signal(%d)",
                              signal);
        return 0;
    }

    // Deliberately do not call DBMgr::setUserIdx().  Every native message
    // carries its server group, and DBMgr's TLS path performs an explicit,
    // non-mutating lookup of the dedicated connection.
    return dispatcher->dispatch(signal, user_idx, stream) ? 1 : 0;
}

void FeatureDBTask::FreeStream(Stream* stream,
                                  int stream_size,
                                  int pool_kind,
                                  int signal) {
    if (!stream) {
        return;
    }
    if (pool_kind == 1) {
        if (stream_size > 4095) {
            LogManager::logFormat(1,
                                  "Native0828DBTask.cpp",
                                  "Native0828DBTask::FreeStream",
                                  __LINE__,
                                  "Small stream too large: signal(%d), size(%d)",
                                  signal,
                                  stream_size);
        }
        if (GlobalData::small_stream_pool()) {
            GlobalData::small_stream_pool()->Free(stream);
        }
    } else if (pool_kind == 2) {
        if (stream_size > 0x3ffff) {
            LogManager::logFormat(1,
                                  "Native0828DBTask.cpp",
                                  "Native0828DBTask::FreeStream",
                                  __LINE__,
                                  "Big stream too large: signal(%d), size(%d)",
                                  signal,
                                  stream_size);
        }
        if (GlobalData::big_stream_pool()) {
            GlobalData::big_stream_pool()->Free(stream);
        }
    } else if (pool_kind == 0) {
        if (stream_size > 0x3fff) {
            LogManager::logFormat(1,
                                  "Native0828DBTask.cpp",
                                  "Native0828DBTask::FreeStream",
                                  __LINE__,
                                  "Default stream too large: signal(%d), size(%d)",
                                  signal,
                                  stream_size);
        }
        if (GlobalData::stream_pool()) {
            GlobalData::stream_pool()->Free(stream);
        }
    }
}
