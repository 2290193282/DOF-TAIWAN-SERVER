#ifndef OLDSRV_OLD_GAME_WORLD_GAME_WORLD_H_
#define OLDSRV_OLD_GAME_WORLD_GAME_WORLD_H_

#include <stdint.h>
#include <stddef.h>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "common/config.h"
#include "common/manager/statistics_shusia_service_abi.h"
#include "old_data/db/core/db_conn_info.h"
#include "old_game/activity/schedule/lottery_info.h"
#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "old_game/world/region/world_area_allocators.h"
#include "old_game/world/region/world_area_types.h"
#include "infra/concurrent/thread/mutex.h"

class CUser;
class CUserModifier;
class PacketGuard;
class CParty;
struct MSG_ITEM_HYPER_LINK_MESSAGE;
enum ENUM_NOTIPACKET : uint16_t;
enum ENUM_POWER_SIDE_TYPE : int;
enum ENUM_POWER_UP_ACTIVATE_TYPE : int;
struct TownAreaScript;
struct TownScript;
struct STMapScript;
enum TownAreaType : int32_t;

// IDA type: BuddyList.  The only recovered consumer is the exact monitor
// reply sender below, which forwards each record as one opaque 42-byte client
// payload.  Keeping it as the real global type preserves the member ABI
// `_ZN9GameWorld20send_buddy_conn_listEjP9BuddyListi` without inventing fields
// the original function never reads.
struct BuddyList {
    uint8_t wire[0x2a];
};

static_assert(sizeof(BuddyList) == 0x2au,
              "BuddyList must retain its IDA wire record size");

// IDA_EA: 0x86C4568
// IDA_NAME: _Z42Is_restrictive_commercial_transaction_zoneiiii
// DEMANGLED: Is_restrictive_commercial_transaction_zone(int,int,int,int)
bool Is_restrictive_commercial_transaction_zone(int center_x,
                                                int center_y,
                                                int pos_x,
                                                int pos_y);

namespace pvp_assault {
class CDisposition;
}

namespace village_object {

class IObject {
public:
    IObject();
    IObject& operator=(const IObject& rhs);

    virtual bool include(const IObject& rhs) const = 0;
    virtual bool put_object_info(PacketGuard& packet) = 0;
};

struct Zone {
    Zone(uint8_t village_value, uint8_t area_value);

    bool operator<(const Zone& rhs) const;

    uint8_t village;
    uint8_t area;
};

class CVillageObjectMgr {
public:
    // IDA_EA: 0x86B6F30
    // IDA_NAME: _ZN14village_object17CVillageObjectMgrC2Ev
    // DEMANGLED: village_object::CVillageObjectMgr::CVillageObjectMgr(void)
    CVillageObjectMgr();

    // IDA_EA: 0x86B6F44
    // IDA_NAME: _ZN14village_object17CVillageObjectMgrD2Ev
    // DEMANGLED: village_object::CVillageObjectMgr::~CVillageObjectMgr()
    ~CVillageObjectMgr();

    // IDA_EA: 0x86B6F58
    // IDA_NAME: _ZN14village_object17CVillageObjectMgr15register_objectENS_4ZoneEPNS_7IObjectE
    // DEMANGLED: village_object::CVillageObjectMgr::register_object(village_object::Zone,village_object::IObject *)
    int register_object(Zone zone, IObject* object);

    // IDA_EA: 0x86B70D0
    // IDA_NAME: _ZN14village_object17CVillageObjectMgr17unregister_objectENS_4ZoneEPNS_7IObjectE
    // DEMANGLED: village_object::CVillageObjectMgr::unregister_object(village_object::Zone,village_object::IObject *)
    int unregister_object(Zone zone, IObject* object);

    // IDA_EA: 0x86B71DA
    // IDA_NAME: _ZN14village_object17CVillageObjectMgr12on_move_areaEP5CUser
    // DEMANGLED: village_object::CVillageObjectMgr::on_move_area(CUser *)
    void on_move_area(CUser* user);

    std::map<Zone, std::list<IObject*> > objects;
};

}  // namespace village_object

// IDA unit: SimpleInfo @0x86D2108.  This is the value type of GameWorld's
// character-name map at +0x8c; it must not be redefined in a world caller.
struct SimpleInfo {
    // IDA_EA: 0x86D2108
    // IDA_NAME: _ZN10SimpleInfoC2Ev
    // DEMANGLED: SimpleInfo::SimpleInfo(void)
    SimpleInfo();

    uint32_t account_id;        // +0x00
    uint8_t forbid_chat;        // +0x04
    uint8_t forbid_move;        // +0x05
    uint16_t reserved;          // +0x06
    uint32_t forbid_chat_until; // +0x08
    uint32_t forbid_move_until; // +0x0c
};

typedef char SimpleInfo_Size_Must_Be_0x10[
    (sizeof(SimpleInfo) == 0x10) ? 1 : -1];

// GameWorld+0x11c owns this exact NAT/peer result directory.  Keeping both
// key and value types here lets every world consumer use the typed member;
// the offset is asserted below rather than repeated in callers.
struct WorldNatTypeKey {
    int32_t nat_type;
    int32_t peer_type;

    bool operator<(const WorldNatTypeKey& rhs) const {
        return nat_type != rhs.nat_type ? nat_type < rhs.nat_type
                                        : peer_type < rhs.peer_type;
    }
};

struct WorldNatTypeCount {
    int32_t failure_count;
    int32_t success_count;
};

typedef std::map<int32_t, int32_t> GameWorldIntCountMap;
typedef std::map<std::string, SimpleInfo> GameWorldSimpleInfoMap;
typedef std::map<WorldNatTypeKey, WorldNatTypeCount*>
    GameWorldNatTypeStatisticsMap;
typedef std::map<uint16_t, CUser*> GameWorldUserMap;
typedef std::map<uint32_t, uint16_t> GameWorldSessionMap;
typedef std::map<uint32_t, CUser*> GameWorldLoginUserMap;

class Area {
public:
    // IDA_EA: 0x86D214C
    // IDA_NAME: _ZN4AreaC2Ev
    // DEMANGLED: Area::Area(void)
    Area();
    // IDA_EA: 0x86D2270
    // IDA_NAME: _ZN4AreaD2Ev
    // DEMANGLED: Area::~Area(void)
    ~Area();
    // IDA_EA: 0x86C1D14
    // IDA_NAME: _ZN4Area8set_areaEiR14TownAreaScriptR11STMapScript
    // DEMANGLED: Area::set_area(int,TownAreaScript &,STMapScript &)
    void set_area(int village_index,
                  TownAreaScript& area_script,
                  STMapScript& map_script);
    int IsExistItemShopNPC();
    int hasNPC(uint32_t npc_id) const;
    int IsMovalbeArea(int target_village, int before_area) const;
    int GetUserCount() const;
    void insert_user(CUser* user);
    void delete_user(CUser* user);
    int erase_user(CUser* user);
    bool send_to_all(PacketGuard& packet);
    // IDA_EA: 0x86C2C1E
    // IDA_NAME: _ZN4Area25make_power_war_hp_of_villER11PacketGuardRi
    // DEMANGLED: Area::make_power_war_hp_of_vill(PacketGuard &,int &)
    bool make_power_war_hp_of_vill(PacketGuard& packet, int& user_count);
    // IDA_EA: 0x86C2F2C
    // IDA_NAME: _ZN4Area11send_to_allER11PacketGuardRSt6vectorIP5CUserSaIS4_EE
    // DEMANGLED: Area::send_to_all(PacketGuard &,std::vector<CUser *,std::allocator<CUser *>> &)
    bool send_to_all(PacketGuard& packet, std::vector<CUser*>& users);
    // IDA_EA: 0x86C2E1E
    // IDA_NAME: _ZN4Area10send_groupER11PacketGuardP7CBelong
    // DEMANGLED: Area::send_group(PacketGuard &,CBelong *)
    bool send_group(PacketGuard& packet, CBelong* group);
    bool get_user_id_list(std::vector<uint16_t>& user_ids);
    int get_area_index();
    int get_world_map_index();

