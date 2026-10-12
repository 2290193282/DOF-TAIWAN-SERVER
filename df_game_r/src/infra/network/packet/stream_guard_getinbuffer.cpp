#include "infra/network/packet/stream_guard.h"

#include "old_game/activity/reward/advance_altar_statistics.h"
#include "old_data/Login/inter_returns/inter_select_mobile_auth_reward.h"
#include "old_data/Login/inter_returns/auto_punish_inter_receivers.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_data/channel/channel_db_payloads.h"
#include "old_data/channel/log_game_channel.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/legacy_dispatch/db_check_add_friend.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h"
#include "old_data/db/anti_cheat/db_save_same_mid_other_char_login_hack.h"
#include "old_data/db/anti_cheat/db_hades_punish.h"
#include "old_data/db/auction_cashshop/db_get_premium_selected_cube_info.h"
#include "old_data/db/auth_security/db_secu_service_reward.h"
#include "old_data/db/auth_security/db_secu_service_update_validity_time.h"
#include "old_data/db/mail/letter_stat_payload.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/character/character_option_payload.h"
#include "old_data/db/inventory/account_cargo_payload.h"
#include "old_game/economy/mail/mailbox_system_multi_mail_payload.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/player/core/userinfo_dispatcher_reads.h"


// The remaining DB-wire payloads are intentionally opaque at this provider
// boundary.  Their byte widths are fixed by the old Stream/CStreamGuard
// specializations below, so no second, cross-domain payload definition is
// introduced here.
struct SIG_ARRANGE_USER_LOGIN_INFO;
struct SIG_AUCTION_OWNER_IS_VIP;
struct SIG_AUCTION_RESULT_ASK_PRICE;
struct SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM;
struct SIG_AUCTION_RESULT_BIDDING;
struct SIG_AUCTION_RESULT_BUY_ITEM_APIECE;
struct SIG_AUCTION_RESULT_REGIST;
struct SIG_AUCTION_RESULT_REGIST_CANCEL;
struct SIG_DEATH_TOWER_LOAD_TOP_RANKER;
struct SIG_DEATH_TOWER_UPDATE_BEST_RECORD;
struct SIG_DUNGEON_DATA_STATISTIC;
struct SIG_EVENT_LEVEUP_SUPPORT_INFO;
struct SIG_FIND_FACTORY_HUB_FATIGUE_CHECK;
struct SIG_FIND_FACTORY_HUB_USER;
struct SIG_FIND_FACTORY_HUB_USER_NO_CONN;
struct SIG_GET_PREMIUM_SELECTED_CUBE_INFO;
struct SIG_HACK_IP_SAVE;
struct SIG_LOAD_DIMENSION;
struct SIG_LOAD_PU_USER_INFO;
struct SIG_NAT_TYPE_USER;
struct SIG_ONLINE_PRELIMINARY_TEAM;
struct SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST;
struct SIG_REQUEST_JOIN_GUILD;
struct SIG_REQUEST_JOIN_GUILD_DB_REPLY;
struct SIG_SAVE_EVENTITEM;
struct SIG_SAVE_HACK_USER_PUNISH;
struct SIG_SECURITY_CARD_ACTIVATE;
struct SIG_SECURITY_CARD_ISSUE;
struct SIG_SECURITY_CARD_RETRANSFER;
struct SIG_SECURITY_CARD_UPDATE_VALIDITYTIME;
struct SIG_UPDATE_CHARACTER_DAY_EVENT;
struct SIG_UPDATE_DIMENSION;
struct SIG_UPDATE_USER_HANDICAP;
struct SIG_USER_PLAY_TIME_STATISTIC;
struct Packet_Tcp_Server_Connect;

// IDA_EA: 0x8162802
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN27InterSelectMobileAuthReward3MsgEEEPT_v
// DEMANGLED: InterSelectMobileAuthReward::Msg * CStreamGuard::GetInBuffer<InterSelectMobileAuthReward::Msg>(void)
template <>
InterSelectMobileAuthReward::Msg*
CStreamGuard::GetInBuffer<InterSelectMobileAuthReward::Msg>() {
    return stream_->GetInBuffer<InterSelectMobileAuthReward::Msg>();
}

// IDA_EA: 0x8237932
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_PARTY_STATUSEEPT_v
// DEMANGLED: SIG_PARTY_STATUS * CStreamGuard::GetInBuffer<SIG_PARTY_STATUS>(void)
template <>
SIG_PARTY_STATUS* CStreamGuard::GetInBuffer<SIG_PARTY_STATUS>() {
    return stream_->GetInBuffer<SIG_PARTY_STATUS>();
}

// IDA_EA: 0x80E2938
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI35SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPEEEPT_v
// DEMANGLED: SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE * CStreamGuard::GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>(void)
template <>
SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE*
CStreamGuard::GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>() {
    return stream_->GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>();
}

// IDA_EA: 0x80F9E44
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_SAVE_CLIENT_HACK_LOGEEPT_v
// DEMANGLED: SIG_SAVE_CLIENT_HACK_LOG * CStreamGuard::GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>(void)
template <>
SIG_SAVE_CLIENT_HACK_LOG*
CStreamGuard::GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>() {
    return stream_->GetInBuffer<SIG_SAVE_CLIENT_HACK_LOG>();
}

