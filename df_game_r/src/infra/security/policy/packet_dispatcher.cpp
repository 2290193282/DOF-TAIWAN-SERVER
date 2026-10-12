#include "infra/security/policy/packet_dispatcher.h"
#include "infra/security/policy/packet_dispatcher_restored_registry.h"

#include <cstring>

#include "common/manager/app_load_checker.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/network/protocol/client_protocol_profile.h"
#include "infra/network/transport/packet_tracer.h"
#include "infra/log/format/clog.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/packet_dispatch/security_card_dispatchers.h"
#include "old_data/channel/packet_dispatch/pvp_channel_info_dispatcher.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/dungeon/instance/monster_move_system.h"
#include "old_game/dungeon/instance/packet_dispatcher_instant_reinforce.h"
#include "old_game/dungeon/instance/packet_dispatcher_break_trap_result.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/dungeon/progress/dragon_timeattack_rank_runtime.h"
#include "old_game/dungeon/instance/packet_dispatcher_derived_batch040.h"
#include "old_game/economy/inventory/packet_dispatcher_exchange_item_from_npc.h"
#include "old_game/economy/equipment/packet_dispatcher_upgrade_room.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"
#include "old_game/pvp/battlefield/packet_dispatcher_pvp_assault.h"
#include "old_game/pvp/battlefield/packet_dispatcher_sr08_pvp_unclaimed.h"
#include "old_game/player/session/packet_dispatcher_return_select_character.h"
#include "old_game/pvp/season/packet_dispatcher_res_pvp_rank.h"
#include "old_game/pvp/battlefield/integrated_match_pvp_score_runtime.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/social/party/packet_dispatcher_set_party_info.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/activity/reward/packet_dispatcher_event_rewards.h"
#include "old_game/activity/schedule/collect_box_dispatcher.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/activity/schedule/kiri_cargo_packet_dispatchers.h"
#include "old_game/activity/schedule/levelup_support_3rd.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/packet_dispatcher_event_account_fatigue_stat.h"
#include "old_game/activity/schedule/rainbow_snake_packet_dispatchers.h"
#include "old_game/social/party/packet_dispatcher_group_business_043.h"
#include "old_game/economy/inventory/packet_dispatcher_item_actions.h"
#include "old_game/economy/inventory/commerce_packet_dispatch_runtime.h"
#include "old_game/economy/trade/trade_state_dispatcher.h"
#include "old_game/gm/core/dispatcher_new_gmdebug_command.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/packet_dispatcher_response_creature.h"
#include "old_game/player/core/inform_notice_dispatcher.h"
#include "old_game/player/core/userinfo_dispatcher_reads.h"
#include "old_game/progression/quest/npc_item_drop_runtime.h"
#include "old_game/progression/career/packet_dispatcher_monstercard_bind.h"
#include "old_game/world/region/packet_dispatcher_set_pos_runtime.h"
#include "old_game/world/region/packet_dispatcher_world_business_043.h"
#include "old_game/world/region/packet_dispatcher_world_session.h"
#include "old_game/world/region/world_dispatcher_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/change_host_dispatcher.h"
#include "infra/network/dispatcher/peer_connect_result_dispatcher.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/task_queue/message_counter.h"
#include "common/manager/system_time.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"
#include "infra/security/policy/check_conn_dispatcher.h"
#include "infra/security/policy/packet_integrity.h"
#include "infra/security/policy/sender_security_telemetry_dispatchers.h"
#include "infra/network/dispatcher/trace_error_dispatcher.h"
#include "infra/network/dispatcher/set_ip_dispatcher.h"

namespace {
IDispatcher* g_channel_zombie_exit_dispatcher = 0;
IDispatcher* g_channel_zombie_success_dispatcher = 0;
}

