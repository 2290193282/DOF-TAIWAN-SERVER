#ifndef OLDSRV_GLOBAL_GAME_MANAGER_H_
#define OLDSRV_GLOBAL_GAME_MANAGER_H_

#include <ext/hash_map>
#include <map>
#include <set>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/trade/trade_space_pool_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/thread/mutex.h"


namespace QuickParty {
struct CQuickPartySystemManager;
struct CQuickPartyRewardManager;
class CQuickParty;
}  // namespace QuickParty

class CParty;
class CTradeSpace;
class CSpecialItemRoutingManager;
class CSharedServerMessageManager;
class CConditionEventManager;
class CraneMinigameManager;
class CPremiumLetheManager;
class CAuctionAveragePrice;
class PvP_Room;
class WarRoom;
class BlueMarble;
namespace WongWork {
class CDeathTower;
class CBossTower;
}  // namespace WongWork
namespace advancealtar {
class StageControl;
}  // namespace advancealtar

// IDA unit: 99_misc_global/CGameManager.  The manager owns ten concrete
// StaticPool instances; their order is the original C2/D2 construction order.
class CGameManager : public Mutex {
public:
    // IDA_EA: 0x82930C2
    // IDA_NAME: _ZN12CGameManagerC2Ev
    // DEMANGLED: CGameManager::CGameManager(void)
    CGameManager();
    // IDA_EA: 0x8293D1A
    // IDA_NAME: _ZN12CGameManagerD2Ev
    // DEMANGLED: CGameManager::~CGameManager()
    ~CGameManager();

    // IDA_EA: 0x82941D8
    // IDA_NAME: _ZN12CGameManager4initEv
    // DEMANGLED: CGameManager::init(void)
    bool init();

    // IDA_EA: 0x829424C / 0x8294296 / 0x82948A2.  These are the original
    // accept-path allocation helpers.  The slot/increment UID and the
    // protocol-visible unique ID are intentionally separate values.
    int getNextUID();
    CUser* createUser();
    // IDA_EA: 0x829445E
    // IDA_NAME: _ZN12CGameManager17insertUserByAccIDEP5CUser
    // DEMANGLED: CGameManager::insertUserByAccID(CUser *)
    int insertUserByAccID(CUser* user);
    int GetIdx(CUser* user);
    short check_user_var(CUser* user);
    // IDA_EA: 0x8294B2C / 0x82985A8 / 0x8298B48
    // IDA_NAME: _ZN12CGameManager14returnUserPoolEP5CUser /
    //           _ZN12CGameManager9user_exitEP5CUser
    // DEMANGLED: CGameManager::returnUserPool(CUser *) /
    //            CGameManager::user_exit(CUser *)
    void returnUserPool(CUser* user);
    int user_exit(CUser* user);
    // IDA_NAME: _ZN12CGameManager15user_disconnectEP5CUser
    // DEMANGLED: CGameManager::user_disconnect(CUser *)
    // This is the forced-disconnect path used by Inter_Disconnect.  It is
    // distinct from user_exit: it always returns the pool node and queues the
    // shutdown manager's final DB notification.
    char user_disconnect(CUser* user);

    CUser* getUser(int user_uid, int signal_bias);
    CUser* GetUserByAccId(unsigned int account_id);
    int isConnectUserByAccId(unsigned int account_id);
    CParty* GetParty();
    CParty* GetParty(int party_index);
    void PutParty(CParty* party);
    // IDA_EA: 0x8294D3C
    // IDA_NAME: _ZN12CGameManager13GetTradeSpaceEv
    // DEMANGLED: CGameManager::GetTradeSpace(void)
    // Acquires a pool-backed transaction and assigns its pool index before
    // the peer-response transaction installs the two traders.
    CTradeSpace* GetTradeSpace();
    CTradeSpace* GetTradeSpace(int trade_space_index);
    int PutTradeSpace(CTradeSpace* trade_space);
    PvP_Room* GetPvp();
    PvP_Room* GetPvp(int pvp_index, CUser* user, int channel);
    void PutPvp(PvP_Room* room);
    // IDA_EA: 0x82A3202
    // IDA_NAME: _ZN12CGameManager24FindQuickJoinablePvPRoomEbP5CUser
    // DEMANGLED: CGameManager::FindQuickJoinablePvPRoom(bool,CUser *)
    PvP_Room* FindQuickJoinablePvPRoom(bool quick_join, CUser* user);
    // IDA_EA: 0x82A3424
    // IDA_NAME: _ZN12CGameManager22findJoinableBlueMarbleEv
    // DEMANGLED: CGameManager::findJoinableBlueMarble(void)
    BlueMarble* findJoinableBlueMarble();