// IDA_EA: 0x81575F0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21STBossDungeonTowerLogEEPT_v
// DEMANGLED: STBossDungeonTowerLog * CStreamGuard::GetInBuffer<STBossDungeonTowerLog>(void)
template <>
STBossDungeonTowerLog*
CStreamGuard::GetInBuffer<STBossDungeonTowerLog>() {
    return stream_->GetInBuffer<STBossDungeonTowerLog>();
}

// IDA_EA: 0x8236E34
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_FIND_FACTORY_HUBEEPT_v
// DEMANGLED: SIG_FIND_FACTORY_HUB * CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB>(void)
template <>
SIG_FIND_FACTORY_HUB*
CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB>() {
    return stream_->GetInBuffer<SIG_FIND_FACTORY_HUB>();
}

// IDA_EA: 0x82378D8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_FORBID_USER_TO_PLAYEEPT_v
// DEMANGLED: SIG_FORBID_USER_TO_PLAY * CStreamGuard::GetInBuffer<SIG_FORBID_USER_TO_PLAY>(void)
template <>
SIG_FORBID_USER_TO_PLAY*
CStreamGuard::GetInBuffer<SIG_FORBID_USER_TO_PLAY>() {
    return stream_->GetInBuffer<SIG_FORBID_USER_TO_PLAY>();
}

// IDA_EA: 0x8237B96
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_AUCTION_MONEY_SUBEEPT_v
// DEMANGLED: SIG_AUCTION_MONEY_SUB * CStreamGuard::GetInBuffer<SIG_AUCTION_MONEY_SUB>(void)
template <>
SIG_AUCTION_MONEY_SUB*
CStreamGuard::GetInBuffer<SIG_AUCTION_MONEY_SUB>() {
    return stream_->GetInBuffer<SIG_AUCTION_MONEY_SUB>();
}

// IDA_EA: 0x82B2CA2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI36SIG_UPDATE_LINK_CHARAC_CONNECT_STATEEEPT_v
// DEMANGLED: SIG_UPDATE_LINK_CHARAC_CONNECT_STATE * CStreamGuard::GetInBuffer<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE>(void)
template <>
SIG_UPDATE_LINK_CHARAC_CONNECT_STATE*
CStreamGuard::GetInBuffer<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE>() {
    return stream_->GetInBuffer<SIG_UPDATE_LINK_CHARAC_CONNECT_STATE>();
}

// IDA_EA: 0x8450256
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_LOAD_EVENTITEMEEPT_v
// DEMANGLED: SIG_LOAD_EVENTITEM * CStreamGuard::GetInBuffer<SIG_LOAD_EVENTITEM>(void)
template <>
SIG_LOAD_EVENTITEM* CStreamGuard::GetInBuffer<SIG_LOAD_EVENTITEM>() {
    return stream_->GetInBuffer<SIG_LOAD_EVENTITEM>();
}
// IDA_EA: 0x8450A1E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_UPDATE_LOGINEEPT_v
// DEMANGLED: SIG_UPDATE_LOGIN * CStreamGuard::GetInBuffer<SIG_UPDATE_LOGIN>(void)
template <>
SIG_UPDATE_LOGIN* CStreamGuard::GetInBuffer<SIG_UPDATE_LOGIN>() {
    return stream_->GetInBuffer<SIG_UPDATE_LOGIN>();
}
// IDA_EA: 0x8451E84
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_GOBLIN_UPDATE_REWARDEEPT_v
// DEMANGLED: SIG_GOBLIN_UPDATE_REWARD * CStreamGuard::GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>(void)
template <>
SIG_GOBLIN_UPDATE_REWARD*
CStreamGuard::GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>() {
    return stream_->GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>();
}

// IDA_EA: 0x8451FEA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_SAVE_HACK_USER_PUNISHEEPT_v
// DEMANGLED: SIG_SAVE_HACK_USER_PUNISH * CStreamGuard::GetInBuffer<SIG_SAVE_HACK_USER_PUNISH>(void)
template <>
SIG_SAVE_HACK_USER_PUNISH*
CStreamGuard::GetInBuffer<SIG_SAVE_HACK_USER_PUNISH>() {
    return reinterpret_cast<SIG_SAVE_HACK_USER_PUNISH*>(GetInBufferUntyped(24));
}

// IDA_EA: 0x845228E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_UPDATE_USER_HANDICAPEEPT_v
// DEMANGLED: SIG_UPDATE_USER_HANDICAP * CStreamGuard::GetInBuffer<SIG_UPDATE_USER_HANDICAP>(void)
template <>
SIG_UPDATE_USER_HANDICAP*
CStreamGuard::GetInBuffer<SIG_UPDATE_USER_HANDICAP>() {
    return reinterpret_cast<SIG_UPDATE_USER_HANDICAP*>(GetInBufferUntyped(24));
}

// IDA_EA: 0x845248C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI31SIG_DEATH_TOWER_LOAD_TOP_RANKEREEPT_v
// DEMANGLED: SIG_DEATH_TOWER_LOAD_TOP_RANKER * CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_LOAD_TOP_RANKER>(void)
template <>
SIG_DEATH_TOWER_LOAD_TOP_RANKER*
CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_LOAD_TOP_RANKER>() {
    return reinterpret_cast<SIG_DEATH_TOWER_LOAD_TOP_RANKER*>(
        GetInBufferUntyped(220008));
}

