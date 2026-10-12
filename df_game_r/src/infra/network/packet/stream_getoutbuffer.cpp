#include "infra/network/packet/stream.h"

#include "common/config.h"
#include "common/manager/mannerless_user_accusation_packet.h"
#include "old_data/db/anti_cheat/account_hack_payload.h"
#include "old_data/db/anti_cheat/db_report_mannerless_user.h"
#include "old_data/db/auction_cashshop/cera_shop_purchase_count_payload.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/mail/mailbox_system_multi_mail_payload.h"
#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/account/break_away_payload.h"
#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"
#include "old_data/db/auth_security/db_check_secu_reward.h"
#include "old_data/db/auth_security/db_mouse_register.h"
#include "old_data/db/auth_security/security_card_payloads.h"
#include "old_data/db/auction_cashshop/db_billing_record.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/dungeon/blood_best_record_payload.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_data/db/pvp_party/db_save_pvp.h"
#include "old_data/db/quest/db_update_quest.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_data/db/skill/db_update_one_day_lethe_skill.h"
#include "old_data/db/statistics_log/db_insert_unlimit_support_log.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/progression/skill/combo_skill.h"


struct MSG_STAT_BREAKJAR;
struct MSG_STAT_COMPOSITON;
struct MSG_STAT_DUNGEONCLEAR;
struct MSG_STAT_PVPPLAY;
struct MSG_STAT_RANKRES;
struct SIG_LOAD_DIMENSION;

#include "old_game/activity/reward/advance_altar_statistics.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_data/channel/log_game_channel.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/character/create_charac_payload.h"
#include "old_data/db/character/db_check_charac_name.h"
#include "old_data/db/character/db_delete_charac.h"
#include "old_data/db/character/db_get_user_regdate.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"
#include "old_data/db/character/db_req_change_charac_name.h"
#include "old_data/db/character/db_select_charac_mid.h"
#include "old_data/db/character/load_charac_payload.h"
#include "old_data/db/character/query_charac_info_payload.h"
#include "old_data/db/character/tag_charac_info_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_data/db/legacy_dispatch/db_check_add_friend.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "old_data/db/event/reward_event_request.h"
#include "old_data/channel/arad_server_state.h"
#include "old_data/db/skill/item_making_skill_payload.h"
#include "old_game/progression/quest/quest_state_runtime.h"


typedef char SIG_INSERT_SERVER_LOAD_STATISTIC_Size_Must_Be_10[
    (sizeof(SIG_INSERT_SERVER_LOAD_STATISTIC) == 10) ? 1 : -1];
typedef char SIG_VALUE_STATISTIC_Size_Must_Be_128[
    (sizeof(SIG_VALUE_STATISTIC) == 128) ? 1 : -1];
typedef char SIG_CIRCULATION_STATISTIC_Size_Must_Be_200[
    (sizeof(SIG_CIRCULATION_STATISTIC) == 200) ? 1 : -1];
typedef char SIG_INSERT_RESPONSE_TIME_Size_Must_Be_2008[
    (sizeof(SIG_INSERT_RESPONSE_TIME) == 2008) ? 1 : -1];
typedef char SIG_MAILBOX_LIST_Size_Must_Be_0x33DA[
    (sizeof(SIG_MAILBOX_LIST) == 0x33DA) ? 1 : -1];
typedef char SIG_LOGIN_DATA_Size_Must_Be_0x4478[
    (sizeof(SIG_LOGIN_DATA) == 0x4478) ? 1 : -1];
typedef char SIG_AUTO_MARKET_CONDITIONS_CONTROL_Size_Must_Be_0x6C[
    (sizeof(SIG_AUTO_MARKET_CONDITIONS_CONTROL) == 0x6C) ? 1 : -1];
typedef char SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL_Size_Must_Be_0x30[
    (sizeof(SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL) == 0x30) ? 1 : -1];
typedef char SIG_SWAP_CREATURE_ITEM_Size_Must_Be_0x18[
    (sizeof(SIG_SWAP_CREATURE_ITEM) == 0x18) ? 1 : -1];
typedef char SIG_CREATURE_ITEM_Size_Must_Be_0x74[
    (sizeof(SIG_CREATURE_ITEM) == 0x74) ? 1 : -1];
typedef char SIG_CREATURE_ITEM_LIST_Size_Must_Be_0x4078[
    (sizeof(SIG_CREATURE_ITEM_LIST) == 0x4078) ? 1 : -1];
typedef char SIG_LOAD_KILL_MONSTER_INFO_Size_Must_Be_0xAFCC[
    (sizeof(SIG_LOAD_KILL_MONSTER_INFO) == 0xAFCC) ? 1 : -1];
typedef char SIG_ITEM_MAKING_SKILL_Size_Must_Be_0x1C[
    (sizeof(SIG_ITEM_MAKING_SKILL) == 0x1C) ? 1 : -1];
typedef char SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO_Size_Must_Be_0x198[
    (sizeof(SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO) == 0x198) ? 1 : -1];
typedef char Packet_Mannerless_User_Accusation_Size_Must_Be_0x15F[
    (sizeof(Packet_Mannerless_User_Accusation) == 0x15F) ? 1 : -1];
typedef char SIG_MANNERLESS_USER_Size_Must_Be_0x13A[
    (sizeof(SIG_MANNERLESS_USER) == 0x13A) ? 1 : -1];
typedef char stCeraShopIPGRequest_t_Size_Must_Be_0x27C[
    (sizeof(stCeraShopIPGRequest_t) == 0x27C) ? 1 : -1];
typedef char SIG_AVATAR_INSERT_Size_Must_Be_0x53[
    (sizeof(SIG_AVATAR_INSERT) == 0x53) ? 1 : -1];
typedef char SIG_AVATAR_CHANGE_STAT_Size_Must_Be_0x51[
    (sizeof(SIG_AVATAR_CHANGE_STAT) == 0x51) ? 1 : -1];
typedef char SIG_SAVE_LOGGAMECHANNEL_Size_Must_Be_0x3B[
    (sizeof(SIG_SAVE_LOGGAMECHANNEL) == 0x3B) ? 1 : -1];
typedef char CharacAdvanceAltarDbData_Size_Must_Be_0x51C[
    (sizeof(advancealtar::_CharacAdvanceAltarDbData) == 0x51C) ? 1 : -1];
typedef char STAdvanceAltarEntranceData_Size_Must_Be_0x10[
    (sizeof(STAdvanceAltarEntranceData) == 0x10) ? 1 : -1];
typedef char STAdvanceAltarLayerStatData_Size_Must_Be_0x10[
    (sizeof(STAdvanceAltarLayerStatData) == 0x10) ? 1 : -1];
typedef char SIG_CHECK_ADD_FRIEND_Size_Must_Be_0x24[
    (sizeof(SIG_CHECK_ADD_FRIEND) == 0x24) ? 1 : -1];
typedef char SIG_SELECT_SERVER_MESSAGE_Size_Must_Be_4[
    (sizeof(SIG_SELECT_SERVER_MESSAGE) == 4) ? 1 : -1];
typedef char SIG_UPDATE_SERVER_MESSAGE_Size_Must_Be_0x60[
    (sizeof(SIG_UPDATE_SERVER_MESSAGE) == 0x60) ? 1 : -1];
typedef char SigAradEventItemLog_Size_Must_Be_0x10[
    (sizeof(arad::SigAradEventItemLog) == 0x10) ? 1 : -1];
typedef char SigSetCharacCreateTime_Size_Must_Be_0x08[
    (sizeof(arad::SigSetCharacCreateTime) == 0x08) ? 1 : -1];
typedef char SigLoadRewardEventItem_Size_Must_Be_0x56[
    (sizeof(arad::SigLoadRewardEventItem) == 0x56) ? 1 : -1];
typedef char SigSaveRewardEventItem_Size_Must_Be_0x16[
    (sizeof(arad::SigSaveRewardEventItem) == 0x16) ? 1 : -1];
typedef char SigServerState_Size_Must_Be_0x195[
    (sizeof(arad::SigServerState) == 0x195) ? 1 : -1];
typedef char SigAradAvatarHiddenOptionSave_Size_Must_Be_0x10[
    (sizeof(arad::SigAradAvatarHiddenOptionSave) == 0x10) ? 1 : -1];
typedef char SigAradAvatarEnduranceSave_Size_Must_Be_0x08[
    (sizeof(arad::SigAradAvatarEnduranceSave) == 0x08) ? 1 : -1];
typedef char InformNoticeFlagData_GetOutBuffer_Size_Must_Be_0x14[
    (sizeof(InformNoticeFlagData) == 0x14) ? 1 : -1];
typedef char SIG_AVATAR_COLOR_CHANGE_GetOutBuffer_Size_Must_Be_0xC[
    (sizeof(SIG_AVATAR_COLOR_CHANGE) == 0xC) ? 1 : -1];
typedef char SIG_AVATAR_MOVE_GetOutBuffer_Size_Must_Be_0x16[
    (sizeof(SIG_AVATAR_MOVE) == 0x16) ? 1 : -1];
typedef char SIG_AVATAR_OPTION_CHANGE_GetOutBuffer_Size_Must_Be_0xC[
    (sizeof(SIG_AVATAR_OPTION_CHANGE) == 0xC) ? 1 : -1];
typedef char SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD_GetOutBuffer_Size_Must_Be_0x10[
    (sizeof(SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD) == 0x10) ? 1 : -1];
typedef char SIG_BILLING_RECORD_GetOutBuffer_Size_Must_Be_0x10E[
    (sizeof(SIG_BILLING_RECORD) == 0x10E) ? 1 : -1];
typedef char SIG_RESTING_USER_RESTRICT_GetOutBuffer_Size_Must_Be_0x4[
    (sizeof(SIG_RESTING_USER_RESTRICT) == 0x4) ? 1 : -1];
typedef char SIG_CHECK_USER_SECU_REWARD_GetOutBuffer_Size_Must_Be_0x8[
    (sizeof(SIG_CHECK_USER_SECU_REWARD) == 0x8) ? 1 : -1];