// IDA_EA: 0x8590A2E
// IDA_NAME: _ZN16PacketDispatcherC2Ev
// DEMANGLED: PacketDispatcher::PacketDispatcher(void)
PacketDispatcher::PacketDispatcher() : local_china_error_dispatcher_() {
    // The target registry clears every direct 140828 CMD slot before
    // registering concrete handlers. Register only leaves whose typed receive
    // and send paths are restored; all other slots remain null and take the
    // original get_dispatcher failure route until their real closure exists.
    std::memset(dispatchers_, 0, sizeof(dispatchers_));

    // The generated block below reproduces the active old-C2 assignments as
    // direct source statements.  It has no registry include, factory, or
    // second routing layer.
    // BEGIN GENERATED 0828 DIRECT DISPATCHER REGISTRATIONS
    // Generated from old PacketDispatcher::C2 in slot order.
    // Explicit zero-return SR08 holdbacks remain null.
    dispatchers_[ENUM_CMDPACKET_CHECK_CONNECTION] = new DisPatcher_CheckConn();
    dispatchers_[ENUM_CMDPACKET_LOGIN] = new DisPatcher_Login();
    dispatchers_[ENUM_CMDPACKET_SET_UDP_IP_PORT] = new DisPatcher_SetIP();
    dispatchers_[ENUM_CMDPACKET_EXIT] = new DisPatcher_Exit();
    dispatchers_[ENUM_CMDPACKET_SELECT_CHARACTER] = new DisPatcher_SelectCharac();
    dispatchers_[ENUM_CMDPACKET_CREATE_CHARACTER] = new DisPatcher_CreateCharac();
    dispatchers_[ENUM_CMDPACKET_DELETE_CHARACTER] = new DisPatcher_DeleteCharac();
    dispatchers_[ENUM_CMDPACKET_RETURN_SELECT_CHARACTER] = new DisPatcher_ReturnToSelectCharacter();
    dispatchers_[ENUM_CMDPACKET_GET_USERINFO] = new DisPatcher_GetUserInfo();
    dispatchers_[ENUM_CMDPACKET_RECOVER_STAMINA] = new DisPatcher_RecoverStamina();
    dispatchers_[ENUM_CMDPACKET_REQUEST_PEER] = new DisPatcher_ReqPeer();
    dispatchers_[ENUM_CMDPACKET_RESPONSE_PEER] = new DisPatcher_ResPeer();
    dispatchers_[ENUM_CMDPACKET_SET_PARTY_INFO] = new DisPatcher_SetPartyInfo();
    dispatchers_[ENUM_CMDPACKET_LEAVE_PARTY] = new DisPatcher_LeaveParty();
    dispatchers_[ENUM_CMDPACKET_WALKOUT_PARTY_MEMBER] = new DisPatcher_WalkoutParty();
    dispatchers_[ENUM_CMDPACKET_ENTER_SELECT_DUNGEON] = new DisPatcher_StartGame();
    dispatchers_[ENUM_CMDPACKET_SELECT_DUNGEON] = new DisPatcher_SelectDungeon();
    dispatchers_[ENUM_CMDPACKET_SEND_MESSAGE] = new DisPatcher_SendMess();
    dispatchers_[ENUM_CMDPACKET_DELETE_ITEM] = new DisPatcher_DeleteItem();
    dispatchers_[ENUM_CMDPACKET_MOVE_ITEMSPACE] = new DisPatcher_MoveItem();
    dispatchers_[ENUM_CMDPACKET_SORT_ITEM] = new DisPatcher_SortItem();
    dispatchers_[ENUM_CMDPACKET_BUY_ITEM] = new DisPatcher_BuyItem();
    // 0828 removed ENUM_CMDPACKET_RENT_AVATAR; DisPatcher_RentAvatar() is not registered.
    // 0828 removed EXTEND_AVATAR; no legacy dispatcher is compiled.
    dispatchers_[ENUM_CMDPACKET_SELL_ITEM] = new DisPatcher_SellItem();
    dispatchers_[ENUM_CMDPACKET_REPAIR_EQUIPMENT] = new DisPatcher_RepairEquip();
    dispatchers_[ENUM_CMDPACKET_SET_ITEMTRADE_STATE] = new DisPatcher_SetTradeState();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_ITEM] = new DisPatcher_Compound();
    dispatchers_[ENUM_CMDPACKET_DISJOINT_ITEM] = new DisPatcher_DisJointItem();
    dispatchers_[ENUM_CMDPACKET_USE_LOTTERY_ITEM] = new DisPatcher_Lottery();
    dispatchers_[ENUM_CMDPACKET_CHANGE_SKILLSLOT] = new DisPatcher_ChangeSkill();
    dispatchers_[ENUM_CMDPACKET_BUY_SKILL] = new DisPatcher_BuySkill();
    dispatchers_[ENUM_CMDPACKET_INCREASE_STATUS] = new DisPatcher_IncreStatus();
    dispatchers_[ENUM_CMDPACKET_ACCEPT_QUEST] = new DisPatcher_QuestAction();
    dispatchers_[ENUM_CMDPACKET_GIVEUP_QUEST] = new DisPatcher_QuestAction();
    dispatchers_[ENUM_CMDPACKET_SET_QUEST_TRIGGER] = new DisPatcher_QuestAction();
    dispatchers_[ENUM_CMDPACKET_FINISH_QUEST] = new DisPatcher_QuestAction();
    dispatchers_[ENUM_CMDPACKET_SET_USER_POSITION] = new DisPatcher_SetPos();
    dispatchers_[ENUM_CMDPACKET_SET_USER_AREA] = new DisPatcher_SetArea();
    // 0828 removed ENUM_CMDPACKET_FISHING; DisPatcher_Fish() is not registered.
    dispatchers_[ENUM_CMDPACKET_FINISH_LOADING] = new DisPatcher_FinishLoad();
    dispatchers_[ENUM_CMDPACKET_USE_SKILL] = new DisPatcher_UseSkill();
    dispatchers_[ENUM_CMDPACKET_DIE_MONSTER] = new DisPatcher_DieMob();
    dispatchers_[ENUM_CMDPACKET_DIE_CHARACTER] = new DisPatcher_DieCharacInDungeon();
    dispatchers_[ENUM_CMDPACKET_USE_COIN] = new DisPatcher_UseCoin();
    dispatchers_[ENUM_CMDPACKET_GIVEUP_GAME] = new DisPatcher_GiveUpGame();
    dispatchers_[ENUM_CMDPACKET_GET_ITEM] = new DisPatcher_GetItem();
    dispatchers_[ENUM_CMDPACKET_USE_STACKABLE] = new DisPatcher_UseStack();
    dispatchers_[ENUM_CMDPACKET_MOVE_MAP] = new DisPatcher_MoveMap();
    dispatchers_[ENUM_CMDPACKET_SET_PLAY_RESULT] = new DisPatcher_SetPlayResult();
    dispatchers_[ENUM_CMDPACKET_DROP_ITEM] = new DisPatcher_DropItem();
    dispatchers_[ENUM_CMDPACKET_DECREASE_DURABILITY] = new DisPatcher_UseEquip();
    dispatchers_[ENUM_CMDPACKET_REPORT_BAD_P2P_USER] = new Dispatcher_ReportBadP2PUser();
    dispatchers_[ENUM_CMDPACKET_MAKE_PVP_ROOM] = new DisPatcher_MakePVP();
    dispatchers_[ENUM_CMDPACKET_ENTER_PVP_ROOM] = new DisPatcher_EnterPVP();
    dispatchers_[ENUM_CMDPACKET_SET_PVP_SEAT_STATE] = new DisPatcher_SetPVPSeat();
    dispatchers_[ENUM_CMDPACKET_SET_PVP_READY_STATE] = new DisPatcher_SetPVPReady();
    dispatchers_[ENUM_CMDPACKET_SET_PVP_TEAM_MODE] = new DisPatcher_SetPVPTeam();
    dispatchers_[ENUM_CMDPACKET_DIE_PVP_CHARACTER] = new DisPatcher_DiePvP();
    dispatchers_[ENUM_CMDPACKET_PVP_TIME_OUT] = new DisPatcher_PvPTimeout();
    dispatchers_[ENUM_CMDPACKET_END_PVP_RESULT] = new DisPatcher_EndPvPResult();
    dispatchers_[ENUM_CMDPACKET_RES_PVP_RANK] = new DisPatcher_ResPvpRank();
    dispatchers_[ENUM_CMDPACKET_SET_PVP_MAP_INDEX] = new DisPatcher_SetPVPMapIndex();
    dispatchers_[ENUM_CMDPACKET_ADD_FRIEND] = new DisPatcher_AddFriend();
    dispatchers_[ENUM_CMDPACKET_REMOVE_FRIEND] = new DisPatcher_RemoveFriend();
    dispatchers_[ENUM_CMDPACKET_DEBUG_COMMAND] = new DisPatcher_DebugCommand();
    dispatchers_[ENUM_CMDPACKET_CERA] = new DisPatcher_Cera();
    dispatchers_[ENUM_CMDPACKET_BUY_CERASHOP_ITEM] = new Dispatcher_BuyCeraShopItem();
    dispatchers_[ENUM_CMDPACKET_GEN_CERATICKET] = new Dispatcher_GenCeraTicket();
    dispatchers_[ENUM_CMDPACKET_REQUEST_PVPEXP_OF_WEEK] = new Dispatcher_Request_PvpExpOfWeek();
    dispatchers_[ENUM_CMDPACKET_GUILD_MEMER_LIST] = new Dispatcher_CallGuildMembers();
    dispatchers_[ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT] = new Dispatcher_CallGuildCreateRight();
    dispatchers_[ENUM_CMDPACKET_SCORE_SCROLL_STATE] = new Dispatcher_Score_Scroll_State();
    dispatchers_[ENUM_CMDPACKET_CARD_SELECT_RIGHT_STATE] = new Dispatcher_Card_Select_Right_State();
    dispatchers_[ENUM_CMDPACKET_SELECT_CARD] = new Dispatcher_Select_Card();
    dispatchers_[ENUM_CMDPACKET_EPLP_COMMAND] = new Dispatcher_EPLPCommand();
    dispatchers_[ENUM_CMDPACKET_CALL_GUILD_LEVEL_UP] = new Dispatcher_CallGuildLevelUp();
    dispatchers_[ENUM_CMDPACKET_GUILD_INFO] = new Dispatcher_CallGuildInfo();
    dispatchers_[ENUM_CMDPACKET_REQUEST_MEMBER_ENTER] = new Dispatcher_RequestMemberEnter();
    dispatchers_[ENUM_CMDPACKET_MEMBER_ENTER_REPLY] = new Dispatcher_MemberEnterReply();
    dispatchers_[ENUM_CMDPACKET_MEMBER_SECEDE] = new Dispatcher_MemberSecede();
    dispatchers_[ENUM_CMDPACKET_CALL_MEMER_LIST] = new Dispatcher_CallMemberList();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ITEM] = new Dispatcher_UpgradeItem();
    dispatchers_[ENUM_CMDPACKET_RESET_ITEM_ATTR] = new Dispatcher_ModItemAttr();
    dispatchers_[ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM] = new Dispatcher_BuyPrivateStoreItem();
    dispatchers_[ENUM_CMDPACKET_ENTER_PRIVATE_STORE] = new Dispatcher_EnterPrivateStore();
    dispatchers_[ENUM_CMDPACKET_EXIT_PRIVATE_STORE] = new Dispatcher_ExitPrivateStore();
    dispatchers_[ENUM_CMDPACKET_CREATE_PRIVATE_STORE] = new Dispatcher_CreatePrivateStore();
    dispatchers_[ENUM_CMDPACKET_REMOVE_PRIVATE_STORE] = new Dispatcher_RemovePrivateStore();
    dispatchers_[ENUM_CMDPACKET_COMPLETE_DISPLAY] = new Dispatcher_CompleteDisplay();
    dispatchers_[ENUM_CMDPACKET_MOVE_TO_GATE] = new Dispatcher_MoveToGate();
    dispatchers_[ENUM_CMDPACKET_ENTER_WARROOM] =
        new Dispatcher_EnterWarRoom();
    dispatchers_[ENUM_CMDPACKET_SET_WARROOM_SEAT_STATE] =
        new Dispatcher_SetWarRoomSeatState();
    dispatchers_[ENUM_CMDPACKET_DIE_WARROOM_CHARACTER] =
        new Dispatcher_DieWarRoomCharacter();
    dispatchers_[ENUM_CMDPACKET_START_WARROOM_TEMP] = new DisPatcher_StartWarRoomTemp();
    dispatchers_[ENUM_CMDPACKET_MAILBOX_SEND] = new Dispatcher_MailBox_Send();
    dispatchers_[ENUM_CMDPACKET_MAILBOX_EXTRACT_ITEM] = new Dispatcher_MailBox_Extract();
    dispatchers_[ENUM_CMDPACKET_MAILBOX_OPEN] = new Dispatcher_MailBox_Open();
    dispatchers_[ENUM_CMDPACKET_PEER_CONNECT_RESULT] = new Dispatcher_PeerConnectResult();
    dispatchers_[ENUM_CMDPACKET_QUICK_JOIN_ROOM] = new Dispatcher_QuickJoinRoom();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_AVATAR] = new Dispatcher_Compound_Avatar();
    dispatchers_[ENUM_CMDPACKET_RENAME_CREATURE] = new Dispatcher_RenameCreature();
    // 140828 CMD101: exact u16 requester scene id; success is the existing
    // area-scoped NOTI104 creature response broadcast.
    dispatchers_[ENUM_CMDPACKET_RESPONSE_CREATURE] =
        new Dispatcher_ResponseCreature();
    dispatchers_[ENUM_CMDPACKET_HATCH_CREATURE] = new Dispatcher_HatchCreature();
    dispatchers_[ENUM_CMDPACKET_BUY_AUTOMAT_ITEM] = new Dispatcher_BuyAutomatItem();
    dispatchers_[ENUM_CMDPACKET_GATHERING_PARTY_STATUS] = new Dispatcher_GatheringPartyStatus();
    dispatchers_[ENUM_CMDPACKET_GM_COMMAND] = new Dispatcher_GM_Command();
    dispatchers_[ENUM_CMDPACKET_REPORT_4_HACK] = new DisPatcher_ReportClient4Hack();
    dispatchers_[ENUM_CMDPACKET_GUILD_WAR_INFO] = new DisPatcher_RequestGuildWarInfo();
    dispatchers_[ENUM_CMDPACKET_PVP_HEART_BEAT] = new DisPatcher_PvpHeartBeat();
    dispatchers_[ENUM_CMDPACKET_CODE_CHECKSUM] = new Dispatcher_CodeCheckSum();
    dispatchers_[ENUM_CMDPACKET_PVP_REQUEST_FIGHT] = new Dispatcher_PVPRequestFight();
    dispatchers_[ENUM_CMDPACKET_MOUSEREGISTER] = new Dispatcher_MouseRegister();
    dispatchers_[ENUM_CMDPACKET_CREATURE_SEND_MESSAGE] = new Dispatcher_CreatureSendMessage();
    dispatchers_[ENUM_CMDPACKET_TRACE_ERROR] = new Dispatcher_TraceError();
    dispatchers_[ENUM_CMDPACKET_BOSS_DIE_CHECK] = new Dispatcher_BossDieCheck();
    dispatchers_[ENUM_CMDPACKET_REGISITER_TO_BLACKLIST] = new Dispatcher_RegisterToBlackList();
    dispatchers_[ENUM_CMDPACKET_DELETE_TO_BLACKLIST] = new Dispatcher_DeleteToBlackList();
    dispatchers_[ENUM_CMDPACKET_REQUEST_BLACKLIST] = new Dispatcher_RequestBlackList();
    dispatchers_[ENUM_CMDPACKET_CHANGE_HOST] = new Dispatcher_ChangeHost();
    dispatchers_[ENUM_CMDPACKET_CREATURE_SCRIPT_MESSAGE] = new Dispatcher_CreatureScriptMessage();
    dispatchers_[ENUM_CMDPACKET_CHARACTER_STATISTIC] = new Dispatcher_CharacterStatistic();
    // RECONSTRUCTED_0828: the client request and signal-100 response are
    // exact; the intervening Guild/Monitor request type 0x042f is recovered
    // from the adjacent proven 0x0430 owner and is documented by the feature.
    dispatchers_[ENUM_CMDPACKET_GUILDMEMBER_NAMING] =
        new Dispatcher_GuildMemberNaming();
    dispatchers_[ENUM_CMDPACKET_SET_SUB_GUILD_MASTER] = new Dispatcher_SetSubGuildMaster();
    dispatchers_[ENUM_CMDPACKET_BACK_2_VILLAGE] = new Dispatcher_Back2Village();
    dispatchers_[ENUM_CMDPACKET_CHANGE_LETTER_STAT] = new Dispatcher_ChangeLetterStat();
    dispatchers_[ENUM_CMDPACKET_CHANGE_CHARAC_NAME] = new Dispatcher_ChangeCharacName();
    dispatchers_[ENUM_CMDPACKET_QUERY_CHARAC_INFO] = new Dispatcher_QueryCharacInfo();
    dispatchers_[ENUM_CMDPACKET_REPORT_MANNERLESS_USER] = new Dispatcher_ReportMannerlessUser();
    dispatchers_[ENUM_CMDPACKET_ALLDIE_MONSTER] = new Dispatcher_DieMobAll();
    dispatchers_[ENUM_CMDPACKET_GUILD_ALL_MEMBER_LIST] = new Dispatcher_CallGuildAllMembers();
    dispatchers_[ENUM_CMDPACKET_RPY_HUMAN_CERTIFY] = new Dispatcher_HumanCertify();
    dispatchers_[ENUM_CMDPACKET_CHANGE_TUTORIAL_FLAG] = new Dispatcher_ChangeTutorialFlag();
    dispatchers_[ENUM_CMDPACKET_COMPLETE_LOAD_ASSAULT] = new Dispatcher_CompleteLoadAssault();
    dispatchers_[ENUM_CMDPACKET_CONNECT_P2P_ASSAULT] = new Dispatcher_ConnectP2PAssault();
    dispatchers_[ENUM_CMDPACKET_DIE_ASSAULT_PLAYER] = new Dispatcher_DieAssaultPlayer();
    dispatchers_[ENUM_CMDPACKET_CHANGE_HP] = new Dispatcher_ChangeHp();
    dispatchers_[ENUM_CMDPACKET_BVHACKINFO] = new Dispatcher_BVHackInfo();
    dispatchers_[ENUM_CMDPACKET_CALL_GUILD_INVITE] = new Dispatcher_CallGuildInvite();
    dispatchers_[ENUM_CMDPACKET_REPLY_GUILD_INVITE] = new Dispatcher_ReplyGuildInvite();
    dispatchers_[ENUM_CMDPACKET_REQ_GUILD_SECEDE] = new Dispatcher_RequestGuildSecede();
    dispatchers_[ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD] = new Dispatcher_NotifyMessageToGuild();
    dispatchers_[ENUM_CMDPACKET_GUILD_MASTER_DELEGATE] = new Dispatcher_GuildMasterDelegate();
    dispatchers_[ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE] = new Dispatcher_CheckGuildNameDouble();
    dispatchers_[ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE] = new Dispatcher_CheckGuildAddressDouble();
    dispatchers_[ENUM_CMDPACKET_OPEN_GUILD_CREATE_WINDOW] = new Dispatcher_OpenGuildCreateWindow();
    dispatchers_[ENUM_CMDPACKET_DEATH_TOWER_STAGE_CMD] = new Dispatcher_DeathTowerStageCommand();
    dispatchers_[ENUM_CMDPACKET_USE_BOOSTER_ITEM] = new Dispatcher_UseBoosterItem();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_ISSUE] = new Dispatcher_SecurityCardIssue();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_DISUSE] = new Dispatcher_SecurityCardDisuse();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_AUTH_REQ] = new Dispatcher_SecurityCardAuthReq();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY] = new Dispatcher_SecurityCardAuthRpy();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_CERT_KEY] = new Dispatcher_SecurityCardCertKey();
    dispatchers_[ENUM_CMDPACKET_CALL_PARTY_MEMBER_REALTIME_INFO] = new Dispatcher_CallPartyMemberRealtimeInfo();
    dispatchers_[ENUM_CMDPACKET_EVADE_ASSAULT] = new Dispatcher_EvadeAssault();
    dispatchers_[ENUM_CMDPACKET_AGREE_ENCHANT] = new Dispatcher_AgreeEnchant();
    dispatchers_[ENUM_CMDPACKET_TRY_ENCHANT] = new Dispatcher_TryEnchant();
    dispatchers_[ENUM_CMDPACKET_PUT_ITEM_FOR_ENCHANT] = new Dispatcher_PutItemForEnchant();
    dispatchers_[ENUM_CMDPACKET_CLIENT_SPEC_STATISTIC] = new Dispatcher_Client_Spec_Statistic();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_AUTH_CANCEL] = new Dispatcher_SecurityCardAuthCancel();
    // CLIENT_IDA_EVIDENCE: the official 0627 registry owns the complete
    // Dispatcher_New_Gmdebug_Command at old CMD179.  The same semantic command
    // is CMD176 in the 0828 enum; only the root assignment was missing.
    dispatchers_[ENUM_CMDPACKET_GMDEBUG_COMMAND] = new Dispatcher_New_Gmdebug_Command();
    dispatchers_[ENUM_CMDPACKET_JOIN_POWER] = new Dispatcher_Join_Power();
    dispatchers_[ENUM_CMDPACKET_SECEDE_POWER] = new Dispatcher_Secede_Power();
    dispatchers_[ENUM_CMDPACKET_CHANGE_GUILD_NAME] = new Dispatcher_ChangeGuildName();
    dispatchers_[ENUM_CMDPACKET_SDC_DAMAGE_CHECK] = new Dispatcher_SDC_Damage_Check();
    dispatchers_[ENUM_CMDPACKET_SDC_ACTIVESTATUS_CHECK] = new Dispatcher_SDC_ActiveStatus_Damage_Check();
    dispatchers_[ENUM_CMDPACKET_AUCTION_ASK_AVERAGE_PRICE] = new Dispatcher_AuctionAskAveragePrice();
    dispatchers_[ENUM_CMDPACKET_AUCTION_REGIST_ITEM] = new Dispatcher_AuctionRegistItem();
    dispatchers_[ENUM_CMDPACKET_AUCTION_REGIST_CANCEL] = new Dispatcher_AuctionRegistCancel();
    dispatchers_[ENUM_CMDPACKET_AUCTION_BIDDING] = new Dispatcher_AuctionBidding();
    dispatchers_[ENUM_CMDPACKET_AUCTION_SEARCH_BY_ITEMKEY] = new Dispatcher_AuctionSearchByItemKey();
    dispatchers_[ENUM_CMDPACKET_AUCTION_SEARCH_BY_NOITEMKEY] = new Dispatcher_AuctionSearchByNoItmeKey();
    dispatchers_[ENUM_CMDPACKET_AUCTION_MY_REGISTED_ITEM_INFO] = new Dispatcher_AuctionMyRegistedItemInfo();
    dispatchers_[ENUM_CMDPACKET_AUCTION_MY_BIDDING_INFO] = new Dispatcher_AuctionMyBiddingInfo();
    dispatchers_[ENUM_CMDPACKET_AUCTION_MY_AUCTION_HISTORY] = new Dispatcher_AuctionMyAuctionHistory();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_EVENT_STORY_PAUSE] = new Dispatcher_DungeonEventStoryPause();
    dispatchers_[ENUM_CMDPACKET_JOIN_POWER_WAR] = new Dispatcher_JoinPowerWar();
    dispatchers_[ENUM_CMDPACKET_GOBLIN_PAD_STATUS] = new Dispatcher_GoblinPadStatus();
    dispatchers_[ENUM_CMDPACKET_FRAME_LAG_STATISTICS] = new Dispatcher_FrameLagStatistics();
    dispatchers_[ENUM_CMDPACKET_PVP_CHANNEL_INFO] = new Dispatcher_PvPChannelInfo();
    dispatchers_[ENUM_CMDPACKET_REQUEST_MATCH] = new Dispatcher_RequestMatch();
    dispatchers_[ENUM_CMDPACKET_SAVE_GAME_OPTION_1] = new Dispatcher_SaveGameOption_1();
    dispatchers_[ENUM_CMDPACKET_SAVE_GAME_OPTION_2] = new Dispatcher_SaveGameOption_2();
    dispatchers_[ENUM_CMDPACKET_SECURITY_CARD_RETRANSFER] = new Dispatcher_SecurityCardRetransfer();
    dispatchers_[ENUM_CMDPACKET_USE_EMBLEM] = new Dispatcher_UseJewel();
    dispatchers_[ENUM_CMDPACKET_DISJOINT_AVATAR] = new Dispatcher_DisJointAvatar();
    dispatchers_[ENUM_CMDPACKET_PURIFY_ITEM] = new Dispatcher_PurifyItem();
    dispatchers_[ENUM_CMDPACKET_INVEST_ITEM_AMPLIFY_OPTION] = new Dispatcher_InvestItemAmplifyOption();
    dispatchers_[ENUM_CMDPACKET_ADD_AVATAR_SOCKET] = new Dispatcher_AddSocketToAvatar();
    dispatchers_[ENUM_CMDPACKET_SHOP_COIN_EVENT] = new Dispatcher_Shop_Coin_Event();
    dispatchers_[ENUM_CMDPACKET_USE_RANDOMBOX_ITEM] = new DisPatcher_MultiBoxLottery();
    dispatchers_[ENUM_CMDPACKET_UDP_CHARACTERISTIC] = new DisPatcher_UdpCharacteristic();
    dispatchers_[ENUM_CMDPACKET_ONEDAY_LETHE] = new DIspatcher_OneDayLetheTicket();
    dispatchers_[ENUM_CMDPACKET_DISGUISE_REQUEST] =
        new Dispatcher_DisguiseRequest();
    dispatchers_[ENUM_CMDPACKET_DISGUISE_CANCEL] = new Dispatcher_DisguiseCancel();
    dispatchers_[ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_LIST] = new Dispatcher_RequestPCRoomPlayerList();
    dispatchers_[ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_COUNT] = new Dispatcher_RequestPCRoomPlayerCount();
    dispatchers_[ENUM_CMDPACKET_USE_VENDING_MACHINE] = new Dispatcher_UseVendingMachine();
    dispatchers_[ENUM_CMDPACKET_ASSERT_INFO] = new Dispatcher_AssertManager();
    dispatchers_[ENUM_CMDPACKET_OVERFLOW_INFO] = new Dispatcher_Overflow_Info();
    dispatchers_[ENUM_CMDPACKET_SERVER_MESSAGE_SEND] = new Dispatcher_UpdateServerMessage();
    dispatchers_[ENUM_CMDPACKET_SERVER_MESSAGE_CHECK] = new Dispatcher_CheckEnableServerMessage();
    dispatchers_[ENUM_CMDPACKET_GIVE_GIFT_TO_NPC] =
        new Dispatcher_GiveGiftToNPC();
    dispatchers_[ENUM_CMDPACKET_GOBLIN_PAD_RESPONSE_CRYPT_KEY] = new Dispatcher_GoblinPadRequestCryptKey();
    dispatchers_[ENUM_CMDPACKET_WRITE_GUILD_MEMBER_MEMO] = new Dispatcher_WriteGuildMemberMemo();
    // 0828 removed HELLPARTY_START; no legacy dispatcher is compiled.
    dispatchers_[ENUM_CMDPACKET_POWER_WAR_PROCESS_INFO] = new Dispatcher_PowerWarProcessInfo();
    dispatchers_[ENUM_CMDPACKET_CREATE_DISJOINT_STORE] = new Dispatcher_CreateDisjointStore();
    dispatchers_[ENUM_CMDPACKET_REQUEST_DISJOINT_ITEM] = new Dispatcher_RequestDisjointItem();
    dispatchers_[ENUM_CMDPACKET_REPAIR_DISJOINT_MACHINE] = new Dispatcher_RepairDisjointMachine();
    dispatchers_[ENUM_CMDPACKET_TELEPORT] = new Dispatcher_Teleport();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB] = new Dispatcher_CompoundItemByExpertJob();
    dispatchers_[ENUM_CMDPACKET_GIVEUP_EXPERT_JOB] = new Dispatcher_GiveupExpertJob();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_DISJOINT_MACHINE] = new Dispatcher_UpgradeDisjointMachine();
    dispatchers_[ENUM_CMDPACKET_ENTER_DISJOINT_STORE] = new Dispatcher_EnterDisjointStore();
    dispatchers_[ENUM_CMDPACKET_CLOSE_DISJOINT_STORE] = new Dispatcher_CloseDisjointStore();
    dispatchers_[ENUM_CMDPACKET_REPORT_ABUSE_USER] = new Dispatcher_ReportAbuseUser();
    dispatchers_[ENUM_CMDPACKET_LOAD_COMPLETE_AFTER_ASSAULT] = new Dispatcher_CompleteLoadAfterAssault();
    dispatchers_[ENUM_CMDPACKET_CONNECT_P2P_AFTER_ASSAULT] = new Dispatcher_ConnectP2PAfterAssault();
    dispatchers_[ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM] = new Dispatcher_GuildCargoPushItem();
    dispatchers_[ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM] = new Dispatcher_GuildCargoPopItem();
    dispatchers_[ENUM_CMDPACKET_GUILD_CARGO_MOVE_ITEM] = new Dispatcher_GuildCargoMoveItem();
    dispatchers_[ENUM_CMDPACKET_LODING_TIME_REPORT] = new Dispatcher_LoadingTimeReport();
    dispatchers_[ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM] = new Dispatcher_UseSharedEffectItem();
    dispatchers_[ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM] = new Dispatcher_BuyCeraShopLimitItem();
    dispatchers_[ENUM_CMDPACKET_ADD_HACKTYPE_CNT] = new Dispatcher_AddHackTypeCount();
    dispatchers_[ENUM_CMDPACKET_CHANGE_EMOTION] = new Dispatcher_ChangeEmotion();
    dispatchers_[ENUM_CMDPACKET_DIE_BLOOD_MONSTER] = new Dispatcher_DieBloodMonster();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_EMBLEM] = new Dispatcher_CompoundEmblem();
    dispatchers_[ENUM_CMDPACKET_MOTION_RESULT] = new Dispatcher_CheckAssaultMotionHack();
    dispatchers_[ENUM_CMDPACKET_BLOOD_ROUND_UI_PREPARE_FINISH_] = new Dispatcher_BloodRoundUiPrepareFinish();
    dispatchers_[ENUM_CMDPACKET_REQUEST_CONDITION_EVENT_REWARD] = new Dispatcher_RequestConditionEventReward();
    dispatchers_[ENUM_CMDPACKET_EVENT_ALL_USER_GIFT] =
        new Dispatcher_EventAllUserGift();
    dispatchers_[ENUM_CMDPACKET_CHANGE_ANOTHER_SKILL_TREE] = new Dispatcher_ChangeAnotherSkillTree();
    dispatchers_[ENUM_CMDPACKET_GUILD_CARGO] = new Dispatcher_GuildCargo();
    dispatchers_[ENUM_CMDPACKET_GUILD_CARGO_HISTORY] = new Dispatcher_GuildCargoHistory();
    dispatchers_[ENUM_CMDPACKET_FIGHT_VILLAGE_MONSTER] = new Dispatcher_FightVillageMonster();
    dispatchers_[ENUM_CMDPACKET_FINISH_VILLAGE_MONSTER_FIGHTING] = new Dispatcher_FinishVillageMonsterFighting();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_GUILD_CARGO] = new Dispatcher_UpgradeGuildCargo();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ITEM_LOCK] = new Dispatcher_RequestItemLock();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK] = new Dispatcher_RequestItemUnlock();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK_CANCEL] = new Dispatcher_RequestItemUnlockCancel();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_CHRONICLE] = new Dispatcher_UpgradeChronicle();
    dispatchers_[ENUM_CMDPACKET_ENCHANT_BY_BEAD] = new Dispatcher_EnchantByBead();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_NPC_BUFF_INFO] = new Dispatcher_DungeonNPCBuffInfo();
    dispatchers_[ENUM_CMDPACKET_LAG_STATISTICS] = new Dispatcher_Lag_Statistics();
    dispatchers_[ENUM_CMDPACKET_HT_PS] = new Dispatcher_Security_ProcessScan();
    dispatchers_[ENUM_CMDPACKET_HT_IS] = new Dispatcher_Security_IPScan();
    dispatchers_[ENUM_CMDPACKET_PI] = new Dispatcher_Security_PacketIntegrity();
    dispatchers_[ENUM_CMDPACKET_VERIFY_GOLD] = new Dispatcher_VerifyGold();
    dispatchers_[ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD] = new Dispatcher_RequestOnTimeEventReward();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ADD_PVP_BUDDY] = new Dispatcher_RequestAddPvPBuddy();
    dispatchers_[ENUM_CMDPACKET_RESPONSE_ADD_PVP_BUDDY] = new Dispatcher_ResponseAddPvPBuddy();
    dispatchers_[ENUM_CMDPACKET_REMOVE_PVP_BUDDY] = new Dispatcher_RemovePvPBuddy();
    dispatchers_[ENUM_CMDPACKET_PVP_BUDDY_CONN_LIST] = new Dispatcher_PvPBuddyConnList();
    dispatchers_[ENUM_CMDPACKET_NCC] = new Dispatcher_Security_NonClient_GetResponse();
    dispatchers_[ENUM_CMDPACKET_MI] = new Dispatcher_Security_MemoryIntegrity();
    dispatchers_[ENUM_CMDPACKET_CHANGE_CHARAC_SLOT] = new Dispatcher_Change_Charac_Slot();
    dispatchers_[ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM] = new Dispatcher_SecretShopBuyItem();
    dispatchers_[ENUM_CMDPACKET_SECRET_SHOP_OPEN_CLOSE] = new Dispatcher_SecretShopOpenClose();
    dispatchers_[ENUM_CMDPACKET_COMPLETE_LOAD_PVP] = new Dispatcher_CompleteLoadPvP();
    dispatchers_[ENUM_CMDPACKET_CONNECT_P2P_PVP] = new Dispatcher_ConnectP2PPvP();
    dispatchers_[ENUM_CMDPACKET_BIDDING_ROUTING_ITEM] = new Dispatcher_BiddingRoutingItem();
    dispatchers_[ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO] = new Dispatcher_CreateAccountCargo();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO] = new Dispatcher_UpgradeAccountCargo();
    dispatchers_[ENUM_CMDPACKET_DEPOSIT_MONEY] = new Dispatcher_DepositMoney();
    dispatchers_[ENUM_CMDPACKET_WITHDRAW_MONEY] = new Dispatcher_WithdrawMoney();
    dispatchers_[ENUM_CMDPACKET_REDEEM_LIST] = new Dispatcher_RedeemList();
    dispatchers_[ENUM_CMDPACKET_REDEEM] = new Dispatcher_Redeem();
    dispatchers_[ENUM_CMDPACKET_SECU_DATA_CONTROL] = new Dispatcher_SecuDataControl();
    dispatchers_[ENUM_CMDPACKET_CONNECT_LINK_CHARAC] =
        new Dispatcher_ConnectLinkCharacter();
    dispatchers_[ENUM_CMDPACKET_DISCONNECT_LINK_CHARAC] =
        new Dispatcher_DisconnectLinkCharacter();
    dispatchers_[ENUM_CMDPACKET_MULTI_MAILBOX_SEND] = new Dispatcher_MultiMailBoxSend();
    dispatchers_[ENUM_CMDPACKET_OPERATE_RIDABLE_OBJECT] = new Dispatcher_OperateRidableObject();
    dispatchers_[ENUM_CMDPACKET_SELECT_ULTIMATE_DIFFICULTY] = new Dispatcher_SelectUltimateDifficulty();
    dispatchers_[ENUM_CMDPACKET_PIV] = new Dispatcher_Security_PI_HashModulation_S2C();
    dispatchers_[ENUM_CMDPACKET_PID] = new Dispatcher_Security_PI_HashModulation_C2C();
    dispatchers_[ENUM_CMDPACKET_USE_CREATURE_EVOLUTION_ITEM] = new Dispatcher_UseCreatureEvolutionItem();
    dispatchers_[ENUM_CMDPACKET_QUERY_CHARAC_INFO_MAILBOX] = new Dispatcher_QueryCharacInfoMailbox();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE] = new Dispatcher_Compound_ExtreamItem();
    dispatchers_[ENUM_CMDPACKET_ONE_TO_ONE_CHAT_STATE] = new Dispatcer_polic_one_to_one_chat_disable();
    dispatchers_[ENUM_CMDPACKET_FIND_CHAR_NAME_USE_CHARAC_NO] = new Dispatcher_police_queryCharacName_useMid();
    dispatchers_[ENUM_CMDPACKET_SKILL_COMMAND_CUSTOMIZING] = new Dispatcher_Skill_Command_Customizing();
    dispatchers_[ENUM_CMDPACKET_SKILL_COMMAND_ALL_DEFAULT] = new Dispatcher_Skill_Command_All_Default();
    dispatchers_[ENUM_CMDPACKET_HACK_SCRIPT_HASH] = new Dispatcher_ValidateScriptHash();
    dispatchers_[ENUM_CMDPACKET_AUCTION_BUY_ITEM_APIECE] = new Dispatcher_AuctionBuyItemApiece();
    dispatchers_[ENUM_CMDPACKET_CHANGE_PARTY_MEMBER_POSITION] = new Dispatcher_Change_PartyMemberPosition();
    dispatchers_[ENUM_CMDPACKET_SCAN_BOT_BY_DLL] = new Dispatcher_ScanBotByDll();
    dispatchers_[ENUM_CMDPACKET_USE_LIMIT_CUBE] = new Dispatcher_UseLimitCube();
    dispatchers_[ENUM_CMDPACKET_REFRESH_GUILD_INFO] = new Dispatcher_RefreshGuildInfo();
    dispatchers_[ENUM_CMDPACKET_OPEN_GUILD_BOARD] = new Dispatcher_Open_Guild_Board();
    dispatchers_[ENUM_CMDPACKET_WRITE_ON_THE_GUILDBOARD] = new Dispatcher_Write_On_The_GuildBoard();
    dispatchers_[ENUM_CMDPACKET_DELETE_GUILDBOARD_TEXT] = new Dispatcher_Delete_GuildBoard_Text();
    dispatchers_[ENUM_CMDPACKET_DONATE_GUILD_FUND] = new Dispatcher_DonateGuildFund();
    dispatchers_[ENUM_CMDPACKET_CHECK_JOIN_GUILD] = new Dispatcher_CheckJoinGuild();
    dispatchers_[ENUM_CMDPACKET_REQUEST_JOIN_GUILD] = new Dispatcher_RequestJoinGuild();
    dispatchers_[ENUM_CMDPACKET_CANCEL_JOIN_GUILD] = new Dispatcher_CancelJoinGuild();
    dispatchers_[ENUM_CMDPACKET_APPROVE_JOIN_GUILD] = new Dispatcher_ApproveJoinGuild();
    dispatchers_[ENUM_CMDPACKET_DENY_JOIN_GUILD] = new Dispatcher_DenyJoinGuild();
    dispatchers_[ENUM_CMDPACKET_GUILD_JOIN_LIST] = new Dispatcher_GuildJoinList();
    dispatchers_[ENUM_CMDPACKET_GUILD_ATTENDANCE_INFO] = new Dispatcher_GuildAttendanceInfo();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_MOTION] = new Dispatcher_MotionHack();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_EQUIPMENT_UPGRADE_CARD] = new Dispatcher_Compound_Equipment_Upgrade_Card();
    dispatchers_[ENUM_CMDPACKET_BREAK_AWAY_QUEST_CHECK] = new Dispatcher_Break_Away_Quest_Check();
    dispatchers_[ENUM_CMDPACKET_JOIN_GUILD_INFO] = new Dispatcher_JoinGuildInfo();
    dispatchers_[ENUM_CMDPACKET_SCAN_BOT_BY_DRV] = new Dispatcher_ScanBotByDrv();
    dispatchers_[ENUM_CMDPACKET_ASK_REMATCH] = new Dispatcher_AskRematch();
    dispatchers_[ENUM_CMDPACKET_SAVE_GAME_OPTION_QUICKCHATTING] = new Dispatcher_SaveGameOption_QuickChatting();
    dispatchers_[ENUM_CMDPACKET_SELECT_3RD_CHRONICLE_ITEM_FOR_ENCHANT] = new Dispatcher_Select_3rdChronicleItem_ForEnchant();
    dispatchers_[ENUM_CMDPACKET_ENCHANT_3RD_CHRONICLE_ITEM] = new Dispatcher_Enchant_3rdChronicleItem();
    dispatchers_[ENUM_CMDPACKET_GOLD_TAKE_INCREASING_AMOUNT] = new Dispatcher_GoldTakeIncreasingAmount();
    dispatchers_[ENUM_CMDPACKET_CHECK_SECURITY_PROTECTION] = new DisPatcher_CheckSecurityProtection();
    dispatchers_[ENUM_CMDPACKET_EVENT_ACCOUNT_FATIGUE_STAT] =
        new Dispatcher_EventAccountFatigueStat();
    dispatchers_[ENUM_CMDPACKET_LEVELUP_SUPPORT_3RD_EVENT_GET_ITEM] =
        new Dispatcher_LevelupSupport3rdEventGetItem();
    dispatchers_[ENUM_CMDPACKET_INTEGRATE_MATCH_PVP_SCORE] = new Dispatcher_IntegrateMatchPvPScore();
    dispatchers_[ENUM_CMDPACKET_FAIR_PVP_SCORE] = new Dispatcher_FairPvPScore();
    dispatchers_[ENUM_CMDPACKET_PVP_MISSION_HP_PERCENT] = new Dispatcher_PvpMissionHPPercent();
    dispatchers_[ENUM_CMDPACKET_WARROOM_WP_PER_MONSTER] = new Dispatcher_WarRoomWpPerMonster();
    dispatchers_[ENUM_CMDPACKET_ABNORMAL_FUNCTION_CALL] = new Dispatcher_AbnormalFunctionCall();
    dispatchers_[ENUM_CMDPACKET_EQUIPSLOT_SWITCH] = new Dispatcher_Equipslot_Switch();
    dispatchers_[ENUM_CMDPACKET_EXPAND_EQUIPSLOT_FLAG_UPDATE] = new Dispatcher_Expand_Equipslot_Flag_Update();
    dispatchers_[ENUM_CMDPACKET_BUY_CHARAC_STATUS_USING_QP] = new Dispatcher_Buy_Charac_Status_Using_QP();
    dispatchers_[ENUM_CMDPACKET_CLEAR_USED_QP] = new Dispatcher_Clear_Used_QP();
    dispatchers_[ENUM_CMDPACKET_UNSEAL_RANDOM_OPTION] = new Dispatcher_UnsealRandomOption();
    dispatchers_[ENUM_CMDPACKET_CHANGE_HOST_WARROOM] = new Dispatcher_ChangeHostWarRoom();
    dispatchers_[ENUM_CMDPACKET_VERIFY_PRIVATE_STORE_ITEM] = new Dispatcher_VerifyPrivateStoreItem();
    dispatchers_[ENUM_CMDPACKET_SELECT_ITEM] = new Dispatcher_SelectItem();
    dispatchers_[ENUM_CMDPACKET_REGENERATION_RANDOM_OPTION] = new Dispather_RegenerationRandomOption();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_CARGO] = new Dispatcher_UpgradeCargo();
    dispatchers_[ENUM_CMDPACKET_PVP_MISSION_COMBO] = new Dispatcher_PvpMissionComboClear();
    dispatchers_[ENUM_CMDPACKET_TITLE_BOOK_PUT] = new Dispatcher_TitleBookPut();
    dispatchers_[ENUM_CMDPACKET_TITLE_BOOK_GET] = new Dispatcher_TitleBookGet();
    // The direct 140828 sender writes one catalyst u16 followed by a two-pass
    // card-slot loop; its response reader mirrors all three slots.
    dispatchers_[ENUM_CMDPACKET_MONSTERCARD_BIND] =
        new Dispatcher_MonstercardBind();
    dispatchers_[ENUM_CMDPACKET_CHARAC_SLOT_EXTEND_EFFECT] = new Dispatcher_CharacSlotExtendEffect();
    dispatchers_[ENUM_CMDPACKET_EXPERT_EXTRACTION] = new Dispatcher_ExpertExtraction();
    dispatchers_[ENUM_CMDPACKET_ACHIEVEMENT_TRIGGER] = new Dispatcher_AchievementTrigger();
    dispatchers_[ENUM_CMDPACKET_OTHER_USER_TITLE_BOOK_LIST] = new Dispatcher_TitleBookOtherUser();
    dispatchers_[ENUM_CMDPACKET_ITEM_HYPERLINK_MESSAGE] = new Dispatcher_ItemHyperLinkMessage();
    dispatchers_[ENUM_CMDPACKET_USER_HISTORY_LOG] = new Dispatcher_UserHistoryLog();
    dispatchers_[ENUM_CMDPACKET_REFUND_SKILL] = new Dispatcher_RefundSkill();
    dispatchers_[ENUM_CMDPACKET_ATTENDANCE_CHECK] = new Dispatcher_FatigueAttendance();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_INVENTORY] = new Dispatcher_UpgradeInventory();
    dispatchers_[ENUM_CMDPACKET_SELECT_ITEM_GROWTH_POWER] = new Dispatcher_Select_Item_Grwoth_Power();
    dispatchers_[ENUM_CMDPACKET_REQUEST_SERIA_BUFF] = new Dispatcher_Request_Seria_Buff();
    dispatchers_[ENUM_CMDPACKET_USE_CHESTNUT_STONE_ITEM] = new Dispatcher_UseChestnutStone();
    dispatchers_[ENUM_CMDPACKET_PARTY_TELEPORT] = new Dispatcher_PartyTeleport();
    dispatchers_[ENUM_CMDPACKET_PARTY_TELEPORT_CONFIRM] = new Dispatcher_PartyTeleportConfirm();
    dispatchers_[ENUM_CMDPACKET_ABNORMAL_USE_STACKABLE] = new Dispatcher_AbnormalUseStackable();
    dispatchers_[ENUM_CMDPACKET_CHANGE_RANDOM_OPTION] = new Dispatcher_ChangeRandomOption();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ITEM_SEPARATE] = new Dispatcher_UpgradeItemSeparate();
    dispatchers_[ENUM_CMDPACKET_ITEM_DICTIONARY] = new Dispatcher_ItemDictionary();
    dispatchers_[ENUM_CMDPACKET_MERCENARY_RETURN] = new Dispatcher_Mercenary_Return();
    dispatchers_[ENUM_CMDPACKET_MERCENARY_INFO] = new Dispatcher_Mercenary_Info();
    dispatchers_[ENUM_CMDPACKET_MERCENARY_COMPETITION] = new Dispatcher_Mercenary_Competition();
    dispatchers_[ENUM_CMDPACKET_REGISTER_QUICK_PARTY] = new Dispatcher_Register_Quick_Party();
    dispatchers_[ENUM_CMDPACKET_CANCEL_QUICK_PARTY] = new Dispatcher_Cancel_Quick_Party();
    dispatchers_[ENUM_CMDPACKET_DIRECT_ENTRANCE_DUNGEON_QUICK_PARTY] = new Dispatcher_Direct_Entrance_Quick_Party();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ASSAULT_PRICE] = new Dispatcher_RequestAssaultPrice();
    dispatchers_[ENUM_CMDPACKET_SAVE_CHARACTER_OPTION] = new Dispatcher_SaveCharacterOption();
    dispatchers_[ENUM_CMDPACKET_EXCHANGE_RANDOM_ITEM_REWARD] = new Dispatcher_Exchange_Random_Item_Reward();
    dispatchers_[ENUM_CMDPACKET_AVATAR_DISJOINT_RANDOM_REWARD] = new Dispatcher_Avatar_Disjoint_Random_Reward();
    dispatchers_[ENUM_CMDPACKET_CHECK_3RDPARTY_CONCENT] = new Dispatcher_Check3rdPartyConcent();
    dispatchers_[ENUM_CMDPACKET_FLOAT_RDATA_MODULATION] = new Dispatcher_Float_RDATA_Modulation();
    dispatchers_[ENUM_CMDPACKET_REQ_URGENT_QUEST] = new Dispatcher_Req_Urgent_Quest();
    dispatchers_[ENUM_CMDPACKET_RESET_RANDOM_OPTION] = new DispatcherResetRandomOption();
    dispatchers_[ENUM_CMDPACKET_CLEAR_QUEST] = new Dispatcher_Debug_Clear_Quest();
    dispatchers_[ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT_STATE] = new Dispatcher_TournamentRewardSelectState();
    dispatchers_[ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT] = new Dispatcher_TournamentRewardSelect();
    dispatchers_[ENUM_CMDPACKET_AVATAR_OPTION_CHANGE] = new Dispatcher_AvatarOptionChange();
    dispatchers_[ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM] = new Dispatcher_LimitNpcBuyItem();
    dispatchers_[ENUM_CMDPACKET_OBJECT_BRING_UP_USE_ITEM] = new Dispatcher_ObjectBringUp();
    dispatchers_[ENUM_CMDPACKET_PRECHECK_SOLO_TELEPOART] = new Dispatcher_PrecheckSoloTeleport();
    dispatchers_[ENUM_CMDPACKET_SOLO_TELEPOART] = new Dispatcher_SoloTeleport();
    dispatchers_[ENUM_CMDPACKET_SAVE_GAME_OPTION_CHATTING_EMOTICON] = new Dispatcher_SaveGameOption_ChattingEmoticon();
    dispatchers_[ENUM_CMDPACKET_REPORT_CLIENT_HACK] = new Dispatcher_ReportClientHack();
    dispatchers_[ENUM_CMDPACKET_IMAGE_COMMUNICATION_EQUIPMENT_USE] = new Dispatcher_ImageCommunicationEquipmentUse();
    dispatchers_[ENUM_CMDPACKET_COMPATIBILITY_INDEX] = new Dispatcher_CompatibilityIndex();
    dispatchers_[ENUM_CMDPACKET_INFORM_NOTICE] = new Dispatcher_InformNotice();
    dispatchers_[ENUM_CMDPACKET_P2P_STATISTICS] = new Dispatcher_P2P_Statistics();
    dispatchers_[ENUM_CMDPACKET_VERIFY_CREATURE_QUEST] = new Dispatcher_VerifyCreatureQuest();
    dispatchers_[ENUM_CMDPACKET_REPORT_PVP_LAG] = new Dispatcher_ReportPvPLag();
    dispatchers_[ENUM_CMDPACKET_VERIFY_PVP_LAG_USER] = new Dispatcher_VerifyPvPLagUser();
    dispatchers_[ENUM_CMDPACKET_COLLECT_ITEMS] = new Dispatcher_CollectItems();
    dispatchers_[ENUM_CMDPACKET_TUTORIAL_LEVEL_UP] = new Dispatcher_TutorialLevelUp();
    dispatchers_[ENUM_CMDPACKET_REQUEST_CHARAC_SKILL_INFO] = new Dispatcher_RequestCharacSkillInfo();
    dispatchers_[ENUM_CMDPACKET_CRANE_START_USE] = new Dispatcher_UseCraneStart();
    dispatchers_[ENUM_CMDPACKET_CRANE_PICKUP] = new Dispatcher_CranePickUp();
    dispatchers_[ENUM_CMDPACKET_SELECT_STRIKER] = new Dispatcher_SelectStriker();
    dispatchers_[ENUM_CMDPACKET_REQUEST_INGAME_ADVERTISEMENT] = new Dispatcher_RequestIngameAdvertisement();
    dispatchers_[ENUM_CMDPACKET_LOG_INGAME_ADVERTISEMENT] = new Dispatcher_LogIngameAdvertisement();
    dispatchers_[ENUM_CMDPACKET_SKILL_INIT] = new Dispatcher_SkillInit();
    dispatchers_[ENUM_CMDPACKET_PC_ROOM_PLAY_TIME_REWARD] = new Dispatcher_PcRoomPlayTimeReward();
    dispatchers_[ENUM_CMDPACKET_PC_ROOM_RENT_ITEM] = new Dispatcher_PcRoomRentItem();
    dispatchers_[ENUM_CMDPACKET_SERIAROOM_DECO_EVENT] = new Dispatcher_SeriaRoom_Deco_Event();
    dispatchers_[ENUM_CMDPACKET_BLUE_MARBLE] = new Dispatcher_BlueMarble();
    dispatchers_[ENUM_CMDPACKET_USE_DYE] = new Dispatcher_UseDye();
    dispatchers_[ENUM_CMDPACKET_USE_RIGHT_OF_CHANGE_GROW_TYPE] = new Dispatcher_UseRightOfChangeGrowType();
    dispatchers_[ENUM_CMDPACKET_INFORM_NOTICE_2ND] = new Dispatcher_InformNotice_2nd();
    dispatchers_[ENUM_CMDPACKET_GROWTH_WEAPON_CHANGE_INFINITY] = new Dispatcher_GrowthWeaponEventChangeInfinity();
    dispatchers_[ENUM_CMDPACKET_GROWTH_WEAPON_USE_MATERIAL] = new Dispatcher_GrowthWeaponEventUseMaterial();
    dispatchers_[ENUM_CMDPACKET_SAVE_QUEST_NOTIFY] = new Dispatcher_SaveQuestNotify();
    dispatchers_[ENUM_CMDPACKET_BLUE_MARBLE_CONFIRM_INFO] = new Dispatcher_BlueMarbleConfirmInfo();
    dispatchers_[ENUM_CMDPACKET_COMBO_SKILL_INFO] = new Dispatcher_ComboSkill();
    dispatchers_[ENUM_CMDPACKET_USE_RENAME_CARD] = new Dispatcher_UseRenameCard();
    dispatchers_[ENUM_CMDPACKET_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET] = new Dispatcher_ComboSkillExtensionQuickSlotReset();
    dispatchers_[ENUM_CMDPACKET_EQUIPED_CREATURE_CHANGE_INFINITY_CREATURE] = new Dispatcher_GrowthCreatureChangeInfinityCreature();
    dispatchers_[ENUM_CMDPACKET_SERIAROOM_ANIMATION_DECO_EVENT] = new Dispatcher_SeriaRoom_Ani_Deco_Event();
    dispatchers_[ENUM_CMDPACKET_BINGO_REWARD] = new Dispatcher_BingoReward();
    dispatchers_[ENUM_CMDPACKET_BINGO_QUIZ] = new Dispatcher_BingoQuiz();
    dispatchers_[ENUM_CMDPACKET_USE_STACKABLE_ACTION] = new Dispatcher_StackableActionUse();
    dispatchers_[ENUM_CMDPACKET_OPEN_CERAPACKAGE] = new Dispatcher_CeraPackageOpen();
    dispatchers_[ENUM_CMDPACKET_GET_ITEMBOX] = new DisPatcher_GetItemBox();
    dispatchers_[ENUM_CMDPACKET_SELECT_ZOMBIE] =
        new DisPatcher_SelectZombie();
    dispatchers_[ENUM_CMDPACKET_TO_BE_ZOMBIE] =
        new Dispatcher_ToBeZombie();
    dispatchers_[ENUM_CMDPACKET_ZOMBIE_MODE_RESULT_SCORE] =
        new Dispatcher_ZombieModeResultScore();
    dispatchers_[ENUM_CMDPACKET_MONSTER_MOVE_SYSTEM] =
        new Dispatcher_MonsterMoveSystem();
    dispatchers_[ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING] =
        new Dispatcher_RequestIntegratedMatching();
    dispatchers_[ENUM_CMDPACKET_CANCEL_INTEGRATED_MATCHING] =
        new Dispatcher_CancelIntegratedMatching();
    dispatchers_[ENUM_CMDPACKET_INTEGRATE_MATCHING_DIRECT_COMPLETE] =
        new Dispatcher_IntegrateMatchingDirectComplete();
    dispatchers_[ENUM_CMDPACKET_INTEGRATED_MATCHING_MODE_CHANGE] =
        new Dispatcher_IntegratedMatchingModeChange();
    dispatchers_[ENUM_CMDPACKET_INTEGRATE_MATCHING_USER_COUNT] =
        new Dispatcher_IntegrateMatchingUserCount();
    dispatchers_[ENUM_CMDPACKET_DUAL_RAID_DUNGEON_JOIN] =
        new Dispatcher_DualRaidJoin();
    dispatchers_[ENUM_CMDPACKET_DUAL_RAID_DUNGEON] =
        new Dispatcher_DualRaidCommand();
    dispatchers_[ENUM_CMDPACKET_UPDATE_CONTRACT_OF_CUBE_INFO] = new Dispatcher_UpdateContractOfCubeInfo();
    dispatchers_[ENUM_CMDPACKET_DUAL_RAID_DUNGEON_READY] =
        new Dispatcher_DualRaidReady();
    dispatchers_[ENUM_CMDPACKET_DUAL_RAID_DUNGEON_REWARD] =
        new Dispatcher_DualRaidReward();
    dispatchers_[ENUM_CMDPACKET_RACING_DUNGEON_JOIN] =
        new Dispatcher_RacingDungeonJoin();
    dispatchers_[ENUM_CMDPACKET_RACING_DUNGEON_DISJOIN] =
        new Dispatcher_RacingDungeonDisjoin();
    dispatchers_[ENUM_CMDPACKET_RACING_DUNGEON_GOAL_IN_PLAYER] =
        new Dispatcher_RacingDungeonGoal();
    dispatchers_[ENUM_CMDPACKET_RACING_DUNGEON_RETURN_TO_VILLAGE] =
        new Dispatcher_RacingDungeonReturn();
    dispatchers_[ENUM_CMDPACKET_LOADING_CART] =
        new Dispatcher_LoadingCart();
    dispatchers_[ENUM_CMDPACKET_GOLDEN_COMMERCE_START] =
        new Dispatcher_GoldenCommerceStart();
    dispatchers_[ENUM_CMDPACKET_GOLDEN_COMMERCE_REWARD] =
        new Dispatcher_GoldenCommerceReward();
    dispatchers_[ENUM_CMDPACKET_REQUEST_FREELY_GIVE_ITEM_BOX] =
        new Dispatcher_RequestFreelyGiveItemBox();
    dispatchers_[ENUM_CMDPACKET_REQUEST_COUPON_CHANGE] =
        new Dispatcher_RequestCouponChange();
    dispatchers_[ENUM_CMDPACKET_USE_FREE_GIVE_ITEM_COUPON] =
        new Dispatcher_UseFreelyGiveItemCoupon();
    dispatchers_[ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM] =
        new Dispatcher_ChangePeriodicToUnlimited();
    dispatchers_[ENUM_CMDPACKET_INIT_FREE_GIVE_ITEM_EVENT] =
        new Dispatcher_InitFreelyGiveItemEvent();
    dispatchers_[ENUM_CMDPACKET_CREATE_UPGRADE_ROOM] =
        new Dispatcher_CreateUpgradeRoom();
    dispatchers_[ENUM_CMDPACKET_DESTROY_UPGRADE_ROOM] =
        new Dispatcher_DestroyUpgradeRoom();
    dispatchers_[ENUM_CMDPACKET_JOIN_SPECTATOR_UPGRADE_ROOM] =
        new Dispatcher_JoinSpectatorUpgradeRoom();
    dispatchers_[ENUM_CMDPACKET_LEAVE_SPECTATOR_UPGRADE_ROOM] =
        new Dispatcher_LeaveSpectatorUpgradeRoom();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ROOM_GIVE_ITEM_INFO] =
        new Dispatcher_UpgradeRoomGiveItemInfo();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ROOM_PUT_UP_ITEM] =
        new Dispatcher_UpgradeRoomPutUpItem();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ROOM_UPGRADE_START] =
        new Dispatcher_UpgradeRoomUpgradeStart();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ROOM_UPGRADE_CANCEL] =
        new Dispatcher_UpgradeRoomUpgradeCancel();
    dispatchers_[ENUM_CMDPACKET_UPGRADE_ROOM_MASTER_MESSAGE] =
        new Dispatcher_UpgradeRoomMasterMessage();
    dispatchers_[ENUM_CMDPACKET_ANTIBOT_DELAY_LOG] = new Dispatcher_SecurityLog();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_MISSION_START] =
        new Dispatcher_DungeonMissionStart();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_MISSION_UPDATE] =
        new Dispatcher_DungeonMissionUpdate();
    dispatchers_[ENUM_CMDPACKET_DUNGEON_MISSION_CHECK_SUCCESS] =
        new Dispatcher_DungeonMissionCheckSuccess();
    dispatchers_[ENUM_CMDPACKET_PVP_TOURNAMENT_MATCH_LIST] =
        new Dispatcher_PvpTournamentMatchList();
    dispatchers_[ENUM_CMDPACKET_PVP_TOURNAMENT_REQUEST] =
        new Dispatcher_PvpTournamentRequest();
    dispatchers_[ENUM_CMDPACKET_REQUEST_INSTANT_REINFORCE] =
        new Dispatcher_RequestInstantReinforce();
    if (time_spiral::HasNativeClosure()) {
        dispatchers_[ENUM_CMDPACKET_BREAK_TRAP_RESULT] =
            new Dispatcher_BreakTrapResult();
    }
    if (blue_horse_event::HasNativeBlueHorseClosure()) {
        dispatchers_[ENUM_CMDPACKET_PLAY_BLUE_HORSE_SLOT] =
            new Dispatcher_PlayBlueHorseSlot();
        dispatchers_[ENUM_CMDPACKET_REWARD_BLUE_HORSE_SLOT] =
            new Dispatcher_RewardBlueHorseSlot();
    }
    dispatchers_[ENUM_CMDPACKET_EVENT_DUNGEON_DESTORY_OBJECT] = new DisPatcher_EventDungeon_DestoryObject();
    dispatchers_[ENUM_CMDPACKET_EVENT_DUNGEON_CLEAR_ROOM] = new DisPatcher_EventDungeon_ClearRoom();
    dispatchers_[ENUM_CMDPACKET_ACTION_POINT_ACTION_CLEAR] = new APSystem::Dispatcher_ClearAction();
    dispatchers_[ENUM_CMDPACKET_ACTION_POINT_GET_REWARD_ITEM] = new APSystem::Dispatcher_GetTodayReward();
    dispatchers_[ENUM_CMDPACKET_COMPOUND_CREATURE] = new Dispatcher_CompoundCreature();
    dispatchers_[ENUM_CMDPACKET_CREATURE_FILLPOINT] = new Dispatcher_CreatureFillPoint();
    dispatchers_[ENUM_CMDPACKET_KIRI_CARGO_PURCHASE] =
        new Dispatcher_KiriCargoPurchase();
    dispatchers_[ENUM_CMDPACKET_KIRI_CARGO_GET_BONUS] =
        new Dispatcher_KiriCargoGetBonus();
    dispatchers_[ENUM_CMDPACKET_CARGO_TRANSPORT_ITEM] = new Dispatcher_CargoTransportItem();
    dispatchers_[ENUM_CMDPACKET_FATIGUE_ACCELERATION_STATE_CHANGE] =
        new Dispatcher_FatigueAccelerationStateChange();
    dispatchers_[ENUM_CMDPACKET_SET_CLONE_TITLE] = new Dispatcher_SetCloneTitle();
    dispatchers_[ENUM_CMDPACKET_DRAGON_DUNGEON_TIMEATTACK_RANKER_BOARD] =
        new Dispatcher_DragonDungeonTimeAttackRankerBoard();
    dispatchers_[ENUM_CMDPACKET_SURVEY_CONTENTS] =
        new Dispatcher_SatisfactionSurvey();
    dispatchers_[ENUM_CMDPACKET_UPDATE_BEST_DAMAGE_RANK] =
        new Dispatcher_UpdateBestDamageRank();
    dispatchers_[ENUM_CMDPACKET_REQUEST_BEST_DAMAGE_RANK] =
        new Dispatcher_RequestBestDamageRank();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_START_GAME] = new advancealtar::Dispatcher_StartGame();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_BUY_ITEM] = new advancealtar::Dispatcher_BuyItem();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_SET_SLOT] = new advancealtar::Dispatcher_SetSlot();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_UPGRADE_GAGE] = new advancealtar::Dispatcher_UpgradeGage();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_SUMMON_UNIT] = new advancealtar::Dispatcher_SummonUnit();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_EXCHANGE_SLOT] = new advancealtar::Dispatcher_ExchangeSlot();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_PAUSE] = new advancealtar::Dispatcher_Pause();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_GET_ACHIEVEMENT_REWARD] = new advancealtar::Dispatcher_GetAchievementReward();
    dispatchers_[ENUM_CMDPACKET_ADVANCE_ALTAR_RESET_STAR] = new advancealtar::Dispatcher_ResetStar();
    dispatchers_[ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET] = new ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter();
    dispatchers_[ENUM_CMDPACKET_USE_AVATAR_ROULETTE] = new ARAD::DISPATCHER::Dispatcher_AvartarRoulette();
    dispatchers_[ENUM_CMDPACKET_AVATAR_COIN_COUNT] = new ARAD::DISPATCHER::Dispatcher_AvartarCoinCount();
    dispatchers_[ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE] = new ARAD::DISPATCHER::Dispatcher_AvartarHiddenOptionChange();
    dispatchers_[ENUM_CMDPACKET_USE_AVATAR_RECHARGE_JPN] = new ARAD::DISPATCHER::Dispatcher_AradUseAvatarRecharge();
    dispatchers_[ENUM_CMDPACKET_EMBLEM_COMPOUND_JPN] = new ARAD::DISPATCHER::Dispatcher_AradEmblemCompound();
    dispatchers_[ENUM_CMDPACKET_AVATAR_CONVERT_JPN] = new ARAD::DISPATCHER::Dispatcher_AradAvatarConvert();
    dispatchers_[ENUM_CMDPACKET_LEVELUP_SUPPORT_REQ_ITEM] = new Dispatcher_LevelupSupportReqestItem();
    dispatchers_[ENUM_CMDPACKET_REQUEST_EVENT_SERVER_LEVEL_UP] = new Dispatcher_RequestEventServerLevelUp();
    dispatchers_[ENUM_CMDPACKET_P2P_HOLE_PUNCHING_SUCCESS_RATE] = new Dispatcher_P2PHolePunchingSuccessRateStatistics();
    dispatchers_[ENUM_CMDPACKET_LETS_PICK_PRESENT] =
        new Dispatcher_LetsPickPresent();
    dispatchers_[ENUM_CMDPACKET_REQUEST_ADD_PICK_CHANCE] =
        new Dispatcher_RequestAddPickChance();
    dispatchers_[ENUM_CMDPACKET_REQUEST_COMBO_SCORE] =
        new Dispatcher_RequestComboScore();
    dispatchers_[ENUM_CMDPACKET_COMBO_SCORE_INFO] =
        new Dispatcher_ComboScoreInfo();
    dispatchers_[ENUM_CMDPACKET_ADD_RAINBOW_POINT] = new Dispatcher_AddRainbowPoint();
    dispatchers_[ENUM_CMDPACKET_REQUEST_RAINBOW_POINT] = new Dispatcher_RequestRainbowPoint();
    dispatchers_[ENUM_CMDPACKET_REQUEST_RAINBOW_POINT_REWARD] = new Dispatcher_RequestRainbowPointReward();
    dispatchers_[ENUM_CMDPACKET_SECURITY_STATUS] = new Dispatcher_SecurityStatus();
    dispatchers_[ENUM_CMDPACKET_EVENT_NPC_DROP_ITEM_] =
        new Dispatcher_EventNpcDropItem();
    dispatchers_[ENUM_CMDPACKET_TAKE_MAGIC_LAMP] =
        new Dispatcher_TakeMagicLamp();
    dispatchers_[ENUM_CMDPACKET_EXCHANGE_ITEM_FROM_NPC] =
        new Dispatcher_ExchangeItemFromNpc();
    dispatchers_[ENUM_CMDPACKET_FACEBOOK_LIKE] =
        new Dispatcher_FacebookLike();
    if (glass_bottle_event::HasNativeGlassBottleClosure()) {
        dispatchers_[ENUM_CMDPACKET_SEND_GLASS_BOTTLE_LETTER] =
            new Dispatcher_SendGlassBottleLetter();
    }
    if (shooting_star_dimension_pocket::HasNativeClosure()) {
        dispatchers_[ENUM_CMDPACKET_SHOOTINGSTAR_BUY_ITEM] =
            new Dispatcher_ShootingStarBuyItem();
        dispatchers_[ENUM_CMDPACKET_DIMENSION_POCKET_ITEM] =
            new Dispatcher_DimensionPocketItem();
    }
    if (sea_bottom_event::HasNativeClosure()) {
        dispatchers_[ENUM_CMDPACKET_THE_SEA_BOTTOM2000_USE] =
            new Dispatcher_TheSeaBottom2000Use();
    }
    dispatchers_[ENUM_CMDPACKET_REQUEST_EXP_KEEPING_EVENT_REWARD] =
        new Dispatcher_RequestExpKeepingEventReward();
    dispatchers_[ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM] =
        new Dispatcher_SecretEventShopBuyItem();
    dispatchers_[ENUM_CMDPACKET_EVENT_ATTENDANCE_REWARD] =
        new Dispatcher_EventAttendanceReward();
    dispatchers_[ENUM_CMDPACKET_DECIDE_LEVELUP] = new Dispatcher_DecideLevelUp();
    dispatchers_[ENUM_CMDPACKET_USE_TITLE_CHANGE_ITEM] = new Dispatcher_UseTitleChangeItem();
    dispatchers_[ENUM_CMDPACKET_SELECT_COLLECTBOX] = new Dispatcher_SelectCollectBox();
    dispatchers_[ENUM_CMDPACKET_ADD_COLLECTBOX_ITEM] = new Dispatcher_AddCollectBoxItem();
    dispatchers_[ENUM_CMDPACKET_REMOVE_COLLECTBOX_ITEM] = new Dispatcher_RemoveCollectBoxItem();
    dispatchers_[ENUM_CMDPACKET_EXTEND_COLLECTBOX_EXPIRY_DATE] = new Dispatcher_ExtendCollectBoxExpiryDate();
    dispatchers_[ENUM_CMDPACKET_EVENT_CREATE_DNF_REQUEST] = new ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest();
    dispatchers_[ENUM_CMDPACKET_REQUEST_PCROOM_DAYILY_REWARD] = new ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward();
    dispatchers_[ENUM_CMDPACKET_HERO_MISSION_DATA_REWARD] = new Dispatcher_HeroMissionEventState_Reward();
    dispatchers_[ENUM_CMDPACKET_ANTIBOT] = new Dispatcher_Antibot();
    dispatchers_[ENUM_CMDPACKET_DPROTO] = new Dispatcher_Antibot_DP();
    dispatchers_[ENUM_CMDPACKET_DPROTO_CALLBACK] = new Dispatcher_Antibot_DPCallBack();
    // END GENERATED 0828 DIRECT DISPATCHER REGISTRATIONS
}