// IDA_EA: 0x845259E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_ARRANGE_USER_LOGIN_INFOEEPT_v
// DEMANGLED: SIG_ARRANGE_USER_LOGIN_INFO * CStreamGuard::GetInBuffer<SIG_ARRANGE_USER_LOGIN_INFO>(void)
template <>
SIG_ARRANGE_USER_LOGIN_INFO*
CStreamGuard::GetInBuffer<SIG_ARRANGE_USER_LOGIN_INFO>() {
    return reinterpret_cast<SIG_ARRANGE_USER_LOGIN_INFO*>(GetInBufferUntyped(12));
}

// IDA_EA: 0x8452608
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_SECURITY_CARD_ISSUEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_ISSUE * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ISSUE>(void)
template <>
SIG_SECURITY_CARD_ISSUE*
CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ISSUE>() {
    return reinterpret_cast<SIG_SECURITY_CARD_ISSUE*>(GetInBufferUntyped(144));
}

// IDA_EA: 0x84526DC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_SECURITY_CARD_ACTIVATEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_ACTIVATE * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ACTIVATE>(void)
template <>
SIG_SECURITY_CARD_ACTIVATE*
CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ACTIVATE>() {
    return reinterpret_cast<SIG_SECURITY_CARD_ACTIVATE*>(GetInBufferUntyped(12));
}

// IDA_EA: 0x8452746
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI37SIG_SECURITY_CARD_UPDATE_VALIDITYTIMEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_UPDATE_VALIDITYTIME * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_UPDATE_VALIDITYTIME>(void)
template <>
SIG_SECURITY_CARD_UPDATE_VALIDITYTIME*
CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_UPDATE_VALIDITYTIME>() {
    return reinterpret_cast<SIG_SECURITY_CARD_UPDATE_VALIDITYTIME*>(
        GetInBufferUntyped(8));
}

// IDA_EA: 0x845275C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_SECURITY_CARD_RETRANSFEREEPT_v
// DEMANGLED: SIG_SECURITY_CARD_RETRANSFER * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_RETRANSFER>(void)
template <>
SIG_SECURITY_CARD_RETRANSFER*
CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_RETRANSFER>() {
    return reinterpret_cast<SIG_SECURITY_CARD_RETRANSFER*>(GetInBufferUntyped(140));
}
// IDA_EA: 0x84527C6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_AUTO_PUNISH_RULE_BLACK_IPEEPT_v
// DEMANGLED: SIG_AUTO_PUNISH_RULE_BLACK_IP * CStreamGuard::GetInBuffer<SIG_AUTO_PUNISH_RULE_BLACK_IP>(void)
template <>
SIG_AUTO_PUNISH_RULE_BLACK_IP*
CStreamGuard::GetInBuffer<SIG_AUTO_PUNISH_RULE_BLACK_IP>() {
    return stream_->GetInBuffer<SIG_AUTO_PUNISH_RULE_BLACK_IP>();
}

// IDA_EA: 0x84527DC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_AUTO_PUNISH_RULE_HACK_TYPEEEPT_v
// DEMANGLED: SIG_AUTO_PUNISH_RULE_HACK_TYPE * CStreamGuard::GetInBuffer<SIG_AUTO_PUNISH_RULE_HACK_TYPE>(void)
template <>
SIG_AUTO_PUNISH_RULE_HACK_TYPE*
CStreamGuard::GetInBuffer<SIG_AUTO_PUNISH_RULE_HACK_TYPE>() {
    return stream_->GetInBuffer<SIG_AUTO_PUNISH_RULE_HACK_TYPE>();
}
// IDA_EA: 0x845291A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI35SIG_INSERT_0712_WINTER_EVENT_COUPONEEPT_v
// DEMANGLED: SIG_INSERT_0712_WINTER_EVENT_COUPON * CStreamGuard::GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>(void)
template <>
SIG_INSERT_0712_WINTER_EVENT_COUPON*
CStreamGuard::GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>() {
    return stream_->GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>();
}

// IDA_EA: 0x8452984
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_INSERT_ACHIEVE_EVENTEEPT_v
// DEMANGLED: SIG_INSERT_ACHIEVE_EVENT * CStreamGuard::GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>(void)
template <>
SIG_INSERT_ACHIEVE_EVENT*
CStreamGuard::GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>() {
    return stream_->GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>();
}

// IDA_EA: 0x8453440
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_FIND_FACTORY_HUB_FATIGUE_CHECKEEPT_v
// DEMANGLED: SIG_FIND_FACTORY_HUB_FATIGUE_CHECK * CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_FATIGUE_CHECK>(void)
template <>
SIG_FIND_FACTORY_HUB_FATIGUE_CHECK*
CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_FATIGUE_CHECK>() {
    return reinterpret_cast<SIG_FIND_FACTORY_HUB_FATIGUE_CHECK*>(
        GetInBufferUntyped(36));
}

// IDA_EA: 0x84534AA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_HACK_IP_SAVEEEPT_v
// DEMANGLED: SIG_HACK_IP_SAVE * CStreamGuard::GetInBuffer<SIG_HACK_IP_SAVE>(void)
template <>
SIG_HACK_IP_SAVE* CStreamGuard::GetInBuffer<SIG_HACK_IP_SAVE>() {
    return reinterpret_cast<SIG_HACK_IP_SAVE*>(GetInBufferUntyped(40));
}

// IDA_EA: 0x84535D2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_SECU_SERVICE_REWARDEEPT_v
// DEMANGLED: SIG_SECU_SERVICE_REWARD * CStreamGuard::GetInBuffer<SIG_SECU_SERVICE_REWARD>(void)
template <>
SIG_SECU_SERVICE_REWARD*
CStreamGuard::GetInBuffer<SIG_SECU_SERVICE_REWARD>() {
    return reinterpret_cast<SIG_SECU_SERVICE_REWARD*>(
        GetInBufferUntyped(sizeof(SIG_SECU_SERVICE_REWARD)));
}

