#include "old_data/db/taiwan/taiwan_db_dispatcher.h"

#include "infra/log/format/trace.h"
#include "old_data/db/event/db_create_dnf_user_info.h"
#include "old_data/db/event/db_hero_mission_event_data.h"
#include "old_data/db/event/db_insert_give_box_event_reward.h"
#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_data/db/event/db_select_mobile_auth_reward_tw.h"
#include "old_data/db/event/db_select_create_dnf_event_info.h"
#include "old_data/db/event/db_exp_keeping_event.h"
#include "old_data/db/event/db_freely_give_item.h"
#include "old_data/db/event/db_dual_raid_reward.h"
#include "old_data/db/event/db_racing_dungeon.h"
#include "old_data/db/event/db_golden_commerce.h"
#include "old_data/db/event/db_zombie_mode_result.h"
#include "old_data/db/event/db_glass_bottle_event.h"
#include "old_data/db/event/db_blue_horse_event.h"
#include "old_data/db/event/db_sugar_time.h"
#include "old_data/db/event/db_shooting_star_dimension_pocket.h"
#include "old_data/db/event/db_sea_bottom_event.h"
#include "old_data/db/pvp_party/db_matching_channel_handoff.h"
#include "old_data/db/pvp_party/db_pvp_tournament.h"
#include "old_data/db/event/taiwan_staytime_dispatchers.h"
#include "old_data/db/taiwan/taiwan_restrict_dispatchers.h"
#include "infra/network/packet/stream.h"

#include <algorithm>

namespace {

template <typename T>
bool AddTaiwanDispatcher(TaiwanDBDispatcher* owner) {
    T* const dispatcher = new T();
    if (!dispatcher) {
        return false;
    }
    if (owner->addDispatcher(
            static_cast<TaiwanInternalPack::T>(dispatcher->getCode()),
            dispatcher)) {
        return true;
    }
    delete dispatcher;
    return false;
}

}  // namespace


// IDA_EA: 0x815DFCE
// IDA_NAME: _ZN18TaiwanDBDispatcher8FnDeleteclERSt4pairIKN18TaiwanInternalPack1TEP27TaiwanInterfaceDBDispatcherE
// DEMANGLED: TaiwanDBDispatcher::FnDelete::operator()(std::pair<TaiwanInternalPack::T const, TaiwanInterfaceDBDispatcher *> &)
void TaiwanDBDispatcher::FnDelete::operator()(
    TaiwanDBDispatcher::DispatcherMap::value_type& entry) {
    delete entry.second;
}

// IDA_EA: 0x815A134
// IDA_NAME: _ZN18TaiwanDBDispatcherC2Ev
// DEMANGLED: TaiwanDBDispatcher::TaiwanDBDispatcher(void)
TaiwanDBDispatcher::TaiwanDBDispatcher() : dispatchers_() {
    dispatchers_.clear();
}

// IDA_EA: 0x815A176
// IDA_NAME: _ZN18TaiwanDBDispatcherD2Ev
// DEMANGLED: TaiwanDBDispatcher::~TaiwanDBDispatcher()
TaiwanDBDispatcher::~TaiwanDBDispatcher() {
    std::for_each(dispatchers_.begin(), dispatchers_.end(), FnDelete());
}

// IDA_EA: 0x815A1F8
// IDA_NAME: _ZN18TaiwanDBDispatcher4initEv
// DEMANGLED: TaiwanDBDispatcher::init(void)
bool TaiwanDBDispatcher::init() {
    return init(INIT_MODE_LEGACY);
}

bool TaiwanDBDispatcher::init(InitMode mode) {
    if (mode == INIT_MODE_FEATURE_DB) {
        // The native worker owns a separate aggregate and separate concrete
        // dispatcher instances.  Register every implemented 0828 inner code;
        // producer migration remains deliberately more conservative below.
        return AddTaiwanDispatcher<exp_keeping::DBExpKeepingEvent>(this) &&
               AddTaiwanDispatcher<freely_give_item_db::DBFreelyGiveItem>(this) &&
               AddTaiwanDispatcher<pvp_tournament::DBPvpTournament>(this) &&
               AddTaiwanDispatcher<dual_raid::DBDualRaidReward>(this) &&
               AddTaiwanDispatcher<racing_dungeon::DBRacingDungeon>(this) &&
               AddTaiwanDispatcher<matching_channel_handoff_db::DBMatchingChannelHandoff>(this) &&
               AddTaiwanDispatcher<zombie_mode::DBZombieModeResult>(this) &&
               AddTaiwanDispatcher<golden_commerce::DBGoldenCommerceReward>(this) &&
               AddTaiwanDispatcher<glass_bottle_event_db::DBGlassBottleEvent>(this) &&
               AddTaiwanDispatcher<blue_horse_event_db::DBBlueHorseEvent>(this) &&
               AddTaiwanDispatcher<sugar_time_db::DBSugarTime>(this) &&
               AddTaiwanDispatcher<shooting_star_dimension_pocket_db::DBShootingStarDimensionPocket>(this) &&
               AddTaiwanDispatcher<sea_bottom_event_db::DBSeaBottomEvent>(this);
    }
    if (mode != INIT_MODE_LEGACY) {
        return false;
    }

    // Codes 1..18 are the canonical legacy map.  During the queue split the
    // All reconstructed 0828 owners now live only in the dedicated native
    // aggregate.  The legacy map remains the canonical codes 1..18 and never
    // competes with the queue-6 worker for an inner-code owner.
    return AddTaiwanDispatcher<LoadServiceRestrictPolicy>(this) &&
           AddTaiwanDispatcher<SaveServiceRestrictPolicy>(this) &&
           AddTaiwanDispatcher<LoadGeolocation>(this) &&
           AddTaiwanDispatcher<CheckStayTimeEvent>(this) &&
           AddTaiwanDispatcher<ResetStayTimeEvent>(this) &&
           AddTaiwanDispatcher<DBUpdateExitTime>(this) &&
           AddTaiwanDispatcher<DBSelectCreateDnfEventInfo>(this) &&
           AddTaiwanDispatcher<DBSelectCreateDnfUserInfo>(this) &&
           AddTaiwanDispatcher<DBInsertCreateDnfUserInfo>(this) &&
           AddTaiwanDispatcher<DBSelectMobileAuthRewardTw>(this) &&
           AddTaiwanDispatcher<DBSelectPcroomDailyReward>(this) &&
           AddTaiwanDispatcher<DBUpdatePcroomDailyReward>(this) &&
           AddTaiwanDispatcher<DBInsertGiveBoxEventReward>(this) &&
           AddTaiwanDispatcher<DBLoadHeroMissionEventData>(this) &&
           AddTaiwanDispatcher<DBSaveHeroMissionEventData>(this);
}