// IDA_NAME: _ZN16PacketDispatcher19pUserPacketCounter_E
message_counter::CUserPacketCounter* PacketDispatcher::pUserPacketCounter_ = 0;

// IDA_EA: 0x8594840
// IDA_NAME: _ZN16PacketDispatcherD2Ev
// DEMANGLED: PacketDispatcher::~PacketDispatcher()
PacketDispatcher::~PacketDispatcher() {
    // IDispatcher's deleting destructor is vtable slot +8 in the original
    // i386 ABI; a typed delete emits that same virtual D0 path.
    if (dispatchers_[ENUM_CMDPACKET_MATCHING_DUNGEON_EXIT] ==
        g_channel_zombie_exit_dispatcher)
        g_channel_zombie_exit_dispatcher = 0;
    if (dispatchers_[ENUM_CMDPACKET_CHANNEL_MOVE_SUCCESS] ==
        g_channel_zombie_success_dispatcher)
        g_channel_zombie_success_dispatcher = 0;
    for (int index = 0; index < ENUM_CMDPACKET_END; ++index) {
        delete dispatchers_[index];
        dispatchers_[index] = 0;
    }
}

bool PacketDispatcher::ConfigureChannelZombieMatchingDispatchers() {
    if (!channel_zombie_matching_producer::CanRegisterPackets()) return false;
    IDispatcher*& exit_dispatcher =
        dispatchers_[ENUM_CMDPACKET_MATCHING_DUNGEON_EXIT];
    IDispatcher*& success_dispatcher =
        dispatchers_[ENUM_CMDPACKET_CHANNEL_MOVE_SUCCESS];
    if (exit_dispatcher || success_dispatcher) {
        return exit_dispatcher == g_channel_zombie_exit_dispatcher &&
               success_dispatcher == g_channel_zombie_success_dispatcher;
    }

    Dispatcher_MatchingDungeonExit* const exit_owner =
        new Dispatcher_MatchingDungeonExit();
    Dispatcher_ChannelMoveSuccess* const success_owner =
        new Dispatcher_ChannelMoveSuccess();
    exit_dispatcher = exit_owner;
    success_dispatcher = success_owner;
    g_channel_zombie_exit_dispatcher = exit_owner;
    g_channel_zombie_success_dispatcher = success_owner;
    return true;
}

