#ifndef OLDSRV_OLD_GAME_USERINFO_CUSER_H_
#define OLDSRV_OLD_GAME_USERINFO_CUSER_H_

// Canonical C-owner CUser runtime surface.  The legacy public declarations
// remain included while their owner-neutral compatibility header is retired
// incrementally; new C gameplay code must include this header.
#include "common/manager/user.h"
#include "common/manager/common_time_game_option_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "common/manager/reliable_person.h"
#include "common/manager/play_info_public_abi.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "old_data/db/core/action_point_db_exchange_proc_abi.h"
#include "old_data/Login/security/security_card.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/growth_creature_event.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/player/core/character_cache_data.h"
#include "old_game/player/core/character_load_mercenary.h"
#include "old_game/player/core/character_misc_data.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/player/core/charac_visible_flag.h"
#include "old_game/player/core/cerashop_add_restrict_user_info.h"
#include "old_game/player/core/charac_expand_data_mgr.h"
#include "old_game/player/core/cuser_lifecycle_support_types.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/player/setting/character_option.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/progression/skill/combo_skill.h"
#include "old_game/progression/skill/guild_skill_layout.h"
#include "infra/network/dispatcher/client_spec_statistic_dispatcher.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/security/policy/pi_sender_manager.h"

#include <stdint.h>
#include <stddef.h>
#include <cstdio>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


class UserQuest;
class Quest;
class CUser;
class CQuestShop;
class CParty;
class PvP_Room;
class WarRoom;
class BlueMarble;
namespace pvp_assault {
class CAssaultPlace;
}
class CTradeSpace;
class ItemVendingMachine;
class PacketBuf;
class Dispatcher_PeerConnectResult;
class DisPatcher_ReqPeer;
class DisPatcher_SetIP;
class InterUpdateP2pHolePunchingState;
class Inter_LoadCerashopAddRestrictType;
class Inter_LoadCharac;
class Inter_LoadEventLevelupSupport;
class Inter_LoadCharacLinkBonus;
class Inter_LoadDimensionInout;
class Inter_RequestCharacSkillInfo;
class Inter_RequestTagCharacInfo;
class PacketGuard;
class Stream;
class PISenderManager;
class CItem;
class SkillSlot;
class CReliablePerson;
class CGameMasterCharacter;
class cSecuUserAction;
namespace expert_job {
class CExpertJob;
}  // namespace expert_job
namespace APSystem {
class Inter_LoadActionPoint;
class CUserProc;
}
namespace charac_link_db {
struct Message;
}
struct CGameOption;
struct _Charac_info;
struct _Additioal_info;
struct CHARAC_LOGIN_DATA;
struct TAG_CHARAC_DATA;
struct TAG_CHARAC_INVEN_DATA;
struct STSkillCommandData;
struct CONTINUOUS_ITEM_INFO;
struct SIG_LIMIT_ITEM_INFO;
struct STGuildProxy;
struct stAuraAvatarOption;
struct seriaRoom_AniDeco;
struct STBreakAwayRewards;
struct competition_reward_result;
struct SIG_ATTENDANCE_EVENT;
struct SIG_GROWTH_WEAPON_EVENT;
struct SIG_GIVE_GROWTH_CREATURE_EVENT;
struct SIG_LEVELUP_GIFT_ACCOUNT_ONCE;
struct SIG_INGAME_EVENT_HISTORY;
struct SIG_SAVE_PC_ROOM_RENT_ITEM;
struct SIG_SAVE_DIMENSION_RENT_ITEM;
struct SIG_LOAD_PREMIUM_LETHE;
struct MSG_MAILBOX_SEND;
struct MSG_MULTI_MAILBOX_SEND;
struct MSG_REQ_TO_PEER;
struct CodeHackChecksum;
enum PAY_TYPE : int;
enum ENUM_MESSAGE_TYPE : int;
enum ENUM_DUNGEON_MODE : int;
enum ENUM_SECURITY_REWARD_TYPE : int;
enum ENUM_BILLING_TYPE : int;

// A narrow internal helper used by the Secu_GoldControl provider while
// recording its own Hades endpoint.  It is a friendship boundary, not a
// general CUser network accessor.
void Secu_GoldControl_capture_hades_endpoint(Secu_GoldControl* gold_control,
                                             char* ip,
                                             char* mac);

// IDA unit: 99_misc_global/STGuildDBInfoOnly.  CUser embeds this exact
// 0xbd-byte guild record at expanded +0x8d370.  It is a shared CUser
// layout, not the old byte-offset convention duplicated in packet .cpps.
#pragma pack(push, 1)
struct STGuildDBInfoOnly {
    // IDA_EA: 0x8693F56
    // IDA_NAME: _ZN17STGuildDBInfoOnlyC2Ev
    // DEMANGLED: STGuildDBInfoOnly::STGuildDBInfoOnly(void)
    STGuildDBInfoOnly();

    char guild_name[0x17];       // IDA +0x00
    // Inter_MonitorNoticeGuildDismiss @0x84CA00C and IsGuildMaster
    // @0x8230172 compare this value with the current character number.
    uint32_t guild_master_charac_no; // IDA +0x17
    uint8_t guild_level;         // IDA +0x1b
    uint8_t guild_flag;          // IDA +0x1c
    uint8_t raw_1d;
    uint32_t raw_1e;
    // GuildParameterScript indexes its member-count weight/price rows with
    // this value (CParty clear reward and CUser::CheckGuildLevelUp).
    uint16_t guild_member_count; // IDA +0x22
    uint32_t raw_24;
    uint8_t raw_28;
    uint32_t guild_experience;   // IDA +0x29
    uint8_t raw_2d;
    uint32_t sub_guild_master_character_no[5]; // IDA +0x2e
    uint16_t raw_42;
    uint8_t skill_count;         // IDA +0x44
    STGuildSkill skills[16];     // IDA +0x45
    uint8_t power_side;          // IDA +0x95, CUser::getPowerSide
    uint32_t raw_96;
    uint32_t guild_power_war_point; // IDA +0x9a
    uint8_t guild_agit_flag;     // IDA +0x9e, CUser::SetGuildAgitFlag
    uint8_t raw_9f;
    uint32_t guild_fund;       // IDA +0xa0, CMD269/CUser guild level-up
    uint8_t raw_a4[0x15];
    uint32_t raw_b9;
};
#pragma pack(pop)

static_assert(offsetof(STGuildDBInfoOnly, guild_master_charac_no) == 0x17u,
              "STGuildDBInfoOnly guild-master character offset must stay 0x17");
static_assert(offsetof(STGuildDBInfoOnly, guild_level) == 0x1bu,
              "STGuildDBInfoOnly guild level offset must stay 0x1b");
static_assert(offsetof(STGuildDBInfoOnly, guild_member_count) == 0x22u,
              "STGuildDBInfoOnly guild member count offset must stay 0x22");
static_assert(offsetof(STGuildDBInfoOnly, guild_experience) == 0x29u,
              "STGuildDBInfoOnly guild experience offset must stay 0x29");
static_assert(offsetof(STGuildDBInfoOnly, skill_count) == 0x44u,
              "STGuildDBInfoOnly skill count offset must stay 0x44");
static_assert(offsetof(STGuildDBInfoOnly, skills) == 0x45u,
              "STGuildDBInfoOnly skills offset must stay 0x45");
static_assert(offsetof(STGuildDBInfoOnly, sub_guild_master_character_no) ==
                  0x2eu,
              "STGuildDBInfoOnly sub-master list offset must stay 0x2e");
static_assert(offsetof(STGuildDBInfoOnly, power_side) == 0x95u,
              "STGuildDBInfoOnly power-side offset must stay 0x95");
static_assert(offsetof(STGuildDBInfoOnly, guild_agit_flag) == 0x9eu,
              "STGuildDBInfoOnly guild-agit flag offset must stay 0x9e");
static_assert(sizeof(STGuildDBInfoOnly) == 0xbdu,
              "STGuildDBInfoOnly IDA ABI size must stay 0xbd");
// IDA preserves CACHE_SECTION as a named int-backed enum in the public CUser
// ABI.  Keep the opaque declaration so restored members mangle to the original
// symbol without inventing values that belong to the DB/cache protocol owner.
enum CACHE_SECTION : int;
enum ENUM_CHARAC_LINK_BONUS_STATE : int;
enum ENUM_CHARAC_EXPAND_TYPE : int;
enum ENUM_PREMIUM_TYPE : int;
namespace secretshop {
struct SECRET_SHOP_DATA;
struct SECRET_SHOP_INFO;
}  // namespace secretshop
struct BingoData;
struct CharacterOption;
struct SIG_CREATE_CHARAC;
struct CHARAC_DATA;
struct CCharacterView;
struct SIG_SAVE_INVENTORY;
struct SIG_LOAD_INVENTORY;
struct SIG_AUCTION_LOG_MESSAGE_AG;
struct SIG_AUCTION_LOG_MESSAGE_PG;
enum ENUM_DBLOG_ITEM_TYPE : int;
enum ENUM_PVP_HISTORY_REASON : int;
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  The source binary
// exposes the type names in the CeraShop balance and purchase-history ABI,
// but not their complete enumerator tables.
enum ENUM_CERAPOINT_LOG_TYPE : int;
namespace CeraShopPaymentType {
enum T : int;
}  // namespace CeraShopPaymentType
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  The old guild
// cargo-history logger exposes this distinct enum name in its member ABI;
// only request reason 3 is observed by CMD266, so keep its value domain
// opaque rather than inventing the guild-service protocol's full table.
enum ENUM_GUILD_CARGO_HISTORY_REASON : int;
enum ENUM_GM_ITEM_UPGRADE : int;
enum ENUM_ITEM_MAKING_TYPE : int;
namespace AntibotPacketLogType {
enum T : int;
}  // namespace AntibotPacketLogType
enum ENUM_ABNORMAL_USER_LOG_TYPE : int;
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  The two observed
// integer domains are intentionally opaque: no unobserved enum value is
// reconstructed here.
enum ENUM_SECU_SERVICE_TYPE : int;
enum ENUM_SECU_SERVICE_ACTION : int;
enum ENUM_ITEMSPACE : int;
enum ENUM_SERVER_GROUP : int;
enum ENUM_BREAK_AWAY_REWARD_ORDER : int;
enum ENUM_BADGE_TYPE : int;
enum ENUM_CMDPACKET : uint16_t;
enum ENUM_PEER_REQUEST_TYPE : int;
enum ENUM_MONEY_FULL_REASON : int;
// IDA_EA: 0x8676826.  This type is opaque here because CeraShop only sends
// the observed action value zero; the statistic protocol owns its enumerators.
enum ENUM_RANDOMBOX_ACTION_KIND : int;
enum ENUM_QUEST_ENEMY_TYPE : int;
enum ch_state : int;
enum eWPAddReason : int;
enum eWPSubReason : int;
enum eFPAddReason : int;
enum eFPSubReason : int;
enum eCoinAddReason : int;
enum eExpAddReason : int;
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  These three
// history-only reason domains are visible in the original member symbols but
// their enumerator tables belong to the cash/inventory protocol owners.
enum eCashInvenAddReason : int;
enum eCashInvenDelReason : int;
enum eSealItemReason : int;
enum eLevelUpReason : int;
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  SetCharacLevel
// writes this distinct history reason domain while applying a GM downgrade.
// The old binary exposes the type name but no enumerator table, so retain the
// original int-backed ABI without inventing values.
enum eLevelDownReason : int;
enum eChangeGrowTypeReason : int;
enum eSPAddReason : int;
enum eSPSubReason : int;
enum eSkillAddReason : int;
enum eSkillDelReason : int;
enum eSkillInitReason : int;
// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  These two reason
// domains are only observed as raw values 0 and 1 in the Lethe load path.
enum eSkillSaveReason : int;
enum eSkillSPInfoSaveReason : int;
enum eAvatarItemAddReason : int;
enum eAvatarItemDelReason : int;
enum eTradeEndReason : int;
enum eApplyItemReason : int;
enum DISCONN_SIG : int;
namespace StackableAction {
enum Type : int;
}  // namespace StackableAction
namespace expert_job {
class CExpertJob;
}  // namespace expert_job
namespace online_preliminary {
struct PlayCountInfoEx {
    unsigned int play_count;
    bool dirty;

    // IDA_EA: 0x8588D2A
    // IDA_NAME: _ZN18online_preliminary15PlayCountInfoExC2Ev
    // DEMANGLED: online_preliminary::PlayCountInfoEx::PlayCountInfoEx(void)
    PlayCountInfoEx();

    // IDA_EA: 0x8588D40
    // IDA_NAME: _ZN18online_preliminary15PlayCountInfoExC2Ejb
    // DEMANGLED: online_preliminary::PlayCountInfoEx::PlayCountInfoEx(unsigned int,bool)
    PlayCountInfoEx(unsigned int play_count_value, bool dirty_value);
};

static_assert(sizeof(void*) != 4 || sizeof(PlayCountInfoEx) == 0x08u,
              "PlayCountInfoEx i386 ABI size must stay 0x08");

class COnlinePreliminaryTeam {
public:
    // IDA_EA: 0x8587EB8
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeamC2Ev
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::COnlinePreliminaryTeam(void)
    COnlinePreliminaryTeam();

    // IDA_EA: 0x8587ECE
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeamD2Ev
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::~COnlinePreliminaryTeam()
    ~COnlinePreliminaryTeam();

    // IDA_EA: 0x8568EEE
    // IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam9GetRatingEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetRating(void)const
    float GetRating() const;

    // IDA_EA: 0x8587EE4
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam9_saveTeamEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::_saveTeam(void)
    int _saveTeam();

    // IDA_EA: 0x858800C
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam18_saveTeamMatchListEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::_saveTeamMatchList(void)
    int _saveTeamMatchList();

    // IDA_EA: 0x85882B8
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam23loadDataPreliminaryTeamEPc
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::loadDataPreliminaryTeam(char *)
    int loadDataPreliminaryTeam(char* data);

    // IDA_EA: 0x858831A
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam32loadDataPreliminaryTeamMatchListEPc
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::loadDataPreliminaryTeamMatchList(char *)
    int loadDataPreliminaryTeamMatchList(char* data);

    // IDA_EA: 0x8588374
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam18ResetDailyMidnightEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::ResetDailyMidnight(void)
    void ResetDailyMidnight();

    // IDA_EA: 0x858838A
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam5ResetEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::Reset(void)
    COnlinePreliminaryTeam* Reset();

    // IDA_EA: 0x85883FE
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam16IncreaseWinCountEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseWinCount(void)
    COnlinePreliminaryTeam* IncreaseWinCount();

    // IDA_EA: 0x858841A
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam17IncreaseLoseCountEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseLoseCount(void)
    COnlinePreliminaryTeam* IncreaseLoseCount();

    // IDA_EA: 0x8588436
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam9SetRatingEf
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::SetRating(float)
    COnlinePreliminaryTeam* SetRating(float rating);

    // IDA_EA: 0x858844C
    // IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam25GetEachOtherTeamPlayCountEi
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetEachOtherTeamPlayCount(int)const
    int GetEachOtherTeamPlayCount(int other_team_id) const;

    // IDA_EA: 0x85884B6
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam30IncreaseEachOtherTeamPlayCountEi
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseEachOtherTeamPlayCount(int)
    COnlinePreliminaryTeam* IncreaseEachOtherTeamPlayCount(int other_team_id);

    int saveData();
    int GetCurrentUserCount();

    // IDA_EA: 0x8588D88
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam19SetCurrentUserCountEi
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::SetCurrentUserCount(int)
    COnlinePreliminaryTeam* SetCurrentUserCount(int count);

    // IDA_EA: 0x8588D96
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam24IncreaseCurrentUserCountEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseCurrentUserCount(void)
    COnlinePreliminaryTeam* IncreaseCurrentUserCount();

    COnlinePreliminaryTeam* DecreaseCurrentUserCount();

    // IDA_EA: 0x8697892
    // IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam9GetTeamIdEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetTeamId(void)const
    int GetTeamId() const;

    // IDA_EA: 0x869789E
    // IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam11GetTeamNameEv
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetTeamName(void)
    char* GetTeamName();

private:
    // The original StaticPool advances CNode instances by 0x48: the team
    // object is 0x40 bytes and the node trailer is 0x08 bytes.  Each member
    // below is read or written by the original team methods.
    int current_user_count_;                         // +0x00
    int team_id_;                                    // +0x04
    float rating_;                                   // +0x08
    unsigned int win_count_;                         // +0x0c
    unsigned int lose_count_;                        // +0x10
    char team_name_[16];                             // +0x14
    std::map<int, PlayCountInfoEx> play_counts_;     // +0x24
    bool team_dirty_;                                // +0x3c
    bool play_counts_dirty_;                         // +0x3d
};

static_assert(sizeof(void*) != 4 || sizeof(COnlinePreliminaryTeam) == 0x40u,
              "COnlinePreliminaryTeam i386 ABI size must stay 0x40");

// The original CTeamFactory is exactly a named 600-entry StaticPool, not a
// heap-per-team substitute.  Keep the inheritance so the old pool symbols and
// CNode stride remain the storage contract used by logout/save paths.
class CTeamFactory : public StaticPool<COnlinePreliminaryTeam, 600> {
public:
    // IDA_EA: 0x8588E2E
    // IDA_NAME: _ZN18online_preliminary12CTeamFactoryC2Ev
    // DEMANGLED: online_preliminary::CTeamFactory::CTeamFactory(void)
    CTeamFactory();

    // IDA_EA: 0x8588E42
    // IDA_NAME: _ZN18online_preliminary12CTeamFactoryD2Ev
    // DEMANGLED: online_preliminary::CTeamFactory::~CTeamFactory()
    ~CTeamFactory();

    // IDA_EA: 0x8588DD8
    // IDA_NAME: _ZN18online_preliminary12CTeamFactory7AcquireEv
    // DEMANGLED: online_preliminary::CTeamFactory::Acquire(void)
    COnlinePreliminaryTeam* Acquire();

    // IDA_EA: 0x8588DEC
    // IDA_NAME: _ZN18online_preliminary12CTeamFactory4FreeEPNS_22COnlinePreliminaryTeamE
    // DEMANGLED: online_preliminary::CTeamFactory::Free(
    //     online_preliminary::COnlinePreliminaryTeam *)
    int Free(COnlinePreliminaryTeam* team);
};

static_assert(sizeof(void*) != 4 || sizeof(CTeamFactory) == 0x30u,
              "CTeamFactory i386 ABI size must stay 0x30");

class COnlinePreliminaryTeamMgr {
public:
    // IDA_EA: 0x858860C / 0x85886FE
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgrC2Ev / D2Ev
    COnlinePreliminaryTeamMgr();
    ~COnlinePreliminaryTeamMgr();

    // IDA_EA: 0x858875E
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr8FindTeamEi
    COnlinePreliminaryTeam* FindTeam(int team_id);

    // IDA_EA: 0x85887C8
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr11OnEnterUserEP5CUserPc
    int OnEnterUser(CUser* user, char* data);

    // IDA_EA: 0x8588AE8
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr11OnLeaveUserEP5CUser
    // DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::OnLeaveUser(CUser *)
    int OnLeaveUser(CUser* user);

    // IDA_EA: 0x8588BE4
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr14OnLoadTeamInfoEP5CUserPc
    int OnLoadTeamInfo(CUser* user, char* data);

    // IDA_EA: 0x8588C5C
    // IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr18ResetDailyMidnightEv
    int ResetDailyMidnight();

    CTeamFactory* factory;
    std::map<int, COnlinePreliminaryTeam*> teams;
};

static_assert(sizeof(void*) != 4 || sizeof(COnlinePreliminaryTeamMgr) == 0x1cu,
              "COnlinePreliminaryTeamMgr i386 ABI size must stay 0x1c");
}  // namespace online_preliminary
namespace charac_expand {
class CData;
}  // namespace charac_expand
namespace advancealtar {
class DBAndExchangeServerProc;
class Dispatcher_BuyItem;
class Dispatcher_ExchangeSlot;
class Dispatcher_GetAchievementReward;
class Dispatcher_ResetStar;
class Dispatcher_SetSlot;
class Dispatcher_StartGame;
class Inter_Load;
class Manager;
struct CharacData;
class ProcStage;
class StageControl;
}
namespace UserSpace {
enum T : int;
}  // namespace UserSpace
namespace Sanicova {
struct CPadLayoutVerifier;

class CPad {
public:
    // Raw callers pass 0 and 1 through this original nested enum.  Keep the
    // enum in the public ABI rather than weakening setRequestState to int.
    enum ENUM_REQUEST_STATE : int {
        ENUM_REQUEST_STATE_NONE = 0,
        ENUM_REQUEST_STATE_PENDING = 1,
    };

    // IDA_EA: 0x822EFDA
    // IDA_NAME: _ZNK8Sanicova4CPad10isActivateEv
    // DEMANGLED: Sanicova::CPad::isActivate(void)const
    bool isActivate() const;
    // IDA_EA: 0x822EFE6
    // IDA_NAME: _ZNK8Sanicova4CPad11isCertifiedEv
    // DEMANGLED: Sanicova::CPad::isCertified(void)const
    bool isCertified() const;
    // IDA_EA: 0x822EF8E
    // IDA_NAME: _ZN8Sanicova4CPad10getFailCntEv
    // DEMANGLED: Sanicova::CPad::getFailCnt(void)
    int getFailCnt();
    // IDA_EA: 0x822EFC0
    // IDA_NAME: _ZNK8Sanicova4CPad15getRequestStateEv
    // DEMANGLED: Sanicova::CPad::getRequestState(void)const
    int getRequestState() const;
    // IDA_EA: 0x822EFCC
    // IDA_NAME: _ZN8Sanicova4CPad15setRequestStateENS0_18ENUM_REQUEST_STATEE
    // DEMANGLED: Sanicova::CPad::setRequestState(Sanicova::CPad::ENUM_REQUEST_STATE)
    CPad* setRequestState(ENUM_REQUEST_STATE state);
    // IDA_EA: 0x822EF9A
    // IDA_NAME: _ZN8Sanicova4CPad12incCancelCntEv
    // DEMANGLED: Sanicova::CPad::incCancelCnt(void)
    int incCancelCnt();
    // IDA_EA: 0x822EFB4
    // IDA_NAME: _ZN8Sanicova4CPad12getCancelCntEv
    // DEMANGLED: Sanicova::CPad::getCancelCnt(void)
    int getCancelCnt();
    // IDA_EA: 0x84EBDFA
    // IDA_NAME: _ZN8Sanicova4CPad12setCancelCntEi
    // DEMANGLED: Sanicova::CPad::setCancelCnt(int)
    CPad* setCancelCnt(int cancel_count);
    // IDA_EA: 0x84EBE08
    // IDA_NAME: _ZN8Sanicova4CPad11setActivateEb
    // DEMANGLED: Sanicova::CPad::setActivate(bool)
    CPad* setActivate(bool activated);
    // IDA_EA: 0x826530E
    // IDA_NAME: _ZN8Sanicova4CPad12setCertifiedEb
    // DEMANGLED: Sanicova::CPad::setCertified(bool)
    CPad* setCertified(bool certified);
    // IDA_EA: 0x82652D8
    // IDA_NAME: _ZN8Sanicova4CPad10incFailCntEv
    // DEMANGLED: Sanicova::CPad::incFailCnt(void)
    int incFailCnt();
    // IDA_EA: 0x82652F2
    // IDA_NAME: _ZN8Sanicova4CPad12resetFailCntEv
    // DEMANGLED: Sanicova::CPad::resetFailCnt(void)
    CPad* resetFailCnt();
    // IDA_EA: 0x8265302
    // IDA_NAME: _ZN8Sanicova4CPad18getAdvideChangePwdEv
    // DEMANGLED: Sanicova::CPad::getAdvideChangePwd(void)
    int getAdvideChangePwd();
    // IDA_EA: 0x8265326
    // IDA_NAME: _ZN8Sanicova4CPad11getPasswordEv
    // DEMANGLED: Sanicova::CPad::getPassword(void)
    char* getPassword();
    // IDA_EA: 0x828712C
    // IDA_NAME: _ZN8Sanicova4CPad10setFailCntEi
    // DEMANGLED: Sanicova::CPad::setFailCnt(int)
    CPad* setFailCnt(int fail_count);
    // IDA_EA: 0x828713A
    // IDA_NAME: _ZN8Sanicova4CPad11setPasswordEPc
    // DEMANGLED: Sanicova::CPad::setPassword(char *)
    char* setPassword(char* password);
    // IDA_EA: 0x84EBDB2
    // IDA_NAME: _ZN8Sanicova4CPad18setAdvideChangePwdEb
    // DEMANGLED: Sanicova::CPad::setAdvideChangePwd(bool)
    CPad* setAdvideChangePwd(bool advise_change_password);
    // IDA_EA: 0x84EBDCA
    // IDA_NAME: _ZN8Sanicova4CPad13getChangedPwdEv
    // DEMANGLED: Sanicova::CPad::getChangedPwd(void)
    int getChangedPwd();
    // IDA_EA: 0x84EBDD6
    // IDA_NAME: _ZN8Sanicova4CPad14setVersionInfoEc
    // DEMANGLED: Sanicova::CPad::setVersionInfo(char)
    CPad* setVersionInfo(char version_info);
    // IDA_EA: 0x84EBDEE
    // IDA_NAME: _ZN8Sanicova4CPad14getVersionInfoEv
    // DEMANGLED: Sanicova::CPad::getVersionInfo(void)
    int getVersionInfo();
    // IDA_EA: 0x859984C
    // IDA_NAME: _ZN8Sanicova4CPad20GetSelectedCipherIdxEv
    // DEMANGLED: Sanicova::CPad::GetSelectedCipherIdx(void)
    int GetSelectedCipherIdx();

    // IDA_EA: 0x8599872
    // IDA_NAME: _ZN8Sanicova4CPad9GetKeyIdxEi
    // DEMANGLED: Sanicova::CPad::GetKeyIdx(int)
    int GetKeyIdx(int key_group);
    // IDA_EA: 0x85998A6
    // IDA_NAME: _ZN8Sanicova4CPad17isValidKeyGenTimeEv
    // DEMANGLED: Sanicova::CPad::isValidKeyGenTime(void)
    int isValidKeyGenTime();
    // IDA_EA: 0x85998EA
    // IDA_NAME: _ZN8Sanicova4CPad18GenerateRandomDataEv
    // DEMANGLED: Sanicova::CPad::GenerateRandomData(void)
    int GenerateRandomData();
    // IDA_EA: 0x8599918
    // IDA_NAME: _ZN8Sanicova4CPad20GenerateRandomKeyIdxEv
    // DEMANGLED: Sanicova::CPad::GenerateRandomKeyIdx(void)
    CPad* GenerateRandomKeyIdx();
    // IDA_EA: 0x8599936
    // IDA_NAME: _ZN8Sanicova4CPad12GetShakeDataEi
    // DEMANGLED: Sanicova::CPad::GetShakeData(int)
    int GetShakeData(int index);
    // IDA_EA: 0x8599946
    // IDA_NAME: _ZN8Sanicova4CPad23GenerateRandomShakeDataEv
    // DEMANGLED: Sanicova::CPad::GenerateRandomShakeData(void)
    bool GenerateRandomShakeData();

    // IDA_EA: 0x8599712
    // IDA_NAME: _ZNK8Sanicova4CPad11checkAnswerEPc
    // DEMANGLED: Sanicova::CPad::checkAnswer(char *)const
    bool checkAnswer(char* answer) const;

    // IDA_EA: 0x859957C
    // IDA_NAME: _ZN8Sanicova4CPad4initEv
    // DEMANGLED: Sanicova::CPad::init(void)
    int init();

    // IDA_EA: 0x84EBE20
    // IDA_NAME: _ZN8Sanicova4CPad12setCheckTimeEl
    // DEMANGLED: Sanicova::CPad::setCheckTime(long)
    CPad* setCheckTime(long check_time);

    // IDA_EA: 0x85997B8 / 0x859981E
    // IDA_NAME: _ZN8Sanicova4CPad18SendChagePwdRewardEP5CUser /
    //           _ZN8Sanicova4CPad22SendChagePwdRewardMailEP5CUser
    int SendChagePwdReward(CUser* user);
    CPad* SendChagePwdRewardMail(CUser* user);
    // IDA_EA: 0x85999E8
    // IDA_NAME: _ZN8Sanicova4CPad13setChangedPwdEb
    // DEMANGLED: Sanicova::CPad::setChangedPwd(bool)
    CPad* setChangedPwd(bool changed_password);

private:
    friend struct CPadLayoutVerifier;

    // Every member through +0x47 is observed in the live old-IDB CPad
    // methods.  The nearby +0x48..+0x57 bytes belong to CUser, not CPad.
    char encrypted_password_[0x24];             // +0x00
    bool activated_;                             // +0x24
    bool certified_;                             // +0x25
    uint8_t version_info_;                       // +0x26
    long check_time_;                            // +0x28
    uint8_t init_cleared_byte_2c_;               // +0x2c
    uint8_t init_cleared_byte_2d_;               // +0x2d
    uint16_t selected_cipher_idx_;               // +0x2e
    uint32_t key_generation_time_;               // +0x30
    uint8_t shake_data_[4];                      // +0x34
    int fail_count_;                             // +0x38
    bool advise_change_password_;                // +0x3c
    bool changed_password_;                      // +0x3d
    ENUM_REQUEST_STATE request_state_;           // +0x40
    int cancel_count_;                           // +0x44
};

#if defined(__i386__)
struct CPadLayoutVerifier {
    static_assert(sizeof(CPad::ENUM_REQUEST_STATE) == 0x04u,
                  "CPad request-state enum must remain int-backed");
    static_assert(offsetof(CPad, activated_) == 0x24u,
                  "CPad activation offset must stay 0x24");
    static_assert(offsetof(CPad, certified_) == 0x25u,
                  "CPad certification offset must stay 0x25");
    static_assert(offsetof(CPad, check_time_) == 0x28u,
                  "CPad check-time offset must stay 0x28");
    static_assert(offsetof(CPad, init_cleared_byte_2c_) == 0x2cu,
                  "CPad init-cleared byte offset must stay 0x2c");
    static_assert(offsetof(CPad, init_cleared_byte_2d_) == 0x2du,
                  "CPad init-cleared byte offset must stay 0x2d");
    static_assert(offsetof(CPad, selected_cipher_idx_) == 0x2eu,
                  "CPad selected cipher offset must stay 0x2e");
    static_assert(offsetof(CPad, key_generation_time_) == 0x30u,
                  "CPad key-generation time offset must stay 0x30");
    static_assert(offsetof(CPad, shake_data_) == 0x34u,
                  "CPad shake-data offset must stay 0x34");
    static_assert(offsetof(CPad, fail_count_) == 0x38u,
                  "CPad fail-count offset must stay 0x38");
    static_assert(offsetof(CPad, request_state_) == 0x40u,
                  "CPad request-state offset must stay 0x40");
    static_assert(offsetof(CPad, cancel_count_) == 0x44u,
                  "CPad cancel-count offset must stay 0x44");
    static_assert(sizeof(CPad) == 0x48u,
                  "CPad itself ends before CUser's adjacent anti-hack fields");
};
#endif
}  // namespace Sanicova

namespace WongWork {
class CBossStage;
class CBossTower;
class CDeathTower;
class CDungeonClear;
class CHackAnalyzer;
class CMailBox;
class CSecurityCard;
class CUserPremium;
class CMCAPManager {
public:
    // IDA_EA: 0x8695068
    // IDA_NAME: _ZN8WongWork12CMCAPManagerC2Ev
    // DEMANGLED: WongWork::CMCAPManager::CMCAPManager(void)
    CMCAPManager();

    // IDA_EA: 0x8695080
    // IDA_NAME: _ZN8WongWork12CMCAPManagerD2Ev
    // DEMANGLED: WongWork::CMCAPManager::~CMCAPManager()
    ~CMCAPManager();

    // IDA_EA: 0x8695025
    // IDA_NAME: _ZN8WongWork12CMCAPManager19getPointPerHackTypeENS_13ENUM_HACKTYPEE
    // DEMANGLED: WongWork::CMCAPManager::getPointPerHackType(WongWork::ENUM_HACKTYPE)
    static int getPointPerHackType(ENUM_HACKTYPE hack_type);

    // IDA_EA: 0x80F8BE0
    // IDA_NAME: _ZN8WongWork12CMCAPManager19setPointPerHackTypeEPi
    // The DB return signal 295 contains the complete 101..4000 ratio table.
    static void setPointPerHackType(int* point_per_hack_type);

    // IDA_EA: 0x84EB53E
    // IDA_NAME: _ZN8WongWork12CMCAPManager8incPointEi
    // DEMANGLED: WongWork::CMCAPManager::incPoint(int)
    CMCAPManager* incPoint(int point);

    // IDA_EA: 0x82870DE
    // IDA_NAME: _ZN8WongWork12CMCAPManager8setPointEt
    // DEMANGLED: WongWork::CMCAPManager::setPoint(unsigned short)
    CMCAPManager* setPoint(uint16_t point);

    unsigned int getPoint();
    // IDA_EA: 0x822EEDE / 0x822EEEA.  The save marker belongs to the
    // manager itself; disconnect paths must not synthesize a separate flag.
    int IsSaveCleanPadPoint();
    CMCAPManager* disableCleanPadPoint();
    int getExposedCount() const;

    // IDA_EA: 0x8695038
    // IDA_NAME: _ZN8WongWork12CMCAPManager15incExposedCountEv
    // DEMANGLED: WongWork::CMCAPManager::incExposedCount(void)
    CMCAPManager* incExposedCount();

    // IDA_EA: 0x8695058
    // IDA_NAME: _ZN8WongWork12CMCAPManager17resetExposedCountEv
    // DEMANGLED: WongWork::CMCAPManager::resetExposedCount(void)
    CMCAPManager* resetExposedCount();

    // IDA_EA: 0x8694FA0
    // IDA_NAME: _ZN8WongWork12CMCAPManager5resetEv
    // DEMANGLED: WongWork::CMCAPManager::reset(void)
    CMCAPManager* reset();

    // IDA_EA: 0x8694FE0 / 0x8694FF0
    // IDA_NAME: _ZN8WongWork12CMCAPManager12resetIncRateEv /
    //           _ZN8WongWork12CMCAPManager10decIncRateEi
    // DEMANGLED: WongWork::CMCAPManager::resetIncRate(void) /
    //            WongWork::CMCAPManager::decIncRate(int)
    CMCAPManager* resetIncRate();
    CMCAPManager* decIncRate(int decrement);

private:
    // IDA_NAME: _ZN8WongWork12CMCAPManager17pointPerHackType_E
    // Old CMCAPManager::init clears exactly 3900 signed entries before the
    // clean-pad table is loaded by setPointPerHackType(int*).
    static int pointPerHackType_[3900];

    uint32_t point_;             // +0x00
    int32_t increment_rate_;     // +0x04; incPoint performs signed / 1000
    uint32_t snapshot_point_;    // +0x08
    uint32_t raw_0c_;            // +0x0c
    uint32_t exposed_count_;     // +0x10
    uint8_t save_clean_pad_;     // +0x14
    uint8_t pad_15_17_[3];
};

#if defined(__i386__)
static_assert(sizeof(CMCAPManager) == 0x18u,
              "WongWork::CMCAPManager must preserve its old i386 extent");
#endif
}  // namespace WongWork

class UserMercenaryInfoMgr {
public:
    // IDA_EA: 0x8697B3A
    // IDA_NAME: _ZN20UserMercenaryInfoMgrC2Ev
    // DEMANGLED: UserMercenaryInfoMgr::UserMercenaryInfoMgr(void)
    UserMercenaryInfoMgr();

    // IDA_EA: 0x8697B4E
    // IDA_NAME: _ZN20UserMercenaryInfoMgrD2Ev
    // DEMANGLED: UserMercenaryInfoMgr::~UserMercenaryInfoMgr()
    ~UserMercenaryInfoMgr();

    // IDA_EA: 0x81B441A
    // IDA_NAME: _ZN20UserMercenaryInfoMgr12RemoveCharacEj
    // DEMANGLED: UserMercenaryInfoMgr::RemoveCharac(unsigned int)
    int RemoveCharac(unsigned int charac_no);

    // IDA_EA: 0x81B4262
    // IDA_NAME: _ZNK20UserMercenaryInfoMgr16CheckCharacIndexEh
    // DEMANGLED: UserMercenaryInfoMgr::CheckCharacIndex(unsigned char) const
    bool CheckCharacIndex(unsigned char index) const;

    // IDA_EA: 0x81B42AE
    // IDA_NAME: _ZNK20UserMercenaryInfoMgr19FindIndexByCharacNoEj
    // DEMANGLED: UserMercenaryInfoMgr::FindIndexByCharacNo(unsigned int) const
    int FindIndexByCharacNo(unsigned int charac_no) const;

    // The complete waiting-state predicate must call the separate original
    // CHARAC_LOAD_MERCENARY::GetCompetitionState identity.  That member is
    // not yet exposed by the B-owned DB layout, so preserve the existing
    // declaration-only boundary until the whole predicate can be restored.
    bool IsWaitingStateCharac(unsigned int charac_no);

    // IDA_EA: 0x81B462E
    // IDA_NAME: _ZN20UserMercenaryInfoMgr16UpdateCharacNameEjRKSs
    // DEMANGLED: UserMercenaryInfoMgr::UpdateCharacName(unsigned int,std::string const&)
    char* UpdateCharacName(unsigned int charac_no,
                           const std::string& charac_name);
    int ReturnCharac(CUser* user,
                     unsigned int charac_no,
                     competition_reward_result& result);
    bool CompetitionCharac(CUser* user,
                           unsigned int charac_no,
                           char competition_type,
                           char competition_count);
    // IDA_EA: 0x81B4198
    // IDA_NAME: _ZN20UserMercenaryInfoMgr16CheckValidAppendERK21CHARAC_LOAD_MERCENARY
    // DEMANGLED: UserMercenaryInfoMgr::CheckValidAppend(CHARAC_LOAD_MERCENARY const&)
    int CheckValidAppend(const CHARAC_LOAD_MERCENARY& data);

    // IDA_EA: 0x81B4300
    // IDA_NAME: _ZN20UserMercenaryInfoMgr9AddCharacER21CHARAC_LOAD_MERCENARY
    // DEMANGLED: UserMercenaryInfoMgr::AddCharac(CHARAC_LOAD_MERCENARY&)
    // This is intentionally a writable reference: the original body derives
    // and stores avatar_bonus_index in the supplied DB record before copying
    // it into the manager's vector.
    int AddCharac(CHARAC_LOAD_MERCENARY& data);

    // Non-IDA owner-local reset boundary used by CUser::reset.  The old
    // CUser body clears the manager's leading vector directly; keep that
    // representation inside its concrete owner.
    void ClearForUserReset();

private:
    // CUser owns this exact embedded manager at +0x06ef6c.  The original
    // CUser packet writer iterates its vector directly at 0x868E704; keep
    // that owner-local ABI access rather than introducing an extra facade.
    friend class CUser;
    std::vector<CHARAC_LOAD_MERCENARY> mercenaries_;
};

typedef std::vector<InstanceRentalSystem::RentalInfo> CUserRentalInfoList;
typedef std::map<unsigned int, CUserRentalInfoList> CUserRentalInfoByCharac;
typedef std::map<unsigned int, CUserRentalInfoByCharac> CUserRentalInfoStore;
typedef std::set<std::pair<unsigned char, unsigned short> > CUserCSHashSet;

// The old Send path updates the low packet-id byte and packet class
// independently, while setLastPacketID/getLastPacketID address the same four
// bytes as one integer.  Keep one real storage word and update its wire bytes
// with masks; a union overlay is not part of the recovered source contract.
struct CUserLastPacketIdentity {
    uint32_t value;
};

static_assert(sizeof(CUserLastPacketIdentity) == 0x4u,
              "CUser last-packet identity must stay four bytes");

// IDA unit: 99_misc_global/LimitItemUsage.  The record is packed: CommonTime
// occupies +0x00..+0x0f, followed by type and three unaligned dwords.
#pragma pack(push, 1)
struct LimitItemUsage {
    // IDA_EA: 0x8695EF0
    // IDA_NAME: _ZN14LimitItemUsageC2Ev
    // DEMANGLED: LimitItemUsage::LimitItemUsage(void)
    LimitItemUsage();
    // IDA_EA: 0x86A6480
    // IDA_NAME: _ZN14LimitItemUsageD2Ev
    // DEMANGLED: LimitItemUsage::~LimitItemUsage()
    ~LimitItemUsage();
    // IDA_EA: 0x8688FBA
    // IDA_NAME: _ZN14LimitItemUsage12isAnotherDayEv
    // DEMANGLED: LimitItemUsage::isAnotherDay(void)
    int isAnotherDay();
    // IDA_EA: 0x8695F28
    // IDA_NAME: _ZN14LimitItemUsage5useItEv
    // DEMANGLED: LimitItemUsage::useIt(void)
    int useIt();
    // IDA_EA: 0x8695F68
    // IDA_NAME: _ZN14LimitItemUsage8isUsableEv
    // DEMANGLED: LimitItemUsage::isUsable(void)
    int isUsable();

    CommonTime common_time;
    uint8_t limit_type;
    int32_t limit_count;
    int32_t item_id;
    int32_t used_count;
};
#pragma pack(pop)

static_assert(sizeof(LimitItemUsage) == 0x1du,
              "LimitItemUsage IDA ABI size must stay 0x1d");
static_assert(offsetof(LimitItemUsage, common_time) == 0x00u &&
                  offsetof(LimitItemUsage, limit_type) == 0x10u &&
                  offsetof(LimitItemUsage, limit_count) == 0x11u &&
                  offsetof(LimitItemUsage, item_id) == 0x15u &&
                  offsetof(LimitItemUsage, used_count) == 0x19u,
              "LimitItemUsage named fields must retain the old packed ABI");

// IDA unit: 99_misc_global/ContinuousItemInfo.  The two dwords are the item
// index and its absolute expiry tick in every selected-character vector.
struct ContinuousItemInfo {
    // IDA_EA: 0x8695C48
    // IDA_NAME: _ZN18ContinuousItemInfoC2Ev
    // DEMANGLED: ContinuousItemInfo::ContinuousItemInfo(void)
    ContinuousItemInfo();

    // IDA_EA: 0x86A04B2
    // IDA_NAME: _ZN18ContinuousItemInfoD2Ev
    // DEMANGLED: ContinuousItemInfo::~ContinuousItemInfo()
    ~ContinuousItemInfo();

    int item_index;
    int expire_tick;
};

// IDA unit: 99_misc_global/WpBonusPointResult.
struct WpBonusPointResult {
    // IDA_EA: 0x8695C78
    // IDA_NAME: _ZN18WpBonusPointResultC2Ev
    // DEMANGLED: WpBonusPointResult::WpBonusPointResult(void)
    WpBonusPointResult();

    // IDA_EA: 0x8695C8C
    // IDA_NAME: _ZN18WpBonusPointResult5clearEv
    // DEMANGLED: WpBonusPointResult::clear(void)
    WpBonusPointResult* clear();

    int normal;
    int premium;
};

// IDA unit: 99_misc_global/ExpBonusPointResult.
struct ExpBonusPointResult {
    // IDA_EA: 0x8695CA4
    // IDA_NAME: _ZN19ExpBonusPointResultC2Ev
    // DEMANGLED: ExpBonusPointResult::ExpBonusPointResult(void)
    ExpBonusPointResult();

    // IDA_EA: 0x8695CB8
    // IDA_NAME: _ZN19ExpBonusPointResult5clearEv
    // DEMANGLED: ExpBonusPointResult::clear(void)
    ExpBonusPointResult* clear();

    int normal;
    int premium;
};

// `_Charac_info` stores the two result pairs at unaligned ABI offsets
// +0x129d/+0x12a5.  The Result classes above retain their original C2
// symbols for temporaries; this POD record is the physical stored form so
// the packed character aggregate cannot acquire C++ alignment padding.
struct __attribute__((packed)) WpBonusPointStorage {
    int32_t normal;
    int32_t premium;
};

struct __attribute__((packed)) ExpBonusPointStorage {
    int32_t normal;
    int32_t premium;
};

static_assert(sizeof(ContinuousItemInfo) == 0x08u,
              "ContinuousItemInfo IDA ABI size must stay 0x08");
static_assert(sizeof(WpBonusPointResult) == 0x08u,
              "WpBonusPointResult IDA ABI size must stay 0x08");
static_assert(sizeof(ExpBonusPointResult) == 0x08u,
              "ExpBonusPointResult IDA ABI size must stay 0x08");
static_assert(sizeof(WpBonusPointStorage) == 0x08u,
              "persistent WP bonus result storage must stay 0x08");
static_assert(sizeof(ExpBonusPointStorage) == 0x08u,
              "persistent EXP bonus result storage must stay 0x08");

// IDA identifies CUserCharacInfo as the same-address base subobject of CUser:
// CUser::CUser at 0x864E410 constructs CUserCharacInfo at `this` and the CUser
// destructor tears it down at that same address.  Keep it as the actual base
// interface, not as free CUserCharacInfo_* ABI adapters.
class CUserCharacInfo {
public:
    // IDA_EA: 0x864E15E
    // IDA_NAME: _ZN15CUserCharacInfoC2Ev
    // DEMANGLED: CUserCharacInfo::CUserCharacInfo(void)
    CUserCharacInfo();

    // IDA_EA: 0x8697AC0
    // IDA_NAME: _ZN15CUserCharacInfoD2Ev
    // DEMANGLED: CUserCharacInfo::~CUserCharacInfo()
    ~CUserCharacInfo();

    // IDA_EA: 0x868EFAE
    // IDA_NAME: _ZN15CUserCharacInfo18SetCurCharacOptionER15CharacterOption
    // DEMANGLED: CUserCharacInfo::SetCurCharacOption(CharacterOption &)
    void* SetCurCharacOption(CharacterOption& option);

    // IDA_EA: 0x812CDBE
    // IDA_NAME: _ZN15CUserCharacInfo13getCurCharacWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacW(void)
    _Charac_info* getCurCharacW();

    // IDA_EA: 0x84EBEA8
    // IDA_NAME: _ZNK15CUserCharacInfo13getTagCharacREv
    // DEMANGLED: CUserCharacInfo::getTagCharacR(void)const
    const _Charac_info* getTagCharacR() const;

    // IDA_EA: 0x822F110
    // IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacInvenREv
    // DEMANGLED: CUserCharacInfo::getTagCharacInvenR(void)const
    CInventory* getTagCharacInvenR() const;

    // IDA_EA: 0x822F120
    // IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacSkillREv
    // DEMANGLED: CUserCharacInfo::getTagCharacSkillR(void)const
    SkillSlot* getTagCharacSkillR() const;

    // IDA_EA: 0x84EBEB4
    // IDA_NAME: _ZN15CUserCharacInfo18getTagCharacSkillWEv
    // DEMANGLED: CUserCharacInfo::getTagCharacSkillW(void)
    void* getTagCharacSkillW();

    // IDA_EA: 0x812CDD6
    // IDA_NAME: _ZNK15CUserCharacInfo22getCurCharacMaxFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacMaxFatigue(void)const
    int getCurCharacMaxFatigue() const;

    // IDA_EA: 0x812CDF6
    // IDA_NAME: _ZN15CUserCharacInfo29setCurCharacMaxPremiumFatigueEt
    // DEMANGLED: CUserCharacInfo::setCurCharacMaxPremiumFatigue(unsigned short)
    int setCurCharacMaxPremiumFatigue(uint16_t fatigue);

    // IDA_EA: 0x814AA52
    // IDA_NAME: _ZN15CUserCharacInfo15enableSaveQuestEv
    // DEMANGLED: CUserCharacInfo::enableSaveQuest(void)
    CUserCharacInfo* enableSaveQuest();

    // IDA_EA: 0x8151AB2
    // IDA_NAME: _ZN15CUserCharacInfo15enableSaveCargoEv
    // DEMANGLED: CUserCharacInfo::enableSaveCargo(void)
    CUserCharacInfo* enableSaveCargo();

    // IDA_EA: 0x8151A94
    // IDA_NAME: _ZN15CUserCharacInfo18getCurCharacCargoWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacCargoW(void)
    CCargo* getCurCharacCargoW();

    // IDA_EA: 0x81803CA
    // IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacInvenRefREv
    // DEMANGLED: CUserCharacInfo::getCurCharacInvenRefR(void)const
    CInventory* getCurCharacInvenRefR() const;

    // IDA_EA: 0x8680E04
    // IDA_NAME: _ZNK15CUserCharacInfo12FindItemLockEhR14ENUM_ITEMSPACERiP5CUser
    // DEMANGLED: CUserCharacInfo::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &,CUser *)const
    bool FindItemLock(unsigned char lock,
                      ENUM_ITEMSPACE& item_space,
                      int& slot,
                      CUser* user) const;

    // IDA_EA: 0x819340A
    // IDA_NAME: _ZN15CUserCharacInfo21getCurCharacInvenRefWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacInvenRefW(void)
    void* getCurCharacInvenRefW();

    // IDA_EA: 0x822F23C
    // IDA_NAME: _ZNK15CUserCharacInfo24getCurCharSecondGrowTypeEv
    // DEMANGLED: CUserCharacInfo::getCurCharSecondGrowType(void)const
    char getCurCharSecondGrowType() const;

    // IDA_EA: 0x822F894
    // IDA_NAME: _ZNK15CUserCharacInfo25GetCurCharacExpertJobTypeEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacExpertJobType(void)const
    int GetCurCharacExpertJobType() const;

    // IDA_EA: 0x822F8B6
    // IDA_NAME: _ZN15CUserCharacInfo21SetCurCharacExpertJobEPN10expert_job10CExpertJobE
    // DEMANGLED: CUserCharacInfo::SetCurCharacExpertJob(expert_job::CExpertJob *)
    int SetCurCharacExpertJob(expert_job::CExpertJob* expert_job);

    // IDA_EA: 0x8568DEA
    // IDA_NAME: _ZN15CUserCharacInfo33GetCurCharacOnlinePreliminaryTeamEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeam(void)
    // The target slot is a real, unaligned COnlinePreliminaryTeam* at
    // _Charac_info+0x0e89.  The old decompiler displayed EAX as `int`, but
    // every consumer (including COnlinePreliminary and TeamInfoSecond) uses
    // it as this exact pointer type; the return type is not encoded in the
    // Itanium member-function symbol.
    online_preliminary::COnlinePreliminaryTeam*
    GetCurCharacOnlinePreliminaryTeam();

    // IDA_EA: 0x8645DE2
    // IDA_NAME: _ZNK15CUserCharacInfo35GetCurCharacOnlinePreliminaryTeamIdEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamId(void)const
    int GetCurCharacOnlinePreliminaryTeamId() const;

    // IDA_EA: 0x8588DBA
    // IDA_NAME: _ZN15CUserCharacInfo33SetCurCharacOnlinePreliminaryTeamEPN18online_preliminary22COnlinePreliminaryTeamE
    // DEMANGLED: CUserCharacInfo::SetCurCharacOnlinePreliminaryTeam(online_preliminary::COnlinePreliminaryTeam *)
    int SetCurCharacOnlinePreliminaryTeam(
        online_preliminary::COnlinePreliminaryTeam* team);

    // IDA_EA: 0x849F53E
    // IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacExpertJobTypeEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacExpertJobType(int)
    int SetCurCharacExpertJobType(int type);

    // IDA_EA: 0x849F56A
    // IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacExpertJobExpEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacExpertJobExp(int)
    int SetCurCharacExpertJobExp(int exp);

    // IDA_EA: 0x8375026
    // IDA_NAME: _ZNK15CUserCharacInfo24GetCurCharacExpertJobExpEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacExpertJobExp(void)const
    int GetCurCharacExpertJobExp() const;

    // IDA_EA: 0x8120432
    // IDA_NAME: _ZNK15CUserCharacInfo13getCurCharacREv
    // DEMANGLED: CUserCharacInfo::getCurCharacR(void)const
    const _Charac_info* getCurCharacR() const;

    // IDA_EA: 0x80CBC4E
    // IDA_NAME: _ZNK15CUserCharacInfo14getCurCharacNoEv
    // DEMANGLED: CUserCharacInfo::getCurCharacNo(void)const
    int getCurCharacNo() const;

    // IDA_EA: 0x8101028
    // IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacNameEv
    // DEMANGLED: CUserCharacInfo::getCurCharacName(void)const
    const char* getCurCharacName() const;

    // IDA_EA: 0x80DA2B8
    // IDA_NAME: _ZNK15CUserCharacInfo16get_charac_levelEv
    // DEMANGLED: CUserCharacInfo::get_charac_level(void)const
    int get_charac_level() const;

    // IDA_EA: 0x80FDF20
    // IDA_NAME: _ZNK15CUserCharacInfo14get_charac_jobEv
    // DEMANGLED: CUserCharacInfo::get_charac_job(void)const
    int get_charac_job() const;

    // IDA_EA: 0x817A188
    // IDA_NAME: _ZNK15CUserCharacInfo17getCurCharacMoneyEv
    // DEMANGLED: CUserCharacInfo::getCurCharacMoney(void)const
    int getCurCharacMoney() const;

    // IDA_EA: 0x822F27A
    // IDA_NAME: _ZN15CUserCharacInfo15enableSaveSkillEv
    // DEMANGLED: CUserCharacInfo::enableSaveSkill(void)
    CUserCharacInfo* enableSaveSkill();

    // IDA_EA: 0x822F262
    // IDA_NAME: _ZN15CUserCharacInfo20enableSaveCharacInfoEv
    // DEMANGLED: CUserCharacInfo::enableSaveCharacInfo(void)
    CUserCharacInfo* enableSaveCharacInfo();

    // IDA_EA: 0x822F26E
    // IDA_NAME: _ZN15CUserCharacInfo26enableSaveCharacManageInfoEv
    // DEMANGLED: CUserCharacInfo::enableSaveCharacManageInfo(void)
    CUserCharacInfo* enableSaveCharacManageInfo();

    // IDA_EA: 0x822F286
    // IDA_NAME: _ZN15CUserCharacInfo13enableSavePvPEv
    // DEMANGLED: CUserCharacInfo::enableSavePvP(void)
    CUserCharacInfo* enableSavePvP();

    // IDA_EA: 0x819A870
    // IDA_NAME: _ZN15CUserCharacInfo20enableSaveCharacStatEv
    // DEMANGLED: CUserCharacInfo::enableSaveCharacStat(void)
    CUserCharacInfo* enableSaveCharacStat();

    // IDA_EA: 0x822F140
    // IDA_NAME: _ZN15CUserCharacInfo18getCurCharacSkillWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacSkillW(void)
    SkillSlot* getCurCharacSkillW();

    // IDA_EA: 0x822F202
    // IDA_NAME: _ZNK15CUserCharacInfo22getCurCharacCreateTimeEv
    // DEMANGLED: CUserCharacInfo::getCurCharacCreateTime(void)const
    int getCurCharacCreateTime() const;

    // IDA_EA: 0x822F292
    // IDA_NAME: _ZN15CUserCharacInfo19setSaveCharacOptionEb
    // DEMANGLED: CUserCharacInfo::setSaveCharacOption(bool)
    CUserCharacInfo* setSaveCharacOption(bool save);

    // IDA_EA: 0x80DA27E
    // IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacInvenREv
    // DEMANGLED: CUserCharacInfo::getCurCharacInvenR(void)const
    CInventory* getCurCharacInvenR() const;

    // IDA_EA: 0x80DA2AC
    // IDA_NAME: _ZN15CUserCharacInfo15enableSaveInvenEv
    // DEMANGLED: CUserCharacInfo::enableSaveInven(void)
    CUserCharacInfo* enableSaveInven();

    // IDA_EA: 0x80DA28E
    // IDA_NAME: _ZN15CUserCharacInfo18getCurCharacInvenWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacInvenW(void)
    CInventory* getCurCharacInvenW();

    // IDA_EA: 0x8151A84
    // IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacCargoREv
    // DEMANGLED: CUserCharacInfo::getCurCharacCargoR(void)const
    CCargo* getCurCharacCargoR() const;

    // IDA_EA: 0x822F8F6
    // IDA_NAME: _ZN15CUserCharacInfo16isDisguiseCharacEv
    // DEMANGLED: CUserCharacInfo::isDisguiseCharac(void)
    int isDisguiseCharac();

    // IDA_EA: 0x822F976
    // IDA_NAME: _ZN15CUserCharacInfo15setDisguiseKindEh
    // DEMANGLED: CUserCharacInfo::setDisguiseKind(unsigned char)
    int setDisguiseKind(uint8_t kind);

    // IDA_EA: 0x822F99E
    // IDA_NAME: _ZN15CUserCharacInfo16setDisguiseIndexEt
    // DEMANGLED: CUserCharacInfo::setDisguiseIndex(unsigned short)
    int setDisguiseIndex(uint16_t index);

    // IDA_EA: 0x822F91A
    // IDA_NAME: _ZN15CUserCharacInfo17setDisguiseCharacEbht
    // DEMANGLED: CUserCharacInfo::setDisguiseCharac(bool,unsigned char,unsigned short)
    int setDisguiseCharac(bool flag, uint8_t kind, uint16_t index);

    // IDA_EA: 0x8645CB4
    // IDA_NAME: _ZN15CUserCharacInfo15reset_guild_expEv
    // DEMANGLED: CUserCharacInfo::reset_guild_exp(void)
    int reset_guild_exp();

    // IDA_EA: 0x864E25C
    // IDA_NAME: _ZN15CUserCharacInfo12setCurCharacEP12_Charac_info
    // DEMANGLED: CUserCharacInfo::setCurCharac(_Charac_info *)
    CUserCharacInfo* setCurCharac(_Charac_info* charac);

    // IDA_EA: 0x864E28E
    // IDA_NAME: _ZN15CUserCharacInfo12setTagCharacEP12_Charac_info
    // DEMANGLED: CUserCharacInfo::setTagCharac(_Charac_info *)
    CUserCharacInfo* setTagCharac(_Charac_info* charac);

    // IDA_EA: 0x8110C94
    // IDA_NAME: _ZNK15CUserCharacInfo23getCurCharFirstGrowTypeEv
    // DEMANGLED: CUserCharacInfo::getCurCharFirstGrowType(void)const
    int getCurCharFirstGrowType() const;

    // IDA_EA: 0x8110CBA
    // IDA_NAME: _ZN15CUserCharacInfo23getCurCharacUsedFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacUsedFatigue(void)
    int getCurCharacUsedFatigue();

    // IDA_EA: 0x8110CDA
    // IDA_NAME: _ZN15CUserCharacInfo29IncCurCharacEventAddItemCountEt
    // DEMANGLED: CUserCharacInfo::IncCurCharacEventAddItemCount(unsigned short)
    int IncCurCharacEventAddItemCount(uint16_t count);

    // IDA_EA: 0x8110D12
    // IDA_NAME: _ZNK15CUserCharacInfo29getCurUseBringUpMaterialTotalEv
    // DEMANGLED: CUserCharacInfo::getCurUseBringUpMaterialTotal(void)const
    int getCurUseBringUpMaterialTotal() const;

    // IDA_EA: 0x8110D76
    // IDA_NAME: _ZNK15CUserCharacInfo20getBringUpRewardFlagEv
    // DEMANGLED: CUserCharacInfo::getBringUpRewardFlag(void)const
    int getBringUpRewardFlag() const;

    // IDA_EA: 0x8110D9A
    // IDA_NAME: _ZN15CUserCharacInfo20setBringUpRewardFlagEb
    // DEMANGLED: CUserCharacInfo::setBringUpRewardFlag(bool)
    int setBringUpRewardFlag(bool flag);

    // IDA_EA: 0x811EDA6
    // IDA_NAME: _ZN15CUserCharacInfo26getUpgradeSeparateTrueFlagEv
    // DEMANGLED: CUserCharacInfo::getUpgradeSeparateTrueFlag(void)
    int getUpgradeSeparateTrueFlag();

    // IDA_EA: 0x811EDCA
    // IDA_NAME: _ZN15CUserCharacInfo27getUpgradeSeparateFlaseFlagEv
    // DEMANGLED: CUserCharacInfo::getUpgradeSeparateFlaseFlag(void)
    int getUpgradeSeparateFlaseFlag();

    // IDA_EA: 0x8696116
    // IDA_NAME: _ZN15CUserCharacInfo25IsSaveDemensionInoutValueEv
    // DEMANGLED: CUserCharacInfo::IsSaveDemensionInoutValue(void)
    int IsSaveDemensionInoutValue();

    // IDA_EA: 0x8696128
    // IDA_NAME: _ZN15CUserCharacInfo26SetSaveDemensionInoutValueEb
    // DEMANGLED: CUserCharacInfo::SetSaveDemensionInoutValue(bool)
    int SetSaveDemensionInoutValue(bool value);

    // IDA_EA: 0x8696146
    // IDA_NAME: _ZN15CUserCharacInfo21IsSaveBloodInoutValueEv
    // DEMANGLED: CUserCharacInfo::IsSaveBloodInoutValue(void)
    int IsSaveBloodInoutValue();

    // IDA_EA: 0x8696158
    // IDA_NAME: _ZN15CUserCharacInfo22SetSaveBloodInoutValueEb
    // DEMANGLED: CUserCharacInfo::SetSaveBloodInoutValue(bool)
    int SetSaveBloodInoutValue(bool value);

    // IDA_EA: 0x85BF9D8
    // IDA_NAME: _ZN15CUserCharacInfo22getDemensionInoutValueEc
    // DEMANGLED: CUserCharacInfo::getDemensionInoutValue(char)
    int getDemensionInoutValue(char index);

    // IDA_EA: 0x8696176
    // IDA_NAME: _ZN15CUserCharacInfo21getUltimateInoutValueEc
    // DEMANGLED: CUserCharacInfo::getUltimateInoutValue(char)
    int getUltimateInoutValue(char index);

    // IDA_EA: 0x8645564
    // IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacVillEv
    // DEMANGLED: CUserCharacInfo::getCurCharacVill(void)const
    int getCurCharacVill() const;

    // IDA_EA: 0x86455BC
    // IDA_NAME: _ZN15CUserCharacInfo16setCurCharacVillEc
    // DEMANGLED: CUserCharacInfo::setCurCharacVill(char)
    int setCurCharacVill(char village);

    // IDA_EA: 0x8645620
    // IDA_NAME: _ZN15CUserCharacInfo14UseFinishPointEi
    // DEMANGLED: CUserCharacInfo::UseFinishPoint(int)
    int UseFinishPoint(int point);

    // IDA_EA: 0x8645764
    // IDA_NAME: _ZN15CUserCharacInfo14GetFinishPointEv
    // DEMANGLED: CUserCharacInfo::GetFinishPoint(void)
    int GetFinishPoint();

    // IDA_EA: 0x86457C0
    // IDA_NAME: _ZN15CUserCharacInfo17ChangeFinishPointEi
    // DEMANGLED: CUserCharacInfo::ChangeFinishPoint(int)
    int ChangeFinishPoint(int point);

    // IDA_EA: 0x86459C4
    // IDA_NAME: _ZN15CUserCharacInfo11WarAreaKillEiif
    // DEMANGLED: CUserCharacInfo::WarAreaKill(int,int,float)
    int WarAreaKill(int monster_kind, int point, float multiplier);

    // IDA_EA: 0x86458CE
    // IDA_NAME: _ZN15CUserCharacInfo19GetFinishPointTotalEv
    // DEMANGLED: CUserCharacInfo::GetFinishPointTotal(void)
    int GetFinishPointTotal();

    // IDA_EA: 0x864592A
    // IDA_NAME: _ZN15CUserCharacInfo15InitFinishPointEv
    // DEMANGLED: CUserCharacInfo::InitFinishPoint(void)
    int InitFinishPoint();

    // IDA_EA: 0x8645AA0
    // IDA_NAME: _ZN15CUserCharacInfo16DecreaseWarPointEi
    // DEMANGLED: CUserCharacInfo::DecreaseWarPoint(int)
    int DecreaseWarPoint(int point);

    // IDA_EA: 0x80DA2DA
    // IDA_NAME: _ZN15CUserCharacInfo16IsCurCharacGhostEv
    // DEMANGLED: CUserCharacInfo::IsCurCharacGhost(void)
    int IsCurCharacGhost();

    // IDA_EA: 0x8646406
    // IDA_NAME: _ZN15CUserCharacInfo13LiveCurCharacEv
    // DEMANGLED: CUserCharacInfo::LiveCurCharac(void)
    int LiveCurCharac();

    // IDA_EA: 0x822F46C
    // IDA_NAME: _ZN15CUserCharacInfo19get_charac_guildkeyEv
    // DEMANGLED: CUserCharacInfo::get_charac_guildkey(void)
    int get_charac_guildkey();

    // IDA_EA: 0x822F6D2
    // IDA_NAME: _ZN15CUserCharacInfo15GetAssaultPlaceEv
    // DEMANGLED: CUserCharacInfo::GetAssaultPlace(void)
    int GetAssaultPlace();

    // IDA_EA: 0x822F8D4
    // IDA_NAME: _ZNK15CUserCharacInfo21GetCurCharacExpertJobEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacExpertJob(void)const
    expert_job::CExpertJob* GetCurCharacExpertJob() const;

    // IDA_EA: 0x864781E
    // IDA_NAME: _ZN15CUserCharacInfo19IsBusyDisjointStoreEv
    // DEMANGLED: CUserCharacInfo::IsBusyDisjointStore(void)
    int IsBusyDisjointStore();

    // IDA_EA: 0x86961E0
    // IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacVillPrevEv
    // DEMANGLED: CUserCharacInfo::getCurCharacVillPrev(void)const
    int getCurCharacVillPrev() const;

    // IDA_EA: 0x868BDE4
    // IDA_NAME: _ZN15CUserCharacInfo25get_charac_visible_valuesEv
    // DEMANGLED: CUserCharacInfo::get_charac_visible_values(void)
    int get_charac_visible_values();

    // IDA_EA: 0x868BEE4
    // IDA_NAME: _ZN15CUserCharacInfo18IsCurCharacVisibleEv
    // DEMANGLED: CUserCharacInfo::IsCurCharacVisible(void)
    int IsCurCharacVisible();

    // IDA_EA: 0x812CDCA
    // IDA_NAME: _ZN15CUserCharacInfo19enableSaveCharacEtcEv
    // DEMANGLED: CUserCharacInfo::enableSaveCharacEtc(void)
    CUserCharacInfo* enableSaveCharacEtc();

    // IDA_EA: 0x822F48E
    // IDA_NAME: _ZN15CUserCharacInfo20resetGuildCreateFlagEv
    // DEMANGLED: CUserCharacInfo::resetGuildCreateFlag(void)
    int resetGuildCreateFlag();

    // IDA_EA: 0x822F4AC
    // IDA_NAME: _ZN15CUserCharacInfo22resetTempGuildNameAddrEv
    // DEMANGLED: CUserCharacInfo::resetTempGuildNameAddr(void)
    void* resetTempGuildNameAddr();

    // IDA_EA: 0x822F504
    // IDA_NAME: _ZN15CUserCharacInfo18getCreateGuildNameEv
    // DEMANGLED: CUserCharacInfo::getCreateGuildName(void)
    char* getCreateGuildName();

    // IDA_EA: 0x822F526
    // IDA_NAME: _ZN15CUserCharacInfo18getCreateGuildAddrEv
    // DEMANGLED: CUserCharacInfo::getCreateGuildAddr(void)
    char* getCreateGuildAddr();

    // IDA_EA: 0x822F548
    // IDA_NAME: _ZN15CUserCharacInfo19getGuildMemberGradeEv
    // DEMANGLED: CUserCharacInfo::getGuildMemberGrade(void)
    int getGuildMemberGrade();

    // IDA_EA: 0x85BFB64
    // IDA_NAME: _ZN15CUserCharacInfo24set_level_before_dungeonEs
    // DEMANGLED: CUserCharacInfo::set_level_before_dungeon(short)
    int set_level_before_dungeon(int16_t level);

    // IDA_EA: 0x8645D52
    // IDA_NAME: _ZNK15CUserCharacInfo31get_charac_level_before_dungeonEv
    // DEMANGLED: CUserCharacInfo::get_charac_level_before_dungeon(void)const
    int get_charac_level_before_dungeon() const;

    // IDA_EA: 0x8645D96
    // IDA_NAME: _ZN15CUserCharacInfo26reset_level_before_dungeonEv
    // DEMANGLED: CUserCharacInfo::reset_level_before_dungeon(void)
    int reset_level_before_dungeon();

    // IDA_EA: 0x822F56C
    // IDA_NAME: _ZN15CUserCharacInfo22get_guild_create_rightEv
    // DEMANGLED: CUserCharacInfo::get_guild_create_right(void)
    int get_guild_create_right();

    // IDA_EA: 0x822F590
    // IDA_NAME: _ZN15CUserCharacInfo22set_guild_create_rightEh
    // DEMANGLED: CUserCharacInfo::set_guild_create_right(unsigned char)
    int set_guild_create_right(uint8_t right);

    // IDA_EA: 0x84EC07C
    // IDA_NAME: _ZN15CUserCharacInfo18setGuildCreateFlagEt
    // DEMANGLED: CUserCharacInfo::setGuildCreateFlag(unsigned short)
    int setGuildCreateFlag(uint16_t flag);

    // IDA_EA: 0x866C0DA
    // IDA_NAME: _ZN15CUserCharacInfo20isSetGuildCreateFlagEt
    // DEMANGLED: CUserCharacInfo::isSetGuildCreateFlag(unsigned short)
    int isSetGuildCreateFlag(uint16_t flag);

    // IDA_EA: 0x866C11E
    // IDA_NAME: _ZN15CUserCharacInfo18resetCharGuildInfoEv
    // DEMANGLED: CUserCharacInfo::resetCharGuildInfo(void)
    int resetCharGuildInfo();

    // IDA_EA: 0x84EC0B4
    // IDA_NAME: _ZN15CUserCharacInfo19setGuildMemberGradeEh
    // DEMANGLED: CUserCharacInfo::setGuildMemberGrade(unsigned char)
    int setGuildMemberGrade(uint8_t grade);

    // IDA_EA: 0x85BFB8E
    // IDA_NAME: _ZN15CUserCharacInfo18get_guild_temp_expEv
    // DEMANGLED: CUserCharacInfo::get_guild_temp_exp(void)
    int get_guild_temp_exp();

    // IDA_EA: 0x8645C18
    // IDA_NAME: _ZN15CUserCharacInfo18add_guild_temp_expEi
    // DEMANGLED: CUserCharacInfo::add_guild_temp_exp(int)
    int add_guild_temp_exp(int exp);

    // IDA_EA: 0x8645C58
    // IDA_NAME: _ZN15CUserCharacInfo20reset_guild_temp_expEv
    // DEMANGLED: CUserCharacInfo::reset_guild_temp_exp(void)
    int reset_guild_temp_exp();

    // IDA_EA: 0x84EC0DC
    // IDA_NAME: _ZN15CUserCharacInfo20set_charac_memberkeyEj
    // DEMANGLED: CUserCharacInfo::set_charac_memberkey(unsigned int)
    int set_charac_memberkey(uint32_t key);

    // IDA_EA: 0x84EC0FA
    // IDA_NAME: _ZN15CUserCharacInfo24set_connect_upper_memberEj
    // DEMANGLED: CUserCharacInfo::set_connect_upper_member(unsigned int)
    int set_connect_upper_member(uint32_t member_key);

    // IDA_EA: 0x85BFBB4
    // IDA_NAME: _ZN15CUserCharacInfo24get_connect_upper_memberEv
    // DEMANGLED: CUserCharacInfo::get_connect_upper_member(void)
    int get_connect_upper_member();

    // IDA_EA: 0x822F692
    // IDA_NAME: _ZN15CUserCharacInfo12GetLoginTickEv
    // DEMANGLED: CUserCharacInfo::GetLoginTick(void)
    int GetLoginTick();

    // IDA_EA: 0x84EC118
    // IDA_NAME: _ZN15CUserCharacInfo12SetLoginTickEl
    // DEMANGLED: CUserCharacInfo::SetLoginTick(long)
    int SetLoginTick(long tick);

    // IDA_EA: 0x84EC12C
    // IDA_NAME: _ZN15CUserCharacInfo22SetCharacStartPlayTickEl
    // DEMANGLED: CUserCharacInfo::SetCharacStartPlayTick(long)
    CUserCharacInfo* SetCharacStartPlayTick(long tick);

    // IDA_EA: 0x822F6B4
    // IDA_NAME: _ZN15CUserCharacInfo23setCurrCharacBlackCountEj
    // DEMANGLED: CUserCharacInfo::setCurrCharacBlackCount(unsigned int)
    int setCurrCharacBlackCount(uint32_t count);

    // IDA_EA: 0x8151ABE
    // IDA_NAME: _ZNK15CUserCharacInfo15getCurCharacExpEv
    // DEMANGLED: CUserCharacInfo::getCurCharacExp(void)const
    int getCurCharacExp() const;

    // IDA_EA: 0x815741C
    // IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacGrowTypeEv
    // DEMANGLED: CUserCharacInfo::getCurCharacGrowType(void)const
    int getCurCharacGrowType() const;

    // IDA_EA: 0x8162338
    // IDA_NAME: _ZN15CUserCharacInfo16IsFirstTimeLoginEv
    // DEMANGLED: CUserCharacInfo::IsFirstTimeLogin(void)
    int IsFirstTimeLogin();

    // IDA_EA: 0x84EBEFA
    // IDA_NAME: _ZN15CUserCharacInfo17SetFirstTimeLoginEb
    // DEMANGLED: CUserCharacInfo::SetFirstTimeLogin(bool)
    int SetFirstTimeLogin(bool flag);

    // IDA_EA: 0x817A17C
    // IDA_NAME: _ZN15CUserCharacInfo11GetWinPointEv
    // DEMANGLED: CUserCharacInfo::GetWinPoint(void)
    int GetWinPoint();

    // IDA_EA: 0x84EC002
    // IDA_NAME: _ZN15CUserCharacInfo26setCurCharacTradeGoldDailyEj
    // DEMANGLED: CUserCharacInfo::setCurCharacTradeGoldDaily(unsigned int)
    CReliablePerson* setCurCharacTradeGoldDaily(uint32_t gold);

    // IDA_EA: 0x84EC02E
    // IDA_NAME: _ZN15CUserCharacInfo17SetReliablePersonEb
    // DEMANGLED: CUserCharacInfo::SetReliablePerson(bool)
    CUserCharacInfo* SetReliablePerson(bool flag);

    // IDA_EA: 0x84EC04A
    // IDA_NAME: _ZN15CUserCharacInfo21SetOverTradeGoldCountEj
    // DEMANGLED: CUserCharacInfo::SetOverTradeGoldCount(unsigned int)
    CUserCharacInfo* SetOverTradeGoldCount(uint32_t count);

    // IDA_EA: 0x819A87C
    // IDA_NAME: _ZN15CUserCharacInfo15setCurCharacExpEi
    // DEMANGLED: CUserCharacInfo::setCurCharacExp(int)
    int setCurCharacExp(int exp);

    // IDA_EA: 0x819EE4A
    // IDA_NAME: _ZNK15CUserCharacInfo13get_pvp_gradeEv
    // DEMANGLED: CUserCharacInfo::get_pvp_grade(void)const
    int get_pvp_grade() const;

    // IDA_EA: 0x822F0B0
    // IDA_NAME: _ZN15CUserCharacInfo29setCurCharacStrikerSkillIndexEh
    // DEMANGLED: CUserCharacInfo::setCurCharacStrikerSkillIndex(unsigned char)
    int setCurCharacStrikerSkillIndex(uint8_t index);

    // IDA_EA: 0x822F0E4
    // IDA_NAME: _ZN15CUserCharacInfo23setCurCharacTagCharacNoEj
    // DEMANGLED: CUserCharacInfo::setCurCharacTagCharacNo(unsigned int)
    int setCurCharacTagCharacNo(uint32_t charac_no);

    // IDA_EA: 0x822F2AE
    // IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacFatigue(void)const
    int getCurCharacFatigue() const;

    // IDA_EA: 0x822F2CE
    // IDA_NAME: _ZN15CUserCharacInfo19setCurCharacFatigueEi
    // DEMANGLED: CUserCharacInfo::setCurCharacFatigue(int)
    int setCurCharacFatigue(int fatigue);

    // IDA_EA: 0x822F2F8
    // IDA_NAME: _ZN15CUserCharacInfo25getUserEventCharacterFlagEv
    // DEMANGLED: CUserCharacInfo::getUserEventCharacterFlag(void)
    int getUserEventCharacterFlag();

    // IDA_EA: 0x822F308
    // IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacSkillTreeIndexEc
    // DEMANGLED: CUserCharacInfo::SetCurCharacSkillTreeIndex(char)
    int SetCurCharacSkillTreeIndex(char index);

    // IDA_EA: 0x822F33C
    // IDA_NAME: _ZNK15CUserCharacInfo26GetCurCharacSkillTreeIndexEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacSkillTreeIndex(void)const
    int GetCurCharacSkillTreeIndex() const;

    // IDA_EA: 0x822F362
    // IDA_NAME: _ZN15CUserCharacInfo34setCurCharacHelpAbuseComputedRatioEj
    // DEMANGLED: CUserCharacInfo::setCurCharacHelpAbuseComputedRatio(unsigned int)
    int setCurCharacHelpAbuseComputedRatio(uint32_t ratio);

    // IDA_EA: 0x822F380
    // IDA_NAME: _ZN15CUserCharacInfo17GetReliablePersonEv
    // DEMANGLED: CUserCharacInfo::GetReliablePerson(void)
    int GetReliablePerson();

    // IDA_EA: 0x822F390
    // IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacStaminaEv
    // DEMANGLED: CUserCharacInfo::getCurCharacStamina(void)const
    int getCurCharacStamina() const;

    // IDA_EA: 0x822F3A0
    // IDA_NAME: _ZNK15CUserCharacInfo27get_charac_10_level_sectionEv
    // DEMANGLED: CUserCharacInfo::get_charac_10_level_section(void)const
    int get_charac_10_level_section() const;

    // IDA_EA: 0x822F44E
    // IDA_NAME: _ZN15CUserCharacInfo18setCharacViewStateEi
    // DEMANGLED: CUserCharacInfo::setCharacViewState(int)
    int setCharacViewState(int view_state);

    // IDA_EA: 0x822F78A
    // IDA_NAME: _ZNK15CUserCharacInfo25GetCurRevengeDungeonCountEv
    // DEMANGLED: CUserCharacInfo::GetCurRevengeDungeonCount(void)const
    int GetCurRevengeDungeonCount() const;

    // IDA_EA: 0x84EC05C
    // IDA_NAME: _ZNK15CUserCharacInfo14get_charac_expEv
    // DEMANGLED: CUserCharacInfo::get_charac_exp(void)const
    int get_charac_exp() const;

    // IDA_EA: 0x8696AFE
    // IDA_NAME: _ZN15CUserCharacInfo21GetCurExEquipSlotStatEv
    // DEMANGLED: CUserCharacInfo::GetCurExEquipSlotStat(void)
    int GetCurExEquipSlotStat();

    // IDA_EA: 0x822F6F4
    // IDA_NAME: _ZN15CUserCharacInfo22GetCurCharacSuperStateEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacSuperState(void)
    int GetCurCharacSuperState();

    // IDA_EA: 0x822F718
    // IDA_NAME: _ZN15CUserCharacInfo22SetCurCharacChaosPointEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosPoint(int)
    int SetCurCharacChaosPoint(int point);

    // IDA_EA: 0x822F736
    // IDA_NAME: _ZN15CUserCharacInfo19SetCurCharacEmotionEs
    // DEMANGLED: CUserCharacInfo::SetCurCharacEmotion(short)
    int SetCurCharacEmotion(int16_t emotion);

    // IDA_EA: 0x822F762
    // IDA_NAME: _ZN15CUserCharacInfo25SetCurRevengeDungeonCountEh
    // DEMANGLED: CUserCharacInfo::SetCurRevengeDungeonCount(unsigned char)
    int SetCurRevengeDungeonCount(uint8_t count);

    // IDA_EA: 0x822F7AE
    // IDA_NAME: _ZN15CUserCharacInfo25GetCurCharacChaosKillTimeEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosKillTime(void)
    int GetCurCharacChaosKillTime();

    // IDA_EA: 0x822F7D0
    // IDA_NAME: _ZN15CUserCharacInfo11SetOpencoinEc
    // DEMANGLED: CUserCharacInfo::SetOpencoin(char)
    int SetOpencoin(char open_coin);

    // IDA_EA: 0x822F816
    // IDA_NAME: _ZN15CUserCharacInfo11GetOpencoinEv
    // DEMANGLED: CUserCharacInfo::GetOpencoin(void)
    int GetOpencoin();

    // IDA_EA: 0x82F0AD8
    // IDA_NAME: _ZN15CUserCharacInfo22GetCurCharacChaosPointEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosPoint(void)
    int GetCurCharacChaosPoint();

    // IDA_EA: 0x8645F80
    // IDA_NAME: _ZN15CUserCharacInfo22AddCurCharacChaosPointEi
    // DEMANGLED: CUserCharacInfo::AddCurCharacChaosPoint(int)
    int AddCurCharacChaosPoint(int point);

    // IDA_EA: 0x8645FB8
    // IDA_NAME: _ZN15CUserCharacInfo22SubCurCharacChaosPointEi
    // DEMANGLED: CUserCharacInfo::SubCurCharacChaosPoint(int)
    int SubCurCharacChaosPoint(int point);

    // IDA_EA: 0x8646010
    // IDA_NAME: _ZN15CUserCharacInfo24GmSetCurCharacChaosPointEi
    // DEMANGLED: CUserCharacInfo::GmSetCurCharacChaosPoint(int)
    int GmSetCurCharacChaosPoint(int point);

    // IDA_EA: 0x84810B4
    // IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosStateTimeEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosStateTime(void)
    int GetCurCharacChaosStateTime();

    // IDA_EA: 0x82F0AFA
    // IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosModeCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosModeCount(void)
    int GetCurCharacChaosModeCount();

    // IDA_EA: 0x82F0B1C
    // IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosKillCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosKillCount(void)
    int GetCurCharacChaosKillCount();

    // IDA_EA: 0x82F0B3E
    // IDA_NAME: _ZN15CUserCharacInfo25GetCurCharacChaosDieCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosDieCount(void)
    int GetCurCharacChaosDieCount();

    // IDA_EA: 0x82F0B60
    // IDA_NAME: _ZN15CUserCharacInfo22SetCurCharacSuperStateEb
    // DEMANGLED: CUserCharacInfo::SetCurCharacSuperState(bool)
    int SetCurCharacSuperState(bool state);

    // IDA_EA: 0x82F0B88
    // IDA_NAME: _ZN15CUserCharacInfo24GetCurCharacChaosDieTimeEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosDieTime(void)
    int GetCurCharacChaosDieTime();

    // IDA_EA: 0x82F0BAA
    // IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacChaosModeCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosModeCount(int)
    int SetCurCharacChaosModeCount(int count);

    // IDA_EA: 0x82F0BC8
    // IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacChaosKillCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosKillCount(int)
    int SetCurCharacChaosKillCount(int count);

    // IDA_EA: 0x82F0BE6
    // IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacChaosDieCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosDieCount(int)
    int SetCurCharacChaosDieCount(int count);

    // IDA_EA: 0x82F0C04
    // IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacChaosDieTimeEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosDieTime(int)
    int SetCurCharacChaosDieTime(int time);

    // IDA_EA: 0x82F0C30
    // IDA_NAME: _ZN15CUserCharacInfo17SetCurCharacGhostEb
    // DEMANGLED: CUserCharacInfo::SetCurCharacGhost(bool)
    int SetCurCharacGhost(bool ghost);

    // IDA_EA: 0x82F0C64
    // IDA_NAME: _ZN15CUserCharacInfo26IncCurCharacChaosModeCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacChaosModeCount(void)
    int IncCurCharacChaosModeCount();

    // IDA_EA: 0x82F0C94
    // IDA_NAME: _ZN15CUserCharacInfo26IncCurCharacChaosKillCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacChaosKillCount(void)
    int IncCurCharacChaosKillCount();

    // IDA_EA: 0x82F0CC4
    // IDA_NAME: _ZN15CUserCharacInfo25IncCurCharacChaosDieCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacChaosDieCount(void)
    int IncCurCharacChaosDieCount();

    // IDA_EA: 0x822F9C8
    // IDA_NAME: _ZN15CUserCharacInfo20GetUserPowerWarPointEv
    // DEMANGLED: CUserCharacInfo::GetUserPowerWarPoint(void)
    int GetUserPowerWarPoint();

    // IDA_EA: 0x8687EFC
    // IDA_NAME: _ZN15CUserCharacInfo21IncreasePowerWarPointEs
    // DEMANGLED: CUserCharacInfo::IncreasePowerWarPoint(short)
    int IncreasePowerWarPoint(int16_t value);

    // IDA_EA: 0x8687F66
    // IDA_NAME: _ZN15CUserCharacInfo21DecreasePowerWarPointEs
    // DEMANGLED: CUserCharacInfo::DecreasePowerWarPoint(short)
    int DecreasePowerWarPoint(int16_t value);

    // IDA_EA: 0x82F0982
    // IDA_NAME: _ZN15CUserCharacInfo23SetPowerWarLastPlayTimeEl
    // DEMANGLED: CUserCharacInfo::SetPowerWarLastPlayTime(long)
    int SetPowerWarLastPlayTime(long time);

    // IDA_EA: 0x8481052
    // IDA_NAME: _ZNK15CUserCharacInfo32getCurCharacLastPlayTickPowerWarEv
    // DEMANGLED: CUserCharacInfo::getCurCharacLastPlayTickPowerWar(void)const
    int getCurCharacLastPlayTickPowerWar() const;

    // IDA_EA: 0x8481060
    // IDA_NAME: _ZN15CUserCharacInfo32setCurCharacLastPlayTickPowerWarEl
    // DEMANGLED: CUserCharacInfo::setCurCharacLastPlayTickPowerWar(long)
    int setCurCharacLastPlayTickPowerWar(long tick);

    // IDA_EA: 0x82F09A0
    // IDA_NAME: _ZNK15CUserCharacInfo14isJoinPowerWarEv
    // DEMANGLED: CUserCharacInfo::isJoinPowerWar(void)const
    int isJoinPowerWar() const;

    // IDA_EA: 0x8481080
    // IDA_NAME: _ZN15CUserCharacInfo22setCurrCharacJoinPowerEc
    // DEMANGLED: CUserCharacInfo::setCurrCharacJoinPower(char)
    int setCurrCharacJoinPower(char join);

    // IDA_EA: 0x82F09C4
    // IDA_NAME: _ZN15CUserCharacInfo13setPowerWarHPEt
    // DEMANGLED: CUserCharacInfo::setPowerWarHP(unsigned short)
    int setPowerWarHP(uint16_t hp);

    // IDA_EA: 0x82F09EE
    // IDA_NAME: _ZN15CUserCharacInfo13getPowerWarHPEv
    // DEMANGLED: CUserCharacInfo::getPowerWarHP(void)
    int getPowerWarHP();

    // IDA_EA: 0x82F0A12
    // IDA_NAME: _ZNK15CUserCharacInfo20getStraightVictoriesEv
    // DEMANGLED: CUserCharacInfo::getStraightVictories(void)const
    int getStraightVictories() const;

    // IDA_EA: 0x84B42F4
    // IDA_NAME: _ZN15CUserCharacInfo20setStraightVictoriesEt
    // DEMANGLED: CUserCharacInfo::setStraightVictories(unsigned short)
    int setStraightVictories(uint16_t victories);

    // IDA_EA: 0x82F0A36
    // IDA_NAME: _ZN15CUserCharacInfo20incStraightVictoriesEv
    // DEMANGLED: CUserCharacInfo::incStraightVictories(void)
    int incStraightVictories();

    // IDA_EA: 0x82F0A5C
    // IDA_NAME: _ZN15CUserCharacInfo22resetStraightVictoriesEv
    // DEMANGLED: CUserCharacInfo::resetStraightVictories(void)
    int resetStraightVictories();

    // IDA_EA: 0x82F0A7A
    // IDA_NAME: _ZN15CUserCharacInfo15SetAssaultPlaceEi
    // DEMANGLED: CUserCharacInfo::SetAssaultPlace(int)
    int SetAssaultPlace(int place);

    // IDA_EA: 0x82F0A98
    // IDA_NAME: _ZN15CUserCharacInfo15SetAssaultCountEi
    // DEMANGLED: CUserCharacInfo::SetAssaultCount(int)
    int SetAssaultCount(int count);

    // IDA_EA: 0x82F0AB6
    // IDA_NAME: _ZN15CUserCharacInfo15GetAssaultCountEv
    // DEMANGLED: CUserCharacInfo::GetAssaultCount(void)
    int GetAssaultCount();

    // IDA_EA: 0x82F0CF4
    // IDA_NAME: _ZN15CUserCharacInfo24IncCurCharacAssaultCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacAssaultCount(void)
    int IncCurCharacAssaultCount();

    // IDA_EA: 0x82F0D24
    // IDA_NAME: _ZN15CUserCharacInfo21GetUserStateByAssaultEv
    // DEMANGLED: CUserCharacInfo::GetUserStateByAssault(void)
    int GetUserStateByAssault();

    // IDA_EA: 0x82F0D80
    // IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacLeaveTimeoutEb
    // DEMANGLED: CUserCharacInfo::SetCurCharacLeaveTimeout(bool)
    int SetCurCharacLeaveTimeout(bool timeout);

    // IDA_EA: 0x82F0DC6
    // IDA_NAME: _ZN15CUserCharacInfo23IncPowerWarAssaultCountEv
    // DEMANGLED: CUserCharacInfo::IncPowerWarAssaultCount(void)
    int IncPowerWarAssaultCount();

    // IDA_EA: 0x82F0DF8
    // IDA_NAME: _ZN15CUserCharacInfo30IncPowerWarAssaultVictoryCountEv
    // DEMANGLED: CUserCharacInfo::IncPowerWarAssaultVictoryCount(void)
    int IncPowerWarAssaultVictoryCount();

    // IDA_EA: 0x84EC3CA
    // IDA_NAME: _ZN15CUserCharacInfo26ResetPowerWarAssaultRecordEv
    // DEMANGLED: CUserCharacInfo::ResetPowerWarAssaultRecord(void)
    int ResetPowerWarAssaultRecord();

    // IDA_EA: 0x84EC408
    // IDA_NAME: _ZN15CUserCharacInfo23GetPowerWarAssaultCountEv
    // DEMANGLED: CUserCharacInfo::GetPowerWarAssaultCount(void)
    int GetPowerWarAssaultCount();

    // IDA_EA: 0x84EC42A
    // IDA_NAME: _ZN15CUserCharacInfo30GetPowerWarAssaultVictoryCountEv
    // DEMANGLED: CUserCharacInfo::GetPowerWarAssaultVictoryCount(void)
    int GetPowerWarAssaultVictoryCount();

    // C17: UpdateData persistence state is owned by the verified same-address
    // CUserCharacInfo base, so these retain their original member ABI here.

    // IDA_EA: 0x822F5C4
    // IDA_NAME: _ZN15CUserCharacInfo20get_charac_memberkeyEv
    // DEMANGLED: CUserCharacInfo::get_charac_memberkey(void)
    int get_charac_memberkey();

    // IDA_EA: 0x822F612
    // IDA_NAME: _ZNK15CUserCharacInfo33get_member_pay_tex_money_to_upperEv
    // DEMANGLED: CUserCharacInfo::get_member_pay_tex_money_to_upper(void)const
    int get_member_pay_tex_money_to_upper() const;

    // IDA_EA: 0x822F652
    // IDA_NAME: _ZNK15CUserCharacInfo35get_member_pay_tex_fatigue_to_upperEv
    // DEMANGLED: CUserCharacInfo::get_member_pay_tex_fatigue_to_upper(void)const
    int get_member_pay_tex_fatigue_to_upper() const;

    // IDA_EA: 0x822F5E6
    // IDA_NAME: _ZN15CUserCharacInfo23is_connect_upper_memberEv
    // DEMANGLED: CUserCharacInfo::is_connect_upper_member(void)
    int is_connect_upper_member();

    // IDA_EA: 0x822F634
    // IDA_NAME: _ZN15CUserCharacInfo33set_member_pay_tex_money_to_upperEi
    // DEMANGLED: CUserCharacInfo::set_member_pay_tex_money_to_upper(int)
    int set_member_pay_tex_money_to_upper(int value);

    // IDA_EA: 0x822F674
    // IDA_NAME: _ZN15CUserCharacInfo35set_member_pay_tex_fatigue_to_upperEi
    // DEMANGLED: CUserCharacInfo::set_member_pay_tex_fatigue_to_upper(int)
    int set_member_pay_tex_fatigue_to_upper(int value);

    // IDA_EA: 0x863BDC4
    // IDA_NAME: _ZN15CUserCharacInfo11isSaveInvenEv
    // DEMANGLED: CUserCharacInfo::isSaveInven(void)
    int isSaveInven();

    // IDA_EA: 0x86962EA
    // IDA_NAME: _ZN15CUserCharacInfo11isSaveCargoEv
    // DEMANGLED: CUserCharacInfo::isSaveCargo(void)
    int isSaveCargo();

    // IDA_EA: 0x84EBF18
    // IDA_NAME: _ZN15CUserCharacInfo16disableSaveInvenEv
    // DEMANGLED: CUserCharacInfo::disableSaveInven(void)
    CUserCharacInfo* disableSaveInven();

    // IDA_EA: 0x84EBF48
    // IDA_NAME: _ZN15CUserCharacInfo16disableSaveCargoEv
    // DEMANGLED: CUserCharacInfo::disableSaveCargo(void)
    CUserCharacInfo* disableSaveCargo();

    // IDA_EA: 0x86962AE
    // IDA_NAME: _ZN15CUserCharacInfo11isSaveSkillEv
    // DEMANGLED: CUserCharacInfo::isSaveSkill(void)
    int isSaveSkill();

    // IDA_EA: 0x84EBF24
    // IDA_NAME: _ZN15CUserCharacInfo16disableSaveSkillEv
    // DEMANGLED: CUserCharacInfo::disableSaveSkill(void)
    CUserCharacInfo* disableSaveSkill();

    // IDA_EA: 0x822F130
    // IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacSkillREv
    // DEMANGLED: CUserCharacInfo::getCurCharacSkillR(void)const
    SkillSlot* getCurCharacSkillR() const;

    // IDA_EA: 0x86962BA
    // IDA_NAME: _ZN15CUserCharacInfo9isSavePvPEv
    // DEMANGLED: CUserCharacInfo::isSavePvP(void)
    int isSavePvP();

    // IDA_EA: 0x84EBF30
    // IDA_NAME: _ZN15CUserCharacInfo14disableSavePvPEv
    // DEMANGLED: CUserCharacInfo::disableSavePvP(void)
    CUserCharacInfo* disableSavePvP();

    // IDA_EA: 0x822F15E
    // IDA_NAME: _ZNK15CUserCharacInfo16getPVPResultRefREv
    // DEMANGLED: CUserCharacInfo::getPVPResultRefR(void)const
    const PvpResultType* getPVPResultRefR() const;

    // IDA_EA: 0x822F16A
    // IDA_NAME: _ZN15CUserCharacInfo13getPVPResultWEv
    // DEMANGLED: CUserCharacInfo::getPVPResultW(void)
    PvpResultType* getPVPResultW();

    // IDA_EA: 0x84EBEC4
    // IDA_NAME: _ZN15CUserCharacInfo16getPVPResultRefWEv
    // DEMANGLED: CUserCharacInfo::getPVPResultRefW(void)
    PvpResultType* getPVPResultRefW();

    // IDA_EA: 0x850D488
    // IDA_NAME: _ZNK15CUserCharacInfo13getPVPResultREv
    // DEMANGLED: CUserCharacInfo::getPVPResultR(void)const
    const PvpResultType* getPVPResultR() const;

    // IDA_EA: 0x85DF99A
    // IDA_NAME: _ZNK15CUserCharacInfo24get_pvp_result_exp_pointEv
    // DEMANGLED: CUserCharacInfo::get_pvp_result_exp_point(void)const
    int get_pvp_result_exp_point() const;

    // IDA_EA: 0x85DF9A6
    // IDA_NAME: _ZNK15CUserCharacInfo27get_pvp_result_oldexp_pointEv
    // DEMANGLED: CUserCharacInfo::get_pvp_result_oldexp_point(void)const
    int get_pvp_result_oldexp_point() const;

    // IDA_EA: 0x86962D2
    // IDA_NAME: _ZN15CUserCharacInfo18isSaveDungeonClearEv
    // DEMANGLED: CUserCharacInfo::isSaveDungeonClear(void)
    int isSaveDungeonClear();

    // IDA_EA: 0x84EBF3C
    // IDA_NAME: _ZN15CUserCharacInfo23disableSaveDungeonClearEv
    // DEMANGLED: CUserCharacInfo::disableSaveDungeonClear(void)
    CUserCharacInfo* disableSaveDungeonClear();

    // IDA_EA: 0x85BFAB2
    // IDA_NAME: _ZNK15CUserCharacInfo19getDungeonClearRefREv
    // DEMANGLED: CUserCharacInfo::getDungeonClearRefR(void)const
    void* getDungeonClearRefR() const;

    // IDA_EA: 0x84B42CC
    // IDA_NAME: _ZN15CUserCharacInfo19getDungeonClearRefWEv
    // DEMANGLED: CUserCharacInfo::getDungeonClearRefW(void)
    WongWork::CDungeonClear* getDungeonClearRefW();

    // IDA_EA: 0x84B42E8
    // IDA_NAME: _ZN15CUserCharacInfo22enableSaveDungeonClearEv
    // DEMANGLED: CUserCharacInfo::enableSaveDungeonClear(void)
    CUserCharacInfo* enableSaveDungeonClear();

    // IDA_EA: 0x86962DE
    // IDA_NAME: _ZN15CUserCharacInfo18isSaveBattleRecordEv
    // DEMANGLED: CUserCharacInfo::isSaveBattleRecord(void)
    int isSaveBattleRecord();

    // IDA_EA: 0x8696266
    // IDA_NAME: _ZN15CUserCharacInfo23disableSaveBattleRecordEv
    // DEMANGLED: CUserCharacInfo::disableSaveBattleRecord(void)
    CUserCharacInfo* disableSaveBattleRecord();

    // IDA_EA: 0x86962C6
    // IDA_NAME: _ZN15CUserCharacInfo11isSaveQuestEv
    // DEMANGLED: CUserCharacInfo::isSaveQuest(void)
    int isSaveQuest();

    // IDA_EA: 0x869625A
    // IDA_NAME: _ZN15CUserCharacInfo16disableSaveQuestEv
    // DEMANGLED: CUserCharacInfo::disableSaveQuest(void)
    CUserCharacInfo* disableSaveQuest();

    // IDA_EA: 0x86964A6
    // IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacEventItemCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacEventItemCount(void)
    int GetCurCharacEventItemCount();

    // IDA_EA: 0x8696488
    // IDA_NAME: _ZN15CUserCharacInfo28ResetCurCharacEventItemCountEv
    // DEMANGLED: CUserCharacInfo::ResetCurCharacEventItemCount(void)
    int ResetCurCharacEventItemCount();

    // IDA_EA: 0x8110D52
    // IDA_NAME: _ZNK15CUserCharacInfo29getCurUseBringUpMaterialCountEv
    // DEMANGLED: CUserCharacInfo::getCurUseBringUpMaterialCount(void)const
    int getCurUseBringUpMaterialCount() const;

    // IDA_EA: 0x8110D36
    // IDA_NAME: _ZN15CUserCharacInfo31resetCurUseBringUpMaterialCountEv
    // DEMANGLED: CUserCharacInfo::resetCurUseBringUpMaterialCount(void)
    int resetCurUseBringUpMaterialCount();

    // IDA_EA: 0x86964E8
    // IDA_NAME: _ZN15CUserCharacInfo29GetCurCharacEventAddItemCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacEventAddItemCount(void)
    int GetCurCharacEventAddItemCount();

    // IDA_EA: 0x86964CA
    // IDA_NAME: _ZN15CUserCharacInfo31ResetCurCharacEventAddItemCountEv
    // DEMANGLED: CUserCharacInfo::ResetCurCharacEventAddItemCount(void)
    int ResetCurCharacEventAddItemCount();

    // IDA_EA: 0x8696528
    // IDA_NAME: _ZN15CUserCharacInfo23getCurCharacSchoolPointEv
    // DEMANGLED: CUserCharacInfo::getCurCharacSchoolPoint(void)
    int getCurCharacSchoolPoint();

    // IDA_EA: 0x869650C
    // IDA_NAME: _ZN15CUserCharacInfo25resetCurCharacSchoolPointEv
    // DEMANGLED: CUserCharacInfo::resetCurCharacSchoolPoint(void)
    int resetCurCharacSchoolPoint();

    // IDA_EA: 0x8696196
    // IDA_NAME: _ZN15CUserCharacInfo23isBloodBestRecordUpdateEv
    // DEMANGLED: CUserCharacInfo::isBloodBestRecordUpdate(void)
    int isBloodBestRecordUpdate();

    // IDA_EA: 0x8687FD8
    // IDA_NAME: _ZN15CUserCharacInfo18setBloodBestRecordEjiib
    // DEMANGLED: CUserCharacInfo::setBloodBestRecord(unsigned int,int,int,bool)
    int setBloodBestRecord(unsigned int play_time,
                           int max_round,
                           int dungeon_index,
                           bool from_database);

    // IDA_EA: 0x84EC474
    // IDA_NAME: _ZN15CUserCharacInfo22getLinkCharacSlotIndexEv
    // DEMANGLED: CUserCharacInfo::getLinkCharacSlotIndex(void)
    int getLinkCharacSlotIndex();

    // IDA_EA: 0x8696B22
    // IDA_NAME: _ZN15CUserCharacInfo22setLinkCharacSlotIndexEc
    // DEMANGLED: CUserCharacInfo::setLinkCharacSlotIndex(char)
    int setLinkCharacSlotIndex(char index);

    // IDA_EA: 0x8696B4A
    // IDA_NAME: _ZN15CUserCharacInfo17getCharacLinkTypeEv
    // DEMANGLED: CUserCharacInfo::getCharacLinkType(void)
    int getCharacLinkType();

    // IDA_EA: 0x8696B6E
    // IDA_NAME: _ZN15CUserCharacInfo21getCharacLinkBonusExpEv
    // DEMANGLED: CUserCharacInfo::getCharacLinkBonusExp(void)
    int getCharacLinkBonusExp();

    // IDA_EA: 0x8696BAE
    // IDA_NAME: _ZN15CUserCharacInfo22getCharacLinkBonusGoldEv
    // DEMANGLED: CUserCharacInfo::getCharacLinkBonusGold(void)
    int getCharacLinkBonusGold();

    // IDA_EA: 0x8690ED8
    // IDA_NAME: _ZN15CUserCharacInfo25resetBlueMarbleEnterCountEv
    // DEMANGLED: CUserCharacInfo::resetBlueMarbleEnterCount(void)
    int resetBlueMarbleEnterCount();

    // IDA_EA: 0x8690EF4
    // IDA_NAME: _ZN15CUserCharacInfo23setBlueMarbleEnterCountEh
    // DEMANGLED: CUserCharacInfo::setBlueMarbleEnterCount(unsigned char)
    int setBlueMarbleEnterCount(uint8_t count);

    // IDA_EA: 0x8690F1C
    // IDA_NAME: _ZN15CUserCharacInfo28increaseBlueMarbleEnterCountEv
    // DEMANGLED: CUserCharacInfo::increaseBlueMarbleEnterCount(void)
    int increaseBlueMarbleEnterCount();

    // IDA_EA: 0x8690F42
    // IDA_NAME: _ZNK15CUserCharacInfo23getBlueMarbleEnterCountEv
    // DEMANGLED: CUserCharacInfo::getBlueMarbleEnterCount(void)const
    int getBlueMarbleEnterCount() const;

    // IDA_EA: 0x8690F66
    // IDA_NAME: _ZN15CUserCharacInfo28resetBlueMarbleUserDeadCountEv
    // DEMANGLED: CUserCharacInfo::resetBlueMarbleUserDeadCount(void)
    int resetBlueMarbleUserDeadCount();

    // IDA_EA: 0x8690F86
    // IDA_NAME: _ZN15CUserCharacInfo31increaseBlueMarbleUserDeadCountEv
    // DEMANGLED: CUserCharacInfo::increaseBlueMarbleUserDeadCount(void)
    int increaseBlueMarbleUserDeadCount();

    // IDA_EA: 0x8690FAA
    // IDA_NAME: _ZNK15CUserCharacInfo26getBlueMarbleUserDeadCountEv
    // DEMANGLED: CUserCharacInfo::getBlueMarbleUserDeadCount(void)const
    int getBlueMarbleUserDeadCount() const;

    // IDA_EA: 0x8696306
    // IDA_NAME: _ZNK15CUserCharacInfo10GetFatigueEv
    // DEMANGLED: CUserCharacInfo::GetFatigue(void)const
    int GetFatigue() const;

    // IDA_EA: 0x8696326
    // IDA_NAME: _ZN15CUserCharacInfo30decCurCharacFatigueGrownUpBuffEj
    // DEMANGLED: CUserCharacInfo::decCurCharacFatigueGrownUpBuff(unsigned int)
    int decCurCharacFatigueGrownUpBuff(uint32_t value);

    // IDA_EA: 0x8696386
    // IDA_NAME: _ZN15CUserCharacInfo29resetCharacFatigueGrownUpBuffEv
    // DEMANGLED: CUserCharacInfo::resetCharacFatigueGrownUpBuff(void)
    int resetCharacFatigueGrownUpBuff();

    // IDA_EA: 0x84A852C
    // IDA_NAME: _ZNK15CUserCharacInfo30getCurCharacFatigueGrownUpBuffEv
    // DEMANGLED: CUserCharacInfo::getCurCharacFatigueGrownUpBuff(void)const
    int getCurCharacFatigueGrownUpBuff() const;

    // IDA_EA: 0x84A854C
    // IDA_NAME: _ZN15CUserCharacInfo30setCurCharacFatigueGrownUpBuffEt
    // DEMANGLED: CUserCharacInfo::setCurCharacFatigueGrownUpBuff(unsigned short)
    int setCurCharacFatigueGrownUpBuff(uint16_t fatigue);

    // IDA_EA: 0x84A857E
    // IDA_NAME: _ZN15CUserCharacInfo25GetFatigueBatteryChargingEv
    // DEMANGLED: CUserCharacInfo::GetFatigueBatteryCharging(void)
    int GetFatigueBatteryCharging();

    // IDA_EA: 0x86566E4
    // IDA_NAME: _ZN15CUserCharacInfo25IncFatigueBatteryChargingEs
    // DEMANGLED: CUserCharacInfo::IncFatigueBatteryCharging(short)
    int IncFatigueBatteryCharging(int16_t fatigue);

    // IDA_EA: 0x8656784
    // IDA_NAME: _ZN15CUserCharacInfo25DecFatigueBatteryChargingEs
    // DEMANGLED: CUserCharacInfo::DecFatigueBatteryCharging(short)
    int DecFatigueBatteryCharging(int16_t fatigue);

    // IDA_EA: 0x84EBF54
    // IDA_NAME: _ZN15CUserCharacInfo30incCurCharacMemberBonusFatigueEs
    // DEMANGLED: CUserCharacInfo::incCurCharacMemberBonusFatigue(short)
    int incCurCharacMemberBonusFatigue(int16_t fatigue);

    // IDA_EA: 0x84EBF90
    // IDA_NAME: _ZN15CUserCharacInfo32resetCurCharacMemberBonusFatigueEv
    // DEMANGLED: CUserCharacInfo::resetCurCharacMemberBonusFatigue(void)
    int resetCurCharacMemberBonusFatigue();

    // IDA_EA: 0x84EBFB2
    // IDA_NAME: _ZN15CUserCharacInfo30getCurCharacMemberBonusFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacMemberBonusFatigue(void)
    int getCurCharacMemberBonusFatigue();

    // IDA_EA: 0x86963B0
    // IDA_NAME: _ZNK15CUserCharacInfo26getCurCharacPremiumFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacPremiumFatigue(void)const
    int getCurCharacPremiumFatigue() const;

    // IDA_EA: 0x86963D0
    // IDA_NAME: _ZN15CUserCharacInfo26setCurCharacPremiumFatigueEt
    // DEMANGLED: CUserCharacInfo::setCurCharacPremiumFatigue(unsigned short)
    int setCurCharacPremiumFatigue(uint16_t fatigue);

    // IDA_EA: 0x8696402
    // IDA_NAME: _ZNK15CUserCharacInfo29getCurCharacMaxPremiumFatigueEv
    // DEMANGLED: CUserCharacInfo::getCurCharacMaxPremiumFatigue(void)const
    int getCurCharacMaxPremiumFatigue() const;

    // IDA_EA: 0x8696422
    // IDA_NAME: _ZN15CUserCharacInfo23incCurCharacUsedFatigueEt
    // DEMANGLED: CUserCharacInfo::incCurCharacUsedFatigue(unsigned short)
    int incCurCharacUsedFatigue(uint16_t fatigue);

    // IDA_EA: 0x8655BCE
    // IDA_NAME: _ZN15CUserCharacInfo24calcCurCharacSchoolPointEi
    // DEMANGLED: CUserCharacInfo::calcCurCharacSchoolPoint(int)
    unsigned int calcCurCharacSchoolPoint(int used_fatigue_before);

    // IDA_EA: 0x869645E
    // IDA_NAME: _ZN15CUserCharacInfo25resetCurCharacUsedFatigueEv
    // DEMANGLED: CUserCharacInfo::resetCurCharacUsedFatigue(void)
    int resetCurCharacUsedFatigue();

    // IDA_EA: 0x8696664
    // IDA_NAME: _ZN15CUserCharacInfo19addCurcharacStaminaEh
    // DEMANGLED: CUserCharacInfo::addCurcharacStamina(unsigned char)
    int addCurcharacStamina(uint8_t value);

    // IDA_EA: 0x8335C14
    // IDA_NAME: _ZNK15CUserCharacInfo30GetCurConditionEventRewardStepEv
    // DEMANGLED: CUserCharacInfo::GetCurConditionEventRewardStep(void)const
    int GetCurConditionEventRewardStep() const;

    // IDA_EA: 0x8335BA8
    // IDA_NAME: _ZNK15CUserCharacInfo24GetCurConditionEventStepEv
    // DEMANGLED: CUserCharacInfo::GetCurConditionEventStep(void)const
    int GetCurConditionEventStep() const;

    // IDA_EA: 0x8335C80
    // IDA_NAME: _ZNK15CUserCharacInfo26GetProperDungeonClearCountEv
    // DEMANGLED: CUserCharacInfo::GetProperDungeonClearCount(void)const
    int GetProperDungeonClearCount() const;

    // IDA_EA: 0x8696930
    // IDA_NAME: _ZN15CUserCharacInfo20GetCurCharacChaosExpEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosExp(void)
    int GetCurCharacChaosExp();

    // IDA_EA: 0x8696952
    // IDA_NAME: _ZN15CUserCharacInfo19GetCurCharacEmotionEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacEmotion(void)
    int GetCurCharacEmotion();

    // IDA_EA: 0x8696976
    // IDA_NAME: _ZNK15CUserCharacInfo23IsVillageAttackDBUpdateEv
    // DEMANGLED: CUserCharacInfo::IsVillageAttackDBUpdate(void)const
    int IsVillageAttackDBUpdate() const;

    // IDA_EA: 0x869699A
    // IDA_NAME: _ZN15CUserCharacInfo31SetCurCharacAssaultFatigueCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacAssaultFatigueCount(int)
    int SetCurCharacAssaultFatigueCount(int count);

    // IDA_EA: 0x86969B8
    // IDA_NAME: _ZN15CUserCharacInfo31GetCurCharacAssaultFatigueCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacAssaultFatigueCount(void)
    int GetCurCharacAssaultFatigueCount();

    // IDA_EA: 0x86969DA
    // IDA_NAME: _ZN15CUserCharacInfo31IncCurCharacAssaultFatigueCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacAssaultFatigueCount(void)
    int IncCurCharacAssaultFatigueCount();

    // IDA_EA: 0x85BFC5A
    // IDA_NAME: _ZN15CUserCharacInfo28SetCurCharacDungeonPlayCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacDungeonPlayCount(int)
    int SetCurCharacDungeonPlayCount(int count);

    // IDA_EA: 0x85BFC78
    // IDA_NAME: _ZNK15CUserCharacInfo28GetCurCharacDungeonPlayCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacDungeonPlayCount(void)const
    int GetCurCharacDungeonPlayCount() const;

    // IDA_EA: 0x85BFC9A
    // IDA_NAME: _ZN15CUserCharacInfo28IncCurCharacDungeonPlayCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacDungeonPlayCount(void)
    int IncCurCharacDungeonPlayCount();

    // IDA_EA: 0x84EC256
    // IDA_NAME: _ZN15CUserCharacInfo24SetCurConditionEventStepEs
    // DEMANGLED: CUserCharacInfo::SetCurConditionEventStep(short)
    int SetCurConditionEventStep(int16_t step);

    // IDA_EA: 0x84EC23A
    // IDA_NAME: _ZN15CUserCharacInfo26ClearCurConditionEventStepEv
    // DEMANGLED: CUserCharacInfo::ClearCurConditionEventStep(void)
    int ClearCurConditionEventStep();

    // IDA_EA: 0x8335BCC
    // IDA_NAME: _ZN15CUserCharacInfo24IncCurConditionEventStepEv
    // DEMANGLED: CUserCharacInfo::IncCurConditionEventStep(void)
    int IncCurConditionEventStep();

    // IDA_EA: 0x84EC29C
    // IDA_NAME: _ZN15CUserCharacInfo30SetCurConditionEventRewardStepEs
    // DEMANGLED: CUserCharacInfo::SetCurConditionEventRewardStep(short)
    int SetCurConditionEventRewardStep(int16_t step);

    // IDA_EA: 0x84EC280
    // IDA_NAME: _ZN15CUserCharacInfo32ClearCurConditionEventRewardStepEv
    // DEMANGLED: CUserCharacInfo::ClearCurConditionEventRewardStep(void)
    int ClearCurConditionEventRewardStep();

    // IDA_EA: 0x8335C38
    // IDA_NAME: _ZN15CUserCharacInfo30IncCurConditionEventRewardStepEv
    // DEMANGLED: CUserCharacInfo::IncCurConditionEventRewardStep(void)
    int IncCurConditionEventRewardStep();

    // IDA_EA: 0x84EC2E2
    // IDA_NAME: _ZN15CUserCharacInfo26SetProperDungeonClearCountEs
    // DEMANGLED: CUserCharacInfo::SetProperDungeonClearCount(short)
    int SetProperDungeonClearCount(int16_t count);

    // IDA_EA: 0x84EC2C6
    // IDA_NAME: _ZN15CUserCharacInfo28ClearProperDungeonClearCountEv
    // DEMANGLED: CUserCharacInfo::ClearProperDungeonClearCount(void)
    int ClearProperDungeonClearCount();

    // IDA_EA: 0x85BFBFC
    // IDA_NAME: _ZN15CUserCharacInfo26IncProperDungeonClearCountEv
    // DEMANGLED: CUserCharacInfo::IncProperDungeonClearCount(void)
    int IncProperDungeonClearCount();

    // IDA_EA: 0x8335CA4
    // IDA_NAME: _ZN15CUserCharacInfo19IsProperDungeonUserEv
    // DEMANGLED: CUserCharacInfo::IsProperDungeonUser(void)
    int IsProperDungeonUser();

    // IDA_EA: 0x85BFC22
    // IDA_NAME: _ZN15CUserCharacInfo20SetProperDungeonUserEv
    // DEMANGLED: CUserCharacInfo::SetProperDungeonUser(void)
    int SetProperDungeonUser();

    // IDA_EA: 0x85BFC3E
    // IDA_NAME: _ZN15CUserCharacInfo22ClearProperDungeonUserEv
    // DEMANGLED: CUserCharacInfo::ClearProperDungeonUser(void)
    int ClearProperDungeonUser();

    // IDA_EA: 0x85BFAEC
    // IDA_NAME: _ZNK15CUserCharacInfo20getMemberDungeonFlagEv
    // DEMANGLED: CUserCharacInfo::getMemberDungeonFlag(void)const
    int getMemberDungeonFlag() const;

    // IDA_EA: 0x85BFB10
    // IDA_NAME: _ZN15CUserCharacInfo20setMemberDungeonFlagEb
    // DEMANGLED: CUserCharacInfo::setMemberDungeonFlag(bool)
    int setMemberDungeonFlag(bool flag);

    // IDA_EA: 0x85BFBD6
    // IDA_NAME: _ZN15CUserCharacInfo28DecreaseRevengeDungeonTicketEv
    // DEMANGLED: CUserCharacInfo::DecreaseRevengeDungeonTicket(void)
    int DecreaseRevengeDungeonTicket();

    // IDA_EA: 0x84EC1A0
    // IDA_NAME: _ZN15CUserCharacInfo23SetCurCharacRevengeDataEhh
    // DEMANGLED: CUserCharacInfo::SetCurCharacRevengeData(unsigned char,unsigned char)
    int SetCurCharacRevengeData(uint8_t attack_count, uint8_t revenge_count);

    // IDA_EA: 0x84EC1DE
    // IDA_NAME: _ZN15CUserCharacInfo22VillageAttack_DBUpdateEv
    // DEMANGLED: CUserCharacInfo::VillageAttack_DBUpdate(void)
    int VillageAttack_DBUpdate();

    // IDA_EA: 0x84EC1FA
    // IDA_NAME: _ZN15CUserCharacInfo25VillageAttack_NotDBUpdateEv
    // DEMANGLED: CUserCharacInfo::VillageAttack_NotDBUpdate(void)
    int VillageAttack_NotDBUpdate();

    // IDA_EA: 0x84EC216
    // IDA_NAME: _ZNK15CUserCharacInfo24GetCurVillageAttackCountEv
    // DEMANGLED: CUserCharacInfo::GetCurVillageAttackCount(void)const
    int GetCurVillageAttackCount() const;

    // IDA_EA: 0x85BFAC0
    // IDA_NAME: _ZN15CUserCharacInfo32setCurCharacLastPlayDungeonIndexEj
    // DEMANGLED: CUserCharacInfo::setCurCharacLastPlayDungeonIndex(unsigned int)
    int setCurCharacLastPlayDungeonIndex(uint32_t index);

    // IDA_EA: 0x84EBFC4
    // IDA_NAME: _ZN15CUserCharacInfo32getCurCharacLastPlayDungeonIndexEv
    // DEMANGLED: CUserCharacInfo::getCurCharacLastPlayDungeonIndex(void)
    int getCurCharacLastPlayDungeonIndex();

    // IDA_EA: 0x822F9EC
    // IDA_NAME: _ZN15CUserCharacInfo33updateLastItemLimitEditionBuyTimeEl
    // DEMANGLED: CUserCharacInfo::updateLastItemLimitEditionBuyTime(long)
    int updateLastItemLimitEditionBuyTime(long time);

    // IDA_EA: 0x822FA0A
    // IDA_NAME: _ZNK15CUserCharacInfo30getLastItemLimitEditionBuyTimeEv
    // DEMANGLED: CUserCharacInfo::getLastItemLimitEditionBuyTime(void)const
    int getLastItemLimitEditionBuyTime() const;

    // IDA_EA: 0x822FA2C
    // IDA_NAME: _ZN15CUserCharacInfo26setUpgradeSeparateTrueFlagEc
    // DEMANGLED: CUserCharacInfo::setUpgradeSeparateTrueFlag(char)
    int setUpgradeSeparateTrueFlag(char flag);

    // IDA_EA: 0x822FA54
    // IDA_NAME: _ZN15CUserCharacInfo27setUpgradeSeparateFalseFlagEc
    // DEMANGLED: CUserCharacInfo::setUpgradeSeparateFalseFlag(char)
    int setUpgradeSeparateFalseFlag(char flag);

    // IDA_EA: 0x822FA7C
    // IDA_NAME: _ZN15CUserCharacInfo28getCurChannelEquipslotSwitchEv
    // DEMANGLED: CUserCharacInfo::getCurChannelEquipslotSwitch(void)
    int getCurChannelEquipslotSwitch();

    // IDA_EA: 0x822FAA0
    // IDA_NAME: _ZN15CUserCharacInfo27getCurExpandEquipslotSwitchEv
    // DEMANGLED: CUserCharacInfo::getCurExpandEquipslotSwitch(void)
    int getCurExpandEquipslotSwitch();

    // IDA_EA: 0x822FAC4
    // IDA_NAME: _ZN15CUserCharacInfo29incCurUseBringUpMaterialCountEv
    // DEMANGLED: CUserCharacInfo::incCurUseBringUpMaterialCount(void)
    int incCurUseBringUpMaterialCount();

    // IDA_EA: 0x822FB00
    // IDA_NAME: _ZNK15CUserCharacInfo23get_growth_power_rewardEv
    // DEMANGLED: CUserCharacInfo::get_growth_power_reward(void)const
    int get_growth_power_reward() const;

    // IDA_EA: 0x822FB24
    // IDA_NAME: _ZN15CUserCharacInfo23set_growth_power_rewardEi
    // DEMANGLED: CUserCharacInfo::set_growth_power_reward(int)
    int set_growth_power_reward(int reward);

    // IDA_EA: 0x822FB50
    // IDA_NAME: _ZN15CUserCharacInfo33set_growth_power_exp_reward_rightEb
    // DEMANGLED: CUserCharacInfo::set_growth_power_exp_reward_right(bool)
    int set_growth_power_exp_reward_right(bool right);

    // IDA_EA: 0x8260494
    // IDA_NAME: _ZN15CUserCharacInfo29setConctractOfCubePremiumInfoEbh
    // DEMANGLED: CUserCharacInfo::setConctractOfCubePremiumInfo(bool,unsigned char)
    int setConctractOfCubePremiumInfo(bool enabled, uint8_t premium_type);

    // IDA_EA: 0x84EC580
    // IDA_NAME: _ZN15CUserCharacInfo29initContractOfCubePremiumInfoEbh
    // DEMANGLED: CUserCharacInfo::initContractOfCubePremiumInfo(bool,unsigned char)
    int initContractOfCubePremiumInfo(bool enabled, uint8_t premium_type);

    // IDA_EA: 0x827DEFA
    // IDA_NAME: _ZN15CUserCharacInfo17IsEndLetheAtLoginEv
    // DEMANGLED: CUserCharacInfo::IsEndLetheAtLogin(void)
    int IsEndLetheAtLogin();

    // IDA_EA: 0x84EBEDE
    // IDA_NAME: _ZN15CUserCharacInfo27reset_is_end_lethe_at_loginEv
    // DEMANGLED: CUserCharacInfo::reset_is_end_lethe_at_login(void)
    int reset_is_end_lethe_at_login();

    // IDA_EA: 0x82A66B8
    // IDA_NAME: _ZN15CUserCharacInfo34getCurCharacEventCharacterGrowtypeEv
    // DEMANGLED: CUserCharacInfo::getCurCharacEventCharacterGrowtype(void)
    int getCurCharacEventCharacterGrowtype();

    // IDA_EA: 0x82A66DC
    // IDA_NAME: _ZN15CUserCharacInfo23getCurCharacGuildSecedeEv
    // DEMANGLED: CUserCharacInfo::getCurCharacGuildSecede(void)
    int getCurCharacGuildSecede();

    // IDA_EA: 0x82A6700
    // IDA_NAME: _ZN15CUserCharacInfo23setCurCharacGuildSecedeEc
    // DEMANGLED: CUserCharacInfo::setCurCharacGuildSecede(char)
    int setCurCharacGuildSecede(char secede);

    // IDA_EA: 0x82A671E
    // IDA_NAME: _ZN15CUserCharacInfo19set_charac_guildkeyEj
    // DEMANGLED: CUserCharacInfo::set_charac_guildkey(unsigned int)
    int set_charac_guildkey(uint32_t guild_key);

    // IDA_EA: 0x82A673C
    // IDA_NAME: _ZN15CUserCharacInfo13get_guild_expEv
    // DEMANGLED: CUserCharacInfo::get_guild_exp(void)
    int get_guild_exp();

    // IDA_EA: 0x8645C76
    // IDA_NAME: _ZN15CUserCharacInfo13add_guild_expEi
    // DEMANGLED: CUserCharacInfo::add_guild_exp(int)
    int add_guild_exp(int exp);

    // IDA_EA: 0x8645DB4
    // IDA_NAME: _ZN15CUserCharacInfo13set_guild_expEi
    // DEMANGLED: CUserCharacInfo::set_guild_exp(int)
    int set_guild_exp(int exp);

    // IDA_EA: 0x82F0914
    // IDA_NAME: _ZN15CUserCharacInfo19setCurCharacStaminaEh
    // DEMANGLED: CUserCharacInfo::setCurCharacStamina(unsigned char)
    int setCurCharacStamina(uint8_t stamina);

    // IDA_EA: 0x82F093A
    // IDA_NAME: _ZN15CUserCharacInfo25setCurCharacBackupStaminaEh
    // DEMANGLED: CUserCharacInfo::setCurCharacBackupStamina(unsigned char)
    int setCurCharacBackupStamina(uint8_t stamina);

    // IDA_EA: 0x82F0960
    // IDA_NAME: _ZNK15CUserCharacInfo23getCurrCharacBlackCountEv
    // DEMANGLED: CUserCharacInfo::getCurrCharacBlackCount(void)const
    int getCurrCharacBlackCount() const;

    // IDA_EA: 0x84EC30C
    // IDA_NAME: _ZN15CUserCharacInfo36SetCurCharacLuckPointUpdateTimeCountEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacLuckPointUpdateTimeCount(int)
    int SetCurCharacLuckPointUpdateTimeCount(int count);

    // IDA_EA: 0x84EC32A
    // IDA_NAME: _ZN15CUserCharacInfo18SetOneDayLetheFlagEh
    // DEMANGLED: CUserCharacInfo::SetOneDayLetheFlag(unsigned char)
    int SetOneDayLetheFlag(uint8_t flag);

    // IDA_EA: 0x85C4B8C
    // IDA_NAME: _ZNK15CUserCharacInfo18GetOneDayLetheFlagEv
    // DEMANGLED: CUserCharacInfo::GetOneDayLetheFlag(void)const
    int GetOneDayLetheFlag() const;

    // IDA_EA: 0x84EC352
    // IDA_NAME: _ZN15CUserCharacInfo18SetIsInitSkillFlagEb
    // DEMANGLED: CUserCharacInfo::SetIsInitSkillFlag(bool)
    int SetIsInitSkillFlag(bool flag);

    // IDA_EA: 0x84EC37A
    // IDA_NAME: _ZN15CUserCharacInfo21SetOneDayLetheFlag2NDEh
    // DEMANGLED: CUserCharacInfo::SetOneDayLetheFlag2ND(unsigned char)
    int SetOneDayLetheFlag2ND(uint8_t flag);

    // IDA_EA: 0x85C4BB0
    // IDA_NAME: _ZNK15CUserCharacInfo21GetOneDayLetheFlag2NDEv
    // DEMANGLED: CUserCharacInfo::GetOneDayLetheFlag2ND(void)const
    int GetOneDayLetheFlag2ND() const;

    // IDA_EA: 0x84EC3A2
    // IDA_NAME: _ZN15CUserCharacInfo21SetIsInitSkillFlag2NDEb
    // DEMANGLED: CUserCharacInfo::SetIsInitSkillFlag2ND(bool)
    int SetIsInitSkillFlag2ND(bool flag);

    // IDA_EA: 0x84EC44C
    // IDA_NAME: _ZN15CUserCharacInfo22SetCurExpEquipSlotStatEc
    // DEMANGLED: CUserCharacInfo::SetCurExpEquipSlotStat(char)
    int SetCurExpEquipSlotStat(char stat);

    // IDA_EA: 0x84EC498
    // IDA_NAME: _ZN15CUserCharacInfo29setCurUseBringUpMaterialTotalEh
    // DEMANGLED: CUserCharacInfo::setCurUseBringUpMaterialTotal(unsigned char)
    int setCurUseBringUpMaterialTotal(uint8_t total);

    // IDA_EA: 0x863BDD0
    // IDA_NAME: _ZN15CUserCharacInfo36GetCurCharacLuckPointUpdateTimeCountEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacLuckPointUpdateTimeCount(void)
    int GetCurCharacLuckPointUpdateTimeCount();

    // IDA_EA: 0x863BDF2
    // IDA_NAME: _ZN15CUserCharacInfo36IncCurCharacLuckPointUpdateTimeCountEv
    // DEMANGLED: CUserCharacInfo::IncCurCharacLuckPointUpdateTimeCount(void)
    int IncCurCharacLuckPointUpdateTimeCount();

    // IDA_EA: 0x864670A
    // IDA_NAME: _ZN15CUserCharacInfo21SetCurCharacLuckPointEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacLuckPoint(int)
    int SetCurCharacLuckPoint(int luck_point);

    // IDA_EA: 0x822F828
    // IDA_NAME: _ZN15CUserCharacInfo21GetCurCharacLuckPointEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacLuckPoint(void)
    int GetCurCharacLuckPoint();

    // IDA_EA: 0x822F84A
    // IDA_NAME: _ZN15CUserCharacInfo21IncCurCharacLuckPointEi
    // DEMANGLED: CUserCharacInfo::IncCurCharacLuckPoint(int)
    char IncCurCharacLuckPoint(int delta);

    // IDA_EA: 0x855100A
    // IDA_NAME: _ZN15CUserCharacInfo21DecCurCharacLuckPointEi
    // DEMANGLED: CUserCharacInfo::DecCurCharacLuckPoint(int)
    int DecCurCharacLuckPoint(int deduction_rate);

    // IDA_EA: 0x86467A0
    // IDA_NAME: _ZNK15CUserCharacInfo25GetCurCharacMaxEquipLevelEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacMaxEquipLevel(void)const
    int GetCurCharacMaxEquipLevel() const;

    // IDA_EA: 0x86467C2
    // IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacMaxEquipLevelEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacMaxEquipLevel(int)
    int SetCurCharacMaxEquipLevel(int level);

    // IDA_EA: 0x822F184
    // IDA_NAME: _ZN15CUserCharacInfo22setDemensionInoutValueEcc
    // DEMANGLED: CUserCharacInfo::setDemensionInoutValue(char,char)
    int setDemensionInoutValue(char index, char value);

    // IDA_EA: 0x822F1CC
    // IDA_NAME: _ZN15CUserCharacInfo21setUltimateInoutValueEcc
    // DEMANGLED: CUserCharacInfo::setUltimateInoutValue(char,char)
    int setUltimateInoutValue(char index, char value);

    // IDA_EA: 0x848ED5A
    // IDA_NAME: _ZN15CUserCharacInfo34setCurCharacEventCharacterGrowtypeEc
    // DEMANGLED: CUserCharacInfo::setCurCharacEventCharacterGrowtype(char)
    int setCurCharacEventCharacterGrowtype(char grow_type);

    // IDA_EA: 0x849AAB0
    // IDA_NAME: _ZN15CUserCharacInfo28setCurChannelEquipslotSwitchEc
    // DEMANGLED: CUserCharacInfo::setCurChannelEquipslotSwitch(char)
    int setCurChannelEquipslotSwitch(char sw);

    // IDA_EA: 0x85BFA0C
    // IDA_NAME: _ZN15CUserCharacInfo22subDemensionInoutValueEc
    // DEMANGLED: CUserCharacInfo::subDemensionInoutValue(char)
    int subDemensionInoutValue(char index);

    // IDA_EA: 0x85BFA56
    // IDA_NAME: _ZN15CUserCharacInfo21subUltimateInoutValueEc
    // DEMANGLED: CUserCharacInfo::subUltimateInoutValue(char)
    int subUltimateInoutValue(char index);

    // IDA_EA: 0x85BFA8E
    // IDA_NAME: _ZN15CUserCharacInfo23getBloodBestRoundRecordEv
    // DEMANGLED: CUserCharacInfo::getBloodBestRoundRecord(void)
    int getBloodBestRoundRecord();

    // IDA_EA: 0x85BFAA0
    // IDA_NAME: _ZN15CUserCharacInfo22getBloodBestTimeRecordEv
    // DEMANGLED: CUserCharacInfo::getBloodBestTimeRecord(void)
    int getBloodBestTimeRecord();

    // IDA_EA: 0x864E1F2
    // IDA_NAME: _ZN15CUserCharacInfo14resetSaveFlagsEv
    // DEMANGLED: CUserCharacInfo::resetSaveFlags(void)
    CUserCharacInfo* resetSaveFlags();

    // IDA_EA: 0x86961A8
    // IDA_NAME: _ZN15CUserCharacInfo24setBloodBestRecordUpdateEb
    // DEMANGLED: CUserCharacInfo::setBloodBestRecordUpdate(bool)
    int setBloodBestRecordUpdate(bool flag);

    // IDA_EA: 0x86961D0
    // IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacVill4SaveEv
    // DEMANGLED: CUserCharacInfo::getCurCharacVill4Save(void)const
    int getCurCharacVill4Save() const;

    // IDA_EA: 0x84EBFE6
    // IDA_NAME: _ZN15CUserCharacInfo25setUserEventCharacterFlagEh
    // DEMANGLED: CUserCharacInfo::setUserEventCharacterFlag(unsigned char)
    CUserCharacInfo* setUserEventCharacterFlag(uint8_t flag);

    // IDA_EA: 0x869622A
    // IDA_NAME: _ZN15CUserCharacInfo21disableSaveCharacInfoEv
    // DEMANGLED: CUserCharacInfo::disableSaveCharacInfo(void)
    CUserCharacInfo* disableSaveCharacInfo();

    // IDA_EA: 0x8696236
    // IDA_NAME: _ZN15CUserCharacInfo21disableSaveCharacStatEv
    // DEMANGLED: CUserCharacInfo::disableSaveCharacStat(void)
    CUserCharacInfo* disableSaveCharacStat();

    // IDA_EA: 0x8696242
    // IDA_NAME: _ZN15CUserCharacInfo20disableSaveCharacEtcEv
    // DEMANGLED: CUserCharacInfo::disableSaveCharacEtc(void)
    CUserCharacInfo* disableSaveCharacEtc();

    // IDA_EA: 0x869624E
    // IDA_NAME: _ZN15CUserCharacInfo27disableSaveCharacManageInfoEv
    // DEMANGLED: CUserCharacInfo::disableSaveCharacManageInfo(void)
    CUserCharacInfo* disableSaveCharacManageInfo();

    // IDA_EA: 0x869627E
    // IDA_NAME: _ZN15CUserCharacInfo16isSaveCharacInfoEv
    // DEMANGLED: CUserCharacInfo::isSaveCharacInfo(void)
    int isSaveCharacInfo();

    // IDA_EA: 0x869628A
    // IDA_NAME: _ZN15CUserCharacInfo16IsSaveCharacStatEv
    // DEMANGLED: CUserCharacInfo::IsSaveCharacStat(void)
    int IsSaveCharacStat();

    // IDA_EA: 0x8696296
    // IDA_NAME: _ZN15CUserCharacInfo15isSaveCharacEtcEv
    // DEMANGLED: CUserCharacInfo::isSaveCharacEtc(void)
    int isSaveCharacEtc();

    // IDA_EA: 0x86962A2
    // IDA_NAME: _ZN15CUserCharacInfo22IsSaveCharacManageInfoEv
    // DEMANGLED: CUserCharacInfo::IsSaveCharacManageInfo(void)
    int IsSaveCharacManageInfo();

    // IDA_EA: 0x8696694
    // IDA_NAME: _ZNK15CUserCharacInfo26get_charac_party_bonus_expEv
    // DEMANGLED: CUserCharacInfo::get_charac_party_bonus_exp(void)const
    int get_charac_party_bonus_exp() const;

    // IDA_EA: 0x8696738
    // IDA_NAME: _ZN15CUserCharacInfo17setCurCharacLevelEi
    // DEMANGLED: CUserCharacInfo::setCurCharacLevel(int)
    int setCurCharacLevel(int level);

    // IDA_EA: 0x8696762
    // IDA_NAME: _ZN15CUserCharacInfo17incCurCharacLevelEv
    // DEMANGLED: CUserCharacInfo::incCurCharacLevel(void)
    int incCurCharacLevel();

    // IDA_EA: 0x8696790
    // IDA_NAME: _ZN15CUserCharacInfo17decCurCharacLevelEv
    // DEMANGLED: CUserCharacInfo::decCurCharacLevel(void)
    int decCurCharacLevel();

    // IDA_EA: 0x86967BE
    // IDA_NAME: _ZN15CUserCharacInfo15addCurCharacExpEi
    // DEMANGLED: CUserCharacInfo::addCurCharacExp(int)
    int addCurCharacExp(int exp);

    // IDA_EA: 0x86967F0
    // IDA_NAME: _ZN15CUserCharacInfo20subtractCurCharacExpEi
    // DEMANGLED: CUserCharacInfo::subtractCurCharacExp(int)
    int subtractCurCharacExp(int exp);

    // IDA_EA: 0x84699E6
    // IDA_NAME: _ZN15CUserCharacInfo26set_charac_party_bonus_expEi
    // DEMANGLED: CUserCharacInfo::set_charac_party_bonus_exp(int)
    int set_charac_party_bonus_exp(int exp);

    // IDA_EA: 0x8469A02
    // IDA_NAME: _ZN15CUserCharacInfo32set_charac_fatigue_buf_bonus_expEi
    // DEMANGLED: CUserCharacInfo::set_charac_fatigue_buf_bonus_exp(int)
    int set_charac_fatigue_buf_bonus_exp(int exp);

    // IDA_EA: 0x8469A20
    // IDA_NAME: _ZN15CUserCharacInfo27set_charac_member_bonus_expEi
    // DEMANGLED: CUserCharacInfo::set_charac_member_bonus_exp(int)
    int set_charac_member_bonus_exp(int exp);

    // IDA_EA: 0x8469A3E
    // IDA_NAME: _ZN15CUserCharacInfo30set_charac_seria_buf_bonus_expEi
    // DEMANGLED: CUserCharacInfo::set_charac_seria_buf_bonus_exp(int)
    int set_charac_seria_buf_bonus_exp(int exp);

    // IDA_EA: 0x86962F6
    // IDA_NAME: _ZN15CUserCharacInfo18isSaveCharacOptionEv
    // DEMANGLED: CUserCharacInfo::isSaveCharacOption(void)
    int isSaveCharacOption();

    // IDA_EA: 0x8696548
    // IDA_NAME: _ZNK15CUserCharacInfo33getCurCharacDungeonHelpAbuseRatioEv
    // DEMANGLED: CUserCharacInfo::getCurCharacDungeonHelpAbuseRatio(void)const
    int getCurCharacDungeonHelpAbuseRatio() const;

    // IDA_EA: 0x869656A
    // IDA_NAME: _ZNK15CUserCharacInfo31getCurCharacDungeonHelpAbuseExpEv
    // DEMANGLED: CUserCharacInfo::getCurCharacDungeonHelpAbuseExp(void)const
    int getCurCharacDungeonHelpAbuseExp() const;

    // IDA_EA: 0x864E3BA
    // IDA_NAME: _ZN15CUserCharacInfo16get_level_up_expEi
    // DEMANGLED: CUserCharacInfo::get_level_up_exp(int)
    int get_level_up_exp(int level);

    // IDA_EA: 0x8658A4E
    // IDA_NAME: _ZN15CUserCharacInfo26calc_disp_help_abuse_ratioEi
    // DEMANGLED: CUserCharacInfo::calc_disp_help_abuse_ratio(int)
    int calc_disp_help_abuse_ratio(int level_delta);

    // IDA_EA: 0x84EC13E
    // IDA_NAME: _ZN15CUserCharacInfo27needCurCharacCheckOverEquipEv
    // DEMANGLED: CUserCharacInfo::needCurCharacCheckOverEquip(void)
    int needCurCharacCheckOverEquip();

    // IDA_EA: 0x84EC162
    // IDA_NAME: _ZN15CUserCharacInfo27resetCurCharacOverEquipFlagEv
    // DEMANGLED: CUserCharacInfo::resetCurCharacOverEquipFlag(void)
    int resetCurCharacOverEquipFlag();

    // IDA_EA: 0x850D494
    // IDA_NAME: _ZNK15CUserCharacInfo34getCurCharacHelpAbuseComputedRatioEv
    // DEMANGLED: CUserCharacInfo::getCurCharacHelpAbuseComputedRatio(void)const
    int getCurCharacHelpAbuseComputedRatio() const;

    // IDA_EA: 0x86965AC
    // IDA_NAME: _ZN15CUserCharacInfo26getCurCharacTradeGoldTotalEv
    // DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldTotal(void)
    int getCurCharacTradeGoldTotal();

    // IDA_EA: 0x86965D6
    // IDA_NAME: _ZN15CUserCharacInfo33getCurCharacTradeGoldTotalBillionEv
    // DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldTotalBillion(void)
    int getCurCharacTradeGoldTotalBillion();

    // IDA_EA: 0x8696600
    // IDA_NAME: _ZN15CUserCharacInfo26getCurCharacTradeGoldDailyEv
    // DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldDaily(void)
    int getCurCharacTradeGoldDaily();

    // IDA_EA: 0x869662A
    // IDA_NAME: _ZN15CUserCharacInfo32IncrementCurCharacTradeGoldDailyEj
    // DEMANGLED: CUserCharacInfo::IncrementCurCharacTradeGoldDaily(unsigned int)
    CReliablePerson* IncrementCurCharacTradeGoldDaily(uint32_t gold);

    // IDA_EA: 0x8696656
    // IDA_NAME: _ZN15CUserCharacInfo21GetOverTradeGoldCountEv
    // DEMANGLED: CUserCharacInfo::GetOverTradeGoldCount(void)
    int GetOverTradeGoldCount();

    // IDA_EA: 0x8696822
    // IDA_NAME: _ZNK15CUserCharacInfo21getRequestRenameStateEv
    // DEMANGLED: CUserCharacInfo::getRequestRenameState(void)const
    int getRequestRenameState() const;

    // IDA_EA: 0x8696844
    // IDA_NAME: _ZN15CUserCharacInfo19get_guild_today_expEv
    // DEMANGLED: CUserCharacInfo::get_guild_today_exp(void)
    int get_guild_today_exp();

    // IDA_EA: 0x8645CE4
    // IDA_NAME: _ZN15CUserCharacInfo19add_guild_today_expEi
    // DEMANGLED: CUserCharacInfo::add_guild_today_exp(int)
    int add_guild_today_exp(int exp);

    // IDA_EA: 0x8645D22
    // IDA_NAME: _ZN15CUserCharacInfo21reset_guild_today_expEv
    // DEMANGLED: CUserCharacInfo::reset_guild_today_exp(void)
    int reset_guild_today_exp();

    // IDA_EA: 0x8645B80
    // IDA_NAME: _ZN15CUserCharacInfo25set_uppermember_exp_levelEh
    // DEMANGLED: CUserCharacInfo::set_uppermember_exp_level(unsigned char)
    int set_uppermember_exp_level(uint8_t level);

    // IDA_EA: 0x8696866
    // IDA_NAME: _ZN15CUserCharacInfo25get_uppermember_exp_levelEv
    // DEMANGLED: CUserCharacInfo::get_uppermember_exp_level(void)
    int get_uppermember_exp_level();

    // IDA_EA: 0x869688A
    // IDA_NAME: _ZNK15CUserCharacInfo32get_charac_fatigue_buf_bonus_expEv
    // DEMANGLED: CUserCharacInfo::get_charac_fatigue_buf_bonus_exp(void)const
    int get_charac_fatigue_buf_bonus_exp() const;

    // IDA_EA: 0x86968AC
    // IDA_NAME: _ZNK15CUserCharacInfo27get_charac_member_bonus_expEv
    // DEMANGLED: CUserCharacInfo::get_charac_member_bonus_exp(void)const
    int get_charac_member_bonus_exp() const;

    // IDA_EA: 0x86968CE
    // IDA_NAME: _ZN15CUserCharacInfo33add_member_pay_tex_money_to_upperEi
    // DEMANGLED: CUserCharacInfo::add_member_pay_tex_money_to_upper(int)
    int add_member_pay_tex_money_to_upper(int value);

    // IDA_EA: 0x86968F8
    // IDA_NAME: _ZN15CUserCharacInfo35add_member_pay_tex_fatigue_to_upperEi
    // DEMANGLED: CUserCharacInfo::add_member_pay_tex_fatigue_to_upper(int)
    int add_member_pay_tex_fatigue_to_upper(int value);

    // IDA_EA: 0x8696922
    // IDA_NAME: _ZN15CUserCharacInfo22GetCharacStartPlayTickEv
    // DEMANGLED: CUserCharacInfo::GetCharacStartPlayTick(void)
    int GetCharacStartPlayTick();

    // IDA_EA: 0x8645538
    // IDA_NAME: _ZN15CUserCharacInfo17GetCharacPlayTickEv
    // DEMANGLED: CUserCharacInfo::GetCharacPlayTick(void)
    int GetCharacPlayTick();

    // IDA_EA: 0x868EF20
    // IDA_NAME: _ZN15CUserCharacInfo24GetCurCharacAssaultMoneyEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacAssaultMoney(void)
    int GetCurCharacAssaultMoney();

    // IDA_EA: 0x868EF60
    // IDA_NAME: _ZN15CUserCharacInfo27GetCurCharacAssaultedUserIdEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacAssaultedUserId(void)
    int GetCurCharacAssaultedUserId();

    // IDA_EA: 0x868EED2
    // IDA_NAME: _ZN15CUserCharacInfo27GetCurCharacChaosResponTimeEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacChaosResponTime(void)
    int GetCurCharacChaosResponTime();

    // IDA_EA: 0x85BFCCA
    // IDA_NAME: _ZN15CUserCharacInfo16GetExpAffectRateEv
    // DEMANGLED: CUserCharacInfo::GetExpAffectRate(void)
    float GetExpAffectRate();

    // IDA_EA: 0x86D200E
    // IDA_NAME: _ZN15CUserCharacInfo23GetPowerWarLastPlayTimeEv
    // DEMANGLED: CUserCharacInfo::GetPowerWarLastPlayTime(void)
    int GetPowerWarLastPlayTime();

    // IDA_EA: 0x86D2030
    // IDA_NAME: _ZN15CUserCharacInfo36GetPowerWarProcessInfoLastUpdateTimeEv
    // DEMANGLED: CUserCharacInfo::GetPowerWarProcessInfoLastUpdateTime(void)
    int GetPowerWarProcessInfoLastUpdateTime();

    // IDA_EA: 0x859503C
    // IDA_NAME: _ZN15CUserCharacInfo23IsCurCharacLeaveTimeoutEv
    // DEMANGLED: CUserCharacInfo::IsCurCharacLeaveTimeout(void)
    int IsCurCharacLeaveTimeout();

    // IDA_EA: 0x85BFCF6
    // IDA_NAME: _ZN15CUserCharacInfo17IsExpItemAffectedEv
    // DEMANGLED: CUserCharacInfo::IsExpItemAffected(void)
    int IsExpItemAffected();

    // IDA_EA: 0x8659CF0
    // IDA_NAME: _ZN15CUserCharacInfo19EnablePeriodExpItemEv
    // DEMANGLED: CUserCharacInfo::EnablePeriodExpItem(void)
    int EnablePeriodExpItem();

    // IDA_EA: 0x86966B4
    // IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacCoinEv
    // DEMANGLED: CUserCharacInfo::getCurCharacCoin(void)const
    int getCurCharacCoin() const;

    // IDA_EA: 0x869670C
    // IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacEventCoinEv
    // DEMANGLED: CUserCharacInfo::getCurCharacEventCoin(void)const
    int getCurCharacEventCoin() const;

    // IDA_EA: 0x82A66AA
    // IDA_NAME: _ZNK15CUserCharacInfo24getCurCharacLastPlayTickEv
    // DEMANGLED: CUserCharacInfo::getCurCharacLastPlayTick(void)const
    int getCurCharacLastPlayTick() const;

    // IDA_EA: 0x86966E0
    // IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacPayCoinEv
    // DEMANGLED: CUserCharacInfo::getCurCharacPayCoin(void)const
    int getCurCharacPayCoin() const;

    // IDA_EA: 0x8696018
    // IDA_NAME: _ZNK15CUserCharacInfo29getCurCharacStrikerSkillIndexEv
    // DEMANGLED: CUserCharacInfo::getCurCharacStrikerSkillIndex(void)const
    int getCurCharacStrikerSkillIndex() const;

    // IDA_EA: 0x869603C
    // IDA_NAME: _ZNK15CUserCharacInfo23getCurCharacTagCharacNoEv
    // DEMANGLED: CUserCharacInfo::getCurCharacTagCharacNo(void)const
    int getCurCharacTagCharacNo() const;

    // IDA_EA: 0x85BFB44
    // IDA_NAME: _ZN15CUserCharacInfo24getCurCharacTutorialFlagEv
    // DEMANGLED: CUserCharacInfo::getCurCharacTutorialFlag(void)
    int getCurCharacTutorialFlag();

    // IDA_EA: 0x8696C10
    // IDA_NAME: _ZN15CUserCharacInfo19getCurEquipslotFlagEv
    // DEMANGLED: CUserCharacInfo::getCurEquipslotFlag(void)
    int getCurEquipslotFlag();

    // IDA_EA: 0x8696ADA
    // IDA_NAME: _ZN15CUserCharacInfo16getDisguiseIndexEv
    // DEMANGLED: CUserCharacInfo::getDisguiseIndex(void)
    int getDisguiseIndex();

    // IDA_EA: 0x8696AB6
    // IDA_NAME: _ZN15CUserCharacInfo15getDisguiseKindEv
    // DEMANGLED: CUserCharacInfo::getDisguiseKind(void)
    int getDisguiseKind();

    // IDA_EA: 0x868E262
    // IDA_NAME: _ZNK15CUserCharacInfo18getPremiumBonusExpEv
    // DEMANGLED: CUserCharacInfo::getPremiumBonusExp(void)const
    int getPremiumBonusExp() const;

    // IDA_EA: 0x868E328
    // IDA_NAME: _ZN15CUserCharacInfo18getWeekendBonusExpEv
    // DEMANGLED: CUserCharacInfo::getWeekendBonusExp(void)
    int getWeekendBonusExp();

    // IDA_EA: 0x8696C68
    // IDA_NAME: _ZNK15CUserCharacInfo30get_charac_seria_buf_bonus_expEv
    // DEMANGLED: CUserCharacInfo::get_charac_seria_buf_bonus_exp(void)const
    int get_charac_seria_buf_bonus_exp() const;

    // IDA_EA: 0x868BEB4
    // IDA_NAME: _ZN15CUserCharacInfo30get_charac_visible_grow_avatarEv
    // DEMANGLED: CUserCharacInfo::get_charac_visible_grow_avatar(void)
    int get_charac_visible_grow_avatar();

    // IDA_EA: 0x8696D18
    // IDA_NAME: _ZNK15CUserCharacInfo22isSavedPremiumCubeInfoEv
    // DEMANGLED: CUserCharacInfo::isSavedPremiumCubeInfo(void)const
    int isSavedPremiumCubeInfo() const;

    // IDA_EA: 0x8696CA4
    // IDA_NAME: _ZNK15CUserCharacInfo32is_growth_power_exp_reward_rightEv
    // DEMANGLED: CUserCharacInfo::is_growth_power_exp_reward_right(void)const
    int is_growth_power_exp_reward_right() const;

    // IDA_EA: 0x8696A72
    // IDA_NAME: _ZNK15CUserCharacInfo23GetCurCharacBackupSkillEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacBackupSkill(void)const
    SkillSlot* GetCurCharacBackupSkill() const;

    // IDA_EA: 0x8696A94
    // IDA_NAME: _ZNK15CUserCharacInfo26GetCurCharacBackupSkill2NDEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacBackupSkill2ND(void)const
    SkillSlot* GetCurCharacBackupSkill2ND() const;

    // IDA_EA: 0x8696A24
    // IDA_NAME: _ZN15CUserCharacInfo28GetCurCharacCoolTimeItemListEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacCoolTimeItemList(void)
    std::vector<ContinuousItemInfo>* GetCurCharacCoolTimeItemList();

    // IDA_EA: 0x8696A34
    // IDA_NAME: _ZNK15CUserCharacInfo29GetCurCharacCoolTimeItemListREv
    // DEMANGLED: CUserCharacInfo::GetCurCharacCoolTimeItemListR(void)const
    std::vector<ContinuousItemInfo>* GetCurCharacCoolTimeItemListR() const;

    // IDA_EA: 0x8696A44
    // IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacEffectItemListEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacEffectItemList(void)
    std::vector<ContinuousItemInfo>* GetCurCharacEffectItemList();

    // IDA_EA: 0x85D19FA
    // IDA_NAME: _ZNK15CUserCharacInfo27GetCurCharacEffectItemListREv
    // DEMANGLED: CUserCharacInfo::GetCurCharacEffectItemListR(void)const
    std::vector<ContinuousItemInfo>* GetCurCharacEffectItemListR() const;

    // IDA_EA: 0x8659E3E
    // IDA_NAME: _ZN15CUserCharacInfo24ClearUselessCooltimeItemEj
    // DEMANGLED: CUserCharacInfo::ClearUselessCooltimeItem(unsigned int)
    int ClearUselessCooltimeItem(unsigned int current_sec);

    // IDA_EA: 0x8659EF4
    // IDA_NAME: _ZN15CUserCharacInfo22ClearUselessEffectItemEj
    // DEMANGLED: CUserCharacInfo::ClearUselessEffectItem(unsigned int)
    int ClearUselessEffectItem(unsigned int current_sec);

    // IDA_EA: 0x8677F24
    // IDA_NAME: _ZNK15CUserCharacInfo21GetContinuousItemInfoEP12SIG_LOAD_ETC
    // DEMANGLED: CUserCharacInfo::GetContinuousItemInfo(SIG_LOAD_ETC *)const
    bool GetContinuousItemInfo(SIG_LOAD_ETC* payload) const;

    // IDA_EA: 0x86B4BE6
    // IDA_NAME: _ZN15CUserCharacInfo21IncVillageAttackCountEv
    // DEMANGLED: CUserCharacInfo::IncVillageAttackCount(void)
    int IncVillageAttackCount();

    // IDA_EA: 0x8646496
    // IDA_NAME: _ZN15CUserCharacInfo34IsAvailableCurCharacTradeGoldDailyEj
    // DEMANGLED: CUserCharacInfo::IsAvailableCurCharacTradeGoldDaily(unsigned int)
    bool IsAvailableCurCharacTradeGoldDaily(unsigned int add_gold);

    // IDA_EA: 0x86969FE
    // IDA_NAME: _ZN15CUserCharacInfo30ResetCurCharacDungeonPlayCountEv
    // DEMANGLED: CUserCharacInfo::ResetCurCharacDungeonPlayCount(void)
    int ResetCurCharacDungeonPlayCount();

    // IDA_EA: 0x86D2052
    // IDA_NAME: _ZN15CUserCharacInfo24ResetRevengeDungeonClearEv
    // DEMANGLED: CUserCharacInfo::ResetRevengeDungeonClear(void)
    int ResetRevengeDungeonClear();

    // IDA_EA: 0x868EF42
    // IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacAssaultMoneyEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacAssaultMoney(int)
    int SetCurCharacAssaultMoney(int money);

    // IDA_EA: 0x868EF84
    // IDA_NAME: _ZN15CUserCharacInfo27SetCurCharacAssaultedUserIdEt
    // DEMANGLED: CUserCharacInfo::SetCurCharacAssaultedUserId(unsigned short)
    int SetCurCharacAssaultedUserId(uint16_t unique_id);

    // IDA_EA: 0x82F0DA8
    // IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacChaosKillTimeEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosKillTime(int)
    int SetCurCharacChaosKillTime(int time);

    // IDA_EA: 0x868EEF4
    // IDA_NAME: _ZN15CUserCharacInfo27SetCurCharacChaosResponTimeEi
    // DEMANGLED: CUserCharacInfo::SetCurCharacChaosResponTime(int)
    int SetCurCharacChaosResponTime(int time);

    // IDA_EA: 0x864628E
    // IDA_NAME: _ZN15CUserCharacInfo12DieCurCharacEiP5CUser
    // DEMANGLED: CUserCharacInfo::DieCurCharac(int,CUser *)
    int DieCurCharac(int reason, CUser* attacker);

    // IDA_EA: 0x863BE1C
    // IDA_NAME: _ZN15CUserCharacInfo22SetExpAffectItemEffectEbf
    // DEMANGLED: CUserCharacInfo::SetExpAffectItemEffect(bool,float)
    int SetExpAffectItemEffect(bool affected, float rate);

    // IDA_EA: 0x868BDAC
    // IDA_NAME: _ZN15CUserCharacInfo12SetInvisibleEv
    // DEMANGLED: CUserCharacInfo::SetInvisible(void)
    int SetInvisible();

    // IDA_EA: 0x8696A54
    // IDA_NAME: _ZN15CUserCharacInfo22SetIsAffectedExpDoubleEb
    // DEMANGLED: CUserCharacInfo::SetIsAffectedExpDouble(bool)
    int SetIsAffectedExpDouble(bool affected);

    // IDA_EA: 0x86B087C
    // IDA_NAME: _ZN15CUserCharacInfo20SetIsEndLetheAtLoginEv
    // DEMANGLED: CUserCharacInfo::SetIsEndLetheAtLogin(void)
    int SetIsEndLetheAtLogin();

    // IDA_EA: 0x84EC182
    // IDA_NAME: _ZN15CUserCharacInfo36SetPowerWarProcessInfoLastUpdateTimeEl
    // DEMANGLED: CUserCharacInfo::SetPowerWarProcessInfoLastUpdateTime(long)
    int SetPowerWarProcessInfoLastUpdateTime(long current_sec);

    // IDA_EA: 0x868BDC8
    // IDA_NAME: _ZN15CUserCharacInfo10SetVisibleEv
    // DEMANGLED: CUserCharacInfo::SetVisible(void)
    int SetVisible();

    // IDA_EA: 0x864E402
    // IDA_NAME: _ZN15CUserCharacInfo18calcHelpAbuseRatioEjb
    // DEMANGLED: CUserCharacInfo::calcHelpAbuseRatio(unsigned int,bool)
    int calcHelpAbuseRatio(unsigned int ratio, bool flag);

    // IDA_EA: 0x864E3FA
    // IDA_NAME: _ZN15CUserCharacInfo31calcHelpAbuseRatioBeforeLevelUpEi
    // DEMANGLED: CUserCharacInfo::calcHelpAbuseRatioBeforeLevelUp(int)
    void calcHelpAbuseRatioBeforeLevelUp(int level);

    // IDA_EA: 0x8696D3C
    // IDA_NAME: _ZN15CUserCharacInfo27disableSavedPremiumCubeInfoEv
    // DEMANGLED: CUserCharacInfo::disableSavedPremiumCubeInfo(void)
    int disableSavedPremiumCubeInfo();

    // IDA_EA: 0x8696272
    // IDA_NAME: _ZN15CUserCharacInfo22enableSaveBattleRecordEv
    // DEMANGLED: CUserCharacInfo::enableSaveBattleRecord(void)
    CUserCharacInfo* enableSaveBattleRecord();

    // IDA_EA: 0x8696200
    // IDA_NAME: _ZNK15CUserCharacInfo19getBattleRecordRefREv
    // DEMANGLED: CUserCharacInfo::getBattleRecordRefR(void)const
    std::map<int, GameResultType>* getBattleRecordRefR() const;

    // IDA_EA: 0x869620E
    // IDA_NAME: _ZN15CUserCharacInfo19getBattleRecordRefWEv
    // DEMANGLED: CUserCharacInfo::getBattleRecordRefW(void)
    std::map<int, GameResultType>* getBattleRecordRefW();

    // IDA_EA: 0x8696CC8
    // IDA_NAME: _ZN15CUserCharacInfo28getContractOfCubePremiumInfoERhS0_
    // DEMANGLED: CUserCharacInfo::getContractOfCubePremiumInfo(unsigned char &,unsigned char &)
    int getContractOfCubePremiumInfo(uint8_t& enabled,
                                     uint8_t& premium_type);

    // IDA_EA: 0x84B42BC
    // IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacAddInfoREv
    // DEMANGLED: CUserCharacInfo::getCurCharacAddInfoR(void)const
    _Additioal_info* getCurCharacAddInfoR() const;

    // IDA_EA: 0x86960AA
    // IDA_NAME: _ZNK15CUserCharacInfo23getCurCharacAddInfoRefREv
    // DEMANGLED: CUserCharacInfo::getCurCharacAddInfoRefR(void)const
    _Additioal_info* getCurCharacAddInfoRefR() const;

    // IDA_EA: 0x86960D8
    // IDA_NAME: _ZN15CUserCharacInfo23getCurCharacAddInfoRefWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacAddInfoRefW(void)
    _Additioal_info* getCurCharacAddInfoRefW();

    // IDA_EA: 0x86960BA
    // IDA_NAME: _ZN15CUserCharacInfo20getCurCharacAddInfoWEv
    // DEMANGLED: CUserCharacInfo::getCurCharacAddInfoW(void)
    _Additioal_info* getCurCharacAddInfoW();

    // IDA_EA: 0x864751E
    // IDA_NAME: _ZN15CUserCharacInfo32getCurCharacEscaladeTutorialFlagEj
    // DEMANGLED: CUserCharacInfo::getCurCharacEscaladeTutorialFlag(unsigned int)
    bool getCurCharacEscaladeTutorialFlag(unsigned int flag);

    // IDA_EA: 0x869658C
    // IDA_NAME: _ZN15CUserCharacInfo32getCurCharacEscaladeTutorialFlagEv
    // DEMANGLED: CUserCharacInfo::getCurCharacEscaladeTutorialFlag(void)
    const uint8_t* getCurCharacEscaladeTutorialFlag();

    // IDA_EA: 0x868EE50
    // IDA_NAME: _ZN15CUserCharacInfo16getExpBonusPointER19ExpBonusPointResult
    // DEMANGLED: CUserCharacInfo::getExpBonusPoint(ExpBonusPointResult &)
    int getExpBonusPoint(ExpBonusPointResult& result);

    // IDA_EA: 0x822FB96
    // IDA_NAME: _ZN15CUserCharacInfo23getSeriaRoomAniDecoInfoEv
    // DEMANGLED: CUserCharacInfo::getSeriaRoomAniDecoInfo(void)
    seriaRoom_AniDeco getSeriaRoomAniDecoInfo();

    // IDA_EA: 0x8696BEE
    // IDA_NAME: _ZNK15CUserCharacInfo15getSkillCommandEv
    // DEMANGLED: CUserCharacInfo::getSkillCommand(void)const
    std::map<int, STSkillCommandData>* getSkillCommand() const;

    // IDA_EA: 0x869607A
    // IDA_NAME: _ZNK15CUserCharacInfo20getTagCharacAddInfoREv
    // DEMANGLED: CUserCharacInfo::getTagCharacAddInfoR(void)const
    _Additioal_info* getTagCharacAddInfoR() const;

    // IDA_EA: 0x869609A
    // IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacCargoREv
    // DEMANGLED: CUserCharacInfo::getTagCharacCargoR(void)const
    CCargo* getTagCharacCargoR() const;

    // IDA_EA: 0x869608A
    // IDA_NAME: _ZN15CUserCharacInfo18getTagCharacCargoWEv
    // DEMANGLED: CUserCharacInfo::getTagCharacCargoW(void)
    CCargo* getTagCharacCargoW();

    // IDA_EA: 0x869606A
    // IDA_NAME: _ZN15CUserCharacInfo18getTagCharacInvenWEv
    // DEMANGLED: CUserCharacInfo::getTagCharacInvenW(void)
    CInventory* getTagCharacInvenW();

    // IDA_EA: 0x869605E
    // IDA_NAME: _ZN15CUserCharacInfo13getTagCharacWEv
    // DEMANGLED: CUserCharacInfo::getTagCharacW(void)
    _Charac_info* getTagCharacW();

    // IDA_EA: 0x868EE26
    // IDA_NAME: _ZN15CUserCharacInfo15getWpBonusPointER18WpBonusPointResult
    // DEMANGLED: CUserCharacInfo::getWpBonusPoint(WpBonusPointResult &)
    int getWpBonusPoint(WpBonusPointResult& result);

    // IDA_EA: 0x868EE7A
    // IDA_NAME: _ZN15CUserCharacInfo15resetBonusPointEv
    // DEMANGLED: CUserCharacInfo::resetBonusPoint(void)
    void* resetBonusPoint();

    // IDA_EA: 0x868EC5C
    // IDA_NAME: _ZN15CUserCharacInfo15checkBonusPointEv
    // DEMANGLED: CUserCharacInfo::checkBonusPoint(void)
    char checkBonusPoint();

    // IDA_EA: 0x864739A
    // IDA_NAME: _ZN15CUserCharacInfo23resetCharacTutorialFlagEv
    // DEMANGLED: CUserCharacInfo::resetCharacTutorialFlag(void)
    void* resetCharacTutorialFlag();

    // IDA_EA: 0x868E284
    // IDA_NAME: _ZN15CUserCharacInfo20resetPremiumBonusExpEv
    // DEMANGLED: CUserCharacInfo::resetPremiumBonusExp(void)
    int resetPremiumBonusExp();

    // IDA_EA: 0x868E34A
    // IDA_NAME: _ZN15CUserCharacInfo20resetWeekendBonusExpEv
    // DEMANGLED: CUserCharacInfo::resetWeekendBonusExp(void)
    int resetWeekendBonusExp();

    // IDA_EA: 0x8696B90
    // IDA_NAME: _ZN15CUserCharacInfo21setCharacLinkBonusExpEj
    // DEMANGLED: CUserCharacInfo::setCharacLinkBonusExp(unsigned int)
    int setCharacLinkBonusExp(unsigned int exp);

    // IDA_EA: 0x8696BD0
    // IDA_NAME: _ZN15CUserCharacInfo22setCharacLinkBonusGoldEj
    // DEMANGLED: CUserCharacInfo::setCharacLinkBonusGold(unsigned int)
    int setCharacLinkBonusGold(unsigned int gold);

    // IDA_EA: 0x84EC4F2
    // IDA_NAME: _ZN15CUserCharacInfo15setCharacOptionER15CharacterOption
    // DEMANGLED: CUserCharacInfo::setCharacOption(CharacterOption &)
    CharacterOption* setCharacOption(CharacterOption& option);

    // IDA_EA: 0x864E34E
    // IDA_NAME: _ZN15CUserCharacInfo28setCurCharChangingGrowthTypeEcc
    // DEMANGLED: CUserCharacInfo::setCurCharChangingGrowthType(char,char)
    int setCurCharChangingGrowthType(char growth, char second);

    // IDA_EA: 0x864E2D8
    // IDA_NAME: _ZN15CUserCharacInfo20setCurCharGrowthTypeEcc
    // DEMANGLED: CUserCharacInfo::setCurCharGrowthType(char,char)
    int setCurCharGrowthType(char growth, char second);

    // IDA_EA: 0x86960F6
    // IDA_NAME: _ZN15CUserCharacInfo24setCurCharacLastPlayTickEl
    // DEMANGLED: CUserCharacInfo::setCurCharacLastPlayTick(long)
    int setCurCharacLastPlayTick(long tick);

    // IDA_EA: 0x86473E6
    // IDA_NAME: _ZN15CUserCharacInfo24setCurCharacTutorialFlagEj
    // DEMANGLED: CUserCharacInfo::setCurCharacTutorialFlag(unsigned int)
    int setCurCharacTutorialFlag(unsigned int flag);

    // IDA_EA: 0x8696C34
    // IDA_NAME: _ZN15CUserCharacInfo19setCurEquipslotFlagEc
    // DEMANGLED: CUserCharacInfo::setCurEquipslotFlag(char)
    int setCurEquipslotFlag(char flag);

    // IDA_EA: 0x868E1F6
    // IDA_NAME: _ZN15CUserCharacInfo18setPremiumBonusExpEji
    // DEMANGLED: CUserCharacInfo::setPremiumBonusExp(unsigned int,int)
    int setPremiumBonusExp(unsigned int reward_exp, int bonus_rate);

    // IDA_EA: 0x822FB78
    // IDA_NAME: _ZN15CUserCharacInfo23setSeriaRoomAniDecoInfoE17seriaRoom_AniDeco
    // DEMANGLED: CUserCharacInfo::setSeriaRoomAniDecoInfo(seriaRoom_AniDeco)
    int setSeriaRoomAniDecoInfo(seriaRoom_AniDeco deco_info);

    // IDA_EA: 0x868E2D2
    // IDA_NAME: _ZN15CUserCharacInfo18setWeekendBonusExpEji
    // DEMANGLED: CUserCharacInfo::setWeekendBonusExp(unsigned int,int)
    int setWeekendBonusExp(unsigned int reward_exp, int bonus_rate);

    // IDA_EA: 0x868E2A4
    // IDA_NAME: _ZN15CUserCharacInfo22setWeekendBonusFatigueEt
    // DEMANGLED: CUserCharacInfo::setWeekendBonusFatigue(unsigned short)
    int setWeekendBonusFatigue(uint16_t fatigue);

    // IDA_EA: 0x84EC4C0
    // IDA_NAME: _ZN15CUserCharacInfo22set_aura_avatar_optionER18stAuraAvatarOption
    // DEMANGLED: CUserCharacInfo::set_aura_avatar_option(stAuraAvatarOption &)
    int set_aura_avatar_option(stAuraAvatarOption& option);

    // IDA_EA: 0x8284490
    // IDA_NAME: _ZN15CUserCharacInfo28set_aura_avatar_option_valueEii
    // DEMANGLED: CUserCharacInfo::set_aura_avatar_option_value(int,int)
    int set_aura_avatar_option_value(int index, int value);

    // IDA_EA: 0x868BE30
    // IDA_NAME: _ZN15CUserCharacInfo30set_charac_visible_grow_avatarEb
    // DEMANGLED: CUserCharacInfo::set_charac_visible_grow_avatar(bool)
    int set_charac_visible_grow_avatar(bool visible);

    // IDA_EA: 0x868BE70
    // IDA_NAME: _ZN15CUserCharacInfo28set_charac_visible_teleport_Eb
    // DEMANGLED: CUserCharacInfo::set_charac_visible_teleport_(bool)
    int set_charac_visible_teleport_(bool visible);

    // IDA_EA: 0x868BE08
    // IDA_NAME: _ZN15CUserCharacInfo25set_charac_visible_valuesEc
    // DEMANGLED: CUserCharacInfo::set_charac_visible_values(char)
    int set_charac_visible_values(char values);

    // IDA_EA: 0x864E29C
    // IDA_NAME: _ZN15CUserCharacInfo11useWinPointEi12eWPSubReason
    // DEMANGLED: CUserCharacInfo::useWinPoint(int,eWPSubReason)
    CUserCharacInfo* useWinPoint(int point, eWPSubReason reason);

    // IDA_EA: 0x822F21C
    // IDA_NAME: _ZN15CUserCharacInfo21set_charac_createtimeEl
    // DEMANGLED: CUserCharacInfo::set_charac_createtime(long)
    CommonTime* set_charac_createtime(long timer);

    // IDA_EA: 0x86456D0
    // IDA_NAME: _ZN15CUserCharacInfo24GetWarAreaKillCountTotalEv
    // DEMANGLED: CUserCharacInfo::GetWarAreaKillCountTotal(void)
    float GetWarAreaKillCountTotal();

    // IDA_EA: 0x8645866
    // IDA_NAME: _ZN15CUserCharacInfo19GetWarAreaKillCountEi
    // DEMANGLED: CUserCharacInfo::GetWarAreaKillCount(int)
    float GetWarAreaKillCount(int area);

    // IDA_EA: 0x8645E20
    // IDA_NAME: _ZNK15CUserCharacInfo37GetCurCharacOnlinePreliminaryTeamNameEv
    // DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamName(void)const
    char* GetCurCharacOnlinePreliminaryTeamName() const;

    // IDA_EA: 0x8645E5E
    // IDA_NAME: _ZN15CUserCharacInfo21GainCurCharacChaosExpEi
    // DEMANGLED: CUserCharacInfo::GainCurCharacChaosExp(int)
    int GainCurCharacChaosExp(int experience);

    // IDA_EA: 0x864607E
    // IDA_NAME: _ZN15CUserCharacInfo21getSeparatedTextUnitsERSt6vectorISsSaISsEEPKcS5_i
    // DEMANGLED: CUserCharacInfo::getSeparatedTextUnits(std::vector<std::string,std::allocator<std::string>> &,char const*,char const*,int)
    int getSeparatedTextUnits(std::vector<std::string>& units,
                              const char* source,
                              const char* delimiter,
                              int maximum);

    // IDA_EA: 0x8659BEC
    // IDA_NAME: _ZN15CUserCharacInfo21EnablePeriodExpDoubleEv
    // DEMANGLED: CUserCharacInfo::EnablePeriodExpDouble(void)
    int EnablePeriodExpDouble();

    // IDA_EA: 0x8678084
    // IDA_NAME: _ZN15CUserCharacInfo11isExistItemEi
    // DEMANGLED: CUserCharacInfo::isExistItem(int)
    bool isExistItem(int item_id);

private:
    // Exact CUserCharacInfo prefix recovered from C2 @0x864E15E and the
    // direct-access bodies.  Keep the two reset-only bytes explicit: they
    // are cleared by resetSaveFlags() but have no independently named old
    // server semantic yet.
    uint8_t save_charac_info_;             // +0x00
    uint8_t save_charac_stat_;             // +0x01
    uint8_t save_charac_etc_;              // +0x02
    uint8_t save_charac_manage_info_;      // +0x03
    uint8_t save_inventory_;               // +0x04
    uint8_t save_skill_;                   // +0x05
    uint8_t save_pvp_;                     // +0x06
    uint8_t save_quest_;                   // +0x07
    uint8_t reset_only_08_;                // +0x08
    uint8_t save_dungeon_clear_;           // +0x09
    uint8_t save_battle_record_;           // +0x0a
    uint8_t save_cargo_;                   // +0x0b
    uint8_t reset_only_0c_;                // +0x0c
    uint8_t pad_0d_0f_[3];
    _Charac_info* current_charac_;         // +0x10
    _Charac_info* tag_charac_;             // +0x14
    PvpResultType pvp_result_;             // +0x18, sizeof 0x84
    std::map<int, GameResultType> battle_records_;  // +0x9c
    WongWork::CDungeonClear dungeon_clear_;          // +0xb4
    bool reliable_person_;                 // +0xcc
    uint8_t pad_cd_cf_[3];
    uint32_t over_trade_gold_count_;       // +0xd0
    bool save_charac_option_;              // +0xd4
    uint8_t pad_d5_d7_[3];
    int32_t lifecycle_tick_;               // +0xd8, old i386 long
    uint8_t user_event_character_flag_;    // +0xdc
    uint8_t pad_dd_df_[3];

    friend struct CUserCharacInfoLayoutAssert;
};

struct CUserCharacInfoLayoutAssert {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
    static_assert(offsetof(CUserCharacInfo, current_charac_) == 0x10u,
                  "CUserCharacInfo current-character offset must stay 0x10");
    static_assert(offsetof(CUserCharacInfo, pvp_result_) == 0x18u,
                  "CUserCharacInfo PvpResultType offset must stay 0x18");
    static_assert(offsetof(CUserCharacInfo, battle_records_) == 0x9cu,
                  "CUserCharacInfo battle-record map offset must stay 0x9c");
    static_assert(offsetof(CUserCharacInfo, dungeon_clear_) == 0xb4u,
                  "CUserCharacInfo dungeon-clear offset must stay 0xb4");
    static_assert(sizeof(CUserCharacInfo) == 0xe0u,
                  "CUserCharacInfo IDA ABI size must stay 0xe0");
#endif
};

namespace randomOptionReason {
// The symbol proves the named ABI enum but not its enumerator table.
enum T : int {};
}

// IDA unit: 06_userinfo_stat_inventory/cUserHistoryLog.  This component is a
// real CUser subobject; business code reaches it through CUser::history_log(),
// while CUserLayoutAssert below freezes its original i386 physical offset.
class cUserHistoryLog {
public:
    // The old server exposes this named nested enum in the two transform-log
    // symbols below, but not its enumerator table.
    enum eRidableType : int {};

    // IDA_EA: 0x8695FE4
    // IDA_NAME: _ZN15cUserHistoryLogC2Ev
    // DEMANGLED: cUserHistoryLog::cUserHistoryLog(void)
    cUserHistoryLog();

    // IDA_EA: 0x8695FF2
    // IDA_NAME: _ZN15cUserHistoryLog7SetUserEP5CUser
    // DEMANGLED: cUserHistoryLog::SetUser(CUser *)
    cUserHistoryLog* SetUser(CUser* user);

    // IDA_EA: 0x86848DA
    // IDA_NAME: _ZN15cUserHistoryLog23SendMailForCeraShopGiftEjcjj
    // DEMANGLED: cUserHistoryLog::SendMailForCeraShopGift(unsigned int,char,unsigned int,unsigned int)
    int SendMailForCeraShopGift(unsigned int receiver_charac_no,
                                char gift_kind,
                                unsigned int item_id,
                                unsigned int item_count);

    // IDA_EA: 0x868593C
    // IDA_NAME: _ZN15cUserHistoryLog9CeraPointE23ENUM_CERAPOINT_LOG_TYPEj
    // DEMANGLED: cUserHistoryLog::CeraPoint(ENUM_CERAPOINT_LOG_TYPE,unsigned int)
    int CeraPoint(ENUM_CERAPOINT_LOG_TYPE log_type,
                  unsigned int amount);

    // IDA_EA: 0x8685838
    // IDA_NAME: _ZN15cUserHistoryLog11BillingFailE17ENUM_BILLING_TYPEyiiPc
    // DEMANGLED: cUserHistoryLog::BillingFail(ENUM_BILLING_TYPE,unsigned long long,int,int,char *)
    int BillingFail(ENUM_BILLING_TYPE billing_type,
                    unsigned long long transaction_id,
                    int error_code,
                    int detail_code,
                    char* detail_text);

    // IDA_EA: 0x86858A0
    // IDA_NAME: _ZN15cUserHistoryLog14BillingSuccessE17ENUM_BILLING_TYPEyi
    // DEMANGLED: cUserHistoryLog::BillingSuccess(ENUM_BILLING_TYPE,unsigned long long,int)
    int BillingSuccess(ENUM_BILLING_TYPE billing_type,
                       unsigned long long transaction_id,
                       int amount);

    // IDA_EA: 0x8686EA0
    // IDA_NAME: _ZN15cUserHistoryLog15BuyCashShopItemEjmjPKccN19CeraShopPaymentType1TEy
    // DEMANGLED: cUserHistoryLog::BuyCashShopItem(unsigned int,unsigned long,unsigned int,char const*,char,CeraShopPaymentType::T,unsigned long long)
    int BuyCashShopItem(unsigned int goods_no,
                        unsigned long item_id,
                        unsigned int item_count,
                        const char* order_id,
                        char gift,
                        CeraShopPaymentType::T payment_type,
                        unsigned long long transaction_id);

    // IDA_EA: 0x8686928
    // IDA_NAME: _ZN15cUserHistoryLog12StartRidableENS_12eRidableTypeEi
    // DEMANGLED: cUserHistoryLog::StartRidable(cUserHistoryLog::eRidableType,int)
    int StartRidable(eRidableType ridable_type, int object_id);

    // IDA_EA: 0x868695C
    // IDA_NAME: _ZN15cUserHistoryLog10EndRidableENS_12eRidableTypeEi
    // DEMANGLED: cUserHistoryLog::EndRidable(cUserHistoryLog::eRidableType,int)
    int EndRidable(eRidableType ridable_type, int object_id);

    // IDA_EA: 0x86869BC
    // IDA_NAME: _ZN15cUserHistoryLog13RedeemItemDelEii
    // DEMANGLED: cUserHistoryLog::RedeemItemDel(int,int)
    int RedeemItemDel(int item_id, int item_count);

    // IDA_EA: 0x8686990
    // IDA_NAME: _ZN15cUserHistoryLog13RedeemItemAddEii
    // DEMANGLED: cUserHistoryLog::RedeemItemAdd(int,int)
    int RedeemItemAdd(int item_id, int item_count);

    // IDA_EA: 0x8696000
    // IDA_NAME: _ZN15cUserHistoryLog11getItemXPosEv
    // DEMANGLED: cUserHistoryLog::getItemXPos(void)
    int getItemXPos();

    // IDA_EA: 0x869600C
    // IDA_NAME: _ZN15cUserHistoryLog11getItemYPosEv
    // DEMANGLED: cUserHistoryLog::getItemYPos(void)
    int getItemYPos();

    // IDA_EA: 0x868479C
    // IDA_NAME: _ZN15cUserHistoryLog11SecuServiceE22ENUM_SECU_SERVICE_TYPE24ENUM_SECU_SERVICE_ACTION
    // DEMANGLED: cUserHistoryLog::SecuService(ENUM_SECU_SERVICE_TYPE,ENUM_SECU_SERVICE_ACTION)
    // All recovered callers use this as a statement. The raw decompiler's
    // inferred int result is not an evidenced return ABI.
    void SecuService(ENUM_SECU_SERVICE_TYPE service_type,
                     ENUM_SECU_SERVICE_ACTION action);

    // IDA_EA: 0x8686DD0
    // IDA_NAME: _ZN15cUserHistoryLog16AntibotPacketLogEN20AntibotPacketLogType1TE
    // DEMANGLED: cUserHistoryLog::AntibotPacketLog(AntibotPacketLogType::T)
    int AntibotPacketLog(AntibotPacketLogType::T event_type);

    // IDA_EA: 0x8686E3A
    // IDA_NAME: _ZN15cUserHistoryLog21SendBuffertoClientLogEishb
    // DEMANGLED: cUserHistoryLog::SendBuffertoClientLog(int,short,unsigned char,bool)
    int SendBuffertoClientLog(int destination_kind,
                              int16_t buffer_size,
                              uint8_t attempt_count,
                              bool send_result);

    // IDA_EA: 0x8686DA0
    // IDA_NAME: _ZN15cUserHistoryLog27SelectCubeForPremiumServiceEi
    // DEMANGLED: cUserHistoryLog::SelectCubeForPremiumService(int)
    int SelectCubeForPremiumService(int premium_type);

    // IDA_EA: 0x8686DF4
    // IDA_NAME: _ZN15cUserHistoryLog12AbnormalUserE27ENUM_ABNORMAL_USER_LOG_TYPEtii
    // DEMANGLED: cUserHistoryLog::AbnormalUser(ENUM_ABNORMAL_USER_LOG_TYPE,unsigned short,int,int)
    int AbnormalUser(ENUM_ABNORMAL_USER_LOG_TYPE log_type,
                     uint16_t command,
                     int value,
                     int detail);

    // IDA_EA: 0x8685320
    // IDA_NAME: _ZN15cUserHistoryLog12HackDetectedEjjjjjj
    // DEMANGLED: cUserHistoryLog::HackDetected(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int)
    int HackDetected(unsigned int hack_type,
                     unsigned int count,
                     unsigned int server_count,
                     unsigned int accumulated_count,
                     unsigned int value1,
                     unsigned int value2);

    // IDA_EA: 0x8685D0C
    // IDA_NAME: _ZN15cUserHistoryLog12IPCounterLogEiiiiiPc
    // DEMANGLED: cUserHistoryLog::IPCounterLog(int,int,int,int,int,char *)
    int IPCounterLog(int passed,
                     int hack_type,
                     int hack_sub_type,
                     int required_count,
                     int current_count,
                     char* peer_ip);

    // IDA_EA: 0x8685D54
    // IDA_NAME: _ZN15cUserHistoryLog16FullIPCounterLogEiiiiiPc
    // DEMANGLED: cUserHistoryLog::FullIPCounterLog(int,int,int,int,int,char *)
    int FullIPCounterLog(int passed,
                         int hack_type,
                         int hack_sub_type,
                         int required_count,
                         int current_count,
                         char* peer_ip);

    // IDA_EA: 0x86853E6
    // IDA_NAME: _ZN15cUserHistoryLog15RequestCleanPadEjc
    // DEMANGLED: cUserHistoryLog::RequestCleanPad(unsigned int,char)
    int RequestCleanPad(unsigned int point, char request_type);

    // IDA_EA: 0x8685D9C
    // IDA_NAME: _ZN15cUserHistoryLog21IPCounterLog_InsertIpEiiPc
    // DEMANGLED: cUserHistoryLog::IPCounterLog_InsertIp(int,int,char *)
    int IPCounterLog_InsertIp(int hack_type,
                              int hack_sub_type,
                              char* peer_ip);

    // IDA_EA: 0x868646C
    // IDA_NAME: _ZN15cUserHistoryLog9GiveupPvPE23ENUM_PVP_HISTORY_REASON
    // DEMANGLED: cUserHistoryLog::GiveupPvP(ENUM_PVP_HISTORY_REASON)
    int GiveupPvP(ENUM_PVP_HISTORY_REASON reason);

    // IDA_EA: 0x86863B6
    // IDA_NAME: _ZN15cUserHistoryLog8StartPvPE23ENUM_PVP_HISTORY_REASONPKcS2_S2_S2_S2_S2_S2_S2_
    // DEMANGLED: cUserHistoryLog::StartPvP(ENUM_PVP_HISTORY_REASON,char const*,...)
    int StartPvP(ENUM_PVP_HISTORY_REASON reason,
                 const char* team0_name0,
                 const char* team0_name1,
                 const char* team0_name2,
                 const char* team0_name3,
                 const char* team1_name0,
                 const char* team1_name1,
                 const char* team1_name2,
                 const char* team1_name3);

    // IDA_EA: 0x868642E
    // IDA_NAME: _ZN15cUserHistoryLog6EndPvPE23ENUM_PVP_HISTORY_REASONbj
    // DEMANGLED: cUserHistoryLog::EndPvP(ENUM_PVP_HISTORY_REASON,bool,unsigned int)
    int EndPvP(ENUM_PVP_HISTORY_REASON reason,
               bool team_result,
               unsigned int elapsed_seconds);

    // IDA_EA: 0x8684AC4
    // IDA_NAME: _ZN15cUserHistoryLog16DungeonClearInfoEil
    // DEMANGLED: cUserHistoryLog::DungeonClearInfo(int,long)
    int DungeonClearInfo(int cleared, long elapsed_seconds);

    // IDA_EA: 0x8684A6E
    // IDA_NAME: _ZN15cUserHistoryLog16DungeonClearInfoEiliii17ENUM_DUNGEON_MODEPKci
    // DEMANGLED: cUserHistoryLog::DungeonClearInfo(
    //     int,long,int,int,int,ENUM_DUNGEON_MODE,char const *,int)
    int DungeonClearInfo(int entrance_count,
                         long elapsed_seconds,
                         int stage_index,
                         int layer,
                         int clear_state,
                         ENUM_DUNGEON_MODE dungeon_mode,
                         const char* member_names,
                         int character_gage);

    // IDA_EA: 0x8682D3A
    // IDA_NAME: _ZN15cUserHistoryLog12RandomOptionERK10Inven_ItemN18randomOptionReason1TE
    // DEMANGLED: cUserHistoryLog::RandomOption(Inven_Item const&,randomOptionReason::T)
    int RandomOption(const Inven_Item& item, randomOptionReason::T reason);

    // IDA_EA: 0x8684DFA
    // IDA_NAME: _ZN15cUserHistoryLog9CharacDieEPKctt
    // DEMANGLED: cUserHistoryLog::CharacDie(char const*,unsigned short,unsigned short)
    int CharacDie(const char* character_name,
                  uint16_t death_type,
                  uint16_t death_detail);

    // IDA_EA: 0x86869E8
    // IDA_NAME: _ZN15cUserHistoryLog13pvpMissionAddEiiii
    // DEMANGLED: cUserHistoryLog::pvpMissionAdd(int,int,int,int)
    int pvpMissionAdd(int mission_kind,
                      int mission_index,
                      int current_value,
                      int target_value);

    // IDA_EA: 0x8686A22
    // IDA_NAME: _ZN15cUserHistoryLog13pvpMissionDelEii
    // DEMANGLED: cUserHistoryLog::pvpMissionDel(int,int)
    int pvpMissionDel(int mission_kind, int mission_index);

    // IDA_EA: 0x8686A4E
    // IDA_NAME: _ZN15cUserHistoryLog21pvpMissionClearRewardEiiii
    // DEMANGLED: cUserHistoryLog::pvpMissionClearReward(int,int,int,int)
    int pvpMissionClearReward(int pvp_point,
                              int mission_exp,
                              int mission_kind,
                              int mission_index);

    // IDA_EA: 0x8686ACA
    // IDA_NAME: _ZN15cUserHistoryLog20MercenaryCompetitionEjcc
    // DEMANGLED: cUserHistoryLog::MercenaryCompetition(unsigned int,char,char)
    int MercenaryCompetition(unsigned int charac_no,
                             char competition_type,
                             char competition_count);
    // IDA_EA: 0x8686B0A
    // IDA_NAME: _ZN15cUserHistoryLog15MercenaryReturnEjiiiii
    // DEMANGLED: cUserHistoryLog::MercenaryReturn(
    //             unsigned int,int,int,int,int,int)
    int MercenaryReturn(unsigned int charac_no,
                        int start_time,
                        int end_time,
                        int period_term,
                        int gold,
                        int item_count);
    // IDA_EA: 0x8682A92
    // IDA_NAME: _ZN15cUserHistoryLog9CharacAddEi
    // DEMANGLED: cUserHistoryLog::CharacAdd(int)
    int CharacAdd(int charac_slot);

    // IDA_EA: 0x8682AE0
    // IDA_NAME: _ZN15cUserHistoryLog9CharacDelEi
    // DEMANGLED: cUserHistoryLog::CharacDel(int)
    int CharacDel(int charac_slot);

    // IDA_EA: 0x8685A48
    // IDA_NAME: _ZN15cUserHistoryLog9GuildJoinEjPKcjtc
    // DEMANGLED: cUserHistoryLog::GuildJoin(unsigned int,char const*,
    //                                       unsigned int,unsigned short,char)
    int GuildJoin(unsigned int guild_key,
                  const char* guild_name,
                  unsigned int charac_no,
                  unsigned short guild_member_index,
                  char guild_member_grade);

    // IDA_EA: 0x8685978
    // IDA_NAME: _ZN15cUserHistoryLog11GuildCreateEjPKcj
    // DEMANGLED: cUserHistoryLog::GuildCreate(unsigned int,char const*,unsigned int)
    int GuildCreate(unsigned int guild_key,
                    const char* guild_name,
                    unsigned int charac_no);

    // IDA_EA: 0x8685B00
    // IDA_NAME: _ZN15cUserHistoryLog8GuildOutEjPKcjct
    // DEMANGLED: cUserHistoryLog::GuildOut(unsigned int,char const*,unsigned int,char,unsigned short)
    int GuildOut(unsigned int guild_key,
                 const char* guild_name,
                 unsigned int charac_no,
                 char guild_member_grade,
                 unsigned short guild_member_index);

    // IDA_EA: 0x8685C4E
    // IDA_NAME: _ZN15cUserHistoryLog16GuildChangeGradeEjPKcjjhh
    // DEMANGLED: cUserHistoryLog::GuildChangeGrade(
    //             unsigned int,char const*,unsigned int,unsigned int,
    //             unsigned char,unsigned char)
    int GuildChangeGrade(unsigned int guild_key,
                         const char* guild_name,
                         unsigned int charac_no,
                         unsigned int target_charac_no,
                         unsigned char target_grade,
                         unsigned char prior_grade);

    // IDA_EA: 0x822F084
    // IDA_NAME: _ZN15cUserHistoryLog12SetItemXYPosEtt
    // DEMANGLED: cUserHistoryLog::SetItemXYPos(unsigned short,unsigned short)
    cUserHistoryLog* SetItemXYPos(uint16_t item_x, uint16_t item_y);

    // IDA_EA: 0x8685172
    // IDA_NAME: _ZN15cUserHistoryLog8MoveAreaEiiiii
    // DEMANGLED: cUserHistoryLog::MoveArea(int,int,int,int,int)
    int MoveArea(int from_village,
                 int from_area,
                 int to_village,
                 int to_area,
                 int move_type);

    // IDA_EA: 0x8683A20
    // IDA_NAME: _ZN15cUserHistoryLog8MoneyAddEii15eMoneyAddReason
    // DEMANGLED: cUserHistoryLog::MoneyAdd(int,int,eMoneyAddReason)
    int MoneyAdd(int money, int add_count, eMoneyAddReason reason);

    // IDA_EA: 0x8683AE0
    // IDA_NAME: _ZN15cUserHistoryLog13MoneyAddTradeEii15eMoneyAddReason
    // DEMANGLED: cUserHistoryLog::MoneyAddTrade(int,int,eMoneyAddReason)
    int MoneyAddTrade(int money, int add_count, eMoneyAddReason reason);

    // IDA_EA: 0x8683A80
    // IDA_NAME: _ZN15cUserHistoryLog8MoneySubEii15eMoneySubReason
    // DEMANGLED: cUserHistoryLog::MoneySub(int,int,eMoneySubReason)
    int MoneySub(int money, int sub_count, eMoneySubReason reason);

    // IDA_EA: 0x8683B38
    // IDA_NAME: _ZN15cUserHistoryLog13MoneySubTradeEii15eMoneySubReason
    // DEMANGLED: cUserHistoryLog::MoneySubTrade(int,int,eMoneySubReason)
    int MoneySubTrade(int money, int sub_count, eMoneySubReason reason);

    // IDA_EA: 0x8683B90
    // IDA_NAME: _ZN15cUserHistoryLog7CoinAddEii14eCoinAddReason
    // DEMANGLED: cUserHistoryLog::CoinAdd(int,int,eCoinAddReason)
    int CoinAdd(int coin, int add_count, eCoinAddReason reason);

    // IDA_EA: 0x8683BC2
    // IDA_NAME: _ZN15cUserHistoryLog7CoinSubEii14eCoinSubReason
    // DEMANGLED: cUserHistoryLog::CoinSub(int,int,eCoinSubReason)
    int CoinSub(int coin, int sub_count, eCoinSubReason reason);

    // IDA_EA: 0x8683BF4
    // IDA_NAME: _ZN15cUserHistoryLog10PayCoinAddEii14eCoinAddReason
    // DEMANGLED: cUserHistoryLog::PayCoinAdd(int,int,eCoinAddReason)
    int PayCoinAdd(int coin, int add_count, eCoinAddReason reason);

    // IDA_EA: 0x8683C26
    // IDA_NAME: _ZN15cUserHistoryLog10PayCoinSubEii14eCoinSubReason
    // DEMANGLED: cUserHistoryLog::PayCoinSub(int,int,eCoinSubReason)
    int PayCoinSub(int coin, int sub_count, eCoinSubReason reason);

    // IDA_EA: 0x8682BAE
    // IDA_NAME: _ZN15cUserHistoryLog5SPAddEiii12eSPAddReason
    // DEMANGLED: cUserHistoryLog::SPAdd(int,int,int,eSPAddReason)
    int SPAdd(int skill_tree_kind,
              int remaining_sp,
              int gained_sp,
              eSPAddReason reason);

    // IDA_EA: 0x8682BE8
    // IDA_NAME: _ZN15cUserHistoryLog5SPSubEiii12eSPSubReason
    int SPSub(int skill_tree_kind,
              int remaining_sp,
              int spent_sp,
              eSPSubReason reason);

    // IDA_EA: 0x8682C22
    // IDA_NAME: _ZN15cUserHistoryLog6SFPAddEiii12eSPAddReason
    // DEMANGLED: cUserHistoryLog::SFPAdd(int,int,int,eSPAddReason)
    int SFPAdd(int skill_tree_kind,
               int remaining_sfp,
               int gained_sfp,
               eSPAddReason reason);

    // IDA_EA: 0x8682C5C
    // IDA_NAME: _ZN15cUserHistoryLog6SFPSubEiii12eSPSubReason
    int SFPSub(int skill_tree_kind,
               int remaining_sfp,
               int spent_sfp,
               eSPSubReason reason);

    // IDA_EA: 0x8682C96
    // IDA_NAME: _ZN15cUserHistoryLog5WPAddEii12eWPAddReason
    // DEMANGLED: cUserHistoryLog::WPAdd(int,int,eWPAddReason)
    int WPAdd(int current_point, int added_point, eWPAddReason reason);

    // IDA_EA: 0x8682CC8
    // IDA_NAME: _ZN15cUserHistoryLog5WPSubEii12eWPSubReason
    // DEMANGLED: cUserHistoryLog::WPSub(int,int,eWPSubReason)
    int WPSub(int current_point, int removed_point, eWPSubReason reason);

    // IDA_EA: 0x8683C58
    // IDA_NAME: _ZN15cUserHistoryLog12EventCoinAddEii14eCoinAddReason
    // DEMANGLED: cUserHistoryLog::EventCoinAdd(int,int,eCoinAddReason)
    int EventCoinAdd(int current_coin, int added_coin, eCoinAddReason reason);

    // IDA_EA: 0x8683C8A
    // IDA_NAME: _ZN15cUserHistoryLog12EventCoinSubEii14eCoinSubReason
    // DEMANGLED: cUserHistoryLog::EventCoinSub(int,int,eCoinSubReason)
    int EventCoinSub(int coin, int sub_count, eCoinSubReason reason);

    // IDA_EA: 0x86848A8
    // IDA_NAME: _ZN15cUserHistoryLog8RecvMailEjPKcS1_
    // DEMANGLED: cUserHistoryLog::RecvMail(unsigned int,char const*,char const*)
    int RecvMail(unsigned int mail_idx,
                 const char* sender_name,
                 const char* account_name);

    // IDA_EA: 0x8683CBC
    // IDA_NAME: _ZN15cUserHistoryLog15ExpandInventoryEiii
    // DEMANGLED: cUserHistoryLog::ExpandInventory(int,int,int)
    int ExpandInventory(int old_capacity, int new_capacity, int reason);

    // IDA_EA: 0x8689808
    // IDA_NAME: _ZN15cUserHistoryLog16ItemLockUsingLogEhiiii
    // DEMANGLED: cUserHistoryLog::ItemLockUsingLog(unsigned char,int,int,int,int)
    int ItemLockUsingLog(unsigned char action,
                         int item_space,
                         int slot,
                         int item_id,
                         int item_key);

    // IDA_EA: 0x868885A
    // IDA_NAME: _ZN15cUserHistoryLog5QPSubEiii
    // DEMANGLED: cUserHistoryLog::QPSub(int,int,int)
    int QPSub(int before_qp, int after_qp, int status_id);

    // IDA_EA: 0x86888BE
    // IDA_NAME: _ZN15cUserHistoryLog11ClearUsedQPEiiii
    // DEMANGLED: cUserHistoryLog::ClearUsedQP(int,int,int,int)
    int ClearUsedQP(int before_qp,
                    int after_qp,
                    int init_count,
                    int init_cost);

    // IDA_EA: 0x8684B22
    // IDA_NAME: _ZN15cUserHistoryLog8MapClearEtii
    // DEMANGLED: cUserHistoryLog::MapClear(unsigned short,int,int)
    int MapClear(unsigned short hit_count, int play_time_sec, int map_index);

    // IDA_EA: 0x8684AF0
    // IDA_NAME: _ZN15cUserHistoryLog15BloodRoundClearEijjPKcS1_
    // DEMANGLED: cUserHistoryLog::BloodRoundClear(int,unsigned int,unsigned int,char const*,char const*)
    int BloodRoundClear(int blood_round,
                        unsigned int round_seconds,
                        unsigned int total_play_seconds,
                        const char* dungeon_name,
                        const char* member_names);

    // IDA_EA: 0x8684B96
    // IDA_NAME: _ZN15cUserHistoryLog5FPSubEiii12eFPSubReason
    // DEMANGLED: cUserHistoryLog::FPSub(int,int,int,eFPSubReason)
    int FPSub(int point,
              int remaining_point,
              int total_point,
              eFPSubReason reason);

    // IDA_EA: 0x8684B5C
    // IDA_NAME: _ZN15cUserHistoryLog5FPAddEiii12eFPAddReason
    // DEMANGLED: cUserHistoryLog::FPAdd(int,int,int,eFPAddReason)
    int FPAdd(int point,
              int remaining_point,
              int total_point,
              eFPAddReason reason);

    // IDA_EA: 0x8682B2E
    // IDA_NAME: _ZN15cUserHistoryLog8SkillAddEiiii15eSkillAddReason
    // DEMANGLED: cUserHistoryLog::SkillAdd(
    //             int,int,int,int,eSkillAddReason)
    int SkillAdd(int charac_job,
                 int skill_tree,
                 int skill_index,
                 int skill_level,
                 eSkillAddReason reason);

    // IDA_EA: 0x8682B6E
    // IDA_NAME: _ZN15cUserHistoryLog8SkillDelEiiii15eSkillDelReason
    // DEMANGLED: cUserHistoryLog::SkillDel(
    //             int,int,int,int,eSkillDelReason)
    int SkillDel(int charac_job,
                 int skill_tree,
                 int skill_index,
                 int skill_level,
                 eSkillDelReason reason);

    // IDA_EA: 0x8682CFA
    // IDA_NAME: _ZN15cUserHistoryLog9InitSkillEiiii16eSkillInitReason
    // DEMANGLED: cUserHistoryLog::InitSkill(int,int,int,int,eSkillInitReason)
    int InitSkill(int skill_tree,
                  int charac_level,
                  int remaining_sp,
                  int remaining_sfp,
                  eSkillInitReason reason);

    // IDA_EA: 0x8685DCE
    // IDA_NAME: _ZN15cUserHistoryLog9SkillSaveEPK15_Mastered_skillji16eSkillSaveReason
    // DEMANGLED: cUserHistoryLog::SkillSave(
    //     _Mastered_skill const*,unsigned int,int,eSkillSaveReason)
    void SkillSave(const _Mastered_skill* skills,
                   unsigned int charac_job,
                   int skill_tree,
                   eSkillSaveReason reason);

    // IDA_EA: 0x8686188
    // IDA_NAME: _ZN15cUserHistoryLog15SkillSPInfoSaveEP5CUser22eSkillSPInfoSaveReason
    // DEMANGLED: cUserHistoryLog::SkillSPInfoSave(
    //     CUser*,eSkillSPInfoSaveReason)
    void SkillSPInfoSave(CUser* user, eSkillSPInfoSaveReason reason);

    // IDA_EA: 0x8685BB8
    // IDA_NAME: _ZN15cUserHistoryLog11GuildAddExpEjPKcji
    // DEMANGLED: cUserHistoryLog::GuildAddExp(unsigned int,char const*,unsigned int,int)
    int GuildAddExp(unsigned int guild_key,
                    const char* guild_name,
                    unsigned int charac_no,
                    int guild_exp);

    // IDA_EA: 0x8686544
    // IDA_NAME: _ZN15cUserHistoryLog17GuildCargoHistoryE31ENUM_GUILD_CARGO_HISTORY_REASONi
    // DEMANGLED: cUserHistoryLog::GuildCargoHistory(
    //             ENUM_GUILD_CARGO_HISTORY_REASON,int)
    int GuildCargoHistory(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                          int charac_guildkey);

    // IDA_EA: 0x86864E6
    // IDA_NAME: _ZN15cUserHistoryLog10GuildCargoE31ENUM_GUILD_CARGO_HISTORY_REASONi
    // DEMANGLED: cUserHistoryLog::GuildCargo(
    //             ENUM_GUILD_CARGO_HISTORY_REASON,int)
    int GuildCargo(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                   int charac_guildkey);

    // IDA_EA: 0x8686512
    // IDA_NAME: _ZN15cUserHistoryLog16GuildCargoResultE31ENUM_GUILD_CARGO_HISTORY_REASONii
    int GuildCargoResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                         int result,
                         int capacity);

    // IDA_EA: 0x8686570
    // IDA_NAME: _ZN15cUserHistoryLog23GuildCargoHistoryResultE31ENUM_GUILD_CARGO_HISTORY_REASONi
    int GuildCargoHistoryResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                int item_count);

    // IDA_EA: 0x868659C
    // IDA_NAME: _ZN15cUserHistoryLog23GuildCargoCheckPushItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
    // DEMANGLED: cUserHistoryLog::GuildCargoCheckPushItem(
    //             ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,int)
    int GuildCargoCheckPushItem(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                int item_count,
                                int item_id,
                                int add_info,
                                int item_space,
                                int slot,
                                int is_stackable);

    // IDA_EA: 0x86865EA
    // IDA_NAME: _ZN15cUserHistoryLog29GuildCargoCheckPushItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
    int GuildCargoCheckPushItemResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                      int result,
                                      int item_count,
                                      int item_id,
                                      int add_info,
                                      int item_space,
                                      int slot);

    // IDA_EA: 0x8686638
    // IDA_NAME: _ZN15cUserHistoryLog18GuildCargoPushItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiiih
    int GuildCargoPushItem(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                           int item_count,
                           int item_id,
                           int add_info,
                           int item_space,
                           int slot,
                           int is_stackable,
                           unsigned char result);

    // IDA_EA: 0x8686694
    // IDA_NAME: _ZN15cUserHistoryLog22GuildCargoItemRecoveryE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiih
    int GuildCargoItemRecovery(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                               int item_id,
                               int add_info,
                               int avatar_select,
                               int extend_info,
                               int item_attr,
                               unsigned char upgrade_separate);

    // IDA_EA: 0x86866EA
    // IDA_NAME: _ZN15cUserHistoryLog17GuildCargoPopItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiii
    // DEMANGLED: cUserHistoryLog::GuildCargoPopItem(
    //             ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int)
    int GuildCargoPopItem(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                          int cargo_slot,
                          int item_id,
                          int add_info,
                          int is_stackable,
                          int item_count);

    // IDA_EA: 0x8686732
    // IDA_NAME: _ZN15cUserHistoryLog24GuildCargoPushItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
    int GuildCargoPushItemResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                 int result,
                                 int cargo_slot,
                                 int item_id,
                                 int add_info,
                                 int item_space,
                                 int slot);

    // IDA_EA: 0x8686780
    // IDA_NAME: _ZN15cUserHistoryLog23GuildCargoPopItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
    int GuildCargoPopItemResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                int result,
                                int cargo_slot,
                                int item_id,
                                int add_info,
                                int avatar_select,
                                int extend_info);

    // IDA_EA: 0x86867CE
    // IDA_NAME: _ZN15cUserHistoryLog18GuildCargoMoveItemE31ENUM_GUILD_CARGO_HISTORY_REASONii
    // DEMANGLED: cUserHistoryLog::GuildCargoMoveItem(
    //             ENUM_GUILD_CARGO_HISTORY_REASON,int,int)
    int GuildCargoMoveItem(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                           int from_slot,
                           int to_slot);

    // IDA_EA: 0x8686800
    // IDA_NAME: _ZN15cUserHistoryLog24GuildCargoMoveItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiii
    int GuildCargoMoveItemResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                 int result,
                                 int from_slot,
                                 int to_slot);

    // IDA_EA: 0x8532036
    // IDA_NAME: _ZN15cUserHistoryLog9SetTraderEPKcS1_
    // DEMANGLED: cUserHistoryLog::SetTrader(char const*,char const*)
    char* SetTrader(const char* account_name, const char* character_name);

    // IDA_EA: 0x868503E
    // IDA_NAME: _ZN15cUserHistoryLog10TradeBeginEv
    // DEMANGLED: cUserHistoryLog::TradeBegin(void)
    int TradeBegin();

    // IDA_EA: 0x868507C
    // IDA_NAME: _ZN15cUserHistoryLog8TradeEndE15eTradeEndReasoniiii
    // DEMANGLED: cUserHistoryLog::TradeEnd(eTradeEndReason,int,int,int,int)
    int TradeEnd(eTradeEndReason reason,
                 int own_item_count,
                 int own_gold,
                 int peer_item_count,
                 int peer_gold);

    // IDA_EA: 0x8683D5A
    // IDA_NAME: _ZN15cUserHistoryLog16TradeItemAddFailEii
    // DEMANGLED: cUserHistoryLog::TradeItemAddFail(int,int)
    int TradeItemAddFail(int item_id, int item_count);

    // IDA_EA: 0x8684652
    // IDA_NAME: _ZN15cUserHistoryLog13AvatarItemAddEiiPKc20eAvatarItemAddReason
    // DEMANGLED: cUserHistoryLog::AvatarItemAdd(int,int,char const*,eAvatarItemAddReason)
    int AvatarItemAdd(int item_id,
                      int avatar_ui_id,
                      const char* ipg_no,
                      eAvatarItemAddReason reason);

    // IDA_EA: 0x86846FA
    // IDA_NAME: _ZN15cUserHistoryLog13AvatarItemDelEiiPKc20eAvatarItemDelReason
    // DEMANGLED: cUserHistoryLog::AvatarItemDel(int,int,char const*,eAvatarItemDelReason)
    int AvatarItemDel(int item_id,
                      int avatar_ui_id,
                      const char* ipg_no,
                      eAvatarItemDelReason reason);

    // IDA_EA: 0x8686CBA
    // IDA_NAME: _ZN15cUserHistoryLog6UseDyeEiiiRKSsi
    // DEMANGLED: cUserHistoryLog::UseDye(int,int,int,std::string const&,int)
    CUser* UseDye(int dye_item_id,
                  int avatar_item_id,
                  int avatar_key,
                  const std::string& ipg_agency_no,
                  int avatar_slot);

    // IDA_EA: 0x8686D0E
    // IDA_NAME: _ZN15cUserHistoryLog21UseAvatarOptionChangeEiiii
    // DEMANGLED: cUserHistoryLog::UseAvatarOptionChange(int,int,int,int)
    CUser* UseAvatarOptionChange(int avatar_ui_id,
                                 int avatar_item_id,
                                 int old_option,
                                 int new_option);

    // IDA_EA: 0x8682E84
    // IDA_NAME: _ZN15cUserHistoryLog7ItemAddE10INVEN_TYPEiiRK10Inven_Item14eItemAddReason
    // DEMANGLED: cUserHistoryLog::ItemAdd(INVEN_TYPE,int,int,Inven_Item const&,eItemAddReason)
    void ItemAdd(INVEN_TYPE inventory_type,
                 int value_1,
                 int value_2,
                 const Inven_Item& item,
                 eItemAddReason reason);

    // IDA_EA: 0x8684E3C
    // IDA_NAME: _ZN15cUserHistoryLog15CreatureItemAddE10INVEN_TYPEiiii14eItemAddReason
    // DEMANGLED: cUserHistoryLog::CreatureItemAdd(INVEN_TYPE,int,int,int,int,eItemAddReason)
    void CreatureItemAdd(INVEN_TYPE inventory_type,
                         int item_id,
                         int add_info,
                         int item_count,
                         int item_type,
                         eItemAddReason reason);

    // IDA_EA: 0x8684F40
    // IDA_NAME: _ZN15cUserHistoryLog15CreatureItemDelE10INVEN_TYPEiiii14eItemDelReason
    // DEMANGLED: cUserHistoryLog::CreatureItemDel(INVEN_TYPE,int,int,int,int,eItemDelReason)
    void CreatureItemDel(INVEN_TYPE inventory_type,
                         int item_id,
                         int add_info,
                         int item_count,
                         int item_type,
                         eItemDelReason reason);

    // IDA_EA: 0x8686D54
    // IDA_NAME: _ZN15cUserHistoryLog20ChangeItemLimitCountEiiiii
    // DEMANGLED: cUserHistoryLog::ChangeItemLimitCount(int,int,int,int,int)
    CUser* ChangeItemLimitCount(int direction,
                                int slot,
                                int item_id,
                                int value,
                                int reason);

    // IDA_EA: 0x868340A
    // IDA_NAME: _ZN15cUserHistoryLog7ItemDelE10INVEN_TYPEiiRK10Inven_Item14eItemDelReason
    // DEMANGLED: cUserHistoryLog::ItemDel(INVEN_TYPE,int,int,Inven_Item const&,eItemDelReason)
    void ItemDel(INVEN_TYPE inventory_type,
                 int value_1,
                 int value_2,
                 const Inven_Item& item,
                 eItemDelReason reason);

    // IDA_EA: 0x8683D20
    // IDA_NAME: _ZN15cUserHistoryLog12ItemDelCargoEiii14eItemDelReason
    // DEMANGLED: cUserHistoryLog::ItemDelCargo(int,int,int,eItemDelReason)
    int ItemDelCargo(int item_id,
                     int item_count,
                     int remaining_count,
                     eItemDelReason reason);

    // IDA_EA: 0x86868B4
    // IDA_NAME: _ZN15cUserHistoryLog19AccountCargoAddItemEiii14eItemAddReason
    // DEMANGLED: cUserHistoryLog::AccountCargoAddItem(int,int,int,eItemAddReason)
    int AccountCargoAddItem(int item_id,
                            int money,
                            int amount,
                            eItemAddReason reason);

    // IDA_EA: 0x86868EE
    // IDA_NAME: _ZN15cUserHistoryLog19AccountCargoDelItemEiii14eItemDelReason
    // DEMANGLED: cUserHistoryLog::AccountCargoDelItem(int,int,int,eItemDelReason)
    int AccountCargoDelItem(int item_id,
                            int money,
                            int amount,
                            eItemDelReason reason);

    // IDA_EA: 0x8686A88
    // IDA_NAME: _ZN15cUserHistoryLog18ItemMakingSkillAddE21ENUM_ITEM_MAKING_TYPEiis
    // DEMANGLED: cUserHistoryLog::ItemMakingSkillAdd(ENUM_ITEM_MAKING_TYPE,int,int,short)
    int ItemMakingSkillAdd(ENUM_ITEM_MAKING_TYPE type,
                           int reason,
                           int gain,
                           short current);

    // IDA_EA: 0x868491A
    // IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEiiiPKcci
    // DEMANGLED: cUserHistoryLog::EnterDungeon(int,int,int,char const*,char,int)
    int EnterDungeon(int stage_index,
                     int stage_difficulty,
                     int enter_type,
                     const char* member_names,
                     char stage_kind,
                     int reserved);

    // IDA_EA: 0x8683D86
    // IDA_NAME: _ZN15cUserHistoryLog5LoginEPKcS1_iihi
    // DEMANGLED: cUserHistoryLog::Login(
    //     char const*,char const*,int,int,unsigned char,int)
    int Login(const char* web_address,
              const char* peer_ip,
              int peer_port,
              int channel_no,
              unsigned char login_out_state,
              int server_group);

    // IDA_EA: 0x86840C4
    // IDA_NAME: _ZN15cUserHistoryLog6LogoutEPKcS1_iiih
    // DEMANGLED: cUserHistoryLog::Logout(char const*,char const*,int,int,int,unsigned char)
    int Logout(const char* web_address,
               const char* peer_ip,
               int port,
               int process_sequence,
               int play_time,
               unsigned char login_out_state);

    // IDA_EA: 0x86849A2
    // IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEPKciS1_i
    // DEMANGLED: cUserHistoryLog::EnterDungeon(char const*,int,char const*,int)
    int EnterDungeon(const char* dungeon_name,
                     int difficulty,
                     const char* member_names,
                     int map_count);

    // IDA_EA: 0x8684A16
    // IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEPKci
    // DEMANGLED: cUserHistoryLog::EnterDungeon(char const*,int)
    int EnterDungeon(const char* dungeon_name, int difficulty);

    // IDA_EA: 0x8684968
    // IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEiiPKci
    // DEMANGLED: cUserHistoryLog::LeaveDungeon(int,int,char const*,int)
    int LeaveDungeon(int stage_index,
                     int stage_difficulty,
                     const char* member_names,
                     int stage_kind);

    // IDA_EA: 0x86849DC
    // IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEPKciS1_i
    // DEMANGLED: cUserHistoryLog::LeaveDungeon(char const*,int,char const*,int)
    int LeaveDungeon(const char* dungeon_name,
                     int difficulty,
                     const char* member_names,
                     int map_count);

    // IDA_EA: 0x8684A42
    // IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEPKci
    // DEMANGLED: cUserHistoryLog::LeaveDungeon(char const*,int)
    int LeaveDungeon(const char* dungeon_name, int difficulty);

    // IDA_EA: 0x8685452
    // IDA_NAME: _ZN15cUserHistoryLog7AuctionEP26SIG_AUCTION_LOG_MESSAGE_AG
    // DEMANGLED: cUserHistoryLog::Auction(SIG_AUCTION_LOG_MESSAGE_AG *)
    int Auction(SIG_AUCTION_LOG_MESSAGE_AG* message);

    // IDA_EA: 0x8685654
    // IDA_NAME: _ZN15cUserHistoryLog12AuctionPointEP26SIG_AUCTION_LOG_MESSAGE_PG
    // DEMANGLED: cUserHistoryLog::AuctionPoint(SIG_AUCTION_LOG_MESSAGE_PG *)
    int AuctionPoint(SIG_AUCTION_LOG_MESSAGE_PG* message);

    // IDA_EA: 0x868512A
    // IDA_NAME: _ZN15cUserHistoryLog19AchievementCompleteEi
    // DEMANGLED: cUserHistoryLog::AchievementComplete(int)
    int AchievementComplete(int achievement_id);

    // IDA_EA: 0x868514E
    // IDA_NAME: _ZN15cUserHistoryLog13QuestCompleteEi
    // DEMANGLED: cUserHistoryLog::QuestComplete(int)
    int QuestComplete(int quest_index);

    // IDA_EA: 0x868888C
    // IDA_NAME: _ZN15cUserHistoryLog5QPAddEiii
    // DEMANGLED: cUserHistoryLog::QPAdd(int,int,int)
    int QPAdd(int previous_quest_point,
              int quest_index,
              int added_quest_point);

    // IDA_EA: 0x8686F1E
    // IDA_NAME: _ZN15cUserHistoryLog17DeleteInvaildItemERKSsii
    // DEMANGLED: cUserHistoryLog::DeleteInvaildItem(std::string const&,int,int)
    int DeleteInvaildItem(const std::string& source,
                          int item_id,
                          int add_info);

    // IDA_EA: 0x8684D0A
    // IDA_NAME: _ZN15cUserHistoryLog19SeparateUpgradeItemEiiib
    // DEMANGLED: cUserHistoryLog::SeparateUpgradeItem(int,int,int,bool)
    int SeparateUpgradeItem(int item_index,
                            int old_separate_upgrade,
                            int new_separate_upgrade,
                            bool succeeded);
    // IDA_EA: 0x8684D74
    // IDA_NAME: _ZN15cUserHistoryLog11EnchantItemEiiibPKcS1_
    // DEMANGLED: cUserHistoryLog::EnchantItem(int,int,int,bool,char const*,char const*)
    int EnchantItem(int card_item_id,
                    int old_enchant,
                    int new_enchant,
                    bool succeeded,
                    const char* account_name,
                    const char* character_name);
    // IDA_EA: 0x8686490
    // IDA_NAME: _ZN15cUserHistoryLog6DiePvPE23ENUM_PVP_HISTORY_REASONjjiiiiPKc
    // DEMANGLED: cUserHistoryLog::DiePvP(ENUM_PVP_HISTORY_REASON,unsigned int,unsigned int,int,int,int,int,char const*)
    int DiePvP(ENUM_PVP_HISTORY_REASON reason,
               unsigned int damage_value,
               unsigned int damage_count,
               int attacker_stat0,
               int attacker_stat1,
               int attacker_stat2,
               int attacker_stat3,
               const char* killer_name);
    // IDA_EA: 0x8684CA0
    // IDA_NAME: _ZN15cUserHistoryLog11UpgradeItemEiiib
    // DEMANGLED: cUserHistoryLog::UpgradeItem(int,int,int,bool)
    int UpgradeItem(int item_index,
                    int old_upgrade,
                    int new_upgrade,
                    bool succeeded);

    // IDA_EA: 0x8684C28
    // IDA_NAME: _ZN15cUserHistoryLog23Enchant3rdChronicleItemEbiiii
    // DEMANGLED: cUserHistoryLog::Enchant3rdChronicleItem(bool,int,int,int,int)
    int Enchant3rdChronicleItem(bool succeeded,
                                int target_item_index,
                                int material_item_index,
                                int enchant_type,
                                int applied_enchant_count);
    // IDA_EA: 0x8686C7C
    // IDA_NAME: _ZN15cUserHistoryLog10apply_ItemEii16eApplyItemReason
    // DEMANGLED: cUserHistoryLog::apply_Item(int,int,eApplyItemReason)
    int apply_Item(int item_index,
                   int add_info,
                   eApplyItemReason reason);

    // IDA_EA: 0x8686B52
    // IDA_NAME: _ZN15cUserHistoryLog16UseSelectBoosterEjiiRSt6vectorI10Inven_ItemSaIS1_EE
    // DEMANGLED: cUserHistoryLog::UseSelectBooster(unsigned int,int,int,std::vector<Inven_Item,std::allocator<Inven_Item>> &)
    int UseSelectBooster(unsigned int source_item_id,
                         int first_category,
                         int second_category,
                         std::vector<Inven_Item>& selected_items);

    // IDA_EA: 0x868455A
    // IDA_NAME: _ZN15cUserHistoryLog18CreatePrivateStoreEiiPKc
    // DEMANGLED: cUserHistoryLog::CreatePrivateStore(int,int,char const*)
    int CreatePrivateStore(int village_index,
                           int area_index,
                           const char* character_name);
    // IDA_EA: 0x8684586
    // IDA_NAME: _ZN15cUserHistoryLog18RemovePrivateStoreEiiPKc
    // DEMANGLED: cUserHistoryLog::RemovePrivateStore(int,int,char const*)
    int RemovePrivateStore(int village_index,
                           int area_index,
                           const char* character_name);

    // IDA_EA: 0x86845B2
    // IDA_NAME: _ZN15cUserHistoryLog7LevelUpEi14eLevelUpReason
    // DEMANGLED: cUserHistoryLog::LevelUp(int,eLevelUpReason)
    int LevelUp(int charac_level, eLevelUpReason reason);

    // IDA_EA: 0x86845DE
    // IDA_NAME: _ZN15cUserHistoryLog9LevelDownEi16eLevelDownReason
    // DEMANGLED: cUserHistoryLog::LevelDown(int,eLevelDownReason)
    int LevelDown(int charac_level, eLevelDownReason reason);

    // IDA_EA: 0x868460A
    // IDA_NAME: _ZN15cUserHistoryLog14ChangeGrowTypeEiiiii21eChangeGrowTypeReason
    // DEMANGLED: cUserHistoryLog::ChangeGrowType(int,int,int,int,int,eChangeGrowTypeReason)
    int ChangeGrowType(int charac_job,
                       int village,
                       int old_grow_type,
                       int new_grow_type,
                       int value,
                       eChangeGrowTypeReason reason);

    // IDA_EA: 0x86847EA
    // IDA_NAME: _ZN15cUserHistoryLog8SendMailEP16MSG_MAILBOX_SENDj
    // DEMANGLED: cUserHistoryLog::SendMail(MSG_MAILBOX_SEND*,unsigned int)
    void SendMail(MSG_MAILBOX_SEND* message, unsigned int value);

    // IDA_EA: 0x86851AC
    // IDA_NAME: _ZN15cUserHistoryLog11KillMonsterEijjttRK15MSG_MONSTER_DIEcbii13eExpAddReason
    // DEMANGLED: cUserHistoryLog::KillMonster(int,unsigned int,unsigned int,
    //             unsigned short,unsigned short,MSG_MONSTER_DIE const&,char,
    //             bool,int,int,eExpAddReason)
    int KillMonster(int monster_id,
                    unsigned int message_value_a1e,
                    unsigned int message_value_a1a,
                    unsigned short message_value_a24,
                    unsigned short message_value_a26,
                    const MSG_MONSTER_DIE& message,
                    char monster_kind,
                    bool is_apc,
                    int current_exp,
                    int gained_exp,
                    eExpAddReason reason);

    // IDA_EA: 0x8683CEE
    // IDA_NAME: _ZN15cUserHistoryLog12ItemAddCargoEiii
    // DEMANGLED: cUserHistoryLog::ItemAddCargo(int,int,int)
    int ItemAddCargo(int item_id, int item_count, int resulting_count);

    // IDA_EA: 0x8684BD0
    // IDA_NAME: _ZN15cUserHistoryLog6ExpAddEii13eExpAddReasoni
    // DEMANGLED: cUserHistoryLog::ExpAdd(int,int,eExpAddReason,int)
    int ExpAdd(int current_exp, int added_exp, eExpAddReason reason,
               int source_value);

    // IDA_EA: 0x8685418
    // IDA_NAME: _ZN15cUserHistoryLog15HadesHistoryLogEjjjj
    // DEMANGLED: cUserHistoryLog::HadesHistoryLog(unsigned int,unsigned int,unsigned int,unsigned int)
    int HadesHistoryLog(unsigned int value0,
                        unsigned int value1,
                        unsigned int value2,
                        unsigned int value3);

    // IDA_EA: 0x8685A08
    // IDA_NAME: _ZN15cUserHistoryLog9useEmblemEmmjPKcj
    // DEMANGLED: cUserHistoryLog::useEmblem(unsigned long,unsigned long,unsigned int,char const*,unsigned int)
    int useEmblem(unsigned long avatar_key,
                  unsigned long emblem_key,
                  unsigned int emblem_item_id,
                  const char* agency_no,
                  unsigned int value);

    // IDA_EA: 0x868683A
    // IDA_NAME: _ZN15cUserHistoryLog12CashInvenAddE19eCashInvenAddReasonjjm
    // DEMANGLED: cUserHistoryLog::CashInvenAdd(eCashInvenAddReason,unsigned int,unsigned int,unsigned long)
    int CashInvenAdd(eCashInvenAddReason reason,
                     unsigned int item_id,
                     unsigned int item_count,
                     unsigned long serial);

    // IDA_EA: 0x8686874
    // IDA_NAME: _ZN15cUserHistoryLog12CashInvenDelE19eCashInvenDelReasonjjmj
    // DEMANGLED: cUserHistoryLog::CashInvenDel(eCashInvenDelReason,unsigned int,unsigned int,unsigned long,unsigned int)
    int CashInvenDel(eCashInvenDelReason reason,
                     unsigned int item_id,
                     unsigned int item_count,
                     unsigned long serial,
                     unsigned int value);

    // IDA_EA: 0x8686C36
    // IDA_NAME: _ZN15cUserHistoryLog9seal_ItemEis15eSealItemReason
    // DEMANGLED: cUserHistoryLog::seal_Item(int,short,eSealItemReason)
    CUser* seal_Item(int item_id, short reseal_count,
                     eSealItemReason reason);

    // IDA_EA: 0x868412A
    // IDA_NAME: _ZN15cUserHistoryLog19PrivateStoreItemDelE10INVEN_TYPERK10Inven_Itemii14eItemDelReasonPKcS6_
    // DEMANGLED: cUserHistoryLog::PrivateStoreItemDel(INVEN_TYPE,Inven_Item const&,int,int,eItemDelReason,char const*,char const*)
    void PrivateStoreItemDel(INVEN_TYPE inventory_type,
                             const Inven_Item& item,
                             int value_1,
                             int value_2,
                             eItemDelReason reason,
                             const char* first_name,
                             const char* second_name);
    // IDA_EA: 0x868433E
    // IDA_NAME: _ZN15cUserHistoryLog19PrivateStoreItemAddE10INVEN_TYPERK10Inven_Itemii14eItemAddReasonPKcS6_
    // DEMANGLED: cUserHistoryLog::PrivateStoreItemAdd(INVEN_TYPE,Inven_Item const&,int,int,eItemAddReason,char const*,char const*)
    void PrivateStoreItemAdd(INVEN_TYPE inventory_type,
                             const Inven_Item& item,
                             int value_1,
                             int value_2,
                             eItemAddReason reason,
                             const char* first_name,
                             const char* second_name);

private:
    // IDA reads these members directly at +0x00, +0x04 and +0x22 in the
    // transaction/history functions above; SetItemXYPos @0x822F084 writes
    // the final two words at +0x40/+0x42.  Keeping the complete 0x44-byte
    // layout here removes the former CUser** / byte-offset pseudo-member ABI.
    CUser* owner_;                    // IDA +0x00
    char trader_character_name_[30];  // IDA +0x04
    char trader_account_name_[30];    // IDA +0x22
    uint16_t item_x_;                 // IDA +0x40
    uint16_t item_y_;                 // IDA +0x42
};

static_assert(sizeof(cUserHistoryLog) == 0x44u,
              "cUserHistoryLog IDA ABI size must stay 0x44");

// fillUserInfo4Antibot @0x84EC8AC fills the leading name and sentinel, while
// Inter_LoadCharac forwards the first 0x40 bytes to the old anti-bot API.
struct CUserAntibotInfo {
    char character_name[0x40];
    int32_t sentinel;
    uint8_t protocol_reserved[0x08];
};

static_assert(sizeof(CUserAntibotInfo) == 0x4cu,
              "CUser anti-bot user-info record must stay 0x4c");
static_assert(offsetof(CUserAntibotInfo, sentinel) == 0x40u,
              "CUser anti-bot sentinel offset drift");

struct CUserNetworkLifecycleState {
    uint8_t mercenary_reward_dice_override;
    int8_t mercenary_selection_index;
    uint8_t reserved[2];
};

static_assert(sizeof(CUserNetworkLifecycleState) == 0x04u,
              "CUser network lifecycle state must stay four bytes");

// IDA_EA: CUser::GetItemTimeChecker @0x82302AC owns this complete sampling
// window at expanded CUser+0x8e784.  Login clears sample_count at +0x0c.  The final
// two signed distances are adjacent members of the same state, not padding.
struct CUserItemTimeCheckState {
    int16_t minimum_x;       // +0x00
    int16_t maximum_x;       // +0x02
    int16_t minimum_y;       // +0x04
    int16_t maximum_y;       // +0x06
    uint32_t last_tick;      // +0x08
    int32_t sample_count;    // +0x0c
    int16_t distance_x;      // +0x10
    int16_t distance_y;      // +0x12
};

static_assert(sizeof(CUserItemTimeCheckState) == 0x14u,
              "CUser item-time checker state must stay 0x14 bytes");

// IDA_EA: CUser::GetItemSerialChecker @0x823066A owns the coordinate and
// serial tuple at expanded CUser+0x8e7c0.  The constructor clear at +0x8e7c4 is the
// serial counter itself; it is not a separate Hades scalar.
struct CUserItemSerialCheckState {
    int16_t previous_x;       // +0x00
    int16_t previous_y;       // +0x02
    int32_t serial_count;     // +0x04
};

static_assert(sizeof(CUserItemSerialCheckState) == 0x08u,
              "CUser item-serial checker state must stay eight bytes");

// IDA_EA: CUser::reset @0x8658030 clears the first two values; the adjacent
// CUser C2/D2 children are Character (+0x8e034) and std::string (+0x8e080),
// so this 0x0c range is CUser-owned POD rather than a hidden child object.
// Dispatcher_CharacterStatistic::dispatch_sig @0x82051D0 writes the flag,
// while SetRating/GetRating @0x84ECAA2/@0x8568E84 and
// SetSchoolNo/GetSchoolNo @0x84ECA90/@0x822FE02 access the two dwords.
struct CUserCharacterStatisticState {
    uint8_t statistic_received_;  // +0x00
    uint8_t reserved_01_03_[3];
    // CParty::die_user @0x85a7828 increments this exact word. SetRating and
    // GetRating preserve the same float bit pattern through memcpy, avoiding
    // a synthetic union-overlay view in the public layout.
    uint32_t rating_storage_;      // +0x04
    uint32_t school_no_;          // +0x08
};

static_assert(sizeof(CUserCharacterStatisticState) == 0x0cu,
              "CUser character-statistic state must stay 0x0c");

// IDA_EA: CUser::ClearChildrensDayEvent @0x832815A and its four paired
// accessors/setters at 0x83281BE..0x8328262 establish the complete 0x54-byte
// POD state.  No C2/D2 is called between character_string_ and the following
// clean-pad fields, so this is not a surrogate for a non-trivial child.
struct CUserChildrensDayEventState {
    uint8_t target_mask_;  // +0x00
    uint8_t reserved_01_03_[3];
    int32_t target_item_slots_[10];    // +0x04
    int32_t target_item_periods_[10];  // +0x2c
};

static_assert(sizeof(CUserChildrensDayEventState) == 0x54u,
              "CUser ChildrensDay state must stay 0x54");

// IDA_EA: CUser::handleInformNoticeFlag @0x868F956,
// CUser::checkInformNoticeFlag @0x868F9E8, and the neighbouring lifecycle
// helpers.  The original keeps sixteen one-byte flag banks followed by the
// dirty/save byte at expanded +0x08ef01.  The following std::vector requires 4-byte
// alignment, so the compiler supplies the two original alignment bytes after
// this 0x11-byte state; no opaque tail storage is introduced here.
struct CUserInformNoticeState {
    uint8_t flags[16];
    uint8_t save_pending;
};

static_assert(sizeof(CUserInformNoticeState) == 0x11u,
              "CUser inform-notice state must retain its 17-byte ABI");

// IDA unit: CUser.  The process allocates the full pool-backed object through
// its runtime owner, so this header exposes the verified gameplay member ABI
// without inventing an incomplete storage size.
class CUser : public CUserCharacInfo {
    friend class APSystem::Inter_LoadActionPoint;
    friend class APSystem::CUserProc;
public:
    // IDA type: CUser::st_monstorkill.  CMD42 invokes Check on the embedded
// object at expanded CUser+0x8e798; keeping the value type here preserves the
    // original nested-function identity and the physical owner relationship.
    struct st_monstorkill {
        int16_t min_x;                   // +0x00
        int16_t max_x;                   // +0x02
        int16_t min_y;                   // +0x04
        int16_t max_y;                   // +0x06
        uint32_t last_tick;              // +0x08
        uint32_t report_count;           // +0x0c
        int16_t span_x;                  // +0x10
        int16_t span_y;                  // +0x12

        // IDA_EA: 0x823049C
        // IDA_NAME: _ZN5CUser14st_monstorkill5CheckEjttttj
        // DEMANGLED: CUser::st_monstorkill::Check(unsigned int,unsigned short,unsigned short,unsigned short,unsigned short,unsigned int)
        int Check(unsigned int tick,
                  unsigned short reported_x,
                  unsigned short reported_y,
                  unsigned short maximum_span_x,
                  unsigned short maximum_span_y,
                  unsigned int reset_interval);
    };

    static_assert(sizeof(st_monstorkill) == 0x14u,
                  "CUser::st_monstorkill must retain its old 20-byte ABI");

    // Named enum surface used by the original packet-routing ABI.  It is
    // deliberately opaque here: the recovered call sites pass its verified
    // int-backed ABI value, while the original enumerator table is not yet
    // recovered.
    enum eSendTarget : int;

    // IDA_EA: 0x84FA298
    // IDA_NAME: _ZN5CUser11getHashCodeEPh
    // DEMANGLED: CUser::getHashCode(unsigned char *)
    int getHashCode(unsigned char* output);

    // IDA_EA: 0x868A782
    // IDA_NAME: _ZN5CUser32SendEventJarOutputItemStatisticsERKSt6vectorI10Inven_ItemSaIS1_EE
    // DEMANGLED: CUser::SendEventJarOutputItemStatistics(std::vector<Inven_Item,std::allocator<Inven_Item>> const&)
    unsigned char SendEventJarOutputItemStatistics(
        const std::vector<Inven_Item>& output_items);

    // IDA_EA: 0x8676826
    // IDA_NAME: _ZN5CUser22SendRandomBoxStatisticEi26ENUM_RANDOMBOX_ACTION_KIND
    // DEMANGLED: CUser::SendRandomBoxStatistic(int,ENUM_RANDOMBOX_ACTION_KIND)
    // The original is a static namespace member: callers pass only the box
    // category and action kind, with no CUser instance.
    static void SendRandomBoxStatistic(
        int box_category,
        ENUM_RANDOMBOX_ACTION_KIND action_kind);
    // IDA_EA: 0x864E410
    // IDA_NAME: _ZN5CUserC2Ev
    // DEMANGLED: CUser::CUser(void)
    CUser();

    // IDA_EA: 0x864EF9C
    // IDA_NAME: _ZN5CUserD2Ev
    // DEMANGLED: CUser::~CUser()
    ~CUser();

    // IDA_EA: 0x8658030
    // IDA_NAME: _ZN5CUser5resetEv
    // DEMANGLED: CUser::reset(void)
    CUser* reset();

    // IDA_EA: 0x8658910
    // IDA_NAME: _ZN5CUser7log_outEv
    // DEMANGLED: CUser::log_out(void)
    CUser* log_out();

    // IDA_EA: 0x86554AA
    // IDA_NAME: _ZN5CUser12UpdateLogoutEb
    // DEMANGLED: CUser::UpdateLogout(bool)
    void UpdateLogout(bool send_complete_packet);

    // IDA_EA: 0x8647890 / 0x8647FFC.  These are the game-TCP receive
    // boundary: encrypted frame validation followed by DISPATCH_Q handoff.
    // IDA_EA: 0x86493C8.  This is the one accept-side handshake/state reset
    // that precedes NetworkThread epoll registration.
    int InitOnAccept(int fd);
    int Parsing(int received_bytes);
    int Recv();

    // IDA unit: 04_character_select_create/CUser/CharacMoneyLog. CUser keeps
// seven contiguous 0x0c records at expanded +0x799c4 for logout auditing.
    struct CharacMoneyLog {
        // IDA_EA: 0x869714C
        // IDA_NAME: _ZN5CUser14CharacMoneyLogC2Ev
        // DEMANGLED: CUser::CharacMoneyLog::CharacMoneyLog(void)
        CharacMoneyLog();

        // IDA_EA: 0x8697160
        // IDA_NAME: _ZN5CUser14CharacMoneyLog5resetEv
        // DEMANGLED: CUser::CharacMoneyLog::reset(void)
        CharacMoneyLog* reset();

        uint32_t plus_money;
        uint32_t minus_money;
        uint32_t tick;
    };

    // IDA_EA: 0x816EE1E
    // IDA_NAME: _ZN5CUser17getGarenaAuthDataEv
    // DEMANGLED: CUser::getGarenaAuthData(void)
    Taiwan::GarenaAuthData* getGarenaAuthData();

    // IDA_EA: 0x822FD20
    // IDA_NAME: _ZN5CUser15getSecurityCardEv
    // DEMANGLED: CUser::getSecurityCard(void)
    WongWork::CSecurityCard* getSecurityCard();

    // IDA_EA: 0x82300AC
    // IDA_NAME: _ZN5CUser14SetServerGroupE17ENUM_SERVER_GROUP
    // DEMANGLED: CUser::SetServerGroup(ENUM_SERVER_GROUP)
    CUser* SetServerGroup(ENUM_SERVER_GROUP server_group);

    // IDA_EA: 0x82300D0
    // IDA_NAME: _ZNK5CUser7IsStateE8ch_state
    // DEMANGLED: CUser::IsState(ch_state)const
    bool IsState(ch_state state) const;

    // IDA_EA: 0x82604EC
    // IDA_NAME: _ZNK5CUser21getCurCharacQuestRefREv
    // DEMANGLED: CUser::getCurCharacQuestRefR(void)const
    UserQuest* getCurCharacQuestRefR() const;

    // IDA_EA: 0x831D4DE
    // IDA_NAME: _ZN5CUser23setBreakAwayRewardOrderE28ENUM_BREAK_AWAY_REWARD_ORDER
    // DEMANGLED: CUser::setBreakAwayRewardOrder(ENUM_BREAK_AWAY_REWARD_ORDER)
    CUser* setBreakAwayRewardOrder(ENUM_BREAK_AWAY_REWARD_ORDER order);

    // IDA_EA: 0x831D4F0
    // IDA_NAME: _ZN5CUser23getBreakAwayRewardOrderEv
    // DEMANGLED: CUser::getBreakAwayRewardOrder(void)
    int getBreakAwayRewardOrder();

    // IDA_EA: 0x831D4FE
    // IDA_NAME: _ZN5CUser22getBreakAwayAccureCeraEv
    // DEMANGLED: CUser::getBreakAwayAccureCera(void)
    int getBreakAwayAccureCera();

    // IDA_EA: 0x831D50C
    // IDA_NAME: _ZN5CUser22setBreakAwayRewardDataEb
    // DEMANGLED: CUser::setBreakAwayRewardData(bool)
    CUser* setBreakAwayRewardData(bool loaded);

    // IDA_EA: 0x832815A
    // IDA_NAME: _ZN5CUser22ClearChildrensDayEventEv
    // DEMANGLED: CUser::ClearChildrensDayEvent(void)
    void ClearChildrensDayEvent();

    // IDA_EA: 0x83281BE
    // IDA_NAME: _ZN5CUser25SetChildrensDayEventValueEiii
    // DEMANGLED: CUser::SetChildrensDayEventValue(int,int,int)
    void SetChildrensDayEventValue(int index, int item_slot, int item_period);

    // IDA_EA: 0x8328220
    // IDA_NAME: _ZN5CUser30IsOKChildrensDayEventConditionEv
    // DEMANGLED: CUser::IsOKChildrensDayEventCondition(void)
    bool IsOKChildrensDayEventCondition();

    // IDA_EA: 0x8328240
    // IDA_NAME: _ZN5CUser34GetChildrensDayEventTargetItemSlotEi
    // DEMANGLED: CUser::GetChildrensDayEventTargetItemSlot(int)
    int GetChildrensDayEventTargetItemSlot(int index);

    // IDA_EA: 0x8328262
    // IDA_NAME: _ZN5CUser36GetChildrensDayEventTargetItemPeriodEi
    // DEMANGLED: CUser::GetChildrensDayEventTargetItemPeriod(int)
    int GetChildrensDayEventTargetItemPeriod(int index);

    // IDA_EA: 0x84B431E
    // IDA_NAME: _ZN5CUser22SetGameMasterCharacterEP20CGameMasterCharacter
    // DEMANGLED: CUser::SetGameMasterCharacter(CGameMasterCharacter *)
    CUser* SetGameMasterCharacter(CGameMasterCharacter* character);

    // IDA_EA: 0x84B9A82
    // IDA_NAME: _ZN5CUser16setHackUserTimerEv
    // DEMANGLED: CUser::setHackUserTimer(void)
    CUser* setHackUserTimer();

    // IDA_EA: 0x84B9A92
    // IDA_NAME: _ZN5CUser15setHackUserFlagEv
    // DEMANGLED: CUser::setHackUserFlag(void)
    CUser* setHackUserFlag();

    // IDA_EA: 0x84EC60E
    // IDA_NAME: _ZN5CUser31CheckLoadRentalInfoFromExchangeEv
    // DEMANGLED: CUser::CheckLoadRentalInfoFromExchange(void)
    int CheckLoadRentalInfoFromExchange();

    // IDA_EA: 0x84EC65E
    // IDA_NAME: _ZN5CUser19getBreakAwayRewardsEv
    // DEMANGLED: CUser::getBreakAwayRewards(void)
    STBreakAwayRewards* getBreakAwayRewards();

    // IDA_EA: 0x84EC722
    // IDA_NAME: _ZN5CUser31setOnlinePreliminaryServerGroupE17ENUM_SERVER_GROUP
    // DEMANGLED: CUser::setOnlinePreliminaryServerGroup(ENUM_SERVER_GROUP)
    CUser* setOnlinePreliminaryServerGroup(ENUM_SERVER_GROUP server_group);

    // IDA_EA: 0x84EC940
    // IDA_NAME: _ZN5CUser25setTotalHackUserTradeGoldEy
    // DEMANGLED: CUser::setTotalHackUserTradeGold(unsigned long long)
    unsigned long long setTotalHackUserTradeGold(unsigned long long gold);

    // IDA_EA: 0x84ECA54
    // IDA_NAME: _ZN5CUser13SetDailyBadgeEPs
    // DEMANGLED: CUser::SetDailyBadge(short *)
    void SetDailyBadge(short* badges);

    // IDA_EA: 0x84ECA90
    // IDA_NAME: _ZN5CUser11SetSchoolNoEj
    // DEMANGLED: CUser::SetSchoolNo(unsigned int)
    CUser* SetSchoolNo(unsigned int school_no);

    // IDA_EA: 0x84ECAA2
    // IDA_NAME: _ZN5CUser9SetRatingEf
    // DEMANGLED: CUser::SetRating(float)
    CUser* SetRating(float rating);

    // IDA_EA: 0x84ECAB4
    // IDA_NAME: _ZN5CUser13SetSchoolNameEPKc
    // DEMANGLED: CUser::SetSchoolName(char const*)
    std::string& SetSchoolName(const char* school_name);

    // IDA_EA: 0x84ECE20
    // IDA_NAME: _ZN5CUser24setCharacLinkMessageFlagEb
    // DEMANGLED: CUser::setCharacLinkMessageFlag(bool)
    CUser* setCharacLinkMessageFlag(bool enabled);

    // IDA_EA: 0x84ECE3C
    // IDA_NAME: _ZN5CUser14SetGuildDBInfoER17STGuildDBInfoOnly
    // DEMANGLED: CUser::SetGuildDBInfo(STGuildDBInfoOnly &)
    void* SetGuildDBInfo(STGuildDBInfoOnly& info);

    // IDA_EA: 0x865CD0A
    // IDA_NAME: _ZN5CUser17SetGuildInfoProxyER12STGuildProxy
    // DEMANGLED: CUser::SetGuildInfoProxy(STGuildProxy &)
    CUser* SetGuildInfoProxy(STGuildProxy& info);

    // IDA_EA: 0x84ECE64
    // IDA_NAME: _ZN5CUser15ChangeGuildNameEPc
    // DEMANGLED: CUser::ChangeGuildName(char *)
    void* ChangeGuildName(char* guild_name);

    // IDA_EA: 0x84ECEAC
    // IDA_NAME: _ZN5CUser16SetGuildAgitFlagEb
    // DEMANGLED: CUser::SetGuildAgitFlag(bool)
    CUser* SetGuildAgitFlag(bool enabled);

    // IDA_EA: 0x84ECED6
    // IDA_NAME: _ZN5CUser18SetGuildAgitDBInfoER17STGuildAgitDBInfo
    // DEMANGLED: CUser::SetGuildAgitDBInfo(STGuildAgitDBInfo &)
    void* SetGuildAgitDBInfo(STGuildAgitDBInfo& info);

    // IDA_EA: 0x850D522
    // IDA_NAME: _ZN5CUser14getGmQuestFlagEv
    // DEMANGLED: CUser::getGmQuestFlag(void)
    int getGmQuestFlag();

    // IDA_EA: 0x855105E
    // IDA_NAME: _ZN5CUser22GetGameMasterCharacterEv
    // DEMANGLED: CUser::GetGameMasterCharacter(void)
    CGameMasterCharacter* GetGameMasterCharacter();

    // IDA_EA: 0x8568E0C
    // IDA_NAME: _ZN5CUser13GetDailyBadgeEPs
    // DEMANGLED: CUser::GetDailyBadge(short *)
    void GetDailyBadge(short* badges);

    // IDA_EA: 0x8568E48
    // IDA_NAME: _ZN5CUser13GetDailyBadgeE15ENUM_BADGE_TYPE
    // DEMANGLED: CUser::GetDailyBadge(ENUM_BADGE_TYPE)
    int GetDailyBadge(ENUM_BADGE_TYPE badge_type);

    // IDA_EA: 0x8568E5E
    // IDA_NAME: _ZN5CUser13AddDailyBadgeE15ENUM_BADGE_TYPE
    // DEMANGLED: CUser::AddDailyBadge(ENUM_BADGE_TYPE)
    void AddDailyBadge(ENUM_BADGE_TYPE badge_type);

    // IDA_EA: 0x857F406
    // IDA_NAME: _ZN5CUser21set_public_ip_addressEi
    // DEMANGLED: CUser::set_public_ip_address(int)
    CUser* set_public_ip_address(int address);

    // IDA_EA: 0x8595060
    // IDA_NAME: _ZN5CUser10backupSeedEv
    // DEMANGLED: CUser::backupSeed(void)
    CUser* backupSeed();

    // IDA_EA: 0x8595078
    // IDA_NAME: _ZN5CUser10SetPowerUpEb
    // DEMANGLED: CUser::SetPowerUp(bool)
    CUser* SetPowerUp(bool enabled);

    // IDA_EA: 0x85DFA56
    // IDA_NAME: _ZN5CUser30get_guildwar_point_per_pvpplayEv
    // DEMANGLED: CUser::get_guildwar_point_per_pvpplay(void)
    int get_guildwar_point_per_pvpplay();

    // IDA_EA: 0x863BE74
    // IDA_NAME: _ZN5CUser20GetCheckConnIntervalEv
    // DEMANGLED: CUser::GetCheckConnInterval(void)
    int GetCheckConnInterval();

    // IDA_EA: 0x863BE82
    // IDA_NAME: _ZN5CUser21SetResumeChecksumTimeEi
    // DEMANGLED: CUser::SetResumeChecksumTime(int)
    CUser* SetResumeChecksumTime(int resume_time);

    // IDA_EA: 0x863BF46
    // IDA_NAME: _ZN5CUser17incre_check_countEv
    // DEMANGLED: CUser::incre_check_count(void)
    CUser* incre_check_count();

    // IDA_EA: 0x863BF62
    // IDA_NAME: _ZN5CUser17get_connect_countEv
    // DEMANGLED: CUser::get_connect_count(void)
    int get_connect_count();

    // IDA_EA: 0x863BF72
    // IDA_NAME: _ZN5CUser19isSendMailCertifiedEv
    // DEMANGLED: CUser::isSendMailCertified(void)
    int isSendMailCertified();

    // IDA_EA: 0x868A4B6
    // IDA_NAME: _ZN5CUser15set_MailMsgDataEP16MSG_MAILBOX_SEND
    // DEMANGLED: CUser::set_MailMsgData(MSG_MAILBOX_SEND *)
    void* set_MailMsgData(MSG_MAILBOX_SEND* message);

    // IDA_EA: 0x8689F12
    // IDA_NAME: _ZN5CUser8SendMailEb
    // DEMANGLED: CUser::SendMail(bool)
    void* SendMail(bool db_receiver_verified);

    // IDA_EA: 0x868A2DC
    // IDA_NAME: _ZN5CUser13SendMultiMailEb
    // DEMANGLED: CUser::SendMultiMail(bool)
    void* SendMultiMail(bool db_receiver_verified);

    // IDA_EA: 0x868A4E8
    // IDA_NAME: _ZN5CUser20set_MultimailMsgDataEP22MSG_MULTI_MAILBOX_SEND
    // DEMANGLED: CUser::set_MultimailMsgData(MSG_MULTI_MAILBOX_SEND *)
    void* set_MultimailMsgData(MSG_MULTI_MAILBOX_SEND* message);

    // IDA_EA: 0x868A51A
    // IDA_NAME: _ZN5CUser18reqSendMailCertifyEv
    // DEMANGLED: CUser::reqSendMailCertify(void)
    int reqSendMailCertify();

    // IDA_EA: 0x863BF82
    // IDA_NAME: _ZN5CUser23getHumanCertifyTimerKeyEv
    // DEMANGLED: CUser::getHumanCertifyTimerKey(void)
    int getHumanCertifyTimerKey();

    // IDA_EA: 0x863BF90
    // IDA_NAME: _ZN5CUser11IsRecvEventEv
    // DEMANGLED: CUser::IsRecvEvent(void)
    int IsRecvEvent();

    // IDA_EA: 0x863BFA0
    // IDA_NAME: _ZN5CUser13isARSUserKickEv
    // DEMANGLED: CUser::isARSUserKick(void)
    int isARSUserKick();

    // IDA_EA: 0x868F716
    // IDA_NAME: _ZN5CUser16getForceDropFlagEv
    // DEMANGLED: CUser::getForceDropFlag(void)
    int getForceDropFlag();

    // IDA_EA: 0x868995A
    // IDA_NAME: _ZN5CUser18GetTradePunishTypeEv
    // DEMANGLED: CUser::GetTradePunishType(void)
    int GetTradePunishType();

    // IDA_EA: 0x8692A08
    // IDA_NAME: _ZN5CUser13SetBuyingGoldEb
    // DEMANGLED: CUser::SetBuyingGold(bool)
    CUser* SetBuyingGold(bool buying);

    // IDA_EA: 0x8692A24
    // IDA_NAME: _ZN5CUser13GetBuyingGoldEv
    // DEMANGLED: CUser::GetBuyingGold(void)
    int GetBuyingGold();

    // IDA_EA: 0x86928C2
    // IDA_NAME: _ZN5CUser13DoGetGoldItemEiRSt6vectorISt4pairIiiESaIS2_EEbR10ENUM_ERROR
    // DEMANGLED: CUser::DoGetGoldItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,bool,ENUM_ERROR &)
    int DoGetGoldItem(int gold_item_id,
                      std::vector<std::pair<int, int> >& inserted_slots,
                      bool is_received,
                      ENUM_ERROR& error);

    // IDA_EA: 0x8692A34
    // IDA_NAME: _ZN5CUser18DoGetCeraPointItemEjbR10ENUM_ERRORm
    // DEMANGLED: CUser::DoGetCeraPointItem(unsigned int,bool,ENUM_ERROR &,unsigned long)
    int DoGetCeraPointItem(unsigned int package_money,
                           bool is_received,
                           ENUM_ERROR& error,
                           unsigned long cera_item_id);

    // IDA_EA: 0x86903F8 / 0x8690776
    // IDA_NAME: _ZN5CUser17SendTagCharacInfoEv
    //           _ZN5CUser17MakeTagCharacInfoER11PacketGuard
    // The SelectStriker DB continuation emits this complete tag-character
    // packet; both bodies retain the original packet and skill-list ABI.
    void SendTagCharacInfo();
    int MakeTagCharacInfo(PacketGuard& packet);

    // IDA_EA: 0x8692AF6
    // IDA_NAME: _ZN5CUser12GetCeraPointEv
    // DEMANGLED: CUser::GetCeraPoint(void)
    int GetCeraPoint();

    // IDA_EA: 0x80DA2FE
    // IDA_NAME: _ZN5CUser12CheckInTradeEv
    // DEMANGLED: CUser::CheckInTrade(void)
    short CheckInTrade();

    // IDA_EA: 0x86810A8
    // IDA_NAME: _ZN5CUser26isEnableAvatarSocketActionEv
    // DEMANGLED: CUser::isEnableAvatarSocketAction(void)
    // Returns 0 when avatar-socket actions may proceed; otherwise returns
    // the original security/state error code.
    int isEnableAvatarSocketAction();

    // IDA_EA: 0x8681160
    // IDA_NAME: _ZN5CUser21isEnableGiveGiftToNPCEv
    // DEMANGLED: CUser::isEnableGiveGiftToNPC(void)
    int isEnableGiveGiftToNPC();

    // IDA_EA: 0x867A95C
    // IDA_NAME: _ZN5CUser14SetCharacLevelEi
    // DEMANGLED: CUser::SetCharacLevel(int)
    int SetCharacLevel(int level);

    // IDA_EA: 0x867B6D4
    // IDA_NAME: _ZN5CUser7AddItemEii14eItemAddReasonR14ENUM_ITEMSPACEi
    int AddItem(int item_id,
                int count,
                eItemAddReason reason,
                ENUM_ITEMSPACE& item_space,
                int upgrade);

    // The old CParty::ClearDungeon and WarRoom::HandleMonsterKill bodies
    // consume and clear this expanded CUser +0x8e3dc accumulator when a map/room
    // ends.  It has no standalone legacy member symbol, so keep the direct
    // field access in the CUser owner rather than duplicating it in callers.
    int takePendingCreatureExp();

    // IDA_EA: 0x867B048
    // IDA_NAME: _ZN5CUser17ChangeGrowType_GMEii
    // DEMANGLED: CUser::ChangeGrowType_GM(int,int)
    int ChangeGrowType_GM(int first_value, int second_value);

    // IDA_EA: 0x864F930
    // IDA_NAME: _ZN5CUser20reset_tag_info_to_meEj
    // DEMANGLED: CUser::reset_tag_info_to_me(unsigned int)
    uint8_t reset_tag_info_to_me(uint32_t charac_no);

    // IDA_EA: 0x8678526
    // IDA_NAME: _ZN5CUser13CheckMoveTownEi
    // DEMANGLED: CUser::CheckMoveTown(int)
    int CheckMoveTown(int village_index);

    // IDA_EA: 0x8697526
    // IDA_NAME: _ZN5CUser18ClearItemSellCountEv
    // DEMANGLED: CUser::ClearItemSellCount(void)
    void ClearItemSellCount();

    // IDA_EA: 0x868B106
    // IDA_NAME: _ZN5CUser25getInDungeonItemSellCountEii
    // DEMANGLED: CUser::getInDungeonItemSellCount(int,int)
    int getInDungeonItemSellCount(int item_space, int slot);

    // IDA_EA: 0x80DD584
    // IDA_NAME: _ZNK5CUser19GetCharacExpandDataE23ENUM_CHARAC_EXPAND_TYPE
    // DEMANGLED: CUser::GetCharacExpandData(ENUM_CHARAC_EXPAND_TYPE)const
    charac_expand::CData* GetCharacExpandData(
        ENUM_CHARAC_EXPAND_TYPE expand_type) const;

    // IDA_EA: 0x828B5DE
    // IDA_NAME: _ZNK5CUser20GetCharacExpandDataRE23ENUM_CHARAC_EXPAND_TYPE
    // DEMANGLED: CUser::GetCharacExpandDataR(ENUM_CHARAC_EXPAND_TYPE)const
    charac_expand::CData* GetCharacExpandDataR(
        ENUM_CHARAC_EXPAND_TYPE expand_type) const;

    // IDA_EA: 0x822FC4E
    // IDA_NAME: _ZN5CUser16GetGameOptionRefEv
    // DEMANGLED: CUser::GetGameOptionRef(void)
    CGameOption* GetGameOptionRef();

    // Narrow typed boundary for the direct CCharacterView child already
    // present at expanded CUser+0x079a44.  It avoids raw offsets in packet
    // business code and does not alter the CUser object layout.
    CCharacterView& character_view() { return character_view_; }

    void send_charac_visible_data();
    void ReCalcChattingEmoticon();
    void SendChattingEmoticon();

    // IDA_EA: 0x8230164
    // IDA_NAME: _ZN5CUser14GetGuildDBInfoEv
    // DEMANGLED: CUser::GetGuildDBInfo(void)
    STGuildDBInfoOnly* GetGuildDBInfo();

    // IDA_EA: 0x82301AE
    // IDA_NAME: _ZN5CUser21GetGuildPowerWarPointEv
    // DEMANGLED: CUser::GetGuildPowerWarPoint(void)
    int GetGuildPowerWarPoint();

    // IDA_EA: 0x82301D8
    // IDA_NAME: _ZN5CUser16IsExistGuildAgitEv
    // DEMANGLED: CUser::IsExistGuildAgit(void)
    bool IsExistGuildAgit();

    // IDA_EA: 0x82F0E70
    // IDA_NAME: _ZN5CUser12GetUserStateEv
    // DEMANGLED: CUser::GetUserState(void)
    int GetUserState();

    // IDA_EA: 0x85BFD66
    // IDA_NAME: _ZN5CUser12IsReturnUserEv
    // DEMANGLED: CUser::IsReturnUser(void)
    int IsReturnUser();

    // IDA_EA: 0x869742A
    // IDA_NAME: _ZN5CUser12GetGuildNameEv
    // DEMANGLED: CUser::GetGuildName(void)
    char* GetGuildName();

    // IDA_EA: 0x8697438
    // IDA_NAME: _ZN5CUser13GetGuildLevelEv
    // DEMANGLED: CUser::GetGuildLevel(void)
    int GetGuildLevel();

    // IDA_EA: 0x812CE28
    // IDA_NAME: _ZNK5CUser14GetPremiumInfoEv
    // DEMANGLED: CUser::GetPremiumInfo(void)const
    WongWork::CUserPremium* GetPremiumInfo() const;

    // IDA_EA: 0x863BE94
    // IDA_NAME: _ZN5CUser15GetPremiumInfoWEv
    // DEMANGLED: CUser::GetPremiumInfoW(void)
    WongWork::CUserPremium* GetPremiumInfoW();

    // IDA_EA: 0x812CE36
    // IDA_NAME: _ZNK5CUser22IsHavePremiumAdvantageEv
    // DEMANGLED: CUser::IsHavePremiumAdvantage(void)const
    bool IsHavePremiumAdvantage() const;

    // IDA_EA: 0x864D878
    // IDA_NAME: _ZN5CUser17IsRestrictedGoodsEi
    // DEMANGLED: CUser::IsRestrictedGoods(int)
    bool IsRestrictedGoods(int item_id);

    // IDA_EA: 0x869724A
    // IDA_NAME: _ZN5CUser21ResetCharacExpandDataEv
    // DEMANGLED: CUser::ResetCharacExpandData(void)
    bool ResetCharacExpandData();

    // IDA_EA: 0x864870E
    // IDA_NAME: _ZN5CUser29SendClientCrashDownLogToLogDBEi
    // DEMANGLED: CUser::SendClientCrashDownLogToLogDB(int)
    int SendClientCrashDownLogToLogDB(int src);

    // IDA_EA: 0x864D84E
    // IDA_NAME: _ZN5CUser18SetRestrictedGoodsEi
    // DEMANGLED: CUser::SetRestrictedGoods(int)
    void SetRestrictedGoods(int item_id);

    // IDA_EA: 0x864D9A4
    // IDA_NAME: _ZN5CUser19SendRestrictedGoodsEbi
    // DEMANGLED: CUser::SendRestrictedGoods(bool,int)
    void SendRestrictedGoods(bool immediate, int item_id);

    // IDA_EA: 0x864FCC6
    // IDA_NAME: _ZN5CUser11UseWinPointEi12eWPSubReason
    // DEMANGLED: CUser::UseWinPoint(int,eWPSubReason)
    int UseWinPoint(int point, eWPSubReason reason);

    // IDA_EA: 0x864FD2C
    // IDA_NAME: _ZN5CUser12gainWinPointEi12eWPAddReason
    // DEMANGLED: CUser::gainWinPoint(int,eWPAddReason)
    void gainWinPoint(int point, eWPAddReason reason);

    // IDA_EA: 0x864FC34
    // IDA_NAME: _ZN5CUser21gainPowerWarRewardExpEi
    // DEMANGLED: CUser::gainPowerWarRewardExp(int)
    int gainPowerWarRewardExp(int unused);

    // IDA_EA: 0x822FD2E
    // IDA_NAME: _ZN5CUser6getPadEv
    // DEMANGLED: CUser::getPad(void)
    Sanicova::CPad* getPad();

    // IDA_EA: 0x868DE0A
    // IDA_NAME: _ZN5CUser22UpdateAuraAvatarOptionEii
    // DEMANGLED: CUser::UpdateAuraAvatarOption(int,int)
    void UpdateAuraAvatarOption(int option_type, int value);

    // IDA_EA: 0x868DE56
    // IDA_NAME: _ZN5CUser28get_aura_avatar_option_valueEi
    // DEMANGLED: CUser::get_aura_avatar_option_value(int)
    int get_aura_avatar_option_value(int option_type);

    // IDA_EA: 0x868DFF8
    // IDA_NAME: _ZN5CUser20is_equip_aura_avatarEcRi
    // DEMANGLED: CUser::is_equip_aura_avatar(char,int &)
    int is_equip_aura_avatar(char option_type, int& value);

    // IDA_EA: 0x80DA38C
    // IDA_NAME: _ZN5CUser9get_stateEv
    // DEMANGLED: CUser::get_state(void)
    int get_state();

    // IDA_EA: 0x86804CE
    // IDA_NAME: _ZN5CUser12OnDisconnectEv
    // DEMANGLED: CUser::OnDisconnect(void)
    char OnDisconnect();

    // IDA_EA: 0x867EDB2
    // IDA_NAME: _ZN5CUser9set_stateE8ch_state
    // DEMANGLED: CUser::set_state(ch_state)
    char set_state(int state);

    // IDA_EA: 0x84EC99C
    // IDA_NAME: _ZN5CUser14setHangameUserEb
    // DEMANGLED: CUser::setHangameUser(bool)
    CUser* setHangameUser(bool hangame);

    // IDA_EA: 0x84EC9B8
    // IDA_NAME: _ZN5CUser13isHangameUserEv
    // DEMANGLED: CUser::isHangameUser(void)
    int isHangameUser();

    // IDA_EA: 0x86796DC
    // IDA_NAME: _ZN5CUser17SetGameMasterModeEb
    // DEMANGLED: CUser::SetGameMasterMode(bool)
    int SetGameMasterMode(bool enabled);

    // IDA_EA: 0x833F4BC
    // IDA_NAME: _ZN5CUser22get_cera_item_order_noEv
    // DEMANGLED: CUser::get_cera_item_order_no(void)
    int get_cera_item_order_no();

    // IDA_EA: 0x867CB90
    // IDA_NAME: _ZN5CUser23getDungeonIdxAfterClearEv
    // DEMANGLED: CUser::getDungeonIdxAfterClear(void)
    int getDungeonIdxAfterClear();

    // IDA_EA: 0x80E600E
    // IDA_NAME: _ZNK5CUser17isAffectedPremiumE17ENUM_PREMIUM_TYPE
    // DEMANGLED: CUser::isAffectedPremium(ENUM_PREMIUM_TYPE)const
    int isAffectedPremium(ENUM_PREMIUM_TYPE premium_type) const;
    // IDA_EA: 0x85BFD76
    // IDA_NAME: _ZN5CUser24getAddExpRateFromPremiumE17ENUM_PREMIUM_TYPE
    // DEMANGLED: CUser::getAddExpRateFromPremium(ENUM_PREMIUM_TYPE)
    int getAddExpRateFromPremium(ENUM_PREMIUM_TYPE premium_type);

    // IDA_EA: 0x830EDAA
    // IDA_NAME: _ZN5CUser36getAddIndependentDropRateFromPremiumE17ENUM_PREMIUM_TYPEj
    // DEMANGLED: CUser::getAddIndependentDropRateFromPremium(
    //     ENUM_PREMIUM_TYPE,unsigned int)
    int getAddIndependentDropRateFromPremium(
        ENUM_PREMIUM_TYPE premium_type,
        unsigned int drop_slot);

    // IDA_EA: 0x869733E
    // IDA_NAME: _ZN5CUser34getAddQuestItemDropRateFromPremiumE17ENUM_PREMIUM_TYPE
    // DEMANGLED: CUser::getAddQuestItemDropRateFromPremium(
    //             ENUM_PREMIUM_TYPE)
    int getAddQuestItemDropRateFromPremium(
        ENUM_PREMIUM_TYPE premium_type);

    // IDA_EA: 0x8110DC2
    // IDA_NAME: _ZN5CUser11GetPCRoomNoEv
    // DEMANGLED: CUser::GetPCRoomNo(void)
    int GetPCRoomNo();

    // IDA_EA: 0x84ECB00
    // IDA_NAME: _ZNK5CUser12IsPCRoomUserEv
    // DEMANGLED: CUser::IsPCRoomUser(void)const
    // The original login-reply leaf returns zero unconditionally; it must
    // remain distinct from GetPCRoomNo(), whose nonzero policy differs.
    int IsPCRoomUser() const;

    // IDA_EA: 0x80FDF7A
    // IDA_NAME: _ZN5CUser7GetCeraEv
    // DEMANGLED: CUser::GetCera(void)
    int GetCera();

    // IDA_EA: 0x8101072
    // IDA_NAME: _ZNK5CUser12get_acc_nameEv
    // DEMANGLED: CUser::get_acc_name(void)const
    char* get_acc_name() const;

    // IDA_EA: 0x80C8C96
    // IDA_NAME: _ZNK5CUser6GetUIDEv
    // DEMANGLED: CUser::GetUID(void)const
    int GetUID() const;

    // IDA_EA: 0x80DA36E
    // IDA_NAME: _ZNK5CUser10get_acc_idEv
    // DEMANGLED: CUser::get_acc_id(void)const
    int get_acc_id() const;

    // IDA_EA: 0x8692B84
    // IDA_NAME: _ZN5CUser20SendAradEventItemLogEmi
    // DEMANGLED: CUser::SendAradEventItemLog(unsigned long,int)
    void SendAradEventItemLog(unsigned long item_index, int count);

    // IDA_EA: 0x8689D74
    // IDA_NAME: _ZN5CUser19CalLevelUpItemStateERcRiii
    // DEMANGLED: CUser::CalLevelUpItemState(char &,int &,int,int)
    int CalLevelUpItemState(char& level_up_item_state,
                            int& remaining_exp,
                            int minimum_level,
                            int maximum_level);

    // IDA_EA: 0x86657FC
    // IDA_NAME: _ZN5CUser15increase_statusEs
    // DEMANGLED: CUser::increase_status(short)
    void increase_status(short slot);

    // IDA_EA: 0x867C8FE
    // IDA_NAME: _ZN5CUser10LogHistoryEPKcz
    // DEMANGLED: CUser::LogHistory(char const*,...)
    int LogHistory(const char* format, ...);

    // IDA_EA: 0x867CD20
    // IDA_NAME: _ZN5CUser21_UpdateUserInfo4GuildEv
    // DEMANGLED: CUser::_UpdateUserInfo4Guild(void)
    void _UpdateUserInfo4Guild();

    // IDA_EA: 0x8679CEE
    // IDA_NAME: _ZN5CUser26makeGuildItemUpgradMessageEPKciib
    // DEMANGLED: CUser::makeGuildItemUpgradMessage(char const*,int,int,bool)
    void makeGuildItemUpgradMessage(const char* item_name,
                                    int old_upgrade,
                                    int new_upgrade,
                                    bool succeeded);

    // IDA_EA: 0x8679754
    // IDA_NAME: _ZN5CUser23makeGuildLevelUpMessageEi
    // DEMANGLED: CUser::makeGuildLevelUpMessage(int)
    void makeGuildLevelUpMessage(int charac_level);

    // IDA_EA: 0x8679A36
    // IDA_NAME: _ZN5CUser21makeGuildSkillMessageEPKci
    // DEMANGLED: CUser::makeGuildSkillMessage(char const*,int)
    void makeGuildSkillMessage(const char* skill_name, int skill_level);

    // IDA_EA: 0x867A242
    // IDA_NAME: _ZN5CUser34makeGuildItemSeparateUpgradMessageEPKciibi
    // DEMANGLED: CUser::makeGuildItemSeparateUpgradMessage(char const*,int,int,bool,int)
    void makeGuildItemSeparateUpgradMessage(const char* item_name,
                                            int old_separate_upgrade,
                                            int new_separate_upgrade,
                                            bool succeeded,
                                            int normal_upgrade);

    // IDA_EA: 0x86931C4
    // IDA_NAME: _ZN5CUser28RewardItem2DeleteInvalidItemERKSsRKSt6vectorISt4pairIiiESaIS4_EE
    // DEMANGLED: CUser::RewardItem2DeleteInvalidItem(std::string const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    void RewardItem2DeleteInvalidItem(
        const std::string& source,
        const std::vector<std::pair<int, int> >& invalid_items);

    // IDA_EA: 0x867CF0C
    // IDA_NAME: _ZN5CUser14_isMatchedItemEiRKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::_isMatchedItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    int _isMatchedItem(int item_id,
                       const std::vector<std::pair<int, int> >& matched_items);

    // IDA_EA: 0x867D0BE
    // IDA_NAME: _ZN5CUser24_deleteSpecificItemInvenE10INVEN_TYPERKSt6vectorISt4pairIiiESaIS3_EERS5_ii
    // DEMANGLED: CUser::_deleteSpecificItemInven(INVEN_TYPE,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&,int,int)
    void _deleteSpecificItemInven(
        INVEN_TYPE inventory_type,
        const std::vector<std::pair<int, int> >& matched_items,
        std::vector<std::pair<int, int> >& changed_slots,
        int begin_slot,
        int end_slot);

    // IDA_EA: 0x867D3EE
    // IDA_NAME: _ZN5CUser24_deleteSpecificItemCargoERKSt6vectorISt4pairIiiESaIS2_EERS4_
    // DEMANGLED: CUser::_deleteSpecificItemCargo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
    void _deleteSpecificItemCargo(
        const std::vector<std::pair<int, int> >& matched_items,
        std::vector<std::pair<int, int> >& changed_slots);

    // IDA_EA: 0x867D69C
    // IDA_NAME: _ZN5CUser25_deleteSpecificRedeemItemERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::_deleteSpecificRedeemItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    int _deleteSpecificRedeemItem(
        const std::vector<std::pair<int, int> >& matched_items);

    // IDA_EA: 0x867D8A4
    // IDA_NAME: _ZN5CUser31_deleteSpecificItemAccountCargoERKSt6vectorISt4pairIiiESaIS2_EERS4_
    // DEMANGLED: CUser::_deleteSpecificItemAccountCargo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
    void _deleteSpecificItemAccountCargo(
        const std::vector<std::pair<int, int> >& matched_items,
        std::vector<std::pair<int, int> >& changed_slots);

    // IDA_EA: 0x867DB66
    // IDA_NAME: _ZN5CUser28_deleteSpecificItemTitieBookERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::_deleteSpecificItemTitieBook(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    void _deleteSpecificItemTitieBook(
        const std::vector<std::pair<int, int> >& matched_items);

    // IDA_EA: 0x867DBA0
    // IDA_NAME: _ZN5CUser18deleteSpecificItemERKSt6vectorISt4pairIiiESaIS2_EERS4_
    // DEMANGLED: CUser::deleteSpecificItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
    char deleteSpecificItem(
        const std::vector<std::pair<int, int> >& matched_items,
        std::vector<std::pair<int, int> >& changed_slots);

    // IDA_EA: 0x867E734
    // IDA_NAME: _ZN5CUser24deleteDailyStackableItemERSt6vectorISt4pairIiiESaIS2_EEii
    // DEMANGLED: CUser::deleteDailyStackableItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int,int)
    bool deleteDailyStackableItem(
        std::vector<std::pair<int, int> >& changed_slots,
        int begin_slot,
        int end_slot);

    // IDA_EA: 0x8689010
    // IDA_NAME: _ZN5CUser17ResetItemByScriptERSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::ResetItemByScript(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
    bool ResetItemByScript(std::vector<std::pair<int, int> >& reset_items);

    // IDA_EA: 0x86890BE
    // IDA_NAME: _ZN5CUser18RefillItemByScriptERSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::RefillItemByScript(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
    bool RefillItemByScript(std::vector<std::pair<int, int> >& changed_slots);

    // IDA_EA: 0x8656CAA
    // IDA_NAME: _ZN5CUser12AddDailyItemEv
    // DEMANGLED: CUser::AddDailyItem(void)
    void AddDailyItem();

    // IDA_EA: 0x8657450
    // IDA_NAME: _ZN5CUser14resetDailyDataEv
    // DEMANGLED: CUser::resetDailyData(void)
    void resetDailyData();

    // IDA_EA: 0x865784E
    // IDA_NAME: _ZN5CUser15recover_staminaEv
    // DEMANGLED: CUser::recover_stamina(void)
    int recover_stamina();

    // IDA_EA: 0x867E092
    // IDA_NAME: _ZN5CUser19processDelDailyItemEv
    // DEMANGLED: CUser::processDelDailyItem(void)
    void processDelDailyItem();

    // IDA_EA: 0x867E898
    // IDA_NAME: _ZN5CUser18deleteGuildExpBookEv
    // DEMANGLED: CUser::deleteGuildExpBook(void)
    void deleteGuildExpBook();

    // IDA_EA: 0x80DA37C
    // IDA_NAME: _ZNK5CUser13get_unique_idEv
    // DEMANGLED: CUser::get_unique_id(void)const
    int get_unique_id() const;

    // IDA_EA: 0x84B9AA2
    // IDA_NAME: _ZN5CUser11getSecuTypeEv
    // DEMANGLED: CUser::getSecuType(void)
    int getSecuType();

    // IDA_EA: 0x822FCE4
    // IDA_NAME: _ZN5CUser13getAntibotKeyEv
    // DEMANGLED: CUser::getAntibotKey(void)
    int getAntibotKey();

    // IDA_EA: 0x84EC87A
    // IDA_NAME: _ZN5CUser13setAntibotKeyEi
    // DEMANGLED: CUser::setAntibotKey(int)
    CUser* setAntibotKey(int key);

    // IDA_EA: 0x8275676
    // IDA_NAME: _ZN5CUser18GetPISenderManagerEv
    // DEMANGLED: CUser::GetPISenderManager(void)
    PISenderManager* GetPISenderManager();

    // IDA_EA: 0x80FDF88
    // IDA_NAME: _ZN5CUser6GetAgeEv
    // DEMANGLED: CUser::GetAge(void)
    int GetAge();

    // IDA_EA: 0x865C65C
    // IDA_NAME: _ZN5CUser24IsPermissionPrivateStoreEv
    // DEMANGLED: CUser::IsPermissionPrivateStore(void)
    int IsPermissionPrivateStore();

    // IDA_EA: 0x80F9968
    // IDA_NAME: _ZN5CUser6SetETCEsi
    // DEMANGLED: CUser::SetETC(short,int)
    int SetETC(short key, int value);

    // IDA_EA: 0x84EC850
    // IDA_NAME: _ZN5CUser24SetVillageAttackedRewardEb
    // DEMANGLED: CUser::SetVillageAttackedReward(bool)
    CUser* SetVillageAttackedReward(bool reward);

    // IDA_EA: 0x822FE3E
    // IDA_NAME: _ZN5CUser13GetTradeSpaceEv
    // DEMANGLED: CUser::GetTradeSpace(void)
    int GetTradeSpace();

    // IDA_EA: 0x85320B4
    // IDA_NAME: _ZN5CUser13SetTradeSpaceEi
    // DEMANGLED: CUser::SetTradeSpace(int)
    int SetTradeSpace(int trade_space);

    // IDA_EA: 0x822FD3C
    // IDA_NAME: _ZNK5CUser10isHackUserEv
    // DEMANGLED: CUser::isHackUser(void)const
    int isHackUser() const;

    // IDA_EA: 0x822FD4C
    // IDA_NAME: _ZNK5CUser15isHackUserTimerEv
    // DEMANGLED: CUser::isHackUserTimer(void)const
    int isHackUserTimer() const;

    // IDA_EA: 0x8532078
    // IDA_NAME: _ZNK5CUser25getTotalHackUserTradeGoldEv
    // DEMANGLED: CUser::getTotalHackUserTradeGold(void)const
    unsigned long long getTotalHackUserTradeGold() const;

    // IDA_EA: 0x853208C
    // IDA_NAME: _ZNK5CUser24getTotalHackUserTradeCntEv
    // DEMANGLED: CUser::getTotalHackUserTradeCnt(void)const
    int getTotalHackUserTradeCnt() const;

    // IDA_EA: 0x864658A
    // IDA_NAME: _ZN5CUser18IncrementTradeGoldEji
    // DEMANGLED: CUser::IncrementTradeGold(unsigned int,int)
    char IncrementTradeGold(unsigned int gold, int count_over_trade_gold);

    // IDA_EA: 0x853209A
    // IDA_NAME: _ZN5CUser13incTradeCountEv
    // DEMANGLED: CUser::incTradeCount(void)
    CUser* incTradeCount();

    // IDA_EA: 0x867FBBA
    // IDA_NAME: _ZN5CUser21gainMoneyFromHackUserEjyjy
    // DEMANGLED: CUser::gainMoneyFromHackUser(unsigned int,unsigned long long,unsigned int,unsigned long long)
    void gainMoneyFromHackUser(unsigned int acc_id,
                               unsigned long long trade_gold,
                               unsigned int trade_count,
                               unsigned long long other_trade_gold);

    // IDA_EA: 0x867F748
    // IDA_NAME: _ZN5CUser11setHackUserEjjjjtj
    // DEMANGLED: CUser::setHackUser(unsigned int,unsigned int,unsigned int,unsigned int,unsigned short,unsigned int)
    void setHackUser(unsigned int hack_type,
                     unsigned int accumulated_count,
                     unsigned int hack_sub_type,
                     unsigned int sub_accumulated_count,
                     unsigned short apply_flag,
                     unsigned int etc_low);

    // IDA_EA: 0x8681368
    // IDA_NAME: _ZN5CUser12OnPunishUserEv
    // DEMANGLED: CUser::OnPunishUser(void)
    char OnPunishUser();

    // IDA_EA: 0x8681312
    // IDA_NAME: _ZN5CUser15OnRestrictTradeEv
    // DEMANGLED: CUser::OnRestrictTrade(void)
    char OnRestrictTrade();

    // IDA_EA: 0x867FD0A
    // IDA_NAME: _ZN5CUser17onUpdateHackCountEN8WongWork13ENUM_HACKTYPEEijj
    // DEMANGLED: CUser::onUpdateHackCount(WongWork::ENUM_HACKTYPE,int,unsigned int,unsigned int)
    int onUpdateHackCount(WongWork::ENUM_HACKTYPE hack_type,
                          int count,
                          unsigned int value1,
                          unsigned int value2);

    // IDA_EA: 0x8689890
    // IDA_NAME: _ZN5CUser18SetTradePunishTypeEi
    // DEMANGLED: CUser::SetTradePunishType(int)
    CUser* SetTradePunishType(int punish_type);

    // IDA_EA: 0x868991C
    // IDA_NAME: _ZN5CUser17isTradePunishTypeEi
    // DEMANGLED: CUser::isTradePunishType(int)
    int isTradePunishType(int punish_type);

    // IDA_EA: 0x86972B6
    // IDA_NAME: _ZN5CUser25incTotalHackUserTradeGoldEy
    // DEMANGLED: CUser::incTotalHackUserTradeGold(unsigned long long)
    unsigned long long incTotalHackUserTradeGold(unsigned long long amount);

    // IDA_EA: 0x86972EE
    // IDA_NAME: _ZN5CUser24incTotalHackUserTradeCntEj
    // DEMANGLED: CUser::incTotalHackUserTradeCnt(unsigned int)
    CUser* incTotalHackUserTradeCnt(unsigned int count);

    // IDA_EA: 0x8697448
    // IDA_NAME: _ZN5CUser17isLoadingHackGoldEv
    // DEMANGLED: CUser::isLoadingHackGold(void)
    int isLoadingHackGold();

    // IDA_EA: 0x8697458
    // IDA_NAME: _ZN5CUser15setLoadHackGoldEb
    // DEMANGLED: CUser::setLoadHackGold(bool)
    CUser* setLoadHackGold(bool loading);

    // IDA_EA: 0x8649BDC
    // IDA_NAME: _ZN5CUser23isDungeonClearHackCheckEv
    // DEMANGLED: CUser::isDungeonClearHackCheck(void)
    int isDungeonClearHackCheck();

    // IDA_EA: 0x86802AA
    // IDA_NAME: _ZN5CUser14onDungeonClearEb
    // DEMANGLED: CUser::onDungeonClear(bool)
    int onDungeonClear(bool cleared);

    // IDA_EA: 0x8649B96
    // IDA_NAME: _ZN5CUser24setDungeonClearHackCheckEb
    // DEMANGLED: CUser::setDungeonClearHackCheck(bool)
    CUser* setDungeonClearHackCheck(bool cleared);

    // IDA_EA: 0x8693198
    // IDA_NAME: _ZN5CUser23SetEventCreateDnfRewardEb
    // DEMANGLED: CUser::SetEventCreateDnfReward(bool)
    CUser* SetEventCreateDnfReward(bool received);

    // IDA_EA: 0x86931B4
    // IDA_NAME: _ZN5CUser23GetEventCreateDnfRewardEv
    // DEMANGLED: CUser::GetEventCreateDnfReward(void)
    int GetEventCreateDnfReward();

    // IDA_EA: 0x86485BA
    // IDA_NAME: _ZN5CUser4SendER11PacketGuard
    // DEMANGLED: CUser::Send(PacketGuard &)
    int Send(PacketGuard& packet);

    // IDA_EA: 0x86886CE
    // IDA_NAME: _ZN5CUser21SendNotiPacketMessageEPKc17ENUM_MESSAGE_TYPE
    // DEMANGLED: CUser::SendNotiPacketMessage(char const*,ENUM_MESSAGE_TYPE)
    void SendNotiPacketMessage(const char* text, ENUM_MESSAGE_TYPE type);

    // IDA_EA: 0x86475D8
    // IDA_NAME: _ZN5CUser14RewardTutorialEj
    // DEMANGLED: CUser::RewardTutorial(unsigned int)
    void RewardTutorial(unsigned int tutorial_id);

    // IDA_EA: 0x867BEA0
    // IDA_NAME: _ZN5CUser15SendCmdOkPacketE14ENUM_CMDPACKET
    // DEMANGLED: CUser::SendCmdOkPacket(ENUM_CMDPACKET)
    bool SendCmdOkPacket(ENUM_CMDPACKET cmd);

    // IDA_EA: 0x867BF42
    // IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKETh
    // DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char)
    bool SendCmdErrorPacket(ENUM_CMDPACKET cmd, unsigned char error);

    // IDA_EA: 0x867BFAE
    // IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKEThR11PacketGuard
    // DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char,PacketGuard &)
    bool SendCmdErrorPacket(ENUM_CMDPACKET cmd,
                            unsigned char error,
                            PacketGuard& packet);

    // IDA_EA: 0x867C022
    // IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKETh8PAY_TYPE
    // DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char,PAY_TYPE)
    bool SendCmdErrorPacket(ENUM_CMDPACKET cmd,
                            unsigned char error,
                            PAY_TYPE payment_type);

    // IDA_EA: 0x8692824
    // IDA_NAME: _ZN5CUser21SendCmdErrorPacketJpnE14ENUM_CMDPACKETt
    // DEMANGLED: CUser::SendCmdErrorPacketJpn(ENUM_CMDPACKET,unsigned short)
    int SendCmdErrorPacketJpn(ENUM_CMDPACKET cmd, unsigned short error);

    // IDA_EA: 0x867C1D4
    // IDA_NAME: _ZN5CUser13SendCmdPacketENS_11eSendTargetE14ENUM_CMDPACKETi
    // DEMANGLED: CUser::SendCmdPacket(CUser::eSendTarget,ENUM_CMDPACKET,int)
    int SendCmdPacket(eSendTarget target,
                      ENUM_CMDPACKET cmd,
                      int value);

    // IDA_EA: 0x8680A96
    // IDA_NAME: _ZN5CUser15AuctionPreCheckE14ENUM_CMDPACKET8PAY_TYPE
    // DEMANGLED: CUser::AuctionPreCheck(ENUM_CMDPACKET,PAY_TYPE)
    int AuctionPreCheck(ENUM_CMDPACKET cmd, PAY_TYPE payment_type);

    // IDA_EA: 0x865A44E
    // IDA_NAME: _ZN5CUser15make_basic_infoEPcc
    // DEMANGLED: CUser::make_basic_info(char *,char)
    int make_basic_info(PacketGuard& packet, char mode);

    // IDA_EA: 0x8659FAA
    // IDA_NAME: _ZN5CUser16make_charac_statER11PacketGuard
    // DEMANGLED: CUser::make_charac_stat(PacketGuard &)
    int make_charac_stat(PacketGuard& packet);

    // IDA_EA: 0x868FE02
    // IDA_NAME: _ZN5CUser14GetManageLevelEv
    // DEMANGLED: CUser::GetManageLevel(void)
    int GetManageLevel();

    // IDA_EA: 0x868FE10
    // IDA_NAME: _ZN5CUser14GetManagePointEv
    // DEMANGLED: CUser::GetManagePoint(void)
    int GetManagePoint();

    // IDA_EA: 0x868FE2C
    // IDA_NAME: _ZN5CUser14SetManageLevelEi
    // DEMANGLED: CUser::SetManageLevel(int)
    void SetManageLevel(int level);

    // IDA_EA: 0x868FE72
    // IDA_NAME: _ZN5CUser14SetManagePointEi
    // DEMANGLED: CUser::SetManagePoint(int)
    void SetManagePoint(int point);

    // IDA_EA: 0x868FEC8
    // IDA_NAME: _ZN5CUser15SetUserMaxLevelEi
    // DEMANGLED: CUser::SetUserMaxLevel(int)
    void SetUserMaxLevel(int level);

    // IDA_EA: 0x868FF04
    // IDA_NAME: _ZN5CUser21CalcurateUserMaxLevelEv
    // DEMANGLED: CUser::CalcurateUserMaxLevel(void)
    void CalcurateUserMaxLevel();

    // IDA_EA: 0x868FFA0
    // IDA_NAME: _ZN5CUser20CalcurateManagePointEv
    // DEMANGLED: CUser::CalcurateManagePoint(void)
    void CalcurateManagePoint();

    // IDA_EA: 0x8690026
    // IDA_NAME: _ZN5CUser20CalcurateManageLevelEv
    // DEMANGLED: CUser::CalcurateManageLevel(void)
    bool CalcurateManageLevel();

    // IDA_EA: 0x868FE1E
    // IDA_NAME: _ZN5CUser15GetUserMaxLevelEv
    // DEMANGLED: CUser::GetUserMaxLevel(void)
    int GetUserMaxLevel();

    // IDA_EA: 0x8651322
    // IDA_NAME: _ZN5CUser27updateCharacViewVisibleDataERK12_Charac_infoR17CHARAC_LOGIN_DATA
    // DEMANGLED: CUser::updateCharacViewVisibleData(_Charac_info const&, CHARAC_LOGIN_DATA &)
    char updateCharacViewVisibleData(const _Charac_info& character,
                                     CHARAC_LOGIN_DATA& out);

    // IDA_EA: 0x8651740
    // IDA_NAME: _ZN5CUser16UpdateCharacViewEv
    // DEMANGLED: CUser::UpdateCharacView(void)
    void UpdateCharacView();

    // IDA_EA: 0x8656808
    // IDA_NAME: _ZN5CUser8SendCoinEv
    // DEMANGLED: CUser::SendCoin(void)
    void SendCoin();

    // IDA_EA: 0x86568FC
    // IDA_NAME: _ZN5CUser12SendOpenflagEv
    // DEMANGLED: CUser::SendOpenflag(void)
    void SendOpenflag();

    // IDA_EA: 0x8681820 / 0x86818FC
    // IDA_NAME: _ZN5CUser27SendProperDungeonClearCountEv /
    //           _ZN5CUser22SendConditionEventInfoEv
    // DEMANGLED: CUser::SendProperDungeonClearCount(void) /
    //            CUser::SendConditionEventInfo(void)
    void SendProperDungeonClearCount();
    void SendConditionEventInfo();

    // IDA_EA: 0x867C110
    // IDA_NAME: _ZN5CUser23SendCmdErrorPacket2ByteE14ENUM_CMDPACKETt
    // DEMANGLED: CUser::SendCmdErrorPacket2Byte(ENUM_CMDPACKET,unsigned short)
    bool SendCmdErrorPacket2Byte(ENUM_CMDPACKET command,
                                 unsigned short error);

    // IDA_EA: 0x868F60E
    // IDA_NAME: _ZN5CUser26verifyStackableItemProcessEN15StackableAction4TypeEPK5CItem
    // DEMANGLED: CUser::verifyStackableItemProcess(StackableAction::Type,CItem const*)
    bool verifyStackableItemProcess(StackableAction::Type action,
                                    const CItem* item);

    // IDA_EA: 0x868F69A
    // IDA_NAME: _ZN5CUser30verifyStackableItemActionPlaceEN15StackableAction4TypeEPK5CItem
    // DEMANGLED: CUser::verifyStackableItemActionPlace(StackableAction::Type,CItem const*)
    bool verifyStackableItemActionPlace(StackableAction::Type action,
                                        const CItem* item);

    // IDA_EA: 0x868F6BC
    // IDA_NAME: _ZN5CUser30verifyStackableItemActionPlaceEN15StackableAction4TypeEj
    // DEMANGLED: CUser::verifyStackableItemActionPlace(StackableAction::Type,unsigned int)
    bool verifyStackableItemActionPlace(StackableAction::Type action,
                                        unsigned int item_id);

    // IDA_EA: 0x869154A
    // IDA_NAME: _ZN5CUser20giveLotteryCountItemEib
    // DEMANGLED: CUser::giveLotteryCountItem(int,bool)
    void giveLotteryCountItem(int item_index, bool match);

    // IDA_EA: 0x865DD14
    // IDA_NAME: _ZN5CUser10send_equipEi
    // DEMANGLED: CUser::send_equip(int)
    void send_equip(int slot);

    // IDA_EA: 0x865EDE0
    // IDA_NAME: _ZN5CUser13use_equipmentEi
    // DEMANGLED: CUser::use_equipment(int)
    void use_equipment(int equipment_slot);

    // IDA_EA: 0x80CBC90
    // IDA_NAME: _ZNK5CUser14GetServerGroupEv
    // DEMANGLED: CUser::GetServerGroup(void)const
    int GetServerGroup() const;

    // IDA unit: CUser::_DBLogItemInfo.  This vector element is shared by the
    // add, DB-write and reset bodies at the verified CUser offset.
    struct DBLogItemInfo {
        uint32_t item_id;
        uint32_t count;
        uint32_t item_type;
    };

    // IDA_EA: 0x813492C
    // IDA_NAME: _ZN5CUser8get_posXEv
    // DEMANGLED: CUser::get_posX(void)
    int get_posX();

    // IDA_EA: 0x813493C
    // IDA_NAME: _ZN5CUser8get_posYEv
    // DEMANGLED: CUser::get_posY(void)
    int get_posY();

    // IDA_EA: 0x813494C
    // IDA_NAME: _ZN5CUser13get_directionEv
    // DEMANGLED: CUser::get_direction(void)
    int get_direction();

    // IDA_EA: 0x86813BE
    // IDA_NAME: _ZN5CUser8get_areaEb
    // DEMANGLED: CUser::get_area(bool)
    int get_area(bool current);

    // IDA_EA: 0x82F0E2A
    // IDA_NAME: _ZN5CUser12set_positionEttc
    // DEMANGLED: CUser::set_position(unsigned short,unsigned short,char)
    CUser* set_position(uint16_t pos_x, uint16_t pos_y, char direction);

    // IDA_EA: 0x8697418
    // IDA_NAME: _ZN5CUser15set_before_areaEi
    // DEMANGLED: CUser::set_before_area(int)
    CUser* set_before_area(int area_index);

    // IDA_EA: 0x8647864
    // IDA_NAME: _ZN5CUser8set_areaEi
    // DEMANGLED: CUser::set_area(int)
    CUser* set_area(int area_index);

    // IDA_EA: 0x86D20E8
    // IDA_NAME: _ZN5CUser14set_trick_areaEi
    // DEMANGLED: CUser::set_trick_area(int)
    CUser* set_trick_area(int area_index);

    // IDA_EA: 0x86D20FA
    // IDA_NAME: _ZN5CUser15get_before_areaEv
    // DEMANGLED: CUser::get_before_area(void)
    int get_before_area();

    // IDA_EA: 0x868BA38
    // IDA_NAME: _ZNK5CUser17getBeforeAreaTimeEv
    // DEMANGLED: CUser::getBeforeAreaTime(void)const
    int getBeforeAreaTime() const;

    // IDA_EA: 0x868BA46
    // IDA_NAME: _ZN5CUser17setBeforeAreaTimeEl
    // DEMANGLED: CUser::setBeforeAreaTime(long)
    CUser* setBeforeAreaTime(long area_time);

    // IDA_EA: 0x864A97C
    // IDA_NAME: _ZNK5CUser19GetServerGroupToPvPEv
    // DEMANGLED: CUser::GetServerGroupToPvP(void)const
    int GetServerGroupToPvP() const;

    // IDA_EA: 0x8690ECA
    // IDA_NAME: _ZNK5CUser12getMoveSpaceEv
    // DEMANGLED: CUser::getMoveSpace(void)const
    int getMoveSpace() const;

    // IDA_EA: 0x865514C
    // IDA_NAME: _ZN5CUser8GetPartyEv
    // DEMANGLED: CUser::GetParty(void)
    CParty* GetParty();

    // IDA_EA: 0x8680706
    // IDA_NAME: _ZN5CUser27sendBackSecurityAuthRequestEv
    // DEMANGLED: CUser::sendBackSecurityAuthRequest(void)
    void sendBackSecurityAuthRequest();

    // IDA_EA: 0x868056A
    // IDA_NAME: _ZN5CUser18GetUserListByPartyERSt6vectorIPS_SaIS1_EE
    // DEMANGLED: CUser::GetUserListByParty(std::vector<CUser*>&)
    CParty* GetUserListByParty(std::vector<CUser*>& users);

    // IDA_EA: 0x827565A
    // IDA_NAME: _ZN5CUser10GetPICSMapEv
    // DEMANGLED: CUser::GetPICSMap(void)
    std::map<std::pair<char, char>, int>& GetPICSMap();

    // Typed access to CUser's verified embedded history component.  This is
    // an inline layout exposure, not an offset bridge or a new provider.
    cUserHistoryLog& history_log() { return history_log_; }
    const cUserHistoryLog& history_log() const { return history_log_; }

    std::vector<unsigned int>& black_list() { return black_list_; }
    const std::vector<unsigned int>& black_list() const { return black_list_; }

    // GetRewardItem @0x81B33B1 reads the low byte at the proven +0x06EF68
    // slot and forces its 10,000-sided item-drop roll to zero when set.
    // This inline owner projection preserves the direct byte semantics.
    bool mercenary_reward_item_dice_override() const {
        return network_lifecycle_state_.mercenary_reward_dice_override != 0;
    }

    // Expanded CUser::C2 clears this exact member at +0x079a48.
    // BuyPrivateStoreItem/SellItem pass it to HistoryLog as their stream.
    FILE* history_stream() const { return history_stream_; }

    // IDA_EA: 0x865518E
    // IDA_NAME: _ZN5CUser10GetPVPRoomEv
    // DEMANGLED: CUser::GetPVPRoom(void)
    PvP_Room* GetPVPRoom();

    // IDA_EA: 0x80F995A
    // IDA_NAME: _ZN5CUser15getHackAnalyzerEv
    // DEMANGLED: CUser::getHackAnalyzer(void)
    WongWork::CHackAnalyzer* getHackAnalyzer();

    // IDA_EA: 0x8692F36
    // IDA_NAME: _ZN5CUser23isAllowLocationGeoStateEv
    // DEMANGLED: CUser::isAllowLocationGeoState(void)
    int isAllowLocationGeoState();

    // IDA_EA: 0x84ED008
    // IDA_NAME: _ZN5CUser14GetGoldControlEv
    // DEMANGLED: CUser::GetGoldControl(void)
    void* GetGoldControl();

    // IDA_EA: 0x814589C
    // IDA_NAME: _ZN5CUser8isGMUserEv
    // DEMANGLED: CUser::isGMUser(void)
    int isGMUser();

    // IDA_EA: 0x814AA7A
    // IDA_NAME: _ZN5CUser14SetRevivalTimeEl
    // DEMANGLED: CUser::SetRevivalTime(long)
    CUser* SetRevivalTime(long revival_time);

    // IDA_EA: 0x814AA9A
    // IDA_NAME: _ZN5CUser19SetUseAPCPotionTimeEl
    // DEMANGLED: CUser::SetUseAPCPotionTime(long)
    CUser* SetUseAPCPotionTime(long potion_time);

    // IDA_EA: 0x868BC7C
    // IDA_NAME: _ZN5CUser20GetCurExpertJobLevelEi
    // DEMANGLED: CUser::GetCurExpertJobLevel(int)
    int GetCurExpertJobLevel(int exp);

    // IDA_EA: 0x868BA58
    // IDA_NAME: _ZN5CUser27CalculateExpertJobMaterialsERSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::CalculateExpertJobMaterials(std::vector<std::pair<int,int> >&)
    void CalculateExpertJobMaterials(
        std::vector<std::pair<int, int> >& materials);

    // IDA_EA: 0x822FC06
    // IDA_NAME: _ZN5CUser29SetLoadRentalInfoFromExchangeEb
    // DEMANGLED: CUser::SetLoadRentalInfoFromExchange(bool)
    CUser* SetLoadRentalInfoFromExchange(bool load);

    // IDA_EA: 0x822FC8E
    // IDA_NAME: _ZN5CUser14setGmQuestFlagEb
    // DEMANGLED: CUser::setGmQuestFlag(bool)
    CUser* setGmQuestFlag(bool flag);

    // IDA_EA: 0x822FCC6
    // IDA_NAME: _ZN5CUser12getPowerSideEv
    // DEMANGLED: CUser::getPowerSide(void)
    int getPowerSide();

    // IDA_EA: 0x822FCAA
    // IDA_NAME: _ZN5CUser28getOnlinePreliminaryCharacNoEv
    // DEMANGLED: CUser::getOnlinePreliminaryCharacNo(void)
    unsigned int getOnlinePreliminaryCharacNo();

    // IDA_EA: 0x822FDB6
    // IDA_NAME: _ZN5CUser4lockEv
    // DEMANGLED: CUser::lock(void)
    int lock();

    // IDA_EA: 0x822FDCE
    // IDA_NAME: _ZN5CUser6unlockEv
    // DEMANGLED: CUser::unlock(void)
    int unlock();

    // IDA_EA: 0x822FDE6
    // IDA_NAME: _ZNK5CUser10getRegTimeEv
    // DEMANGLED: CUser::getRegTime(void)const
    int getRegTime() const;

    // IDA_EA: 0x822FE02
    // IDA_NAME: _ZN5CUser11GetSchoolNoEv
    // DEMANGLED: CUser::GetSchoolNo(void)
    int GetSchoolNo();

    // IDA_EA: 0x823020C
    // IDA_NAME: _ZN5CUser10GetMailBoxEv
    // DEMANGLED: CUser::GetMailBox(void)
    WongWork::CMailBox* GetMailBox();

    // IDA_EA: 0x823021A
    // IDA_NAME: _ZN5CUser16isHumanCertifiedEv
    // DEMANGLED: CUser::isHumanCertified(void)
    int isHumanCertified();

    // IDA_EA: 0x823022A
    // IDA_NAME: _ZN5CUser17setHumanCertifiedEb
    // DEMANGLED: CUser::setHumanCertified(bool)
    CUser* setHumanCertified(bool certified);

    // IDA_EA: 0x8230800
    // IDA_NAME: _ZN5CUser8getHadesEv
    // DEMANGLED: CUser::getHades(void)
    XNuclear::CHades* getHades();

    // IDA_EA: 0x84ECB1A
    // IDA_NAME: _ZN5CUser19getAcccountHackInfoEv
    // DEMANGLED: CUser::getAcccountHackInfo(void)
    Secu_AccountHacking* getAcccountHackInfo();

    // IDA_EA: 0x823080E
    // IDA_NAME: _ZN5CUser12GetHackCheckEv
    // DEMANGLED: CUser::GetHackCheck(void)
    Secu_HackLogCheck* GetHackCheck();

    // IDA_EA: 0x823081C
    // IDA_NAME: _ZN5CUser12GetCSHashSetEv
    // DEMANGLED: CUser::GetCSHashSet(void)
    CUserCSHashSet* GetCSHashSet();

    // IDA_EA: 0x8275668
    // IDA_NAME: _ZN5CUser20GetPIReceiverManagerEv
    // DEMANGLED: CUser::GetPIReceiverManager(void)
    PIReceiverManager* GetPIReceiverManager();

    // IDA_EA: 0x82A6796
    // IDA_NAME: _ZN5CUser12IsProgLogoutEv
    // DEMANGLED: CUser::IsProgLogout(void)
    int IsProgLogout();

    // IDA_EA: 0x869735E
    // IDA_NAME: _ZN5CUser13SetProgLogoutEv
    // DEMANGLED: CUser::SetProgLogout(void)
    CUser* SetProgLogout();

    // IDA_EA: 0x82A683C
    // IDA_NAME: _ZNK5CUser10GetIncreIDEv
    // DEMANGLED: CUser::GetIncreID(void)const
    int GetIncreID() const;

    // IDA_EA: 0x84ECD42
    // IDA_NAME: _ZNK5CUser10GetSlotIDXEv
    // DEMANGLED: CUser::GetSlotIDX(void)const
    int GetSlotIDX() const;

    // IDA_EA: 0x863BEC0
    // IDA_NAME: _ZN5CUser12GetLoginTimeEv
    // DEMANGLED: CUser::GetLoginTime(void)
    int GetLoginTime();

    // IDA_EA: 0x822FCD6
    // IDA_NAME: _ZN5CUser15getClientHWSpecEv
    // DEMANGLED: CUser::getClientHWSpec(void)
    char* getClientHWSpec();

    // IDA_EA: 0x867CB82
    // IDA_NAME: _ZN5CUser22getPlayingDungeonIndexEv
    // DEMANGLED: CUser::getPlayingDungeonIndex(void)
    int getPlayingDungeonIndex();

    // IDA_EA: 0x80DA314
    // IDA_NAME: _ZN5CUser12CheckInPartyEv
    // DEMANGLED: CUser::CheckInParty(void)
    short CheckInParty();

    // IDA_EA: 0x822FE4E
    // IDA_NAME: _ZN5CUser13GetPartyIndexEv
    // DEMANGLED: CUser::GetPartyIndex(void)
    int GetPartyIndex();

    // IDA_EA: 0x85BFD96
    // IDA_NAME: _ZN5CUser13SetPartyIndexEs
    // DEMANGLED: CUser::SetPartyIndex(short)
    int SetPartyIndex(short party_index);

    // IDA_EA: 0x822FE5E
    // IDA_NAME: _ZN5CUser10CheckInPvpEv
    // DEMANGLED: CUser::CheckInPvp(void)
    short CheckInPvp();

    // IDA_EA: 0x822FE74
    // IDA_NAME: _ZN5CUser11GetPvpIndexEv
    // DEMANGLED: CUser::GetPvpIndex(void)
    int GetPvpIndex();

    // IDA_EA: 0x85DF9B2
    // IDA_NAME: _ZN5CUser11SetPvpIndexEs
    // DEMANGLED: CUser::SetPvpIndex(short)
    int SetPvpIndex(short pvp_index);

    // IDA_EA: 0x80DA32A
    // IDA_NAME: _ZN5CUser17checkInBlueMarbleEv
    // DEMANGLED: CUser::checkInBlueMarble(void)
    short checkInBlueMarble();

    // IDA_EA: 0x80DA340
    // IDA_NAME: _ZN5CUser18setBlueMarbleIndexEs
    // DEMANGLED: CUser::setBlueMarbleIndex(short)
    CUser* setBlueMarbleIndex(short index);

    // IDA_EA: 0x80DA35E
    // IDA_NAME: _ZN5CUser18getBlueMarbleIndexEv
    // DEMANGLED: CUser::getBlueMarbleIndex(void)
    int getBlueMarbleIndex();

    // IDA_EA: 0x86552E6
    // IDA_NAME: _ZN5CUser13getBlueMarbleEv
    // DEMANGLED: CUser::getBlueMarble(void)
    BlueMarble* getBlueMarble();

    // IDA_EA: 0x82A67A6
    // IDA_NAME: _ZN5CUser14CheckInWarRoomEv
    // DEMANGLED: CUser::CheckInWarRoom(void)
    short CheckInWarRoom();

    // IDA_EA: 0x822FE84
    // IDA_NAME: _ZN5CUser15GetWarRoomIndexEv
    // DEMANGLED: CUser::GetWarRoomIndex(void)
    int GetWarRoomIndex();

    // IDA_EA: 0x86C0B50
    // IDA_NAME: _ZN5CUser15SetWarRoomIndexEs
    // DEMANGLED: CUser::SetWarRoomIndex(short)
    CUser* SetWarRoomIndex(short index);

    // IDA_EA: 0x82A67BC
    // IDA_NAME: _ZN5CUser17checkInDeathTowerEv
    // DEMANGLED: CUser::checkInDeathTower(void)
    short checkInDeathTower();

    // IDA_EA: 0x82A67D2
    // IDA_NAME: _ZN5CUser18getDeathTowerIndexEv
    // DEMANGLED: CUser::getDeathTowerIndex(void)
    int getDeathTowerIndex();

    // IDA_EA: 0x8469A6E
    // IDA_NAME: _ZN5CUser18setDeathTowerIndexEs
    // DEMANGLED: CUser::setDeathTowerIndex(short)
    CUser* setDeathTowerIndex(short index);

    // IDA_EA: 0x82A67E2
    // IDA_NAME: _ZN5CUser16checkInBossTowerEv
    // DEMANGLED: CUser::checkInBossTower(void)
    short checkInBossTower();

    // IDA_EA: 0x822FE94
    // IDA_NAME: _ZN5CUser17getBossTowerIndexEv
    // DEMANGLED: CUser::getBossTowerIndex(void)
    int getBossTowerIndex();

    // IDA_EA: 0x814AAAC
    // IDA_NAME: _ZN5CUser17setBossTowerIndexEs
    // DEMANGLED: CUser::setBossTowerIndex(short)
    CUser* setBossTowerIndex(short index);

    // IDA_EA: 0x82A67F8
    // IDA_NAME: _ZNK5CUser19checkInAdvanceAltarEv
    // DEMANGLED: CUser::checkInAdvanceAltar(void)const
    short checkInAdvanceAltar() const;

    // IDA_EA: 0x82A680E
    // IDA_NAME: _ZNK5CUser20getAdvanceAltarIndexEv
    // DEMANGLED: CUser::getAdvanceAltarIndex(void)const
    int getAdvanceAltarIndex() const;

    // IDA_EA: 0x813490E
    // IDA_NAME: _ZN5CUser20setAdvanceAltarIndexEs
    // DEMANGLED: CUser::setAdvanceAltarIndex(short)
    CUser* setAdvanceAltarIndex(short index);

    // IDA_EA: 0x8655262
    // IDA_NAME: _ZN5CUser15getAdvanceAltarEv
    // DEMANGLED: CUser::getAdvanceAltar(void)
    //
    // This is a lookup of the live StageControl registered by CGameManager;
    // it is deliberately distinct from the per-character persisted manager
    // at expanded +0x8e2b0.
    advancealtar::StageControl* getAdvanceAltar();

    // IDA_EA: 0x8655220
    // IDA_NAME: _ZN5CUser12getBossTowerEv
    // DEMANGLED: CUser::getBossTower(void)
    WongWork::CBossTower* getBossTower();

    // IDA_EA: 0x86552A4
    // IDA_NAME: _ZN5CUser13getDeathTowerEv
    // DEMANGLED: CUser::getDeathTower(void)
    WongWork::CDeathTower* getDeathTower();

    // IDA_EA: 0x8657804
    // IDA_NAME: _ZNK5CUser27getCurCharacTotalMaxFatigueEv
    // DEMANGLED: CUser::getCurCharacTotalMaxFatigue(void)const
    int getCurCharacTotalMaxFatigue() const;

    // IDA_EA: 0x8657766
    // IDA_NAME: _ZNK5CUser24getCurCharacTotalFatigueEv
    // DEMANGLED: CUser::getCurCharacTotalFatigue(void)const
    int getCurCharacTotalFatigue() const;

    // IDA_EA: 0x864F7FE
    // IDA_NAME: _ZN5CUser20gainCurCharacFatigueEi
    // DEMANGLED: CUser::gainCurCharacFatigue(int)
    int gainCurCharacFatigue(int refill_fatigue);

    // IDA_EA: 0x8656500
    // IDA_NAME: _ZN5CUser12CheckFatigueEv
    // DEMANGLED: CUser::CheckFatigue(void)
    bool CheckFatigue();

    // IDA_EA: 0x8656540
    // IDA_NAME: _ZN5CUser11SendFatigueEv
    // DEMANGLED: CUser::SendFatigue(void)
    void SendFatigue();

    // IDA_EA: 0x8656636
    // IDA_NAME: _ZN5CUser24GetSpecialLevelCharacCntEi
    // DEMANGLED: CUser::GetSpecialLevelCharacCnt(int)
    int GetSpecialLevelCharacCnt(int level);

    // IDA_EA: 0x8657570
    // IDA_NAME: _ZN5CUser15ResetDailyQuestEv
    // DEMANGLED: CUser::ResetDailyQuest(void)
    void ResetDailyQuest();

    // IDA_EA: 0x8657662
    // IDA_NAME: _ZN5CUser18ResetTrainingQuestEv
    // DEMANGLED: CUser::ResetTrainingQuest(void)
    void ResetTrainingQuest();

    // IDA_EA: 0x8657ADA
    // IDA_NAME: _ZN5CUser14RecoverFatigueEi
    // DEMANGLED: CUser::RecoverFatigue(int)
    char RecoverFatigue(int fatigue);

    // IDA_EA: 0x8657F10
    // IDA_NAME: _ZN5CUser11RecoverCoinEj
    // DEMANGLED: CUser::RecoverCoin(unsigned int)
    char RecoverCoin(unsigned int coin_no_per_day);

    // IDA_EA: 0x867CBE4
    // IDA_NAME: _ZN5CUser18ProcPremiumFatigueEv
    // DEMANGLED: CUser::ProcPremiumFatigue(void)
    char ProcPremiumFatigue();

    // IDA_EA: 0x865798A
    // IDA_NAME: _ZN5CUser15serviceOfShusiaEv
    // DEMANGLED: CUser::serviceOfShusia(void)
    int serviceOfShusia();

    // IDA_EA: 0x8655C60
    // IDA_NAME: _ZN5CUser9FatigueUpEi
    // DEMANGLED: CUser::FatigueUp(int)
    // Hex-Rays propagates the local arithmetic as unsigned, but the original
    // Itanium symbol carries `i`; preserve the true public member ABI here.
    int FatigueUp(int fatigue);

    // IDA_EA: 0x8659490
    // IDA_NAME: _ZN5CUser25_processCharacterDayEventEjj
    // DEMANGLED: CUser::_processCharacterDayEvent(unsigned int,unsigned int)
    int _processCharacterDayEvent(unsigned int used_fatigue_before,
                                  unsigned int amount);

    // IDA_EA: 0x86598BA
    // IDA_NAME: _ZN5CUser24_processFatigueBuffEventEjj
    // DEMANGLED: CUser::_processFatigueBuffEvent(unsigned int,unsigned int)
    int _processFatigueBuffEvent(unsigned int used_fatigue_before,
                                 unsigned int amount);

    // IDA_EA: 0x86925C4
    // IDA_NAME: _ZN5CUser20processFatigueCharacEi
    // DEMANGLED: CUser::processFatigueCharac(int)
    void processFatigueCharac(int amount);

    // IDA_EA: 0x81253F4
    // IDA_NAME: _ZN5CUser16EnableCharacInfoE13CACHE_SECTION
    // DEMANGLED: CUser::EnableCharacInfo(CACHE_SECTION)
    int EnableCharacInfo(CACHE_SECTION section);

    // IDA_EA: 0x81348CE
    // IDA_NAME: _ZN5CUser17DisableCharacInfoE13CACHE_SECTION
    // DEMANGLED: CUser::DisableCharacInfo(CACHE_SECTION)
    int DisableCharacInfo(CACHE_SECTION section);

    // Reconstructed read-only companion for the verified 12-byte completion
    // table.  It adds no CUser storage and is used only to enforce cross-queue
    // load dependencies introduced by native continuations.
    bool IsCharacInfoLoaded(CACHE_SECTION section) const;

    // IDA_EA: 0x81348E2
    // IDA_NAME: _ZN5CUser20GetLastLoginCharacNoEv
    // DEMANGLED: CUser::GetLastLoginCharacNo(void)
    int GetLastLoginCharacNo();

    // IDA_EA: 0x81348F0
    // IDA_NAME: _ZN5CUser21GetLastLoginChannelNoEv
    // DEMANGLED: CUser::GetLastLoginChannelNo(void)
    int GetLastLoginChannelNo();

    // IDA_EA: 0x81348FE
    // IDA_NAME: _ZN5CUser22IsValidLastLoginCharacEv
    // DEMANGLED: CUser::IsValidLastLoginCharac(void)
    int IsValidLastLoginCharac();

    // IDA_EA: 0x822FD5C
    // IDA_NAME: _ZN5CUser20SetLastLoginCharacNoEi
    // DEMANGLED: CUser::SetLastLoginCharacNo(int)
    CUser* SetLastLoginCharacNo(int charac_no);

    // IDA_EA: 0x822FD6E
    // IDA_NAME: _ZN5CUser21SetLastLoginChannelNoEi
    // DEMANGLED: CUser::SetLastLoginChannelNo(int)
    CUser* SetLastLoginChannelNo(int channel_no);

    // IDA_EA: 0x822FD80
    // IDA_NAME: _ZN5CUser23SetValidLastLoginCharacEb
    // DEMANGLED: CUser::SetValidLastLoginCharac(bool)
    CUser* SetValidLastLoginCharac(bool valid);

    // IDA_EA: 0x822FEB6
    // IDA_NAME: _ZNK5CUser15get_charac_nameEi
    // DEMANGLED: CUser::get_charac_name(int)const
    const char* get_charac_name(int slot) const;

    // IDA_EA: 0x822FF0C
    // IDA_NAME: _ZNK5CUser12GetCharacJobEi
    // DEMANGLED: CUser::GetCharacJob(int)const
    int GetCharacJob(int slot) const;

    // IDA_EA: 0x822FF62
    // IDA_NAME: _ZNK5CUser31CheckPossibleRequestRenameStateEi
    // DEMANGLED: CUser::CheckPossibleRequestRenameState(int)const
    bool CheckPossibleRequestRenameState(int slot) const;

    // IDA_EA: 0x822FFC2
    // IDA_NAME: _ZN5CUser18SetCharacViewStateEi
    // DEMANGLED: CUser::SetCharacViewState(int)
    int SetCharacViewState(int state);

    // IDA_EA: 0x822FFEC
    // IDA_NAME: _ZN5CUser18SetCharacViewStateEii
    // DEMANGLED: CUser::SetCharacViewState(int,int)
    int SetCharacViewState(int slot, int state);

    // IDA_EA: 0x8230054
    // IDA_NAME: _ZNK5CUser18GetCharacViewStateEi
    // DEMANGLED: CUser::GetCharacViewState(int)const
    int GetCharacViewState(int slot) const;

    // IDA_EA: 0x8230860
    // IDA_NAME: _ZNK5CUser18getCharacSlotLimitEv
    // DEMANGLED: CUser::getCharacSlotLimit(void)const
    int getCharacSlotLimit() const;

    // IDA_EA: 0x8230870
    // IDA_NAME: _ZNK5CUser18getSlotEffectCountEv
    // DEMANGLED: CUser::getSlotEffectCount(void)const
    int getSlotEffectCount() const;

    // IDA_EA: 0x8230880
    // IDA_NAME: _ZN5CUser18setSlotEffectCountEh
    // DEMANGLED: CUser::setSlotEffectCount(unsigned char)
    CUser* setSlotEffectCount(unsigned char value);

    // IDA_EA: 0x84ECB36
    // IDA_NAME: _ZN5CUser28SetCharacViewStateByCharacNoEii
    // DEMANGLED: CUser::SetCharacViewStateByCharacNo(int,int)
    int SetCharacViewStateByCharacNo(int charac_no, int state);

    // IDA_EA: 0x863BECE
    // IDA_NAME: _ZNK5CUser28get_charac_name_by_charac_noEi
    // DEMANGLED: CUser::get_charac_name_by_charac_no(int)const
    const char* get_charac_name_by_charac_no(int charac_no) const;

    // IDA_EA: 0x864A9AC
    // IDA_NAME: _ZN5CUser13lock4DataLoadEv
    // DEMANGLED: CUser::lock4DataLoad(void)
    int lock4DataLoad();

    // IDA_EA: 0x864A9DA
    // IDA_NAME: _ZN5CUser15unlock4DataLoadEv
    // DEMANGLED: CUser::unlock4DataLoad(void)
    int unlock4DataLoad();

    // IDA_EA: 0x864AA0A
    // IDA_NAME: _ZN5CUser17isLocked4DataLoadEv
    // DEMANGLED: CUser::isLocked4DataLoad(void)
    int isLocked4DataLoad();

    // IDA_EA: 0x864AA1A
    // IDA_NAME: _ZN5CUser16lockDeleteCharacEv
    // DEMANGLED: CUser::lockDeleteCharac(void)
    int lockDeleteCharac();

    // IDA_EA: 0x864AA48
    // IDA_NAME: _ZN5CUser18unlockDeleteCharacEv
    // DEMANGLED: CUser::unlockDeleteCharac(void)
    int unlockDeleteCharac();

    // IDA_EA: 0x864AA78
    // IDA_NAME: _ZN5CUser20isLockedDeleteCharacEv
    // DEMANGLED: CUser::isLockedDeleteCharac(void)
    int isLockedDeleteCharac();

    // IDA_EA: 0x864AA88
    // IDA_NAME: _ZN5CUser16lockCreateCharacEv
    // DEMANGLED: CUser::lockCreateCharac(void)
    int lockCreateCharac();

    // IDA_EA: 0x864AAB6
    // IDA_NAME: _ZN5CUser18unlockCreateCharacEv
    // DEMANGLED: CUser::unlockCreateCharac(void)
    int unlockCreateCharac();

    // IDA_EA: 0x864AAE6
    // IDA_NAME: _ZN5CUser20isLockedCreateCharacEv
    // DEMANGLED: CUser::isLockedCreateCharac(void)
    int isLockedCreateCharac();

    // IDA_EA: 0x864A830
    // IDA_NAME: _ZN5CUser21CheckDeleteCharacTimeEi
    // DEMANGLED: CUser::CheckDeleteCharacTime(int)
    int CheckDeleteCharacTime(int slot);

    // IDA_EA: 0x864AAF6
    // IDA_NAME: _ZN5CUser17checkSelectCharacEi
    // DEMANGLED: CUser::checkSelectCharac(int)
    int checkSelectCharac(int slot);

    // IDA_EA: 0x864DF12
    // IDA_NAME: _ZNK5CUser14get_charac_idxEj
    // DEMANGLED: CUser::get_charac_idx(unsigned int)const
    int get_charac_idx(unsigned int charac_no) const;

    // IDA_EA: 0x864DFA0
    // IDA_NAME: _ZN5CUser13get_charac_noEi
    // DEMANGLED: CUser::get_charac_no(int)
    int get_charac_no(int slot);

    // IDA_EA: 0x864E024
    // IDA_NAME: _ZNK5CUser17get_character_jobEj
    // DEMANGLED: CUser::get_character_job(unsigned int)const
    int get_character_job(unsigned int charac_no) const;

    // IDA_EA: 0x864CD92
    // IDA_NAME: _ZN5CUser12SelectCharacEii
    // DEMANGLED: CUser::SelectCharac(int,int)
    int SelectCharac(int slot, int tag_slot);

    // IDA_EA: 0x8651B7A
    // IDA_NAME: _ZN5CUser16ChangeCharacSlotEjj
    // DEMANGLED: CUser::ChangeCharacSlot(unsigned int,unsigned int)
    void* ChangeCharacSlot(unsigned int src_slot, unsigned int dst_slot);

    // IDA_EA: 0x868745E
    // IDA_NAME: _ZN5CUser19_processLevelRewardEjjc
    // DEMANGLED: CUser::_processLevelReward(unsigned int,unsigned int,char)
    int _processLevelReward(unsigned int charac_level,
                            unsigned int charac_no,
                            char job);

    // IDA_EA: 0x864A5AA
    // IDA_NAME: _ZN5CUser24_processUnlimitChallengeEjc
    // DEMANGLED: CUser::_processUnlimitChallenge(unsigned int,char)
    int _processUnlimitChallenge(unsigned int charac_no, char job);

    // IDA_EA: 0x864A432
    // IDA_NAME: _ZN5CUser18_onCreateCharacterEjPcc
    // DEMANGLED: CUser::_onCreateCharacter(unsigned int,char *,char)
    int _onCreateCharacter(unsigned int charac_no, char* charac_name, char job);

    // IDA_EA: 0x864A186
    // IDA_NAME: _ZN5CUser12CreateCharacEP17SIG_CREATE_CHARAC
    // DEMANGLED: CUser::CreateCharac(SIG_CREATE_CHARAC *)
    int CreateCharac(SIG_CREATE_CHARAC* create_payload);

    // IDA_EA: 0x8649C6A
    // IDA_NAME: _ZN5CUser13SetCharacInfoEiPc
    // DEMANGLED: CUser::SetCharacInfo(int,char *)
    // The caller is Inter_LoginInfo and passes the +0xE0 character-view
    // span of SIG_LOGIN_DATA.  Keep the old char* ABI: this method predates
    // the typed login payload declaration and is exported under that name.
    void SetCharacInfo(int charac_count, char* charac_view_data);

    // IDA_EA: 0x864D160
    // IDA_NAME: _ZN5CUser12SetInventoryEP18SIG_LOAD_INVENTORY
    // DEMANGLED: CUser::SetInventory(SIG_LOAD_INVENTORY *)
    int SetInventory(SIG_LOAD_INVENTORY* inventory);

    // IDA_EA: 0x865DA4E
    // IDA_NAME: _ZN5CUser12SetEquipmentEiPviS0_
    // DEMANGLED: CUser::SetEquipment(int,void *,int,void *)
    void* SetEquipment(int character_index,
                       void* source,
                       int bytes,
                       void* expired_batch);

    // IDA_EA: 0x864AC1A
    // IDA_NAME: _ZN5CUser19setCharacInfoDetailEijP11CHARAC_DATA
    // DEMANGLED: CUser::setCharacInfoDetail(int,unsigned int,CHARAC_DATA *)
    int setCharacInfoDetail(int charac_slot,
                            unsigned int charac_no,
                            CHARAC_DATA* data);

    // IDA_EA: 0x864A63A
    // IDA_NAME: _ZN5CUser12DeleteCharacEj
    // DEMANGLED: CUser::DeleteCharac(unsigned int)
    CCharacterView* DeleteCharac(unsigned int charac_no);

    // IDA_EA: 0x866DA8A
    // IDA_NAME: _ZN5CUser12quest_actionEiiii
    // DEMANGLED: CUser::quest_action(int,int,int,int)
    void quest_action(int action,
                      int quest_index,
                      int trigger_state,
                      int trigger_value);

    // IDA_EA: 0x8664C50
    // IDA_NAME: _ZN5CUser15calc_quest_statER15_Additioal_info
    // DEMANGLED: CUser::calc_quest_stat(_Additioal_info &)
    void calc_quest_stat(_Additioal_info& calculated_stat);

    // IDA_EA: 0x8664E8E
    // IDA_NAME: _ZN5CUser17_fnStatQuestClearEjPc
    // DEMANGLED: CUser::_fnStatQuestClear(unsigned int,char *)
    // This is a static vararg callback, despite its CUser-qualified name.
    static bool _fnStatQuestClear(unsigned int quest_index,
                                  char* argument_block);

    // IDA_EA: 0x866EA92
    // IDA_NAME: _ZN5CUser12quest_rewardEiii
    // DEMANGLED: CUser::quest_reward(int,int,int)
    int quest_reward(int quest_index,
                     int selected_reward_index,
                     int trigger_value);

    // IDA_EA: 0x8664DCE
    // IDA_NAME: _ZN5CUser20complete_reward_listERSt6vectorISt4pairIiiESaIS2_EES5_
    // DEMANGLED: CUser::complete_reward_list(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
    bool complete_reward_list(
        std::vector<std::pair<int, int> >& reward_items,
        std::vector<std::pair<int, int> >& completed_items);

    // IDA_EA: 0x86650B2
    // IDA_NAME: _ZN5CUser16add_matched_listERSt6vectorISt4pairIiiESaIS2_EEii
    // DEMANGLED: CUser::add_matched_list(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int,int)
    // The raw body has no CUser receiver; this is a CUser-namespaced helper.
    static int add_matched_list(
        std::vector<std::pair<int, int> >& matched_items,
        int item_id,
        int item_count);

    // IDA_EA: 0x866514A
    // IDA_NAME: _ZN5CUser20count_specific_itemsERSt6vectorIiSaIiEERS0_ISt4pairIiiESaIS5_EE
    // DEMANGLED: CUser::count_specific_items(std::vector<int,std::allocator<int>> &,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
    bool count_specific_items(
        std::vector<int>& item_ids,
        std::vector<std::pair<int, int> >& item_counts);

    // IDA_EA: 0x8665274
    // IDA_NAME: _ZN5CUser17apply_elixir_itemER15_Additioal_infoRSt4pairIiiE
    // DEMANGLED: CUser::apply_elixir_item(_Additioal_info &,std::pair<int,int> &)
    char apply_elixir_item(
        _Additioal_info& additional_info,
        std::pair<int, int>& item);

    // IDA_EA: 0x866D972
    // IDA_NAME: _ZN5CUser30SendHasNoGoWithApcQuestInPartyEPK5Quest
    // DEMANGLED: CUser::SendHasNoGoWithApcQuestInParty(Quest const*)
    void SendHasNoGoWithApcQuestInParty(const Quest* quest);

    // IDA_EA: 0x8664412
    // IDA_NAME: _ZN5CUser13_onQuestClearEii
    // DEMANGLED: CUser::_onQuestClear(int,int)
    Stream* _onQuestClear(int quest_index, int trigger_value);

    // IDA_EA: 0x866E7A8
    // IDA_NAME: _ZN5CUser18quest_basic_rewardEP5QuestRiS2_S2_S2_b
    // DEMANGLED: CUser::quest_basic_reward(Quest *,int &,int &,int &,int &,bool)
    int quest_basic_reward(Quest* quest,
                           int& first,
                           int& second,
                           int& third,
                           int& fourth,
                           bool enabled);

    // IDA_EA: 0x866E3E2
    // IDA_NAME: _ZN5CUser30compute_quest_basic_reward_expEP5Quest
    // DEMANGLED: CUser::compute_quest_basic_reward_exp(Quest *)
    unsigned int compute_quest_basic_reward_exp(Quest* quest);

    // IDA_EA: 0x866E5E6
    // IDA_NAME: _ZN5CUser31compute_quest_basic_reward_goldEP5Quest
    // DEMANGLED: CUser::compute_quest_basic_reward_gold(Quest *)
    int compute_quest_basic_reward_gold(Quest* quest);

    // IDA_EA: 0x866E914
    // IDA_NAME: _ZN5CUser22getUserQuestRewardItemEPK5QuestPKS_
    // DEMANGLED: CUser::getUserQuestRewardItem(Quest const*,CUser const*)
    // This is a stateless CUser-namespaced helper: the old i386 body receives
    // exactly the quest and user arguments, without a CUser receiver.
    static const std::vector<RewardItemInfo>* getUserQuestRewardItem(
        const Quest* quest,
        const CUser* user);

    // IDA_EA: 0x86645A4
    // IDA_NAME: _ZN5CUser23eventDeliveryQuestClearEiiRSt6vectorISt4pairIiiESaIS2_EEiii
    // DEMANGLED: CUser::eventDeliveryQuestClear(int,int,std::vector<std::pair<int,int> > &,int,int,int)
    int eventDeliveryQuestClear(
        int quest_index,
        int trigger_value,
        std::vector<std::pair<int, int> >& reward_items,
        int reward_exp,
        int reward_gold,
        int reward_quest_point);

    // IDA_EA: 0x86885A6
    // IDA_NAME: _ZN5CUser17GetSecretShopDataEv
    // DEMANGLED: CUser::GetSecretShopData(void)
    secretshop::SECRET_SHOP_DATA* GetSecretShopData();

    // IDA_EA: 0x86885D4
    // IDA_NAME: _ZN5CUser17GetUserSecretShopEv
    // DEMANGLED: CUser::GetUserSecretShop(void)
    secretshop::SECRET_SHOP_INFO* GetUserSecretShop();

    // IDA_EA: 0x868860A
    // IDA_NAME: _ZN5CUser22SendSecretShopItemListEv
    // DEMANGLED: CUser::SendSecretShopItemList(void)
    CParty* SendSecretShopItemList();

    // IDA_EA: 0x86551DE
    // IDA_NAME: _ZN5CUser10GetWarRoomEv
    // DEMANGLED: CUser::GetWarRoom(void)
    WarRoom* GetWarRoom();

    // IDA_EA: 0x8688638
    // IDA_NAME: _ZN5CUser17GetUserPosInPartyEv
    // DEMANGLED: CUser::GetUserPosInParty(void)
    int GetUserPosInParty();

    // IDA_EA: 0x867B8FE
    // IDA_NAME: _ZN5CUser10SendPacketENS_11eSendTargetER11PacketGuard
    // DEMANGLED: CUser::SendPacket(CUser::eSendTarget,PacketGuard &)
    int SendPacket(eSendTarget target, PacketGuard& packet);

    // IDA_EA: 0x867BA5C
    // IDA_NAME: _ZN5CUser14SendNotiPacketENS_11eSendTargetE15ENUM_NOTIPACKETi
    // DEMANGLED: CUser::SendNotiPacket(CUser::eSendTarget,ENUM_NOTIPACKET,int)
    int SendNotiPacket(eSendTarget target,
                       ENUM_NOTIPACKET packet_type,
                       int value);

    // IDA_EA: 0x867C2D8
    // IDA_NAME: _ZN5CUser14SendUpdateItemENS_11eSendTargetE14ENUM_ITEMSPACEi
    // DEMANGLED: CUser::SendUpdateItem(CUser::eSendTarget,ENUM_ITEMSPACE,int)
    int SendUpdateItem(eSendTarget target,
                       ENUM_ITEMSPACE item_space,
                       int slot);

    // IDA_EA: 0x867C65A
    // IDA_NAME: _ZN5CUser18SendUpdateItemListENS_11eSendTargetE14ENUM_ITEMSPACEi
    // DEMANGLED: CUser::SendUpdateItemList(CUser::eSendTarget,ENUM_ITEMSPACE,int)
    int SendUpdateItemList(eSendTarget target,
                           ENUM_ITEMSPACE item_space,
                           int slot);

    // IDA_EA: 0x8681224
    // IDA_NAME: _ZN5CUser13addAvatarItemEmccPKc20eAvatarItemAddReason
    // DEMANGLED: CUser::addAvatarItem(unsigned long,char,char,char const*,eAvatarItemAddReason)
    int addAvatarItem(unsigned long item_id,
                      char first_flag,
                      char item_flag,
                      const char* ipg_no,
                      eAvatarItemAddReason reason);

    // IDA_EA: 0x86893AC
    // IDA_NAME: _ZN5CUser12AddDBLogItemEjj20ENUM_DBLOG_ITEM_TYPE
    // DEMANGLED: CUser::AddDBLogItem(unsigned int,unsigned int,ENUM_DBLOG_ITEM_TYPE)
    int AddDBLogItem(unsigned int item_id,
                     unsigned int count,
                     ENUM_DBLOG_ITEM_TYPE log_type);

    // IDA_EA: 0x8689494
    // IDA_NAME: _ZNK5CUser17DBUpdateDBLogItemEv
    // DEMANGLED: CUser::DBUpdateDBLogItem(void)const
    int DBUpdateDBLogItem() const;

    // IDA_EA: 0x8697262
    // IDA_NAME: _ZN5CUser14ResetDBLogItemEv
    // DEMANGLED: CUser::ResetDBLogItem(void)
    int ResetDBLogItem();

    // IDA_EA: 0x8653270
    // IDA_NAME: _ZN5CUser10UpdateDataEv
    // DEMANGLED: CUser::UpdateData(void)
    char UpdateData();

    // IDA_EA: 0x865DB6C
    // IDA_NAME: _ZN5CUser14send_itemspaceEi
    // DEMANGLED: CUser::send_itemspace(int)
    int send_itemspace(int item_space);

    // IDA_EA: 0x864CB66
    // IDA_NAME: _ZN5CUser21LogoutCachedCharacterEh
    // DEMANGLED: CUser::LogoutCachedCharacter(unsigned char)
    int LogoutCachedCharacter(unsigned char flag);

    // IDA_EA: 0x868F472
    // IDA_NAME: _ZN5CUser12SendMoenyLogEv
    // DEMANGLED: CUser::SendMoenyLog(void)
    int SendMoenyLog();

    // IDA_EA: 0x864C85A
    // IDA_NAME: _ZN5CUser20CacheCharacterMemoryEv
    // DEMANGLED: CUser::CacheCharacterMemory(void)
    int CacheCharacterMemory();

    // IDA_EA: 0x864BC04
    // IDA_NAME: _ZNK5CUser13GetCharacInfoEP15SIG_LOAD_CHARAC
    // DEMANGLED: CUser::GetCharacInfo(SIG_LOAD_CHARAC *)const
    int GetCharacInfo(SIG_LOAD_CHARAC* payload) const;

    // IDA_EA: 0x864BDBA
    // IDA_NAME: _ZNK5CUser19getCharacInfoDetailEijP11CHARAC_DATA
    // DEMANGLED: CUser::getCharacInfoDetail(int,unsigned int,CHARAC_DATA *)const
    int getCharacInfoDetail(int charac_idx,
                            unsigned int charac_no,
                            CHARAC_DATA* payload) const;

    // IDA_EA: 0x86904F0
    // IDA_NAME: _ZNK5CUser16getTagCharacInfoEijP15TAG_CHARAC_DATA
    // DEMANGLED: CUser::getTagCharacInfo(int,unsigned int,TAG_CHARAC_DATA *)const
    int getTagCharacInfo(int charac_idx,
                         unsigned int charac_no,
                         TAG_CHARAC_DATA* payload) const;

    // IDA_EA: 0x864D4BA
    // IDA_NAME: _ZN5CUser16GetInventoryInfoEP18SIG_LOAD_INVENTORY
    // DEMANGLED: CUser::GetInventoryInfo(SIG_LOAD_INVENTORY *)
    int GetInventoryInfo(SIG_LOAD_INVENTORY* payload);

    // IDA_EA: 0x864D646
    // IDA_NAME: _ZN5CUser12GetInventoryEP18SIG_LOAD_INVENTORY
    // DEMANGLED: CUser::GetInventory(SIG_LOAD_INVENTORY *)
    int GetInventory(SIG_LOAD_INVENTORY* payload);

    // IDA_EA: 0x869067C
    // IDA_NAME: _ZNK5CUser21getTagCharacInventoryEP21TAG_CHARAC_INVEN_DATA
    // DEMANGLED: CUser::getTagCharacInventory(TAG_CHARAC_INVEN_DATA *)const
    int getTagCharacInventory(TAG_CHARAC_INVEN_DATA* payload) const;

    // IDA_EA: 0x864DE58
    // IDA_NAME: _ZNK5CUser15GetDungeonClearEP22SIG_LOAD_DUNGEON_CLEAR
    // DEMANGLED: CUser::GetDungeonClear(SIG_LOAD_DUNGEON_CLEAR *)const
    int GetDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload) const;

    // IDA_EA: 0x867846E
    // IDA_NAME: _ZNK5CUser9GetTicketEP15SIG_LOAD_TICKET
    // DEMANGLED: CUser::GetTicket(SIG_LOAD_TICKET *)const
    SIG_LOAD_TICKET* GetTicket(SIG_LOAD_TICKET* payload) const;

    // IDA_EA: 0x8678282
    // IDA_NAME: _ZN5CUser9AddTicketEib
    // DEMANGLED: CUser::AddTicket(int,bool)
    void AddTicket(int village, bool persist);

    // IDA_EA: 0x86785F4
    // IDA_NAME: _ZN5CUser11CheckTicketEi
    // DEMANGLED: CUser::CheckTicket(int)
    int CheckTicket(int village);

    // IDA_EA: 0x867F1D4
    // IDA_NAME: _ZNK5CUser17GetBlackListCountEP19SIG_REQ_BLACK_COUNT
    // DEMANGLED: CUser::GetBlackListCount(SIG_REQ_BLACK_COUNT *)const
    int GetBlackListCount(SIG_REQ_BLACK_COUNT* payload) const;

    // IDA_EA: 0x867794C
    // IDA_NAME: _ZNK5CUser10GetEtcInfoEP12SIG_LOAD_ETC
    // DEMANGLED: CUser::GetEtcInfo(SIG_LOAD_ETC *)const
    int GetEtcInfo(SIG_LOAD_ETC* payload) const;

    // IDA_EA: 0x864DCF2
    // IDA_NAME: _ZNK5CUser8GetSkillEP14SIG_LOAD_SKILLP15SIG_COMBO_SKILL
    // DEMANGLED: CUser::GetSkill(SIG_LOAD_SKILL *,SIG_COMBO_SKILL *)const
    int GetSkill(SIG_LOAD_SKILL* skill, SIG_COMBO_SKILL* combo_skill) const;

    // IDA_EA: 0x864DB20
    // IDA_NAME: _ZN5CUser8SetSkillEP14SIG_LOAD_SKILLP15SIG_COMBO_SKILL
    // DEMANGLED: CUser::SetSkill(SIG_LOAD_SKILL *,SIG_COMBO_SKILL *)
    int SetSkill(SIG_LOAD_SKILL* skill, SIG_COMBO_SKILL* combo_skill);

    // IDA_EA: 0x8677DD8
    // IDA_NAME: _ZNK5CUser9GetRecordEP12SIG_LOAD_ETC
    // DEMANGLED: CUser::GetRecord(SIG_LOAD_ETC *)const
    SIG_LOAD_ETC* GetRecord(SIG_LOAD_ETC* payload) const;

    // IDA_EA: 0x8676876
    // IDA_NAME: _ZN5CUser13update_recordER14GameResultType
    // DEMANGLED: CUser::update_record(GameResultType &)
    int update_record(GameResultType& record);

    // IDA_EA: 0x86888F8
    // IDA_NAME: _ZN5CUser12SetLimitInfoEP19SIG_LIMIT_ITEM_INFO
    // DEMANGLED: CUser::SetLimitInfo(SIG_LIMIT_ITEM_INFO *)
    void SetLimitInfo(SIG_LIMIT_ITEM_INFO* info);

    // IDA_EA: 0x8688DC4
    // IDA_NAME: _ZNK5CUser17GetLimitItemUsageEP12SIG_LOAD_ETC
    // DEMANGLED: CUser::GetLimitItemUsage(SIG_LOAD_ETC *)const
    void GetLimitItemUsage(SIG_LOAD_ETC* payload) const;

    // IDA_EA: 0x868170C
    // IDA_NAME: _ZN5CUser24SendLogoutToPCRoomServerEv
    // DEMANGLED: CUser::SendLogoutToPCRoomServer(void)
    int SendLogoutToPCRoomServer();

    // IDA_EA: 0x868152C
    // IDA_NAME: _ZN5CUser23SendLoginToPCRoomServerEv
    // DEMANGLED: CUser::SendLoginToPCRoomServer(void)
    int SendLoginToPCRoomServer();

    // IDA_EA: 0x865851C
    // IDA_NAME: _ZN5CUser14ResetCurCharacEv
    // DEMANGLED: CUser::ResetCurCharac(void)
    void* ResetCurCharac();

    // IDA_EA: 0x814AA5E
    // IDA_NAME: _ZN5CUser18getCurCharacQuestWEv
    // DEMANGLED: CUser::getCurCharacQuestW(void)
    UserQuest* getCurCharacQuestW();

    // IDA_EA: 0x819A8A6
    // IDA_NAME: _ZNK5CUser18getCurCharacQuestREv
    // DEMANGLED: CUser::getCurCharacQuestR(void)const
    UserQuest* getCurCharacQuestR() const;

    // IDA_EA: 0x8110DD0
    // IDA_NAME: _ZN5CUser26incPcRoomPlayTimeRewardCntEv
    // DEMANGLED: CUser::incPcRoomPlayTimeRewardCnt(void)
    CUser* incPcRoomPlayTimeRewardCnt();

    // IDA_EA: 0x8110DEC
    // IDA_NAME: _ZNK5CUser26getPcRoomPlayTimeRewardCntEv
    // DEMANGLED: CUser::getPcRoomPlayTimeRewardCnt(void)const
    unsigned char getPcRoomPlayTimeRewardCnt() const;

    // Non-IDA, unregistered ABI state boundary for the old PC-room daily
    // reward counter.  The raw CUser field is deliberately private to the
    // companion bridge translation unit and these helpers are not recovered
    // old-server identities or recovery-ledger entries.
    char getPcroomDailyRewardRemain() const;
    CUser* setPcroomDailyRewardRemain(char remain_reward_count);
    CUser* decrementPcroomDailyRewardRemain();

    // IDA_EA: 0x8110DFE
    // IDA_NAME: _ZN5CUser24resetTotalPcRoomPlayTimeEv
    // DEMANGLED: CUser::resetTotalPcRoomPlayTime(void)
    CUser* resetTotalPcRoomPlayTime();

    // IDA_EA: 0x8110E10
    // IDA_NAME: _ZNK5CUser22getTotalPcRoomPlayTimeEv
    // DEMANGLED: CUser::getTotalPcRoomPlayTime(void)const
    unsigned int getTotalPcRoomPlayTime() const;

    // IDA_EA: 0x8110E1E
    // IDA_NAME: _ZN5CUser31IncCurCharacUsedFatigueQuantityEj
    // DEMANGLED: CUser::IncCurCharacUsedFatigueQuantity(unsigned int)
    CUser* IncCurCharacUsedFatigueQuantity(unsigned int amount);

    // IDA_EA: 0x8110E3A
    // IDA_NAME: _ZN5CUser35IncCurCharacUsedGiftFatigueQuantityEj
    // DEMANGLED: CUser::IncCurCharacUsedGiftFatigueQuantity(unsigned int)
    int IncCurCharacUsedGiftFatigueQuantity(unsigned int amount);

    // IDA_EA: 0x8110E6C
    // IDA_NAME: _ZNK5CUser35GetCurCharacUsedGiftFatigueQuantityEv
    // DEMANGLED: CUser::GetCurCharacUsedGiftFatigueQuantity(void)const
    unsigned int GetCurCharacUsedGiftFatigueQuantity() const;

    // IDA_EA: 0x8110E7A
    // IDA_NAME: _ZN5CUser29SetChangedGiftFatigueQuantityEb
    // DEMANGLED: CUser::SetChangedGiftFatigueQuantity(bool)
    CUser* SetChangedGiftFatigueQuantity(bool changed);

    // IDA_EA: 0x84EC86C
    // IDA_NAME: _ZN5CUser20getDungeonGainedGoldEv
    // DEMANGLED: CUser::getDungeonGainedGold(void)
    void* getDungeonGainedGold();

    // IDA_EA: 0x84EC8F8
    // IDA_NAME: _ZN5CUser22SetAccountLastPlayTimeEl
    // DEMANGLED: CUser::SetAccountLastPlayTime(long)
    CUser* SetAccountLastPlayTime(long current_second);

    // IDA_EA: 0x84ECAD4
    // IDA_NAME: _ZN5CUser19GetTutorialSkipableEv
    // DEMANGLED: CUser::GetTutorialSkipable(void)
    unsigned char GetTutorialSkipable();

    // IDA_EA: 0x84ECAE4
    // IDA_NAME: _ZN5CUser19SetTutorialSkipableEh
    // DEMANGLED: CUser::SetTutorialSkipable(unsigned char)
    CUser* SetTutorialSkipable(unsigned char value);

    // IDA_EA: 0x8568E84
    // IDA_NAME: _ZN5CUser9GetRatingEv
    // DEMANGLED: CUser::GetRating(void)
    float GetRating();

    // IDA_EA: 0x865240C
    // IDA_NAME: _ZN5CUser21UpdateCharacLinkBonusEjjj28ENUM_CHARAC_LINK_BONUS_STATE
    // DEMANGLED: CUser::UpdateCharacLinkBonus(unsigned int,unsigned int,unsigned int,ENUM_CHARAC_LINK_BONUS_STATE)
    void UpdateCharacLinkBonus(unsigned int exp,
                               unsigned int gold,
                               unsigned int linked_charac_no,
                               ENUM_CHARAC_LINK_BONUS_STATE state);

    // IDA_EA: 0x8652564
    // IDA_NAME: _ZN5CUser22giveCharacLinkBonusExpEj
    // DEMANGLED: CUser::giveCharacLinkBonusExp(unsigned int)
    void giveCharacLinkBonusExp(unsigned int exp);

    // IDA_EA: 0x86521EC
    // IDA_NAME: _ZN5CUser24UpdateBreakAwayLuckPointEv
    // DEMANGLED: CUser::UpdateBreakAwayLuckPoint(void)
    void UpdateBreakAwayLuckPoint();

    // IDA_EA: 0x86522EA
    // IDA_NAME: _ZN5CUser30UpdateBreakAwayDungeonClearCntEv
    // DEMANGLED: CUser::UpdateBreakAwayDungeonClearCnt(void)
    void UpdateBreakAwayDungeonClearCnt();

    // IDA_EA: 0x8651FBA
    // IDA_NAME: _ZN5CUser20UpdateDimensionInoutEv
    // DEMANGLED: CUser::UpdateDimensionInout(void)
    void UpdateDimensionInout();

    // IDA_EA: 0x8656C12
    // IDA_NAME: _ZN5CUser20DimensionInoutUpdateEbb
    // DEMANGLED: CUser::DimensionInoutUpdate(bool,bool)
    char DimensionInoutUpdate(bool update_dimension, bool update_ultimate);

    // IDA_EA: 0x868148C
    // IDA_NAME: _ZN5CUser19SaveBackupSkillInfoEP22SIG_LOAD_PREMIUM_LETHE20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CUser::SaveBackupSkillInfo(SIG_LOAD_PREMIUM_LETHE *,ENUM_SKILL_TREE_KIND)
    uint8_t SaveBackupSkillInfo(SIG_LOAD_PREMIUM_LETHE* source,
                                ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x8651DE6
    // IDA_NAME: _ZN5CUser17UpdateBloodRecordEv
    // DEMANGLED: CUser::UpdateBloodRecord(void)
    int UpdateBloodRecord();

    // IDA_EA: 0x8652DDE
    // IDA_NAME: _ZN5CUser17_updateSaveOptionEv
    // DEMANGLED: CUser::_updateSaveOption(void)
    void _updateSaveOption();

    // IDA_EA: 0x868F03C
    // IDA_NAME: _ZNK5CUser18GetCurCharacOptionER15CharacterOption
    // DEMANGLED: CUser::GetCurCharacOption(CharacterOption &)const
    bool GetCurCharacOption(CharacterOption& out) const;

    // IDA_EA: 0x868F726
    // IDA_NAME: _ZN5CUser20updatePcRoomPlayTimeEv
    // DEMANGLED: CUser::updatePcRoomPlayTime(void)
    void updatePcRoomPlayTime();

    // IDA_EA: 0x868F7BA
    // IDA_NAME: _ZN5CUser21collectPcRoomPlayTimeEv
    // DEMANGLED: CUser::collectPcRoomPlayTime(void)
    CUser* collectPcRoomPlayTime();

    // IDA_EA: 0x868F808
    // IDA_NAME: _ZN5CUser21setPcRoomPlayTimeDataEjjh
    // DEMANGLED: CUser::setPcRoomPlayTimeData(unsigned int,unsigned int,unsigned char)
    CUser* setPcRoomPlayTimeData(unsigned int start_time,
                                 unsigned int total_time,
                                 unsigned char reward_count);

    // IDA_EA: 0x868F8D8
    // IDA_NAME: _ZN5CUser21resetInformNoticeFlagEv
    // DEMANGLED: CUser::resetInformNoticeFlag(void)
    void* resetInformNoticeFlag();

    // IDA_EA: 0x868F956
    // IDA_NAME: _ZN5CUser22handleInformNoticeFlagEj
    // DEMANGLED: CUser::handleInformNoticeFlag(unsigned int)
    unsigned int handleInformNoticeFlag(unsigned int flag_index);

    // IDA_EA: 0x86912B0
    // IDA_NAME: _ZN5CUser12getBingoDataEv
    // DEMANGLED: CUser::getBingoData(void)
    BingoData* getBingoData();

    // IDA_EA: 0x8690D9A
    // IDA_NAME: _ZN5CUser23sendDecorationObjectCntEs
    // DEMANGLED: CUser::sendDecorationObjectCnt(short)
    void sendDecorationObjectCnt(short count);

    // IDA_EA: 0x86912BE
    // IDA_NAME: _ZN5CUser13initBingoDataEv
    // DEMANGLED: CUser::initBingoData(void)
    int initBingoData();

    // IDA_EA: 0x869134E
    // IDA_NAME: _ZN5CUser13sendBingoDataEv
    // DEMANGLED: CUser::sendBingoData(void)
    void sendBingoData();

    // IDA_EA: 0x8691470
    // IDA_NAME: _ZN5CUser16sendBingoAddDataEv
    // DEMANGLED: CUser::sendBingoAddData(void)
    void sendBingoAddData();

    // IDA_EA: 0x86914CC
    // IDA_NAME: _ZN5CUser15updateBingoDataEv
    // DEMANGLED: CUser::updateBingoData(void)
    char* updateBingoData();

    // IDA_EA: 0x868E36A
    // IDA_NAME: _ZN5CUser17SaveMercenaryDataEv
    // DEMANGLED: CUser::SaveMercenaryData(void)
    int SaveMercenaryData();

    // IDA_EA: 0x869106C
    // IDA_NAME: _ZN5CUser26updateBlueMarbleEnterCountEv
    // DEMANGLED: CUser::updateBlueMarbleEnterCount(void)
    void updateBlueMarbleEnterCount();

    // IDA_EA: 0x8650AA2
    // IDA_NAME: _ZN5CUser15_SaveCharacDataEv
    // DEMANGLED: CUser::_SaveCharacData(void)
    void _SaveCharacData();

    // IDA_EA: 0x864FE52
    // IDA_NAME: _ZN5CUser13SaveInventoryEv
    // DEMANGLED: CUser::SaveInventory(void)
    int SaveInventory();

    // IDA_EA: 0x864FD80
    // IDA_NAME: _ZN5CUser17_GetSaveInvenBaseEP18SIG_SAVE_INVENTORY
    // DEMANGLED: CUser::_GetSaveInvenBase(SIG_SAVE_INVENTORY *)
    int _GetSaveInvenBase(SIG_SAVE_INVENTORY* payload);

    // IDA_EA: 0x86501C8
    // IDA_NAME: _ZN5CUser17SaveInventoryOnlyEv
    // DEMANGLED: CUser::SaveInventoryOnly(void)
    int SaveInventoryOnly();

    // IDA_EA: 0x86504D8
    // IDA_NAME: _ZN5CUser13SaveMoneyCoinEv
    // DEMANGLED: CUser::SaveMoneyCoin(void)
    int SaveMoneyCoin();

    // IDA_EA: 0x866AF1C
    // IDA_NAME: _ZN5CUser10CheckMoneyEi
    // DEMANGLED: CUser::CheckMoney(int)
    bool CheckMoney(int money);

    // IDA_EA: 0x86505F6
    // IDA_NAME: _ZN5CUser11SaveMileageEv
    // DEMANGLED: CUser::SaveMileage(void)
    void SaveMileage();

    // IDA_EA: 0x8650722
    // IDA_NAME: _ZN5CUser11LoadMileageEv
    // DEMANGLED: CUser::LoadMileage(void)
    void LoadMileage();

    // IDA_EA: 0x865082A
    // IDA_NAME: _ZN5CUser12SendCashDataEb
    // DEMANGLED: CUser::SendCashData(bool)
    void SendCashData(bool clear_values);

    // IDA_EA: 0x868A87A
    // IDA_NAME: _ZN5CUser28SendCeraShopGiftStatisticLogEjjmj
    // DEMANGLED: CUser::SendCeraShopGiftStatisticLog(unsigned int,unsigned int,unsigned long,unsigned int)
    void SendCeraShopGiftStatisticLog(unsigned int receiver_member_id,
                                      unsigned int receiver_charac_no,
                                      unsigned long item_id,
                                      unsigned int item_count);

    // IDA_EA: 0x8650954
    // IDA_NAME: _ZN5CUser10AddMileageEi14eCoinAddReason
    // DEMANGLED: CUser::AddMileage(int,eCoinAddReason)
    void AddMileage(int amount, eCoinAddReason reason);

    // IDA_EA: 0x86509C6
    // IDA_NAME: _ZN5CUser10SetMileageEi
    // DEMANGLED: CUser::SetMileage(int)
    CUser* SetMileage(int mileage);

    // IDA_EA: 0x8650A10
    // IDA_NAME: _ZN5CUser10UseMileageEi
    // DEMANGLED: CUser::UseMileage(int)
    void UseMileage(int amount);

    // IDA_EA: 0x8650A94
    // IDA_NAME: _ZN5CUser10GetMileageEv
    // DEMANGLED: CUser::GetMileage(void)
    int GetMileage();

    // IDA_EA: 0x822FC22
    // IDA_NAME: _ZN5CUser15GetAccountCargoEv
    // DEMANGLED: CUser::GetAccountCargo(void)
    CAccountCargo* GetAccountCargo();

    // IDA_EA: 0x822FC30
    // IDA_NAME: _ZN5CUser19IsExistAccountCargoEv
    // DEMANGLED: CUser::IsExistAccountCargo(void)
    bool IsExistAccountCargo();

    // IDA_EA: 0x86975EE
    // IDA_NAME: _ZNK5CUser29GetChangedGiftFatigueQuantityEv
    // DEMANGLED: CUser::GetChangedGiftFatigueQuantity(void)const
    int GetChangedGiftFatigueQuantity() const;

    // IDA_EA: 0x8697474
    // IDA_NAME: _ZN5CUser6GetETCEs
    // DEMANGLED: CUser::GetETC(short)
    int GetETC(short index);

    // IDA_EA: 0x868FA76
    // IDA_NAME: _ZN5CUser19getInformNoticeFlagEi
    // DEMANGLED: CUser::getInformNoticeFlag(int)
    int getInformNoticeFlag(int index);

    // IDA_EA: 0x868FA8A
    // IDA_NAME: _ZN5CUser24getInformNoticeFlagIndexERSt6vectorIhSaIhEE
    // DEMANGLED: CUser::getInformNoticeFlagIndex(std::vector<unsigned char,std::allocator<unsigned char>> &)
    bool getInformNoticeFlagIndex(std::vector<uint8_t>& indexes);

    // IDA_EA: 0x868FA66
    // IDA_NAME: _ZN5CUser27disableSaveInformNoticeFlagEv
    // DEMANGLED: CUser::disableSaveInformNoticeFlag(void)
    CUser* disableSaveInformNoticeFlag();

    // IDA_EA: 0x868FA46
    // IDA_NAME: _ZN5CUser22isSaveInformNoticeFlagEv
    // DEMANGLED: CUser::isSaveInformNoticeFlag(void)
    int isSaveInformNoticeFlag();

    // IDA_EA: 0x86975E0
    // IDA_NAME: _ZN5CUser31GetCurCharacUsedFatigueQuantityEv
    // DEMANGLED: CUser::GetCurCharacUsedFatigueQuantity(void)
    int GetCurCharacUsedFatigueQuantity();

    // IDA_EA: 0x865459A
    // IDA_NAME: _ZN5CUser21get_skill_command_bufEv
    // DEMANGLED: CUser::get_skill_command_buf(void)
    void* get_skill_command_buf();

    // IDA_EA: 0x8654768
    // IDA_NAME: _ZNK5CUser21get_skill_command_bufEPN18STSkillCommandData9STCommandE
    // DEMANGLED: CUser::get_skill_command_buf(STSkillCommandData::STCommand *)const
    STSkillCommandData::STCommand* get_skill_command_buf(
        STSkillCommandData::STCommand* command) const;

    // IDA_EA: 0x8654924
    // IDA_NAME: _ZN5CUser19setSkillCommandDataEPc
    // DEMANGLED: CUser::setSkillCommandData(char *)
    void* setSkillCommandData(char* source);

    // IDA_EA: 0x86971DE
    // IDA_NAME: _ZN5CUser23isBreakAwayDungeonClearEv
    // DEMANGLED: CUser::isBreakAwayDungeonClear(void)
    int isBreakAwayDungeonClear();

    // IDA_EA: 0x869720A
    // IDA_NAME: _ZN5CUser24isUsedBreakAwayLuckPointEv
    // DEMANGLED: CUser::isUsedBreakAwayLuckPoint(void)
    int isUsedBreakAwayLuckPoint();

    // IDA_EA: 0x86819FA
    // IDA_NAME: _ZN5CUser23_saveConditionEventInfoEjss
    // DEMANGLED: CUser::_saveConditionEventInfo(unsigned int,short,short)
    void _saveConditionEventInfo(unsigned int charac_no,
                                 short event_step,
                                 short reward_step);

    // IDA_EA: 0x8681B3A
    // IDA_NAME: _ZN5CUser28_saveProperDungeonClearCountEjs
    // DEMANGLED: CUser::_saveProperDungeonClearCount(unsigned int,short)
    void _saveProperDungeonClearCount(unsigned int charac_no,
                                      short clear_count);

    // IDA_EA: 0x8692396
    // IDA_NAME: _ZN5CUser16updateComboSkillEv
    // DEMANGLED: CUser::updateComboSkill(void)
    void updateComboSkill();

    // IDA_EA: 0x86773EA
    // IDA_NAME: _ZN5CUser26getCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
    // DEMANGLED: CUser::getCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
    int getCurrentCharacRentalInfo(
        std::vector<InstanceRentalSystem::RentalInfo>& rental_infos,
        int rental_type);

    // IDA_EA: 0x8677516
    // IDA_NAME: _ZN5CUser26setCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
    // DEMANGLED: CUser::setCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
    std::vector<InstanceRentalSystem::RentalInfo>* setCurrentCharacRentalInfo(
        std::vector<InstanceRentalSystem::RentalInfo>& rental_infos,
        int rental_type);

    // IDA_EA: 0x8676DBA
    // IDA_NAME: _ZN5CUser21GetInstanceRentalInfoERSt3mapIjSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS3_EESt4lessIjESaISt4pairIKjS5_EEEi
    // DEMANGLED: CUser::GetInstanceRentalInfo(std::map<unsigned int,std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>>,std::less<unsigned int>,std::allocator<std::pair<unsigned int const,std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>>>>> &,int)
    char GetInstanceRentalInfo(CUserRentalInfoByCharac& rental_infos,
                               int rental_type);

    // IDA_EA: 0x8676E3C
    // IDA_NAME: _ZN5CUser29UpdateCurrentCharacRentalInfoERKN20InstanceRentalSystem10RentOutputEib
    // DEMANGLED: CUser::UpdateCurrentCharacRentalInfo(InstanceRentalSystem::RentOutput const&,int,bool)
    int UpdateCurrentCharacRentalInfo(
        const InstanceRentalSystem::RentOutput& output,
        int rental_type,
        bool clear_current);

    // IDA_EA: 0x8677022
    // IDA_NAME: _ZN5CUser20DeleteRentalItemInfoEi
    // DEMANGLED: CUser::DeleteRentalItemInfo(int)
    void DeleteRentalItemInfo(int rental_type);

    // IDA_EA: 0x8677050
    // IDA_NAME: _ZN5CUser33DeleteCurrentCharacRentalItemInfoEi
    // DEMANGLED: CUser::DeleteCurrentCharacRentalItemInfo(int)
    int DeleteCurrentCharacRentalItemInfo(int rental_type);

    // IDA_EA: 0x8677166
    // IDA_NAME: _ZN5CUser26GetCurrentCharacRentalInfoERSt6vectorIN20InstanceRentalSystem10RentalInfoESaIS2_EEi
    // DEMANGLED: CUser::GetCurrentCharacRentalInfo(std::vector<InstanceRentalSystem::RentalInfo,std::allocator<InstanceRentalSystem::RentalInfo>> &,int)
    int GetCurrentCharacRentalInfo(CUserRentalInfoList& rental_infos,
                                   int rental_type);

    // IDA_EA: 0x8677188
    // IDA_NAME: _ZN5CUser15checkRentedItemEjii
    // DEMANGLED: CUser::checkRentedItem(unsigned int,int,int)
    int checkRentedItem(unsigned int charac_no, int item_id, int rental_type);

    // IDA_EA: 0x86772D2
    // IDA_NAME: _ZNK5CUser22HasCurCharacRentalItemEi
    // DEMANGLED: CUser::HasCurCharacRentalItem(int)const
    int HasCurCharacRentalItem(int rental_type) const;

    // IDA_EA: 0x8677584
    // IDA_NAME: _ZN5CUser32removeRentalItemInfoFromInvenBufEP18SIG_SAVE_INVENTORYi
    // DEMANGLED: CUser::removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY *,int)
    int removeRentalItemInfoFromInvenBuf(SIG_SAVE_INVENTORY* payload,
                                         int rental_type);

    // IDA_EA: 0x867776C
    // IDA_NAME: _ZN5CUser36removeRentalItemInfoFromInvenLoadBufEP18SIG_LOAD_INVENTORYi
    // DEMANGLED: CUser::removeRentalItemInfoFromInvenLoadBuf(SIG_LOAD_INVENTORY *,int)
    int removeRentalItemInfoFromInvenLoadBuf(SIG_LOAD_INVENTORY* payload,
                                             int rental_type);

    // IDA_EA: 0x868C3BC
    // IDA_NAME: _ZN5CUser25delete_ontime_reward_listEv
    // DEMANGLED: CUser::delete_ontime_reward_list(void)
    int delete_ontime_reward_list();

    // IDA_EA: 0x868AD88
    // IDA_NAME: _ZN5CUser19clearInitQuestPointEv
    // DEMANGLED: CUser::clearInitQuestPoint(void)
    CQuestShop* clearInitQuestPoint();

    // IDA_EA: 0x868ADC2
    // IDA_NAME: _ZN5CUser17incInitQuestPointEi
    // DEMANGLED: CUser::incInitQuestPoint(int)
    CQuestShop* incInitQuestPoint(int quest_point);

    // IDA_EA: 0x868ADFC
    // IDA_NAME: _ZN5CUser17setInitQuestPointEv
    // DEMANGLED: CUser::setInitQuestPoint(void)
    bool setInitQuestPoint();

    // IDA_EA: 0x868AF66
    // IDA_NAME: _ZN5CUser13incQuestPieceEiib
    // DEMANGLED: CUser::incQuestPiece(int,int,bool)
    // IDA_EA: 0x868AC66
    // IDA_NAME: _ZN5CUser13incQuestPointEiib
    // DEMANGLED: CUser::incQuestPoint(int,int,bool)
    CQuestShop* incQuestPoint(int quest_point,
                              int quest_index,
                              bool suppress_packet);

    void incQuestPiece(int quest_piece,
                       int quest_index,
                       bool suppress_packet);

    // IDA_EA: 0x868AFD0
    // IDA_NAME: _ZN5CUser15clearQuestPieceEv
    // DEMANGLED: CUser::clearQuestPiece(void)
    CQuestShop* clearQuestPiece();

    // IDA_EA: 0x81BC11E
    // IDA_NAME: _ZN5CUser24set_ontime_last_recv_idxEi
    // DEMANGLED: CUser::set_ontime_last_recv_idx(int)
    CUser* set_ontime_last_recv_idx(int ontime_id);

    // IDA_EA: 0x81BC130
    // IDA_NAME: _ZNK5CUser24get_ontime_last_recv_idxEv
    // DEMANGLED: CUser::get_ontime_last_recv_idx(void)const
    int get_ontime_last_recv_idx() const;

    // IDA_EA: 0x81BC13E
    // IDA_NAME: _ZN5CUser31set_update_ontime_last_recv_idxEv
    // DEMANGLED: CUser::set_update_ontime_last_recv_idx(void)
    CUser* set_update_ontime_last_recv_idx();

    // IDA_EA: 0x8681C30
    // IDA_NAME: _ZN5CUser23InsertOnTimeEventRewardEjj
    // DEMANGLED: CUser::InsertOnTimeEventReward(unsigned int,unsigned int)
    int InsertOnTimeEventReward(unsigned int item_id,
                                unsigned int item_count);

    // IDA_EA: 0x8652DD8
    // IDA_NAME: _ZN5CUser22_updateNPCRelationshipEv
    // DEMANGLED: CUser::_updateNPCRelationship(void)
    void _updateNPCRelationship();

    // IDA_EA: 0x8681218
    // IDA_NAME: _ZN5CUser29resetNPCRelationShipDailyDataEv
    // DEMANGLED: CUser::resetNPCRelationShipDailyData(void)
    void resetNPCRelationShipDailyData();

    // IDA_EA: 0x868121E
    // IDA_NAME: _ZN5CUser24sendNPCRelationShipFavorEv
    // DEMANGLED: CUser::sendNPCRelationShipFavor(void)
    void sendNPCRelationShipFavor();

    // IDA_EA: 0x86975CE
    // IDA_NAME: _ZN5CUser33ResetCurCharacUsedFatigueQuantityEv
    // DEMANGLED: CUser::ResetCurCharacUsedFatigueQuantity(void)
    CUser* ResetCurCharacUsedFatigueQuantity();

    // IDA_EA: 0x85BFD1A
    // IDA_NAME: _ZN5CUser24setBreakAwayDungeonClearEb
    // DEMANGLED: CUser::setBreakAwayDungeonClear(bool)
    CUser* setBreakAwayDungeonClear(bool value);

    // IDA_EA: 0x822FC6E
    // IDA_NAME: _ZN5CUser27getBreakAwayDungeonClearCntEv
    // DEMANGLED: CUser::getBreakAwayDungeonClearCnt(void)
    unsigned int getBreakAwayDungeonClearCnt();

    // IDA_EA: 0x86971EE
    // IDA_NAME: _ZN5CUser25setUsedBreakAwayLuckPointEb
    // DEMANGLED: CUser::setUsedBreakAwayLuckPoint(bool)
    CUser* setUsedBreakAwayLuckPoint(bool value);

    // IDA_EA: 0x869736E
    // IDA_NAME: _ZN5CUser15getLinkCharacNoEc
    // DEMANGLED: CUser::getLinkCharacNo(char)
    unsigned int getLinkCharacNo(char slot);

    // IDA_EA: 0x82300E8
    // IDA_NAME: _ZN5CUser16get_charac_countEv
    // DEMANGLED: CUser::get_charac_count(void)
    int get_charac_count();

    // IDA_EA: 0x84ECD52
    // IDA_NAME: _ZN5CUser24isCharacLinkBonusSettingEc
    // DEMANGLED: CUser::isCharacLinkBonusSetting(char)
    int isCharacLinkBonusSetting(char slot);

    // IDA_EA: 0x84ECDB6
    // IDA_NAME: _ZN5CUser25setCharacLinkBonusSettingEib
    // DEMANGLED: CUser::setCharacLinkBonusSetting(int,bool)
    CCharacterView* setCharacLinkBonusSetting(int slot, bool enabled);

    // IDA_EA: 0x8652B06
    // IDA_NAME: _ZN5CUser27decideLinkCharacSlotIndexesEci
    // DEMANGLED: CUser::decideLinkCharacSlotIndexes(char,int)
    int decideLinkCharacSlotIndexes(char slot, int charac_count);

    // IDA_EA: 0x8652B90
    // IDA_NAME: _ZN5CUser19checkCharacLinkTypeEci
    // DEMANGLED: CUser::checkCharacLinkType(char,int)
    int checkCharacLinkType(char slot, int charac_count);

    // 0828 CMD309/CMD310 native owners.  The request's two selector bytes
    // are each character's grow_type; the durable relationship itself is
    // the existing charac_info.link_charac_no pair.
    bool ConnectLinkCharacters(uint8_t first_slot,
                               uint8_t second_slot,
                               uint8_t first_grow_type,
                               uint8_t second_grow_type);
    bool DisconnectLinkCharacters(uint8_t first_slot,
                                  uint8_t second_slot);
    void RestoreLinkCharactersFromDatabase();
    void ApplyCharacLinkDatabaseResult(
        const charac_link_db::Message& response);

    // IDA_EA: 0x85DFA64
    // IDA_NAME: _ZN5CUser30set_guildwar_point_per_pvpplayEi
    // DEMANGLED: CUser::set_guildwar_point_per_pvpplay(int)
    CUser* set_guildwar_point_per_pvpplay(int value);

    // IDA_EA: 0x865CF6C
    // IDA_NAME: _ZN5CUser32reset_pvp_masterid_walkingout_meEv
    // DEMANGLED: CUser::reset_pvp_masterid_walkingout_me(void)
    int reset_pvp_masterid_walkingout_me();

    // IDA_EA: 0x865CF42
    // IDA_NAME: _ZN5CUser33insert_pvp_masterid_walkingout_meEj
    // DEMANGLED: CUser::insert_pvp_masterid_walkingout_me(unsigned int)
    int insert_pvp_masterid_walkingout_me(unsigned int master_id);

    // IDA_EA: 0x868F6FA
    // IDA_NAME: _ZN5CUser16setForceDropFlagEb
    // DEMANGLED: CUser::setForceDropFlag(bool)
    CUser* setForceDropFlag(bool enabled);

    // IDA_EA: 0x86786BE
    // IDA_NAME: _ZN5CUser14giveup_panaltyEv
    // DEMANGLED: CUser::giveup_panalty(void)
    void giveup_panalty();

    // IDA_EA: 0x868F1C0
    // IDA_NAME: _ZN5CUser18NextCharacMoneyLogEi
    // DEMANGLED: CUser::NextCharacMoneyLog(int)
    int NextCharacMoneyLog(int timer);

    // IDA_EA: 0x868F3BA
    // IDA_NAME: _ZN5CUser21exceptedStateMoneyLogEv
    // DEMANGLED: CUser::exceptedStateMoneyLog(void)
    bool exceptedStateMoneyLog();

    // IDA_EA: 0x868F266
    // IDA_NAME: _ZN5CUser12AddMoneyPlusEi
    // DEMANGLED: CUser::AddMoneyPlus(int)
    int* AddMoneyPlus(int money);

    // IDA_EA: 0x868F310
    // IDA_NAME: _ZN5CUser13AddMoneyMinusEi
    // DEMANGLED: CUser::AddMoneyMinus(int)
    int* AddMoneyMinus(int money);

    // IDA_EA: 0x8652F0C
    // IDA_NAME: _ZN5CUser14WorkPerFiveMinEv
    // DEMANGLED: CUser::WorkPerFiveMin(void)
    void WorkPerFiveMin();

    // IDA_EA: 0x8653196
    // IDA_NAME: _ZN5CUser13WorkPerTenMinEv
    // DEMANGLED: CUser::WorkPerTenMin(void)
    char WorkPerTenMin();

    // IDA_EA: 0x8101056
    // IDA_NAME: _ZN5CUser12GetSsnStringEv
    // DEMANGLED: CUser::GetSsnString(void)
    char* GetSsnString();

    // IDA_EA: 0x867EC12
    // IDA_NAME: _ZN5CUser6getSexEv
    // DEMANGLED: CUser::getSex(void)
    int getSex();

    // IDA_EA: 0x80DD5A4
    // IDA_NAME: _ZN5CUser22GetAccountLastPlayTimeEv
    // DEMANGLED: CUser::GetAccountLastPlayTime(void)
    int GetAccountLastPlayTime();

    // IDA_EA: 0x8690098
    // IDA_NAME: _ZN5CUser16setTagCharacInfoEijP15TAG_CHARAC_DATA
    // DEMANGLED: CUser::setTagCharacInfo(int,unsigned int,TAG_CHARAC_DATA *)
    int setTagCharacInfo(int slot,
                         unsigned int charac_no,
                         TAG_CHARAC_DATA* data);

    // IDA_EA: 0x8690226
    // IDA_NAME: _ZN5CUser17setTagCharacInvenEP21TAG_CHARAC_INVEN_DATA
    // DEMANGLED: CUser::setTagCharacInven(TAG_CHARAC_INVEN_DATA *)
    int setTagCharacInven(TAG_CHARAC_INVEN_DATA* data);

    // IDA_EA: 0x864A956
    // IDA_NAME: _ZN5CUser31GetGuildIdBeforeDeleteCharacterEi
    // DEMANGLED: CUser::GetGuildIdBeforeDeleteCharacter(int)
    int GetGuildIdBeforeDeleteCharacter(int slot);

    // IDA_EA: 0x868E672
    // IDA_NAME: _ZN5CUser25RemoveCharacMercenaryInfoEj
    // DEMANGLED: CUser::RemoveCharacMercenaryInfo(unsigned int)
    int RemoveCharacMercenaryInfo(unsigned int charac_no);

    // IDA_EA: 0x868BF14
    // IDA_NAME: _ZN5CUser32select_charac_set_visible_valuesEc
    // DEMANGLED: CUser::select_charac_set_visible_values(char)
    int select_charac_set_visible_values(char values);

    // IDA_EA: 0x868D780
    // IDA_NAME: _ZN5CUser33decide_growth_power_reward_systemEv
    // DEMANGLED: CUser::decide_growth_power_reward_system(void)
    void decide_growth_power_reward_system();

    // IDA_EA: 0x868DACE
    // IDA_NAME: _ZN5CUser27set_growth_power_exp_rewardEi
    // DEMANGLED: CUser::set_growth_power_exp_reward(int)
    void set_growth_power_exp_reward(int charac_level);
    // IDA_EA: 0x868DC64
    // IDA_NAME: _ZN5CUser27get_growth_power_exp_rewardEv
    // DEMANGLED: CUser::get_growth_power_exp_reward(void)
    STExpReward get_growth_power_exp_reward();

    // IDA_EA: 0x8689968
    // IDA_NAME: _ZN5CUser20SendEventBringUpInfoEv
    // DEMANGLED: CUser::SendEventBringUpInfo(void)
    void SendEventBringUpInfo();

    // IDA_EA: 0x8692F0A
    // IDA_NAME: _ZN5CUser16setComboSkillSetEb
    // DEMANGLED: CUser::setComboSkillSet(bool)
    CUser* setComboSkillSet(bool enabled);

    // IDA_EA: 0x8692F26
    // IDA_NAME: _ZN5CUser15isComboSkillSetEv
    // DEMANGLED: CUser::isComboSkillSet(void)
    int isComboSkillSet();

    // IDA_EA: 0x8692EF2
    // IDA_NAME: _ZN5CUser19GetCharacHotKeyTypeEi
    // DEMANGLED: CUser::GetCharacHotKeyType(int)
    bool GetCharacHotKeyType(int character_job);

    // IDA_EA: 0x8692E9C
    // IDA_NAME: _ZN5CUser31CheckLoadHotKeyCashedCharacTypeEii
    // DEMANGLED: CUser::CheckLoadHotKeyCashedCharacType(int,int)
    bool CheckLoadHotKeyCashedCharacType(int character_job,
                                         int cached_character_job);

    // IDA_EA: 0x867ED68
    // IDA_NAME: _ZN5CUser11is_fightingEv
    // DEMANGLED: CUser::is_fighting(void)
    int is_fighting();

    // IDA_EA: 0x8692D2A
    // IDA_NAME: _ZN5CUser20LoadCharacTypeHotKeyEii
    // DEMANGLED: CUser::LoadCharacTypeHotKey(int,int)
    void LoadCharacTypeHotKey(int character_job, int cached_job);

    // IDA_EA: 0x868AC24
    // IDA_NAME: _ZN5CUser12sendCharacQpEv
    // DEMANGLED: CUser::sendCharacQp(void)
    void sendCharacQp();

    // IDA_EA: 0x86924AA
    // IDA_NAME: _ZN5CUser18sendComboSkillListEv
    // DEMANGLED: CUser::sendComboSkillList(void)
    void sendComboSkillList();

    // IDA_EA: 0x866C46A
    // IDA_NAME: _ZN5CUser15send_skill_infoEv
    // DEMANGLED: CUser::send_skill_info(void)
    void send_skill_info();

    // IDA_EA: 0x8665400
    // IDA_NAME: _ZN5CUser16givePvPSkillTreeEibi
    // DEMANGLED: CUser::givePvPSkillTree(int,bool,int)
    bool givePvPSkillTree(int pvp_grade,
                          bool use_pvp_skill_tree,
                          int target_tree);

    // IDA_EA: 0x8654B94
    // IDA_NAME: _ZN5CUser20findSkillCommandDataEiR18STSkillCommandData
    // DEMANGLED: CUser::findSkillCommandData(int,STSkillCommandData &)
    bool findSkillCommandData(int skill_index, STSkillCommandData& out);

    // IDA_EA: 0x8654E6A
    // IDA_NAME: _ZN5CUser20findSkillCommandDataEiR18STSkillCommandDatab
    // DEMANGLED: CUser::findSkillCommandData(int,STSkillCommandData &,bool)
    bool findSkillCommandData(int skill_index,
                              STSkillCommandData& out,
                              bool use_saved_only);

    // IDA_EA: 0x86973F8
    // IDA_NAME: _ZN5CUser26isLinkCharacDisconnectFlagEv
    // DEMANGLED: CUser::isLinkCharacDisconnectFlag(void)
    int isLinkCharacDisconnectFlag();

    // IDA_EA: 0x8697408
    // IDA_NAME: _ZN5CUser30getDisconnectLinkCharacSlotIdxEv
    // DEMANGLED: CUser::getDisconnectLinkCharacSlotIdx(void)
    int getDisconnectLinkCharacSlotIdx();

    // IDA_EA: 0x86973DC
    // IDA_NAME: _ZN5CUser27setLinkCharacDisconnectFlagEb
    // DEMANGLED: CUser::setLinkCharacDisconnectFlag(bool)
    CUser* setLinkCharacDisconnectFlag(bool value);

    // IDA_EA: 0x8652C8E
    // IDA_NAME: _ZN5CUser22doLinkCharacDisconnectEv
    // DEMANGLED: CUser::doLinkCharacDisconnect(void)
    void doLinkCharacDisconnect();

    // IDA_EA: 0x8651628
    // IDA_NAME: _ZN5CUser40UpdateCurCharacVisibleToCharacViewMemoryEv
    // DEMANGLED: CUser::UpdateCurCharacVisibleToCharacViewMemory(void)
    void UpdateCurCharacVisibleToCharacViewMemory();

    // IDA_EA: 0x86489F4
    // IDA_NAME: _ZN5CUser10DisConnSigE11DISCONN_SIGbi
    // DEMANGLED: CUser::DisConnSig(DISCONN_SIG,bool,int)
    void DisConnSig(DISCONN_SIG source, bool flag, int arg);

    // IDA_EA: 0x86487EA
    // IDA_NAME: _ZN5CUser17prepareDisconnectEv
    // DEMANGLED: CUser::prepareDisconnect(void)
    void prepareDisconnect();

    // IDA_EA: 0x8686FEE
    // IDA_NAME: _ZN5CUser24ReturnToSelectCharacListEb
    // DEMANGLED: CUser::ReturnToSelectCharacList(bool)
    void ReturnToSelectCharacList(bool send_packet);

    // IDA_EA: 0x80FDF44
    // IDA_NAME: _ZN5CUser30set_multiboxLotteryItemFailCntEi
    // DEMANGLED: CUser::set_multiboxLotteryItemFailCnt(int)
    CUser* set_multiboxLotteryItemFailCnt(int count);

    // IDA_EA: 0x80FDF56
    // IDA_NAME: _ZN5CUser30inc_multiboxLotteryItemFailCntEv
    // DEMANGLED: CUser::inc_multiboxLotteryItemFailCnt(void)
    int inc_multiboxLotteryItemFailCnt();

    // IDA_EA: 0x8101048
    // IDA_NAME: _ZN5CUser11GetUserNameEv
    // DEMANGLED: CUser::GetUserName(void)
    char* GetUserName();

    // IDA_EA: 0x8101064
    // IDA_NAME: _ZN5CUser12GetUserEMailEv
    // DEMANGLED: CUser::GetUserEMail(void)
    char* GetUserEMail();

    // IDA_EA: 0x811BAB0
    // IDA_NAME: _ZN5CUser22setPcRoomPlayStartTimeEj
    // DEMANGLED: CUser::setPcRoomPlayStartTime(unsigned int)
    CUser* setPcRoomPlayStartTime(unsigned int start_time);

    // IDA_EA: 0x811EDEE
    // IDA_NAME: _ZNK5CUser16IsGameMasterModeEv
    // DEMANGLED: CUser::IsGameMasterMode(void)const
    int IsGameMasterMode() const;

    // IDA_EA: 0x814AA8C
    // IDA_NAME: _ZN5CUser14GetRevivalTimeEv
    // DEMANGLED: CUser::GetRevivalTime(void)
    int GetRevivalTime();

    // IDA_EA: 0x817A1B4
    // IDA_NAME: _ZN5CUser7SetCeraEj
    // DEMANGLED: CUser::SetCera(unsigned int)
    CUser* SetCera(unsigned int cera);

    // IDA_EA: 0x817A1C6
    // IDA_NAME: _ZN5CUser7UseCeraEj
    // DEMANGLED: CUser::UseCera(unsigned int)
    int UseCera(unsigned int cera);

    // IDA_EA: 0x817A208
    // IDA_NAME: _ZN5CUser13resetUsedCeraEv
    // DEMANGLED: CUser::resetUsedCera(void)
    CUser* resetUsedCera();

    // IDA_EA: 0x817A21A
    // IDA_NAME: _ZN5CUser11addUsedCeraEj
    // DEMANGLED: CUser::addUsedCera(unsigned int)
    CUser* addUsedCera(unsigned int cera);

    // IDA_EA: 0x817A236
    // IDA_NAME: _ZN5CUser11getUsedCeraEv
    // DEMANGLED: CUser::getUsedCera(void)
    int getUsedCera();

    // IDA_EA: 0x822FC5C
    // IDA_NAME: _ZN5CUser27setBreakAwayDungeonClearCntEi
    // DEMANGLED: CUser::setBreakAwayDungeonClearCnt(int)
    CUser* setBreakAwayDungeonClearCnt(int count);

    // IDA_EA: 0x822FC7C
    // IDA_NAME: _ZN5CUser21setBreakAwayQuestTimeEj
    // DEMANGLED: CUser::setBreakAwayQuestTime(unsigned int)
    CUser* setBreakAwayQuestTime(unsigned int time);

    // IDA_EA: 0x822FCB8
    // IDA_NAME: _ZN5CUser13getBackupSeedEv
    // DEMANGLED: CUser::getBackupSeed(void)
    int getBackupSeed();

    // IDA_EA: 0x822FCF2
    // IDA_NAME: _ZN5CUser30get_multiboxLotteryItemFailCntEv
    // DEMANGLED: CUser::get_multiboxLotteryItemFailCnt(void)
    int get_multiboxLotteryItemFailCnt();

    // IDA_EA: 0x822FD00
    // IDA_NAME: _ZN5CUser20set_local_ip_addressEi
    // DEMANGLED: CUser::set_local_ip_address(int)
    CUser* set_local_ip_address(int address);

    // IDA_EA: 0x822FD12
    // IDA_NAME: _ZN5CUser13getWebAddressEv
    // DEMANGLED: CUser::getWebAddress(void)
    char* getWebAddress();

    // IDA_EA: 0x822FD9C
    // IDA_NAME: _ZN5CUser22IncChecksumAnswerCountEv
    // DEMANGLED: CUser::IncChecksumAnswerCount(void)
    CUser* IncChecksumAnswerCount();

    // IDA_EA: 0x867EA5C
    // IDA_NAME: _ZN5CUser22checkCodeChecksumRangeERK18CodeHackChecksumExRK20CodeHackCheckRangeEx
    // DEMANGLED: CUser::checkCodeChecksumRange(CodeHackChecksumEx const&,CodeHackCheckRangeEx const&)
    int checkCodeChecksumRange(const CodeHackChecksumEx& checksum,
                               const CodeHackCheckRangeEx& range);

    // IDA_EA: 0x867EBBA
    // IDA_NAME: _ZN5CUser17checkCodeChecksumERK16CodeHackChecksumPc
    // DEMANGLED: CUser::checkCodeChecksum(CodeHackChecksum const&,char *)
    int checkCodeChecksum(const CodeHackChecksum& checksum, char* dest);

    // IDA_EA: 0x822FDF4
    // IDA_NAME: _ZN5CUser15GetSeedFromDateEv
    // DEMANGLED: CUser::GetSeedFromDate(void)
    int GetSeedFromDate();

    // IDA_EA: 0x822FE10
    // IDA_NAME: _ZN5CUser12SetLoginTimeEl
    // DEMANGLED: CUser::SetLoginTime(long)
    CUser* SetLoginTime(long login_time);

    // IDA_EA: 0x822FE22
    // IDA_NAME: _ZN5CUser16SetLogInOutStateEh
    // DEMANGLED: CUser::SetLogInOutState(unsigned char)
    CUser* SetLogInOutState(unsigned char state);

    // IDA_EA: 0x822FEA4
    // IDA_NAME: _ZN5CUser16SetGMUpgradeModeE20ENUM_GM_ITEM_UPGRADE
    // DEMANGLED: CUser::SetGMUpgradeMode(ENUM_GM_ITEM_UPGRADE)
    CUser* SetGMUpgradeMode(ENUM_GM_ITEM_UPGRADE mode);

    // IDA_EA: 0x868AA54
    // IDA_NAME: _ZN5CUser19UpdateEquipslotFlagEc
    // DEMANGLED: CUser::UpdateEquipslotFlag(char)
    int UpdateEquipslotFlag(char flag);

    // IDA_EA: 0x82300BE
    // IDA_NAME: _ZN5CUser6SetAgeEj
    // DEMANGLED: CUser::SetAge(unsigned int)
    CUser* SetAge(unsigned int age);

    // IDA_EA: 0x8230152
    // IDA_NAME: _ZN5CUser17decre_check_countEv
    // DEMANGLED: CUser::decre_check_count(void)
    CUser* decre_check_count();

    // IDA_EA: 0x82301BC
    // IDA_NAME: _ZN5CUser26DecreaseGuildPowerWarPointEj
    // DEMANGLED: CUser::DecreaseGuildPowerWarPoint(unsigned int)
    CUser* DecreaseGuildPowerWarPoint(unsigned int point);

    // IDA_EA: 0x82301EC
    // IDA_NAME: _ZN5CUser21GetGuildCargoCapacityEv
    // DEMANGLED: CUser::GetGuildCargoCapacity(void)
    int GetGuildCargoCapacity();

    // IDA_EA: 0x82301FA
    // IDA_NAME: _ZN5CUser15SetDebugCommandEi
    // DEMANGLED: CUser::SetDebugCommand(int)
    CUser* SetDebugCommand(int command);

    // IDA_EA: 0x8230246
    // IDA_NAME: _ZN5CUser11OnRecvEventEv
    // DEMANGLED: CUser::OnRecvEvent(void)
    CUser* OnRecvEvent();

    // IDA_EA: 0x8230256
    // IDA_NAME: _ZN5CUser19SellItemTimeCheckerEjj
    // DEMANGLED: CUser::SellItemTimeChecker(unsigned int,unsigned int)
    int SellItemTimeChecker(unsigned int current_tick,
                            unsigned int interval);

    // IDA_EA: 0x82307BA
    // IDA_NAME: _ZN5CUser18getLastLotteryTimeEv
    // DEMANGLED: CUser::getLastLotteryTime(void)
    int getLastLotteryTime();

    // IDA_EA: 0x82307C8
    // IDA_NAME: _ZN5CUser18setLastLotteryTimeEm
    // DEMANGLED: CUser::setLastLotteryTime(unsigned long)
    CUser* setLastLotteryTime(unsigned long lottery_time);

    // IDA_EA: 0x82307DA
    // IDA_NAME: _ZN5CUser20resetCleanpadFailCntEv
    // DEMANGLED: CUser::resetCleanpadFailCnt(void)
    CUser* resetCleanpadFailCnt();

    // IDA_EA: 0x82307EC
    // IDA_NAME: _ZN5CUser21isCleanPadVerifyLimitEv
    // DEMANGLED: CUser::isCleanPadVerifyLimit(void)
    bool isCleanPadVerifyLimit();

    // IDA_EA: 0x823082A
    // IDA_NAME: _ZN5CUser21IncreaseUsedCoinCountEv
    // DEMANGLED: CUser::IncreaseUsedCoinCount(void)
    CUser* IncreaseUsedCoinCount();

    // IDA_EA: 0x8230844
    // IDA_NAME: _ZN5CUser31SetDeleteEventDungeonInvitationEb
    // DEMANGLED: CUser::SetDeleteEventDungeonInvitation(bool)
    CUser* SetDeleteEventDungeonInvitation(bool remove_invitation);

    // IDA_EA: 0x823089C
    // IDA_NAME: _ZNK5CUser25getCharacAntibotSerialNumEv
    // DEMANGLED: CUser::getCharacAntibotSerialNum(void)const
    int getCharacAntibotSerialNum() const;

    // IDA_EA: 0x82308AA
    // IDA_NAME: _ZN5CUser25setCharacAntibotSerialNumEi
    // DEMANGLED: CUser::setCharacAntibotSerialNum(int)
    CUser* setCharacAntibotSerialNum(int serial_number);

    // IDA_EA: 0x82308BC
    // IDA_NAME: _ZN5CUser23InitReceivedRequestTypeEv
    // DEMANGLED: CUser::InitReceivedRequestType(void)
    CUser* InitReceivedRequestType();

    // IDA_EA: 0x82308CC
    // IDA_NAME: _ZN5CUser22SetReceivedRequestTypeEc
    // DEMANGLED: CUser::SetReceivedRequestType(char)
    CUser* SetReceivedRequestType(char request_type);

    // IDA_EA: 0x82308E8
    // IDA_NAME: _ZNK5CUser22GetReceivedRequestTypeEv
    // DEMANGLED: CUser::GetReceivedRequestType(void)const
    int GetReceivedRequestType() const;

    // IDA_EA: 0x826532E
    // IDA_NAME: _ZN5CUser15GetDebugCommandEv
    // DEMANGLED: CUser::GetDebugCommand(void)
    int GetDebugCommand();

    // IDA_EA: 0x8275684
    // IDA_NAME: _ZN5CUser16GetNonClientFlagEv
    // DEMANGLED: CUser::GetNonClientFlag(void)
    int GetNonClientFlag();

    // IDA_EA: 0x8275694
    // IDA_NAME: _ZN5CUser16SetNonClientFlagEb
    // DEMANGLED: CUser::SetNonClientFlag(bool)
    CUser* SetNonClientFlag(bool flag);

    // IDA_EA: 0x82756B0
    // IDA_NAME: _ZN5CUser19GetNonClientRandIntEv
    // DEMANGLED: CUser::GetNonClientRandInt(void)
    int GetNonClientRandInt();

    // IDA_EA: 0x82756BE
    // IDA_NAME: _ZN5CUser19SetNonClientRandIntEi
    // DEMANGLED: CUser::SetNonClientRandInt(int)
    CUser* SetNonClientRandInt(int random_value);

    // IDA_EA: 0x827DF1E
    // IDA_NAME: _ZN5CUser18incCharacSlotLimitEh
    // DEMANGLED: CUser::incCharacSlotLimit(unsigned char)
    CUser* incCharacSlotLimit(unsigned char count);

    // IDA_EA: 0x828715C
    // IDA_NAME: _ZN5CUser14setStdDropRateEi
    // DEMANGLED: CUser::setStdDropRate(int)
    CUser* setStdDropRate(int rate);

    // IDA_EA: 0x8288E7E
    // IDA_NAME: _ZN5CUser13isRestingUserEv
    // DEMANGLED: CUser::isRestingUser(void)
    int isRestingUser();

    // IDA_EA: 0x82A676A
    // IDA_NAME: _ZN5CUser27SetSaveRentalInfoToExchangeEb
    // DEMANGLED: CUser::SetSaveRentalInfoToExchange(bool)
    CUser* SetSaveRentalInfoToExchange(bool save);

    // IDA_EA: 0x82A6786
    // IDA_NAME: _ZN5CUser22IsReturnUserFirstLoginEv
    // DEMANGLED: CUser::IsReturnUserFirstLogin(void)
    int IsReturnUserFirstLogin();

    // IDA_EA: 0x82A681E
    // IDA_NAME: _ZN5CUser10SetSlotIDXEs
    // DEMANGLED: CUser::SetSlotIDX(short)
    CUser* SetSlotIDX(short slot_index);

    // IDA_EA: 0x82A684C
    // IDA_NAME: _ZN5CUser10SetIncreIDEs
    // DEMANGLED: CUser::SetIncreID(short)
    CUser* SetIncreID(short increment_id);

    // IDA_EA: 0x82A686A
    // IDA_NAME: _ZN5CUser13set_unique_idEt
    // DEMANGLED: CUser::set_unique_id(unsigned short)
    CUser* set_unique_id(unsigned short unique_id);

    // IDA_EA: 0x82A6888
    // IDA_NAME: _ZN5CUser19getPunishTradeAlertEv
    // DEMANGLED: CUser::getPunishTradeAlert(void)
    int getPunishTradeAlert();

    // IDA_EA: 0x82F0F36
    // IDA_NAME: _ZN5CUser17GetAutoClientRateEv
    // DEMANGLED: CUser::GetAutoClientRate(void)
    int GetAutoClientRate();

    // IDA_EA: 0x833F4AA
    // IDA_NAME: _ZN5CUser22set_cera_item_order_noEj
    // DEMANGLED: CUser::set_cera_item_order_no(unsigned int)
    CUser* set_cera_item_order_no(unsigned int order_number);

    // IDA_EA: 0x8469A5C
    // IDA_NAME: _ZN5CUser23setChattingMessageCountEi
    // DEMANGLED: CUser::setChattingMessageCount(int)
    CUser* setChattingMessageCount(int count);

    // IDA_EA: 0x84EC61E
    // IDA_NAME: _ZN5CUser13SetFirstLoginEv
    // DEMANGLED: CUser::SetFirstLogin(void)
    CUser* SetFirstLogin();

    // IDA_EA: 0x84EC62E
    // IDA_NAME: _ZN5CUser21getBreakAwayQuestTimeEv
    // DEMANGLED: CUser::getBreakAwayQuestTime(void)
    int getBreakAwayQuestTime();

    // IDA_EA: 0x84EC63C
    // IDA_NAME: _ZN5CUser22setBreakAwayAccureCeraEi
    // DEMANGLED: CUser::setBreakAwayAccureCera(int)
    CUser* setBreakAwayAccureCera(int cera);

    // IDA_EA: 0x84EC64E
    // IDA_NAME: _ZN5CUser21isBreakAwayRewardDataEv
    // DEMANGLED: CUser::isBreakAwayRewardData(void)
    int isBreakAwayRewardData();

    // IDA_EA: 0x84EC66C
    // IDA_NAME: _ZN5CUser24getBreakAwayRewardItemIdEv
    // DEMANGLED: CUser::getBreakAwayRewardItemId(void)
    int getBreakAwayRewardItemId();

    // IDA_EA: 0x84EC67A
    // IDA_NAME: _ZN5CUser26getBreakAwayRewardAItemCntEv
    // DEMANGLED: CUser::getBreakAwayRewardAItemCnt(void)
    int getBreakAwayRewardAItemCnt();

    // IDA_EA: 0x84EC688
    // IDA_NAME: _ZN5CUser27getBreakAwayRewardItemOrderEv
    // DEMANGLED: CUser::getBreakAwayRewardItemOrder(void)
    int getBreakAwayRewardItemOrder();

    // IDA_EA: 0x84EC696
    // IDA_NAME: _ZN5CUser24setBreakAwayRewardItemIdEi
    // DEMANGLED: CUser::setBreakAwayRewardItemId(int)
    CUser* setBreakAwayRewardItemId(int item_id);

    // IDA_EA: 0x84EC6A8
    // IDA_NAME: _ZN5CUser26setBreakAwayRewardAItemCntEi
    // DEMANGLED: CUser::setBreakAwayRewardAItemCnt(int)
    CUser* setBreakAwayRewardAItemCnt(int count);

    // IDA_EA: 0x84EC6BA
    // IDA_NAME: _ZN5CUser31getUserBreakAwayRewardLuckPointEv
    // DEMANGLED: CUser::getUserBreakAwayRewardLuckPoint(void)
    int getUserBreakAwayRewardLuckPoint();

    // IDA_EA: 0x84EC6C8
    // IDA_NAME: _ZN5CUser31setUserBreakAwayRewardLuckPointEi
    // DEMANGLED: CUser::setUserBreakAwayRewardLuckPoint(int)
    CUser* setUserBreakAwayRewardLuckPoint(int luck_point);

    // IDA_EA: 0x84EC6DA
    // IDA_NAME: _ZN5CUser25setJoinedSchoolPointEventEb
    // DEMANGLED: CUser::setJoinedSchoolPointEvent(bool)
    CUser* setJoinedSchoolPointEvent(bool joined);

    // IDA_EA: 0x84EC6F6
    // IDA_NAME: _ZN5CUser20setTournamentAccountEb
    // DEMANGLED: CUser::setTournamentAccount(bool)
    CUser* setTournamentAccount(bool tournament_account);

    // IDA_EA: 0x84EC712
    // IDA_NAME: _ZN5CUser19isTournamentAccountEv
    // DEMANGLED: CUser::isTournamentAccount(void)
    int isTournamentAccount();

    // IDA_EA: 0x84EC734
    // IDA_NAME: _ZN5CUser28setOnlinePreliminaryCharacNoEj
    // DEMANGLED: CUser::setOnlinePreliminaryCharacNo(unsigned int)
    CUser* setOnlinePreliminaryCharacNo(unsigned int charac_no);

    // IDA_EA: 0x84EC746
    // IDA_NAME: _ZN5CUser28setOnlinePreliminaryPlayTypeEi
    // DEMANGLED: CUser::setOnlinePreliminaryPlayType(int)
    CUser* setOnlinePreliminaryPlayType(int play_type);

    // IDA_EA: 0x84EC758
    // IDA_NAME: _ZN5CUser28getOnlinePreliminaryPlayTypeEv
    // DEMANGLED: CUser::getOnlinePreliminaryPlayType(void)
    int getOnlinePreliminaryPlayType();

    // IDA_EA: 0x84EC766
    // IDA_NAME: _ZN5CUser11SetPCRoomNoEj
    // DEMANGLED: CUser::SetPCRoomNo(unsigned int)
    CUser* SetPCRoomNo(unsigned int pc_room_no);

    // IDA_EA: 0x84EC778
    // IDA_NAME: _ZN5CUser13SetReturnUserEv
    // DEMANGLED: CUser::SetReturnUser(void)
    CUser* SetReturnUser();

    // IDA_EA: 0x84EC788
    // IDA_NAME: _ZN5CUser15ResetReturnUserEv
    // DEMANGLED: CUser::ResetReturnUser(void)
    CUser* ResetReturnUser();

    // IDA_EA: 0x84EC798
    // IDA_NAME: _ZN5CUser27SetReturnUserFirstUserLoginEb
    // DEMANGLED: CUser::SetReturnUserFirstUserLogin(bool)
    CUser* SetReturnUserFirstUserLogin(bool first_login);

    // IDA_EA: 0x84EC7B4
    // IDA_NAME: _ZN5CUser23GetReturnUserExpireTimeEv
    // DEMANGLED: CUser::GetReturnUserExpireTime(void)
    int GetReturnUserExpireTime();

    // IDA_EA: 0x84EC7C2
    // IDA_NAME: _ZN5CUser23SetReturnUserExpireTimeEi
    // DEMANGLED: CUser::SetReturnUserExpireTime(int)
    CUser* SetReturnUserExpireTime(int expire_time);

    // IDA_EA: 0x84EC7D4
    // IDA_NAME: _ZN5CUser28EnableSaveMemberBonusFatigueEv
    // DEMANGLED: CUser::EnableSaveMemberBonusFatigue(void)
    CUser* EnableSaveMemberBonusFatigue();

    // IDA_EA: 0x84EC7E4
    // IDA_NAME: _ZN5CUser28incAccountMemberBonusFatigueEs
    // DEMANGLED: CUser::incAccountMemberBonusFatigue(short)
    CUser* incAccountMemberBonusFatigue(short fatigue);

    // IDA_EA: 0x84EC812
    // IDA_NAME: _ZN5CUser30resetAccountMemberBonusFatigueEv
    // DEMANGLED: CUser::resetAccountMemberBonusFatigue(void)
    CUser* resetAccountMemberBonusFatigue();

    // IDA_EA: 0x84EC824
    // IDA_NAME: _ZN5CUser28getAccountMemberBonusFatigueEv
    // DEMANGLED: CUser::getAccountMemberBonusFatigue(void)
    int getAccountMemberBonusFatigue();

    // IDA_EA: 0x84EC834
    // IDA_NAME: _ZN5CUser13setPcRoomAuthEb
    // DEMANGLED: CUser::setPcRoomAuth(bool)
    CUser* setPcRoomAuth(bool authenticated);

    // IDA_EA: 0x84EC88C
    // IDA_NAME: _ZN5CUser16getClientVersionEv
    // DEMANGLED: CUser::getClientVersion(void)
    int getClientVersion();

    // IDA_EA: 0x84EC89A
    // IDA_NAME: _ZN5CUser16setClientVersionEj
    // DEMANGLED: CUser::setClientVersion(unsigned int)
    CUser* setClientVersion(unsigned int version);

    // IDA_EA: 0x84EC90A
    // IDA_NAME: _ZN5CUser21get_public_ip_addressEv
    // DEMANGLED: CUser::get_public_ip_address(void)
    int get_public_ip_address();

    // IDA_EA: 0x84EC918
    // IDA_NAME: _ZN5CUser13setWebAddressEPc
    // DEMANGLED: CUser::setWebAddress(char *)
    char* setWebAddress(char* address);

    // IDA_EA: 0x84EC96A
    // IDA_NAME: _ZN5CUser24setTotalHackUserTradeCntEj
    // DEMANGLED: CUser::setTotalHackUserTradeCnt(unsigned int)
    CUser* setTotalHackUserTradeCnt(unsigned int count);

    // IDA_EA: 0x84EC97C
    // IDA_NAME: _ZN5CUser28SetReportMannerlessUserCountEi
    // DEMANGLED: CUser::SetReportMannerlessUserCount(int)
    CUser* SetReportMannerlessUserCount(int count);

    // IDA_EA: 0x84EC98E
    // IDA_NAME: _ZN5CUser22GetChecksumAnswerCountEv
    // DEMANGLED: CUser::GetChecksumAnswerCount(void)
    int GetChecksumAnswerCount();

    // IDA_EA: 0x84EC9C8
    // IDA_NAME: _ZN5CUser10setRegDateEj
    // DEMANGLED: CUser::setRegDate(unsigned int)
    CUser* setRegDate(unsigned int date);

    // IDA_EA: 0x84EC9DA
    // IDA_NAME: _ZN5CUser10setRegTimeEl
    // DEMANGLED: CUser::setRegTime(long)
    CUser* setRegTime(long time);

    // IDA_EA: 0x84ECB0A
    // IDA_NAME: _ZN5CUser16GetLogInOutStateEv
    // DEMANGLED: CUser::GetLogInOutState(void)
    int GetLogInOutState();

    // IDA_EA: 0x84ECB28
    // IDA_NAME: _ZN5CUser14getStdDropRateEv
    // DEMANGLED: CUser::getStdDropRate(void)
    int getStdDropRate();

    // IDA_EA: 0x84ECEFE
    // IDA_NAME: _ZN5CUser21SetGuildCargoCapacityEj
    // DEMANGLED: CUser::SetGuildCargoCapacity(unsigned int)
    CUser* SetGuildCargoCapacity(unsigned int capacity);

    // IDA_EA: 0x84ECF10
    // IDA_NAME: _ZN5CUser19SetTodayGuildMemberEb
    // DEMANGLED: CUser::SetTodayGuildMember(bool)
    CUser* SetTodayGuildMember(bool today_member);

    // IDA_EA: 0x84ECF2C
    // IDA_NAME: _ZN5CUser23setHumanCertifyErrorCntEi
    // DEMANGLED: CUser::setHumanCertifyErrorCnt(int)
    CUser* setHumanCertifyErrorCnt(int count);

    // IDA_EA: 0x8658A3E
    // IDA_NAME: _ZN5CUser20checkLogOutCorrectlyEv
    // DEMANGLED: CUser::checkLogOutCorrectly(void)
    CUser* checkLogOutCorrectly();

    // IDA_EA: 0x868D32A
    // IDA_NAME: _ZN5CUser24set_server_fatigue_countEs
    // DEMANGLED: CUser::set_server_fatigue_count(short)
    void set_server_fatigue_count(short count);

    // IDA_EA: 0x868D3FE
    // IDA_NAME: _ZN5CUser24get_server_fatigue_countEh
    // DEMANGLED: CUser::get_server_fatigue_count(unsigned char)
    int get_server_fatigue_count(unsigned char day);

    // IDA_EA: 0x868D476
    // IDA_NAME: _ZN5CUser25reset_server_fatigue_dataEv
    // DEMANGLED: CUser::reset_server_fatigue_data(void)
    void reset_server_fatigue_data();

    // IDA_EA: 0x868D4A2
    // IDA_NAME: _ZN5CUser20set_fatigue_count_dbEii
    // DEMANGLED: CUser::set_fatigue_count_db(int,int)
    CUser* set_fatigue_count_db(int day, int count);

    // IDA_EA: 0x868D4C4
    // IDA_NAME: _ZN5CUser20get_fatigue_count_dbEi
    // DEMANGLED: CUser::get_fatigue_count_db(int)
    int get_fatigue_count_db(int day);

    // IDA_EA: 0x868D4F0
    // IDA_NAME: _ZN5CUser24get_ingame_event_historyEv
    // DEMANGLED: CUser::get_ingame_event_history(void)
    std::map<int, std::pair<int, int> >* get_ingame_event_history();

    // IDA_EA: 0x868D4FE
    // IDA_NAME: _ZN5CUser31get_ingame_event_history_updateEv
    // DEMANGLED: CUser::get_ingame_event_history_update(void)
    std::map<int, bool>* get_ingame_event_history_update();

    // IDA_EA: 0x868D50C
    // IDA_NAME: _ZN5CUser24get_ingame_event_historyEiRSt4pairIiiE
    // DEMANGLED: CUser::get_ingame_event_history(int,std::pair<int,int> &)
    bool get_ingame_event_history(int type, std::pair<int, int>& history);

    // IDA_EA: 0x868D580
    // IDA_NAME: _ZN5CUser24set_ingame_event_historyEiii
    // DEMANGLED: CUser::set_ingame_event_history(int,int,int)
    void set_ingame_event_history(int type, int first, int second);

    // IDA_EA: 0x868D664
    // IDA_NAME: _ZN5CUser24set_ingame_event_historyEP24SIG_INGAME_EVENT_HISTORY
    // DEMANGLED: CUser::set_ingame_event_history(SIG_INGAME_EVENT_HISTORY *)
    void set_ingame_event_history(SIG_INGAME_EVENT_HISTORY* history);

    // IDA_EA: 0x868D72C
    // IDA_NAME: _ZN5CUser27update_ingame_event_historyEi
    // DEMANGLED: CUser::update_ingame_event_history(int)
    void update_ingame_event_history(int type);

    // IDA_EA: 0x868D750
    // IDA_NAME: _ZN5CUser26reset_ingame_event_historyEv
    // DEMANGLED: CUser::reset_ingame_event_history(void)
    void reset_ingame_event_history();

    // IDA_EA: 0x868D768
    // IDA_NAME: _ZN5CUser33reset_ingame_event_history_updateEv
    // DEMANGLED: CUser::reset_ingame_event_history_update(void)
    void reset_ingame_event_history_update();

    // IDA_EA: 0x869126E
    // IDA_NAME: _ZN5CUser12SetBingoDataER14SIG_LOGIN_DATA
    // DEMANGLED: CUser::SetBingoData(SIG_LOGIN_DATA &)
    CUser* SetBingoData(SIG_LOGIN_DATA& login_data);

    // IDA_EA: 0x8697182
    // IDA_NAME: _ZN5CUser13resetMoneyLogEv
    // DEMANGLED: CUser::resetMoneyLog(void)
    CUser* resetMoneyLog();

    // IDA_EA: 0x86971CE
    // IDA_NAME: _ZN5CUser13GetFirstLoginEv
    // DEMANGLED: CUser::GetFirstLogin(void)
    int GetFirstLogin();

    // IDA_EA: 0x869721A
    // IDA_NAME: _ZN5CUser24isJoinedSchoolPointEventEv
    // DEMANGLED: CUser::isJoinedSchoolPointEvent(void)
    int isJoinedSchoolPointEvent();

    // IDA_EA: 0x869722A
    // IDA_NAME: _ZN5CUser29DisableSaveMemberBonusFatigueEv
    // DEMANGLED: CUser::DisableSaveMemberBonusFatigue(void)
    CUser* DisableSaveMemberBonusFatigue();

    // IDA_EA: 0x869723A
    // IDA_NAME: _ZN5CUser30IsEnableSaveMemberBonusFatigueEv
    // DEMANGLED: CUser::IsEnableSaveMemberBonusFatigue(void)
    int IsEnableSaveMemberBonusFatigue();

    // IDA_EA: 0x869727A
    // IDA_NAME: _ZN5CUser20get_local_ip_addressEv
    // DEMANGLED: CUser::get_local_ip_address(void)
    int get_local_ip_address();

    // IDA_EA: 0x8697288
    // IDA_NAME: _ZN5CUser15resetPlayExpAddEv
    // DEMANGLED: CUser::resetPlayExpAdd(void)
    CUser* resetPlayExpAdd();

    // IDA_EA: 0x869729A
    // IDA_NAME: _ZN5CUser13incPlayExpAddEi
    // DEMANGLED: CUser::incPlayExpAdd(int)
    CUser* incPlayExpAdd(int experience);

    // IDA_EA: 0x866A9A0
    // IDA_NAME: _ZN5CUser7gain_spEi
    // DEMANGLED: CUser::gain_sp(int)
    // IDA_EA: 0x866A3FE
    // IDA_NAME: _ZN5CUser11gain_exp_spEiRiS0_13eExpAddReasonib
    // DEMANGLED: CUser::gain_exp_sp(int,int &,int &,eExpAddReason,int,bool)
    bool gain_exp_sp(int gained_exp,
                     int& gained_sp,
                     int& gained_sfp,
                     eExpAddReason reason,
                     int monster_id,
                     bool flag);

    // IDA_EA: 0x868B20C
    // IDA_NAME: _ZN5CUser9rewardExpEiRiS0_S0_S0_13eExpAddReasonb
    // DEMANGLED: CUser::rewardExp(int,int &,int &,int &,int &,eExpAddReason,bool)
    void rewardExp(int gained_exp,
                   int& gained_sp,
                   int& gained_sfp,
                   int& previous_sp,
                   int& current_sp,
                   eExpAddReason reason,
                   bool notify);

    // IDA_EA: 0x8662AEA
    // IDA_NAME: _ZN5CUser15_check_level_upEiRiS0_13eExpAddReasoni
    // DEMANGLED: CUser::_check_level_up(
    //             int,int &,int &,eExpAddReason,int)
    bool _check_level_up(int gained_exp,
                         int& gained_sp,
                         int& gained_sfp,
                         eExpAddReason reason,
                         int notify_hades);

    // IDA_EA: 0x86646C8
    // IDA_NAME: _ZN5CUser19_update_charac_statEb
    // DEMANGLED: CUser::_update_charac_stat(bool)
    _Additioal_info* _update_charac_stat(bool add);

    // IDA_EA: 0x866AD92
    // IDA_NAME: _ZN5CUser8lost_extEiRib
    // DEMANGLED: CUser::lost_ext(int,int &,bool)
    int lost_ext(int percentage, int& lost_experience, bool force_level_down);

    // IDA_EA: 0x8664080
    // IDA_NAME: _ZN5CUser17_onChangeGrowTypeEhh21eChangeGrowTypeReason
    // DEMANGLED: CUser::_onChangeGrowType(unsigned char,unsigned char,eChangeGrowTypeReason)
    unsigned int _onChangeGrowType(unsigned char first_grow_type,
                                   unsigned char second_grow_type,
                                   eChangeGrowTypeReason reason);

    // IDA_EA: 0x8664AE8
    // IDA_NAME: _ZN5CUser13calc_lev_statER15_Additioal_info
    // DEMANGLED: CUser::calc_lev_stat(_Additioal_info &)
    bool calc_lev_stat(_Additioal_info& calculated_stat);

    // IDA_EA: 0x8664766
    // IDA_NAME: _ZN5CUser18adjust_charac_statEv
    // DEMANGLED: CUser::adjust_charac_stat(void)
    int adjust_charac_stat();

    // IDA_EA: 0x867925A
    // IDA_NAME: _ZN5CUser20processGrowTypeEventEhh21eChangeGrowTypeReason
    // DEMANGLED: CUser::processGrowTypeEvent(unsigned char,unsigned char,eChangeGrowTypeReason)
    void processGrowTypeEvent(unsigned char first_grow_type,
                              unsigned char second_grow_type,
                              eChangeGrowTypeReason reason);

    // IDA_EA: 0x86787FC
    // IDA_NAME: _ZN5CUser13set_grow_typeEhhPc21eChangeGrowTypeReason
    // DEMANGLED: CUser::set_grow_type(unsigned char,unsigned char,char *,eChangeGrowTypeReason)
    void set_grow_type(unsigned char first_grow_type,
                       unsigned char second_grow_type,
                       char* packet,
                       eChangeGrowTypeReason reason);

    // IDA_EA: 0x867B196
    // IDA_NAME: _ZN5CUser19ChangeGrowType_ItemEis
    // DEMANGLED: CUser::ChangeGrowType_Item(int,short)
    // The canonical body is supplied with the CMD503 Login transaction.  It
    // is a real item-consumption/skill-tree reset path, not a command shim.
    int ChangeGrowType_Item(int first_grow_type, short item_slot);

    // IDA_EA: 0x866311A
    // IDA_NAME: _ZN5CUser10_onLevelUpEv
    // DEMANGLED: CUser::_onLevelUp(void)
    void _onLevelUp();

    // IDA_EA: 0x86689EE
    // IDA_NAME: _ZN5CUser21_postQuestTriggerItemEPK5Quest
    // DEMANGLED: CUser::_postQuestTriggerItem(Quest const*)
    char _postQuestTriggerItem(const Quest* quest);

    // IDA_EA: 0x8668D1A
    // IDA_NAME: _ZN5CUser12UseQuestItemEi
    // DEMANGLED: CUser::UseQuestItem(int)
    void UseQuestItem(int item_slot);

    // IDA_EA: 0x869115A
    // IDA_NAME: _ZN5CUser19processLevelUpEventEv
    // DEMANGLED: CUser::processLevelUpEvent(void)
    void processLevelUpEvent();

    // IDA_EA: 0x8663CC0
    // IDA_NAME: _ZN5CUser25processLevelUpEventRewardEb
    // DEMANGLED: CUser::processLevelUpEventReward(bool)
    int processLevelUpEventReward(bool growup);

    // IDA_EA: 0x866407A
    // IDA_NAME: _ZN5CUser23processNPCGiftOnLevelUpEv
    // DEMANGLED: CUser::processNPCGiftOnLevelUp(void)
    void processNPCGiftOnLevelUp();
    int gain_sp(int gained_sp);

    // IDA_EA: 0x866AAD2
    // IDA_NAME: _ZN5CUser8gain_sfpEi
    // DEMANGLED: CUser::gain_sfp(int)
    void gain_sfp(int gained_sfp);

    // IDA_EA: 0x866AC0E
    // IDA_NAME: _ZN5CUser14history_log_spEii12eSPAddReason
    // DEMANGLED: CUser::history_log_sp(int,int,eSPAddReason)
    char history_log_sp(int previous_sp,
                        int gained_sp,
                        eSPAddReason reason);

    // IDA_EA: 0x866ACD0
    // IDA_NAME: _ZN5CUser15history_log_sfpEii12eSPAddReason
    // DEMANGLED: CUser::history_log_sfp(int,int,eSPAddReason)
    char history_log_sfp(int previous_sfp,
                         int gained_sfp,
                         eSPAddReason reason);

    // IDA_EA: 0x866B53E
    // IDA_NAME: _ZN5CUser17master_new_skillsEP15_Mastered_skill20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CUser::master_new_skills(
    //     _Mastered_skill *,ENUM_SKILL_TREE_KIND)
    int master_new_skills(_Mastered_skill* skills,
                          ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x866BCE8
    // IDA_NAME: _ZN5CUser15init_skill_treeE20ENUM_SKILL_TREE_KIND
    // DEMANGLED: CUser::init_skill_tree(ENUM_SKILL_TREE_KIND)
    void init_skill_tree(ENUM_SKILL_TREE_KIND skill_tree);

    // IDA_EA: 0x867B51C
    // IDA_NAME: _ZN5CUser13SetSkillLevelEii
    // DEMANGLED: CUser::SetSkillLevel(int,int)
    int SetSkillLevel(int skill_index, int requested_level);

    // IDA_EA: 0x866AFA2
    // IDA_NAME: _ZN5CUser16master_new_skillER14stBuySkillInfob
    // DEMANGLED: CUser::master_new_skill(stBuySkillInfo &,bool)
    void master_new_skill(stBuySkillInfo& skill_info, bool legacy_flag);

    // IDA_EA: 0x866CB04
    // IDA_NAME: _ZN5CUser17CheckQuestMonsterEii21ENUM_QUEST_ENEMY_TYPE
    // DEMANGLED: CUser::CheckQuestMonster(
    //             int,int,ENUM_QUEST_ENEMY_TYPE)
    bool CheckQuestMonster(int dungeon_index,
                           int monster_index,
                           ENUM_QUEST_ENEMY_TYPE enemy_type);

    // IDA_EA: 0x869730A
    // IDA_NAME: _ZN5CUser21GetResumeChecksumTimeEv
    // DEMANGLED: CUser::GetResumeChecksumTime(void)
    int GetResumeChecksumTime();

    // IDA_EA: 0x86973CC
    // IDA_NAME: _ZN5CUser23isCharacLinkMessageFlagEv
    // DEMANGLED: CUser::isCharacLinkMessageFlag(void)
    int isCharacLinkMessageFlag();

    // IDA_EA: 0x8697516
    // IDA_NAME: _ZN5CUser22ResetUsedCoinInDungeonEv
    // DEMANGLED: CUser::ResetUsedCoinInDungeon(void)
    CUser* ResetUsedCoinInDungeon();

    // IDA_EA: 0x869755C
    // IDA_NAME: _ZN5CUser18setCharacSlotLimitEh
    // DEMANGLED: CUser::setCharacSlotLimit(unsigned char)
    CUser* setCharacSlotLimit(unsigned char limit);

    // IDA_EA: 0x8697578
    // IDA_NAME: _ZNK5CUser26getFlagPresentAvengerTitleEv
    // DEMANGLED: CUser::getFlagPresentAvengerTitle(void)const
    int getFlagPresentAvengerTitle() const;

    // IDA_EA: 0x868B552
    // IDA_NAME: _ZN5CUser25VerifyPresentAvengerTitleEv
    // DEMANGLED: CUser::VerifyPresentAvengerTitle(void)
    bool VerifyPresentAvengerTitle();

    // IDA_EA: 0x8697588
    // IDA_NAME: _ZN5CUser33reset_update_ontime_last_recv_idxEv
    // DEMANGLED: CUser::reset_update_ontime_last_recv_idx(void)
    CUser* reset_update_ontime_last_recv_idx();

    // IDA_EA: 0x8697598
    // IDA_NAME: _ZN5CUser30is_update_ontime_last_recv_idxEv
    // DEMANGLED: CUser::is_update_ontime_last_recv_idx(void)
    int is_update_ontime_last_recv_idx();

    // IDA_EA: 0x86975A8
    // IDA_NAME: _ZN5CUser27get_server_fatigue_day_dataEv
    // DEMANGLED: CUser::get_server_fatigue_day_data(void)
    std::map<unsigned char, short>* get_server_fatigue_day_data();

    // IDA_EA: 0x86975B6
    // IDA_NAME: _ZN5CUser27get_server_fatigue_day_sizeEv
    // DEMANGLED: CUser::get_server_fatigue_day_size(void)
    int get_server_fatigue_day_size();

    // IDA_EA: 0x84ECF3E
    // IDA_NAME: _ZN5CUser22setRestingUserRestrictEb
    // DEMANGLED: CUser::setRestingUserRestrict(bool)
    CUser* setRestingUserRestrict(bool restricted);

    // IDA_EA: 0x84ECF5A
    // IDA_NAME: _ZN5CUser19setPunishTradeAlertEs
    // DEMANGLED: CUser::setPunishTradeAlert(short)
    CUser* setPunishTradeAlert(short alert);

    // IDA_EA: 0x84ECF78
    // IDA_NAME: _ZN5CUser13setSecuRewardEb
    // DEMANGLED: CUser::setSecuReward(bool)
    CUser* setSecuReward(bool reward);

    // IDA_EA: 0x84ECF94
    // IDA_NAME: _ZN5CUser14NeedSecuRewardEv
    // DEMANGLED: CUser::NeedSecuReward(void)
    int NeedSecuReward();

    // IDA_EA: 0x84ECFA4
    // IDA_NAME: _ZN5CUser11setSecuTypeEh
    // DEMANGLED: CUser::setSecuType(unsigned char)
    CUser* setSecuType(unsigned char type);

    // IDA_EA: 0x84ECFC0
    // IDA_NAME: _ZN5CUser13getRewardTypeEv
    // DEMANGLED: CUser::getRewardType(void)
    int getRewardType();

    // IDA_EA: 0x84ECFD0
    // IDA_NAME: _ZN5CUser13setRewardTypeEh
    // DEMANGLED: CUser::setRewardType(unsigned char)
    CUser* setRewardType(unsigned char type);

    // IDA_EA: 0x84ECFEC
    // IDA_NAME: _ZN5CUser12setSecuGradeEc
    // DEMANGLED: CUser::setSecuGrade(char)
    CUser* setSecuGrade(char grade);

    // IDA_EA: 0x868866E
    // IDA_NAME: _ZN5CUser21isSecurityServiceUsedEv
    // DEMANGLED: CUser::isSecurityServiceUsed(void)
    int isSecurityServiceUsed();

    // IDA_EA: 0x84ED016
    // IDA_NAME: _ZN5CUser26setFlagPresentAvengerTitleEb
    // DEMANGLED: CUser::setFlagPresentAvengerTitle(bool)
    CUser* setFlagPresentAvengerTitle(bool present);

    // IDA_EA: 0x84ED032
    // IDA_NAME: _ZN5CUser30setCheckPickUpRandomOptionItemEb
    // DEMANGLED: CUser::setCheckPickUpRandomOptionItem(bool)
    CUser* setCheckPickUpRandomOptionItem(bool check);

    // IDA_EA: 0x85BFD36
    // IDA_NAME: _ZN5CUser27subBreakAwayDungeonClearCntEv
    // DEMANGLED: CUser::subBreakAwayDungeonClearCnt(void)
    CUser* subBreakAwayDungeonClearCnt();

    // IDA_EA: 0x85BFDCA
    // IDA_NAME: _ZN5CUser18IsTodayGuildMemberEv
    // DEMANGLED: CUser::IsTodayGuildMember(void)
    int IsTodayGuildMember();

    // IDA_EA: 0x86C0B6E
    // IDA_NAME: _ZN5CUser14ResetCoinCountEv
    // DEMANGLED: CUser::ResetCoinCount(void)
    CUser* ResetCoinCount();

    // IDA_EA: 0x86C0B80
    // IDA_NAME: _ZN5CUser16GetUsedCoinCountEv
    // DEMANGLED: CUser::GetUsedCoinCount(void)
    int GetUsedCoinCount();

    // IDA_EA: 0x86D207C
    // IDA_NAME: _ZN5CUser23resetAccountUsedFatigueEv
    // DEMANGLED: CUser::resetAccountUsedFatigue(void)
    CUser* resetAccountUsedFatigue();

    // IDA_EA: 0x86D208E
    // IDA_NAME: _ZN5CUser23incChattingMessageCountEi
    // DEMANGLED: CUser::incChattingMessageCount(int)
    CUser* incChattingMessageCount(int count);

    // IDA_EA: 0x86D20AA
    // IDA_NAME: _ZN5CUser23getChattingMessageCountEv
    // DEMANGLED: CUser::getChattingMessageCount(void)
    int getChattingMessageCount();

    // IDA_EA: 0x86887C4
    // IDA_NAME: _ZN5CUser20isDuplicationMessageERKSs
    // DEMANGLED: CUser::isDuplicationMessage(std::string const&)
    bool isDuplicationMessage(const std::string& message);

    // IDA_EA: 0x8649AC4
    // IDA_NAME: _ZN5CUser24GenerateCleanPadVeriDataERtRhS0_
    // DEMANGLED: CUser::GenerateCleanPadVeriData(unsigned short &,unsigned char &,unsigned short &)
    uint16_t* GenerateCleanPadVeriData(uint16_t& first,
                                       uint8_t& operation,
                                       uint16_t& second);

    // IDA_EA: 0x867F26E
    // IDA_NAME: _ZN5CUser15reqHumanCertifyEv
    // DEMANGLED: CUser::reqHumanCertify(void)
    int reqHumanCertify();

    // IDA_EA: 0x86802B8
    // IDA_NAME: _ZN5CUser24reqHumanCertify4ClearMapEb
    // DEMANGLED: CUser::reqHumanCertify4ClearMap(bool)
    int reqHumanCertify4ClearMap(bool probability_gate);

    // IDA_EA: 0x8649B44
    // IDA_NAME: _ZN5CUser22VerifyCleanPadVeriDataEj
    // DEMANGLED: CUser::VerifyCleanPadVeriData(unsigned int)
    bool VerifyCleanPadVeriData(unsigned int verification_data);

    // IDA_EA: 0x867F4C8
    // IDA_NAME: _ZN5CUser17checkHumanCertifyEiPt
    // DEMANGLED: CUser::checkHumanCertify(int,unsigned short *)
    bool checkHumanCertify(int answer_size, unsigned short* answer);

    // Header-only typed views of C2-established children.  CMD145 needs the
    // original MCAP/history/request-tick state, none of which owns a public
    // out-of-line accessor symbol in the old ABI.
    WongWork::CMCAPManager& getMCAPManagerW() { return mcap_manager_; }
    cUserHistoryLog& getHistoryLogW() { return history_log_; }
    uint32_t getCleanPadRequestTick() const {
        return clean_pad_request_tick_;
    }

    // IDA_EA: 0x868DD18
    // IDA_NAME: _ZN5CUser16setUseCraneStartEb
    // DEMANGLED: CUser::setUseCraneStart(bool)
    CUser* setUseCraneStart(bool started);

    // IDA_EA: 0x868DD34
    // IDA_NAME: _ZNK5CUser15isUseCraneStartEv
    // DEMANGLED: CUser::isUseCraneStart(void)const
    int isUseCraneStart() const;

    // IDA_EA: 0x868F5CA
    // IDA_NAME: _ZN5CUser25SetProperLevelDungeonUserEv
    // DEMANGLED: CUser::SetProperLevelDungeonUser(void)
    CUser* SetProperLevelDungeonUser();

    // IDA_EA: 0x868F5DA
    // IDA_NAME: _ZN5CUser27ClearProperLevelDungeonUserEv
    // DEMANGLED: CUser::ClearProperLevelDungeonUser(void)
    CUser* ClearProperLevelDungeonUser();

    // IDA_EA: 0x868F5EA
    // IDA_NAME: _ZN5CUser24IsProperLevelDungeonUserEv
    // DEMANGLED: CUser::IsProperLevelDungeonUser(void)
    int IsProperLevelDungeonUser();

    // IDA_EA: 0x868F5FA
    // IDA_NAME: _ZN5CUser27ProcProperLevelDungeonClearEv
    // DEMANGLED: CUser::ProcProperLevelDungeonClear(void)
    void ProcProperLevelDungeonClear();

    // IDA_EA: 0x8693758
    // IDA_NAME: _ZN5CUser15GiveMeBoxRewardEv
    // DEMANGLED: CUser::GiveMeBoxReward(void)
    void GiveMeBoxReward();

    // IDA_EA: 0x868FA56
    // IDA_NAME: _ZN5CUser26enableSaveInformNoticeFlagEv
    // DEMANGLED: CUser::enableSaveInformNoticeFlag(void)
    CUser* enableSaveInformNoticeFlag();

    // IDA_EA: 0x8692B04
    // IDA_NAME: _ZN5CUser12SetCeraPointEj
    // DEMANGLED: CUser::SetCeraPoint(unsigned int)
    CUser* SetCeraPoint(unsigned int cera_point);

    // IDA_EA: 0x8692B58
    // IDA_NAME: _ZN5CUser9SetPuUserEb
    // DEMANGLED: CUser::SetPuUser(bool)
    CUser* SetPuUser(bool pu_user);

    // IDA_EA: 0x8692B74
    // IDA_NAME: _ZN5CUser9GetPuUserEv
    // DEMANGLED: CUser::GetPuUser(void)
    int GetPuUser();

    // IDA_EA: 0x8692F46
    // IDA_NAME: _ZN5CUser24setAllowLocationGeoStateEb
    // DEMANGLED: CUser::setAllowLocationGeoState(bool)
    CUser* setAllowLocationGeoState(bool allowed);

    // IDA_EA: 0x8693178
    // IDA_NAME: _ZN5CUser22recipeForceProbabilityEi
    // DEMANGLED: CUser::recipeForceProbability(int)
    CUser* recipeForceProbability(int probability);

    // IDA_EA: 0x869318A
    // IDA_NAME: _ZN5CUser20getRecipeProbabilityEv
    // DEMANGLED: CUser::getRecipeProbability(void)
    int getRecipeProbability();

    // IDA_EA: 0x8689850
    // IDA_NAME: _ZN5CUser28SetTradePunishOverThirtyDaysEb
    // DEMANGLED: CUser::SetTradePunishOverThirtyDays(bool)
    CUser* SetTradePunishOverThirtyDays(bool over_thirty_days);

    // IDA_EA: 0x8689880
    // IDA_NAME: _ZN5CUser27isTradePunishOverThirtyDaysEv
    // DEMANGLED: CUser::isTradePunishOverThirtyDays(void)
    int isTradePunishOverThirtyDays();

    // IDA_EA: 0x86898C2
    // IDA_NAME: _ZN5CUser18DelTradePunishTypeEi
    // DEMANGLED: CUser::DelTradePunishType(int)
    int DelTradePunishType(int punish_type);

    // IDA_EA: 0x867CB70
    // IDA_NAME: _ZN5CUser22setPlayingDungeonIndexEi
    // DEMANGLED: CUser::setPlayingDungeonIndex(int)
    CUser* setPlayingDungeonIndex(int dungeon_index);

    // IDA_EA: 0x867CBC4
    // IDA_NAME: _ZN5CUser15setLastPacketIDEi
    // DEMANGLED: CUser::setLastPacketID(int)
    CUser* setLastPacketID(int packet_id);

    // IDA_EA: 0x867CBD6
    // IDA_NAME: _ZN5CUser15getLastPacketIDEv
    // DEMANGLED: CUser::getLastPacketID(void)
    int getLastPacketID();

    // IDA_EA: 0x866C1CA
    // IDA_NAME: _ZN5CUser19IsCompleteGuildFundEv
    // DEMANGLED: CUser::IsCompleteGuildFund(void)
    bool IsCompleteGuildFund();

    // IDA_EA: 0x8697318
    // IDA_NAME: _ZN5CUser22UpdateTutorialSkipableEv
    // DEMANGLED: CUser::UpdateTutorialSkipable(void)
    CUser* UpdateTutorialSkipable();

    // IDA_EA: 0x8568E9C
    // IDA_NAME: _ZN5CUser12UpdateRatingEf
    // DEMANGLED: CUser::UpdateRating(float)
    CUser* UpdateRating(float rating);

    // IDA_EA: 0x8230172
    // IDA_NAME: _ZN5CUser13IsGuildMasterEv
    // DEMANGLED: CUser::IsGuildMaster(void)
    bool IsGuildMaster();

    // IDA_EA: 0x863BEA2
    // IDA_NAME: _ZNK5CUser13IsPremiumUserEv
    // DEMANGLED: CUser::IsPremiumUser(void)const
    bool IsPremiumUser() const;

    // IDA_EA: 0x8692B16
    // IDA_NAME: _ZN5CUser12UseCeraPointEj
    // DEMANGLED: CUser::UseCeraPoint(unsigned int)
    int UseCeraPoint(unsigned int cera_point);

    // IDA_EA: 0x8230100
    // IDA_NAME: _ZN5CUser19get_pvp_exp_of_weekEv
    // DEMANGLED: CUser::get_pvp_exp_of_week(void)
    int get_pvp_exp_of_week();

    // IDA_EA: 0x867ECF0
    // IDA_NAME: _ZN5CUser8SetBirthEPKc
    // DEMANGLED: CUser::SetBirth(char const*)
    CUser* SetBirth(const char* birth);

    // IDA_EA: 0x867EC66
    // IDA_NAME: _ZN5CUser14IsOverBirthDayEv
    // DEMANGLED: CUser::IsOverBirthDay(void)
    bool IsOverBirthDay();

    // IDA_EA: 0x866AF86
    // IDA_NAME: _ZN5CUser15get_skill_countEv
    // DEMANGLED: CUser::get_skill_count(void)
    int get_skill_count();

    // IDA_EA: 0x85BFDDA
    // IDA_NAME: _ZN5CUser26resetDungeonClearHackCheckEv
    // DEMANGLED: CUser::resetDungeonClearHackCheck(void)
    CUser* resetDungeonClearHackCheck();

    // IDA_EA: 0x85BFDEA
    // IDA_NAME: _ZNK5CUser19IsUsedCoinInDungeonEv
    // DEMANGLED: CUser::IsUsedCoinInDungeon(void)const
    int IsUsedCoinInDungeon() const;

    // IDA_EA: 0x85BFDFA
    // IDA_NAME: _ZNK5CUser30IsDeleteEventDungeonInvitationEv
    // DEMANGLED: CUser::IsDeleteEventDungeonInvitation(void)const
    int IsDeleteEventDungeonInvitation() const;

    // IDA_EA: 0x8689576
    // IDA_NAME: _ZN5CUser23EventDungeonClearRewardERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::EventDungeonClearReward(std::vector<std::pair<int,int> > const&)
    char EventDungeonClearReward(
        const std::vector<std::pair<int, int> >& rewards);

    // IDA_EA: 0x85BFE0A
    // IDA_NAME: _ZN5CUser30getCheckPickUpRandomOptionItemEv
    // DEMANGLED: CUser::getCheckPickUpRandomOptionItem(void)
    int getCheckPickUpRandomOptionItem();

    // IDA_EA: 0x85D1A0A
    // IDA_NAME: _ZN5CUser19GetUseAPCPotionTimeEv
    // DEMANGLED: CUser::GetUseAPCPotionTime(void)
    int GetUseAPCPotionTime();

    // IDA_EA: 0x863BE54
    // IDA_NAME: _ZNK5CUser13getPcRoomAuthEv
    // DEMANGLED: CUser::getPcRoomAuth(void)const
    int getPcRoomAuth() const;

    // IDA_EA: 0x863BE64
    // IDA_NAME: _ZN5CUser9IsPowerUpEv
    // DEMANGLED: CUser::IsPowerUp(void)
    int IsPowerUp();

    // IDA_EA: 0x8690EB8
    // IDA_NAME: _ZN5CUser12setMoveSpaceEN9UserSpace1TE
    // DEMANGLED: CUser::setMoveSpace(UserSpace::T)
    CUser* setMoveSpace(UserSpace::T space);

    // IDA_EA: 0x868E9C0
    // IDA_NAME: _ZNK5CUser22isCompetitionMercenaryEv
    // DEMANGLED: CUser::isCompetitionMercenary(void)const
    bool isCompetitionMercenary() const;

    // IDA_EA: 0x8690FCC
    // IDA_NAME: _ZN5CUser24sendBlueMarbleEnterCountEv
    // DEMANGLED: CUser::sendBlueMarbleEnterCount(void)
    void sendBlueMarbleEnterCount();

    // C gameplay business closure batch 026 exact-96 declarations.
    bool IsWaitingStateCharac(unsigned int charac_no);
    void* DeleteCargo();
    STBreakAwayRewards* setBreakAwayRewards(const STBreakAwayRewards& rewards);
    char* UpdateCharacNameInMercenary(unsigned int charac_no,
                                      const std::string& charac_name);
    char* getUserInfo4Antibot();
    CAccountCargo* SetAccountCargoDBData(Inven_Item* items,
                                         unsigned int money,
                                         unsigned int capacity);
    void* resetHumanCertifyData();
    int AddSecuRewardItem(unsigned char reward_type);

    // IDA_EA: 0x868833C
    // IDA_NAME: _ZN5CUser18sendSecuRewardItemEP10Inven_Item25ENUM_SECURITY_REWARD_TYPEPi
    // DEMANGLED: CUser::sendSecuRewardItem(Inven_Item *,ENUM_SECURITY_REWARD_TYPE,int *)
    int sendSecuRewardItem(Inven_Item* item,
                           ENUM_SECURITY_REWARD_TYPE reward_type,
                           int* result_slots);

    int Return_Mercenary_Charac(unsigned int charac_no,
                                competition_reward_result& result);
    bool Competition_Mercenary_Charac(unsigned int charac_no,
                                      char competition_type,
                                      char competition_count);
    int setCharacSlotCountInfo(unsigned char slot_effect_count,
                               unsigned char slot_limit);
    char* getGrowthWeaponEventdata();
    GrowthCreatureEvent* getGrowthCreatureEventdata();
    CharacLevelUpGiftAccountOnce* getCharacLevelUpGift_AccountOnce();

    // IDA_EA: 0x8691BAE / 0x8691CDE / 0x8691E6A
    // IDA_NAME: _ZN5CUser24SetGrowthWeaponEventInfoERK23SIG_GROWTH_WEAPON_EVENT
    //           _ZN5CUser26SetGrowthCreatureEventInfoERK30SIG_GIVE_GROWTH_CREATURE_EVENT
    //           _ZN5CUser13sendEventInfoEv
    void SetGrowthWeaponEventInfo(const SIG_GROWTH_WEAPON_EVENT& event_info);
    void SetGrowthCreatureEventInfo(
        const SIG_GIVE_GROWTH_CREATURE_EVENT& event_info);
    void sendEventInfo();

    // IDA_EA: 0x8691DCA
    // IDA_NAME: _ZN5CUser32setCharacLevelUpGift_AccountOnceER29SIG_LEVELUP_GIFT_ACCOUNT_ONCE
    // DEMANGLED: CUser::setCharacLevelUpGift_AccountOnce(SIG_LEVELUP_GIFT_ACCOUNT_ONCE &)
    char setCharacLevelUpGift_AccountOnce(
        SIG_LEVELUP_GIFT_ACCOUNT_ONCE& event_info);

    // IDA_EA: 0x8690BE6
    // IDA_NAME: _ZN5CUser19ValidateStrikerInfoEv
    // DEMANGLED: CUser::ValidateStrikerInfo(void)
    bool ValidateStrikerInfo();

    // IDA_EA: 0x867CEAE
    // IDA_NAME: _ZN5CUser25SetNeedCheckOverEquipItemEv
    // DEMANGLED: CUser::SetNeedCheckOverEquipItem(void)
    bool SetNeedCheckOverEquipItem();
    bool ResetDailyCharacExpandDataMidnight();
    bool ResetDailyCharacExpandData();

    // IDA_EA: 0x866C04E
    // IDA_NAME: _ZN5CUser28set_charac_guildkey_in_loginEjj
    // DEMANGLED: CUser::set_charac_guildkey_in_login(unsigned int,unsigned int)
    // This is the character-select-state update used by Inter slot 97.  It
    // must update the matching persisted character record rather than the
    // currently selected CUserCharacInfo record.
    char set_charac_guildkey_in_login(uint32_t guild_key,
                                      uint32_t charac_no);

    void* resetGuildDBInfo();
    int GetRentInfo(SIG_LOAD_INVENTORY& inventory);
    char* SetUserDetailInfo(char* first, char* second, char* third);
    int OnItemUnlockWaitTimeout(int inventory_type, int slot);
    // IDA_EA: 0x8646A9C
    // IDA_NAME: _ZNK5CUser14IsAbleItemLockEii
    // DEMANGLED: CUser::IsAbleItemLock(int,int)const
    int IsAbleItemLock(int inventory_type, int slot) const;
    // IDA_EA: 0x8646BFE
    // IDA_NAME: _ZNK5CUser16IsAbleItemUnlockEiiRi
    // DEMANGLED: CUser::IsAbleItemUnlock(int,int,int &)const
    int IsAbleItemUnlock(int inventory_type,
                         int slot,
                         int& unlock_time) const;
    // IDA_EA: 0x8646CAE
    // IDA_NAME: _ZNK5CUser22IsAbleItemUnlockCancelEii
    // DEMANGLED: CUser::IsAbleItemUnlockCancel(int,int)const
    int IsAbleItemUnlockCancel(int inventory_type, int slot) const;
    int DoItemLock(int inventory_type, int slot);
    int DoItemUnlock(int inventory_type, int slot);
    int DoItemUnlockCancel(int inventory_type, int slot);
    // IDA_EA: 0x86472C0
    // IDA_NAME: _ZN5CUser14Add_RedeemInfoERK10Inven_Itemib
    // DEMANGLED: CUser::Add_RedeemInfo(Inven_Item const&,int,bool)
    int Add_RedeemInfo(const Inven_Item& item, int value, bool add_1000);
    int send_RedeemInfo();
    // IDA_EA: 0x8646DF2
    // IDA_NAME: _ZN5CUser11Redeem_ItemEi
    // DEMANGLED: CUser::Redeem_Item(int)
    int Redeem_Item(int index);
    int send_MissionList();
    void ResetDailyMission();
    char UpdateMissionCondition(std::vector<short>& mission_ids,
                                std::vector<int>& values);
    int ClearMissionKind(short mission_kind);
    int get_pvp_WinningRate_relateMission() const;
    int has_within_Mission() const;
    // IDA_EA: 0x8662774
    // IDA_NAME: _ZNK5CUser25acceptable_within_missionEv
    // DEMANGLED: CUser::acceptable_within_mission(void)const
    int acceptable_within_mission() const;
    charac_expand::CData* Remove_Mission(short mission_kind);
    int is_clear_stealingSkillMission() const;

    // IDA_EA: 0x868D144
    // IDA_NAME: _ZN5CUser16verifyDoingQuestEv
    // DEMANGLED: CUser::verifyDoingQuest(void)
    bool verifyDoingQuest();

    void* set_skill_slot(char* first, char* second, int size);
    char* fillUserInfo4Antibot();
    int SetPvpRecord(PvpResultType& result);
    // IDA_EA: 0x865C678
    // IDA_NAME: _ZN5CUser14add_pvp_resultEbPj
    // DEMANGLED: CUser::add_pvp_result(bool,unsigned int *)
    CUserCharacInfo* add_pvp_result(bool won, uint32_t* reward_exp);

    // IDA_EA: 0x865D986
    // IDA_NAME: _ZN5CUser17add_pvp_play_infoEjj
    // DEMANGLED: CUser::add_pvp_play_info(unsigned int,unsigned int)
    // The old leaf preserves its final writable result pointer in EAX.
    PvpResultType* add_pvp_play_info(unsigned int play_seconds,
                                     unsigned int play_count);
    // IDA_EA: 0x865C936
    // IDA_NAME: _ZN5CUser20add_guild_pvp_resultEi
    // DEMANGLED: CUser::add_guild_pvp_result(int)
    void add_guild_pvp_result(int point_change);
    // IDA_EA: 0x865CD52
    // IDA_NAME: _ZN5CUser15send_pvp_recordEv
    // DEMANGLED: CUser::send_pvp_record(void)
    void send_pvp_record();
    // IDA_EA: 0x865CFD8
    // IDA_NAME: _ZN5CUser16update_pvp_pointEi
    // DEMANGLED: CUser::update_pvp_point(int)
    int update_pvp_point(int point_change);
    void add_guild_point_item();
    int update_pvp_rank(const PvpResultType& result);
    int Update_ComboMission(int mission_index, short value);
    // IDA_EA: 0x866285C
    // IDA_NAME: _ZN5CUser11delete_itemEcimi14eItemDelReason
    // DEMANGLED: CUser::delete_item(char,int,unsigned long,int,eItemDelReason)
    int delete_item(char item_space,
                    int slot,
                    unsigned long item_id,
                    int count,
                    eItemDelReason reason);
    int compute_level_penalty(int level_difference);
    bool isSuccessCompound(int probability);
    void sendCharacQuestPiece();
    int GetItemTimeChecker(unsigned int current_tick,
                           unsigned short x,
                           unsigned short y,
                           unsigned int interval);
    bool IsGetItemDistanceOver(short threshold) const;
    // IDA_EA: 0x823066A
    // IDA_NAME: _ZN5CUser20GetItemSerialCheckerEtti
    // DEMANGLED: CUser::GetItemSerialChecker(unsigned short,unsigned short,int)
    int GetItemSerialChecker(unsigned short x,
                             unsigned short y,
                             int unique_id);
    bool checkInformNoticeFlag(unsigned int bit, unsigned int byte_index);
    SIG_ATTENDANCE_EVENT* GetCharacCashAttendanceInfo(
        SIG_ATTENDANCE_EVENT& attendance) const;
    int CheckQuestMap(int quest_index);
    int CheckDungeonClear(int dungeon_index);
    int IsEquipAvatar();
    // IDA_EA: 0x86792D6
    // IDA_NAME: _ZN5CUser19processAwakenRewardEc
    // DEMANGLED: CUser::processAwakenReward(char)
    int processAwakenReward(char reward_type);
    // IDA_EA: 0x867EFAE / 0x867F040 / 0x867F104.  These three methods own the
    // CUser black-list vector, whose original i386 location is frozen by the
    // offsetof assertion below rather than being used as a runtime address.
    int registerToBlackList(unsigned int charac_no);
    int deleteToBlackList(unsigned int charac_no);
    int isBlackUser(unsigned int charac_no);
    int resetBlackList();
    // IDA_EA: 0x867EE28
    // IDA_NAME: _ZN5CUser14printBlackListEv
    // DEMANGLED: CUser::printBlackList(void)
    int printBlackList();
    int SendItemMakingSkillInfo();
    int CheckLimitMiniumAge(int minimum_age, int& unique_id);
    int update_old_pvp_point();
    int ClearSkills();
    void SendZeroExpPacket();
    int SendCmdPeerErrorPacket(ENUM_CMDPACKET command,
                               ENUM_PEER_REQUEST_TYPE request_type,
                               unsigned char error,
                               PacketGuard& packet);
    // IDA_EA: 0x868DCBC / 0x868DD76 / 0x868E0EE.
    // CMD010 uses this request-specific solo-play gate before the peer
    // subtype is dispatched.  The MSG_REQ_TO_PEER layout is owned by the
    // concrete CMD010 dispatcher, keeping CUser free of packet storage.
    // IDA_EA: 0x868DC7A
    // IDA_NAME: _ZN5CUser14alonePlay_ModeEv
    // DEMANGLED: CUser::alonePlay_Mode(void)
    bool alonePlay_Mode();

    // IDA_EA: 0x868DD44
    // IDA_NAME: _ZN5CUser17get_SoloPlay_modeEv
    // DEMANGLED: CUser::get_SoloPlay_mode(void)
    int get_SoloPlay_mode();

    bool alonePlay_Mode(const MSG_REQ_TO_PEER& request);
    int get_SoloPlay_msgType(const MSG_REQ_TO_PEER& request);
    void Send_SoloPlay_Reject_NotiPack(const char* character_name,
                                       int reject_mode,
                                       int message_type);
    int GetPowerWarPartyVictory();
    int getQuestPoint();
    void SendEventMailAlaramTooltip();
    int getCurCharacSkillCheckSum() const;
    void sendCharacLinkMessage();

    // IDA_EA: 0x86528B4
    // IDA_NAME: _ZN5CUser25saveCharacLinkMessageFlagEb
    // DEMANGLED: CUser::saveCharacLinkMessageFlag(bool)
    void saveCharacLinkMessageFlag(bool enabled);

    // IDA_EA: 0x8652A58
    // IDA_NAME: _ZN5CUser23decideCharacLinkMessageEv
    // DEMANGLED: CUser::decideCharacLinkMessage(void)
    void decideCharacLinkMessage();

    // IDA_EA: 0x8654F74
    // IDA_NAME: _ZN5CUser23customizingSkillCommandER18STSkillCommandData
    // DEMANGLED: CUser::customizingSkillCommand(STSkillCommandData &)
    int customizingSkillCommand(STSkillCommandData& command);
    int defaultSkillCommand();
    bool find_pvp_masterid_walkingout_me(unsigned int master_id);
    void SendUnlimitChallengePacket();
    void setQuestPoint(int quest_point);
    // IDA_EA: 0x868FC46
    // IDA_NAME: _ZN5CUser12initAllSkillEv
    // DEMANGLED: CUser::initAllSkill(void)
    char initAllSkill();
    void initTPSkill();
    void SendManageLevelUpNoti();
    CUser* SetCurCharacUsedGiftFatigueQuantity(unsigned int quantity);
    CUser* ResetCurCharacUsedGiftFatigueQuantity();
    // IDA_EA: 0x84ECC7C
    // IDA_NAME: _ZN5CUser13setCharacNameEiPKc
    // DEMANGLED: CUser::setCharacName(int,char const*)
    int setCharacName(int charac_no, const char* charac_name);
    int getCharacLevelByCharacNo(int charac_no) const;

    // Typed C-owner bridge for the CMD585 account-wide level gate.  It is
    // intentionally composed only from the exact legacy character-list APIs
    // below/above this declaration; packet handlers must not reopen CUser's
    // embedded std::vector<_Charac_info> offset.
    bool HasCharacterLevelAbove(int level) const;

    int SendCmdPeerErrorPacket(ENUM_CMDPACKET command,
                               ENUM_PEER_REQUEST_TYPE request_type,
                               unsigned char error);
    int StartRecvEventWaiting();
    int getCurCharacInvenCheckSum() const;
    int IsGuildSkillLearn(int skill_index);
    // IDA_EA: 0x864FA08
    // IDA_NAME: _ZN5CUser20gainExpAsUpperMemberEi
    // DEMANGLED: CUser::gainExpAsUpperMember(int)
    int gainExpAsUpperMember(int base_exp);
    // IDA_EA: 0x864FB3A
    // IDA_NAME: _ZN5CUser17gainGuildSkillExpEi
    // DEMANGLED: CUser::gainGuildSkillExp(int)
    int gainGuildSkillExp(int base_exp);
    charac_expand::CData* SetMissionParameter(int mission_kind, int value);
    char ClearMission(short mission_kind);
    char ResetMissionList();

    // IDA_EA: 0x866C27E
    // IDA_NAME: _ZN5CUser17CheckGuildLevelUpERj
    // DEMANGLED: CUser::CheckGuildLevelUp(unsigned int&)
    int CheckGuildLevelUp(unsigned int& levelup_price);

    int IsSubGuildMaster();
    void SendLimitItemUsageNotiInfo(int item_id, int limit, int used);
    // IDA_EA: 0x8688AE6
    // IDA_NAME: _ZN5CUser12UseLimitItemEib
    // DEMANGLED: CUser::UseLimitItem(int,bool)
    int UseLimitItem(int item_id, bool notify);
    // IDA_EA: 0x8688C70
    // IDA_NAME: _ZN5CUser17IsLimitItemUsableEi
    // DEMANGLED: CUser::IsLimitItemUsable(int)
    int IsLimitItemUsable(int item_id);
    void sendReturnUserFirstLogin();
    void sendNotiPcRoomPlayTime();
    int processReturnUserQuestAutoClear();
    // IDA_EA: 0x868C170
    // IDA_NAME: _ZN5CUser31send_ontime_reward_start_notifyEv
    // DEMANGLED: CUser::send_ontime_reward_start_notify(void)
    int send_ontime_reward_start_notify();
    void processReturnUser();
    int GiveItemsReturnUser();
    int GiveEquipItemsReturnUser();
    int getReturnUserLevelKey(int charac_level, int first_grow_type);
    bool CheckItemLockSlotSpace(int inventory_type, int slot) const;
    int AddDungeonClear(int dungeon_index, int dungeon_diff);
    bool SetCharacMercenaryInfo(int count, CHARAC_LOAD_MERCENARY* data);
    // IDA_EA: 0x868E704
    // IDA_NAME: _ZN5CUser22SetMercenaryInfoPacketER11PacketGuard
    // DEMANGLED: CUser::SetMercenaryInfoPacket(PacketGuard &)
    bool SetMercenaryInfoPacket(PacketGuard& packet);
    // IDA_EA: 0x868E596
    // IDA_NAME: _ZN5CUser25AddCurCharacMercenaryInfoEv
    // DEMANGLED: CUser::AddCurCharacMercenaryInfo(void)
    bool AddCurCharacMercenaryInfo();
    // IDA_EA: 0x868EA68
    // IDA_NAME: _ZNK5CUser24CalcMercenaryAvatarBonusEv
    // DEMANGLED: CUser::CalcMercenaryAvatarBonus(void)const
    int CalcMercenaryAvatarBonus() const;

    // IDA_EA: 0x868EBAA
    // IDA_NAME: _ZN5CUser18setAvatarTypeBonusEc
    // DEMANGLED: CUser::setAvatarTypeBonus(char)
    char setAvatarTypeBonus(char avatar_bonus_index);
    void* loadInformNoticeFlag(char* flags);
    // IDA_EA: 0x86781B4
    // IDA_NAME: _ZN5CUser20SendDungeonClearListEv
    // DEMANGLED: CUser::SendDungeonClearList(void)
    void SendDungeonClearList();
    // IDA_EA: 0x8656ABC
    // IDA_NAME: _ZN5CUser18SendRevengeDungeonEv
    // DEMANGLED: CUser::SendRevengeDungeon(void)
    void SendRevengeDungeon();
    void SendMaxEquipLevel();
    // IDA_EA: 0x86467EE
    // IDA_NAME: _ZN5CUser34requestCubePremiumInfoIfContractedEv
    // DEMANGLED: CUser::requestCubePremiumInfoIfContracted(void)
    void requestCubePremiumInfoIfContracted();
    // IDA_EA: 0x868DEA4
    // IDA_NAME: _ZN5CUser23send_aura_avatar_optionEv
    // DEMANGLED: CUser::send_aura_avatar_option(void)
    void send_aura_avatar_option();
    // IDA_EA: 0x868F0E8
    // IDA_NAME: _ZN5CUser16sendCharacOptionEv
    // DEMANGLED: CUser::sendCharacOption(void)
    void sendCharacOption();
    // IDA_EA: 0x8680BD2
    // IDA_NAME: _ZN5CUser22discard_mouse_registerEv
    // DEMANGLED: CUser::discard_mouse_register(void)
    void discard_mouse_register();
    // IDA_EA: 0x868A99A
    // IDA_NAME: _ZN5CUser18CheckMaxLuckyLevelEv
    // DEMANGLED: CUser::CheckMaxLuckyLevel(void)
    int CheckMaxLuckyLevel();
    // IDA_EA: 0x86800C6
    // IDA_NAME: _ZN5CUser17onSelectCharacterEv
    // DEMANGLED: CUser::onSelectCharacter(void)
    int onSelectCharacter();
    // IDA_EA: 0x868AAEA
    // IDA_NAME: _ZN5CUser24SendOneADayItemShopIndexERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::SendOneADayItemShopIndex(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    void SendOneADayItemShopIndex(
        const std::vector<std::pair<int, int> >& selected_items);
    int SendMoneyFullReason(ENUM_MONEY_FULL_REASON reason,
                            unsigned long current_money,
                            unsigned long requested_money);
    bool CalLevelUpItemCheck(unsigned long item_id);
    bool CheckItemLock(int inventory_type, int slot) const;
    // IDA_EA: 0x865E0AE
    // IDA_NAME: _ZN5CUser13use_stackableEsiii
    // DEMANGLED: CUser::use_stackable(short,int,int,int)
    int use_stackable(short slot,
                      int item_space,
                      int arg3,
                      int arg4);

    // IDA_EA: 0x865EED2
    // IDA_NAME: _ZN5CUser12change_equipEisi
    // DEMANGLED: CUser::change_equip(int,short,int)
    int change_equip(int source_space,
                     int16_t source_slot,
                     int equipment_slot);

    // IDA_EA: 0x865F382
    // IDA_NAME: _ZN5CUser19change_equip_expandEiiiiPi
    // DEMANGLED: CUser::change_equip_expand(int,int,int,int,int *)
    int change_equip_expand(int source_space,
                            int source_slot,
                            int destination_space,
                            int destination_slot,
                            int* actual_destination_slot);

    // IDA_EA: 0x865F614
    // IDA_NAME: _ZN5CUser26exchange_interspace_expandEiimiiimPicc
    // DEMANGLED: CUser::exchange_interspace_expand(int,int,unsigned long,int,int,int,unsigned long,int *,char,char)
    int exchange_interspace_expand(int source_space,
                                   int source_slot,
                                   unsigned long source_item_id,
                                   int source_count,
                                   int destination_space,
                                   int destination_slot,
                                   unsigned long destination_item_id,
                                   int* actual_destination_slot,
                                   char channel_equip_switch,
                                   char expand_equip_switch);

    // IDA_EA: 0x865F7B0
    // IDA_NAME: _ZN5CUser19exchange_interspaceEiimiiimPi
    // DEMANGLED: CUser::exchange_interspace(int,int,unsigned long,int,int,int,unsigned long,int *)
    int exchange_interspace(int source_space,
                            int source_slot,
                            unsigned long source_item_id,
                            int source_count,
                            int destination_space,
                            int destination_slot,
                            unsigned long destination_item_id,
                            int* actual_destination_slot);

    // IDA_EA: 0x865E994
    // IDA_NAME: _ZNK5CUser17CheckCoolTimeItemEm
    // DEMANGLED: CUser::CheckCoolTimeItem(unsigned long)const
    int CheckCoolTimeItem(unsigned long item_index) const;

    // IDA_EA: 0x865EA62
    // IDA_NAME: _ZN5CUser22_processContinuousItemEjP14CStackableItem
    // DEMANGLED: CUser::_processContinuousItem(unsigned int,CStackableItem *)
    char _processContinuousItem(unsigned int item_index,
                                CStackableItem* item);

    // IDA_EA: 0x865EB0C
    // IDA_NAME: _ZN5CUser20_processCoolTimeItemEP14CStackableItemjj
    // DEMANGLED: CUser::_processCoolTimeItem(CStackableItem *,unsigned int,unsigned int)
    void _processCoolTimeItem(CStackableItem* item,
                              unsigned int item_index,
                              unsigned int current_sec);

    // IDA_EA: 0x865EC0E
    // IDA_NAME: _ZN5CUser18_processEffectItemEP14CStackableItemjj
    // DEMANGLED: CUser::_processEffectItem(CStackableItem *,unsigned int,unsigned int)
    void _processEffectItem(CStackableItem* item,
                            unsigned int item_index,
                            unsigned int current_sec);

    // IDA_EA: 0x8659B60
    // IDA_NAME: _ZN5CUser33_processSpeicalContinuousTimeItemEj
    // DEMANGLED: CUser::_processSpeicalContinuousTimeItem(unsigned int)
    char _processSpeicalContinuousTimeItem(unsigned int item_index);

    // IDA_EA: 0x8658B2A
    // IDA_NAME: _ZN5CUser22UpdateCoolTimeItemInfoEj
    // DEMANGLED: CUser::UpdateCoolTimeItemInfo(unsigned int)
    void UpdateCoolTimeItemInfo(unsigned int current_sec);

    // IDA_EA: 0x8658D90
    // IDA_NAME: _ZN5CUser20UpdateEffectItemInfoEj
    // DEMANGLED: CUser::UpdateEffectItemInfo(unsigned int)
    void UpdateEffectItemInfo(unsigned int current_sec);

    // IDA_EA: 0x8658FF6
    // IDA_NAME: _ZN5CUser20LoadCoolTimeItemInfoEP20CONTINUOUS_ITEM_INFO
    // DEMANGLED: CUser::LoadCoolTimeItemInfo(CONTINUOUS_ITEM_INFO *)
    int LoadCoolTimeItemInfo(CONTINUOUS_ITEM_INFO* item_info);

    // IDA_EA: 0x86590F0
    // IDA_NAME: _ZN5CUser18LoadEffectItemInfoEP20CONTINUOUS_ITEM_INFO
    // DEMANGLED: CUser::LoadEffectItemInfo(CONTINUOUS_ITEM_INFO *)
    int LoadEffectItemInfo(CONTINUOUS_ITEM_INFO* item_info);

    // IDA_EA: 0x86591FE
    // IDA_NAME: _ZN5CUser20SendCoolTimeItemInfoEj
    // DEMANGLED: CUser::SendCoolTimeItemInfo(unsigned int)
    int SendCoolTimeItemInfo(unsigned int current_sec);

    // IDA_EA: 0x8659366
    // IDA_NAME: _ZN5CUser18SendEffectItemInfoEj
    // DEMANGLED: CUser::SendEffectItemInfo(unsigned int)
    int SendEffectItemInfo(unsigned int current_sec);

    int set_acc_info(unsigned int account_id, const char* account_name);
    bool RemoveRestrictedGoods(int item_id);
    char saveTaxMoneyForUpperMember(int tax_money);
    int eventDeliveryBuyItem(int item_id, const Inven_Item& item);
    int IsAddableGuildFund(int amount);
    LimitItemUsage* GetItemLimitInfo(int item_id);
    void send_clear_quest_list();
    char GetAuraAvatarOption(stAuraAvatarOption& option) const;

    // IDA_EA: 0x8672042
    // IDA_NAME: _ZN5CUser23_CalcItemVectorMultiplyERSt6vectorISt4pairIiiESaIS2_EEi
    // DEMANGLED: CUser::_CalcItemVectorMultiply(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int)
    bool _CalcItemVectorMultiply(std::vector<std::pair<int, int> >& items,
                                 int multiplier);

    // IDA_EA: 0x86720D0
    // IDA_NAME: _ZN5CUser27_IsCompoundResult2MoreEquipERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CUser::_IsCompoundResult2MoreEquip(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    int _IsCompoundResult2MoreEquip(
        const std::vector<std::pair<int, int> >& items);

    // IDA_EA: 0x8670E38
    // IDA_NAME: _ZN5CUser14_GetRecipeItemEmbRi
    CStackableItem* _GetRecipeItem(unsigned long recipe_slot,
                                   bool avatar_recipe,
                                   int& recipe_add_info);
    // IDA_EA: 0x8670F1E
    // IDA_NAME: _ZN5CUser19_CheckCompoundSkillEPK14CStackableItemb
    bool _CheckCompoundSkill(const CStackableItem* recipe,
                             bool avatar_recipe);
    // IDA_EA: 0x8671102
    // IDA_NAME: _ZN5CUser17_ProcCompoundCoreERKSt6vectorISt4pairIiiESaIS2_EERS4_RS0_IiSaIiEEibP11PacketGuardRbt16stEquipConditionP14CStackableItemR10Inven_Item
    int _ProcCompoundCore(
        const std::vector<std::pair<int, int> >& materials,
        std::vector<std::pair<int, int> >& results,
        std::vector<int>& changed_slots,
        int recipe_slot,
        bool avatar_recipe,
        PacketGuard* packet,
        bool& equipment_changed,
        unsigned short compound_count,
        stEquipCondition condition,
        CStackableItem* recipe,
        Inven_Item& material);
    // IDA_EA: 0x8671EB2
    // IDA_NAME: _ZN5CUser14copyItemOptionER10Inven_ItemS1_
    int copyItemOption(Inven_Item& output, Inven_Item& material);
    // IDA_EA: 0x867306E
    // IDA_NAME: _ZN5CUser24setRecipeEqiupmentOptionER10Inven_Item16stEquipConditioniPK14CStackableItemi
    void setRecipeEqiupmentOption(Inven_Item& output,
                                  stEquipCondition condition,
                                  int output_slot,
                                  const CStackableItem* recipe,
                                  int output_type);
    // IDA_EA: 0x8682082
    // IDA_NAME: _ZNK5CUser18getCompoundUpgradeEPK14CStackableItem16stEquipCondition
    int getCompoundUpgrade(const CStackableItem* recipe,
                           stEquipCondition condition) const;
    // IDA_EA: 0x868C45E / 0x868C4F4
    int is_recipe_items_avatar_type(
          const std::vector<std::pair<int, int> >& items);
    int process_recipe_avatar_items(int recipe_slot,
                                    bool avatar_recipe,
                                    unsigned short compound_count,
                                    int command_id,
                                    const CStackableItem* recipe);
    // IDA_EA: 0x8692F62 / 0x869304E
    void compoundSystemMessage(const Inven_Item& item);
    char compoundGuildMessage(const Inven_Item& item);
    // IDA_EA: 0x86646C2
    // IDA_NAME: _ZN5CUser15_onCompoundItemEi
    void _onCompoundItem(int item_index);

    // IDA_EA: 0x867218C
    // IDA_NAME: _ZN5CUser13compound_itemEibti
    // DEMANGLED: CUser::compound_item(int,bool,unsigned short,int)
    // This is the exact old transaction boundary.  Its complete recipe,
    // material, inventory rollback, expert-job, notification, and persistence
    // body remains a separate C-owned restoration task; no local substitute is
    // supplied by the two dispatcher entry points.
    int compound_item(int recipe_slot,
                      bool avatar_recipe,
                      unsigned short compound_count,
                      int command_id);

    // IDA_EA: 0x8673342
    // IDA_NAME: _ZN5CUser16IsOldLotteryItemEj
    // DEMANGLED: CUser::IsOldLotteryItem(unsigned int)
    int IsOldLotteryItem(unsigned int item_id);

    // IDA_EA: 0x86733A2
    // IDA_NAME: _ZN5CUser12lottery_itemEi
    // DEMANGLED: CUser::lottery_item(int)
    // Original normal-lottery item, inventory, reward, audit, and persistence
    // transaction.  The C provider preserves the retail packet and mutation
    // order, including the money-reward branch.
    int lottery_item(int item_id);

    // IDA_EA: 0x868D1EC
    // IDA_NAME: _ZN5CUser35send_broadcast_lottery_gold_messageEji
    // DEMANGLED: CUser::send_broadcast_lottery_gold_message(unsigned int,int)
    void send_broadcast_lottery_gold_message(unsigned int item_id,
                                             int reward_money);

    // IDA_EA: 0x8674968
    // IDA_NAME: _ZN5CUser21multibox_lottery_itemEii
    // DEMANGLED: CUser::multibox_lottery_item(int,int)
    // This is the original multi-box lottery transaction boundary.  Its
    // material, reward, counter, and persistence body remains C-owned link
    // debt; CMD211 invokes the exact named transaction directly.
    int multibox_lottery_item(int item_id, int count);

    // IDA_EA: 0x8675AC6 / 0x8675D0C
    // IDA_NAME: _ZN5CUser27process_use_locked_multiboxEiiRSt6vectorISt4pairI10Inven_ItembESaIS3_EE /
    //           _ZN5CUser29process_use_unlocked_multiboxEiiRSt6vectorISt4pairI10Inven_ItembESaIS3_EE
    int process_use_locked_multibox(
        int box_slot,
        int material_slot,
        std::vector<std::pair<Inven_Item, bool> >& rewards);
    int process_use_unlocked_multibox(
        int box_slot,
        int expected_count,
        std::vector<std::pair<Inven_Item, bool> >& rewards);

    // IDA_EA: 0x8675EDC / 0x8675FD2 / 0x86761E0 / 0x8676296.
    int check_multibox_error(int required_empty_slots);
    int check_locked_multibox_error(const CItem* box,
                                    int material_slot,
                                    bool booster_is_full);
    int check_unlocked_multibox_error(const CItem* box,
                                      int expected_count);
    int check_insert_lottery_reward(
        const CItem* box,
        std::vector<std::pair<Inven_Item, bool> >& candidates,
        std::vector<std::pair<Inven_Item, bool> >& accepted_rewards);

    // IDA_EA: 0x86767D2
    // IDA_NAME: _ZN5CUser22use_multibox_statisticEj
    void use_multibox_statistic(unsigned int item_id);

    // Verified non-provider dependencies retained for their real owners.
    int GetPcRoomRentInfo(SIG_SAVE_PC_ROOM_RENT_ITEM* data);
    int GetDimensionRentInfo(SIG_SAVE_DIMENSION_RENT_ITEM* data);
    Inven_Item* GetCurCharacInventoryRef(unsigned short inventory_type,
                                         unsigned short slot);
    const Inven_Item* GetCurCharacInventoryRef(
        unsigned short inventory_type,
        unsigned short slot) const;
    // IDA_EA: 0x868B4FC
    // IDA_NAME: _ZN5CUser29getCharacSlotCountDefaultInfoEv
    // DEMANGLED: CUser::getCharacSlotCountDefaultInfo(void)
    int getCharacSlotCountDefaultInfo();

    // IDA_EA: 0x868FB22
    // IDA_NAME: _ZN5CUser20sendInformNoticeFlagEv
    // DEMANGLED: CUser::sendInformNoticeFlag(void)
    int sendInformNoticeFlag();
    void add_inventory_item(unsigned int item_id);
    int questAutoClear(int level);

private:
    // Only the concrete owners below may touch the recovered direct children.
    // There is deliberately no CUser::network() compatibility bridge: every
    // migrated consumer names this real in-object CNetwork member directly.
    // GameWorld's statistics and midnight-maintenance members directly read
    // network_ and action_point_manager_ in the 0627 implementation.
    friend class GameWorld;
    friend class CParty;
    // CharacAdd/CharacDel index the owning user's persisted character vector
    // directly in the old implementation.
    friend class cUserHistoryLog;
    // Inter_LoadCharac @0x84C3398 serializes this user's actual embedded
    // CNetwork peer endpoint into the signal-157 LogDB audit record when the
    // DB payload account id differs.  No offset bridge is valid here.
    friend class Inter_LoadCharac;
    friend class DisPatcher_DeleteCharac;
    friend class DisPatcher_SelectCharac;
    friend class Dispatcher_RequestCharacSkillInfo;
    // SkillSlot::SendSkillInfo @0x86086CE serializes the selected element's
    // job and packed growth type directly from the persisted character
    // vector.  Keep that ownership instead of rebuilding expanded +0x79a38,
    // +0x26 and +0x29 at the call site.
    friend class SkillSlot;
    // Inter_LoadCharacLinkBonus @0x84E1F02 reads the linked slot directly
    // after the original vector-size gate, rather than through a fallback
    // name accessor.
    friend class Inter_LoadCharacLinkBonus;
    // Inter_LoadDimensionInout is the original DB-reply consumer and must
    // update the selected character record, not merely the current one.
    friend class Inter_LoadDimensionInout;
    // These two DB-return consumers update the indexed persisted character
    // record selected by the reply; getCurCharacR() is not equivalent here.
    friend class Inter_RequestCharacSkillInfo;
    friend class Inter_RequestTagCharacInfo;
    friend class DisPatcher_Exit;
    // CMD040's original Party_hack_check body reads the embedded peer
    // endpoint of every party member.  Keep that direct child access in the
    // dispatcher rather than introducing a synthetic CUser network accessor.
    friend class DisPatcher_FinishLoad;
    // WarRoom::Join serializes the same in-object endpoint into the original
    // room peer packet (IDA 0x86BAE9A).
    friend class WarRoom;
    // CMD16 reads the actual embedded CNetwork NAT field after a failed
    // CParty::dungeon_start, exactly as the original dispatcher does.
    friend class DisPatcher_SelectDungeon;
    // CMD42 updates the embedded monster-position checker and its adjacent
    // special-kill counter directly.  Keep that original ownership without
    // manufacturing non-IDA CUser wrapper symbols for private access.
    friend class DisPatcher_DieMob;
    friend class pvp_assault::CAssaultPlace;
    // CMD010 reads the actual embedded CNetwork NAT state for both peers.
    friend class DisPatcher_ReqPeer;
    friend class Dispatcher_CharacterStatistic;
    // CMD488 owns the real in-object character vector and character-view
    // update.  Friendship keeps the handler on the recovered typed children,
    // instead of adding a synthetic offset accessor.
    friend class Dispatcher_SelectStriker;
    // CMD506 writes the embedded GrowthWeaponEvent child at expanded +0x08ef34;
    // friendship avoids a synthetic offset accessor for this stateful ABI.
    friend class Dispatcher_GrowthWeaponEventUseMaterial;
    // CMD505 reads the same child before delegating the inventory replacement
    // to GrowthEquipEvent.
    friend class Dispatcher_GrowthWeaponEventChangeInfinity;
    // GrowthEquipEvent::SendInfinityGrowthWeapon @0x80F74B2 updates the
    // same embedded child after it atomically replaces the material slot.
    // This is a real owner relationship, not an offset-based facade.
    friend class GrowthEquipEvent;
    // CMD108 @0x8203878 serializes the real in-object CNetwork child into
    // SIG_PARTY_STATUS.  No CUser compatibility accessor is introduced.
    friend class Dispatcher_GatheringPartyStatus;
    // CMD48 @0x81C4B7A uses the original embedded network endpoint to
    // persist and disconnect each affected party member; keep that direct
    // CUser child relationship rather than introducing a network facade.
    friend class DisPatcher_MoveMap;
    // PvP_Room::join_room @0x85D6CF2 serializes the embedded network
    // endpoint and clears the per-match pending win point directly.
    friend class PvP_Room;
    friend class CTradeSpace;
    // The CeraShop restriction manager owns the exact UserInfo object at
    // expanded +0x08d534.  Friendship avoids a raw CUser-offset facade.
    friend class CerashopAddRestrict::Manager;
    // ItemVendingMachine::BillingRecord serializes the original in-object
    // CNetwork peer address into SIG_BILLING_RECORD.  It is the direct owner
    // of that transaction; do not introduce a synthetic network accessor.
    friend class ItemVendingMachine;
    // CGameManager::user_exit @0x82985A8 resets the embedded epoll socket
    // after its persistence transaction and before returning this pool slot.
    friend class CGameManager;
    // The anti-bot request/reply handshake owns these exact scalar fields:
    // CMD603 changes the four-state transaction marker, CMD605 counts
    // callback frames, and Inter slot 594 emits the paired S2C request.
    friend class Dispatcher_Antibot;
    friend class Dispatcher_Antibot_DPCallBack;
    friend class Inter_Antibot_SendBufferClient;
    friend class Inter_LoadCerashopAddRestrictType;
    friend class LevelupSupportEventManger;
    friend class Inter_LoadEventLevelupSupport;
    // Taiwan internal reply code 12 writes the byte at expanded +0x08ef81.
    // Keep that state as an owned CUser field instead of a raw offset facade.
    friend class DBSelectMobileAuthRewardTw;
    friend class InterSelectMobileAuthReward;
    // CMD597 @0x825FC3A reads this original in-object event vector directly,
    // including the old unchecked -1 index path; no accessor facade exists.
    friend class Dispatcher_LevelupSupportReqestItem;
    friend class WongWork::CBossStage;
    friend class WongWork::CBossTower;
    // CMD16's Advance Altar branch configures this verified in-object manager
    // before ProcStage begins.  The original StageControl reads it directly.
    friend class advancealtar::StageControl;
    // ProcStage's clear/reward transaction @0x812E3F6 invokes the complete
    // manager lifecycle in this same child object: clearStage, achievement
    // insertion, ticket consumption, next-stage setup, and unlocked-shop
    // collection.  The friendship preserves that original direct owner
    // relationship without an offset accessor.
    friend class advancealtar::ProcStage;
    // Manager::finishLoad @0x8130EB2 checks the same embedded ticket state
    // before it changes the active stage and emits NOTI30.
    friend class advancealtar::Manager;
    // CharacData's slot materialization calls the embedded manager directly,
    // matching CharacAdvanceAltarManager::getSlotItem @0x8132638.
    friend struct advancealtar::CharacData;
    friend class advancealtar::DBAndExchangeServerProc;
    // DBTask outer signal 25 installs the typed persisted record into the
    // embedded manager before releasing CACHE_SECTION 10.
    friend class advancealtar::Inter_Load;
    // CMD579 sends the actual in-object achievement-reward transaction.
    // It remains a direct child access because C2/D2 establish this manager
    // at expanded +0x08e2b0; no synthetic offset accessor is introduced.
    friend class advancealtar::Dispatcher_GetAchievementReward;
    friend class advancealtar::Dispatcher_BuyItem;
    friend class advancealtar::Dispatcher_ExchangeSlot;
    friend class advancealtar::Dispatcher_ResetStar;
    friend class advancealtar::Dispatcher_SetSlot;
    friend class advancealtar::Dispatcher_StartGame;
    friend class Dispatcher_PeerConnectResult;
    friend class NetworkThread;
    friend class DisPatcher_SetIP;
    // CMD 1 login @0x81E8C78 owns the following per-connection login
    // session fields.  It is deliberately a friend instead of reaching this
    // state through a synthetic offset helper.
    friend class DisPatcher_Login;
    // Inter_LoginInfo @0x84C5294 continues the same login transaction after
    // DB_Login returns SIG_LOGIN_DATA.  Its original body owns the named
    // peer-IP, Black-IP and clean-pad children below; retain direct member
    // semantics instead of a raw-offset compatibility facade.
    friend class Inter_LoginInfo;
    // Slot-32's original post-load history record reads the embedded network
    // endpoint directly before the character enters the world.
    friend class Inter_LoadEtc;
    // Inter_MonitorLoginOK @0x84C7E0C owns the original monitor-login
    // completion writes to these named members: network_ (+0xe0), history
    // stream (+0x79a48), and anti-bot fields (+0x8d250/+0x8d5a1/
    // +0x8d5a2).  Its i386 offsets are asserted below; no CUser offset
    // accessor is introduced for this DB return path.
    friend class Inter_MonitorLoginOK;
    friend class InterUpdateP2pHolePunchingState;
    friend class XNuclear::CHades;
    friend struct Secu_AccountHacking;
    friend struct Secu_GoldControl;
    friend int OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO(
        CUser*, PacketBuf*);
    friend void Secu_GoldControl_capture_hades_endpoint(
        Secu_GoldControl*, char*, char*);
    friend struct CUserPhysicalLayoutAssert;

    // CUser physical tail recovered from C2 @0x864E410 and D2 @0x864EF9C.
    // Named byte ranges are CUser's own scalar state between independently
    // proven children; none substitutes for a known non-trivial object.
    CNetwork4096_450000 network_;                         // +0x0000e0
    CUserNetworkLifecycleState network_lifecycle_state_;  // +0x06ef68
    UserMercenaryInfoMgr mercenary_info_;                 // +0x06ef6c
    CUserRentalInfoStore rental_info_store_;              // +0x06ef78
    // C2 @0x864E410 writes these scalar slots only after every direct child
    // has been constructed.  They remain named so the C2 body never needs an
    // offset bridge into CUser.
    uint8_t save_rental_info_to_exchange_;                // +0x06ef90
    uint8_t load_rental_info_from_exchange_;              // +0x06ef91
    Taiwan::GarenaAuthData garena_auth_data_;             // +0x06ef92
    uint8_t allow_location_geo_state_;                    // +0x06effa
    uint8_t rental_to_cargo_pad_;                         // +0x06effb
    CAccountCargo account_cargo_;                         // +0x06effc
    CGameOption game_option_;                             // +0x06fd64
    uint8_t gm_quest_flag_;                               // +0x070c1c
    uint8_t force_drop_flag_;                             // +0x070c1d
    char web_address_[0x10];                              // +0x070c1e
    uint8_t web_address_to_play_experience_state_[0x02]; // +0x070c2e
    int32_t play_experience_;                             // +0x070c30
    uint8_t break_away_dungeon_clear_;                    // +0x070c34
    uint8_t break_away_clear_padding_[3];                 // +0x070c35
    int32_t break_away_dungeon_clear_count_;              // +0x070c38
    int32_t break_away_accure_cera_;                      // +0x070c3c
    int32_t break_away_quest_time_;                       // +0x070c40
    int32_t break_away_reward_order_;                     // +0x070c44
    int32_t multibox_lottery_item_fail_count_;            // +0x070c48
    uint8_t break_away_reward_data_;                      // +0x070c4c
    uint8_t used_break_away_luck_point_;                  // +0x070c4d
    uint8_t break_away_luck_padding_[2];                  // +0x070c4e
    STBreakAwayRewards break_away_rewards_;               // +0x070c50
    uint8_t charac_link_message_flag_;                    // +0x070c64
    uint8_t link_charac_disconnect_flag_;                 // +0x070c65
    int8_t disconnect_link_charac_slot_index_;            // +0x070c66
    STSkillCommandData::STCommand skill_commands_[50];    // +0x070c67
    uint8_t skill_command_to_slot_index_padding_;         // +0x070d2f
    uint16_t slot_index_;                                 // +0x070d30
    uint16_t increment_id_;                              // +0x070d32
    // Old C2's first interleaved scalar write: after skill_commands_ and
    // before Client_Spec C2, it clears this word at +0x070d34.
    uint16_t unique_id_;                                  // +0x070d34
    uint8_t unique_id_to_account_id_padding_[2];          // +0x070d36
    int32_t account_id_;                                  // +0x070d38
    char account_name_[0x15];                             // +0x070d3c
    uint8_t account_name_to_user_detail_padding_;         // +0x070d51
    char user_name_[0x80];                                // +0x070d52
    char ssn_[0x80];                                      // +0x070dd2
    char user_email_[0x10];                               // +0x070e52
    uint8_t user_email_to_client_spec_padding_[0x02];     // +0x070e62
    uint32_t client_spec_c2_state_;                       // +0x070e64
    Client_Spec client_spec_;                             // +0x070e68
    uint8_t birth_day_;                                   // +0x071a60
    uint8_t birth_month_;                                 // +0x071a61
    uint8_t birth_to_gm_mode_state_[2];                  // +0x071a62
    uint8_t game_master_mode_;                            // +0x071a64
    uint8_t gm_mode_to_upgrade_padding_[0x03];            // +0x071a65
    int32_t gm_upgrade_mode_;                             // +0x071a68
    int32_t age_;                                         // +0x071a6c
    uint8_t program_logout_;                              // +0x071a70
    uint8_t program_logout_to_accept_seed_state_[3];      // +0x071a71
    uint32_t accept_seed_from_date_;                      // +0x071a74
    std::list<int> restricted_goods_;                     // +0x071a78
    CPacketVerifier packet_verifier_;                     // +0x071a80
    WongWork::CUserPremium user_premium_;                 // +0x071aa8
    WongWork::CMailBox* mailbox_;                         // +0x072428
    uint8_t mailbox_to_account_time_state_[4];            // +0x07242c
    uint32_t account_last_play_time_;                     // +0x072430
    uint32_t login_time_;                                 // +0x072434
    uint8_t login_out_state_;                             // +0x072438
    uint8_t login_to_hack_analyzer_state_[3];             // +0x072439
    WongWork::CHackAnalyzer hack_analyzer_;               // +0x07243c
    Secu_AccountHacking account_hacking_;                 // +0x079e78
    CodeHackCheckStorage code_hack_storage_;              // +0x079e8c
    int32_t std_drop_rate_;                               // +0x079eb4
    int32_t trade_punish_type_;                           // +0x079eb8
    uint8_t trade_punish_over_thirty_days_;               // +0x079ebc
    uint8_t tournament_account_;                          // +0x079ebd
    uint8_t login_account_to_server_group_padding_[2];    // +0x079ebe
    int32_t server_group_;                                // +0x079ec0
    int32_t online_preliminary_server_group_;             // +0x079ec4
    uint32_t online_preliminary_charac_no_;               // +0x079ec8
    int32_t online_preliminary_play_type_;                // +0x079ecc
    uint8_t joined_school_point_event_;                   // +0x079ed0
    uint8_t account_fatigue_padding_;                     // +0x079ed1
    uint16_t account_used_fatigue_;                       // +0x079ed2
    // quest_action @0x866DFF6..0x866E0C9 compares these two owner-local
    // counters to throttle repeated finish-quest reward requests.
    uint32_t quest_reward_request_tick_;                  // +0x079ed4
    uint32_t quest_reward_request_count_;                 // +0x079ed8
    CGameMasterCharacter* game_master_character_;        // +0x079edc
    uint32_t pc_room_no_;                                 // +0x079ee0
    uint8_t first_login_;                                 // +0x079ee4
    uint8_t return_user_;                                 // +0x079ee5
    uint8_t return_user_first_login_;                     // +0x079ee6
    uint8_t return_user_padding_;                         // +0x079ee7
    int32_t return_user_expire_time_;                     // +0x079ee8
    uint8_t save_member_bonus_fatigue_;                   // +0x079eec
    uint8_t member_bonus_fatigue_padding_;                // +0x079eed
    uint16_t account_member_bonus_fatigue_;               // +0x079eee
    int32_t chatting_message_count_;                      // +0x079ef0
    std::string user_strings_[2];                         // +0x079ef4
    uint8_t duplicate_message_index_;                     // +0x079efc
    uint8_t strings_to_before_area_time_state_[3];        // +0x079efd
    // Old C2's second interleaved scalar write: after both strings and
    // before CharacMoneyLog construction, it clears +0x079f00.
    uint32_t before_area_time_;                           // +0x079f00
    CharacMoneyLog charac_money_logs_[7];                 // +0x079f04
    // NextCharacMoneyLog/SendMoenyLog @0x868F1C0/@0x868F472 consume this
    // bounded index before the seven named records above are serialized.
    uint32_t money_log_index_;                            // +0x079f58
    // CBossStage::handleMoveMap @0x814F8A3/0x814F8C9 consumes either
    // one-shot GM override, then clears it.  ResetCurCharac @0x865851C
    // clears the same two physical members.
    uint8_t gm_boss_tower_stage_override_;                // +0x079f5c
    uint8_t gm_boss_tower_stage_to_map_override_[3];
    uint32_t gm_boss_tower_map_index_override_;           // +0x079f60
    int32_t mileage_;                                     // +0x079f64
    // Old C2's third interleaved scalar write: immediately after all seven
    // CharacMoneyLog C2 calls, it clears +0x079f68.
    uint8_t pu_user_;                                     // +0x079f68
    uint8_t guild_add_exp_flag_;                          // +0x079f69
    uint8_t money_log_to_accept_random_state_[0x06];      // +0x079f6a
    uint32_t accept_random_nonce_;                        // +0x079f70
    uint32_t backup_seed_;                                // +0x079f74
    std::vector<_Charac_info> characters_;                // +0x079f78
    CCharacterView character_view_;                       // +0x079f84
    uint8_t character_view_to_c2_state_[3];               // +0x079f85
    FILE* history_stream_;                                // +0x079f88
    // add_pvp_result @0x865C678 consumes and clears this per-match pending
    // win-point accumulator before it appends the history-log record.
    uint32_t pending_pvp_win_point_;                       // +0x079f8c
    cUserHistoryLog history_log_;                         // +0x079f90
    charac_expand::CDataMgr charac_expand_data_;          // +0x079fd4
    InventoryMemory inventory_memory_;                    // +0x07a024
    uint8_t inventory_memory_to_quest_state_[1];          // +0x086aeb
    UserQuest quest_;                                     // +0x086aec
    uint8_t village_attacked_reward_;                     // +0x08e108
    uint8_t pc_room_auth_;                                // +0x08e109
    uint8_t quest_to_item_vector_state_[2];               // +0x08e10a
    std::vector<unsigned int> black_list_;                // +0x08e10c
    // CMD603 / Inter slot 594 transition this original four-state anti-bot
    // handshake record through 0 -> 1 -> 2 -> 3 -> 4.
    uint32_t antibot_packet_state_;                       // +0x08e118
    int32_t playing_dungeon_index_;                       // +0x08e11c
    uint8_t playing_dungeon_to_last_packet_state_[0x04]; // +0x08e120
    CUserLastPacketIdentity last_packet_identity_;        // +0x08e124
    int32_t antibot_key_;                                 // +0x08e128
    uint32_t client_version_;                             // +0x08e12c
    CUserAntibotInfo antibot_user_info_;                  // +0x08e130
    CDungeonGainedGold dungeon_gained_gold_;              // +0x08e17c
    std::vector<LimitItemUsage> limit_item_usage_;        // +0x08e180
    std::vector<DBLogItemInfo> db_log_items_;             // +0x08e18c
    std::list<int> reset_list_;                           // +0x08e198
    Mutex first_mutex_;                                   // +0x08e1a0
    Mutex second_mutex_;                                  // +0x08e1b8
    int32_t before_area_;                                 // +0x08e1d0
    int32_t area_;                                        // +0x08e1d4
    int32_t current_area_;                                // +0x08e1d8
    // Old C2's fourth interleaved scalar write: after both Mutex C2 calls
    // and before pvp_master_ids_ construction, it clears +0x08e1dc.
    uint32_t state_;                                      // +0x08e1dc
    uint16_t connection_check_count_;                     // +0x08e1e0
    uint8_t connection_check_count_padding_[2];           // +0x08e1e2
    uint32_t move_space_;                                 // +0x08e1e4
    uint32_t connection_check_interval_;                  // +0x08e1e8
    uint32_t resume_checksum_time_;                       // +0x08e1ec
    uint32_t checksum_answer_count_;                      // +0x08e1f0
    uint8_t logout_state_byte_;                           // +0x08e1f4
    uint8_t logout_state_byte_padding_[3];                // +0x08e1f5
    // add_guild_pvp_result @0x865C936 updates this direct scalar after the
    // guild-proxy packet and local PVP-record notification are sent.
    int32_t guild_pvp_point_;                              // +0x08e1f8
    std::set<unsigned int> pvp_master_ids_;               // +0x08e1fc
    // CUser's session-placement tuple.  The contiguous original words are
    // read and written by the position, trade, party, PvP and tower APIs.
    // In particular, CheckInTrade/SetTradeSpace use +0x08e21a and the trade
    // escrow setup stores the peer's unique-id at the separate +0x08e474
    // scalar below.
    uint16_t position_x_;                                 // +0x08e214
    uint16_t position_y_;                                 // +0x08e216
    uint8_t direction_;                                   // +0x08e218
    uint8_t position_to_trade_padding_;                   // +0x08e219
    int16_t trade_space_index_;                           // +0x08e21a
    int16_t party_index_;                                 // +0x08e21c
    // SetPvpIndex/GetPvpIndex @0x85DF9B2/@0x822FE74.
    int16_t pvp_index_;                                   // +0x08e21e
    int16_t war_room_index_;                              // +0x08e220
    // No recovered operation names this short.  Keeping it distinct
    // preserves the observed tower-index offsets without introducing a raw
    // CUser gap facade.
    int16_t session_index_8d00a_;                          // +0x08e222
    int16_t blue_marble_index_;                           // +0x08e224
    int16_t death_tower_index_;                           // +0x08e226
    int16_t boss_tower_index_;                            // +0x08e228
    // get/setAdvanceAltarIndex and getAdvanceAltar use this signed pool
    // index.  A negative value denotes that the user owns no live altar
    // stage.  The old provisional name incorrectly associated it with PvP.
    int16_t advance_altar_index_;                         // +0x08e22a
    uint32_t registration_date_;                          // +0x08e22c
    uint32_t registration_time_;                          // +0x08e230
    uint32_t cera_;                                       // +0x08e234
    STGuildDBInfoOnly guild_info_;                        // +0x08e238
    STGuildAgitDBInfo guild_agit_info_;                   // +0x08e2f5
    uint8_t today_guild_member_;                          // +0x08e2f6
    uint8_t ting_flag_;                                   // +0x08e2f7
    uint32_t used_cera_;                                  // +0x08e2f8
    std::vector<LevelupSupportEventManger::EventInfo>
        levelup_support_events_;                          // +0x08e2fc
    uint8_t charac_info_load_lock_;                       // +0x08e308
    uint8_t charac_info_delete_lock_;                     // +0x08e309
    uint8_t charac_info_create_lock_;                     // +0x08e30a
    uint8_t charac_info_lock_padding_;                    // +0x08e30b
    int32_t last_login_charac_no_;                        // +0x08e30c
    int32_t last_login_channel_no_;                       // +0x08e310
    uint8_t last_login_charac_valid_;                     // +0x08e314
    // CacheCharacterMemory @0x864CABF copies this exact 12-byte completion
    // state into CACHE_CHARACTER_TYPE::load_state.  It is not anonymous
    // event padding within the security-card predecessor range.
    CacheCharacterLoadState cache_character_load_state_;  // +0x08e315
    uint8_t cache_to_trade_count_state_[3];               // +0x08e321
    uint32_t trade_count_;                                // +0x08e324
    int32_t report_mannerless_user_count_;                // +0x08e328
    WongWork::CSecurityCard security_card_;               // +0x08e32c
    Sanicova::CPad pad_;                                  // +0x08e378
    uint8_t hack_user_timer_;                             // +0x08e3c0
    uint8_t hack_user_flag_;                              // +0x08e3c1
    uint8_t hack_user_trade_padding_[2];                  // +0x08e3c2
    uint64_t total_hack_user_trade_gold_;                 // +0x08e3c4
    uint32_t total_hack_user_trade_count_;                // +0x08e3cc
    std::vector<stLookOuts> lookouts_;                    // +0x08e3d0
    std::vector<stLookOuts> lookout_history_;             // +0x08e3dc
    std::vector<tyIPCounters> ip_counters_;               // +0x08e3e8
    uint32_t revival_time_;                               // +0x08e3f4
    uint32_t use_apc_potion_time_;                        // +0x08e3f8
    CerashopAddRestrict::UserInfo cerashop_restrict_;     // +0x08e3fc
    uint8_t slot_effect_count_;                           // +0x08e45c
    uint8_t charac_slot_limit_;                           // +0x08e45d
    uint8_t present_avenger_title_;                       // +0x08e45e
    uint8_t present_title_to_ontime_state_;               // +0x08e45f
    int32_t ontime_last_recv_idx_;                        // +0x08e460
    uint8_t update_ontime_last_recv_idx_;                 // +0x08e464
    STExpReward exp_reward_;                              // +0x08e465
    // Inter slot 594 and CMD605 increment these independently bounded audit
    // counters.  CUser::reset and Inter_MonitorLoginOK clear both bytes.
    uint8_t antibot_send_buffer_count_;                   // +0x08e469
    uint8_t antibot_dproto_callback_count_;               // +0x08e46a
    // D2 @0x864EF9C first clears this byte before beginning reverse child
    // destruction; it is deliberately not left inside an anonymous gap.
    uint8_t power_up_;                                    // +0x08e46b
    uint32_t local_ip_address_;                           // +0x08e46c
    uint32_t public_ip_address_;                          // +0x08e470
    // CTradeSpace::set_traders @0x85295F0 writes the opposite CUser's
    // unsigned-short unique id as this complete word, clearing its high half.
    uint32_t trade_peer_unique_id_;                        // +0x08e474
    uint8_t hangame_user_;                                // +0x08e478
    uint8_t hangame_to_action_point_state_[0x03];         // +0x08e479
    APSystem::CActionPointManager action_point_manager_;  // +0x08e47c
    advancealtar::CharacAdvanceAltarManager
        advance_altar_manager_;                           // +0x08f178
    Character character_;                                 // +0x08f24c
    CUserCharacterStatisticState character_statistic_;    // +0x08f28c
    std::string character_string_;                        // +0x08f298
    uint16_t daily_badges_[3];                            // +0x08f29c
    uint8_t tutorial_skipable_;                           // +0x08f2a2
    uint8_t tutorial_to_fatigue_padding_;                 // +0x08f2a3
    // FatigueUp stores the current fatigue cost here; room-clear processing
    // consumes that same value through CCreatureMgr::GainExp and clears it.
    int32_t creature_exp_fatigue_;                        // +0x08f2a4
    int32_t debug_command_;                               // +0x08f2a8
    uint32_t guild_cargo_capacity_;                       // +0x08f2ac
    CUserChildrensDayEventState childrens_day_event_;     // +0x08f2b0
    uint16_t clean_pad_captcha_text_[5];                  // +0x08f304
    uint8_t clean_pad_text_to_timer_state_[2];            // +0x08f30e
    uint32_t human_certify_timer_key_;                    // +0x08f310
    uint32_t human_certify_error_count_;                  // +0x08f314
    // CUser's constructor never invokes MSG_* constructors in this span: the
    // two request images are byte-for-byte deferred mail data.  Keep them as
    // named raw storage so introducing the semantic wire structs elsewhere
    // cannot add synthetic child construction to CUser's ABI.
    uint8_t human_certified_;                             // +0x08f318
    uint8_t pending_mail_message_[0x145];                 // +0x08f319
    uint8_t pending_multi_mail_message_[0x1a6];           // +0x08f45e
    uint8_t pending_mail_is_multi_;                       // +0x08f604
    uint8_t mail_send_certified_;                         // +0x08f605
    uint8_t mail_to_mcap_state_[0x02];                    // +0x08f606
    WongWork::CMCAPManager mcap_manager_;                 // +0x08f608
    uint8_t received_event_;                              // +0x08f620
    uint8_t received_event_padding_[3];                   // +0x08f621
    int32_t auto_client_rate_;                            // +0x08f624
    // DisPatcher_Login::dispatch_sig @0x81E8C78 writes the direct session
    // tail after it has queued SIG_LOGIN_DATA.  C2/D2 do not introduce a
    // separate Hades child in this span: these are four login-session
    // scalars plus the actual 20-byte peer address captured from network_.
    uint8_t login_session_active_;                        // +0x08f628
    uint8_t login_session_active_padding_[3];             // +0x08f629
    uint32_t login_session_connected_at_;                 // +0x08f62c
    char login_session_peer_ip_[20];                      // +0x08f630
    uint32_t last_sell_item_tick_;                        // +0x08f644
    uint32_t sell_item_time_count_;                       // +0x08f648
    CUserItemTimeCheckState item_time_check_;             // +0x08f64c
    st_monstorkill monster_kill_position_;                // +0x08f660
    uint8_t monster_kill_to_item_serial_state_[0x14];     // +0x08f674
    CUserItemSerialCheckState item_serial_check_;         // +0x08f688
    // Inter_LoginInfo::dispatch_sig @0x84C5294 stores the Black-IP monitor
    // result at +0x08f690, then marks the private-security branch at +0x08f694
    // when its timed trade restriction is active.  These are CUser session
    // fields adjacent to, but not owned by, the CHades child at +0x08fb6c.
    uint32_t black_ip_punish_type_;                       // +0x08f690
    uint8_t black_ip_private_security_active_;            // +0x08f694
    uint8_t black_ip_to_hades_c2_state_[3];               // +0x08f695
    uint32_t special_monster_kill_counter_;               // +0x08f698
    uint8_t hades_to_resting_user_state_[0x4b0];          // +0x08f69c
    uint8_t resting_user_;                                // +0x08fb4c
    uint8_t resting_user_padding_;                        // +0x08fb4d
    uint16_t punish_trade_alert_;                         // +0x08fb4e
    uint32_t last_lottery_time_;                          // +0x08fb50
    uint8_t dungeon_clear_hack_check_;                    // +0x08fb54
    uint8_t dungeon_clear_hack_padding_;                  // +0x08fb55
    uint16_t clean_pad_verification_first_;               // +0x08fb56
    uint8_t clean_pad_verification_operation_;            // +0x08fb58
    uint8_t clean_pad_verification_padding_;              // +0x08fb59
    uint16_t clean_pad_verification_second_;              // +0x08fb5a
    uint8_t loading_hack_gold_;                           // +0x08fb5c
    uint8_t clean_pad_verification_to_action_state_[3];   // +0x08fb5d
    cSecuUserAction* secu_user_action_;                   // +0x08fb60
    uint32_t clean_pad_request_tick_;                     // +0x08fb64
    uint8_t hack_ip_counter_saved_;                       // +0x08fb68
    uint8_t ars_user_kick_;                               // +0x08fb69
    uint8_t hack_state_to_hades_[2];                      // +0x08fb6a
    XNuclear::CHades hades_;                              // +0x08fb6c
    Secu_GoldControl gold_control_;                       // +0x08fb98
    Secu_HackLogCheck hack_log_check_;                    // +0x08fc80
    std::map<short, int> etc_values_;                     // +0x08fcbc
    std::map<std::pair<char, char>, int> pair_values_;    // +0x08fcd4
    uint8_t pair_values_to_buying_gold_state_[0x04];     // +0x08fcec
    uint8_t buying_gold_;                                 // +0x08fcf0
    uint8_t buying_gold_to_cera_point_padding_[3];        // +0x08fcf1
    uint32_t cera_point_;                                 // +0x08fcf4
    uint8_t used_coin_in_dungeon_;                        // +0x08fcf8
    uint8_t used_coin_to_pi_receiver_state_[0x03];       // +0x08fcf9
    PIReceiverManager pi_receiver_;                       // +0x08fcfc
    PISenderManager pi_sender_;                           // +0x08fd04
    uint8_t non_client_flag_;                             // +0x08fd0c
    uint8_t non_client_flag_padding_[3];                  // +0x08fd0d
    int32_t non_client_random_value_;                     // +0x08fd10
    uint8_t pi_sender_random_key_;                        // +0x08fd14
    uint8_t pi_sender_to_hash_set_state_[3];              // +0x08fd15
    CUserCSHashSet cs_hashes_;                            // +0x08fd18
    uint8_t cs_hash_random_key_;                          // +0x08fd30
    uint8_t cs_hash_to_c2_state_[3];                      // +0x08fd31
    uint32_t used_coin_count_;                            // +0x08fd34
    uint8_t delete_event_dungeon_invitation_;             // +0x08fd38
    uint8_t delete_event_invitation_padding_[3];          // +0x08fd39
    // CMD024 records at most seven distinct item ids sold during a dungeon.
    // The count and ids are direct CUser storage used by set_state(5)'s
    // ClearItemSellCount path; naming them removes the last raw offset bridge
    // from that security-sensitive limiter.
    uint32_t in_dungeon_item_sell_count_;                  // +0x08fd3c
    uint32_t in_dungeon_item_sell_ids_[7];                 // +0x08fd40
    uint8_t check_pick_up_random_option_item_;            // +0x08fd5c
    uint8_t cs_hash_to_c2_state_30_[3];                   // +0x08fd5d
    // CParty::checkMoveMapHackCnt @0x84C8F92 increments this counter for
    // invalid map transitions, reports hack type 220 after the tenth hit and
    // then resets it.  It is gameplay anti-cheat state, not CS-hash storage.
    uint32_t move_map_hack_count_;                        // +0x08fd60
    uint8_t fatigue_count_db_;                            // +0x08fd64
    uint8_t fatigue_day_db_;                              // +0x08fd65
    uint8_t fatigue_db_to_map_state_[2];                  // +0x08fd66
    std::map<unsigned char, short> fatigue_day_data_;     // +0x08fd68
    std::map<int, std::pair<int, int> > pair_int_data_;   // +0x08fd80
    std::map<int, bool> flag_map_;                        // +0x08fd98
    uint8_t use_crane_start_;                             // +0x08fdb0
    uint8_t proper_level_dungeon_user_;                   // +0x08fdb1
    uint8_t proper_level_to_pc_room_state_[0x02];        // +0x08fdb2
    uint32_t pc_room_play_start_time_;                    // +0x08fdb4
    uint32_t pc_room_play_time_;                          // +0x08fdb8
    uint8_t pc_room_reward_count_;                        // +0x08fdbc
    uint8_t pc_room_reward_padding_[3];                   // +0x08fdbd
    uint32_t used_fatigue_;                               // +0x08fdc0
    uint32_t used_gift_fatigue_;                          // +0x08fdc4
    uint8_t gift_fatigue_changed_;                        // +0x08fdc8
    CUserInformNoticeState inform_notice_;                // +0x08fdc9
    std::vector<unsigned char> bingo_values_;             // +0x08fddc
    int32_t manage_level_;                                // +0x08fde8
    int32_t manage_point_;                                // +0x08fdec
    int32_t user_max_level_;                              // +0x08fdf0
    BingoData bingo_data_;                                // +0x08fdf4
    GrowthWeaponEvent growth_weapon_event_;               // +0x08fdfc
    GrowthCreatureEvent growth_creature_event_;           // +0x08fe10
    CharacLevelUpGiftAccountOnce levelup_gifts_;          // +0x08fe1c
    uint32_t cera_item_order_no_;                         // +0x08fe34
    uint8_t cera_item_order_to_c2_state_[4];              // +0x08fe38
    int32_t charac_antibot_serial_number_;                // +0x08fe3c
    int8_t received_request_type_;                        // +0x08fe40
    uint8_t combo_skill_set_;                             // +0x08fe41
    uint8_t final_lifecycle_to_c2_state_10_[2];           // +0x08fe42
    int32_t recipe_force_probability_;                    // +0x08fe44
    // The old C2/response users address these four bytes independently:
    // EventCreateDnfReward at +0x08fe48, Taiwan mobile-auth reward state at
    // +0x08fe49, then the two reset-only lifecycle bytes.  Splitting the
    // record preserves every physical offset without adding a compatibility
    // accessor or a guessed layout.
    uint8_t event_create_dnf_reward_;                      // +0x08fe48
    uint8_t mobile_auth_reward_state_;                     // +0x08fe49
    int8_t pcroom_daily_reward_remain_;                    // +0x08fe4a
    uint8_t final_lifecycle_state_03_;                     // +0x08fe4b
};

struct CUserPhysicalLayoutAssert {
#if defined(__i386__)
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    static_assert(offsetof(CUser, network_) == 0x0000e0u,
                  "CUser CNetwork must begin at old +0xe0");
    static_assert(offsetof(CUser, network_lifecycle_state_) == 0x06ef68u,
                  "CUser network lifecycle scalar offset drift");
    static_assert(offsetof(CUser, save_rental_info_to_exchange_) ==
                      0x06ef90u,
                  "CUser C2 rental initialization flag offset drift");
    static_assert(offsetof(CUser, garena_auth_data_) == 0x06ef92u,
                  "CUser Garena authentication record offset drift");
    static_assert(offsetof(CUser, allow_location_geo_state_) == 0x06effau,
                  "CUser geolocation-state flag offset drift");
    static_assert(offsetof(CUser, mercenary_info_) == 0x06ef6cu,
                  "CUser UserMercenaryInfoMgr offset drift");
    static_assert(offsetof(CUser, account_cargo_) == 0x06effcu,
                  "CUser CAccountCargo offset drift");
    static_assert(offsetof(CUser, game_option_) == 0x06fd64u,
                  "CUser CGameOption offset drift");
    static_assert(offsetof(CUser, gm_quest_flag_) == 0x070c1cu,
                  "CUser GM quest flag offset drift");
    static_assert(offsetof(CUser, force_drop_flag_) == 0x070c1du,
                  "CUser forced-drop flag offset drift");
    static_assert(offsetof(CUser, play_experience_) == 0x070c30u,
                  "CUser accumulated play experience offset drift");
    static_assert(offsetof(CUser, break_away_dungeon_clear_) == 0x070c34u,
                  "CUser break-away clear flag offset drift");
    static_assert(offsetof(CUser, break_away_dungeon_clear_count_) ==
                      0x070c38u,
                  "CUser break-away clear count offset drift");
    static_assert(offsetof(CUser, break_away_reward_order_) == 0x070c44u,
                  "CUser break-away reward order offset drift");
    static_assert(offsetof(CUser, used_break_away_luck_point_) ==
                      0x070c4du,
                  "CUser break-away luck-point flag offset drift");
    static_assert(offsetof(CUser, slot_index_) == 0x070d30u,
                  "CUser account slot index offset drift");
    static_assert(offsetof(CUser, increment_id_) == 0x070d32u,
                  "CUser increment id offset drift");
    static_assert(offsetof(CUser, unique_id_) == 0x070d34u,
                  "CUser C2 unique id offset drift");
    static_assert(offsetof(CUser, account_id_) == 0x070d38u,
                  "CUser account id offset drift");
    static_assert(offsetof(CUser, account_name_) == 0x070d3cu,
                  "CUser account name offset drift");
    static_assert(offsetof(CUser, link_charac_disconnect_flag_) == 0x070c65u,
                  "CUser linked-character disconnect flag offset drift");
    static_assert(offsetof(CUser, disconnect_link_charac_slot_index_) ==
                      0x070c66u,
                  "CUser linked-character slot index offset drift");
    static_assert(offsetof(CUser, ssn_) == 0x070dd2u,
                  "CUser SSN string offset drift");
    static_assert(offsetof(CUser, client_spec_c2_state_) == 0x070e64u,
                  "CUser C2 client-spec state offset drift");
    static_assert(offsetof(CUser, client_spec_) == 0x070e68u,
                  "CUser client hardware specification offset drift");
    static_assert(offsetof(CUser, game_master_mode_) == 0x071a64u,
                  "CUser game-master mode offset drift");
    static_assert(offsetof(CUser, age_) == 0x071a6cu,
                  "CUser account age offset drift");
    static_assert(offsetof(CUser, program_logout_) == 0x071a70u,
                  "CUser program-logout flag offset drift");
    static_assert(offsetof(CUser, packet_verifier_) == 0x071a80u,
                  "CUser CPacketVerifier offset drift");
    static_assert(offsetof(CUser, hack_analyzer_) == 0x07243cu,
                  "CUser CHackAnalyzer offset drift");
    static_assert(offsetof(CUser, mailbox_) ==
                      0x072428u,
                  "CUser accept mailbox offset drift");
    static_assert(offsetof(CUser, account_last_play_time_) == 0x072430u,
                  "CUser account last-play time offset drift");
    static_assert(offsetof(CUser, login_time_) == 0x072434u,
                  "CUser login time offset drift");
    static_assert(offsetof(CUser, code_hack_storage_) == 0x079e8cu,
                  "CUser CodeHackCheckStorage offset drift");
    static_assert(offsetof(CUser, std_drop_rate_) == 0x079eb4u,
                  "CUser standard drop-rate offset drift");
    static_assert(offsetof(CUser, trade_punish_type_) == 0x079eb8u,
                  "CUser trade-punish type offset drift");
    static_assert(offsetof(CUser, trade_punish_over_thirty_days_) ==
                      0x079ebcu,
                  "CUser long-term trade-punish flag offset drift");
    static_assert(offsetof(CUser, quest_reward_request_tick_) == 0x079ed4u,
                  "CUser quest reward request tick offset drift");
    static_assert(offsetof(CUser, quest_reward_request_count_) ==
                      0x079ed8u,
                  "CUser quest reward request count offset drift");
    static_assert(offsetof(CUser, server_group_) == 0x079ec0u,
                  "CUser server group offset drift");
    static_assert(offsetof(CUser, online_preliminary_charac_no_) ==
                      0x079ec8u,
                  "CUser online-preliminary character number offset drift");
    static_assert(offsetof(CUser, pc_room_no_) == 0x079ee0u,
                  "CUser PC room number offset drift");
    static_assert(offsetof(CUser, user_strings_) == 0x079ef4u,
                  "CUser duplicate-message strings offset drift");
    static_assert(offsetof(CUser, duplicate_message_index_) == 0x079efcu,
                  "CUser duplicate-message selector offset drift");
    static_assert(offsetof(CUser, before_area_time_) == 0x079f00u,
                  "CUser C2 before-area-time offset drift");
    static_assert(offsetof(CUser, mileage_) == 0x079f64u,
                  "CUser mileage offset drift");
    static_assert(offsetof(CUser, pu_user_) == 0x079f68u,
                  "CUser C2 pu-user offset drift");
    static_assert(offsetof(CUser, characters_) == 0x079f78u,
                  "CUser character vector offset drift");
    static_assert(offsetof(CUser, character_view_) == 0x079f84u,
                  "CUser character-view offset drift");
    static_assert(offsetof(CUser, history_stream_) == 0x079f88u,
                  "CUser history stream offset drift");
    static_assert(offsetof(CUser, in_dungeon_item_sell_count_) == 0x08fd3cu,
                  "CUser dungeon sell counter offset drift");
    static_assert(offsetof(CUser, in_dungeon_item_sell_ids_) == 0x08fd40u,
                  "CUser dungeon sell item ids offset drift");
    static_assert(offsetof(CUser, pending_pvp_win_point_) == 0x079f8cu,
                  "CUser pending PvP win-point offset drift");
    static_assert(offsetof(CUser, history_log_) == 0x079f90u,
                  "CUser history-log subobject offset drift");
    static_assert(offsetof(CUser, inventory_memory_) == 0x07a024u,
                  "CUser InventoryMemory offset drift");
    static_assert(offsetof(CUser, quest_) == 0x086aecu,
                  "CUser UserQuest offset drift");
    static_assert(offsetof(CUser, village_attacked_reward_) == 0x08e108u,
                  "CUser village-attacked reward offset drift");
    static_assert(offsetof(CUser, black_list_) == 0x08e10cu,
                  "CUser black-list vector offset drift");
    static_assert(offsetof(CUser, antibot_packet_state_) == 0x08e118u,
                  "CUser anti-bot packet-state offset drift");
    static_assert(offsetof(CUser, playing_dungeon_index_) == 0x08e11cu,
                  "CUser playing-dungeon index offset drift");
    static_assert(offsetof(CUser, last_packet_identity_) == 0x08e124u,
                  "CUser last-packet identity offset drift");
    static_assert(offsetof(CUser, antibot_key_) == 0x08e128u,
                  "CUser anti-bot key offset drift");
    static_assert(offsetof(CUser, antibot_user_info_) == 0x08e130u,
                  "CUser anti-bot user-info offset drift");
    static_assert(offsetof(CUser, first_mutex_) == 0x08e1a0u,
                  "CUser first Mutex offset drift");
    static_assert(offsetof(CUser, state_) == 0x08e1dcu,
                  "CUser C2 state offset drift");
    static_assert(offsetof(CUser, connection_check_count_) == 0x08e1e0u,
                  "CUser logout word offset drift");
    static_assert(offsetof(CUser, logout_state_byte_) == 0x08e1f4u,
                  "CUser logout byte offset drift");
    static_assert(offsetof(CUser, before_area_) == 0x08e1d0u,
                  "CUser previous area offset drift");
    static_assert(offsetof(CUser, area_) == 0x08e1d4u,
                  "CUser area offset drift");
    static_assert(offsetof(CUser, current_area_) == 0x08e1d8u,
                  "CUser current area offset drift");
    static_assert(offsetof(CUser, move_space_) == 0x08e1e4u,
                  "CUser move-space offset drift");
    static_assert(offsetof(CUser, connection_check_interval_) == 0x08e1e8u,
                  "CUser connection-check interval offset drift");
    static_assert(offsetof(CUser, resume_checksum_time_) == 0x08e1ecu,
                  "CUser checksum-resume time offset drift");
    static_assert(offsetof(CUser, checksum_answer_count_) == 0x08e1f0u,
                  "CUser checksum-answer count offset drift");
    static_assert(offsetof(CUser, position_x_) == 0x08e214u,
                  "CUser position X offset drift");
    static_assert(offsetof(CUser, position_y_) == 0x08e216u,
                  "CUser position Y offset drift");
    static_assert(offsetof(CUser, direction_) == 0x08e218u,
                  "CUser direction offset drift");
    static_assert(offsetof(CUser, trade_space_index_) == 0x08e21au,
                  "CUser trade-space index offset drift");
    static_assert(offsetof(CUser, party_index_) == 0x08e21cu,
                  "CUser party index offset drift");
    static_assert(offsetof(CUser, guild_pvp_point_) == 0x08e1f8u,
                  "CUser guild PvP point offset drift");
    static_assert(offsetof(CUser, advance_altar_index_) == 0x08e22au,
                  "CUser C2 PvP word offset drift");
    static_assert(offsetof(CUser, registration_time_) == 0x08e230u,
                  "CUser registration time offset drift");
    static_assert(offsetof(CUser, cera_) == 0x08e234u,
                  "CUser Cera balance offset drift");
    static_assert(offsetof(CUser, guild_info_) == 0x08e238u,
                  "CUser guild record offset drift");
    static_assert(offsetof(CUser, charac_info_load_lock_) == 0x08e308u,
                  "CUser character-load lock offset drift");
    static_assert(offsetof(CUser, charac_info_delete_lock_) == 0x08e309u,
                  "CUser character-delete lock offset drift");
    static_assert(offsetof(CUser, charac_info_create_lock_) == 0x08e30au,
                  "CUser character-create lock offset drift");
    static_assert(offsetof(CUser, last_login_charac_no_) == 0x08e30cu,
                  "CUser last-login character number offset drift");
    static_assert(offsetof(CUser, last_login_channel_no_) == 0x08e310u,
                  "CUser last-login channel offset drift");
    static_assert(offsetof(CUser, last_login_charac_valid_) == 0x08e314u,
                  "CUser last-login validity offset drift");
    static_assert(offsetof(CUser, cache_character_load_state_) == 0x08e315u,
                  "CUser cached-character completion-state offset drift");
    static_assert(offsetof(CUser, trade_count_) == 0x08e324u,
                  "CUser trade-count offset drift");
    static_assert(offsetof(CUser, ting_flag_) == 0x08e2f7u,
                  "CUser Ting flag offset drift");
    static_assert(offsetof(CUser, hack_user_timer_) == 0x08e3c0u,
                  "CUser hack-user timer flag offset drift");
    static_assert(offsetof(CUser, hack_user_flag_) == 0x08e3c1u,
                  "CUser hack-user flag offset drift");
    static_assert(offsetof(CUser, total_hack_user_trade_gold_) == 0x08e3c4u,
                  "CUser hack-user trade-gold offset drift");
    static_assert(offsetof(CUser, total_hack_user_trade_count_) == 0x08e3ccu,
                  "CUser hack-user trade-count offset drift");
    static_assert(offsetof(CUser, revival_time_) == 0x08e3f4u,
                  "CUser revival time offset drift");
    static_assert(offsetof(CUser, use_apc_potion_time_) == 0x08e3f8u,
                  "CUser APC potion time offset drift");
    static_assert(offsetof(CUser, exp_reward_) == 0x08e465u,
                  "CUser growth-power experience reward offset drift");
    static_assert(offsetof(CUser, slot_effect_count_) == 0x08e45cu,
                  "CUser slot-effect count offset drift");
    static_assert(offsetof(CUser, charac_slot_limit_) == 0x08e45du,
                  "CUser character-slot limit offset drift");
    static_assert(offsetof(CUser, present_avenger_title_) == 0x08e45eu,
                  "CUser avenger-title presentation flag offset drift");
    static_assert(offsetof(CUser, ontime_last_recv_idx_) == 0x08e460u,
                  "CUser on-time reward index offset drift");
    static_assert(offsetof(CUser, update_ontime_last_recv_idx_) == 0x08e464u,
                  "CUser on-time reward dirty flag offset drift");
    static_assert(offsetof(CUser, antibot_send_buffer_count_) == 0x08e469u,
                  "CUser anti-bot send counter offset drift");
    static_assert(offsetof(CUser, antibot_dproto_callback_count_) ==
                      0x08e46au,
                  "CUser DPROTO callback counter offset drift");
    static_assert(offsetof(CUser, action_point_manager_) == 0x08e47cu,
                  "CUser CActionPointManager offset drift");
    static_assert(offsetof(CUser, power_up_) == 0x08e46bu,
                  "CUser D2 power-up clear offset drift");
    static_assert(offsetof(CUser, trade_peer_unique_id_) == 0x08e474u,
                  "CUser trade peer unique-id offset drift");
    static_assert(offsetof(CUser, hangame_user_) == 0x08e478u,
                  "CUser Hangame-user flag offset drift");
    static_assert(offsetof(CUser, advance_altar_manager_) == 0x08f178u,
                  "CUser advance-altar manager offset drift");
    static_assert(offsetof(CUser, character_statistic_) == 0x08f28cu,
                  "CUser character-statistic POD offset drift");
    static_assert(offsetof(CUser, childrens_day_event_) == 0x08f2b0u,
                  "CUser ChildrensDay POD offset drift");
    static_assert(offsetof(CUser, mcap_manager_) == 0x08f608u,
                  "CUser CMCAPManager offset drift");
    static_assert(offsetof(CUser, guild_cargo_capacity_) == 0x08f2acu,
                  "CUser guild cargo capacity offset drift");
    static_assert(offsetof(CUser, tutorial_skipable_) == 0x08f2a2u,
                  "CUser tutorial-skip flag offset drift");
    static_assert(offsetof(CUser, creature_exp_fatigue_) == 0x08f2a4u,
                  "CUser creature EXP fatigue offset drift");
    static_assert(offsetof(CUser, debug_command_) == 0x08f2a8u,
                  "CUser debug-command scalar offset drift");
    static_assert(offsetof(CUser, clean_pad_captcha_text_) == 0x08f304u,
                  "CUser clean-pad captcha text offset drift");
    static_assert(offsetof(CUser, human_certify_timer_key_) == 0x08f310u,
                  "CUser human-certify timer key offset drift");
    static_assert(offsetof(CUser, human_certify_error_count_) == 0x08f314u,
                  "CUser human-certification error count offset drift");
    static_assert(offsetof(CUser, human_certified_) == 0x08f318u,
                  "CUser human-certification flag offset drift");
    static_assert(offsetof(CUser, pending_mail_message_) == 0x08f319u,
                  "CUser pending single-mail image offset drift");
    static_assert(offsetof(CUser, pending_multi_mail_message_) == 0x08f45eu,
                  "CUser pending multi-mail image offset drift");
    static_assert(offsetof(CUser, pending_mail_is_multi_) == 0x08f604u,
                  "CUser pending-mail selector offset drift");
    static_assert(offsetof(CUser, mail_send_certified_) == 0x08f605u,
                  "CUser mail-certification flag offset drift");
    static_assert(offsetof(CUser, item_time_check_) == 0x08f64cu,
                  "CUser item-time checker offset drift");
    static_assert(offsetof(CUser, item_serial_check_) == 0x08f688u,
                  "CUser item-serial checker offset drift");
    static_assert(offsetof(CUser, monster_kill_position_) == 0x08f660u,
                  "CUser monster-kill position window offset drift");
    static_assert(offsetof(CUser, special_monster_kill_counter_) ==
                      0x08f698u,
                  "CUser special monster-kill counter offset drift");
    static_assert(offsetof(CUser, dungeon_clear_hack_check_) == 0x08fb54u,
                  "CUser dungeon-clear hack state offset drift");
    static_assert(offsetof(CUser, last_lottery_time_) == 0x08fb50u,
                  "CUser lottery timestamp offset drift");
    static_assert(offsetof(CUser, clean_pad_verification_first_) == 0x08fb56u,
                  "CUser clean-pad first verification word offset drift");
    static_assert(offsetof(CUser, clean_pad_verification_operation_) ==
                      0x08fb58u,
                  "CUser clean-pad verification operation offset drift");
    static_assert(offsetof(CUser, clean_pad_verification_second_) ==
                      0x08fb5au,
                  "CUser clean-pad second verification word offset drift");
    static_assert(offsetof(CUser, loading_hack_gold_) == 0x08fb5cu,
                  "CUser loading-hack-gold flag offset drift");
    static_assert(offsetof(CUser, clean_pad_request_tick_) == 0x08fb64u,
                  "CUser clean-pad request tick offset drift");
    static_assert(offsetof(CUser, hack_ip_counter_saved_) == 0x08fb68u,
                  "CUser hack IP-counter flag offset drift");
    static_assert(offsetof(CUser, ars_user_kick_) == 0x08fb69u,
                  "CUser ARS kick flag offset drift");
    static_assert(offsetof(CUser, hades_) == 0x08fb6cu,
                  "CUser CHades offset drift");
    static_assert(offsetof(CUser, gold_control_) == 0x08fb98u,
                  "CUser Secu_GoldControl offset drift");
    static_assert(offsetof(CUser, hack_log_check_) == 0x08fc80u,
                  "CUser Secu_HackLogCheck offset drift");
    static_assert(offsetof(CUser, pi_receiver_) == 0x08fcfcu,
                  "CUser PIReceiverManager offset drift");
    static_assert(offsetof(CUser, used_coin_in_dungeon_) == 0x08fcf8u,
                  "CUser used-coin-in-dungeon flag offset drift");
    static_assert(offsetof(CUser, pi_sender_) == 0x08fd04u,
                  "CUser PISenderManager offset drift");
    static_assert(offsetof(CUser, pi_sender_random_key_) == 0x08fd14u,
                  "CUser C2 PI sender key offset drift");
    static_assert(offsetof(CUser, cs_hash_random_key_) == 0x08fd30u,
                  "CUser C2 CS hash key offset drift");
    static_assert(offsetof(CUser, move_map_hack_count_) == 0x08fd60u,
                  "CUser move-map hack counter offset drift");
    static_assert(offsetof(CUser, check_pick_up_random_option_item_) ==
                      0x08fd5cu,
                  "CUser random-option pickup flag offset drift");
    static_assert(offsetof(CUser, fatigue_count_db_) == 0x08fd64u &&
                      offsetof(CUser, fatigue_day_db_) == 0x08fd65u,
                  "CUser persisted fatigue-day tuple offset drift");
    static_assert(offsetof(CUser, pc_room_play_start_time_) == 0x08fdb4u,
                  "CUser PC-room play start offset drift");
    static_assert(offsetof(CUser, pc_room_play_time_) == 0x08fdb8u,
                  "CUser PC-room play duration offset drift");
    static_assert(offsetof(CUser, pc_room_reward_count_) == 0x08fdbcu,
                  "CUser PC-room reward count offset drift");
    static_assert(offsetof(CUser, used_fatigue_) == 0x08fdc0u,
                  "CUser used-fatigue counter offset drift");
    static_assert(offsetof(CUser, used_gift_fatigue_) == 0x08fdc4u,
                  "CUser used gift-fatigue counter offset drift");
    static_assert(offsetof(CUser, gift_fatigue_changed_) == 0x08fdc8u,
                  "CUser gift-fatigue dirty flag offset drift");
    static_assert(offsetof(CUser, inform_notice_) == 0x08fdc9u,
                  "CUser inform-notice state offset drift");
    static_assert(offsetof(CUser, manage_level_) == 0x08fde8u,
                  "CUser management level offset drift");
    static_assert(offsetof(CUser, manage_point_) == 0x08fdecu,
                  "CUser management point offset drift");
    static_assert(offsetof(CUser, user_max_level_) == 0x08fdf0u,
                  "CUser maximum character level offset drift");
    static_assert(offsetof(CUser, levelup_gifts_) == 0x08fe1cu,
                  "CUser CharacLevelUpGiftAccountOnce offset drift");
    static_assert(offsetof(CUser, cera_item_order_no_) == 0x08fe34u,
                  "CUser Cera item order number offset drift");
    static_assert(offsetof(CUser, charac_antibot_serial_number_) ==
                      0x08fe3cu,
                  "CUser C2 final state-08 offset drift");
    static_assert(offsetof(CUser, combo_skill_set_) == 0x08fe41u,
                  "CUser combo-skill-set flag offset drift");
    static_assert(offsetof(CUser, recipe_force_probability_) == 0x08fe44u,
                  "CUser forced recipe probability offset drift");
    static_assert(offsetof(CUser, event_create_dnf_reward_) == 0x08fe48u,
                  "CUser EventCreateDnfReward byte offset drift");
    static_assert(offsetof(CUser, mobile_auth_reward_state_) == 0x08fe49u,
                  "CUser Taiwan mobile-auth reward byte offset drift");
    static_assert(offsetof(CUser, pcroom_daily_reward_remain_) == 0x08fe4au,
                  "CUser PC-room daily-reward remainder offset drift");
    static_assert(sizeof(CUser) == 0x08fe4cu,
                  "CUser must preserve its target 0x08fe4c-byte i386 extent");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif
};

// Layout-only witness for StaticPool<CUser,600>::CNode.  CNode C2
// @0x82B8D42 constructs the CUser base, writes increment_id=36641 at
// +0x8fe4c, then clears the byte refcount at +0x8fe50.  This is intentionally
// not an instantiated StaticPool provider and carries no allocator behavior.
struct CUserStaticPool600CNodeLayout {
    CUser user_;                 // +0x00000
    uint32_t increment_id_;      // +0x8fe4c
    uint8_t ref_count_;          // +0x8fe50
    uint8_t abi_padding_[3];     // +0x8fe51
};

#if defined(__i386__)
static_assert(offsetof(CUserStaticPool600CNodeLayout, increment_id_) ==
                  0x08fe4cu,
              "StaticPool<CUser,600>::CNode increment id offset drift");
static_assert(offsetof(CUserStaticPool600CNodeLayout, ref_count_) ==
                  0x08fe50u,
              "StaticPool<CUser,600>::CNode refcount offset drift");
static_assert(sizeof(CUserStaticPool600CNodeLayout) == 0x08fe54u,
              "StaticPool<CUser,600>::CNode stride must stay 0x8fe54");
#endif

static_assert(sizeof(CUser::CharacMoneyLog) == 0x0cu,
              "CUser CharacMoneyLog IDA ABI size must stay 0x0c");

static_assert(sizeof(CUser::DBLogItemInfo) == 0x0Cu,
              "CUser DB-log item ABI size must stay 0x0c");

// IDA unit: _Charac_info +0xECC, stBloodBestRecord.
// This is the exact character-vector element used by CUser character selection,
// creation, loading and deletion.  Its non-trivial lifetime is restored in the
// C-owned character-info runtime; callers must not model it as a byte-only
// substitute.
struct stBloodBestRecord {
    int round_record;
    int time_record;
};

static_assert(sizeof(stBloodBestRecord) == 0x08u,
              "stBloodBestRecord IDA ABI size must stay 0x08");

typedef std::map<int, stBloodBestRecord> CUserBloodBestRecordMap;

typedef char CUserBloodBestRecordMap_Size_Must_Be_0x18[
    (sizeof(CUserBloodBestRecordMap) == 0x18) ? 1 : -1];

struct _Charac_info {
    // IDA_EA: 0x86822B8
    // IDA_NAME: _ZN12_Charac_info5ResetEv
    // DEMANGLED: _Charac_info::Reset(void)
    _Charac_info* Reset();

    // IDA_EA: 0x8695CF8
    // IDA_NAME: _ZN12_Charac_infoC2Ev
    // DEMANGLED: _Charac_info::_Charac_info(void)
    _Charac_info();

    // IDA_EA: 0x8697982
    // IDA_NAME: _ZN12_Charac_infoD2Ev
    // DEMANGLED: _Charac_info::~_Charac_info()
    ~_Charac_info();

    // IDA_EA: 0x8699820
    // IDA_NAME: _ZN12_Charac_infoC2ERKS_
    // DEMANGLED: _Charac_info::_Charac_info(_Charac_info const&)
    _Charac_info(const _Charac_info& other);

    // IDA_EA: 0x869DA62
    // IDA_NAME: _ZN12_Charac_infoaSERKS_
    // DEMANGLED: _Charac_info::operator=(_Charac_info const&)
    _Charac_info& operator=(const _Charac_info& other);

    // The selection payload serializers below use this exact packed projection
    // of the physical record.  It deliberately names every field read by
    // getCharacInfoDetail/getTagCharacInfo; the untouched persistence regions
    // remain outside those providers until their owning gameplay subsystems are
    // recovered.  This prevents packet code from recreating byte offsets.
    // The persisted character record itself is byte-packed.  In particular,
    // its real CInventory child begins at +0x0da and is therefore not
    // naturally four-byte aligned.  `#pragma pack(1)` is required here:
    // GCC 4.9 may ignore an attribute-only packing request for non-POD child
    // objects, which would silently move the inventory to +0x0dc.
#pragma pack(push, 1)
    struct __attribute__((packed)) Fields {
        int32_t character_no_;                    // IDA +0x0000
        char character_name_[30];                 // IDA +0x0004
        uint8_t village_;                          // IDA +0x0022
        uint8_t pvp_village_;                      // IDA +0x0023
        uint8_t village_state_;                    // IDA +0x0024
        uint8_t previous_village_;                 // IDA +0x0025
        int8_t job_;                               // IDA +0x0026
        int16_t level_;                            // IDA +0x0027
        STGrowthType grow_type_;                   // IDA +0x0029
        STGrowthType changing_grow_type_;          // IDA +0x002a
        uint32_t experience_;                      // IDA +0x002b
        uint32_t party_bonus_experience_;          // IDA +0x002f
        uint16_t fatigue_;                         // IDA +0x0033
        uint16_t maximum_fatigue_;                 // IDA +0x0035
        uint16_t fatigue_grownup_buff_;            // IDA +0x0037
        CommonTime create_time_;                   // IDA +0x0039
        uint8_t select_state_0049_004c_[4];        // IDA +0x0049
        uint16_t used_fatigue_;                    // IDA +0x004d
        int16_t fatigue_battery_charging_;         // IDA +0x004f
        uint32_t tutorial_flag_;                   // IDA +0x0051
        uint8_t escalade_tutorial_flag_[16];       // IDA +0x0055
        CReliablePerson reliable_person_;          // IDA +0x0065
        uint32_t school_point_;                    // IDA +0x0071
        uint8_t stamina_;                          // IDA +0x0075
        uint8_t backup_stamina_;                   // IDA +0x0076
        uint16_t premium_fatigue_;                 // IDA +0x0077
        uint16_t maximum_premium_fatigue_;         // IDA +0x0079
        uint32_t last_play_tick_;                  // IDA +0x007b
        uint32_t power_war_last_play_time_;        // IDA +0x007f
        uint8_t forbid_user_to_play_;               // IDA +0x0083
        uint32_t forbid_remaining_seconds_;         // IDA +0x0084
        _Additioal_info additional_info_;           // IDA +0x0088

        CInventory inventory_;                     // IDA +0x00da
        SkillSlot skill_slot_;                     // target +0x8da
        CCargo cargo_;                             // target +0xdf2

        uint32_t guild_id_;                        // target +0xe06
        uint8_t guild_right_;                      // target +0xe0a
        uint16_t guild_create_flag_;               // target +0xe0b
        char guild_create_name_[0x17];             // target +0xe0d
        char guild_create_address_[0x0d];          // target +0xe24
        int32_t guild_experience_;                 // target +0xe31
        uint16_t guild_temporary_experience_;      // target +0xe35
        int32_t guild_today_experience_;           // target +0xe37
        uint8_t guild_member_grade_;               // target +0xe3b
        uint32_t help_abuse_ratio_;                // target +0xe3c
        uint32_t help_abuse_experience_;           // target +0xe40
        int16_t level_before_dungeon_;              // target +0xe44
        uint32_t help_abuse_computed_ratio_;        // target +0xe46
        uint32_t login_tick_;                       // target +0xe4a
        int32_t finish_point_;                     // target +0xe4e
        int32_t finish_point_total_;               // target +0xe52
        float finish_point_detail_[5];             // target +0xe56
        uint32_t member_key_;                      // target +0xe6a
        uint32_t upper_member_key_;                // target +0xe6e
        uint32_t member_bonus_experience_;         // target +0xe72
        uint32_t fatigue_buff_bonus_experience_;   // target +0xe76
        uint8_t upper_member_experience_level_;    // target +0xe7a
        uint32_t tax_money_to_upper_;              // target +0xe7b
        uint32_t tax_fatigue_to_upper_;            // target +0xe7f
        uint8_t need_check_over_equip_item_;       // target +0xe83
        uint32_t black_count_;                      // target +0xe84
        tagCharacVisibleFlag visible_flag_;        // target +0xe88
        uint32_t assault_place_;                   // target +0xe89
        uint32_t assault_count_;                   // target +0xe8d
        int32_t chaos_point_;                      // target +0xe91
        int32_t chaos_experience_;                 // target +0xe95
        int32_t chaos_mode_count_;                 // target +0xe99
        int32_t chaos_kill_count_;                 // target +0xe9d
        int32_t chaos_die_count_;                  // target +0xea1
        int32_t chaos_die_time_;                   // target +0xea5
        uint32_t assault_session_state_;           // target +0xea9
        uint8_t super_state_;                      // target +0xead
        uint8_t ghost_state_;                      // target +0xeae
        uint8_t leave_timeout_state_;              // target +0xeaf
        int32_t chaos_kill_time_;                  // target +0xeb0
        int32_t assault_fatigue_count_;            // target +0xeb4
        uint8_t open_coin_flag_;                   // target +0xeb8
        int32_t luck_point_;                       // target +0xeb9
        uint32_t luck_point_update_count_;         // target +0xebd
        int32_t dungeon_play_count_;               // target +0xec1
        int32_t expert_job_;                       // target +0xec5
        int32_t expert_job_experience_;            // target +0xec9
        expert_job::CExpertJob* expert_job_object_; // target +0xecd
        uint32_t online_preliminary_team_handle_;  // target +0xed1

        // The interval includes the existing typed vectors/maps and premium
        // Lethe records.  Inter_LoadDimensionInout @0x84D83D2 is the one
        // consumer in this slice that reads/writes the following exact DB
        // fields inside it, so name them instead of recreating byte offsets
        // in the inter provider.
        STPowerWarCharacData power_war_;            // target +0xed5
        uint8_t power_war_state_0e99_;              // target +0xee1
        std::vector<ContinuousItemInfo> cool_time_items_; // target +0xee2
        std::vector<ContinuousItemInfo> effect_items_;    // target +0xeee
        uint8_t affected_by_exp_double_;            // target +0xefa
        uint8_t one_day_lethe_flag_;                // target +0xefb
        uint8_t initialized_skill_flag_;            // target +0xefc
        uint8_t one_day_lethe_second_flag_;         // target +0xefd
        uint8_t initialized_skill_second_flag_;     // target +0xefe
        uint8_t first_time_login_;                  // target +0xeff
        uint8_t dimension_dirty_;                // target +0xf00
        uint8_t dimension_inout_[6];             // target +0xf01..+0xf06
        uint8_t ultimate_inout_dirty_;           // target +0xf07
        uint8_t ultimate_inout_[2];              // target +0xf08..+0xf09
        uint8_t blood_inout_count_;              // target +0xf0a
        uint8_t blood_best_record_update_;          // target +0xf0b
        int32_t blood_best_round_record_;           // target +0xf0c
        int32_t blood_best_time_record_;            // target +0xf10
        CUserBloodBestRecordMap blood_best_records_; // target +0xf14
        uint8_t persistent_state_0ee4_;              // target +0xf2c
        float exp_affect_rate_;                     // target +0xf2d
        uint8_t disguise_character_;                // target +0xf31
        uint8_t disguise_kind_;                     // target +0xf32
        uint16_t disguise_index_;                   // target +0xf33
        SIG_LOAD_PREMIUM_LETHE premium_lethe_first_;  // target +0xf35
        SIG_LOAD_PREMIUM_LETHE premium_lethe_second_; // target +0x10ce

        int16_t power_war_point_;                  // target +0x1267
        uint32_t power_war_assault_count_;         // target +0x1269
        uint32_t power_war_victory_count_;         // target +0x126d
        uint16_t straight_victories_;              // target +0x1271
        uint8_t end_lethe_at_login_;               // target +0x1273
        int16_t emotion_;                          // target +0x1274
        uint32_t item_limit_buy_time_;             // target +0x1276
        uint16_t condition_event_step_;            // target +0x127a
        uint16_t condition_event_reward_step_;     // target +0x127c
        uint16_t proper_dungeon_clear_count_;      // target +0x127e
        uint8_t proper_dungeon_user_;              // target +0x1280
        int8_t skill_tree_index_;                  // target +0x1281
        uint16_t event_item_count_;                // target +0x1282
        uint16_t event_add_item_count_;            // target +0x1284
        uint8_t exp_equip_slot_state_;              // target +0x1286
        uint8_t event_character_grow_type_;         // target +0x1287
        // target +0x1288: Inter_CreateCharac @0x84BF125 supplies the request
        // byte through CUserCharacInfo::setUserEventCharacterFlag; CreateCharac
        // @0x864A32C stores it here.  Character-view and packet2 role-list
        // serialization then consume this same byte.
        uint8_t user_event_character_flag_;         // target +0x1288
        uint8_t member_dungeon_flag_;              // target +0x1289
        uint8_t village_attack_count_;             // target +0x128a
        uint8_t revenge_dungeon_count_;            // target +0x128b
        uint8_t village_attack_update_flag_;       // target +0x128c
        uint8_t link_charac_slot_index_;           // target +0x128d
        uint8_t link_type_;                        // target +0x128e
        uint32_t link_bonus_experience_;           // target +0x128f
        uint32_t link_bonus_gold_;                 // target +0x1293
        uint8_t link_state_124f_;                  // target +0x1297
        uint32_t link_runtime_flags_;              // target +0x1298
        std::map<int, STSkillCommandData> skill_commands_; // target +0x129c
        uint16_t member_bonus_fatigue_;            // target +0x12b4
        uint8_t guild_secede_;                     // target +0x12b6
        uint32_t last_play_dungeon_index_;         // target +0x12b7
        uint8_t upgrade_separate_true_flag_;       // target +0x12bb
        uint8_t upgrade_separate_false_flag_;      // target +0x12bc
        uint8_t equipslot_flag_;                   // target +0x12bd
        uint8_t channel_equipslot_switch_;         // target +0x12be
        uint8_t expand_equipslot_switch_;          // target +0x12bf
        uint8_t event_bring_up_reward_flag_;       // target +0x12c0
        uint8_t event_bring_up_material_use_count_;// target +0x12c1
        uint8_t event_bring_up_material_total_;    // target +0x12c2
        uint8_t growth_power_exp_reward_right_;    // target +0x12c3
        uint32_t seria_buff_bonus_experience_;     // target +0x12c4
        uint8_t growth_power_reward_;              // target +0x12c8
        stAuraAvatarOption aura_avatar_option_;     // target +0x12c9
        int32_t premium_bonus_experience_;         // target +0x12d5
        int32_t weekend_bonus_experience_;         // target +0x12d9
        uint8_t bonus_point_prefix_[0x12a1u - 0x1299u];
        WpBonusPointResult wp_bonus_point_;        // target +0x12e5
        ExpBonusPointResult exp_bonus_point_;      // target +0x12ed
        int32_t chaos_response_time_late_;         // target +0x12f5
        int32_t assault_money_;                    // target +0x12f9
        uint16_t assaulted_user_id_;               // target +0x12fd
        CharacterOption character_option_;         // target +0x12ff
        uint8_t striker_skill_index_;              // target +0x14ff
        uint32_t tag_character_no_;                // target +0x1500
        uint8_t tag_charac_info_loaded_;           // target +0x1504
        uint8_t blue_marble_enter_count_;          // target +0x1505
        uint8_t blue_marble_dead_count_;           // target +0x1506
        uint8_t blue_marble_to_max_equip_state_[3];// target +0x1507
        int32_t maximum_equip_level_;               // target +0x150a
        int32_t character_view_state_;              // target +0x150e
        seriaRoom_AniDeco seria_room_ani_deco_;     // target +0x1512
        uint8_t contract_cube_premium_enabled_;     // target +0x1516
        uint8_t contract_cube_premium_type_;        // target +0x1517
        uint8_t contract_cube_premium_saved_;       // target +0x1518
    };

#pragma pack(pop)

    Fields fields_;
};

static_assert(offsetof(_Charac_info::Fields, character_no_) == 0x0000u,
              "_Charac_info character number must stay at +0x0000");
static_assert(offsetof(_Charac_info::Fields, character_name_) == 0x0004u,
              "_Charac_info character name must stay at +0x0004");
static_assert(offsetof(_Charac_info::Fields, job_) == 0x0026u,
              "_Charac_info job must stay at +0x0026");
static_assert(offsetof(_Charac_info::Fields, level_) == 0x0027u,
              "_Charac_info level must stay at +0x0027");
static_assert(offsetof(_Charac_info::Fields, grow_type_) == 0x0029u,
              "_Charac_info growth type must stay at +0x0029");
static_assert(offsetof(_Charac_info::Fields, create_time_) == 0x0039u,
              "_Charac_info create time must stay at +0x0039");
static_assert(offsetof(_Charac_info::Fields, forbid_user_to_play_) == 0x0083u,
              "_Charac_info forbid flag must stay at +0x0083");
static_assert(offsetof(_Charac_info::Fields, forbid_remaining_seconds_) ==
                  0x0084u,
              "_Charac_info forbid duration must stay at +0x0084");
static_assert(offsetof(_Charac_info::Fields, additional_info_) == 0x0088u,
              "_Charac_info additional info must stay at +0x0088");
static_assert(offsetof(_Charac_info::Fields, guild_id_) == 0xe06u,
              "_Charac_info guild id must stay at +0xe06");
static_assert(offsetof(_Charac_info::Fields, member_key_) == 0xe6au,
              "_Charac_info member key must stay at +0xe6a");
static_assert(offsetof(_Charac_info::Fields, chaos_point_) == 0xe91u,
              "_Charac_info chaos point must stay at +0xe91");
static_assert(offsetof(_Charac_info::Fields, power_war_point_) == 0x1267u,
              "_Charac_info power-war point must stay at +0x1267");
static_assert(offsetof(_Charac_info::Fields, dimension_dirty_) == 0xf00u,
              "_Charac_info dimension dirty flag must stay at +0xf00");
static_assert(offsetof(_Charac_info::Fields, dimension_inout_) == 0xf01u,
              "_Charac_info dimension values must stay at +0xf01");
static_assert(offsetof(_Charac_info::Fields, ultimate_inout_dirty_) ==
                  0xf07u,
              "_Charac_info ultimate dirty flag must stay at +0xf07");
static_assert(offsetof(_Charac_info::Fields, blood_inout_count_) == 0xf0au,
              "_Charac_info blood in/out value must stay at +0xf0a");
static_assert(offsetof(_Charac_info::Fields, skill_tree_index_) == 0x1281u,
              "_Charac_info skill-tree index must stay at +0x1281");
static_assert(offsetof(_Charac_info::Fields, user_event_character_flag_) ==
                  0x1288u,
              "_Charac_info user-event character flag must stay at +0x1288");
static_assert(offsetof(_Charac_info::Fields, member_bonus_fatigue_) == 0x12b4u,
              "_Charac_info member bonus fatigue must stay at +0x12b4");
static_assert(offsetof(_Charac_info::Fields, equipslot_flag_) ==
                  0x12bdu,
              "_Charac_info add-equipslot flag must stay at +0x12bd");
static_assert(offsetof(_Charac_info::Fields, channel_equipslot_switch_) ==
                  0x12beu,
              "_Charac_info channel equipslot switch must stay at +0x12be");
static_assert(offsetof(_Charac_info::Fields, expand_equipslot_switch_) ==
                  0x12bfu,
              "_Charac_info expand equipslot switch must stay at +0x12bf");
static_assert(offsetof(_Charac_info::Fields, event_bring_up_reward_flag_) ==
                  0x12c0u,
              "_Charac_info bring-up reward flag must stay at +0x12c0");
static_assert(offsetof(_Charac_info::Fields,
                       event_bring_up_material_use_count_) == 0x12c1u,
              "_Charac_info bring-up material count must stay at +0x12c1");
static_assert(offsetof(_Charac_info::Fields,
                       event_bring_up_material_total_) == 0x12c2u,
              "_Charac_info bring-up material total must stay at +0x12c2");
static_assert(offsetof(_Charac_info::Fields,
                       growth_power_exp_reward_right_) == 0x12c3u,
              "_Charac_info growth-power exp reward state must stay at +0x12c3");
static_assert(offsetof(_Charac_info::Fields,
                       seria_buff_bonus_experience_) == 0x12c4u,
              "_Charac_info Seria buff experience must stay at +0x12c4");
static_assert(offsetof(_Charac_info::Fields, growth_power_reward_) == 0x12c8u,
              "_Charac_info growth-power reward must stay at +0x12c8");
static_assert(offsetof(_Charac_info::Fields, premium_bonus_experience_) ==
                  0x12d5u,
              "_Charac_info premium bonus experience must stay at +0x12d5");
static_assert(offsetof(_Charac_info::Fields, weekend_bonus_experience_) ==
                  0x12d9u,
              "_Charac_info weekend bonus experience must stay at +0x12d9");
static_assert(offsetof(_Charac_info::Fields, wp_bonus_point_) == 0x12e5u,
              "_Charac_info WP bonus point result must stay at +0x12e5");
static_assert(offsetof(_Charac_info::Fields, exp_bonus_point_) == 0x12edu,
              "_Charac_info EXP bonus point result must stay at +0x12ed");
static_assert(offsetof(_Charac_info::Fields, chaos_response_time_late_) ==
                  0x12f5u,
              "_Charac_info late chaos response time must stay at +0x12f5");
static_assert(offsetof(_Charac_info::Fields, striker_skill_index_) == 0x14ffu,
              "_Charac_info striker skill index must stay at +0x14ff");
static_assert(offsetof(_Charac_info::Fields, tag_character_no_) == 0x1500u,
              "_Charac_info tag character number must stay at +0x1500");
static_assert(offsetof(_Charac_info::Fields, tag_charac_info_loaded_) ==
                  0x1504u,
              "_Charac_info tag character load state must stay at +0x1504");
static_assert(offsetof(_Charac_info::Fields, guild_create_flag_) == 0xe0bu,
              "_Charac_info guild-create flag must stay at +0xe0b");
static_assert(offsetof(_Charac_info::Fields, guild_experience_) == 0xe31u,
              "_Charac_info guild experience must stay at +0xe31");
static_assert(offsetof(_Charac_info::Fields, level_before_dungeon_) == 0xe44u,
              "_Charac_info pre-dungeon level must stay at +0xe44");
static_assert(offsetof(_Charac_info::Fields, login_tick_) == 0xe4au,
              "_Charac_info login tick must stay at +0xe4a");
static_assert(offsetof(_Charac_info::Fields, black_count_) == 0xe84u,
              "_Charac_info black count must stay at +0xe84");
static_assert(offsetof(_Charac_info::Fields, visible_flag_) ==
                  0xe88u,
              "_Charac_info tag visibility must stay at +0xe88");
static_assert(offsetof(_Charac_info::Fields, ghost_state_) == 0xeaeu,
              "_Charac_info ghost state must stay at +0xeae");
static_assert(offsetof(_Charac_info::Fields, chaos_die_time_) == 0xea5u,
              "_Charac_info chaos/rename expiry must stay at +0xea5");
static_assert(offsetof(_Charac_info::Fields, power_war_) == 0xed5u,
              "_Charac_info power-war HP must stay at +0xed5");
static_assert(offsetof(_Charac_info::Fields, affected_by_exp_double_) ==
                  0xefau,
              "_Charac_info EXP-double state must stay at +0xefa");
static_assert(offsetof(_Charac_info::Fields, ultimate_inout_) == 0xf08u,
              "_Charac_info ultimate values must stay at +0xf08");
static_assert(offsetof(_Charac_info::Fields, blood_best_record_update_) ==
                  0xf0bu,
              "_Charac_info blood-record dirty flag must stay at +0xf0b");
static_assert(offsetof(_Charac_info::Fields, exp_affect_rate_) == 0xf2du,
              "_Charac_info EXP-affect rate must stay at +0xf2d");
static_assert(offsetof(_Charac_info::Fields, exp_equip_slot_state_) ==
                  0x1286u,
              "_Charac_info EXP equip-slot state must stay at +0x1286");
static_assert(offsetof(_Charac_info::Fields, event_character_grow_type_) ==
                  0x1287u,
              "_Charac_info event grow type must stay at +0x1287");
static_assert(offsetof(_Charac_info::Fields, link_type_) == 0x128eu,
              "_Charac_info link type must stay at +0x128e");
static_assert(offsetof(_Charac_info::Fields, aura_avatar_option_) ==
                  0x12c9u,
              "_Charac_info aura option storage must stay at +0x12c9");
static_assert(offsetof(_Charac_info::Fields, assault_money_) == 0x12f9u,
              "_Charac_info assault money must stay at +0x12f9");
static_assert(offsetof(_Charac_info::Fields, character_option_) ==
                  0x12ffu,
              "_Charac_info character option storage must stay at +0x12ff");
static_assert(offsetof(_Charac_info::Fields, blue_marble_enter_count_) ==
                  0x1505u,
              "_Charac_info blue-marble entry count must stay at +0x1505");
static_assert(offsetof(_Charac_info::Fields, maximum_equip_level_) == 0x150au,
              "_Charac_info maximum equip level must stay at +0x150a");
static_assert(offsetof(_Charac_info::Fields, character_view_state_) == 0x150eu,
              "_Charac_info character-view state must stay at +0x150e");
static_assert(offsetof(_Charac_info::Fields,
                       contract_cube_premium_enabled_) == 0x1516u,
              "_Charac_info cube-premium state must stay at +0x1516");
static_assert(offsetof(_Charac_info::Fields, grow_type_) == 0x0029u,
              "_Charac_info STGrowthType must stay at +0x0029");
static_assert(offsetof(_Charac_info::Fields, additional_info_) == 0x0088u,
              "_Charac_info _Additioal_info must stay at +0x0088");
static_assert(offsetof(_Charac_info::Fields, inventory_) == 0x00dau,
              "_Charac_info CInventory must stay at +0x00da");
static_assert(offsetof(_Charac_info::Fields, skill_slot_) == 0x8dau,
              "_Charac_info SkillSlot must stay at +0x8da");
static_assert(offsetof(_Charac_info::Fields, cargo_) == 0xdf2u,
              "_Charac_info CCargo must stay at +0xdf2");
static_assert(offsetof(_Charac_info::Fields, visible_flag_) == 0xe88u,
              "_Charac_info tag visibility must stay at +0xe88");
static_assert(offsetof(_Charac_info::Fields, power_war_) == 0xed5u,
              "_Charac_info power-war child must stay at +0xed5");
static_assert(offsetof(_Charac_info::Fields, cool_time_items_) == 0xee2u,
              "_Charac_info cool-time vector must stay at +0xee2");
static_assert(offsetof(_Charac_info::Fields, effect_items_) == 0xeeeu,
              "_Charac_info effect vector must stay at +0xeee");
static_assert(offsetof(_Charac_info::Fields, blood_best_records_) ==
                  0xf14u,
              "_Charac_info blood-record map must stay at +0xf14");
static_assert(offsetof(_Charac_info::Fields, premium_lethe_first_) ==
                  0xf35u,
              "_Charac_info first Lethe child must stay at +0xf35");
static_assert(offsetof(_Charac_info::Fields, premium_lethe_second_) ==
                  0x10ceu,
              "_Charac_info second Lethe child must stay at +0x10ce");
static_assert(offsetof(_Charac_info::Fields, skill_commands_) == 0x129cu,
              "_Charac_info skill-command map must stay at +0x129c");
static_assert(offsetof(_Charac_info::Fields, aura_avatar_option_) ==
                  0x12c9u,
              "_Charac_info aura option child must stay at +0x12c9");
static_assert(offsetof(_Charac_info::Fields, character_option_) ==
                  0x12ffu,
              "_Charac_info CharacterOption must stay at +0x12ff");
static_assert(offsetof(_Charac_info::Fields, seria_room_ani_deco_) ==
                  0x1512u,
              "_Charac_info Seria animation child must stay at +0x1512");
static_assert(offsetof(_Charac_info::Fields, finish_point_) == 0xe4eu,
              "_Charac_info finish point must stay at +0xe4e");
static_assert(offsetof(_Charac_info::Fields, finish_point_total_) == 0xe52u,
              "_Charac_info finish point total must stay at +0xe52");
static_assert(offsetof(_Charac_info::Fields, finish_point_detail_) == 0xe56u,
              "_Charac_info finish point detail must stay at +0xe56");
static_assert(offsetof(_Charac_info::Fields, online_preliminary_team_handle_) ==
                  0xed1u,
              "_Charac_info online-preliminary team link must stay at +0xed1");
static_assert(sizeof(_Charac_info::Fields) == 0x1519u,
              "_Charac_info field projection must cover the target extent");
static_assert(sizeof(_Charac_info) == 0x1519u,
              "_Charac_info ABI size must stay 0x1519");

typedef std::map<int, GameResultType> CUserBattleRecordMap;

typedef char CUserBattleRecordMap_Size_Must_Be_0x18[
    (sizeof(CUserBattleRecordMap) == 0x18) ? 1 : -1];

// CUserCharacInfo gameplay ABI is declared exclusively on the canonical class
// above; no free member aliases remain in this header.

#endif  // OLDSRV_OLD_GAME_USERINFO_CUSER_H_