    // IDA_EA: 0x82A24E8
    // IDA_NAME: _ZN12CGameManager10GetWarRoomEv
    // DEMANGLED: CGameManager::GetWarRoom(void)
    // Acquires the manager-owned 40-room pool and publishes the assigned
    // pool index in the canonical room map.
    WarRoom* GetWarRoom();
    WarRoom* GetWarRoom(int war_room_index);
    void PutWarRoom(WarRoom* room);

    // IDA_EA: 0x8298940
    // IDA_NAME: _ZN12CGameManager31WarRoomCountAdjustByChannelInfoEv
    // DEMANGLED: CGameManager::WarRoomCountAdjustByChannelInfo(void)
    bool WarRoomCountAdjustByChannelInfo();

    // IDA_EA: 0x8298A30
    // IDA_NAME: _ZN12CGameManager12WarRoomAllocEv
    // DEMANGLED: CGameManager::WarRoomAlloc(void)
    void WarRoomAlloc();

    // IDA_EA: 0x8298B40
    // IDA_NAME: _ZN12CGameManager16PrintWarRoomListEv
    // DEMANGLED: CGameManager::PrintWarRoomList(void)
    void PrintWarRoomList();

    // IDA_EA: 0x82A22B6
    // IDA_NAME: _ZN12CGameManager13getBlueMarbleEv
    // DEMANGLED: CGameManager::getBlueMarble(void)
    BlueMarble* getBlueMarble();
    BlueMarble* getBlueMarble(int blue_marble_index);
    // IDA_EA: 0x82A3396
    // IDA_NAME: _ZN12CGameManager15allocBlueMarbleEv
    // DEMANGLED: CGameManager::allocBlueMarble(void)
    bool allocBlueMarble();
    void putBlueMarble(BlueMarble* blue_marble);
    int CheckOutTrade(CUser* user);
    int CheckOutParty(CUser* user, bool check);
    WarRoom* CheckOutWarRoom(CUser* user);
    void CheckOutPvp(CUser* user, bool skip_end);
    // IDA_EA: 0x82981A4 / 0x8298272 / 0x829833A
    // IDA_NAME: _ZN12CGameManager17checkOutBossTowerEP5CUser /
    //           _ZN12CGameManager20checkOutAdvanceAltarEP5CUser /
    //           _ZN12CGameManager18checkOutDeathTowerEP5CUser
    void checkOutBossTower(CUser* user);
    void checkOutAdvanceAltar(CUser* user);
    void checkOutDeathTower(CUser* user);
    void checkOutBlueMarble(CUser* user);
    int CheckOutQuickParty(CParty* party, bool check_eplp);
    // IDA_EA: 0x8295450 / 0x8295572 / 0x8295608
    // IDA_NAME: _ZN12CGameManager13GetQuickPartyEv
    //           _ZN12CGameManager13GetQuickPartyEi
    //           _ZN12CGameManager13PutQuickPartyEPN10QuickParty11CQuickPartyE
    QuickParty::CQuickParty* GetQuickParty();
    QuickParty::CQuickParty* GetQuickParty(int quick_party_index);
    void PutQuickParty(QuickParty::CQuickParty* quick_party);
    QuickParty::CQuickPartySystemManager* GetQuickPartySystemManager();
    QuickParty::CQuickPartyRewardManager* GetQuickPartyRewardManager();
    // IDA_EA: 0x8298E88
    // IDA_NAME: _ZN12CGameManager22GetPremiumLetheManagerEv
    // DEMANGLED: CGameManager::GetPremiumLetheManager(void)
    CPremiumLetheManager* GetPremiumLetheManager();
    CSpecialItemRoutingManager* GetSpecialItemRoutingManager();
    CSharedServerMessageManager* GetSharedServerMessageManager();
    // IDA_EA: 0x8298F60
    // IDA_NAME: _ZN12CGameManager24GetConditionEventManagerEv
    // DEMANGLED: CGameManager::GetConditionEventManager(void)
    CConditionEventManager* GetConditionEventManager();
    // IDA_EA: 0x8299028
    // IDA_NAME: _ZN12CGameManager29GetAuctionAveragePriceManagerEv
    // DEMANGLED: CGameManager::GetAuctionAveragePriceManager(void)
    CAuctionAveragePrice* GetAuctionAveragePriceManager();
    // IDA_NAME: _ZN12CGameManager23GetCraneMinigameManagerEv
    // DEMANGLED: CGameManager::GetCraneMinigameManager(void)
    CraneMinigameManager* GetCraneMinigameManager();
    WongWork::CDeathTower* getDeathTower();
    // IDA_EA: 0x82957BC
    // IDA_NAME: _ZN12CGameManager13getDeathTowerEi
    WongWork::CDeathTower* getDeathTower(int index);
    // IDA_EA: 0x8295852
    // IDA_NAME: _ZN12CGameManager16returnDeathTowerEPN8WongWork11CDeathTowerE
    // DEMANGLED: CGameManager::returnDeathTower(WongWork::CDeathTower *)
    void returnDeathTower(WongWork::CDeathTower* tower);
    WongWork::CBossTower* getBossTower();
    // IDA_EA: 0x82959FA
    // IDA_NAME: _ZN12CGameManager12getBossTowerEi
    WongWork::CBossTower* getBossTower(int index);
    // IDA_EA: 0x8295B44 / 0x8295C38
    // IDA_NAME: _ZN12CGameManager15getAdvanceAltarEv / Ei
    advancealtar::StageControl* getAdvanceAltar();
    advancealtar::StageControl* getAdvanceAltar(int index);
    // IDA_EA: 0x8295CCE
    // IDA_NAME: _ZN12CGameManager18returnAdvanceAltarEPN12advancealtar12StageControlE
    // DEMANGLED: CGameManager::returnAdvanceAltar(advancealtar::StageControl *)
    void returnAdvanceAltar(advancealtar::StageControl* stage);
    // IDA_EA: 0x8295D82
    // IDA_NAME: _ZN12CGameManager18onTimeAdvanceAltarEv
    // DEMANGLED: CGameManager::onTimeAdvanceAltar(void)
    int onTimeAdvanceAltar();
    // IDA_EA: 0x8295E84
    // IDA_NAME: _ZN12CGameManager31send_userinfos_to_cutoff_serverEv
    char send_userinfos_to_cutoff_server();
    // IDA_EA: 0x82961BE
    // IDA_NAME: _ZN12CGameManager30Send_userinfos_to_upper_serverEh
    // DEMANGLED: CGameManager::Send_userinfos_to_upper_server(unsigned char)
    char Send_userinfos_to_upper_server(unsigned char target);
    // IDA_EA: 0x8296642 / 0x8296792 / 0x82A2F94
    // IDA_NAME: _ZN12CGameManager11SendPvpListEP5CUser /
    //           _ZN12CGameManager13SendPartyListEP5CUser /
    //           _ZN12CGameManager15SendWarRoomListEP5CUser
    // DEMANGLED: CGameManager::SendPvpList(CUser *) /
    //            CGameManager::SendPartyList(CUser *) /
    //            CGameManager::SendWarRoomList(CUser *)
    void SendPvpList(CUser* user);
    void SendPartyList(CUser* user);
    void SendWarRoomList(CUser* user);