    // Exact Area object layout recovered from C2/D2 (0x86D214C/0x86D2270)
    // and set_area (0x86C1D14).  In particular, town_area_type at +0x68 is
    // consumed by the private-store entrance-gate check; it is an enum
    // subobject, never a byte-buffer projection.
    int32_t area_index;                               // +0x00
    int32_t village_index;                            // +0x04
    int32_t world_map_index;                          // +0x08
    Mutex mutex;                                      // +0x0c
    LotteryInfo lottery_info;                         // +0x24
    std::map<uint16_t, CUser*> users;                 // +0x44
    std::vector<int> npc_ids;                         // +0x5c
    TownAreaType town_area_type;                      // +0x68
    uint8_t private_store_enabled;                    // +0x6c
    uint8_t padding_6d_6f[3];
    std::vector<std::pair<int, int> > npc_positions;  // +0x70
    std::vector<MapArea> map_areas;                   // +0x7c
    std::vector<std::pair<int, int> > movable_links;  // +0x88
    uint8_t has_item_shop_npc;                        // +0x94
    uint8_t padding_95_97[3];
};

#if defined(__i386__)
static_assert(offsetof(Area, lottery_info) == 0x24u &&
                  offsetof(Area, users) == 0x44u &&
                  offsetof(Area, town_area_type) == 0x68u &&
                  offsetof(Area, private_store_enabled) == 0x6cu &&
                  offsetof(Area, npc_positions) == 0x70u &&
                  offsetof(Area, map_areas) == 0x7cu &&
                  offsetof(Area, movable_links) == 0x88u &&
                  offsetof(Area, has_item_shop_npc) == 0x94u &&
                  sizeof(Area) == 0x98u,
              "Area must retain the C2/D2 recovered i386 layout");
#endif

class Village {
public:
    // IDA_EA: 0x86C32E0
    // IDA_NAME: _ZN7VillageC2Ev
    // DEMANGLED: Village::Village(void)
    Village();
    // IDA_EA: 0x86C3300
    // IDA_NAME: _ZN7VillageD2Ev
    // DEMANGLED: Village::~Village(void)
    ~Village();
    // IDA_EA: 0x86C3348
    // IDA_NAME: _ZN7Village7destroyEv
    // DEMANGLED: Village::destroy(void)
    Village* destroy();
    // IDA_EA: 0x86C33B8
    // IDA_NAME: _ZN7Village11set_villageER10TownScript
    // DEMANGLED: Village::set_village(TownScript &)
    int set_village(TownScript& town_script);
    bool has_gate_area() const;
    int get_gate_area();
    Area* getArea(int area_index);
    bool send_to_area(int area_index, PacketGuard& packet);
    bool get_user_id_list(int area_index, std::vector<uint16_t>& user_ids);
    int GetUserCount(int area_index) const;

    // Complete Village layout recovered from C2/D2, set_village and the
    // area-routing family.  Offset +0x04 is not read by the recovered family;
    // retain it as one scalar ABI field instead of projecting a byte buffer.
    int32_t village_index;                         // +0x00
    int32_t reserved_04;                           // +0x04
    std::map<uint16_t, CUser*> users;              // +0x08
    uint8_t requires_cleared_dungeon;              // +0x20
    uint8_t padding_21_23[3];
    int32_t area_count;                            // +0x24
    Area* areas;                                   // +0x28
    int32_t required_level;                        // +0x2c
    int32_t required_quest_id;                     // +0x30

    int get_required_level() const { return required_level; }
    int get_required_quest_id() const { return required_quest_id; }
};

typedef char Village_Size_Must_Be_0x34[(sizeof(Village) == 0x34) ? 1 : -1];

#if defined(__i386__)
static_assert(offsetof(Village, village_index) == 0x00u &&
                  offsetof(Village, users) == 0x08u &&
                  offsetof(Village, requires_cleared_dungeon) == 0x20u &&
                  offsetof(Village, area_count) == 0x24u &&
                  offsetof(Village, areas) == 0x28u &&
                  offsetof(Village, required_level) == 0x2cu &&
                  offsetof(Village, required_quest_id) == 0x30u &&
                  sizeof(Village) == 0x34u,
              "Village must retain the recovered i386 ABI layout");
#endif

// IDA unit: stOccStatisticByAge. GameWorld publishes these one-based age
// buckets through the channel service.
struct stOccStatisticByAge {
    uint16_t count_by_age[100];
};

typedef char stOccStatisticByAge_Size_Must_Be_0xC8[
    (sizeof(stOccStatisticByAge) == 0xc8) ? 1 : -1];

// IDA unit: GameWorld. Storage is process-owned; the C gameplay header keeps
// only verified member ABI declarations and does not invent a field layout.
class GameWorld {
public:
    // IDA_EA: 0x86C3D4A
    // IDA_NAME: _ZN9GameWorldC2Ev
    // DEMANGLED: GameWorld::GameWorld(void)
    GameWorld();
    // IDA_EA: 0x86C3F2C
    // IDA_NAME: _ZN9GameWorldD2Ev
    // DEMANGLED: GameWorld::~GameWorld(void)
    ~GameWorld();
    // IDA_EA: 0x86C40CA
    // IDA_NAME: _ZN9GameWorld7destroyEv
    // DEMANGLED: GameWorld::destroy(void)
    GameWorld* destroy();

    // IDA specializations iterate the canonical +0x134 user map in key order
    // and invoke the supplied gameplay operation for entered users.
    template <typename UserOperation>
    void ProcessAllUsers(UserOperation operation);

    // IDA_EA: 0x82A6898
    // IDA_NAME: _ZN9GameWorld15InitChannelTypeEv
    // DEMANGLED: GameWorld::InitChannelType(void)
    bool InitChannelType();
    // IDA_EA: 0x86C4134
    // IDA_NAME: _ZN9GameWorld4initEv
    // DEMANGLED: GameWorld::init(void)
    int init();

    // IDA_EA: 0x86C46BA
    // IDA_NAME: _ZN9GameWorld15get_dispositionEiiPN11pvp_assault12CDispositionE
    // DEMANGLED: GameWorld::get_disposition(int,int,pvp_assault::CDisposition *)
    int get_disposition(int village_index,
                        int area_index,
                        pvp_assault::CDisposition* disposition);

    // IDA_EA: 0x86C5A84
    // IDA_NAME: _ZN9GameWorld9move_areaEP5CUseriiiiibiii
    // DEMANGLED: GameWorld::move_area(CUser *,int,int,int,int,int,bool,int,int,int)
    int move_area(CUser* user,
                  int village_index,
                  int area_index,
                  int pos_x,
                  int pos_y,
                  int direction,
                  bool move_by_user,
                  int move_type,
                  int gate_village,
                  int gate_area);