// IDA_EA: 0x8231F2E
// IDA_NAME: _ZN16PacketDispatcher13getDispatcherEi
// DEMANGLED: PacketDispatcher::getDispatcher(int)
IDispatcher* PacketDispatcher::getDispatcher(int packet_id) {
    return get_dispatcher(packet_id);
}

// IDA_EA: 0x85948E2
// IDA_NAME: _ZN16PacketDispatcher14get_dispatcherEi
// DEMANGLED: PacketDispatcher::get_dispatcher(int)
IDispatcher* PacketDispatcher::get_dispatcher(int packet_id) {
    const oldsrv::protocol::ClientProtocolProfile& profile =
        oldsrv::protocol::ActiveClientProtocolProfile();
    if (packet_id < 0 || packet_id >= profile.wireCommandCount()) {
        return 0;
    }
    return dispatchers_[packet_id];
}

// IDA_EA: 0x825F658
// IDA_NAME: _ZN16PacketDispatcher22server_load_monitoringEv
// DEMANGLED: PacketDispatcher::server_load_monitoring(void)
int PacketDispatcher::server_load_monitoring() {
    GlobalData::s_msgq_mgr->remain(MsgQueueMgr::TIMER_Q);
    const int dispatch_queue =
        GlobalData::s_msgq_mgr->remain(MsgQueueMgr::DISPATCH_Q);
    const int network_queue =
        GlobalData::s_msgq_mgr->remain(MsgQueueMgr::NETWORK_Q);
    const int database_queue =
        GlobalData::s_msgq_mgr->remain(MsgQueueMgr::DB_Q);
    const int log_database_queue =
        GlobalData::s_msgq_mgr->remain(MsgQueueMgr::LOGDB_Q);

    CAppLoadChecker* const checker = CAppLoadCheckerInstance();
    const int should_request = checker->CheckLoad(database_queue,
                                                  log_database_queue,
                                                  network_queue,
                                                  dispatch_queue);
    if (static_cast<unsigned char>(should_request) != 0) {
        CAppLoadCheckerInstance()->RequestDB(database_queue,
                                             log_database_queue,
                                             network_queue,
                                             dispatch_queue);
    }
    return should_request;
}