    // IDA_EA: 0x8296D30
    // IDA_NAME: _ZN12CGameManager17insert_game_worldEP5CUser
    // DEMANGLED: CGameManager::insert_game_world(CUser *)
    int insert_game_world(CUser* user);

    // IDA_EA: 0x8295A90
    // IDA_NAME: _ZN12CGameManager15returnBossTowerEPN8WongWork10CBossTowerE
    // DEMANGLED: CGameManager::returnBossTower(WongWork::CBossTower *)
    void returnBossTower(WongWork::CBossTower* tower);

public:  // Exact in-object ABI members, kept visible for restored owners.
    typedef std::map<unsigned int, CUser*> UserMap;
    typedef __gnu_cxx::hash_map<int, PvP_Room*> PvpMap;
    typedef __gnu_cxx::hash_map<int, CParty*> PartyMap;
    typedef __gnu_cxx::hash_map<int, WarRoom*> WarRoomMap;
    typedef __gnu_cxx::hash_map<int, WongWork::CDeathTower*> DeathTowerMap;
    typedef __gnu_cxx::hash_map<int, WongWork::CBossTower*> BossTowerMap;
    typedef __gnu_cxx::hash_map<int, advancealtar::StageControl*> AdvanceAltarMap;
    typedef __gnu_cxx::hash_map<int, QuickParty::CQuickParty*> QuickPartyMap;
    typedef __gnu_cxx::hash_map<int, BlueMarble*> BlueMarbleMap;