// IDA_EA: 0x815A7F6
// IDA_NAME: _ZN18TaiwanDBDispatcher13getDispatcherEN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanDBDispatcher::getDispatcher(TaiwanInternalPack::T)
TaiwanInterfaceDBDispatcher* TaiwanDBDispatcher::getDispatcher(
    TaiwanInternalPack::T code) {
    DispatcherMap::iterator it = dispatchers_.find(code);
    return it == dispatchers_.end() ? 0 : it->second;
}

// IDA_EA: 0x815A85A
// IDA_NAME: _ZN18TaiwanDBDispatcher13addDispatcherEN18TaiwanInternalPack1TEP27TaiwanInterfaceDBDispatcher
// DEMANGLED: TaiwanDBDispatcher::addDispatcher(TaiwanInternalPack::T, TaiwanInterfaceDBDispatcher *)
bool TaiwanDBDispatcher::addDispatcher(TaiwanInternalPack::T code,
                                        TaiwanInterfaceDBDispatcher* dispatcher) {
    if (getDispatcher(code) != 0) {
        return false;
    }
    dispatchers_[code] = dispatcher;
    return true;
}

// IDA_EA: 0x815A8A0
// IDA_NAME: _ZN18TaiwanDBDispatcher13delDispatcherEN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanDBDispatcher::delDispatcher(TaiwanInternalPack::T)
bool TaiwanDBDispatcher::delDispatcher(TaiwanInternalPack::T code) {
    DispatcherMap::iterator it = dispatchers_.find(code);
    if (it == dispatchers_.end()) {
        return false;
    }
    dispatchers_.erase(it);
    return true;
}

// IDA_EA: 0x844C9D4
// IDA_NAME: _ZN19DB_DispatcherTaiwanC2Ev
// DEMANGLED: DB_DispatcherTaiwan::DB_DispatcherTaiwan(void)
DB_DispatcherTaiwan::DB_DispatcherTaiwan()
    : DBDispatcher(), dispatcher_() {
    (void)dispatcher_.init();
}

DB_DispatcherTaiwan::DB_DispatcherTaiwan(
    TaiwanDBDispatcher::InitMode mode)
    : DBDispatcher(), dispatcher_() {
    (void)dispatcher_.init(mode);
}

// IDA_EA: 0x845A7F4
// IDA_NAME: _ZN19DB_DispatcherTaiwanD2Ev
// DEMANGLED: DB_DispatcherTaiwan::~DB_DispatcherTaiwan()
// IDA_EA: 0x845A856
// IDA_NAME: _ZN19DB_DispatcherTaiwanD0Ev
// DEMANGLED: DB_DispatcherTaiwan::~DB_DispatcherTaiwan()
DB_DispatcherTaiwan::~DB_DispatcherTaiwan() {
}

// IDA_EA: 0x844CA48
// IDA_NAME: _ZN19DB_DispatcherTaiwan8dispatchEiiP6Stream
// DEMANGLED: DB_DispatcherTaiwan::dispatch(int,int,Stream *)
bool DB_DispatcherTaiwan::dispatch(int signal, int user_idx, Stream* stream) {
    int code = 0;
    (*stream) >> code;
    TaiwanInterfaceDBDispatcher* const dispatcher =
        dispatcher_.getDispatcher(static_cast<TaiwanInternalPack::T>(code));
    if (dispatcher) {
        return dispatcher->dispatch(signal, user_idx, stream) != 0;
    }

    cMyTrace trace("virtual bool DB_DispatcherTaiwan::dispatch(int, int, Stream*)",
                   49530,
                   5);
    trace("[TaiwanDispater] Invalid internal packet code. Code:%d, Uid:%d",
          code,
          user_idx);
    return false;
}