    // IDA_EA: 0x86C67A0
    // IDA_NAME: _ZN9GameWorld12goto_dungeonEP5CUser
    // DEMANGLED: GameWorld::goto_dungeon(CUser *)
    void goto_dungeon(CUser* user);

private:
    // IDA_EA: 0x86CD272
    // IDA_NAME: _ZN9GameWorld15_GetChannelTypeEv
    // DEMANGLED: GameWorld::_GetChannelType(void)
    int _GetChannelType();

public:

    // IDA_EA: 0x86D1002
    // IDA_NAME: _ZN9GameWorld15check_move_areaEP5CUseriiiiibi
    // DEMANGLED: GameWorld::check_move_area(CUser *,int,int,int,int,int,bool,int)
    int check_move_area(CUser* user,
                        int village_index,
                        int before_area,
                        int pos_x,
                        int pos_y,
                        int direction,
                        bool move_by_user,
                        int move_type);

    // IDA_EA: 0x8116B20
    // IDA_NAME: _ZNK9GameWorld14GetChannelTypeEv
    // DEMANGLED: GameWorld::GetChannelType(void)const
    int GetChannelType() const;

    // IDA_EA: 0x86C5288
    // IDA_NAME: _ZN9GameWorld16leave_game_worldEP5CUser
    // DEMANGLED: GameWorld::leave_game_world(CUser *)
    int leave_game_world(CUser* user);

    // IDA_EA: 0x86C6A16
    // IDA_NAME: _ZN9GameWorld16out_from_dungeonEP5CUser
    // DEMANGLED: GameWorld::out_from_dungeon(CUser *)
    void out_from_dungeon(CUser* user);

    // IDA_EA: 0x86C5706
    // IDA_NAME: _ZN9GameWorld13move_positionEP5CUseriiit
    // DEMANGLED: GameWorld::move_position(CUser *,int,int,int,unsigned short)
    void move_position(CUser* user,
                       int pos_x,
                       int pos_y,
                       int direction,
                       uint16_t position_context);

    // IDA_EA: 0x86C6CEE
    // IDA_NAME: _ZN9GameWorld8goto_pvpEP5CUser
    // DEMANGLED: GameWorld::goto_pvp(CUser *)
    void goto_pvp(CUser* user);
    // IDA_EA: 0x86C6FAE
    // IDA_NAME: _ZN9GameWorld12out_from_pvpEP5CUser
    // DEMANGLED: GameWorld::out_from_pvp(CUser *)
    void out_from_pvp(CUser* user);
    // IDA_EA: 0x86C7192
    // IDA_NAME: _ZN9GameWorld12goto_warroomEP5CUser
    // DEMANGLED: GameWorld::goto_warroom(CUser *)
    void goto_warroom(CUser* user);
    // IDA_EA: 0x86C7452
    // IDA_NAME: _ZN9GameWorld16out_from_warroomEP5CUser
    // DEMANGLED: GameWorld::out_from_warroom(CUser *)
    void out_from_warroom(CUser* user);
    // IDA_EA: 0x86C7654
    // IDA_NAME: _ZN9GameWorld14gotoBlueMarbleEP5CUser
    // DEMANGLED: GameWorld::gotoBlueMarble(CUser *)
    void gotoBlueMarble(CUser* user);
    // IDA_EA: 0x86C7914
    // IDA_NAME: _ZN9GameWorld17outFromBlueMarbleEP5CUser
    // DEMANGLED: GameWorld::outFromBlueMarble(CUser *)
    void outFromBlueMarble(CUser* user);
    // IDA_EA: 0x86C4B9C
    // IDA_NAME: _ZN9GameWorld15find_from_worldEt
    // DEMANGLED: GameWorld::find_from_world(unsigned short)
    CUser* find_from_world(uint16_t user_uid);

    // IDA_EA: 0x86C7F10
    // IDA_NAME: _ZN9GameWorld12send_to_userEiR11PacketGuard
    // DEMANGLED: GameWorld::send_to_user(int,PacketGuard &)
    void send_to_user(int user_uid, PacketGuard& packet);

    // IDA_EA: 0x86C7F48
    // IDA_NAME: _ZN9GameWorld12send_to_villEiR11PacketGuard
    // DEMANGLED: GameWorld::send_to_vill(int,PacketGuard &)
    void send_to_vill(int village_index, PacketGuard& packet);

    // IDA_EA: 0x86C4B0A
    // IDA_NAME: _ZN9GameWorld14make_unique_idEv
    // DEMANGLED: GameWorld::make_unique_id(void)
    int make_unique_id();
    // IDA_EA: 0x86C4C24
    // IDA_NAME: _ZN9GameWorld12find_sessionEj
    // DEMANGLED: GameWorld::find_session(unsigned int)
    uint16_t find_session(unsigned int account_id);
    // IDA_EA: 0x86C4CAA
    // IDA_NAME: _ZN9GameWorld33find_user_from_world_byaccid_loopEj
    // DEMANGLED: GameWorld::find_user_from_world_byaccid_loop(unsigned int)
    CUser* find_user_from_world_byaccid_loop(unsigned int account_id);
    // IDA_EA: 0x86C4D40
    // IDA_NAME: _ZN9GameWorld28find_user_from_world_byaccidEj
    // DEMANGLED: GameWorld::find_user_from_world_byaccid(unsigned int)
    CUser* find_user_from_world_byaccid(unsigned int account_id);

    // IDA_EA: 0x86C8694
    // IDA_NAME: _ZN9GameWorld20send_buddy_conn_listEjP9BuddyListi
    // DEMANGLED: GameWorld::send_buddy_conn_list(unsigned int,BuddyList *,int)
    void send_buddy_conn_list(unsigned int account_id,
                              BuddyList* buddy_list,
                              int count);
    // IDA_EA: 0x86C4E0C
    // IDA_NAME: _ZN9GameWorld14CheckUserCountEi
    // DEMANGLED: GameWorld::CheckUserCount(int)
    bool CheckUserCount(int additional_user_count);
    // IDA_EA: 0x86C4E50
    // IDA_NAME: _ZN9GameWorld16reach_game_worldEP5CUser
    // DEMANGLED: GameWorld::reach_game_world(CUser *)
    int reach_game_world(CUser* user);

    // IDA_EA: 0x86CF59C
    // IDA_NAME: _ZN9GameWorld15InsertLoginUserEP5CUser
    // DEMANGLED: GameWorld::InsertLoginUser(CUser *)
    int InsertLoginUser(CUser* user);
    // IDA_EA: 0x86CF602
    // IDA_NAME: _ZN9GameWorld14EraseLoginUserEP5CUser
    // DEMANGLED: GameWorld::EraseLoginUser(CUser *)
    bool EraseLoginUser(CUser* user);

    // IDA_EA: 0x86CEBE0
    // IDA_NAME: _ZN9GameWorld16GetWorldMapIndexEP5CUser
    // DEMANGLED: GameWorld::GetWorldMapIndex(CUser *)
    int GetWorldMapIndex(CUser* user);