// IDA_EA: 0x825F72E
// IDA_NAME: _ZN16PacketDispatcher15log_recv_packetEP5CUser16ENUM_PACKETCLASStPci
// DEMANGLED: PacketDispatcher::log_recv_packet(CUser *,ENUM_PACKETCLASS,unsigned short,char *,int)
int PacketDispatcher::log_recv_packet(CUser* user,
                                      ENUM_PACKETCLASS packet_class,
                                      uint16_t packet_id,
                                      char* packet,
                                      int packet_len) {
    (void)user;
    (void)packet;
    (void)packet_len;
    return reinterpret_cast<intptr_t>(
        CPacketTracerInstance()->AddTcpLog(packet_class, packet_id));
}

// IDA_EA: 0x8594922
// IDA_NAME: _ZN16PacketDispatcher10doDispatchEP5CUser16ENUM_PACKETCLASStPciS2_t
// DEMANGLED: PacketDispatcher::doDispatch(CUser *,ENUM_PACKETCLASS,unsigned short,char *,int,ENUM_PACKETCLASS,unsigned short)
int PacketDispatcher::doDispatch(CUser* user,
                                 ENUM_PACKETCLASS packet_class,
                                 uint16_t packet_id,
                                 char* packet_bytes,
                                 int packet_len,
                                 ENUM_PACKETCLASS source_class,
                                 uint16_t source_packet_id) {
    log_recv_packet(user, packet_class, packet_id, packet_bytes, packet_len);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1,
            "PacketDispatcher.cpp",
            "PacketDispatcher::doDispatch",
            0,
            "[CHARVIEW][SELECT_DODISPATCH_ENTER] state=%d class=%d size=%d source_class=%d source_id=%d",
            user->get_state(),
            static_cast<int>(packet_class),
            packet_len,
            static_cast<int>(source_class),
            static_cast<int>(source_packet_id));
    }

    // The 140828 client reuses command 587 for its security-status heartbeat,
    // while the 0627 server table owns 587 as avatar-coin state.  The proven
    // Release/df_game_r.c compatibility hook consumes this command before the
    // legacy dispatcher, without interpreting the encrypted block padding as
    // business fields.  Keep that boundary here as a protocol-version bridge.
    const oldsrv::protocol::ClientProtocolProfile& protocol_profile =
        oldsrv::protocol::ActiveClientProtocolProfile();
    if (protocol_profile.isServerConsumedCommand(packet_class, packet_id)) {
        if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
            LogManager::logFormat(
                1, "PacketDispatcher.cpp", "PacketDispatcher::doDispatch", 0,
                "[CHARVIEW][SELECT_SERVER_CONSUMED] result=1");
        }
        return 1;
    }

    // Login diagnostics intentionally identify only the protocol stage and
    // lengths/state: token and packet bytes must never enter server logs.
    if (packet_id == 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "int PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, unsigned short, char*, int, ENUM_PACKETCLASS, unsigned short)",
            0, "[LOGIN_FLOW][CMD1_HANDLER] state=%d class=%d size=%d",
            user->get_state(), static_cast<int>(packet_class), packet_len);
    }

    const int in_uplist = AntibotCheckerIsCmdInUpList(
        GlobalData::antibot_checker(), user->getAntibotKey(), packet_id);
    if (in_uplist != 0 && source_packet_id != ENUM_CMDPACKET_DPROTO) {
        LogManager::logFormat(1,
                              "PacketDispatcher.cpp",
                              "bool PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, short unsigned int, char*, int, ENUM_PACKETCLASS, short unsigned int)",
                              0xA06,
                              "m_id:%s, cmd:%d",
                              NumberToString(
                                  static_cast<unsigned int>(user->get_acc_id()),
                                  0),
                              packet_id);
    }

    IDispatcher* const dispatcher = get_dispatcher(packet_id);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        const int vptr_present =
            dispatcher && *reinterpret_cast<void**>(dispatcher) ? 1 : 0;
        LogManager::logFormat(
            1,
            "PacketDispatcher.cpp",
            "PacketDispatcher::doDispatch",
            0,
            "[CHARVIEW][SELECT_HANDLER] present=%d vptr=%d state=%d",
            dispatcher ? 1 : 0,
            vptr_present,
            user->get_state());
    }
    if (!dispatcher) {
        return 0;
    }

    server_load_monitoring();
    user->backupSeed();

    const CS_Header* const header =
        reinterpret_cast<const CS_Header*>(packet_bytes);
    uint32_t server_checksum = 0;
    uint32_t client_checksum = 0;
    bool checksum_ok = true;
    if (packet_len > static_cast<int>(sizeof(CS_Header)) &&
        source_packet_id == ENUM_CMDPACKET_DPROTO) {
        server_checksum = Encryption::crc32N(
            packet_bytes + offsetof(CS_Header, seq),
            packet_len - static_cast<int>(offsetof(CS_Header, seq)));
        client_checksum = header->checksum;
        _NS_PI_2ND_MakeChecksumTo1Byte2(server_checksum, client_checksum);
        checksum_ok = client_checksum == server_checksum;
    }

    if (!checksum_ok) {
        LogManager::logFormat(1,
                              "PacketDispatcher.cpp",
                              "bool PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, short unsigned int, char*, int, ENUM_PACKETCLASS, short unsigned int)",
                              0xA37,
                              "Check Sum Error: server(%d), client(%d), mid(%s)",
                              server_checksum,
                              client_checksum,
                              NumberToString(
                                  static_cast<unsigned int>(user->get_acc_id()),
                                  0));
        user->DisConnSig(static_cast<DISCONN_SIG>(13), true, 0);
        return 0;
    }

    PacketBuf packet;
    packet.bind_packet(packet_bytes, packet_len);
    user->setLastPacketID(packet_id);

    const GameWorld* const world = G_GameWorld();
    const bool restricted_world =
        world->GetChannelType() == 7 || world->IsIntegratedPvPBaseChannel();
    if (restricted_world && DenyPacket(packet_id)) {
        if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
            LogManager::logFormat(
                1, "PacketDispatcher.cpp", "PacketDispatcher::doDispatch", 0,
                "[CHARVIEW][SELECT_RESTRICTED_DENY] result=1");
        }
        return 1;
    }

    int begin_tick = 0;
    if (packet_id == ENUM_CMDPACKET_ANTIBOT) {
        begin_tick = GlobalData::s_systemTime_.getCurTickCount();
    }

    const int policy_result =
        local_china_error_dispatcher_.doErrorDispatcher(user, packet_id);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1, "PacketDispatcher.cpp", "PacketDispatcher::doDispatch", 0,
            "[CHARVIEW][SELECT_POLICY] result=%d", policy_result);
    }
    if (policy_result != 0) {
        if (policy_result == 1) {
            LogManager::logFormat(1,
                                  "PacketDispatcher.cpp",
                                  "bool PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, short unsigned int, char*, int, ENUM_PACKETCLASS, short unsigned int)",
                                  0xA60,
                                  "[doErrorDispatcher] ERROR_NOT_TING - code : %d, m_id :%s",
                                  packet_id,
                                  NumberToString(
                                      static_cast<unsigned int>(user->get_acc_id()),
                                      0));
        } else if (policy_result == 2) {
            LogManager::logFormat(1,
                                  "PacketDispatcher.cpp",
                                  "bool PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, short unsigned int, char*, int, ENUM_PACKETCLASS, short unsigned int)",
                                  0xA5B,
                                  "[doErrorDispatcher] ERROR_TING - code : %d, m_id :%s",
                                  packet_id,
                                  NumberToString(
                                      static_cast<unsigned int>(user->get_acc_id()),
                                      0));
            user->DisConnSig(static_cast<DISCONN_SIG>(10), true, 0);
        }
        return 0;
    }

    // In the old table this is the IDispatcher vtable's first entry.  Calling
    // dispatch_sig directly would skip state/template handling for most
    // concrete packet classes.
    const int dispatch_result = dispatcher->dispatch_template(user, packet);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1,
            "PacketDispatcher.cpp",
            "PacketDispatcher::doDispatch",
            0,
            "[CHARVIEW][SELECT_TEMPLATE_DONE] result=%d state=%d",
            dispatch_result,
            user->get_state());
    }
    if (packet_id == 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "int PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, unsigned short, char*, int, ENUM_PACKETCLASS, unsigned short)",
            0, "[LOGIN_FLOW][CMD1_HANDLER_DONE] result=%d state=%d",
            dispatch_result, user->get_state());
    }
    if (dispatch_result != 0) {
        user->DisConnSig(static_cast<DISCONN_SIG>(10), true, dispatch_result);
        return 0;
    }

    if (packet_id == ENUM_CMDPACKET_ANTIBOT) {
        GlobalData::s_systemTime_.update();
        const int elapsed = GlobalData::s_systemTime_.getCurTickCount() -
                            begin_tick;
        if (static_cast<unsigned int>(elapsed) > 2000u) {
            LogManager::logFormat(1,
                                  "PacketDispatcher.cpp",
                                  "bool PacketDispatcher::doDispatch(CUser*, ENUM_PACKETCLASS, short unsigned int, char*, int, ENUM_PACKETCLASS, short unsigned int)",
                                  0xA7A,
                                  "Delay notice, OnRecvBufferFromClient dispatch time : %u",
                                  elapsed);
        }
    }
    if (packet_id != ENUM_CMDPACKET_ANTIBOT) {
        _NS_PI_Check_CMD(source_class, packet_id, packet, user);
    }
    return 1;
}