// IDA_EA: 0x8453690
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI33SIG_FIND_FACTORY_HUB_USER_NO_CONNEEPT_v
// DEMANGLED: SIG_FIND_FACTORY_HUB_USER_NO_CONN * CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_USER_NO_CONN>(void)
template <>
SIG_FIND_FACTORY_HUB_USER_NO_CONN*
CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_USER_NO_CONN>() {
    return reinterpret_cast<SIG_FIND_FACTORY_HUB_USER_NO_CONN*>(
        GetInBufferUntyped(12));
}

// IDA_EA: 0x84536A6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_FIND_FACTORY_HUB_USEREEPT_v
// DEMANGLED: SIG_FIND_FACTORY_HUB_USER * CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_USER>(void)
template <>
SIG_FIND_FACTORY_HUB_USER*
CStreamGuard::GetInBuffer<SIG_FIND_FACTORY_HUB_USER>() {
    return reinterpret_cast<SIG_FIND_FACTORY_HUB_USER*>(GetInBufferUntyped(32));
}

// IDA_EA: 0x84538FA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI38SIG_SECURITY_CARD_SERVICE_VALIDITYTIMEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_SERVICE_VALIDITYTIME * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_SERVICE_VALIDITYTIME>(void)
template <>
SIG_SECURITY_CARD_SERVICE_VALIDITYTIME*
CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_SERVICE_VALIDITYTIME>() {
    return reinterpret_cast<SIG_SECURITY_CARD_SERVICE_VALIDITYTIME*>(
        GetInBufferUntyped(sizeof(SIG_SECURITY_CARD_SERVICE_VALIDITYTIME)));
}

// IDA_EA: 0x8453AFA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI31SIG_REQUEST_JOIN_GUILD_DB_REPLYEEPT_v
// DEMANGLED: SIG_REQUEST_JOIN_GUILD_DB_REPLY * CStreamGuard::GetInBuffer<SIG_REQUEST_JOIN_GUILD_DB_REPLY>(void)
template <>
SIG_REQUEST_JOIN_GUILD_DB_REPLY*
CStreamGuard::GetInBuffer<SIG_REQUEST_JOIN_GUILD_DB_REPLY>() {
    return reinterpret_cast<SIG_REQUEST_JOIN_GUILD_DB_REPLY*>(
        GetInBufferUntyped(72));
}

// IDA_EA: 0x8453B64
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_REQUEST_JOIN_GUILDEEPT_v
// DEMANGLED: SIG_REQUEST_JOIN_GUILD * CStreamGuard::GetInBuffer<SIG_REQUEST_JOIN_GUILD>(void)
template <>
SIG_REQUEST_JOIN_GUILD*
CStreamGuard::GetInBuffer<SIG_REQUEST_JOIN_GUILD>() {
    return reinterpret_cast<SIG_REQUEST_JOIN_GUILD*>(GetInBufferUntyped(68));
}

// IDA_EA: 0x8454286
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_HADES_PUNISHEEPT_v
// DEMANGLED: SIG_HADES_PUNISH * CStreamGuard::GetInBuffer<SIG_HADES_PUNISH>(void)
template <>
SIG_HADES_PUNISH* CStreamGuard::GetInBuffer<SIG_HADES_PUNISH>() {
    return reinterpret_cast<SIG_HADES_PUNISH*>(
        GetInBufferUntyped(sizeof(SIG_HADES_PUNISH)));
}
// IDA_EA: 0x8454738
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoEEEPT_v
// DEMANGLED: SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo * CStreamGuard::GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>(void)
template <>
SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*
CStreamGuard::GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>() {
    return stream_->GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>();
}
// IDA_EA: 0x8454860
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI21SIG_LOAD_PU_USER_INFOEEPT_v
// DEMANGLED: SIG_LOAD_PU_USER_INFO * CStreamGuard::GetInBuffer<SIG_LOAD_PU_USER_INFO>(void)
template <>
SIG_LOAD_PU_USER_INFO* CStreamGuard::GetInBuffer<SIG_LOAD_PU_USER_INFO>() {
    return reinterpret_cast<SIG_LOAD_PU_USER_INFO*>(GetInBufferUntyped(1));
}

// IDA_EA: 0x84549C6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI38SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RESEEPT_v
// DEMANGLED: SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES * CStreamGuard::GetInBuffer<SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES>(void)
template <>
SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES*
CStreamGuard::GetInBuffer<SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES>() {
    return reinterpret_cast<SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES*>(
        GetInBufferUntyped(sizeof(SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES)));
}