    // IDA_EA: 0x86CEC84
    // IDA_NAME: _ZN9GameWorld21isDungeonEntranceAreaEP5CUser
    // DEMANGLED: GameWorld::isDungeonEntranceArea(CUser *)
    bool isDungeonEntranceArea(CUser* user);

    // IDA_EA: 0x86CAD68
    // IDA_NAME: _ZN9GameWorld13get_user_infoEP5CUserii
    // DEMANGLED: GameWorld::get_user_info(CUser *,int,int)
    void get_user_info(CUser* requester, int target_uid, int detail_type);

    // IDA_EA: 0x86C9308
    // IDA_NAME: _ZN9GameWorld11modify_userEP13CUserModifier
    // DEMANGLED: GameWorld::modify_user(CUserModifier *)
    bool modify_user(CUserModifier* modifier);
    // IDA_EA: 0x86CAB92
    // IDA_NAME: _ZN9GameWorld31check_peerTypemsg_sameWorldUserEP5CUserS1_cPci
    // DEMANGLED: GameWorld::check_peerTypemsg_sameWorldUser(CUser *,CUser *,char,char *,int)
    void check_peerTypemsg_sameWorldUser(CUser* from_user,
                                         CUser* to_user,
                                         char chat_type,
                                         char* message,
                                         int message_len);
    // IDA_EA: 0x86CAFAA
    // IDA_NAME: _ZN9GameWorld13arrange_usersEv
    // DEMANGLED: GameWorld::arrange_users(void)
    void arrange_users();

    // IDA_EA: 0x86C83FC
    // IDA_NAME: _ZN9GameWorld17send_AllBasicInfoEP5CUser
    // DEMANGLED: GameWorld::send_AllBasicInfo(CUser *)
    void send_AllBasicInfo(CUser* user);

    // IDA_EA: 0x80F131E
    // IDA_NAME: _ZNK9GameWorld26IsIntegratedPvPBaseChannelEv
    // DEMANGLED: GameWorld::IsIntegratedPvPBaseChannel(void)const
    bool IsIntegratedPvPBaseChannel() const;
    // IDA_EA: 0x813495C
    // IDA_NAME: _ZN9GameWorld8send_allER11PacketGuardP5CUser
    // DEMANGLED: GameWorld::send_all(PacketGuard &,CUser *)
    int send_all(PacketGuard& packet, CUser* user);
    // IDA_EA: 0x86C975E
    // IDA_NAME: _ZN9GameWorld13send_chat_msgEP5CUserctjPciS2_i15ENUM_NOTIPACKET
    // DEMANGLED: GameWorld::send_chat_msg(CUser *,char,unsigned short,unsigned int,char *,int,char *,int,ENUM_NOTIPACKET)
    void send_chat_msg(CUser* user,
                       char chat_type,
                       uint16_t item_space,
                       uint32_t item_position,
                       char* message,
                       int message_length,
                       char* target_name,
                       int target_name_length,
                       ENUM_NOTIPACKET notification);
    // IDA_EA: 0x86CFA10
    // IDA_NAME: _ZN9GameWorld24send_chat_msg_hyper_linkEP5CUserR27MSG_ITEM_HYPER_LINK_MESSAGE15ENUM_NOTIPACKET
    // DEMANGLED: GameWorld::send_chat_msg_hyper_link(CUser *,MSG_ITEM_HYPER_LINK_MESSAGE &,ENUM_NOTIPACKET)
    void send_chat_msg_hyper_link(CUser* user,
                                  MSG_ITEM_HYPER_LINK_MESSAGE& message,
                                  ENUM_NOTIPACKET notification);
    // IDA_EA: 0x86D0EA4
    // IDA_NAME: _ZN9GameWorld31make_packet_chat_msg_hyper_linkER11PacketGuardP5CUserR27MSG_ITEM_HYPER_LINK_MESSAGE15ENUM_NOTIPACKET
    // DEMANGLED: GameWorld::make_packet_chat_msg_hyper_link(PacketGuard &,CUser *,MSG_ITEM_HYPER_LINK_MESSAGE &,ENUM_NOTIPACKET)
    int make_packet_chat_msg_hyper_link(PacketGuard& packet,
                                        CUser* user,
                                        MSG_ITEM_HYPER_LINK_MESSAGE& message,
                                        ENUM_NOTIPACKET notification);
    // IDA_EA: 0x81424E8
    // IDA_NAME: _ZNK9GameWorld12IsPVPChannelEv
    // DEMANGLED: GameWorld::IsPVPChannel(void)const
    bool IsPVPChannel() const;
    // IDA_EA: 0x81A0AB2
    // IDA_NAME: _ZNK9GameWorld16IsFreePvPChannelEv
    // DEMANGLED: GameWorld::IsFreePvPChannel(void)const
    bool IsFreePvPChannel() const;
    // IDA_EA: 0x82343BA
    // IDA_NAME: _ZN9GameWorld20GetStatisticsNatTypeEv
    // DEMANGLED: GameWorld::GetStatisticsNatType(void)
    char* GetStatisticsNatType();
    // IDA_EA: 0x82343C8
    // IDA_NAME: _ZN9GameWorld21IsSameTeamChannelJoinEv
    // DEMANGLED: GameWorld::IsSameTeamChannelJoin(void)
    bool IsSameTeamChannelJoin();
    // IDA_EA: 0x82343FC
    // IDA_NAME: _ZN9GameWorld24IsEnchantRevisionChannelEv
    // DEMANGLED: GameWorld::IsEnchantRevisionChannel(void)
    bool IsEnchantRevisionChannel();
    // IDA_EA: 0x823441E
    // IDA_NAME: _ZNK9GameWorld20IsPvPSkilTreeChannelEv
    // DEMANGLED: GameWorld::IsPvPSkilTreeChannel(void)const
    bool IsPvPSkilTreeChannel() const;
    // IDA_EA: 0x823443E
    // IDA_NAME: _ZNK9GameWorld24IsEquipSlotSwitchChannelEv
    // DEMANGLED: GameWorld::IsEquipSlotSwitchChannel(void)const
    bool IsEquipSlotSwitchChannel() const;
    // IDA_EA: 0x844DC26
    // IDA_NAME: _ZN9GameWorld27getStatisticsToObserveCrashEv
    // DEMANGLED: GameWorld::getStatisticsToObserveCrash(void)
    char* getStatisticsToObserveCrash();
    // IDA_EA: 0x844DC32
    // IDA_NAME: _ZN9GameWorld26getStatisticsShusiaServiceEv
    // DEMANGLED: GameWorld::getStatisticsShusiaService(void)
    StatisticsShusiaService* getStatisticsShusiaService();
    // IDA_EA: 0x844DC40
    // IDA_NAME: _ZN9GameWorld16getStatisticsPvPEv
    // DEMANGLED: GameWorld::getStatisticsPvP(void)
    StatisticsPvP* getStatisticsPvP();
    // IDA_EA: 0x844DC4E
    // IDA_NAME: _ZNK9GameWorld18IsSchoolPvPChannelEv
    // DEMANGLED: GameWorld::IsSchoolPvPChannel(void)const
    bool IsSchoolPvPChannel() const;
    // IDA_EA: 0x84ED128
    // IDA_NAME: _ZNK9GameWorld31IsCharacterLevelRevisionChannelEv
    // DEMANGLED: GameWorld::IsCharacterLevelRevisionChannel(void)const
    bool IsCharacterLevelRevisionChannel() const;
    // IDA_EA: 0x85DFA76
    // IDA_NAME: _ZNK9GameWorld32IsCharacterPvPExpRevisionChannelEv
    // DEMANGLED: GameWorld::IsCharacterPvPExpRevisionChannel(void)const
    bool IsCharacterPvPExpRevisionChannel() const;
    // IDA_EA: 0x85DFA92
    // IDA_NAME: _ZNK9GameWorld20IsWinPointPvPChannelEv
    // DEMANGLED: GameWorld::IsWinPointPvPChannel(void)const
    bool IsWinPointPvPChannel() const;
    // IDA_EA: 0x869779E
    // IDA_NAME: _ZN9GameWorld31IsCheckUnusableEquipmentChannelEv
    // DEMANGLED: GameWorld::IsCheckUnusableEquipmentChannel(void)
    bool IsCheckUnusableEquipmentChannel();
    // IDA_EA: 0x86977BA
    // IDA_NAME: _ZNK9GameWorld22IsPvPVillageMapChannelEv
    // DEMANGLED: GameWorld::IsPvPVillageMapChannel(void)const
    bool IsPvPVillageMapChannel() const;
    // IDA_EA: 0x86977DA
    // IDA_NAME: _ZNK9GameWorld30IsCharacterStatRevisionChannelEv
    // DEMANGLED: GameWorld::IsCharacterStatRevisionChannel(void)const
    bool IsCharacterStatRevisionChannel() const;
    // IDA_EA: 0x86C4550
    // IDA_NAME: _ZN9GameWorld21get_UserCount_InWorldEv
    // DEMANGLED: GameWorld::get_UserCount_InWorld(void)
    int get_UserCount_InWorld();