    uint16_t next_uid_;                              // +0x18
    uint16_t reserved_1a_;                           // +0x1a
    std::set<std::string> test_accounts_;            // +0x1c
    std::set<std::string> test_ips_;                 // +0x34
    StaticPool<CUser, 600> user_pool_;               // +0x4c
    StaticPool<CTradeSpace, 300> trade_space_pool_;  // +0x7c
    StaticPool<CParty, 600> party_pool_;             // +0xac
    StaticPool<PvP_Room, 600> pvp_pool_;             // +0xdc
    StaticPool<WarRoom, 40> war_room_pool_;          // +0x10c
    StaticPool<WongWork::CDeathTower, 600> death_tower_pool_;  // +0x13c
    StaticPool<WongWork::CBossTower, 600> boss_tower_pool_;    // +0x16c
    StaticPool<advancealtar::StageControl, 600> advance_altar_pool_; // +0x19c
    StaticPool<QuickParty::CQuickParty, 300> quick_party_pool_; // +0x1cc
    StaticPool<BlueMarble, 300> blue_marble_pool_;   // +0x1fc
    UserMap users_by_account_;                       // +0x22c
    UserMap users_by_charac_;                        // +0x244
    PvpMap pvp_rooms_;                               // +0x25c
    PartyMap parties_;                               // +0x270
    WarRoomMap war_rooms_;                           // +0x284
    DeathTowerMap death_towers_;                     // +0x298
    BossTowerMap boss_towers_;                       // +0x2ac
    AdvanceAltarMap advance_altars_;                 // +0x2c0
    QuickPartyMap quick_parties_;                    // +0x2d4
    BlueMarbleMap blue_marbles_;                     // +0x2e8
    QuickParty::CQuickPartySystemManager* quick_party_system_manager_; // +0x2fc
    QuickParty::CQuickPartyRewardManager* quick_party_reward_manager_; // +0x300
    CPremiumLetheManager* premium_lethe_manager_;    // +0x304
    CSharedServerMessageManager* shared_server_message_manager_; // +0x308
    CConditionEventManager* condition_event_manager_; // +0x30c
    CSpecialItemRoutingManager* special_item_routing_manager_; // +0x310
    CAuctionAveragePrice* auction_average_price_manager_; // +0x314
    CraneMinigameManager* crane_minigame_manager_;   // +0x318
};

#if defined(__i386__)
static_assert(offsetof(CGameManager, next_uid_) == 0x18u,
              "CGameManager next UID must remain at +0x18");
static_assert(offsetof(CGameManager, test_accounts_) == 0x1cu,
              "CGameManager account test set must remain at +0x1c");
static_assert(offsetof(CGameManager, test_ips_) == 0x34u,
              "CGameManager IP test set must remain at +0x34");
static_assert(offsetof(CGameManager, user_pool_) == 0x4cu,
              "CGameManager user pool must remain at +0x4c");