// IDA_EA: 0x8454A30
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI29SIG_EVENT_LEVEUP_SUPPORT_INFOEEPT_v
// DEMANGLED: SIG_EVENT_LEVEUP_SUPPORT_INFO * CStreamGuard::GetInBuffer<SIG_EVENT_LEVEUP_SUPPORT_INFO>(void)
template <>
SIG_EVENT_LEVEUP_SUPPORT_INFO*
CStreamGuard::GetInBuffer<SIG_EVENT_LEVEUP_SUPPORT_INFO>() {
    return reinterpret_cast<SIG_EVENT_LEVEUP_SUPPORT_INFO*>(
        GetInBufferUntyped(44));
}
// IDA_EA: 0x847DD52
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_AUCTION_NOTIFY_AUCTION_SERVICEEEPT_v
// DEMANGLED: SIG_AUCTION_NOTIFY_AUCTION_SERVICE * CStreamGuard::GetInBuffer<SIG_AUCTION_NOTIFY_AUCTION_SERVICE>(void)
template <>
SIG_AUCTION_NOTIFY_AUCTION_SERVICE*
CStreamGuard::GetInBuffer<SIG_AUCTION_NOTIFY_AUCTION_SERVICE>() {
    return stream_->GetInBuffer<SIG_AUCTION_NOTIFY_AUCTION_SERVICE>();
}
// IDA_EA: 0x847DD68
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_AUCTION_RESULT_ASK_PRICEEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_ASK_PRICE * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_ASK_PRICE>(void)
template <>
SIG_AUCTION_RESULT_ASK_PRICE*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_ASK_PRICE>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_ASK_PRICE*>(GetInBufferUntyped(36));
}

// IDA_EA: 0x847DD7E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI40SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUMEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM>(void)
template <>
SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_ASK_REGISTED_ITEM_NUM*>(
        GetInBufferUntyped(45));
}

// IDA_EA: 0x847DD94
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_AUCTION_RESULT_REGISTEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_REGIST * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_REGIST>(void)
template <>
SIG_AUCTION_RESULT_REGIST*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_REGIST>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_REGIST*>(GetInBufferUntyped(10));
}

// IDA_EA: 0x847DDAA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_AUCTION_RESULT_BIDDINGEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_BIDDING * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_BIDDING>(void)
template <>
SIG_AUCTION_RESULT_BIDDING*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_BIDDING>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_BIDDING*>(GetInBufferUntyped(58));
}

// IDA_EA: 0x847DDC0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI32SIG_AUCTION_RESULT_REGIST_CANCELEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_REGIST_CANCEL * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_REGIST_CANCEL>(void)
template <>
SIG_AUCTION_RESULT_REGIST_CANCEL*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_REGIST_CANCEL>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_REGIST_CANCEL*>(
        GetInBufferUntyped(10));
}

// IDA_EA: 0x847DDEC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_AUCTION_RESULT_BUY_ITEM_APIECEEEPT_v
// DEMANGLED: SIG_AUCTION_RESULT_BUY_ITEM_APIECE * CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_BUY_ITEM_APIECE>(void)
template <>
SIG_AUCTION_RESULT_BUY_ITEM_APIECE*
CStreamGuard::GetInBuffer<SIG_AUCTION_RESULT_BUY_ITEM_APIECE>() {
    return reinterpret_cast<SIG_AUCTION_RESULT_BUY_ITEM_APIECE*>(
        GetInBufferUntyped(sizeof(SIG_AUCTION_RESULT_BUY_ITEM_APIECE)));
}

// IDA_EA: 0x847DE18
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_AUCTION_OWNER_IS_VIPEEPT_v
// DEMANGLED: SIG_AUCTION_OWNER_IS_VIP * CStreamGuard::GetInBuffer<SIG_AUCTION_OWNER_IS_VIP>(void)
template <>
SIG_AUCTION_OWNER_IS_VIP*
CStreamGuard::GetInBuffer<SIG_AUCTION_OWNER_IS_VIP>() {
    return reinterpret_cast<SIG_AUCTION_OWNER_IS_VIP*>(GetInBufferUntyped(42));
}

// IDA_EA: 0x84F1578
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI39SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACKEEPT_v
// DEMANGLED: SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK * CStreamGuard::GetInBuffer<SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK>(void)
template <>
SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK*
CStreamGuard::GetInBuffer<SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK>() {
    return reinterpret_cast<SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK*>(
        GetInBufferUntyped(32));
}
// IDA_EA: 0x8558B3C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL * CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>(void)
// Allocate the complete canonical record rather than repeating its byte size.
template <>
SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL*
CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>() {
    return reinterpret_cast<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL*>(
        stream_->GetInBufferUntyped(sizeof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL)));
}
// IDA_EA: 0x8589132
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27SIG_ONLINE_PRELIMINARY_TEAMEEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_TEAM * CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_TEAM>(void)
template <>
SIG_ONLINE_PRELIMINARY_TEAM*
CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_TEAM>() {
    return reinterpret_cast<SIG_ONLINE_PRELIMINARY_TEAM*>(GetInBufferUntyped(32));
}

// IDA_EA: 0x8589148
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI38SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LISTEEPT_v
// DEMANGLED: SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST * CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST>(void)
template <>
SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*
CStreamGuard::GetInBuffer<SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST>() {
    return reinterpret_cast<SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*>(
        GetInBufferUntyped(808));
}

// IDA_EA: 0x86110F0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_USER_PLAY_TIME_STATISTICEEPT_v
// DEMANGLED: SIG_USER_PLAY_TIME_STATISTIC * CStreamGuard::GetInBuffer<SIG_USER_PLAY_TIME_STATISTIC>(void)
template <>
SIG_USER_PLAY_TIME_STATISTIC*
CStreamGuard::GetInBuffer<SIG_USER_PLAY_TIME_STATISTIC>() {
    return reinterpret_cast<SIG_USER_PLAY_TIME_STATISTIC*>(GetInBufferUntyped(244));
}