    // IDA_EA: 0x86CD568
    // IDA_NAME: _ZN9GameWorld17InsertUserToIDMapEPKcj
    // DEMANGLED: GameWorld::InsertUserToIDMap(char const*,unsigned int)
    bool InsertUserToIDMap(const char* charac_name, unsigned int account_id);
    // IDA_EA: 0x86CD6B8
    // IDA_NAME: _ZN9GameWorld17DeleteUserInIDMapEPKc
    // DEMANGLED: GameWorld::DeleteUserInIDMap(char const*)
    bool DeleteUserInIDMap(const char* charac_name);
    // IDA_EA: 0x86CD7E2
    // IDA_NAME: _ZN9GameWorld16GetIDUserInIDMapEPKc
    // DEMANGLED: GameWorld::GetIDUserInIDMap(char const*)
    unsigned int GetIDUserInIDMap(const char* charac_name);
    // IDA_EA: 0x86CD908
    // IDA_NAME: _ZN9GameWorld15IsForbiddenChatEPKc
    // DEMANGLED: GameWorld::IsForbiddenChat(char const*)
    int IsForbiddenChat(const char* charac_name);
    // IDA_EA: 0x86CDC60
    // IDA_NAME: _ZN9GameWorld10EnableChatEPKc
    // DEMANGLED: GameWorld::EnableChat(char const*)
    int EnableChat(const char* charac_name);
    // IDA_EA: 0x86CE054
    // IDA_NAME: _ZN9GameWorld11DisableChatEPKci
    // DEMANGLED: GameWorld::DisableChat(char const*,int)
    int DisableChat(const char* charac_name, int seconds);
    // IDA_EA: 0x86CE4A0
    // IDA_NAME: _ZN9GameWorld27WhenIsTheEndOfForbiddenChatEPKc
    // DEMANGLED: GameWorld::WhenIsTheEndOfForbiddenChat(char const*)
    unsigned int WhenIsTheEndOfForbiddenChat(const char* charac_name);
    // IDA_EA: 0x86CE73C
    // IDA_NAME: _ZN9GameWorld26GetLeftTimeOfForbiddenChatEPKc
    // DEMANGLED: GameWorld::GetLeftTimeOfForbiddenChat(char const*)
    unsigned int GetLeftTimeOfForbiddenChat(const char* charac_name);
    // IDA_EA: 0x86CDAB4
    // IDA_NAME: _ZN9GameWorld15IsForbiddenMoveEPKc
    // DEMANGLED: GameWorld::IsForbiddenMove(char const*)
    int IsForbiddenMove(const char* charac_name);
    // IDA_EA: 0x86CDE5A
    // IDA_NAME: _ZN9GameWorld10EnableMoveEPKc
    // DEMANGLED: GameWorld::EnableMove(char const*)
    int EnableMove(const char* charac_name);
    // IDA_EA: 0x86CE27A
    // IDA_NAME: _ZN9GameWorld11DisableMoveEPKci
    // DEMANGLED: GameWorld::DisableMove(char const*,int)
    int DisableMove(const char* charac_name, int seconds);
    // IDA_EA: 0x86CE5EE
    // IDA_NAME: _ZN9GameWorld27WhenIsTheEndOfForbiddenMoveEPKc
    // DEMANGLED: GameWorld::WhenIsTheEndOfForbiddenMove(char const*)
    unsigned int WhenIsTheEndOfForbiddenMove(const char* charac_name);
    // IDA_EA: 0x86CE958
    // IDA_NAME: _ZN9GameWorld26GetLeftTimeOfForbiddenMoveEPKc
    // DEMANGLED: GameWorld::GetLeftTimeOfForbiddenMove(char const*)
    unsigned int GetLeftTimeOfForbiddenMove(const char* charac_name);