static_assert(offsetof(CGameManager, trade_space_pool_) == 0x7cu,
              "CGameManager trade-space pool must remain at +0x7c");
static_assert(offsetof(CGameManager, party_pool_) == 0xacu,
              "CGameManager party pool must remain at +0xac");
static_assert(offsetof(CGameManager, pvp_pool_) == 0xdcu,
              "CGameManager PvP pool must remain at +0xdc");
static_assert(offsetof(CGameManager, war_room_pool_) == 0x10cu,
              "CGameManager war-room pool must remain at +0x10c");
static_assert(offsetof(CGameManager, death_tower_pool_) == 0x13cu,
              "CGameManager death-tower pool must remain at +0x13c");
static_assert(offsetof(CGameManager, boss_tower_pool_) == 0x16cu,
              "CGameManager boss-tower pool must remain at +0x16c");
static_assert(offsetof(CGameManager, advance_altar_pool_) == 0x19cu,
              "CGameManager advance-altar pool must remain at +0x19c");
static_assert(offsetof(CGameManager, quick_party_pool_) == 0x1ccu,
              "CGameManager quick-party pool must remain at +0x1cc");
static_assert(offsetof(CGameManager, blue_marble_pool_) == 0x1fcu,
              "CGameManager blue-marble pool must remain at +0x1fc");
static_assert(offsetof(CGameManager, users_by_account_) == 0x22cu,
              "CGameManager account-user map must remain at +0x22c");
static_assert(offsetof(CGameManager, users_by_charac_) == 0x244u,
              "CGameManager character-user map must remain at +0x244");
static_assert(offsetof(CGameManager, pvp_rooms_) == 0x25cu,
              "CGameManager PvP map must remain at +0x25c");
static_assert(offsetof(CGameManager, parties_) == 0x270u,
              "CGameManager party map must remain at +0x270");
static_assert(offsetof(CGameManager, war_rooms_) == 0x284u,
              "CGameManager war-room map must remain at +0x284");
static_assert(offsetof(CGameManager, death_towers_) == 0x298u,
              "CGameManager death-tower map must remain at +0x298");
static_assert(offsetof(CGameManager, boss_towers_) == 0x2acu,
              "CGameManager boss-tower map must remain at +0x2ac");
static_assert(offsetof(CGameManager, advance_altars_) == 0x2c0u,
              "CGameManager advance-altar map must remain at +0x2c0");
static_assert(offsetof(CGameManager, quick_parties_) == 0x2d4u,
              "CGameManager quick-party map must remain at +0x2d4");
static_assert(offsetof(CGameManager, blue_marbles_) == 0x2e8u,
              "CGameManager blue-marble map must remain at +0x2e8");
static_assert(offsetof(CGameManager, quick_party_system_manager_) == 0x2fcu,
              "CGameManager quick-party manager must remain at +0x2fc");
static_assert(offsetof(CGameManager, quick_party_reward_manager_) == 0x300u,
              "CGameManager quick-party reward manager must remain at +0x300");
static_assert(offsetof(CGameManager, premium_lethe_manager_) == 0x304u,
              "CGameManager premium Lethe manager must remain at +0x304");
static_assert(offsetof(CGameManager, shared_server_message_manager_) == 0x308u,
              "CGameManager shared-server manager must remain at +0x308");
static_assert(offsetof(CGameManager, condition_event_manager_) == 0x30cu,
              "CGameManager condition-event manager must remain at +0x30c");
static_assert(offsetof(CGameManager, special_item_routing_manager_) == 0x310u,
              "CGameManager special-item manager must remain at +0x310");
static_assert(offsetof(CGameManager, auction_average_price_manager_) == 0x314u,
              "CGameManager auction-price manager must remain at +0x314");
static_assert(offsetof(CGameManager, crane_minigame_manager_) == 0x318u,
              "CGameManager crane manager must remain at +0x318");
static_assert(sizeof(CGameManager) == 0x31cu,
              "CGameManager must retain its 0x31c-byte i386 layout");
#endif


#endif  // OLDSRV_GLOBAL_GAME_MANAGER_H_