typedef char SIG_MOUSE_REGISTER_GetOutBuffer_Size_Must_Be_0x59[
    (sizeof(SIG_MOUSE_REGISTER) == 0x59) ? 1 : -1];
typedef char SIG_UPDATE_QUEST_GetOutBuffer_Size_Must_Be_0x7601[
    (sizeof(SIG_UPDATE_QUEST) == 0x7601) ? 1 : -1];
typedef char SIG_COMBO_SKILL_GetOutBuffer_Size_Must_Be_0xB2[
    (sizeof(SIG_COMBO_SKILL) == 0xB2) ? 1 : -1];
typedef char SIG_INSERT_UNLIMIT_SUPPORT_LOG_GetOutBuffer_Size_Must_Be_0x2C[
    (sizeof(SIG_INSERT_UNLIMIT_SUPPORT_LOG) == 0x2C) ? 1 : -1];
typedef char SIG_CHECK_MY_ACCOUNT_IN_DB_GetOutBuffer_Size_Must_Be_0x29[
    (sizeof(SIG_CHECK_MY_ACCOUNT_IN_DB) == 0x29) ? 1 : -1];
typedef char SIG_SECOND_USER_DATA_GetOutBuffer_Size_Must_Be_0x14[
    (sizeof(SIG_SECOND_USER_DATA) == 0x14) ? 1 : -1];
typedef char SIG_INSERT_AUTO_PUNISH_FIRST_USER_GetOutBuffer_Size_Must_Be_0x2C[
    (sizeof(SIG_INSERT_AUTO_PUNISH_FIRST_USER) == 0x2C) ? 1 : -1];
typedef char SIG_INSERT_AUTO_PUNISH_SECOND_USER_GetOutBuffer_Size_Must_Be_0x14[
    (sizeof(SIG_INSERT_AUTO_PUNISH_SECOND_USER) == 0x14) ? 1 : -1];
typedef char SIG_CERASHOP_PURCHASE_COUNT_GetOutBuffer_Size_Must_Be_0x10[
    (sizeof(SIG_CERASHOP_PURCHASE_COUNT) == 0x10) ? 1 : -1];
typedef char SIG_SECURITY_CARD_ADD_CERT_CNT_GetOutBuffer_Size_Must_Be_0x4[
    (sizeof(SIG_SECURITY_CARD_ADD_CERT_CNT) == 0x4) ? 1 : -1];
typedef char SIG_SECURITY_CARD_DISUSE_GetOutBuffer_Size_Must_Be_0x4[
    (sizeof(SIG_SECURITY_CARD_DISUSE) == 0x4) ? 1 : -1];
typedef char SIG_AVATAR_CHANGE_OWNER_GetOutBuffer_Size_Must_Be_0xD[
    (sizeof(SIG_AVATAR_CHANGE_OWNER) == 0xD) ? 1 : -1];
typedef char SIG_AVATAR_INSERT_GetOutBuffer_Size_Must_Be_0x53[
    (sizeof(SIG_AVATAR_INSERT) == 0x53) ? 1 : -1];
typedef char SIG_LOAD_DUNGEON_CLEAR_GetOutBuffer_Size_Must_Be_0x1810[
    (sizeof(SIG_LOAD_DUNGEON_CLEAR) == 0x1810) ? 1 : -1];
typedef char SIG_SAVE_DUNGEON_CLEAR_GetOutBuffer_Size_Must_Be_0x180C[
    (sizeof(SIG_SAVE_DUNGEON_CLEAR) == 0x180C) ? 1 : -1];
typedef char SIG_MAILBOX_SEND_NEW_MAIL_GetOutBuffer_Size_Must_Be_0x16C[
    (sizeof(SIG_MAILBOX_SEND_NEW_MAIL) == 0x16C) ? 1 : -1];
typedef char SIG_SAVE_PVP_GetOutBuffer_Size_Must_Be_0x88[
    (sizeof(SIG_SAVE_PVP) == 0x88) ? 1 : -1];
typedef char SIG_REQUEST_CHARAC_SKILL_INFO_GetOutBuffer_Size_Must_Be_0x1A0[
    (sizeof(SIG_REQUEST_CHARAC_SKILL_INFO) == 0x1A0) ? 1 : -1];
typedef char SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD_GetOutBuffer_Size_Must_Be_0x1A8[
    (sizeof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD) == 0x1A8) ? 1 : -1];
typedef char SIG_BREAK_AWAY_UV_LIST_GetOutBuffer_Size_Must_Be_0x1F[
    (sizeof(SIG_BREAK_AWAY_UV_LIST) == 0x1F) ? 1 : -1];
typedef char SIG_SAVE_PC_ROOM_PLAY_TIME_GetOutBuffer_Size_Must_Be_0x9[
    (sizeof(SIG_SAVE_PC_ROOM_PLAY_TIME) == 0x9) ? 1 : -1];
typedef char SIG_UPDATE_AVATAR_JEWEL_SOCKET_GetOutBuffer_Size_Must_Be_0x28[
    (sizeof(SIG_UPDATE_AVATAR_JEWEL_SOCKET) == 0x28) ? 1 : -1];
typedef char SIG_DELETE_CHANNEL_GetOutBuffer_Size_Must_Be_0xC[
    (sizeof(SIG_DELETE_CHANNEL) == 0xC) ? 1 : -1];
typedef char SIG_INSERT_CHANNEL_GetOutBuffer_Size_Must_Be_0x5C[
    (sizeof(SIG_INSERT_CHANNEL) == 0x5C) ? 1 : -1];
typedef char SIG_UPDATE_STATISTIC4LEV_GetOutBuffer_Size_Must_Match_Config[
    (sizeof(SIG_UPDATE_STATISTIC4LEV) ==
     sizeof(int32_t) *
         (3 + oldsrv::config::kChannelLevelStatisticCount))
        ? 1
        : -1];
typedef char SIG_SAVE_LOGGAMECHANNEL_GetOutBuffer_Size_Must_Be_0x3B[
    (sizeof(SIG_SAVE_LOGGAMECHANNEL) == 0x3B) ? 1 : -1];
typedef char SIG_UPDATE_CHANNEL_GetOutBuffer_Size_Must_Be_0x3A[
    (sizeof(SIG_UPDATE_CHANNEL) == 0x3A) ? 1 : -1];
typedef char SIG_UPDATE_DETAIL_CHANNEL_GetOutBuffer_Size_Must_Be_0x10[
    (sizeof(SIG_UPDATE_DETAIL_CHANNEL) == 0x10) ? 1 : -1];
typedef char private_store_SIG_PRIVATE_STORE_GetOutBuffer_Size_Must_Be_0x434[
    (sizeof(private_store::SIG_PRIVATE_STORE) == 0x434) ? 1 : -1];
typedef char SIG_MERCENARY_DB_DATA_GetOutBuffer_Size_Must_Be_0xBD4[
    (sizeof(SIG_MERCENARY_DB_DATA) == 0xBD4) ? 1 : -1];
typedef char SIG_OBJECT_BRING_UP_GetOutBuffer_Size_Must_Be_0x10[
    (sizeof(SIG_OBJECT_BRING_UP) == 0x10) ? 1 : -1];
typedef char Packet_PvPResult_GetOutBuffer_Size_Must_Be_0x80[
    (sizeof(Packet_PvPResult) == 0x80) ? 1 : -1];
typedef char SIG_MAILBOX_CHECK_RECEIVER_GetOutBuffer_Size_Must_Be_0x3FDE[
    (sizeof(SIG_MAILBOX_CHECK_RECEIVER) == 0x3FDE) ? 1 : -1];
typedef char SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL_GetOutBuffer_Size_Must_Be_0x3A9[
    (sizeof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL) == 0x3A9) ? 1 : -1];