    // IDA_EA: 0x86C8C14
    // IDA_NAME: _ZN9GameWorld8send_allER11PacketGuard
    // DEMANGLED: GameWorld::send_all(PacketGuard &)
    bool send_all(PacketGuard& packet);
    // IDA_EA: 0x86C8008
    // IDA_NAME: _ZN9GameWorld25send_power_war_hp_of_villEi
    // DEMANGLED: GameWorld::send_power_war_hp_of_vill(int)
    void send_power_war_hp_of_vill(int village_index);
    // IDA_EA: 0x86C8CC4
    // IDA_NAME: _ZN9GameWorld30send_all_dungeon_inout_messageEi
    // DEMANGLED: GameWorld::send_all_dungeon_inout_message(int)
    // Reads the current open-state from ServerParameterScript immediately
    // before constructing CMD295.  The timed in/out callbacks use this
    // overload; callers that already own the state use the Eib form below.
    void send_all_dungeon_inout_message(int dungeon_index);
    // IDA_EA: 0x86C8E4E
    // IDA_NAME: _ZN9GameWorld30send_all_dungeon_inout_messageEib
    // DEMANGLED: GameWorld::send_all_dungeon_inout_message(int,bool)
    void send_all_dungeon_inout_message(int dungeon_index, bool open);
    // IDA_EA: 0x86C8FC8
    // IDA_NAME: _ZN9GameWorld31send_user_dungeon_inout_messageEP5CUserib
    // DEMANGLED: GameWorld::send_user_dungeon_inout_message(CUser *,int,bool)
    void send_user_dungeon_inout_message(CUser* user,
                                         int dungeon_index,
                                         bool open);
    // IDA_EA: 0x86C9076
    // IDA_NAME: _ZN9GameWorld30getDungeonMinimumRequiredLevelEi
    // DEMANGLED: GameWorld::getDungeonMinimumRequiredLevel(int)
    int getDungeonMinimumRequiredLevel(int dungeon_index);
    // IDA_EA: 0x86CC340
    // IDA_NAME: _ZN9GameWorld13ResetMidnightEv
    // DEMANGLED: GameWorld::ResetMidnight(void)
    bool ResetMidnight();
    // IDA_EA: 0x86CC3EC
    // IDA_NAME: _ZN9GameWorld15ResetAllFatigueEv
    // DEMANGLED: GameWorld::ResetAllFatigue(void)
    int ResetAllFatigue();
    // IDA_EA: 0x86CC8AC
    // IDA_NAME: _ZN9GameWorld21ResetAllCleadpadPointEv
    // DEMANGLED: GameWorld::ResetAllCleadpadPoint(void)
    int ResetAllCleadpadPoint();
    // IDA_EA: 0x86CC992
    // IDA_NAME: _ZN9GameWorld24ResetAllPassPadFailCountEv
    // DEMANGLED: GameWorld::ResetAllPassPadFailCount(void)
    int ResetAllPassPadFailCount();
    // IDA_EA: 0x86CCB22
    // IDA_NAME: _ZN9GameWorld13ResetAllCoinsEv
    // DEMANGLED: GameWorld::ResetAllCoins(void)
    bool ResetAllCoins();
    // IDA_EA: 0x86CCC8E
    // IDA_NAME: _ZN9GameWorld15DeleteDailyItemEv
    // DEMANGLED: GameWorld::DeleteDailyItem(void)
    int DeleteDailyItem();
    // IDA_EA: 0x86CCDCE
    // IDA_NAME: _ZN9GameWorld12AddDailyItemEv
    // DEMANGLED: GameWorld::AddDailyItem(void)
    bool AddDailyItem();
    // IDA_EA: 0x86CCF58
    // IDA_NAME: _ZN9GameWorld14resetDailyDataEv
    // DEMANGLED: GameWorld::resetDailyData(void)
    bool resetDailyData();
    // IDA_EA: 0x86CD004
    // IDA_NAME: _ZN9GameWorld16DailyEventModifyEv
    // DEMANGLED: GameWorld::DailyEventModify(void)
    void DailyEventModify();
    // IDA_EA: 0x86CD010
    // IDA_NAME: _ZN9GameWorld18ReselectDailyQuestEv
    // DEMANGLED: GameWorld::ReselectDailyQuest(void)
    bool ReselectDailyQuest();
    // IDA_EA: 0x86CCC78
    // IDA_NAME: _ZN9GameWorld17ResetAllEventDataEv
    // DEMANGLED: GameWorld::ResetAllEventData(void)
    bool ResetAllEventData();
    // IDA_EA: 0x86CCBF4
    // IDA_NAME: _ZN9GameWorld19deleteSpecificItemsEPKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: GameWorld::deleteSpecificItems(std::vector<std::pair<int,int>> const*)
    bool deleteSpecificItems(
        const std::vector<std::pair<int, int> >* matched_items);
    // IDA_EA: 0x86D12B2
    // IDA_NAME: _ZN9GameWorld20SetWeekendBounsEventEv
    // DEMANGLED: GameWorld::SetWeekendBounsEvent(void)
    int SetWeekendBounsEvent();
    // IDA_EA: 0x86CEB08
    // IDA_NAME: _ZN9GameWorld25ResetEntireReliablePersonEv
    // DEMANGLED: GameWorld::ResetEntireReliablePerson(void)
    bool ResetEntireReliablePerson();
    // IDA_EA: 0x86CF684
    // IDA_NAME: _ZN9GameWorld25ResetCacheCharactorMemoryEv
    // DEMANGLED: GameWorld::ResetCacheCharactorMemory(void)
    bool ResetCacheCharactorMemory();
    // IDA_EA: 0x86CF746
    // IDA_NAME: _ZN9GameWorld25ResetCacheCharactorMemoryEj
    // DEMANGLED: GameWorld::ResetCacheCharactorMemory(unsigned int)
    int ResetCacheCharactorMemory(unsigned int account_id);
    // IDA_EA: 0x86CF7F8
    // IDA_NAME: _ZN9GameWorld20ResetOneADayItemShopEv
    // DEMANGLED: GameWorld::ResetOneADayItemShop(void)
    char ResetOneADayItemShop();
    // IDA_EA: 0x86CF904
    // IDA_NAME: _ZN9GameWorld24testResetOneADayItemShopEv
    // DEMANGLED: GameWorld::testResetOneADayItemShop(void)
    char testResetOneADayItemShop();
    // IDA_EA: 0x86C878A
    // IDA_NAME: _ZN9GameWorld22send_party_info_to_allEP6CPartyi
    // DEMANGLED: GameWorld::send_party_info_to_all(CParty *,int)
    void send_party_info_to_all(CParty* party, int info_type);
    // IDA_EA: 0x86C9234
    // IDA_NAME: _ZN9GameWorld10send_groupER11PacketGuardP7CBelong
    // DEMANGLED: GameWorld::send_group(PacketGuard &,CBelong *)
    bool send_group(PacketGuard& packet, CBelong* group);
    // IDA_EA: 0x86CBBF6
    // IDA_NAME: _ZN9GameWorld31is_dungeon_tag_matching_channelEPc
    // DEMANGLED: GameWorld::is_dungeon_tag_matching_channel(char *)
    int is_dungeon_tag_matching_channel(char* tag);
    // IDA_EA: 0x86C90AE
    // IDA_NAME: _ZN9GameWorld17send_server_groupER11PacketGuardc
    // DEMANGLED: GameWorld::send_server_group(PacketGuard &,char)
    bool send_server_group(PacketGuard& packet, char server_group);
    // IDA_EA: 0x86C9184
    // IDA_NAME: _ZN9GameWorld19send_all_with_stateER11PacketGuardi
    // DEMANGLED: GameWorld::send_all_with_state(PacketGuard &,int)
    bool send_all_with_state(PacketGuard& packet, int state);

    // IDA_EA: 0x86D13BA
    // IDA_NAME: _ZN9GameWorld26ResetCraneItemNeedMaterialEi
    // DEMANGLED: GameWorld::ResetCraneItemNeedMaterial(int)
    bool ResetCraneItemNeedMaterial(int rotation_seed);
    // IDA_EA: 0x86D14D4
    // IDA_NAME: _ZN9GameWorld20UpdateServerSnapShotEv
    // DEMANGLED: GameWorld::UpdateServerSnapShot(void)
    bool UpdateServerSnapShot();
    // IDA_EA: 0x86C93A2
    // IDA_NAME: _ZN9GameWorld22send_all_excepted_userER11PacketGuardP5CUser
    // DEMANGLED: GameWorld::send_all_excepted_user(PacketGuard &,CUser *)
    bool send_all_excepted_user(PacketGuard& packet, CUser* except_user);
    // IDA_EA: 0x86C9464
    // IDA_NAME: _ZN9GameWorld24find_user_by_charac_nameEPKc
    // DEMANGLED: GameWorld::find_user_by_charac_name(char const*)
    CUser* find_user_by_charac_name(const char* charac_name);
    // IDA_EA: 0x86C954C
    // IDA_NAME: _ZN9GameWorld20make_packet_chat_msgER11PacketGuardP5CUserPcic15ENUM_NOTIPACKET
    // DEMANGLED: GameWorld::make_packet_chat_msg(PacketGuard &,CUser *,char *,int,char,ENUM_NOTIPACKET)
    int make_packet_chat_msg(PacketGuard& packet,
                             CUser* user,
                             char* message,
                             int message_len,
                             char chat_type,
                             ENUM_NOTIPACKET noti_packet);
    // IDA_EA: 0x86C9638
    // IDA_NAME: _ZN9GameWorld15WriteLogChatMsgEP5CUsercPc
    // DEMANGLED: GameWorld::WriteLogChatMsg(CUser *,char,char *)
    char WriteLogChatMsg(CUser* user, char chat_type, char* message);
    // IDA_EA: 0x86D2BA4
    // IDA_NAME: _ZN9GameWorld30send_peerTypemsg_sameWorldUserEP5CUserS1_ciiPKci
    // DEMANGLED: GameWorld::send_peerTypemsg_sameWorldUser(CUser *,CUser *,char,int,int,char const*,int)
    void send_peerTypemsg_sameWorldUser(CUser* from_user,
                                        CUser* to_user,
                                        char chat_type,
                                        int result_code,
                                        int peer_type,
                                        const char* message,
                                        int message_len);