// IDA_EA: 0x863D27C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_DUNGEON_DATA_STATISTICEEPT_v
// DEMANGLED: SIG_DUNGEON_DATA_STATISTIC * CStreamGuard::GetInBuffer<SIG_DUNGEON_DATA_STATISTIC>(void)
template <>
SIG_DUNGEON_DATA_STATISTIC*
CStreamGuard::GetInBuffer<SIG_DUNGEON_DATA_STATISTIC>() {
    return reinterpret_cast<SIG_DUNGEON_DATA_STATISTIC*>(GetInBufferUntyped(128));
}

// IDA_EA: 0x863D2EE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25Packet_Tcp_Server_ConnectEEPT_v
// DEMANGLED: Packet_Tcp_Server_Connect * CStreamGuard::GetInBuffer<Packet_Tcp_Server_Connect>(void)
template <>
Packet_Tcp_Server_Connect*
CStreamGuard::GetInBuffer<Packet_Tcp_Server_Connect>() {
    return reinterpret_cast<Packet_Tcp_Server_Connect*>(GetInBufferUntyped(11));
}

// IDA_EA: 0x86985DA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_GET_PREMIUM_SELECTED_CUBE_INFOEEPT_v
// DEMANGLED: SIG_GET_PREMIUM_SELECTED_CUBE_INFO * CStreamGuard::GetInBuffer<SIG_GET_PREMIUM_SELECTED_CUBE_INFO>(void)
template <>
SIG_GET_PREMIUM_SELECTED_CUBE_INFO*
CStreamGuard::GetInBuffer<SIG_GET_PREMIUM_SELECTED_CUBE_INFO>() {
    return reinterpret_cast<SIG_GET_PREMIUM_SELECTED_CUBE_INFO*>(
        GetInBufferUntyped(4));
}

// IDA_EA: 0x869C49C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_UPDATE_CHARACTER_DAY_EVENTEEPT_v
// DEMANGLED: SIG_UPDATE_CHARACTER_DAY_EVENT * CStreamGuard::GetInBuffer<SIG_UPDATE_CHARACTER_DAY_EVENT>(void)
template <>
SIG_UPDATE_CHARACTER_DAY_EVENT*
CStreamGuard::GetInBuffer<SIG_UPDATE_CHARACTER_DAY_EVENT>() {
    return reinterpret_cast<SIG_UPDATE_CHARACTER_DAY_EVENT*>(
        GetInBufferUntyped(8));
}
// IDA_EA: 0x869C9C2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI31SIG_STATISTIC_CERASHOP_GIFT_LOGEEPT_v
// DEMANGLED: SIG_STATISTIC_CERASHOP_GIFT_LOG * CStreamGuard::GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>(void)
template <>
SIG_STATISTIC_CERASHOP_GIFT_LOG*
CStreamGuard::GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>() {
    return stream_->GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>();
}
// IDA_EA: 0x86C1C7E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_SAVE_EVENTITEMEEPT_v
// DEMANGLED: SIG_SAVE_EVENTITEM * CStreamGuard::GetInBuffer<SIG_SAVE_EVENTITEM>(void)
template <>
SIG_SAVE_EVENTITEM* CStreamGuard::GetInBuffer<SIG_SAVE_EVENTITEM>() {
    return reinterpret_cast<SIG_SAVE_EVENTITEM*>(GetInBufferUntyped(25));
}

// IDA_EA: 0x86D4104
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_NAT_TYPE_USEREEPT_v
// DEMANGLED: SIG_NAT_TYPE_USER * CStreamGuard::GetInBuffer<SIG_NAT_TYPE_USER>(void)
template <>
SIG_NAT_TYPE_USER* CStreamGuard::GetInBuffer<SIG_NAT_TYPE_USER>() {
    return reinterpret_cast<SIG_NAT_TYPE_USER*>(GetInBufferUntyped(12));
}

// IDA_EA: 0x8452476
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_DEATH_TOWER_UPDATE_BEST_RECORDEEPT_v
// DEMANGLED: SIG_DEATH_TOWER_UPDATE_BEST_RECORD * CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_UPDATE_BEST_RECORD>(void)
template <>
SIG_DEATH_TOWER_UPDATE_BEST_RECORD*
CStreamGuard::GetInBuffer<SIG_DEATH_TOWER_UPDATE_BEST_RECORD>() {
    return reinterpret_cast<SIG_DEATH_TOWER_UPDATE_BEST_RECORD*>(
        GetInBufferUntyped(108));
}

// IDA_EA: 0x81367A6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN12advancealtar25_CharacAdvanceAltarDbDataEEEPT_v
// DEMANGLED: advancealtar::_CharacAdvanceAltarDbData * CStreamGuard::GetInBuffer<advancealtar::_CharacAdvanceAltarDbData>(void)
template <>
advancealtar::_CharacAdvanceAltarDbData*
CStreamGuard::GetInBuffer<advancealtar::_CharacAdvanceAltarDbData>() {
    return stream_->GetInBuffer<advancealtar::_CharacAdvanceAltarDbData>();
}

// IDA_EA: 0x8550780
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_SAVE_LOGGAMECHANNELEEPT_v
// DEMANGLED: SIG_SAVE_LOGGAMECHANNEL * CStreamGuard::GetInBuffer<SIG_SAVE_LOGGAMECHANNEL>(void)
template <>
WongWork::SIG_SAVE_LOGGAMECHANNEL*
CStreamGuard::GetInBuffer<WongWork::SIG_SAVE_LOGGAMECHANNEL>() {
    return stream_->GetInBuffer<WongWork::SIG_SAVE_LOGGAMECHANNEL>();
}