// IDA_EA: 0x845315E
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_UPDATE_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_UPDATE_SERVER_MESSAGE * Stream::GetOutBuffer<SIG_UPDATE_SERVER_MESSAGE>(void)
template <>
SIG_UPDATE_SERVER_MESSAGE*
Stream::GetOutBuffer<SIG_UPDATE_SERVER_MESSAGE>() {
    const int bytes = sizeof(SIG_UPDATE_SERVER_MESSAGE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_SERVER_MESSAGE* result =
        reinterpret_cast<SIG_UPDATE_SERVER_MESSAGE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453270
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_SELECT_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_SELECT_SERVER_MESSAGE * Stream::GetOutBuffer<SIG_SELECT_SERVER_MESSAGE>(void)
template <>
SIG_SELECT_SERVER_MESSAGE*
Stream::GetOutBuffer<SIG_SELECT_SERVER_MESSAGE>() {
    const int bytes = sizeof(SIG_SELECT_SERVER_MESSAGE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SELECT_SERVER_MESSAGE* result =
        reinterpret_cast<SIG_SELECT_SERVER_MESSAGE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x814278C
// IDA_NAME: _ZN6Stream12GetOutBufferIN12advancealtar25_CharacAdvanceAltarDbDataEEEPT_v
// DEMANGLED: advancealtar::_CharacAdvanceAltarDbData * Stream::GetOutBuffer<advancealtar::_CharacAdvanceAltarDbData>(void)
template <>
advancealtar::_CharacAdvanceAltarDbData*
Stream::GetOutBuffer<advancealtar::_CharacAdvanceAltarDbData>() {
    const int bytes = sizeof(advancealtar::_CharacAdvanceAltarDbData);
    if (!enable_out(bytes)) {
        return 0;
    }
    advancealtar::_CharacAdvanceAltarDbData* result =
        reinterpret_cast<advancealtar::_CharacAdvanceAltarDbData*>(
            get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8139518
// IDA_NAME: _ZN6Stream11GetInBufferIN12advancealtar25_CharacAdvanceAltarDbDataEEEPT_v
// DEMANGLED: advancealtar::_CharacAdvanceAltarDbData * Stream::GetInBuffer<advancealtar::_CharacAdvanceAltarDbData>(void)
template <>
advancealtar::_CharacAdvanceAltarDbData*
Stream::GetInBuffer<advancealtar::_CharacAdvanceAltarDbData>() {
    const int bytes = sizeof(advancealtar::_CharacAdvanceAltarDbData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    advancealtar::_CharacAdvanceAltarDbData* result =
        reinterpret_cast<advancealtar::_CharacAdvanceAltarDbData*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x81427E0
// IDA_NAME: _ZN6Stream12GetOutBufferI26STAdvanceAltarEntranceDataEEPT_v
// DEMANGLED: STAdvanceAltarEntranceData * Stream::GetOutBuffer<STAdvanceAltarEntranceData>(void)
template <>
STAdvanceAltarEntranceData*
Stream::GetOutBuffer<STAdvanceAltarEntranceData>() {
    const int bytes = sizeof(STAdvanceAltarEntranceData);
    if (!enable_out(bytes)) {
        return 0;
    }
    STAdvanceAltarEntranceData* result =
        reinterpret_cast<STAdvanceAltarEntranceData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8142834
// IDA_NAME: _ZN6Stream12GetOutBufferI27STAdvanceAltarLayerStatDataEEPT_v
// DEMANGLED: STAdvanceAltarLayerStatData * Stream::GetOutBuffer<STAdvanceAltarLayerStatData>(void)
template <>
STAdvanceAltarLayerStatData*
Stream::GetOutBuffer<STAdvanceAltarLayerStatData>() {
    const int bytes = sizeof(STAdvanceAltarLayerStatData);
    if (!enable_out(bytes)) {
        return 0;
    }
    STAdvanceAltarLayerStatData* result =
        reinterpret_cast<STAdvanceAltarLayerStatData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x81584A2
// IDA_NAME: _ZN6Stream11GetInBufferI26STAdvanceAltarEntranceDataEEPT_v
// DEMANGLED: STAdvanceAltarEntranceData * Stream::GetInBuffer<STAdvanceAltarEntranceData>(void)
template <>
STAdvanceAltarEntranceData*
Stream::GetInBuffer<STAdvanceAltarEntranceData>() {
    const int bytes = sizeof(STAdvanceAltarEntranceData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    STAdvanceAltarEntranceData* result =
        reinterpret_cast<STAdvanceAltarEntranceData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815853E
// IDA_NAME: _ZN6Stream11GetInBufferI27STAdvanceAltarLayerStatDataEEPT_v
// DEMANGLED: STAdvanceAltarLayerStatData * Stream::GetInBuffer<STAdvanceAltarLayerStatData>(void)
template <>
STAdvanceAltarLayerStatData*
Stream::GetInBuffer<STAdvanceAltarLayerStatData>() {
    const int bytes = sizeof(STAdvanceAltarLayerStatData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    STAdvanceAltarLayerStatData* result =
        reinterpret_cast<STAdvanceAltarLayerStatData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84549DC
// IDA_NAME: _ZN6Stream12GetOutBufferI20SIG_CHECK_ADD_FRIENDEEPT_v
// DEMANGLED: SIG_CHECK_ADD_FRIEND * Stream::GetOutBuffer<SIG_CHECK_ADD_FRIEND>(void)
template <>
SIG_CHECK_ADD_FRIEND* Stream::GetOutBuffer<SIG_CHECK_ADD_FRIEND>() {
    const int bytes = sizeof(SIG_CHECK_ADD_FRIEND);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_ADD_FRIEND* result =
        reinterpret_cast<SIG_CHECK_ADD_FRIEND*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8239A82
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_CHECK_ADD_FRIENDEEPT_v
// DEMANGLED: SIG_CHECK_ADD_FRIEND * Stream::GetInBuffer<SIG_CHECK_ADD_FRIEND>(void)
template <>
SIG_CHECK_ADD_FRIEND* Stream::GetInBuffer<SIG_CHECK_ADD_FRIEND>() {
    const int bytes = sizeof(SIG_CHECK_ADD_FRIEND);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_ADD_FRIEND* result =
        reinterpret_cast<SIG_CHECK_ADD_FRIEND*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453398
// IDA_NAME: _ZN6Stream12GetOutBufferI32SIG_INSERT_SERVER_LOAD_STATISTICEEPT_v
// DEMANGLED: SIG_INSERT_SERVER_LOAD_STATISTIC * Stream::GetOutBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>(void)
template <>
SIG_INSERT_SERVER_LOAD_STATISTIC*
Stream::GetOutBuffer<SIG_INSERT_SERVER_LOAD_STATISTIC>() {
    const int bytes = sizeof(SIG_INSERT_SERVER_LOAD_STATISTIC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_SERVER_LOAD_STATISTIC* result =
        reinterpret_cast<SIG_INSERT_SERVER_LOAD_STATISTIC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84523B8
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_VALUE_STATISTICEEPT_v
// DEMANGLED: SIG_VALUE_STATISTIC * Stream::GetOutBuffer<SIG_VALUE_STATISTIC>(void)
template <>
SIG_VALUE_STATISTIC* Stream::GetOutBuffer<SIG_VALUE_STATISTIC>() {
    const int bytes = sizeof(SIG_VALUE_STATISTIC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_VALUE_STATISTIC* result =
        reinterpret_cast<SIG_VALUE_STATISTIC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452FF4
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_CIRCULATION_STATISTICEEPT_v
// DEMANGLED: SIG_CIRCULATION_STATISTIC * Stream::GetOutBuffer<SIG_CIRCULATION_STATISTIC>(void)
template <>
SIG_CIRCULATION_STATISTIC*
Stream::GetOutBuffer<SIG_CIRCULATION_STATISTIC>() {
    const int bytes = sizeof(SIG_CIRCULATION_STATISTIC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CIRCULATION_STATISTIC* result =
        reinterpret_cast<SIG_CIRCULATION_STATISTIC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453074
// IDA_NAME: _ZN6Stream12GetOutBufferI41SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROLEEPT_v
// DEMANGLED: SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL * Stream::GetOutBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>(void)
template <>
SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL*
Stream::GetOutBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>() {
    const int bytes = sizeof(SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL* result =
        reinterpret_cast<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL*>(
            get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453C64
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_INSERT_RESPONSE_TIMEEEPT_v
// DEMANGLED: SIG_INSERT_RESPONSE_TIME * Stream::GetOutBuffer<SIG_INSERT_RESPONSE_TIME>(void)
template <>
SIG_INSERT_RESPONSE_TIME* Stream::GetOutBuffer<SIG_INSERT_RESPONSE_TIME>() {
    const int bytes = sizeof(SIG_INSERT_RESPONSE_TIME);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_RESPONSE_TIME* result =
        reinterpret_cast<SIG_INSERT_RESPONSE_TIME*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84518FE
// IDA_NAME: _ZN6Stream12GetOutBufferI16SIG_MAILBOX_LISTEEPT_v
// DEMANGLED: SIG_MAILBOX_LIST * Stream::GetOutBuffer<SIG_MAILBOX_LIST>(void)
template <>
SIG_MAILBOX_LIST* Stream::GetOutBuffer<SIG_MAILBOX_LIST>() {
    const int bytes = sizeof(SIG_MAILBOX_LIST);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MAILBOX_LIST* result =
        reinterpret_cast<SIG_MAILBOX_LIST*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x818FBB4
// IDA_NAME: _ZN6Stream12GetOutBufferI14SIG_LOGIN_DATAEEPT_v
// DEMANGLED: SIG_LOGIN_DATA * Stream::GetOutBuffer<SIG_LOGIN_DATA>(void)
template <>
SIG_LOGIN_DATA* Stream::GetOutBuffer<SIG_LOGIN_DATA>() {
    const int bytes = sizeof(SIG_LOGIN_DATA);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOGIN_DATA* result = reinterpret_cast<SIG_LOGIN_DATA*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x817A522
// IDA_NAME: _ZN6Stream12GetOutBufferI22stCeraShopIPGRequest_tEEPT_v
// DEMANGLED: stCeraShopIPGRequest_t * Stream::GetOutBuffer<stCeraShopIPGRequest_t>(void)
template <>
stCeraShopIPGRequest_t* Stream::GetOutBuffer<stCeraShopIPGRequest_t>() {
    const int bytes = sizeof(stCeraShopIPGRequest_t);
    if (!enable_out(bytes)) {
        return 0;
    }
    stCeraShopIPGRequest_t* result =
        reinterpret_cast<stCeraShopIPGRequest_t*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452D66
// IDA_NAME: _ZN6Stream12GetOutBufferI38SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFOEEPT_v
// DEMANGLED: SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO * Stream::GetOutBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>(void)
template <>
SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO*
Stream::GetOutBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>() {
    const int bytes = sizeof(SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO* result =
        reinterpret_cast<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO*>(
            get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453D22
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_ITEM_MAKING_SKILLEEPT_v
// DEMANGLED: SIG_ITEM_MAKING_SKILL * Stream::GetOutBuffer<SIG_ITEM_MAKING_SKILL>(void)
template <>
SIG_ITEM_MAKING_SKILL* Stream::GetOutBuffer<SIG_ITEM_MAKING_SKILL>() {
    const int bytes = sizeof(SIG_ITEM_MAKING_SKILL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_ITEM_MAKING_SKILL* result =
        reinterpret_cast<SIG_ITEM_MAKING_SKILL*>(
            get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453D76
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_LOAD_KILL_MONSTER_INFOEEPT_v
// DEMANGLED: SIG_LOAD_KILL_MONSTER_INFO * Stream::GetOutBuffer<SIG_LOAD_KILL_MONSTER_INFO>(void)
template <>
SIG_LOAD_KILL_MONSTER_INFO* Stream::GetOutBuffer<SIG_LOAD_KILL_MONSTER_INFO>() {
    const int bytes = sizeof(SIG_LOAD_KILL_MONSTER_INFO);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_KILL_MONSTER_INFO* result =
        reinterpret_cast<SIG_LOAD_KILL_MONSTER_INFO*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453DCA
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_EXTEND_CHARAC_SLOTEEPT_v
// DEMANGLED: SIG_EXTEND_CHARAC_SLOT * Stream::GetOutBuffer<SIG_EXTEND_CHARAC_SLOT>(void)
template <>
SIG_EXTEND_CHARAC_SLOT* Stream::GetOutBuffer<SIG_EXTEND_CHARAC_SLOT>() {
    const int bytes = sizeof(SIG_EXTEND_CHARAC_SLOT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_EXTEND_CHARAC_SLOT* result =
        reinterpret_cast<SIG_EXTEND_CHARAC_SLOT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451A52
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM * Stream::GetOutBuffer<SIG_CREATURE_ITEM>(void)
template <>
SIG_CREATURE_ITEM* Stream::GetOutBuffer<SIG_CREATURE_ITEM>() {
    const int bytes = sizeof(SIG_CREATURE_ITEM);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CREATURE_ITEM* result =
        reinterpret_cast<SIG_CREATURE_ITEM*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451AA6
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_CREATURE_ITEM_LISTEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM_LIST * Stream::GetOutBuffer<SIG_CREATURE_ITEM_LIST>(void)
template <>
SIG_CREATURE_ITEM_LIST* Stream::GetOutBuffer<SIG_CREATURE_ITEM_LIST>() {
    const int bytes = sizeof(SIG_CREATURE_ITEM_LIST);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CREATURE_ITEM_LIST* result =
        reinterpret_cast<SIG_CREATURE_ITEM_LIST*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451AFA
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_SWAP_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_SWAP_CREATURE_ITEM * Stream::GetOutBuffer<SIG_SWAP_CREATURE_ITEM>(void)
template <>
SIG_SWAP_CREATURE_ITEM* Stream::GetOutBuffer<SIG_SWAP_CREATURE_ITEM>() {
    const int bytes = sizeof(SIG_SWAP_CREATURE_ITEM);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SWAP_CREATURE_ITEM* result =
        reinterpret_cast<SIG_SWAP_CREATURE_ITEM*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451B4E
// IDA_NAME: _ZN6Stream12GetOutBufferI23SIG_FORBID_USER_TO_PLAYEEPT_v
// DEMANGLED: SIG_FORBID_USER_TO_PLAY * Stream::GetOutBuffer<SIG_FORBID_USER_TO_PLAY>(void)
template <>
SIG_FORBID_USER_TO_PLAY* Stream::GetOutBuffer<SIG_FORBID_USER_TO_PLAY>() {
    const int bytes = sizeof(SIG_FORBID_USER_TO_PLAY);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_FORBID_USER_TO_PLAY* result =
        reinterpret_cast<SIG_FORBID_USER_TO_PLAY*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x82399AE
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_LOAD_KILL_MONSTER_INFOEEPT_v
// DEMANGLED: SIG_LOAD_KILL_MONSTER_INFO * Stream::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>(void)
template <>
SIG_LOAD_KILL_MONSTER_INFO* Stream::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>() {
    const int bytes = sizeof(SIG_LOAD_KILL_MONSTER_INFO);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_KILL_MONSTER_INFO* result =
        reinterpret_cast<SIG_LOAD_KILL_MONSTER_INFO*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x817A9AA
// IDA_NAME: _ZN6Stream11GetInBufferI22stCeraShopIPGRequest_tEEPT_v
// DEMANGLED: stCeraShopIPGRequest_t * Stream::GetInBuffer<stCeraShopIPGRequest_t>(void)
template <>
stCeraShopIPGRequest_t* Stream::GetInBuffer<stCeraShopIPGRequest_t>() {
    const int bytes = sizeof(stCeraShopIPGRequest_t);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    stCeraShopIPGRequest_t* result =
        reinterpret_cast<stCeraShopIPGRequest_t*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8344FFC
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_SWAP_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_SWAP_CREATURE_ITEM * Stream::GetInBuffer<SIG_SWAP_CREATURE_ITEM>(void)
template <>
SIG_SWAP_CREATURE_ITEM* Stream::GetInBuffer<SIG_SWAP_CREATURE_ITEM>() {
    const int bytes = sizeof(SIG_SWAP_CREATURE_ITEM);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SWAP_CREATURE_ITEM* result =
        reinterpret_cast<SIG_SWAP_CREATURE_ITEM*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8345066
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM * Stream::GetInBuffer<SIG_CREATURE_ITEM>(void)
template <>
SIG_CREATURE_ITEM* Stream::GetInBuffer<SIG_CREATURE_ITEM>() {
    const int bytes = sizeof(SIG_CREATURE_ITEM);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CREATURE_ITEM* result =
        reinterpret_cast<SIG_CREATURE_ITEM*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x834546E
// IDA_NAME: _ZN6Stream11GetInBufferI22SIG_CREATURE_ITEM_LISTEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM_LIST * Stream::GetInBuffer<SIG_CREATURE_ITEM_LIST>(void)
template <>
SIG_CREATURE_ITEM_LIST* Stream::GetInBuffer<SIG_CREATURE_ITEM_LIST>() {
    const int bytes = sizeof(SIG_CREATURE_ITEM_LIST);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CREATURE_ITEM_LIST* result =
        reinterpret_cast<SIG_CREATURE_ITEM_LIST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x86A039C
// IDA_NAME: _ZN6Stream11GetInBufferI38SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFOEEPT_v
// DEMANGLED: SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO * Stream::GetInBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>(void)
template <>
SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO*
Stream::GetInBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>() {
    const int bytes = sizeof(SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO* result =
        reinterpret_cast<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x823AE18
// IDA_NAME: _ZN6Stream11GetInBufferI14SIG_LOGIN_DATAEEPT_v
// DEMANGLED: SIG_LOGIN_DATA * Stream::GetInBuffer<SIG_LOGIN_DATA>(void)
template <>
SIG_LOGIN_DATA* Stream::GetInBuffer<SIG_LOGIN_DATA>() {
    const int bytes = sizeof(SIG_LOGIN_DATA);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOGIN_DATA* result =
        reinterpret_cast<SIG_LOGIN_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456B06
// IDA_NAME: _ZN6Stream11GetInBufferI34SIG_AUTO_MARKET_CONDITIONS_CONTROLEEPT_v
// DEMANGLED: SIG_AUTO_MARKET_CONDITIONS_CONTROL * Stream::GetInBuffer<SIG_AUTO_MARKET_CONDITIONS_CONTROL>(void)
template <>
SIG_AUTO_MARKET_CONDITIONS_CONTROL*
Stream::GetInBuffer<SIG_AUTO_MARKET_CONDITIONS_CONTROL>() {
    const int bytes = sizeof(SIG_AUTO_MARKET_CONDITIONS_CONTROL);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AUTO_MARKET_CONDITIONS_CONTROL* result =
        reinterpret_cast<SIG_AUTO_MARKET_CONDITIONS_CONTROL*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8456B70
// IDA_NAME: _ZN6Stream11GetInBufferI41SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROLEEPT_v
// DEMANGLED: SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL * Stream::GetInBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>(void)
template <>
SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL*
Stream::GetInBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>() {
    const int bytes = sizeof(SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL* result =
        reinterpret_cast<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454CF4
// IDA_NAME: _ZN6Stream11GetInBufferI17SIG_AVATAR_INSERTEEPT_v
// DEMANGLED: SIG_AVATAR_INSERT * Stream::GetInBuffer<SIG_AVATAR_INSERT>(void)
template <>
SIG_AVATAR_INSERT* Stream::GetInBuffer<SIG_AVATAR_INSERT>() {
    const int bytes = sizeof(SIG_AVATAR_INSERT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_AVATAR_INSERT* result =
        reinterpret_cast<SIG_AVATAR_INSERT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8550796
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_SAVE_LOGGAMECHANNELEEPT_v
// DEMANGLED: SIG_SAVE_LOGGAMECHANNEL * Stream::GetInBuffer<SIG_SAVE_LOGGAMECHANNEL>(void)
template <>
SIG_SAVE_LOGGAMECHANNEL*
Stream::GetInBuffer<SIG_SAVE_LOGGAMECHANNEL>() {
    const int bytes = sizeof(SIG_SAVE_LOGGAMECHANNEL);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_LOGGAMECHANNEL* result =
        reinterpret_cast<SIG_SAVE_LOGGAMECHANNEL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x85707E8
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_INSERT_RESPONSE_TIMEEEPT_v
// DEMANGLED: SIG_INSERT_RESPONSE_TIME * Stream::GetInBuffer<SIG_INSERT_RESPONSE_TIME>(void)
template <>
SIG_INSERT_RESPONSE_TIME*
Stream::GetInBuffer<SIG_INSERT_RESPONSE_TIME>() {
    const int bytes = sizeof(SIG_INSERT_RESPONSE_TIME);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_RESPONSE_TIME* result =
        reinterpret_cast<SIG_INSERT_RESPONSE_TIME*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8184854
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_CREATE_CHARACEEPT_v
// DEMANGLED: SIG_CREATE_CHARAC * Stream::GetOutBuffer<SIG_CREATE_CHARAC>(void)
template <>
SIG_CREATE_CHARAC* Stream::GetOutBuffer<SIG_CREATE_CHARAC>() {
    const int bytes = sizeof(SIG_CREATE_CHARAC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CREATE_CHARAC* result =
        reinterpret_cast<SIG_CREATE_CHARAC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450540
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_DELETE_CHARACEEPT_v
// DEMANGLED: SIG_DELETE_CHARAC * Stream::GetOutBuffer<SIG_DELETE_CHARAC>(void)
template <>
SIG_DELETE_CHARAC* Stream::GetOutBuffer<SIG_DELETE_CHARAC>() {
    const int bytes = sizeof(SIG_DELETE_CHARAC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_DELETE_CHARAC* result =
        reinterpret_cast<SIG_DELETE_CHARAC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845074A
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_UPDATE_CHARAC_VIEWEEPT_v
// DEMANGLED: SIG_UPDATE_CHARAC_VIEW * Stream::GetOutBuffer<SIG_UPDATE_CHARAC_VIEW>(void)
template <>
SIG_UPDATE_CHARAC_VIEW* Stream::GetOutBuffer<SIG_UPDATE_CHARAC_VIEW>() {
    const int bytes = sizeof(SIG_UPDATE_CHARAC_VIEW);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_CHARAC_VIEW* result =
        reinterpret_cast<SIG_UPDATE_CHARAC_VIEW*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845079E
// IDA_NAME: _ZN6Stream12GetOutBufferI15SIG_LOAD_CHARACEEPT_v
// DEMANGLED: SIG_LOAD_CHARAC * Stream::GetOutBuffer<SIG_LOAD_CHARAC>(void)
template <>
SIG_LOAD_CHARAC* Stream::GetOutBuffer<SIG_LOAD_CHARAC>() {
    const int bytes = sizeof(SIG_LOAD_CHARAC);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_CHARAC* result =
        reinterpret_cast<SIG_LOAD_CHARAC*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84507F2
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_LOAD_INVENTORYEEPT_v
// DEMANGLED: SIG_LOAD_INVENTORY * Stream::GetOutBuffer<SIG_LOAD_INVENTORY>(void)
template <>
SIG_LOAD_INVENTORY* Stream::GetOutBuffer<SIG_LOAD_INVENTORY>() {
    const int bytes = sizeof(SIG_LOAD_INVENTORY);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_INVENTORY* result =
        reinterpret_cast<SIG_LOAD_INVENTORY*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84509CA
// IDA_NAME: _ZN6Stream12GetOutBufferI14SIG_LOAD_QUESTEEPT_v
// DEMANGLED: SIG_LOAD_QUEST * Stream::GetOutBuffer<SIG_LOAD_QUEST>(void)
template <>
SIG_LOAD_QUEST* Stream::GetOutBuffer<SIG_LOAD_QUEST>() {
    const int bytes = sizeof(SIG_LOAD_QUEST);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_QUEST* result =
        reinterpret_cast<SIG_LOAD_QUEST*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452AEE
// IDA_NAME: _ZN6Stream12GetOutBufferI33SIG_REQ_LOAD_CHARAC_HOTKEY_OPTIONEEPT_v
// DEMANGLED: SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION * Stream::GetOutBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>(void)
template <>
SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION*
Stream::GetOutBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>() {
    const int bytes = sizeof(SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* result =
        reinterpret_cast<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453E88
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_CHECK_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHECK_CHARAC_NAME * Stream::GetOutBuffer<SIG_CHECK_CHARAC_NAME>(void)
template <>
SIG_CHECK_CHARAC_NAME* Stream::GetOutBuffer<SIG_CHECK_CHARAC_NAME>() {
    const int bytes = sizeof(SIG_CHECK_CHARAC_NAME);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_CHARAC_NAME* result =
        reinterpret_cast<SIG_CHECK_CHARAC_NAME*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845429C
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_CHECK_CHAR_MIDEEPT_v
// DEMANGLED: SIG_CHECK_CHAR_MID * Stream::GetOutBuffer<SIG_CHECK_CHAR_MID>(void)
template <>
SIG_CHECK_CHAR_MID* Stream::GetOutBuffer<SIG_CHECK_CHAR_MID>() {
    const int bytes = sizeof(SIG_CHECK_CHAR_MID);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_CHAR_MID* result =
        reinterpret_cast<SIG_CHECK_CHAR_MID*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453E1E
// IDA_NAME: _ZN6Stream12GetOutBufferI20SIG_GET_USER_REGDATEEEPT_v
// DEMANGLED: SIG_GET_USER_REGDATE * Stream::GetOutBuffer<SIG_GET_USER_REGDATE>(void)
template <>
SIG_GET_USER_REGDATE* Stream::GetOutBuffer<SIG_GET_USER_REGDATE>() {
    const int bytes = sizeof(SIG_GET_USER_REGDATE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_GET_USER_REGDATE* result =
        reinterpret_cast<SIG_GET_USER_REGDATE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454514
// IDA_NAME: _ZN6Stream12GetOutBufferI27SIG_REQUEST_TAG_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_TAG_CHARAC_INFO * Stream::GetOutBuffer<SIG_REQUEST_TAG_CHARAC_INFO>(void)
template <>
SIG_REQUEST_TAG_CHARAC_INFO*
Stream::GetOutBuffer<SIG_REQUEST_TAG_CHARAC_INFO>() {
    const int bytes = sizeof(SIG_REQUEST_TAG_CHARAC_INFO);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_REQUEST_TAG_CHARAC_INFO* result =
        reinterpret_cast<SIG_REQUEST_TAG_CHARAC_INFO*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84520D4
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_QUERY_CHARAC_INFOEEPT_v
// DEMANGLED: SIG_QUERY_CHARAC_INFO * Stream::GetOutBuffer<SIG_QUERY_CHARAC_INFO>(void)
template <>
SIG_QUERY_CHARAC_INFO* Stream::GetOutBuffer<SIG_QUERY_CHARAC_INFO>() {
    const int bytes = sizeof(SIG_QUERY_CHARAC_INFO);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_QUERY_CHARAC_INFO* result =
        reinterpret_cast<SIG_QUERY_CHARAC_INFO*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845206A
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_CHANGE_CHARAC_NAMEEEPT_v
// DEMANGLED: SIG_CHANGE_CHARAC_NAME * Stream::GetOutBuffer<SIG_CHANGE_CHARAC_NAME>(void)
template <>
SIG_CHANGE_CHARAC_NAME* Stream::GetOutBuffer<SIG_CHANGE_CHARAC_NAME>() {
    const int bytes = sizeof(SIG_CHANGE_CHARAC_NAME);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHANGE_CHARAC_NAME* result =
        reinterpret_cast<SIG_CHANGE_CHARAC_NAME*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8186ED4
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad19SigAradEventItemLogEEEPT_v
// DEMANGLED: arad::SigAradEventItemLog * Stream::GetOutBuffer<arad::SigAradEventItemLog>(void)
template <>
arad::SigAradEventItemLog*
Stream::GetOutBuffer<arad::SigAradEventItemLog>() {
    const int bytes = sizeof(arad::SigAradEventItemLog);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigAradEventItemLog* result =
        reinterpret_cast<arad::SigAradEventItemLog*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8186F28
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad22SigSetCharacCreateTimeEEEPT_v
// DEMANGLED: arad::SigSetCharacCreateTime * Stream::GetOutBuffer<arad::SigSetCharacCreateTime>(void)
template <>
arad::SigSetCharacCreateTime*
Stream::GetOutBuffer<arad::SigSetCharacCreateTime>() {
    const int bytes = sizeof(arad::SigSetCharacCreateTime);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigSetCharacCreateTime* result =
        reinterpret_cast<arad::SigSetCharacCreateTime*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8186F7C
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad22SigLoadRewardEventItemEEEPT_v
// DEMANGLED: arad::SigLoadRewardEventItem * Stream::GetOutBuffer<arad::SigLoadRewardEventItem>(void)
template <>
arad::SigLoadRewardEventItem*
Stream::GetOutBuffer<arad::SigLoadRewardEventItem>() {
    const int bytes = sizeof(arad::SigLoadRewardEventItem);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigLoadRewardEventItem* result =
        reinterpret_cast<arad::SigLoadRewardEventItem*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8186FD0
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad22SigSaveRewardEventItemEEEPT_v
// DEMANGLED: arad::SigSaveRewardEventItem * Stream::GetOutBuffer<arad::SigSaveRewardEventItem>(void)
template <>
arad::SigSaveRewardEventItem*
Stream::GetOutBuffer<arad::SigSaveRewardEventItem>() {
    const int bytes = sizeof(arad::SigSaveRewardEventItem);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigSaveRewardEventItem* result =
        reinterpret_cast<arad::SigSaveRewardEventItem*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8187024
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad14SigServerStateEEEPT_v
// DEMANGLED: arad::SigServerState * Stream::GetOutBuffer<arad::SigServerState>(void)
template <>
arad::SigServerState* Stream::GetOutBuffer<arad::SigServerState>() {
    const int bytes = sizeof(arad::SigServerState);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigServerState* result =
        reinterpret_cast<arad::SigServerState*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8187078
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad29SigAradAvatarHiddenOptionSaveEEEPT_v
// DEMANGLED: arad::SigAradAvatarHiddenOptionSave * Stream::GetOutBuffer<arad::SigAradAvatarHiddenOptionSave>(void)
template <>
arad::SigAradAvatarHiddenOptionSave*
Stream::GetOutBuffer<arad::SigAradAvatarHiddenOptionSave>() {
    const int bytes = sizeof(arad::SigAradAvatarHiddenOptionSave);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigAradAvatarHiddenOptionSave* result =
        reinterpret_cast<arad::SigAradAvatarHiddenOptionSave*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x81870CC
// IDA_NAME: _ZN6Stream12GetOutBufferIN4arad26SigAradAvatarEnduranceSaveEEEPT_v
// DEMANGLED: arad::SigAradAvatarEnduranceSave * Stream::GetOutBuffer<arad::SigAradAvatarEnduranceSave>(void)
template <>
arad::SigAradAvatarEnduranceSave*
Stream::GetOutBuffer<arad::SigAradAvatarEnduranceSave>() {
    const int bytes = sizeof(arad::SigAradAvatarEnduranceSave);
    if (!enable_out(bytes)) {
        return 0;
    }
    arad::SigAradAvatarEnduranceSave* result =
        reinterpret_cast<arad::SigAradAvatarEnduranceSave*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845446C
// IDA_NAME: _ZN6Stream12GetOutBufferI20InformNoticeFlagDataEEPT_v
// DEMANGLED: InformNoticeFlagData * Stream::GetOutBuffer<InformNoticeFlagData>(void)
template <>
InformNoticeFlagData* Stream::GetOutBuffer<InformNoticeFlagData>() {
    const int bytes = sizeof(InformNoticeFlagData);
    if (!enable_out(bytes)) {
        return 0;
    }
    InformNoticeFlagData* result = reinterpret_cast<InformNoticeFlagData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454370
// IDA_NAME: _ZN6Stream12GetOutBufferI23SIG_AVATAR_COLOR_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_COLOR_CHANGE * Stream::GetOutBuffer<SIG_AVATAR_COLOR_CHANGE>(void)
template <>
SIG_AVATAR_COLOR_CHANGE* Stream::GetOutBuffer<SIG_AVATAR_COLOR_CHANGE>() {
    const int bytes = sizeof(SIG_AVATAR_COLOR_CHANGE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_COLOR_CHANGE* result = reinterpret_cast<SIG_AVATAR_COLOR_CHANGE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84503C4
// IDA_NAME: _ZN6Stream12GetOutBufferI15SIG_AVATAR_MOVEEEPT_v
// DEMANGLED: SIG_AVATAR_MOVE * Stream::GetOutBuffer<SIG_AVATAR_MOVE>(void)
template <>
SIG_AVATAR_MOVE* Stream::GetOutBuffer<SIG_AVATAR_MOVE>() {
    const int bytes = sizeof(SIG_AVATAR_MOVE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_MOVE* result = reinterpret_cast<SIG_AVATAR_MOVE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845431C
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_AVATAR_OPTION_CHANGEEEPT_v
// DEMANGLED: SIG_AVATAR_OPTION_CHANGE * Stream::GetOutBuffer<SIG_AVATAR_OPTION_CHANGE>(void)
template <>
SIG_AVATAR_OPTION_CHANGE* Stream::GetOutBuffer<SIG_AVATAR_OPTION_CHANGE>() {
    const int bytes = sizeof(SIG_AVATAR_OPTION_CHANGE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_OPTION_CHANGE* result = reinterpret_cast<SIG_AVATAR_OPTION_CHANGE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452E8E
// IDA_NAME: _ZN6Stream12GetOutBufferI36SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORDEEPT_v
// DEMANGLED: SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD * Stream::GetOutBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>(void)
template <>
SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* Stream::GetOutBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>() {
    const int bytes = sizeof(SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* result = reinterpret_cast<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451E9A
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_BILLING_RECORDEEPT_v
// DEMANGLED: SIG_BILLING_RECORD * Stream::GetOutBuffer<SIG_BILLING_RECORD>(void)
template <>
SIG_BILLING_RECORD* Stream::GetOutBuffer<SIG_BILLING_RECORD>() {
    const int bytes = sizeof(SIG_BILLING_RECORD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_BILLING_RECORD* result = reinterpret_cast<SIG_BILLING_RECORD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84534C0
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_RESTING_USER_RESTRICTEEPT_v
// DEMANGLED: SIG_RESTING_USER_RESTRICT * Stream::GetOutBuffer<SIG_RESTING_USER_RESTRICT>(void)
template <>
SIG_RESTING_USER_RESTRICT* Stream::GetOutBuffer<SIG_RESTING_USER_RESTRICT>() {
    const int bytes = sizeof(SIG_RESTING_USER_RESTRICT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_RESTING_USER_RESTRICT* result = reinterpret_cast<SIG_RESTING_USER_RESTRICT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454004
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_CHECK_USER_SECU_REWARDEEPT_v
// DEMANGLED: SIG_CHECK_USER_SECU_REWARD * Stream::GetOutBuffer<SIG_CHECK_USER_SECU_REWARD>(void)
template <>
SIG_CHECK_USER_SECU_REWARD* Stream::GetOutBuffer<SIG_CHECK_USER_SECU_REWARD>() {
    const int bytes = sizeof(SIG_CHECK_USER_SECU_REWARD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_USER_SECU_REWARD* result = reinterpret_cast<SIG_CHECK_USER_SECU_REWARD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451D5C
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_MOUSE_REGISTEREEPT_v
// DEMANGLED: SIG_MOUSE_REGISTER * Stream::GetOutBuffer<SIG_MOUSE_REGISTER>(void)
template <>
SIG_MOUSE_REGISTER* Stream::GetOutBuffer<SIG_MOUSE_REGISTER>() {
    const int bytes = sizeof(SIG_MOUSE_REGISTER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MOUSE_REGISTER* result = reinterpret_cast<SIG_MOUSE_REGISTER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450ECC
// IDA_NAME: _ZN6Stream12GetOutBufferI16SIG_UPDATE_QUESTEEPT_v
// DEMANGLED: SIG_UPDATE_QUEST * Stream::GetOutBuffer<SIG_UPDATE_QUEST>(void)
template <>
SIG_UPDATE_QUEST* Stream::GetOutBuffer<SIG_UPDATE_QUEST>() {
    const int bytes = sizeof(SIG_UPDATE_QUEST);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_QUEST* result = reinterpret_cast<SIG_UPDATE_QUEST*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845474E
// IDA_NAME: _ZN6Stream12GetOutBufferI15SIG_COMBO_SKILLEEPT_v
// DEMANGLED: SIG_COMBO_SKILL * Stream::GetOutBuffer<SIG_COMBO_SKILL>(void)
template <>
SIG_COMBO_SKILL* Stream::GetOutBuffer<SIG_COMBO_SKILL>() {
    const int bytes = sizeof(SIG_COMBO_SKILL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_COMBO_SKILL* result = reinterpret_cast<SIG_COMBO_SKILL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845299A
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_INSERT_UNLIMIT_SUPPORT_LOGEEPT_v
// DEMANGLED: SIG_INSERT_UNLIMIT_SUPPORT_LOG * Stream::GetOutBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>(void)
template <>
SIG_INSERT_UNLIMIT_SUPPORT_LOG* Stream::GetOutBuffer<SIG_INSERT_UNLIMIT_SUPPORT_LOG>() {
    const int bytes = sizeof(SIG_INSERT_UNLIMIT_SUPPORT_LOG);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_UNLIMIT_SUPPORT_LOG* result = reinterpret_cast<SIG_INSERT_UNLIMIT_SUPPORT_LOG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453826
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_CHECK_MY_ACCOUNT_IN_DBEEPT_v
// DEMANGLED: SIG_CHECK_MY_ACCOUNT_IN_DB * Stream::GetOutBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>(void)
template <>
SIG_CHECK_MY_ACCOUNT_IN_DB* Stream::GetOutBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>() {
    const int bytes = sizeof(SIG_CHECK_MY_ACCOUNT_IN_DB);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CHECK_MY_ACCOUNT_IN_DB* result = reinterpret_cast<SIG_CHECK_MY_ACCOUNT_IN_DB*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453910
// IDA_NAME: _ZN6Stream12GetOutBufferI20SIG_SECOND_USER_DATAEEPT_v
// DEMANGLED: SIG_SECOND_USER_DATA * Stream::GetOutBuffer<SIG_SECOND_USER_DATA>(void)
template <>
SIG_SECOND_USER_DATA* Stream::GetOutBuffer<SIG_SECOND_USER_DATA>() {
    const int bytes = sizeof(SIG_SECOND_USER_DATA);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SECOND_USER_DATA* result = reinterpret_cast<SIG_SECOND_USER_DATA*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84527F2
// IDA_NAME: _ZN6Stream12GetOutBufferI33SIG_INSERT_AUTO_PUNISH_FIRST_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_FIRST_USER * Stream::GetOutBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_FIRST_USER* Stream::GetOutBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>() {
    const int bytes = sizeof(SIG_INSERT_AUTO_PUNISH_FIRST_USER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_AUTO_PUNISH_FIRST_USER* result = reinterpret_cast<SIG_INSERT_AUTO_PUNISH_FIRST_USER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845285C
// IDA_NAME: _ZN6Stream12GetOutBufferI34SIG_INSERT_AUTO_PUNISH_SECOND_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_SECOND_USER * Stream::GetOutBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_SECOND_USER* Stream::GetOutBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>() {
    const int bytes = sizeof(SIG_INSERT_AUTO_PUNISH_SECOND_USER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_AUTO_PUNISH_SECOND_USER* result = reinterpret_cast<SIG_INSERT_AUTO_PUNISH_SECOND_USER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84547A2
// IDA_NAME: _ZN6Stream12GetOutBufferI27SIG_CERASHOP_PURCHASE_COUNTEEPT_v
// DEMANGLED: SIG_CERASHOP_PURCHASE_COUNT * Stream::GetOutBuffer<SIG_CERASHOP_PURCHASE_COUNT>(void)
template <>
SIG_CERASHOP_PURCHASE_COUNT* Stream::GetOutBuffer<SIG_CERASHOP_PURCHASE_COUNT>() {
    const int bytes = sizeof(SIG_CERASHOP_PURCHASE_COUNT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_CERASHOP_PURCHASE_COUNT* result = reinterpret_cast<SIG_CERASHOP_PURCHASE_COUNT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84532DA
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_SECURITY_CARD_ADD_CERT_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_ADD_CERT_CNT * Stream::GetOutBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>(void)
template <>
SIG_SECURITY_CARD_ADD_CERT_CNT* Stream::GetOutBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>() {
    const int bytes = sizeof(SIG_SECURITY_CARD_ADD_CERT_CNT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SECURITY_CARD_ADD_CERT_CNT* result = reinterpret_cast<SIG_SECURITY_CARD_ADD_CERT_CNT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845261E
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_SECURITY_CARD_DISUSEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_DISUSE * Stream::GetOutBuffer<SIG_SECURITY_CARD_DISUSE>(void)
template <>
SIG_SECURITY_CARD_DISUSE* Stream::GetOutBuffer<SIG_SECURITY_CARD_DISUSE>() {
    const int bytes = sizeof(SIG_SECURITY_CARD_DISUSE);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SECURITY_CARD_DISUSE* result = reinterpret_cast<SIG_SECURITY_CARD_DISUSE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450418
// IDA_NAME: _ZN6Stream12GetOutBufferI23SIG_AVATAR_CHANGE_OWNEREEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_OWNER * Stream::GetOutBuffer<SIG_AVATAR_CHANGE_OWNER>(void)
template <>
SIG_AVATAR_CHANGE_OWNER* Stream::GetOutBuffer<SIG_AVATAR_CHANGE_OWNER>() {
    const int bytes = sizeof(SIG_AVATAR_CHANGE_OWNER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_CHANGE_OWNER* result = reinterpret_cast<SIG_AVATAR_CHANGE_OWNER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845035A
// IDA_NAME: _ZN6Stream12GetOutBufferI17SIG_AVATAR_INSERTEEPT_v
// DEMANGLED: SIG_AVATAR_INSERT * Stream::GetOutBuffer<SIG_AVATAR_INSERT>(void)
template <>
SIG_AVATAR_INSERT* Stream::GetOutBuffer<SIG_AVATAR_INSERT>() {
    const int bytes = sizeof(SIG_AVATAR_INSERT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_INSERT* result = reinterpret_cast<SIG_AVATAR_INSERT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450F74
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_LOAD_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_LOAD_DUNGEON_CLEAR * Stream::GetOutBuffer<SIG_LOAD_DUNGEON_CLEAR>(void)
template <>
SIG_LOAD_DUNGEON_CLEAR* Stream::GetOutBuffer<SIG_LOAD_DUNGEON_CLEAR>() {
    const int bytes = sizeof(SIG_LOAD_DUNGEON_CLEAR);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_LOAD_DUNGEON_CLEAR* result = reinterpret_cast<SIG_LOAD_DUNGEON_CLEAR*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450FC8
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_SAVE_DUNGEON_CLEAREEPT_v
// DEMANGLED: SIG_SAVE_DUNGEON_CLEAR * Stream::GetOutBuffer<SIG_SAVE_DUNGEON_CLEAR>(void)
template <>
SIG_SAVE_DUNGEON_CLEAR* Stream::GetOutBuffer<SIG_SAVE_DUNGEON_CLEAR>() {
    const int bytes = sizeof(SIG_SAVE_DUNGEON_CLEAR);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_DUNGEON_CLEAR* result = reinterpret_cast<SIG_SAVE_DUNGEON_CLEAR*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84519FE
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_MAILBOX_SEND_NEW_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL * Stream::GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL* Stream::GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL>() {
    const int bytes = sizeof(SIG_MAILBOX_SEND_NEW_MAIL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MAILBOX_SEND_NEW_MAIL* result = reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450ADC
// IDA_NAME: _ZN6Stream12GetOutBufferI12SIG_SAVE_PVPEEPT_v
// DEMANGLED: SIG_SAVE_PVP * Stream::GetOutBuffer<SIG_SAVE_PVP>(void)
template <>
SIG_SAVE_PVP* Stream::GetOutBuffer<SIG_SAVE_PVP>() {
    const int bytes = sizeof(SIG_SAVE_PVP);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_PVP* result = reinterpret_cast<SIG_SAVE_PVP*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84544C0
// IDA_NAME: _ZN6Stream12GetOutBufferI29SIG_REQUEST_CHARAC_SKILL_INFOEEPT_v
// DEMANGLED: SIG_REQUEST_CHARAC_SKILL_INFO * Stream::GetOutBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>(void)
template <>
SIG_REQUEST_CHARAC_SKILL_INFO* Stream::GetOutBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>() {
    const int bytes = sizeof(SIG_REQUEST_CHARAC_SKILL_INFO);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_REQUEST_CHARAC_SKILL_INFO* result = reinterpret_cast<SIG_REQUEST_CHARAC_SKILL_INFO*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84530DE
// IDA_NAME: _ZN6Stream12GetOutBufferI35SIG_RESULT_PREMIUM_LETHE_SKILL_LOADEEPT_v
// DEMANGLED: SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD * Stream::GetOutBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>(void)
template <>
SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* Stream::GetOutBuffer<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD>() {
    const int bytes = sizeof(SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD* result = reinterpret_cast<SIG_RESULT_PREMIUM_LETHE_SKILL_LOAD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84536E8
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_BREAK_AWAY_UV_LISTEEPT_v
// DEMANGLED: SIG_BREAK_AWAY_UV_LIST * Stream::GetOutBuffer<SIG_BREAK_AWAY_UV_LIST>(void)
template <>
SIG_BREAK_AWAY_UV_LIST* Stream::GetOutBuffer<SIG_BREAK_AWAY_UV_LIST>() {
    const int bytes = sizeof(SIG_BREAK_AWAY_UV_LIST);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_BREAK_AWAY_UV_LIST* result = reinterpret_cast<SIG_BREAK_AWAY_UV_LIST*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8454568
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_SAVE_PC_ROOM_PLAY_TIMEEEPT_v
// DEMANGLED: SIG_SAVE_PC_ROOM_PLAY_TIME * Stream::GetOutBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>(void)
template <>
SIG_SAVE_PC_ROOM_PLAY_TIME* Stream::GetOutBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>() {
    const int bytes = sizeof(SIG_SAVE_PC_ROOM_PLAY_TIME);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_PC_ROOM_PLAY_TIME* result = reinterpret_cast<SIG_SAVE_PC_ROOM_PLAY_TIME*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452DBA
// IDA_NAME: _ZN6Stream12GetOutBufferI30SIG_UPDATE_AVATAR_JEWEL_SOCKETEEPT_v
// DEMANGLED: SIG_UPDATE_AVATAR_JEWEL_SOCKET * Stream::GetOutBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>(void)
template <>
SIG_UPDATE_AVATAR_JEWEL_SOCKET* Stream::GetOutBuffer<SIG_UPDATE_AVATAR_JEWEL_SOCKET>() {
    const int bytes = sizeof(SIG_UPDATE_AVATAR_JEWEL_SOCKET);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_AVATAR_JEWEL_SOCKET* result = reinterpret_cast<SIG_UPDATE_AVATAR_JEWEL_SOCKET*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84504EC
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_DELETE_CHANNELEEPT_v
// DEMANGLED: SIG_DELETE_CHANNEL * Stream::GetOutBuffer<SIG_DELETE_CHANNEL>(void)
template <>
SIG_DELETE_CHANNEL* Stream::GetOutBuffer<SIG_DELETE_CHANNEL>() {
    const int bytes = sizeof(SIG_DELETE_CHANNEL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_DELETE_CHANNEL* result = reinterpret_cast<SIG_DELETE_CHANNEL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450594
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_INSERT_CHANNELEEPT_v
// DEMANGLED: SIG_INSERT_CHANNEL * Stream::GetOutBuffer<SIG_INSERT_CHANNEL>(void)
template <>
SIG_INSERT_CHANNEL* Stream::GetOutBuffer<SIG_INSERT_CHANNEL>() {
    const int bytes = sizeof(SIG_INSERT_CHANNEL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_INSERT_CHANNEL* result = reinterpret_cast<SIG_INSERT_CHANNEL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84512F4
// IDA_NAME: _ZN6Stream12GetOutBufferI24SIG_UPDATE_STATISTIC4LEVEEPT_v
// DEMANGLED: SIG_UPDATE_STATISTIC4LEV * Stream::GetOutBuffer<SIG_UPDATE_STATISTIC4LEV>(void)
template <>
SIG_UPDATE_STATISTIC4LEV* Stream::GetOutBuffer<SIG_UPDATE_STATISTIC4LEV>() {
    const int bytes = sizeof(SIG_UPDATE_STATISTIC4LEV);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_STATISTIC4LEV* result = reinterpret_cast<SIG_UPDATE_STATISTIC4LEV*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84513B2
// IDA_NAME: _ZN6Stream12GetOutBufferI23SIG_SAVE_LOGGAMECHANNELEEPT_v
// DEMANGLED: SIG_SAVE_LOGGAMECHANNEL * Stream::GetOutBuffer<SIG_SAVE_LOGGAMECHANNEL>(void)
template <>
SIG_SAVE_LOGGAMECHANNEL* Stream::GetOutBuffer<SIG_SAVE_LOGGAMECHANNEL>() {
    const int bytes = sizeof(SIG_SAVE_LOGGAMECHANNEL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_LOGGAMECHANNEL* result = reinterpret_cast<SIG_SAVE_LOGGAMECHANNEL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450D7C
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_UPDATE_CHANNELEEPT_v
// DEMANGLED: SIG_UPDATE_CHANNEL * Stream::GetOutBuffer<SIG_UPDATE_CHANNEL>(void)
template <>
SIG_UPDATE_CHANNEL* Stream::GetOutBuffer<SIG_UPDATE_CHANNEL>() {
    const int bytes = sizeof(SIG_UPDATE_CHANNEL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_CHANNEL* result = reinterpret_cast<SIG_UPDATE_CHANNEL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84512A0
// IDA_NAME: _ZN6Stream12GetOutBufferI25SIG_UPDATE_DETAIL_CHANNELEEPT_v
// DEMANGLED: SIG_UPDATE_DETAIL_CHANNEL * Stream::GetOutBuffer<SIG_UPDATE_DETAIL_CHANNEL>(void)
template <>
SIG_UPDATE_DETAIL_CHANNEL* Stream::GetOutBuffer<SIG_UPDATE_DETAIL_CHANNEL>() {
    const int bytes = sizeof(SIG_UPDATE_DETAIL_CHANNEL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_UPDATE_DETAIL_CHANNEL* result = reinterpret_cast<SIG_UPDATE_DETAIL_CHANNEL*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451348
// IDA_NAME: _ZN6Stream12GetOutBufferIN13private_store17SIG_PRIVATE_STOREEEEPT_v
// DEMANGLED: private_store::SIG_PRIVATE_STORE * Stream::GetOutBuffer<private_store::SIG_PRIVATE_STORE>(void)
template <>
private_store::SIG_PRIVATE_STORE* Stream::GetOutBuffer<private_store::SIG_PRIVATE_STORE>() {
    const int bytes = sizeof(private_store::SIG_PRIVATE_STORE);
    if (!enable_out(bytes)) {
        return 0;
    }
    private_store::SIG_PRIVATE_STORE* result = reinterpret_cast<private_store::SIG_PRIVATE_STORE*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84541DE
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_MERCENARY_DB_DATAEEPT_v
// DEMANGLED: SIG_MERCENARY_DB_DATA * Stream::GetOutBuffer<SIG_MERCENARY_DB_DATA>(void)
template <>
SIG_MERCENARY_DB_DATA* Stream::GetOutBuffer<SIG_MERCENARY_DB_DATA>() {
    const int bytes = sizeof(SIG_MERCENARY_DB_DATA);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MERCENARY_DB_DATA* result = reinterpret_cast<SIG_MERCENARY_DB_DATA*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84543C4
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_OBJECT_BRING_UPEEPT_v
// DEMANGLED: SIG_OBJECT_BRING_UP * Stream::GetOutBuffer<SIG_OBJECT_BRING_UP>(void)
template <>
SIG_OBJECT_BRING_UP* Stream::GetOutBuffer<SIG_OBJECT_BRING_UP>() {
    const int bytes = sizeof(SIG_OBJECT_BRING_UP);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_OBJECT_BRING_UP* result = reinterpret_cast<SIG_OBJECT_BRING_UP*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8450BD8
// IDA_NAME: _ZN6Stream12GetOutBufferI17MSG_STAT_BREAKJAREEPT_v
// DEMANGLED: MSG_STAT_BREAKJAR * Stream::GetOutBuffer<MSG_STAT_BREAKJAR>(void)
template <>
MSG_STAT_BREAKJAR* Stream::GetOutBuffer<MSG_STAT_BREAKJAR>() {
    if (!enable_out(12)) {
        return 0;
    }
    MSG_STAT_BREAKJAR* result =
        reinterpret_cast<MSG_STAT_BREAKJAR*>(get_buffer());
    out_ptr(12);
    return result;
}

// IDA_EA: 0x8450C2C
// IDA_NAME: _ZN6Stream12GetOutBufferI19MSG_STAT_COMPOSITONEEPT_v
// DEMANGLED: MSG_STAT_COMPOSITON * Stream::GetOutBuffer<MSG_STAT_COMPOSITON>(void)
template <>
MSG_STAT_COMPOSITON* Stream::GetOutBuffer<MSG_STAT_COMPOSITON>() {
    if (!enable_out(13)) {
        return 0;
    }
    MSG_STAT_COMPOSITON* result =
        reinterpret_cast<MSG_STAT_COMPOSITON*>(get_buffer());
    out_ptr(13);
    return result;
}

// IDA_EA: 0x8450C80
// IDA_NAME: _ZN6Stream12GetOutBufferI21MSG_STAT_DUNGEONCLEAREEPT_v
// DEMANGLED: MSG_STAT_DUNGEONCLEAR * Stream::GetOutBuffer<MSG_STAT_DUNGEONCLEAR>(void)
template <>
MSG_STAT_DUNGEONCLEAR* Stream::GetOutBuffer<MSG_STAT_DUNGEONCLEAR>() {
    if (!enable_out(25)) {
        return 0;
    }
    MSG_STAT_DUNGEONCLEAR* result =
        reinterpret_cast<MSG_STAT_DUNGEONCLEAR*>(get_buffer());
    out_ptr(25);
    return result;
}

// IDA_EA: 0x8450CD4
// IDA_NAME: _ZN6Stream12GetOutBufferI16MSG_STAT_RANKRESEEPT_v
// DEMANGLED: MSG_STAT_RANKRES * Stream::GetOutBuffer<MSG_STAT_RANKRES>(void)
template <>
MSG_STAT_RANKRES* Stream::GetOutBuffer<MSG_STAT_RANKRES>() {
    if (!enable_out(108)) {
        return 0;
    }
    MSG_STAT_RANKRES* result =
        reinterpret_cast<MSG_STAT_RANKRES*>(get_buffer());
    out_ptr(108);
    return result;
}

// IDA_EA: 0x8450D28
// IDA_NAME: _ZN6Stream12GetOutBufferI16MSG_STAT_PVPPLAYEEPT_v
// DEMANGLED: MSG_STAT_PVPPLAY * Stream::GetOutBuffer<MSG_STAT_PVPPLAY>(void)
template <>
MSG_STAT_PVPPLAY* Stream::GetOutBuffer<MSG_STAT_PVPPLAY>() {
    if (!enable_out(16)) {
        return 0;
    }
    MSG_STAT_PVPPLAY* result =
        reinterpret_cast<MSG_STAT_PVPPLAY*>(get_buffer());
    out_ptr(16);
    return result;
}

// IDA_EA: 0x8451DC6
// IDA_NAME: _ZN6Stream12GetOutBufferI33SIG_SECURITY_CARD_UPDATE_FAIL_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_UPDATE_FAIL_CNT * Stream::GetOutBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>(void)
template <>
SIG_SECURITY_CARD_UPDATE_FAIL_CNT*
Stream::GetOutBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>() {
    const int bytes = sizeof(SIG_SECURITY_CARD_UPDATE_FAIL_CNT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SECURITY_CARD_UPDATE_FAIL_CNT* result =
        reinterpret_cast<SIG_SECURITY_CARD_UPDATE_FAIL_CNT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452EF8
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_LOAD_DIMENSIONEEPT_v
// DEMANGLED: SIG_LOAD_DIMENSION * Stream::GetOutBuffer<SIG_LOAD_DIMENSION>(void)
template <>
SIG_LOAD_DIMENSION* Stream::GetOutBuffer<SIG_LOAD_DIMENSION>() {
    if (!enable_out(16)) {
        return 0;
    }
    SIG_LOAD_DIMENSION* result =
        reinterpret_cast<SIG_LOAD_DIMENSION*>(get_buffer());
    out_ptr(16);
    return result;
}

// IDA_EA: 0x8450482
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_AVATAR_CHANGE_STATEEPT_v
// DEMANGLED: SIG_AVATAR_CHANGE_STAT * Stream::GetOutBuffer<SIG_AVATAR_CHANGE_STAT>(void)
template <>
SIG_AVATAR_CHANGE_STAT*
Stream::GetOutBuffer<SIG_AVATAR_CHANGE_STAT>() {
    const int bytes = sizeof(SIG_AVATAR_CHANGE_STAT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_AVATAR_CHANGE_STAT* result =
        reinterpret_cast<SIG_AVATAR_CHANGE_STAT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84546E4
// IDA_NAME: _ZN6Stream12GetOutBufferIN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoEEEPT_v
// DEMANGLED: SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo *
//            Stream::GetOutBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>(void)
template <>
SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*
Stream::GetOutBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>() {
    const int bytes = sizeof(SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo* const result =
        reinterpret_cast<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo*>(
            get_buffer() + out_ptr());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452192
// IDA_NAME: _ZN6Stream12GetOutBufferI33Packet_Mannerless_User_AccusationEEPT_v
// DEMANGLED: Packet_Mannerless_User_Accusation *
//            Stream::GetOutBuffer<Packet_Mannerless_User_Accusation>(void)
template <>
Packet_Mannerless_User_Accusation*
Stream::GetOutBuffer<Packet_Mannerless_User_Accusation>() {
    const int bytes = sizeof(Packet_Mannerless_User_Accusation);
    if (!enable_out(bytes)) {
        return 0;
    }
    Packet_Mannerless_User_Accusation* const result =
        reinterpret_cast<Packet_Mannerless_User_Accusation*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x84521E6
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_MANNERLESS_USEREEPT_v
// DEMANGLED: SIG_MANNERLESS_USER * Stream::GetOutBuffer<SIG_MANNERLESS_USER>(void)
template <>
SIG_MANNERLESS_USER* Stream::GetOutBuffer<SIG_MANNERLESS_USER>() {
    const int bytes = sizeof(SIG_MANNERLESS_USER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MANNERLESS_USER* const result =
        reinterpret_cast<SIG_MANNERLESS_USER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453B10
// IDA_NAME: _ZN6Stream12GetOutBufferI22SIG_REQUEST_JOIN_GUILDEEPT_v
// DEMANGLED: SIG_REQUEST_JOIN_GUILD *
//            Stream::GetOutBuffer<SIG_REQUEST_JOIN_GUILD>(void)
template <>
SIG_REQUEST_JOIN_GUILD* Stream::GetOutBuffer<SIG_REQUEST_JOIN_GUILD>() {
    const int bytes = sizeof(SIG_REQUEST_JOIN_GUILD);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_REQUEST_JOIN_GUILD* const result =
        reinterpret_cast<SIG_REQUEST_JOIN_GUILD*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452E24
// IDA_NAME: _ZN6Stream12GetOutBufferI16Packet_PvPResultEEPT_v
// DEMANGLED: Packet_PvPResult * Stream::GetOutBuffer<Packet_PvPResult>(void)
template <>
Packet_PvPResult* Stream::GetOutBuffer<Packet_PvPResult>() {
    const int bytes = sizeof(Packet_PvPResult);
    if (!enable_out(bytes)) {
        return 0;
    }
    Packet_PvPResult* const result =
        reinterpret_cast<Packet_PvPResult*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x845155A
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_MAILBOX_CHECK_RECEIVEREEPT_v
// DEMANGLED: SIG_MAILBOX_CHECK_RECEIVER *
//            Stream::GetOutBuffer<SIG_MAILBOX_CHECK_RECEIVER>(void)
template <>
SIG_MAILBOX_CHECK_RECEIVER*
Stream::GetOutBuffer<SIG_MAILBOX_CHECK_RECEIVER>() {
    const int bytes = sizeof(SIG_MAILBOX_CHECK_RECEIVER);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MAILBOX_CHECK_RECEIVER* const result =
        reinterpret_cast<SIG_MAILBOX_CHECK_RECEIVER*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453F08
// IDA_NAME: _ZN6Stream12GetOutBufferI34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL *
//            Stream::GetOutBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>(void)
template <>
SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL*
Stream::GetOutBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>() {
    const int bytes = sizeof(SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* const result =
        reinterpret_cast<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL*>(get_buffer());
    out_ptr(bytes);
    return result;
}