    // IDA_EA: 0x86C471E
    // IDA_NAME: _ZNK9GameWorld21get_area_center_pointEiiRiS0_
    // DEMANGLED: GameWorld::get_area_center_point(int,int,int &,int &)const
    bool get_area_center_point(int village_index,
                               int area_index,
                               int& out_x,
                               int& out_y) const;
    // IDA_EA: 0x86C4814
    // IDA_NAME: _ZNK9GameWorld18is_available_pointEiiii
    // DEMANGLED: GameWorld::is_available_point(int,int,int,int)const
    bool is_available_point(int village_index,
                            int area_index,
                            int pos_x,
                            int pos_y) const;
    // IDA_EA: 0x86C497C
    // IDA_NAME: _ZNK9GameWorld13get_gate_areaEi
    // DEMANGLED: GameWorld::get_gate_area(int)const
    int get_gate_area(int village_index) const;
    // IDA_EA: 0x86C49B2
    // IDA_NAME: _ZNK9GameWorld12GetUserCountEii
    // DEMANGLED: GameWorld::GetUserCount(int,int)const
    int GetUserCount(int village_index, int area_index) const;
    // IDA_EA: 0x86C49EE
    // IDA_NAME: _ZN9GameWorld16check_valid_areaEii
    // DEMANGLED: GameWorld::check_valid_area(int,int)
    bool check_valid_area(int village_index, int area_index);
    // IDA_EA: 0x86C45AA
    // IDA_NAME: _ZN9GameWorld45check_restrictive_commercial_transaction_zoneEiiii
    // DEMANGLED: GameWorld::check_restrictive_commercial_transaction_zone(int,int,int,int)
    int check_restrictive_commercial_transaction_zone(int village_index,
                                                      int area_index,
                                                      int pos_x,
                                                      int pos_y);
    // IDA_EA: 0x86C7AF8
    // IDA_EA: 0x86C592C
    // IDA_NAME: _ZN9GameWorld18IsPowerWarSafeZoneEii20ENUM_POWER_SIDE_TYPE
    // DEMANGLED: GameWorld::IsPowerWarSafeZone(int,int,ENUM_POWER_SIDE_TYPE)
    bool IsPowerWarSafeZone(int village_index,
                            int area_index,
                            ENUM_POWER_SIDE_TYPE power_side);
    // IDA_EA: 0x86C59DE
    // IDA_NAME: _ZN9GameWorld18IsPowerWarSafeZoneEii
    // DEMANGLED: GameWorld::IsPowerWarSafeZone(int,int)
    bool IsPowerWarSafeZone(int village_index, int area_index);
    // IDA_EA: 0x86CD084
    // IDA_NAME: _ZN9GameWorld14getUserVillageEi
    // DEMANGLED: GameWorld::getUserVillage(int)
    Village* getUserVillage(int village_index);
    // IDA_EA: 0x86CD038
    // IDA_NAME: _ZN9GameWorld14getUserVillageEP5CUser
    // DEMANGLED: GameWorld::getUserVillage(CUser *)
    Village* getUserVillage(CUser* user);
    // IDA_EA: 0x86D1764
    // IDA_NAME: _ZN9GameWorld10GetVillageEi
    // DEMANGLED: GameWorld::GetVillage(int)
    Village* GetVillage(int village_index);
    // IDA_EA: 0x86C8168
    // IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuard
    // DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &)
    void send_to_area(int village_index, int area_index, PacketGuard& packet);
    // IDA_EA: 0x86C81F8
    // IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuardP7CBelong
    // DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &,CBelong *)
    void send_to_area(int village_index,
                      int area_index,
                      PacketGuard& packet,
                      CBelong* group);
    // IDA_EA: 0x86C828E
    // IDA_NAME: _ZN9GameWorld12send_to_areaEiiR11PacketGuardRSt6vectorIP5CUserSaIS4_EE
    // DEMANGLED: GameWorld::send_to_area(int,int,PacketGuard &,
    //                                   std::vector<CUser *> &)
    bool send_to_area(int village_index,
                      int area_index,
                      PacketGuard& packet,
                      std::vector<CUser*>& touched_users);
    // IDA_EA: 0x86C8316
    bool get_user_id_list(int village_index,
                           int area_index,
                           std::vector<uint16_t>& user_ids);

    // IDA_EA: 0x86CB17C
    // IDA_NAME: _ZN9GameWorld30updateDetailChannelServiceInfoEv
    // DEMANGLED: GameWorld::updateDetailChannelServiceInfo(void)
    int updateDetailChannelServiceInfo();
    // IDA_EA: 0x86CB61A
    // IDA_NAME: _ZN9GameWorld19GetChannelUserCountE17ENUM_SERVER_GROUPR19stOccStatisticByAge
    // DEMANGLED: GameWorld::GetChannelUserCount(ENUM_SERVER_GROUP,stOccStatisticByAge &)
    int GetChannelUserCount(ENUM_SERVER_GROUP server_group,
                            stOccStatisticByAge& count_by_age);
    // IDA_EA: 0x86CB72A
    // IDA_NAME: _ZN9GameWorld17UpdateChannelUserEv
    // DEMANGLED: GameWorld::UpdateChannelUser(void)
    void UpdateChannelUser();
    // IDA_EA: 0x86CD0B0
    // IDA_NAME: _ZN9GameWorld30UpdateStatistic4ChannelUserLevEv
    // DEMANGLED: GameWorld::UpdateStatistic4ChannelUserLev(void)
    void UpdateStatistic4ChannelUserLev();
    // IDA_EA: 0x86CD2F6
    // IDA_NAME: _ZN9GameWorld17UpdateNatTypeUserEv
    // DEMANGLED: GameWorld::UpdateNatTypeUser(void)
    int UpdateNatTypeUser();
    // IDA_EA: 0x86CB88A
    // IDA_NAME: _ZN9GameWorld13InsertChannelEi17ENUM_SERVER_GROUP
    // DEMANGLED: GameWorld::InsertChannel(int,ENUM_SERVER_GROUP)
    void InsertChannel(int game_channel_no, ENUM_SERVER_GROUP server_group);
    // IDA_EA: 0x86CBABA
    // IDA_NAME: _ZN9GameWorld13DeleteChannelEi17ENUM_SERVER_GROUP
    // DEMANGLED: GameWorld::DeleteChannel(int,ENUM_SERVER_GROUP)
    void DeleteChannel(int game_channel_no, ENUM_SERVER_GROUP server_group);