// IDA_EA: 0x84503AE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_AVATAR_INSERTEEPT_v
// DEMANGLED: SIG_AVATAR_INSERT * CStreamGuard::GetInBuffer<SIG_AVATAR_INSERT>(void)
template <>
SIG_AVATAR_INSERT* CStreamGuard::GetInBuffer<SIG_AVATAR_INSERT>() {
    return stream_->GetInBuffer<SIG_AVATAR_INSERT>();
}

// IDA_EA: 0x8570436
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_INSERT_RESPONSE_TIMEEEPT_v
// DEMANGLED: SIG_INSERT_RESPONSE_TIME * CStreamGuard::GetInBuffer<SIG_INSERT_RESPONSE_TIME>(void)
template <>
SIG_INSERT_RESPONSE_TIME*
CStreamGuard::GetInBuffer<SIG_INSERT_RESPONSE_TIME>() {
    return stream_->GetInBuffer<SIG_INSERT_RESPONSE_TIME>();
}

// IDA_EA: 0x8157A6A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26STAdvanceAltarEntranceDataEEPT_v
// DEMANGLED: STAdvanceAltarEntranceData * CStreamGuard::GetInBuffer<STAdvanceAltarEntranceData>(void)
template <>
STAdvanceAltarEntranceData*
CStreamGuard::GetInBuffer<STAdvanceAltarEntranceData>() {
    return stream_->GetInBuffer<STAdvanceAltarEntranceData>();
}

// IDA_EA: 0x8157AEC
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI27STAdvanceAltarLayerStatDataEEPT_v
// DEMANGLED: STAdvanceAltarLayerStatData * CStreamGuard::GetInBuffer<STAdvanceAltarLayerStatData>(void)
template <>
STAdvanceAltarLayerStatData*
CStreamGuard::GetInBuffer<STAdvanceAltarLayerStatData>() {
    return stream_->GetInBuffer<STAdvanceAltarLayerStatData>();
}

// IDA_EA: 0x8236DF0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_CHECK_ADD_FRIENDEEPT_v
// DEMANGLED: SIG_CHECK_ADD_FRIEND * CStreamGuard::GetInBuffer<SIG_CHECK_ADD_FRIEND>(void)
template <>
SIG_CHECK_ADD_FRIEND*
CStreamGuard::GetInBuffer<SIG_CHECK_ADD_FRIEND>() {
    return stream_->GetInBuffer<SIG_CHECK_ADD_FRIEND>();
}

// IDA_EA: 0x81627A8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN29InterSelectCreateDnfEventInfo3MsgEEEPT_v
// DEMANGLED: InterSelectCreateDnfEventInfo::Msg * CStreamGuard::GetInBuffer<InterSelectCreateDnfEventInfo::Msg>(void)
template <>
InterSelectCreateDnfEventInfo::Msg*
CStreamGuard::GetInBuffer<InterSelectCreateDnfEventInfo::Msg>() {
    return stream_->GetInBuffer<InterSelectCreateDnfEventInfo::Msg>();
}

// IDA_EA: 0x8162818
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN28InterSelectPcroomDailyReward3MsgEEEPT_v
// DEMANGLED: InterSelectPcroomDailyReward::Msg * CStreamGuard::GetInBuffer<InterSelectPcroomDailyReward::Msg>(void)
template <>
InterSelectPcroomDailyReward::Msg*
CStreamGuard::GetInBuffer<InterSelectPcroomDailyReward::Msg>() {
    return stream_->GetInBuffer<InterSelectPcroomDailyReward::Msg>();
}

// IDA_EA: 0x869AAB8
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20InformNoticeFlagDataEEPT_v
// DEMANGLED: InformNoticeFlagData * CStreamGuard::GetInBuffer<InformNoticeFlagData>(void)
template <>
InformNoticeFlagData*
CStreamGuard::GetInBuffer<InformNoticeFlagData>() {
    return stream_->GetInBuffer<InformNoticeFlagData>();
}

// IDA_EA: 0x869C126
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_SAVE_CONDITION_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CONDITION_EVENT * CStreamGuard::GetInBuffer<SIG_SAVE_CONDITION_EVENT>(void)
template <>
SIG_SAVE_CONDITION_EVENT*
CStreamGuard::GetInBuffer<SIG_SAVE_CONDITION_EVENT>() {
    return stream_->GetInBuffer<SIG_SAVE_CONDITION_EVENT>();
}

// IDA_EA: 0x869C13C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_SAVE_BLOOD_CLEAR_COUNTEEPT_v
// DEMANGLED: SIG_SAVE_BLOOD_CLEAR_COUNT * CStreamGuard::GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>(void)
template <>
SIG_SAVE_BLOOD_CLEAR_COUNT*
CStreamGuard::GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>() {
    return stream_->GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>();
}

// IDA_EA: 0x81627BE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN28InterSelectCreateDnfUserInfo3MsgEEEPT_v
// DEMANGLED: InterSelectCreateDnfUserInfo::Msg * CStreamGuard::GetInBuffer<InterSelectCreateDnfUserInfo::Msg>(void)
template <>
InterSelectCreateDnfUserInfo::Msg*
CStreamGuard::GetInBuffer<InterSelectCreateDnfUserInfo::Msg>() {
    return reinterpret_cast<InterSelectCreateDnfUserInfo::Msg*>(
        GetInBufferUntyped(8));
}

