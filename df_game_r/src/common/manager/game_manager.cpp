#include "common/manager/game_manager.h"
#include "common/manager/history_log_abi.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/blue_marble_script.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/activity/condition/condition_event_manager_runtime.h"
#include "old_game/social/party/quick_party_runtime.h"
#include "old_game/social/party/quick_party_system_manager_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/tournament/pvp_tournament.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mall/crane_minigame_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/cuser_global_info_handle.h"
#include "old_data/channel/log_game_channel.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/transport/epoll_handler.h"
#include "infra/concurrent/thread/guard_mutex.h"
#include "core/lifecycle/shutdown/shutdown_manager.h"

#include <ext/hash_map>
#include <cstdio>
#include <new>
#include <stdint.h>


namespace {

typedef __gnu_cxx::hash_map<int, CParty*> PartyMap;
typedef __gnu_cxx::hash_map<int, PvP_Room*> PvpMap;
typedef __gnu_cxx::hash_map<int, WarRoom*> WarRoomMap;
typedef __gnu_cxx::hash_map<int, BlueMarble*> BlueMarbleMap;
typedef __gnu_cxx::hash_map<int, WongWork::CDeathTower*> DeathTowerMap;
typedef __gnu_cxx::hash_map<int, WongWork::CBossTower*> BossTowerMap;
typedef std::map<unsigned int, CUser*> UserAccountMap;

}  // namespace

class CDungeon;
struct CDungeonClearTracer;
class CDataManager;

CGameManager* G_CGameManager() __asm__("_Z14G_CGameManagerv");
CDataManager* G_CDataManager() __asm__("_Z14G_CDataManagerv");
// IDA_EA: 0x82930C2
// IDA_NAME: _ZN12CGameManagerC2Ev
// DEMANGLED: CGameManager::CGameManager(void)
CGameManager::CGameManager()
    : Mutex(),
      next_uid_(0),
      test_accounts_(),
      test_ips_(),
      user_pool_(),
      trade_space_pool_(),
      party_pool_(),
      pvp_pool_(),
      war_room_pool_(),
      death_tower_pool_(),
      boss_tower_pool_(),
      advance_altar_pool_(),
      quick_party_pool_(),
      blue_marble_pool_(),
      users_by_account_(),
      users_by_charac_(),
      pvp_rooms_(),
      parties_(),
      war_rooms_(),
      death_towers_(),
      boss_towers_(),
      advance_altars_(),
      quick_parties_(),
      blue_marbles_(),
      quick_party_system_manager_(0),
      quick_party_reward_manager_(0),
      premium_lethe_manager_(0),
      shared_server_message_manager_(0),
      condition_event_manager_(0),
      special_item_routing_manager_(0),
      auction_average_price_manager_(0),
      crane_minigame_manager_(0) {
    static const char* const kTestAccounts[] = {
        "testman820", "htilil", "azzuman", "cuwaki", "slainer",
        "mrsbscom", "arandra", "krucef", "dnftest126", "dnftest127",
        "dnftest128"};
    static const char* const kTestIps[] = {
        "59.10.138.244", "218.145.141.85", "121.134.32.42"};
    for (size_t index = 0;
         index < sizeof(kTestAccounts) / sizeof(kTestAccounts[0]); ++index) {
        test_accounts_.insert(kTestAccounts[index]);
    }
    for (size_t index = 0; index < sizeof(kTestIps) / sizeof(kTestIps[0]);
         ++index) {
        test_ips_.insert(kTestIps[index]);
    }
}

// IDA_EA: 0x8293D1A
// IDA_NAME: _ZN12CGameManagerD2Ev
// DEMANGLED: CGameManager::~CGameManager()
CGameManager::~CGameManager() {
    if (quick_party_system_manager_) {
        quick_party_system_manager_->~CQuickPartySystemManager();
        ::operator delete(quick_party_system_manager_);
    }
    quick_party_system_manager_ = 0;
    if (quick_party_reward_manager_) {
        quick_party_reward_manager_->~CQuickPartyRewardManager();
        ::operator delete(quick_party_reward_manager_);
    }
    quick_party_reward_manager_ = 0;
    if (auction_average_price_manager_) {
        auction_average_price_manager_->~CAuctionAveragePrice();
        ::operator delete(auction_average_price_manager_);
    }
    auction_average_price_manager_ = 0;
    if (crane_minigame_manager_) {
        crane_minigame_manager_->~CraneMinigameManager();
        ::operator delete(crane_minigame_manager_);
    }
}

// IDA_EA: 0x82941D8
// IDA_NAME: _ZN12CGameManager4initEv
// DEMANGLED: CGameManager::init(void)
bool CGameManager::init() {
    crane_minigame_manager_ = new CraneMinigameManager;
    return GetCraneMinigameManager()->init() == 1;
}

// IDA_EA: 0x829424C
// IDA_NAME: _ZN12CGameManager10getNextUIDEv
// DEMANGLED: CGameManager::getNextUID(void)
int CGameManager::getNextUID() {
    const uint16_t result = static_cast<uint16_t>(++next_uid_);
    if (result > 0x7ffeu) {
        next_uid_ = 0;
    }
    return static_cast<int>(result);
}

// IDA_EA: 0x82948A2
// IDA_NAME: _ZN12CGameManager6GetIdxEP5CUser
// DEMANGLED: CGameManager::GetIdx(CUser *)
int CGameManager::GetIdx(CUser* user) {
    return user_pool_.GetIndex(user);
}