// IDA_EA: 0x8594E52
// IDA_NAME: _ZN16PacketDispatcher8dispatchEiPci
// DEMANGLED: PacketDispatcher::dispatch(int,char *,int)
int PacketDispatcher::dispatch(int user_index, char* packet, int packet_len) {
    const CS_Header* const c2s_header =
        reinterpret_cast<const CS_Header*>(packet);
    const ENUM_PACKETCLASS packet_class =
        static_cast<ENUM_PACKETCLASS>(c2s_header->cmd);
    const uint16_t packet_id = c2s_header->type;
    if (packet_class == static_cast<ENUM_PACKETCLASS>(1) && packet_id == 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp", "PacketDispatcher::dispatch", 0,
            "[DIAG0627][PACKET_DISPATCH_ENTER] uid=%d size=%d",
            user_index, packet_len);
    }
    CUser* const user = G_CGameManager()->getUser(user_index, 0);
    if (packet_class == static_cast<ENUM_PACKETCLASS>(1) &&
        packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1,
            "PacketDispatcher.cpp",
            "PacketDispatcher::dispatch",
            0,
            "[CHARVIEW][SELECT_DISPATCH] size=%d user_found=%d",
            packet_len,
            user ? 1 : 0);
    }
    if (packet_class == static_cast<ENUM_PACKETCLASS>(1) && packet_id == 1) {
        LogManager::logFormat(
            1, "login_flow_diag.cpp", "PacketDispatcher::dispatch", 0,
            "[DIAG0627][PACKET_USER_LOOKUP] uid=%d found=%d",
            user_index, user != 0 ? 1 : 0);
    }
    if (!user) {
        return 0;
    }

    // DispatchThread establishes this stack-owned counter before any worker
    // consumes DISPATCH_Q; preserve the old direct accounting call.
    pUserPacketCounter_->IncreasePacket(
        static_cast<uint32_t>(user->get_acc_id()));
    // IDA 0x8594EEC resolves checker vtable +0x3C here.  The +0x30
    // IsCmdInUpList query belongs to doDispatch and is diagnostic-only;
    // using it at this outer gate rejects ordinary login CMD1 packets.
    const int anti_cheat_result =
        AntibotCheckerAntiCheatPacket(GlobalData::antibot_checker(),
                                      user->getAntibotKey(),
                                      packet_id);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1, "PacketDispatcher.cpp", "PacketDispatcher::dispatch", 0,
            "[CHARVIEW][SELECT_ANTICHEAT] result=%d state=%d",
            anti_cheat_result,
            user->get_state());
    }
    if (anti_cheat_result) {
        user->history_log().AbnormalUser(
            static_cast<ENUM_ABNORMAL_USER_LOG_TYPE>(0),
            packet_id,
            user->getAntibotKey(),
            0);
        return 0;
    }

    const int dispatch_result = doDispatch(user,
                                           packet_class,
                                           packet_id,
                                           packet,
                                           packet_len,
                                           static_cast<ENUM_PACKETCLASS>(0),
                                           0);
    if (packet_id == ENUM_CMDPACKET_SELECT_CHARACTER) {
        LogManager::logFormat(
            1, "PacketDispatcher.cpp", "PacketDispatcher::dispatch", 0,
            "[CHARVIEW][SELECT_DISPATCH_DONE] result=%d state=%d",
            dispatch_result,
            user->get_state());
    }
    if (dispatch_result != 1) {
        return 0;
    }

    if (user->getCurCharacR() && user->IsCurCharacGhost() &&
        user->IsCurCharacLeaveTimeout()) {
        user->LiveCurCharac();
        pvp_assault::CAssaultPlace::SendUserState(user);
    }
    return 1;
}