// IDA_EA: 0x82370A0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_EXTEND_CHARAC_SLOTEEPT_v
// DEMANGLED: SIG_EXTEND_CHARAC_SLOT * CStreamGuard::GetInBuffer<SIG_EXTEND_CHARAC_SLOT>(void)
template <>
SIG_EXTEND_CHARAC_SLOT*
CStreamGuard::GetInBuffer<SIG_EXTEND_CHARAC_SLOT>() {
    return stream_->GetInBuffer<SIG_EXTEND_CHARAC_SLOT>();
}

// IDA_EA: 0x8452054
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI15SIG_LETTER_STATEEPT_v
// DEMANGLED: SIG_LETTER_STAT * CStreamGuard::GetInBuffer<SIG_LETTER_STAT>(void)
template <>
SIG_LETTER_STAT* CStreamGuard::GetInBuffer<SIG_LETTER_STAT>() {
    return stream_->GetInBuffer<SIG_LETTER_STAT>();
}

// IDA_EA: 0x8452EE2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_LOAD_DIMENSIONEEPT_v
// DEMANGLED: SIG_LOAD_DIMENSION * CStreamGuard::GetInBuffer<SIG_LOAD_DIMENSION>(void)
template <>
SIG_LOAD_DIMENSION* CStreamGuard::GetInBuffer<SIG_LOAD_DIMENSION>() {
    return reinterpret_cast<SIG_LOAD_DIMENSION*>(GetInBufferUntyped(16));
}

// IDA_EA: 0x84539E4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI31SIG_CREATE_ACCOUNT_CARGO_RESULTEEPT_v
// DEMANGLED: SIG_CREATE_ACCOUNT_CARGO_RESULT * CStreamGuard::GetInBuffer<SIG_CREATE_ACCOUNT_CARGO_RESULT>(void)
template <>
SIG_CREATE_ACCOUNT_CARGO_RESULT*
CStreamGuard::GetInBuffer<SIG_CREATE_ACCOUNT_CARGO_RESULT>() {
    return reinterpret_cast<SIG_CREATE_ACCOUNT_CARGO_RESULT*>(
        GetInBufferUntyped(sizeof(SIG_CREATE_ACCOUNT_CARGO_RESULT)));
}

// IDA_EA: 0x84539FA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI32SIG_UPGRADE_ACCOUNT_CARGO_RESULTEEPT_v
// DEMANGLED: SIG_UPGRADE_ACCOUNT_CARGO_RESULT * CStreamGuard::GetInBuffer<SIG_UPGRADE_ACCOUNT_CARGO_RESULT>(void)
template <>
SIG_UPGRADE_ACCOUNT_CARGO_RESULT*
CStreamGuard::GetInBuffer<SIG_UPGRADE_ACCOUNT_CARGO_RESULT>() {
    return reinterpret_cast<SIG_UPGRADE_ACCOUNT_CARGO_RESULT*>(
        GetInBufferUntyped(sizeof(SIG_UPGRADE_ACCOUNT_CARGO_RESULT)));
}

// IDA_EA: 0x8453A10
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_ACCOUNT_CARGO_DATAEEPT_v
// DEMANGLED: SIG_ACCOUNT_CARGO_DATA * CStreamGuard::GetInBuffer<SIG_ACCOUNT_CARGO_DATA>(void)
template <>
SIG_ACCOUNT_CARGO_DATA*
CStreamGuard::GetInBuffer<SIG_ACCOUNT_CARGO_DATA>() {
    return reinterpret_cast<SIG_ACCOUNT_CARGO_DATA*>(
        GetInBufferUntyped(sizeof(SIG_ACCOUNT_CARGO_DATA)));
}

// IDA_EA: 0x8699272
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI22SIG_UPDATE_CHARAC_VIEWEEPT_v
// DEMANGLED: SIG_UPDATE_CHARAC_VIEW * CStreamGuard::GetInBuffer<SIG_UPDATE_CHARAC_VIEW>(void)
template <>
SIG_UPDATE_CHARAC_VIEW*
CStreamGuard::GetInBuffer<SIG_UPDATE_CHARAC_VIEW>() {
    return reinterpret_cast<SIG_UPDATE_CHARAC_VIEW*>(
        GetInBufferUntyped(sizeof(SIG_UPDATE_CHARAC_VIEW)));
}

// IDA_EA: 0x869AA4A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_UPDATE_DIMENSIONEEPT_v
// DEMANGLED: SIG_UPDATE_DIMENSION * CStreamGuard::GetInBuffer<SIG_UPDATE_DIMENSION>(void)
template <>
SIG_UPDATE_DIMENSION*
CStreamGuard::GetInBuffer<SIG_UPDATE_DIMENSION>() {
    return reinterpret_cast<SIG_UPDATE_DIMENSION*>(GetInBufferUntyped(16));
}

// IDA_EA: 0x869AA60
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_CHARACTER_OPTIONEEPT_v
// DEMANGLED: SIG_CHARACTER_OPTION * CStreamGuard::GetInBuffer<SIG_CHARACTER_OPTION>(void)
template <>
SIG_CHARACTER_OPTION*
CStreamGuard::GetInBuffer<SIG_CHARACTER_OPTION>() {
    return reinterpret_cast<SIG_CHARACTER_OPTION*>(
        GetInBufferUntyped(sizeof(SIG_CHARACTER_OPTION)));
}