// IDA_EA: 0x829451E
// IDA_NAME: _ZN12CGameManager14check_user_varEP5CUser
// DEMANGLED: CGameManager::check_user_var(CUser *)
short CGameManager::check_user_var(CUser* user) {
    if (user->get_charac_guildkey() && user->get_guild_exp()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x92c,
                              "[CHECK_USER_VAR] RESET GUILD EXP ERROR!");
    }
    if (user->get_charac_level_before_dungeon()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x92f,
                              "[CHECK_USER_VAR] RESET CHARACTER LEVEL BEFORE DUNGEON ERROR!");
    }
    if (user->CheckInParty()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x932, "[CHECK_USER_VAR] RESET PARTY ERROR!");
    }
    if (user->CheckInTrade()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x935, "[CHECK_USER_VAR] RESET TRADE INDEX ERROR!");
    }
    if (user->CheckInPvp()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x938, "[CHECK_USER_VAR] RESET PVP INDEX ERROR!");
    }
    if (user->CheckInWarRoom()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x93b, "[CHECK_USER_VAR] RESET WARROOM INDEX ERROR!");
    }
    if (user->getPlayingDungeonIndex() != -1) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x93e, "[CHECK_USER_VAR] RESET DUNGEON ERROR!");
    }
    if (user->IsProgLogout()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x941, "[CHECK_USER_VAR] RESET PROG LOGOUT ERROR!");
    }
    if (user->character_view().isSaveCharacView()) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x944, "[CHECK_USER_VAR] RESET CHARACTER VIEW ERROR!");
    }
    const short result = user->checkInBlueMarble();
    if (result) {
        LogManager::logFormat(1, "App.cpp",
                              "void CGameManager::check_user_var(CUser*)",
                              0x948, "[CHECK_USER_VAR] RESET BLUEMARBLE INDEX ERROR!");
    }
    return result;
}

// IDA_EA: 0x8294296
// IDA_NAME: _ZN12CGameManager10createUserEv
// DEMANGLED: CGameManager::createUser(void)
CUser* CGameManager::createUser() {
    CUser* user = 0;
    {
        Guard<Mutex> guard(static_cast<Mutex*>(this));
        user = user_pool_.Acquire();
    }
    if (!user) {
        cMyTrace trace("CUser* CGameManager::createUser()", 2275, 5);
        trace("createUser Failed! need to restart!");
        return 0;
    }

    const int prior_uid = user->GetUID();
    const unsigned int prior_account = user->get_acc_id();
    check_user_var(user);
    const int pool_index = GetIdx(user);
    user->SetSlotIDX(static_cast<short>(pool_index));

    const int pool_error = user_pool_.error_state;
    if (pool_error) {
        cMyTrace trace("CUser* CGameManager::createUser()", 2294, 0);
        trace("STATIC MEMORY_POOL createUser error(%d), user(%x), last user m_acc_id(%s), idx(%d), new user idx(%d)",
              pool_error, prior_uid, NumberToString(prior_account, 0),
              pool_index, pool_index);
    }

    user->SetIncreID(static_cast<short>(getNextUID()));
    user->set_unique_id(static_cast<unsigned short>(
        CUserGlobalInfoHandleInstance()->get_uniqueid()));
    return user;
}

// IDA_EA: 0x829445E
// IDA_NAME: _ZN12CGameManager17insertUserByAccIDEP5CUser
// DEMANGLED: CGameManager::insertUserByAccID(CUser *)
int CGameManager::insertUserByAccID(CUser* user) {
    const unsigned int account_id = user->get_acc_id();
    users_by_account_[account_id] = user;

    const unsigned int second_account_id = user->get_acc_id();
    if (second_account_id) {
        users_by_charac_[second_account_id] = user;
        return static_cast<int>(reinterpret_cast<uintptr_t>(
            GlobalData::s_pLogGameChannel->IncInUser()));
    }

    cMyTrace trace("void CGameManager::insertUserByAccID(CUser*)", 2333, 5);
    return trace("[QQID_ERROR]CGameManager::insertUserByAccID  qq_id:%s",
                 user->get_acc_name());
}

// IDA_EA: 0x82990A6
// IDA_NAME: _ZN12CGameManager23GetCraneMinigameManagerEv
// DEMANGLED: CGameManager::GetCraneMinigameManager(void)
CraneMinigameManager* CGameManager::GetCraneMinigameManager() {
    return crane_minigame_manager_;
}

// IDA_EA: 0x82947A4
// IDA_NAME: _ZN12CGameManager7getUserEii
// DEMANGLED: CGameManager::getUser(int,int)
CUser* CGameManager::getUser(int user_uid, int signal_bias) {
    const short slot_id = static_cast<short>(user_uid >> 16);
    const short incre_id = static_cast<short>(user_uid & 0xFFFF);
    (void)signal_bias;

    CUser* user = 0;
    {
        Guard<Mutex> guard(static_cast<Mutex*>(this));
        user = user_pool_.Get(slot_id);
    }

    if (!user) {
        cMyTrace trace("CUser* CGameManager::getUser(int, int)", 2396, 5);
        trace(" No way! getUser , slot_id(%d) is abnormal!", slot_id);
        return 0;
    }

    const short current_incre_id = static_cast<short>(user->GetIncreID());
    if (!current_incre_id) return 0;
    if (current_incre_id != incre_id && incre_id) return 0;
    return user;
}

// IDA_EA: 0x82948C6
// IDA_NAME: _ZN12CGameManager14GetUserByAccIdEj
// DEMANGLED: CGameManager::GetUserByAccId(unsigned int)
CUser* CGameManager::GetUserByAccId(unsigned int account_id) {
    UserAccountMap& users = users_by_account_;
    UserAccountMap::iterator found = users.find(account_id);
    if (found == users.end()) {
        return 0;
    }

    CUser* const user = found->second;
    if (static_cast<unsigned int>(user->get_acc_id()) == account_id) {
        return user;
    }

    const char* const map_account = NumberToString(
        static_cast<unsigned int>(user->get_acc_id()), 1);
    const char* const requested_account = NumberToString(account_id, 0);
    cMyTrace trace(
        "CUser* CGameManager::GetUserByAccId(memberIdentificationNumber_t)",
        2475,
        5);
    trace("[%s][%d][IN m_id: %s][Map m_id: %s]",
          "CUser* CGameManager::GetUserByAccId(memberIdentificationNumber_t)",
          2475,
          requested_account,
          map_account);
    return 0;
}