    // IDA_EA: 0x86CEDA0
    // IDA_NAME: _ZN9GameWorld13OnEndPowerWarER11PacketGuard
    // DEMANGLED: GameWorld::OnEndPowerWar(PacketGuard &)
    int OnEndPowerWar(PacketGuard& packet);
    // IDA_EA: 0x86CEDF4
    // IDA_NAME: _ZN9GameWorld28OnRefreshPowerWarProcessInfoEv
    // DEMANGLED: GameWorld::OnRefreshPowerWarProcessInfo(void)
    bool OnRefreshPowerWarProcessInfo();
    // IDA_EA: 0x86CEF36
    // IDA_NAME: _ZN9GameWorld23OnBanAbuserFromPowerWarEv
    // DEMANGLED: GameWorld::OnBanAbuserFromPowerWar(void)
    bool OnBanAbuserFromPowerWar();
    // IDA_EA: 0x86CF138
    // IDA_NAME: _ZN9GameWorld26CleanupPowerWarVillToStartEv
    // DEMANGLED: GameWorld::CleanupPowerWarVillToStart(void)
    bool CleanupPowerWarVillToStart();
    // IDA_EA: 0x86CF1FE
    // IDA_NAME: _ZN9GameWorld16GetPowerUpDomainERSt6vectorIP5CUserSaIS2_EES5_
    // DEMANGLED: GameWorld::GetPowerUpDomain(std::vector<CUser *> &,std::vector<CUser *> &)
    bool GetPowerUpDomain(std::vector<CUser*>& power_a_users,
                          std::vector<CUser*>& power_b_users);
    // IDA_EA: 0x86CF356
    // IDA_NAME: _ZN9GameWorld13CancelPowerUpEv
    // DEMANGLED: GameWorld::CancelPowerUp(void)
    void CancelPowerUp();
    // IDA_EA: 0x86CF46A
    // IDA_NAME: _ZN9GameWorld15SendPowerUpListE27ENUM_POWER_UP_ACTIVATE_TYPERSt6vectorIP5CUserSaIS3_EE
    // DEMANGLED: GameWorld::SendPowerUpList(ENUM_POWER_UP_ACTIVATE_TYPE,std::vector<CUser *> &)
    void SendPowerUpList(ENUM_POWER_UP_ACTIVATE_TYPE activate_type,
                         std::vector<CUser*>& users);
    // IDA_EA: 0x86D1480
    // IDA_NAME: _ZN9GameWorld19UpdateMiniCraneSeedEv
    // DEMANGLED: GameWorld::UpdateMiniCraneSeed(void)
    char UpdateMiniCraneSeed();

    // Complete GameWorld storage recovered from C2/D2 and the world routing,
    // statistics, channel and login families.  The twelve occupation counters
    // at +0x164 are serialized in order by UpdateChannelUser; their individual
    // job labels are kept in the SIG payload rather than duplicated here.
    Mutex world_mutex_;                                  // +0x000
    int32_t village_count_;                              // +0x018
    Village* villages_;                                 // +0x01c
    int32_t channel_type_;                               // +0x020
    uint16_t next_unique_id_;                            // +0x024
    uint16_t padding_026_027_;
    int32_t crash_observation_count_;                    // +0x028
    Mutex crash_observation_mutex_;                      // +0x02c
    GameWorldIntCountMap crash_packet_counts_;           // +0x044
    GameWorldIntCountMap crash_dungeon_counts_;          // +0x05c
    GameWorldIntCountMap crash_account_counts_;          // +0x074
    GameWorldSimpleInfoMap simple_info_by_name_;          // +0x08c
    StatisticsShusiaService shusia_statistics_;          // +0x0a4
    StatisticsPvP pvp_statistics_;                       // +0x0ec
    GameWorldNatTypeStatisticsMap nat_type_statistics_;  // +0x11c
    GameWorldUserMap users_in_world_;                    // +0x134
    GameWorldSessionMap sessions_by_account_;            // +0x14c
    int32_t occupation_counts_[
        oldsrv::config::kCharacterJobCount];             // +0x164
    int32_t other_user_count_;                           // +0x194
    int32_t hangame_user_count_;                         // +0x198
    GameWorldLoginUserMap login_users_;                  // +0x19c
};

typedef char GameWorld_Size_Must_Be_0x1B4[
    (sizeof(GameWorld) == 0x1b4) ? 1 : -1];

#if defined(__i386__)
static_assert(offsetof(GameWorld, village_count_) == 0x018u &&
                  offsetof(GameWorld, villages_) == 0x01cu &&
                  offsetof(GameWorld, channel_type_) == 0x020u &&
                  offsetof(GameWorld, next_unique_id_) == 0x024u &&
                  offsetof(GameWorld, crash_observation_count_) == 0x028u &&
                  offsetof(GameWorld, crash_observation_mutex_) == 0x02cu &&
                  offsetof(GameWorld, crash_packet_counts_) == 0x044u &&
                  offsetof(GameWorld, crash_dungeon_counts_) == 0x05cu &&
                  offsetof(GameWorld, crash_account_counts_) == 0x074u &&
                  offsetof(GameWorld, simple_info_by_name_) == 0x08cu &&
                  offsetof(GameWorld, shusia_statistics_) == 0x0a4u &&
                  offsetof(GameWorld, pvp_statistics_) == 0x0ecu &&
                  offsetof(GameWorld, nat_type_statistics_) == 0x11cu &&
                  offsetof(GameWorld, users_in_world_) == 0x134u &&
                  offsetof(GameWorld, sessions_by_account_) == 0x14cu &&
                  offsetof(GameWorld, occupation_counts_) == 0x164u &&
                  offsetof(GameWorld, other_user_count_) == 0x194u &&
                  offsetof(GameWorld, hangame_user_count_) == 0x198u &&
                  offsetof(GameWorld, login_users_) == 0x19cu &&
                  sizeof(GameWorld) == 0x1b4u,
              "GameWorld must retain the recovered i386 ABI layout");
#endif

// This is an internal iteration helper, not a newly exported GameWorld ABI.
// Its directory offset is established by the old daily-reset and broadcast
// loops (for example 0x827B5F0 and 0x8116108), which traverse the canonical
// `std::map<unsigned short,CUser*>` at GameWorld+0x134 in key order.
template <typename UserOperation>
void GameWorld::ProcessAllUsers(UserOperation operation) {
    for (typename GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end();
         ++it) {
        operation(it->second);
    }
}

// IDA_EA: 0x80DA3A7
// IDA_NAME: _Z11G_GameWorldv
// DEMANGLED: G_GameWorld(void)
GameWorld* G_GameWorld();

// IDA_EA: 0x86CBC85
// IDA_NAME: _Z19deleteSpecificItemsP5CUserPKSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: deleteSpecificItems(CUser *,std::vector<std::pair<int,int>> const*)
int deleteSpecificItems(
    CUser* user,
    const std::vector<std::pair<int, int> >* matched_items);

#endif  // OLDSRV_OLD_GAME_WORLD_GAME_WORLD_H_