// IDA_EA: 0x8294AD0
// IDA_NAME: _ZN12CGameManager20isConnectUserByAccIdEj
// DEMANGLED: CGameManager::isConnectUserByAccId(unsigned int)
int CGameManager::isConnectUserByAccId(unsigned int account_id) {
    for (int index = 0; index <= 599; ++index) {
        CUser* const user = user_pool_.Get(index);
        if (static_cast<unsigned int>(user->get_acc_id()) == account_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8294B2C
// IDA_NAME: _ZN12CGameManager14returnUserPoolEP5CUser
// DEMANGLED: CGameManager::returnUserPool(CUser *)
void CGameManager::returnUserPool(CUser* user) {
    if (!user) {
        return;
    }

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    UserMap::iterator account = users_by_account_.find(account_id);
    if (account != users_by_account_.end()) {
        users_by_account_.erase(account);
        GlobalData::s_pLogGameChannel->IncOutUser();
    }

    UserMap::iterator character = users_by_charac_.find(account_id);
    if (character != users_by_charac_.end()) {
        users_by_charac_.erase(character);
    }

    const int user_uid = user->GetUID();
    user->log_out();
    {
        Guard<Mutex> guard(this);
        user_pool_.Free(user);
    }

    if (user_pool_.error_state) {
        cMyTrace trace("void CGameManager::returnUserPool(CUser*)", 2574, 0);
        trace("STATIC MEMORY_POOL returnUserPool error(%d), user(%x), "
              "last user m_acc_id(%s), idx(%d)",
              user_pool_.error_state,
              user,
              NumberToString(account_id, 0),
              user_uid);
    }
}

// IDA_EA: 0x82A3424
// IDA_NAME: _ZN12CGameManager22findJoinableBlueMarbleEv
// DEMANGLED: CGameManager::findJoinableBlueMarble(void)
BlueMarble* CGameManager::findJoinableBlueMarble() {
    BlueMarbleScriptManager* const script =
        G_CDataManager()->getBlueMarbleScript();
    if (!script) {
        return 0;
    }

    int candidate_user_count = script->info_script.player_max;
    BlueMarbleMap& blue_marbles = blue_marbles_;
    while (candidate_user_count > 0) {
        for (BlueMarbleMap::iterator it = blue_marbles.begin();
             it != blue_marbles.end();
             ++it) {
            BlueMarble* const blue_marble = it->second;
            if (blue_marble && blue_marble->checkJoin() &&
                blue_marble->getWaitUserCount() ==
                    candidate_user_count - 1) {
                return blue_marble;
            }
        }
        --candidate_user_count;
    }
    return 0;
}

// IDA_EA: 0x8294E10
// IDA_NAME: _ZN12CGameManager8GetPartyEv
// DEMANGLED: CGameManager::GetParty(void)
CParty* CGameManager::GetParty() {
    CParty* party = party_pool_.Acquire();
    if (!party) {
        return 0;
    }

    const int pool_index = party_pool_.GetIndex(party);
    party->SetIDX(pool_index);

    const int party_index = party->GetPartyIndex();
    PartyMap& party_map = parties_;
    PartyMap::iterator found = party_map.find(party_index);
    if (found == party_map.end()) {
        const int insert_index = party->GetPartyIndex();
        party_map[insert_index] = party;
    } else {
        LogManager::logFormat(1,
                              "App.cpp",
                              "CParty* CGameManager::GetParty()",
                              0xA5F,
                              "Exist PartyIndex[%d]",
                              party->GetPartyIndex());
    }
    return party;
}

// IDA_EA: 0x8294D3C
// IDA_NAME: _ZN12CGameManager13GetTradeSpaceEv
// DEMANGLED: CGameManager::GetTradeSpace(void)
CTradeSpace* CGameManager::GetTradeSpace() {
    CTradeSpace* const trade_space = trade_space_pool_.Acquire();
    if (!trade_space) return 0;

    trade_space->SetIDX(trade_space_pool_.GetIndex(trade_space));
    return trade_space;
}

// IDA_EA: 0x8294D8A
// IDA_NAME: _ZN12CGameManager13GetTradeSpaceEi
// DEMANGLED: CGameManager::GetTradeSpace(int)
CTradeSpace* CGameManager::GetTradeSpace(int trade_space_index) {
    CTradeSpace* trade_space = trade_space_pool_.Get(trade_space_index);
    if (trade_space) {
        return trade_space;
    }
    return 0;
}

// IDA_EA: 0x8294DDE
// IDA_NAME: _ZN12CGameManager13PutTradeSpaceEP11CTradeSpace
// DEMANGLED: CGameManager::PutTradeSpace(CTradeSpace*)
int CGameManager::PutTradeSpace(CTradeSpace* trade_space) {
    if (trade_space) {
        trade_space->reset();
        return static_cast<int>(
            reinterpret_cast<intptr_t>(trade_space_pool_.Free(trade_space)));
    }
    return 0;
}

// IDA_EA: 0x8294F32
// IDA_NAME: _ZN12CGameManager8GetPartyEi
// DEMANGLED: CGameManager::GetParty(int)
CParty* CGameManager::GetParty(int party_index) {
    PartyMap& party_map = parties_;
    PartyMap::iterator found = party_map.find(party_index);
    if (found == party_map.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x8295450
// IDA_NAME: _ZN12CGameManager13GetQuickPartyEv
// DEMANGLED: CGameManager::GetQuickParty(void)
QuickParty::CQuickParty* CGameManager::GetQuickParty() {
    QuickParty::CQuickParty* const quick_party = quick_party_pool_.Acquire();
    if (!quick_party) {
        return 0;
    }
    quick_party->set_quick_party_index(quick_party_pool_.GetIndex(quick_party));
    const int quick_party_index = quick_party->get_quick_party_index();
    if (quick_parties_.find(quick_party_index) == quick_parties_.end()) {
        quick_parties_[quick_party->get_quick_party_index()] = quick_party;
    } else {
        LogManager::logFormat(1,
                              "App.cpp",
                              "QuickParty::CQuickParty* CGameManager::GetQuickParty()",
                              0xB64,
                              "QUICK_PARTY_LOG : duplicate quick party index(%d)",
                              quick_party_index);
    }
    return quick_party;
}

// IDA_EA: 0x8295572
// IDA_NAME: _ZN12CGameManager13GetQuickPartyEi
// DEMANGLED: CGameManager::GetQuickParty(int)
QuickParty::CQuickParty* CGameManager::GetQuickParty(int quick_party_index) {
    QuickPartyMap::iterator found = quick_parties_.find(quick_party_index);
    return found == quick_parties_.end() ? 0 : found->second;
}

// IDA_EA: 0x8295608
// IDA_NAME: _ZN12CGameManager13PutQuickPartyEPN10QuickParty11CQuickPartyE
// DEMANGLED: CGameManager::PutQuickParty(QuickParty::CQuickParty *)
void CGameManager::PutQuickParty(QuickParty::CQuickParty* quick_party) {
    if (!quick_party) {
        return;
    }
    const int quick_party_index = quick_party->get_quick_party_index();
    if (quick_parties_.find(quick_party_index) != quick_parties_.end()) {
        quick_parties_.erase(quick_party_index);
        quick_party_pool_.Free(quick_party);
        quick_party->init();
    }
}

// IDA_EA: 0x8294FC8
// IDA_NAME: _ZN12CGameManager8PutPartyEP6CParty
// DEMANGLED: CGameManager::PutParty(CParty *)
void CGameManager::PutParty(CParty* party) {
    if (!party) {
        return;
    }

    PartyMap& party_map = parties_;
    const int party_index = party->GetPartyIndex();
    if (party_map.find(party_index) != party_map.end()) {
        party_map.erase(party_index);
        party_pool_.Free(party);
    }
}

// IDA_EA: 0x82956C8
// IDA_NAME: _ZN12CGameManager13getDeathTowerEv
// DEMANGLED: CGameManager::getDeathTower(void)
WongWork::CDeathTower* CGameManager::getDeathTower() {
    WongWork::CDeathTower* tower = death_tower_pool_.Acquire();
    if (!tower) {
        return 0;
    }

    tower->setIdx(death_tower_pool_.GetIndex(tower));
    const int idx = tower->getIdx();
    DeathTowerMap& towers = death_towers_;
    DeathTowerMap::iterator found = towers.find(idx);
    if (found != towers.end()) {
        return 0;
    }

    towers[idx] = tower;
    tower->reset();
    return tower;
}

// IDA_EA: 0x82957BC
// IDA_NAME: _ZN12CGameManager13getDeathTowerEi
// DEMANGLED: CGameManager::getDeathTower(int)
__attribute__((used, noinline)) WongWork::CDeathTower*
CGameManager::getDeathTower(int index) {
    DeathTowerMap::iterator found = death_towers_.find(index);
    return found == death_towers_.end() ? 0 : found->second;
}

// IDA_EA: 0x8295852
// IDA_NAME: _ZN12CGameManager16returnDeathTowerEPN8WongWork11CDeathTowerE
// DEMANGLED: CGameManager::returnDeathTower(WongWork::CDeathTower *)
void CGameManager::returnDeathTower(WongWork::CDeathTower* tower) {
    if (!tower) {
        return;
    }

    const int index = tower->getIdx();
    DeathTowerMap::iterator found = death_towers_.find(index);
    if (found != death_towers_.end()) {
        death_towers_.erase(index);
        death_tower_pool_.Free(tower);
    }
}

// IDA_EA: 0x8295906
// IDA_NAME: _ZN12CGameManager12getBossTowerEv
// DEMANGLED: CGameManager::getBossTower(void)
WongWork::CBossTower* CGameManager::getBossTower() {
    WongWork::CBossTower* tower = boss_tower_pool_.Acquire();
    if (!tower) {
        return 0;
    }

    tower->setIdx(boss_tower_pool_.GetIndex(tower));
    const int idx = tower->getIdx();
    BossTowerMap& towers = boss_towers_;
    BossTowerMap::iterator found = towers.find(idx);
    if (found != towers.end()) {
        return 0;
    }

    towers[idx] = tower;
    tower->reset();
    return tower;
}

// IDA_EA: 0x82959FA
// IDA_NAME: _ZN12CGameManager12getBossTowerEi
// DEMANGLED: CGameManager::getBossTower(int)
__attribute__((used, noinline)) WongWork::CBossTower*
CGameManager::getBossTower(int index) {
    BossTowerMap::iterator found = boss_towers_.find(index);
    return found == boss_towers_.end() ? 0 : found->second;
}

// IDA_EA: 0x8295B44
// IDA_NAME: _ZN12CGameManager15getAdvanceAltarEv
// DEMANGLED: CGameManager::getAdvanceAltar(void)
__attribute__((used, noinline)) advancealtar::StageControl*
CGameManager::getAdvanceAltar() {
    advancealtar::StageControl* stage = advance_altar_pool_.Acquire();
    if (!stage) {
        return 0;
    }

    stage->setIndex(advance_altar_pool_.GetIndex(stage));
    const int index = stage->getIndex();
    AdvanceAltarMap::iterator found = advance_altars_.find(index);
    if (found != advance_altars_.end()) {
        // A genuinely active collision only releases the duplicate acquire;
        // StageControlPool::Free deliberately does not enqueue refcount > 1.
        advance_altar_pool_.Free(stage);
        return 0;
    }

    advance_altars_[stage->getIndex()] = stage;
    stage->reset();
    return stage;
}

// IDA_EA: 0x8295C38
// IDA_NAME: _ZN12CGameManager15getAdvanceAltarEi
// DEMANGLED: CGameManager::getAdvanceAltar(int)
__attribute__((used, noinline)) advancealtar::StageControl*
CGameManager::getAdvanceAltar(int index) {
    AdvanceAltarMap::iterator found = advance_altars_.find(index);
    return found == advance_altars_.end() ? 0 : found->second;
}

// IDA_EA: 0x8295CCE
// IDA_NAME: _ZN12CGameManager18returnAdvanceAltarEPN12advancealtar12StageControlE
// DEMANGLED: CGameManager::returnAdvanceAltar(advancealtar::StageControl *)
void CGameManager::returnAdvanceAltar(advancealtar::StageControl* stage) {
    if (!stage) {
        return;
    }
    const int index = stage->getIndex();
    AdvanceAltarMap::iterator found = advance_altars_.find(index);
    if (found != advance_altars_.end() && found->second == stage) {
        advance_altars_.erase(index);
        stage->setIndex(-1);
        advance_altar_pool_.Free(stage);
    }
}

// IDA_EA: 0x8295D82
// IDA_NAME: _ZN12CGameManager18onTimeAdvanceAltarEv
// DEMANGLED: CGameManager::onTimeAdvanceAltar(void)
int CGameManager::onTimeAdvanceAltar() {
    AdvanceAltarMap::iterator current = advance_altars_.begin();
    while (current != advance_altars_.end()) {
        advancealtar::StageControl* const stage = current->second;
        if (stage->onTimerStageTick() == 1) {
            ++current;
            continue;
        }

        AdvanceAltarMap::iterator expired = current++;
        advance_altars_.erase(expired);
        advance_altar_pool_.Free(stage);
    }
    return 0;
}

// IDA_EA: 0x8295A90
// IDA_NAME: _ZN12CGameManager15returnBossTowerEPN8WongWork10CBossTowerE
// DEMANGLED: CGameManager::returnBossTower(WongWork::CBossTower *)
void CGameManager::returnBossTower(WongWork::CBossTower* tower) {
    if (!tower) {
        return;
    }

    const int index = tower->getIdx();
    BossTowerMap& towers = boss_towers_;
    if (towers.find(index) == towers.end()) {
        return;
    }

    towers.erase(tower->getIdx());
    boss_tower_pool_.Free(tower);
}

// IDA_EA: 0x8295206
// IDA_NAME: _ZN12CGameManager6GetPvpEiP5CUseri
// DEMANGLED: CGameManager::GetPvp(int, CUser*, int)
PvP_Room* CGameManager::GetPvp(int pvp_index, CUser* user, int channel) {
    (void)user;
    (void)channel;

    PvpMap& pvp_map = pvp_rooms_;
    PvpMap::iterator found = pvp_map.find(pvp_index);
    if (found == pvp_map.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x829507C
// IDA_NAME: _ZN12CGameManager6GetPvpEv
// DEMANGLED: CGameManager::GetPvp(void)
PvP_Room* CGameManager::GetPvp() {
    PvP_Room* const room = pvp_pool_.Acquire();
    if (!room) {
        return 0;
    }

    room->SetIDX(pvp_pool_.GetIndex(room));
    PvpMap& pvp_map = pvp_rooms_;
    const int room_index = room->get_index();
    if (pvp_map.find(room_index) != pvp_map.end()) {
        cMyTrace trace("PvP_Room* CGameManager::GetPvp()", 2738, 0);
        trace("pvp@log room(%d) already alloc", room_index);
        return 0;
    }

    cMyTrace trace("PvP_Room* CGameManager::GetPvp()", 2729, 0);
    trace("pvp@log room(%d) new alloc", room_index);
    pvp_map[room_index] = room;
    return room;
}

// IDA_EA: 0x82A3202
// IDA_NAME: _ZN12CGameManager24FindQuickJoinablePvPRoomEbP5CUser
// DEMANGLED: CGameManager::FindQuickJoinablePvPRoom(bool,CUser *)
PvP_Room* CGameManager::FindQuickJoinablePvPRoom(bool quick_join,
                                                  CUser* user) {
    PvpMap& pvp_map = pvp_rooms_;
    for (PvpMap::iterator it = pvp_map.begin(); it != pvp_map.end(); ++it) {
        PvP_Room* const room = it->second;
        if (room && room->get_pvp_battle_mode() != 6 &&
            (!quick_join || room->IsExistPassword() != 1) &&
            room->IsQuickJoinablePVPRoom(user)) {
            return room;
        }
    }
    return 0;
}

// IDA_EA: 0x8295348
// IDA_NAME: _ZN12CGameManager6PutPvpEP8PvP_Room
// DEMANGLED: CGameManager::PutPvp(PvP_Room*)
void CGameManager::PutPvp(PvP_Room* room) {
    if (!room) {
        return;
    }

    const int room_index = room->get_index();
    PvpMap& pvp_map = pvp_rooms_;
    PvpMap::iterator found = pvp_map.find(room_index);
    if (found != pvp_map.end()) {
        pvp_tournament::OnRoomReturned(room);
        zombie_mode::ResetRoom(room);
        pvp_map.erase(room_index);
        pvp_pool_.Free(room);
    }
}

// IDA_EA: 0x82A25D0
// IDA_NAME: _ZN12CGameManager10GetWarRoomEi
// DEMANGLED: CGameManager::GetWarRoom(int)
WarRoom* CGameManager::GetWarRoom(int war_room_index) {
    WarRoomMap& war_room_map = war_rooms_;
    WarRoomMap::iterator found = war_room_map.find(war_room_index);
    if (found == war_room_map.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x82A2666
// IDA_NAME: _ZN12CGameManager10PutWarRoomEP7WarRoom
// DEMANGLED: CGameManager::PutWarRoom(WarRoom*)
void CGameManager::PutWarRoom(WarRoom* room) {
    if (!room) {
        return;
    }

    const int room_index = room->GetIndex();
    WarRoomMap& war_room_map = war_rooms_;
    WarRoomMap::iterator found = war_room_map.find(room_index);
    if (found != war_room_map.end()) {
        war_room_map.erase(room_index);
        war_room_pool_.Free(room);
    }
}

// IDA_EA: 0x82A239E
// IDA_NAME: _ZN12CGameManager13getBlueMarbleEi
// DEMANGLED: CGameManager::getBlueMarble(int)
BlueMarble* CGameManager::getBlueMarble(int blue_marble_index) {
    BlueMarbleMap& blue_marble_map = blue_marbles_;
    BlueMarbleMap::iterator found = blue_marble_map.find(blue_marble_index);
    if (found == blue_marble_map.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x82A2434
// IDA_NAME: _ZN12CGameManager13putBlueMarbleEP10BlueMarble
// DEMANGLED: CGameManager::putBlueMarble(BlueMarble*)
void CGameManager::putBlueMarble(BlueMarble* blue_marble) {
    if (!blue_marble) {
        return;
    }

    const int pool_index = blue_marble->getMemoryPoolIndex();
    BlueMarbleMap& blue_marble_map = blue_marbles_;
    BlueMarbleMap::iterator found = blue_marble_map.find(pool_index);
    if (found != blue_marble_map.end()) {
        blue_marble_map.erase(pool_index);
        blue_marble_pool_.Free(blue_marble);
    }
}

// IDA_EA: 0x8297C14
// IDA_NAME: _ZN12CGameManager13CheckOutTradeEP5CUser
// DEMANGLED: CGameManager::CheckOutTrade(CUser*)
int CGameManager::CheckOutTrade(CUser* user) {
    const int check = user->CheckInTrade();
    int result = check ^ 1;
    if (!static_cast<unsigned char>(result)) {
        const int trade_space_index = user->GetTradeSpace();
        CTradeSpace* trade_space = trade_space_pool_.Get(trade_space_index);
        trade_space->cancel_trade_by_dis(user);
        return PutTradeSpace(trade_space);
    }
    return result;
}

// IDA_EA: 0x8297C76
// IDA_NAME: _ZN12CGameManager13CheckOutPartyEP5CUserb
// DEMANGLED: CGameManager::CheckOutParty(CUser*, bool)
int CGameManager::CheckOutParty(CUser* user, bool check) {
    (void)check;

    const int check_in_party = user->CheckInParty();
    int result = check_in_party ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    CParty* party = GetParty(user->GetPartyIndex());
    if (!party) {
        return result;
    }

    CBattle_Field& battle_field = party->battle_field();
    CDungeon* const dungeon = battle_field.get_dungeon();
    bool special_dungeon = false;
    if (dungeon) {
        special_dungeon = dungeon->get_blood_dungeon_type() > 0;
        if (dungeon->isTowerOfDespairDungeon()) {
            special_dungeon = true;
        }
    }
    if (dungeon) {
        if (static_cast<char>(dungeon->get_dimension_possible()) > 0 ||
            dungeon->isAncientDungeon() || dungeon->isTournamentDungeon()) {
            special_dungeon = true;
        }
    }

    battle_field.dungeon_clear_tracer().Trace(
        "void CGameManager::CheckOutParty(CUser*, bool)");

    if (battle_field.instance_dungeon_mode() != 1 &&
        static_cast<unsigned char>(party->get_state()) == 2) {
        bool event_dungeon = false;
        if (dungeon) {
            event_dungeon = dungeon->isEventDungeon();
        }
        const bool has_penalty = dungeon && dungeon->isRevengeDungeon();
        if (!special_dungeon && event_dungeon && has_penalty) {
            user->giveup_panalty();
        }
    }

    return party->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
}

// IDA_EA: 0x8297DF8
// IDA_NAME: _ZN12CGameManager15CheckOutWarRoomEP5CUser
// DEMANGLED: CGameManager::CheckOutWarRoom(CUser*)
WarRoom* CGameManager::CheckOutWarRoom(CUser* user) {
    const int check_in_war_room = user->CheckInWarRoom();
    WarRoom* result = reinterpret_cast<WarRoom*>(check_in_war_room ^ 1);
    if (static_cast<unsigned char>(reinterpret_cast<uintptr_t>(result))) {
        return result;
    }

    WarRoom* room = GetWarRoom(user->GetWarRoomIndex());
    if (!room) {
        return result;
    }

    if (room->WalkOutUser(user) == 4) {
        LogManager::logFormat(
            1,
            "App.cpp",
            "void CGameManager::CheckOutWarRoom(CUser*)",
            0x117B,
            "CGameManager::CheckOutWarRoom Can't find user charno(%d)",
            user->get_charac_no(-1));
    }
    return reinterpret_cast<WarRoom*>(room->CheckState());
}

// IDA_EA: 0x8297EAA
// IDA_NAME: _ZN12CGameManager11CheckOutPvpEP5CUserb
// DEMANGLED: CGameManager::CheckOutPvp(CUser*, bool)
void CGameManager::CheckOutPvp(CUser* user, bool skip_end) {
    if (static_cast<unsigned char>(user->CheckInPvp()) != 1) {
        return;
    }

    PvP_Room* room = GetPvp(user->GetPvpIndex(), user, 0);
    if (!room) {
        return;
    }

    PacketGuard packet;
    bool changed = false;
    const int seat = room->leave_room(user, changed);
    room->get_pvp_battle_mode();
    room->get_index();
    user->getCurCharacName();
    user->get_acc_name();

    if (seat < 0) {
        LogManager::logFormat(1,
                              "App.cpp",
                              "void CGameManager::CheckOutPvp(CUser*, bool)",
                              0x11BD,
                              "CGameManager::CheckOutPvp Can't erase pvp room(%d) accid(%s)",
                              room->get_index(),
                              NumberToString(
                                  static_cast<unsigned int>(
                                      user->get_acc_id()),
                                  0));
    } else {
        packet.clear();
        room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
        G_GameWorld()->send_all(packet);

        if (changed) {
            packet.clear();
            room->make_state_info(reinterpret_cast<char*>(&packet));
            G_GameWorld()->send_all(packet);
        }

        if (!room->get_waiter_count()) {
            room->destroy_room(false);
            PutPvp(room);
            packet.clear();
            room->make_state_info(reinterpret_cast<char*>(&packet));
            G_GameWorld()->send_all(packet);
        }
    }

    if (!skip_end && room->get_recv_pvp_rank_count() > 0 &&
        static_cast<unsigned char>(room->IsInsertTimerRecvPvpRank()) != 1 &&
        static_cast<unsigned char>(room->IsEndPvpBattle()) != 1) {
        room->send_pvp_end();
    }
}

// IDA_EA: 0x82981A4
// IDA_NAME: _ZN12CGameManager17checkOutBossTowerEP5CUser
// DEMANGLED: CGameManager::checkOutBossTower(CUser *)
void CGameManager::checkOutBossTower(CUser* user) {
    if (!user || user->checkInBossTower() != 1) {
        return;
    }

    WongWork::CBossTower* const tower =
        getBossTower(user->getBossTowerIndex());
    if (tower && tower->handleLeaveUser(user) != 1) {
        cMyTrace trace("void CGameManager::checkOutBossTower(CUser*)",
                       4569,
                       1);
        trace("BossTower checkOutBossTower ID[%d] ACCID[%s]",
              tower->getIdx(),
              NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                             0));
    }
}

// IDA_EA: 0x8298272
// IDA_NAME: _ZN12CGameManager20checkOutAdvanceAltarEP5CUser
// DEMANGLED: CGameManager::checkOutAdvanceAltar(CUser *)
void CGameManager::checkOutAdvanceAltar(CUser* user) {
    if (!user || user->checkInAdvanceAltar() != 1) {
        return;
    }

    const int stage_index = user->getAdvanceAltarIndex();
    advancealtar::StageControl* const stage = getAdvanceAltar(stage_index);
    if (!stage || stage->charac_data.getUser() != user) {
        user->setAdvanceAltarIndex(-1);
        return;
    }
    if (stage->leaveUser() != 1) {
        user->setAdvanceAltarIndex(-1);
        stage->destroy();
        cMyTrace trace("void CGameManager::checkOutAdvanceAltar(CUser*)",
                       4591,
                       1);
        trace("invalid user : advance altar logout ID[%d] ACCID[%s]",
              stage->getIndex(),
              NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                             0));
    }
}

// IDA_EA: 0x829833A
// IDA_NAME: _ZN12CGameManager18checkOutDeathTowerEP5CUser
// DEMANGLED: CGameManager::checkOutDeathTower(CUser *)
void CGameManager::checkOutDeathTower(CUser* user) {
    if (!user || user->checkInDeathTower() != 1) {
        return;
    }

    WongWork::CDeathTower* const tower =
        getDeathTower(user->getDeathTowerIndex());
    if (tower && tower->onLeaveUser(user) != 1) {
        cMyTrace trace("void CGameManager::checkOutDeathTower(CUser*)",
                       4666,
                       1);
        trace("DeathTower checkOutDeathTower ID[%d] ACCID[%s]",
              tower->getIdx(),
              NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                             0));
    }
}

// IDA_EA: 0x829844A
// IDA_NAME: _ZN12CGameManager18checkOutBlueMarbleEP5CUser
// DEMANGLED: CGameManager::checkOutBlueMarble(CUser*)
void CGameManager::checkOutBlueMarble(CUser* user) {
    if (static_cast<unsigned char>(user->checkInBlueMarble()) == 1) {
        BlueMarble* blue_marble =
            getBlueMarble(user->getBlueMarbleIndex());
        if (blue_marble) {
            blue_marble->leaveUser(user);
        }
    }
}

// IDA_EA: 0x82985A8
// IDA_NAME: _ZN12CGameManager9user_exitEP5CUser
// DEMANGLED: CGameManager::user_exit(CUser *)
int CGameManager::user_exit(CUser* user) {
    if (!user) {
        cMyTrace trace("bool CGameManager::user_exit(CUser*)", 4721, 5);
        trace("USER_EXIT [%s][%d]", "bool CGameManager::user_exit(CUser*)");
        return 0;
    }
    if (!user->get_state()) {
        cMyTrace trace("bool CGameManager::user_exit(CUser*)", 4730, 5);
        trace("USER_EXIT STATE NONE [%s][%d]mid(%d)",
              "bool CGameManager::user_exit(CUser*)",
              user->get_acc_id(),
              user->get_acc_id());
        return 0;
    }

    HistoryLog::LogClose(user->history_stream());
    user->prepareDisconnect();
    G_GameWorld()->EraseLoginUser(user);

    if (user->get_state() <= 2) {
        user->SetSaveRentalInfoToExchange(false);
        user->LogoutCachedCharacter(0);
        user->UpdateLogout(false);
        return 1;
    }

    CheckOutTrade(user);
    CheckOutParty(user, false);
    CheckOutPvp(user, false);
    CheckOutWarRoom(user);
    checkOutDeathTower(user);
    checkOutBossTower(user);
    checkOutAdvanceAltar(user);
    checkOutBlueMarble(user);
    G_GameWorld()->leave_game_world(user);
    user->SetGameMasterMode(false);
    user->UpdateData();
    user->LogoutCachedCharacter(0);
    user->UpdateLogout(false);

    cMyTrace logout_trace("bool CGameManager::user_exit(CUser*)", 4828, 0);
    logout_trace("ID: %s - UID: %d from Logout",
                 user->getCurCharacName(),
                 user->get_unique_id());
    const int epoll_error =
        G_EpollHandler()->ResetEpoll(user->network_.get_socket());
    if (epoll_error) {
        cMyTrace epoll_trace("bool CGameManager::user_exit(CUser*)", 4835, 1);
        epoll_trace("(m_id: %s) G_EpollHandler()->ResetEpoll("
                    "pUser->get_socket(%d)) %d",
                    NumberToString(
                        static_cast<unsigned int>(user->get_acc_id()), 0),
                    user->network_.get_socket(),
                    epoll_error);
    }

    user->ResetCurCharac();
    user->doLinkCharacDisconnect();
    returnUserPool(user);
    user->checkLogOutCorrectly();
    return 1;
}

// IDA_EA: 0x8298B48
// IDA_NAME: _ZN12CGameManager15user_disconnectEP5CUser
// DEMANGLED: CGameManager::user_disconnect(CUser *)
char CGameManager::user_disconnect(CUser* user) {
    if (!user) {
        cMyTrace trace("void CGameManager::user_disconnect(CUser*)", 5003, 5);
        trace("USER DISCONNECT [%s][%d]",
              "void CGameManager::user_disconnect(CUser*)");
        return 0;
    }

    HistoryLog::LogClose(user->history_stream());
    if (!user->get_state()) {
        return 1;
    }

    user->prepareDisconnect();
    G_GameWorld()->EraseLoginUser(user);
    user->SetSaveRentalInfoToExchange(false);

    if (user->get_state() <= 2) {
        // The original executes this second store on the <=2 branch.
        user->SetSaveRentalInfoToExchange(false);
        user->LogoutCachedCharacter(0);
        user->UpdateLogout(true);
    } else {
        CheckOutTrade(user);
        CheckOutParty(user, false);
        CheckOutPvp(user, false);
        CheckOutWarRoom(user);
        checkOutDeathTower(user);
        checkOutBossTower(user);
        checkOutAdvanceAltar(user);
        checkOutBlueMarble(user);
        G_GameWorld()->leave_game_world(user);
        user->SetGameMasterMode(false);
        user->UpdateData();
        user->LogoutCachedCharacter(0);
        user->UpdateLogout(true);
        user->DeleteRentalItemInfo(1);
        user->DeleteRentalItemInfo(2);
        user->ResetCurCharac();
        user->doLinkCharacDisconnect();
    }

    returnUserPool(user);
    return static_cast<char>(
        GetInstanceShutdowManager()->SendLastMsgDBQueue(user));
}

// IDA_EA: 0x82983FC
// IDA_NAME: _ZN12CGameManager18CheckOutQuickPartyEP6CPartyb
// DEMANGLED: CGameManager::CheckOutQuickParty(CParty*, bool)
int CGameManager::CheckOutQuickParty(CParty* party, bool check_eplp) {
    int result = check_eplp;
    if (party) {
        CGameManager* manager = G_CGameManager();
        QuickParty::CQuickPartySystemManager* quick_party_manager =
            manager->GetQuickPartySystemManager();
        return quick_party_manager->cancel_quick_party(party, check_eplp, 0) ^
               1;
    }
    return result;
}

// IDA_EA: 0x8298DC0
// IDA_NAME: _ZN12CGameManager26GetQuickPartySystemManagerEv
// DEMANGLED: CGameManager::GetQuickPartySystemManager(void)
QuickParty::CQuickPartySystemManager*
CGameManager::GetQuickPartySystemManager() {
    if (!quick_party_system_manager_) {
        void* storage = ::operator new(0x18u);
        quick_party_system_manager_ =
            static_cast<QuickParty::CQuickPartySystemManager*>(storage);
        new (quick_party_system_manager_) QuickParty::CQuickPartySystemManager();
    }
    return quick_party_system_manager_;
}

// IDA_EA: 0x8298E24
// IDA_NAME: _ZN12CGameManager26GetQuickPartyRewardManagerEv
// DEMANGLED: CGameManager::GetQuickPartyRewardManager(void)
QuickParty::CQuickPartyRewardManager*
CGameManager::GetQuickPartyRewardManager() {
    if (!quick_party_reward_manager_) {
        void* storage = ::operator new(0x228u);
        quick_party_reward_manager_ =
            static_cast<QuickParty::CQuickPartyRewardManager*>(storage);
        new (quick_party_reward_manager_) QuickParty::CQuickPartyRewardManager();
    }
    return quick_party_reward_manager_;
}

// IDA_EA: 0x8298E88
// IDA_NAME: _ZN12CGameManager22GetPremiumLetheManagerEv
// DEMANGLED: CGameManager::GetPremiumLetheManager(void)
CPremiumLetheManager* CGameManager::GetPremiumLetheManager() {
    if (!premium_lethe_manager_) {
        premium_lethe_manager_ = new CPremiumLetheManager();
    }
    return premium_lethe_manager_;
}

// IDA_EA: 0x8298EEC
// IDA_NAME: _ZN12CGameManager29GetSharedServerMessageManagerEv
// DEMANGLED: CGameManager::GetSharedServerMessageManager(void)
CSharedServerMessageManager*
CGameManager::GetSharedServerMessageManager() {
    if (!shared_server_message_manager_) {
        const stSharedServerMessageInfo* const raw_info =
            G_CDataManager()->get_server_parameter_script()
                ->get_shared_server_message_info();
        shared_server_message_manager_ = new CSharedServerMessageManager(
            const_cast<stSharedServerMessageInfo*>(raw_info));
    }
    return shared_server_message_manager_;
}

// IDA_EA: 0x8298F60
// IDA_NAME: _ZN12CGameManager24GetConditionEventManagerEv
// DEMANGLED: CGameManager::GetConditionEventManager(void)
CConditionEventManager* CGameManager::GetConditionEventManager() {
    // The raw C2 is empty and the original allocation is exactly one byte.
    // This manager has no owned fields; its data remains in CDataManager.
    if (!condition_event_manager_) {
        void* const storage = ::operator new(1u);
        condition_event_manager_ =
            static_cast<CConditionEventManager*>(storage);
        new (condition_event_manager_) CConditionEventManager();
    }
    return condition_event_manager_;
}

// IDA_EA: 0x8299028
// IDA_NAME: _ZN12CGameManager29GetAuctionAveragePriceManagerEv
// DEMANGLED: CGameManager::GetAuctionAveragePriceManager(void)
CAuctionAveragePrice* CGameManager::GetAuctionAveragePriceManager() {
    if (!auction_average_price_manager_) {
        auction_average_price_manager_ = new (std::nothrow) CAuctionAveragePrice();
    }
    return auction_average_price_manager_;
}

// IDA_EA: 0x8298FC4
// IDA_NAME: _ZN12CGameManager28GetSpecialItemRoutingManagerEv
// DEMANGLED: CGameManager::GetSpecialItemRoutingManager(void)
CSpecialItemRoutingManager* CGameManager::GetSpecialItemRoutingManager() {
    if (!special_item_routing_manager_) {
        void* storage = ::operator new(1u);
        special_item_routing_manager_ =
            static_cast<CSpecialItemRoutingManager*>(storage);
        new (special_item_routing_manager_) CSpecialItemRoutingManager();
    }
    return special_item_routing_manager_;
}
