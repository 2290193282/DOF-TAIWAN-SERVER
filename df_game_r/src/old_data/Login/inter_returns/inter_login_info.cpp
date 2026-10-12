#include "old_data/Login/inter_returns/inter_login_info.h"

#include <cstring>
#include <ctime>
#include <string>

#include "core/config/env/environment.h"
#include "infra/security/policy/black_ip_monitor.h"
#include "common/manager/double_conn_check_server_proxy_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/security/policy/restrict_geolocation_abi.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_data/Login/security/auto_punish_black_ip_manager.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/auth_security/db_passpad_update_fail_cnt.h"
#include "old_data/db/event/db_create_dnf_user_info.h"
#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_data/db/game_option/db_load_game_option.h"
#include "old_data/db/inventory/db_load_account_cargo.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/packet/udp_packets.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "old_game/player/session/timer_return_user_timeout.h"

namespace {

// IDA_EA: Inter_LoginInfo::dispatch_sig @0x84C52A3..0x84C5316.
void ApplyLoginRouting(CUser* user, SIG_LOGIN_DATA& login_data) {
    GameWorld* const world = G_GameWorld();
    CEnvironment* const environment = G_CEnvironment();
    if (world && environment &&
        world->GetChannelType() ==
            channel_zombie_matching_producer::kZombieChannelType) {
        // The 0828 client sends its external group enum in the login frame.
        // Type19 was moved under configured service group 3, so bind the live
        // CUser identity to this Game's authoritative routing group before
        // matching/monitor/guild consumers observe it.
        user->SetServerGroup(static_cast<ENUM_SERVER_GROUP>(
            environment->get_server_group()));
    } else if (world && world->GetChannelType() == 14) {
        user->SetServerGroup(
            static_cast<ENUM_SERVER_GROUP>(login_data.online_preliminary_server_group));
    }
    if (world && world->GetChannelType() == 7) {
        user->SetServerGroup(
            static_cast<ENUM_SERVER_GROUP>(login_data.school_broadcast_server_group));
    }
    user->setWebAddress(login_data.sso_login_ip);
}

// IDA calls 0x84C72E8/0x84C742D pass these two bit values.  The enum has no
// recovered policy spellings yet, so the values remain local to this proven
// original consumer rather than creating unproven process-wide policy names.
const WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE kBlackIpTrade =
    static_cast<WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE>(1);
const WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE kBlackIpPrivateSecurity =
    static_cast<WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE>(2);

}  // namespace

// IDA_EA: 0x84C5294
// IDA_NAME: _ZN15Inter_LoginInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoginInfo::dispatch_sig(CUser *,char *,int)
int Inter_LoginInfo::dispatch_sig(CUser* user,
                                  char* payload,
                                  int /*payload_size*/) {
    SIG_LOGIN_DATA& login_data = *reinterpret_cast<SIG_LOGIN_DATA*>(payload);
    ApplyLoginRouting(user, login_data);

    PacketGuard login_reply;
    login_reply.put_header(1, 1);
    if (login_data.fail_code == 1) {
        login_reply.put_byte(0); login_reply.put_byte(6); login_reply.finalize(true);
        user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(11), true, 0);
        return 0;
    }
    if (login_data.fail_code == 8) {
        login_reply.put_byte(0); login_reply.put_byte(96); login_reply.finalize(true);
        user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(33), true, 0);
        return 0;
    }
    if (login_data.fail_code >= 10 && login_data.fail_code <= 12) {
        login_reply.put_byte(0);
        login_reply.put_byte(static_cast<unsigned char>(login_data.fail_code - 38));
        login_reply.put_byte(static_cast<unsigned char>(login_data.punish_duration));
        login_reply.finalize(true); user->Send(login_reply);
        user->DisConnSig(static_cast<DISCONN_SIG>(26), true, 0);
        return 0;
    }
    switch (login_data.fail_code) {
        case 4:
            login_reply.put_byte(0); login_reply.put_byte(8);
            login_reply.put_int(static_cast<int>(
                login_data.punish_rows[login_data.punish_count].end_time));
            login_reply.finalize(true); user->Send(login_reply);
            user->DisConnSig(static_cast<DISCONN_SIG>(26), true, 0);
            return 0;
        case 5:
            login_reply.put_byte(0); login_reply.put_byte(72);
            login_reply.put_byte(static_cast<unsigned char>(login_data.passpad_fail_count));
            login_reply.finalize(true); user->Send(login_reply);
            user->DisConnSig(static_cast<DISCONN_SIG>(26), true, 0);
            return 0;
        case 6:
            login_reply.put_byte(0); login_reply.put_byte(73); login_reply.finalize(true);
            user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(26), true, 0);
            return 0;
        case 9:
            login_reply.put_byte(0); login_reply.put_byte(166); login_reply.finalize(true);
            user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(26), true, 0);
            return 0;
        case 7:
            login_reply.put_byte(0); login_reply.put_byte(95); login_reply.finalize(true);
            user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(32), true, 0);
            return 0;
        default:
            break;
    }

    user->setHangameUser(login_data.hangame_flag != 0);
    if (login_data.fail_code != 0) {
        login_reply.put_byte(0); login_reply.put_byte(1); login_reply.finalize(true);
        user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(14), true, 0);
        return 0;
    }

    // IDA_EA: 0x84C5B98..0x84C5C49: live mappings are duplicate logins, and
    // the original never overwrites the older session in this branch.
    CUser* const old_user = G_CGameManager()->GetUserByAccId(login_data.member_id);
    if (old_user && old_user->network_.get_socket() != -1 && old_user->get_state()) {
        old_user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 18);
        old_user->DisConnSig(static_cast<DISCONN_SIG>(12), true, 0);
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 18);
        return 7192;
    }

    user->set_acc_info(login_data.member_id, login_data.login_user_id);
    char peer_ip[20] = {};
    user->network_.GetPeerIP2(peer_ip, sizeof(peer_ip));
    if (login_data.ip_permission_granted != 1) {
        const std::string peer_ip_text(peer_ip);
        const std::string account_name(user->get_acc_name());
        if (!ARAD::Singleton<RestrictGeolocation>::Get()->isAllow(
                account_name, peer_ip_text)) {
            login_reply.put_byte(0); login_reply.put_byte(65); login_reply.finalize(true);
            user->Send(login_reply); user->DisConnSig(static_cast<DISCONN_SIG>(21), true, 0);
        }
    }
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        char country_code[16] = {};
        ARAD::Singleton<RestrictGeolocation>::Get()->getCountryCode(
            country_code, sizeof(country_code), std::string(peer_ip));
        user->setAllowLocationGeoState(ARAD::Singleton<RestrictGeolocation>::Get()->isAllow(
            std::string(country_code)));
    }

    user->set_state(2);
    G_GameWorld()->InsertLoginUser(user);
    // IDA_EA: 0x84C60F1..0x84C6198.  The original instruction at 0x84C611E
    // writes DB_LoadGameOption's signal 0x12e (302), followed by the full
    // integer CUser::GetUID() result at 0x84C6149.
    CStreamGuard game_option_guard(
        GlobalData::stream_pool()->Acquire("InterDispatcher.cpp", 7294), true);
    *game_option_guard << 302;
    *game_option_guard << user->GetUID();
    game_option_guard->GetInBuffer<SIG_REQ_LOAD_GAME_OPTION>()->m_id = user->get_acc_id();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, game_option_guard);

    G_CGameManager()->insertUserByAccID(user);
    user->SetUserDetailInfo(login_data.user_name, login_data.member_identity, login_data.email);
    user->SetBirth(login_data.member_identity);
    for (uint32_t index = 0; index < login_data.premium_count; ++index) {
        const SIG_LOGIN_DATA::PremiumEntry& premium = login_data.premium_entries[index];
        WongWork::CHandlePremium::handleSetUserPremium(
            user, static_cast<ENUM_PREMIUM_TYPE>(premium.premium_type),
            premium.service_start, premium.service_end, premium.event_id,
            index + 1 == login_data.premium_count);
    }
    user->setRegDate(login_data.register_day);
    user->setRegTime(static_cast<long>(login_data.register_time));
    if (login_data.charac_view_count > 0) {
        user->SetCharacInfo(static_cast<int>(login_data.charac_view_count),
                            reinterpret_cast<char*>(login_data.charac_view));
        user->SetNeedCheckOverEquipItem();
    }
    if (login_data.mercenary_count > 0) {
        user->SetCharacMercenaryInfo(static_cast<int>(login_data.mercenary_count),
                                     login_data.mercenaries);
        bool all_saved = true;
        for (uint32_t index = 0; index < login_data.mercenary_count && index <= 35; ++index) {
            if (login_data.mercenaries[index].save_flag) { all_saved = false; break; }
        }
        if (all_saved) user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x1AAu), 1, 0, 0);
    }
    for (uint16_t index = 0; index < login_data.premium_event_count; ++index)
        user->SetRestrictedGoods(login_data.premium_event_ids[index]);

    CEnvironment* const environment = G_CEnvironment();
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))->SendLogin(
        user->GetUID(), login_data.member_id, environment->get_channel_no(), peer_ip,
        user->getSex(), user->GetSsnString());
    GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))->SendLogin(
        user->GetUID(), login_data.member_id, environment->get_channel_no(), peer_ip,
        user->getSex(), user->GetSsnString());
    GlobalData::s_double_check_proxy->SendLogin(login_data.member_id,
        environment->get_channel_no(), environment->get_server_group());

    user->gold_control_.SetInfo(login_data.gold_control_info);
    const int current_second = GlobalData::system_time()->getCurSec();
    for (uint32_t index = 0; index < login_data.punish_count; ++index) {
        const SIG_LOGIN_DATA::PunishRow& punish = login_data.punish_rows[index];
        if (punish.punish_type > 99 && punish.punish_type <= 199 &&
            (punish.end_time >= static_cast<uint32_t>(current_second) || punish.end_time == 0) &&
            punish.punish_type == 101)
            user->setStdDropRate(punish.punish_value);
    }
    for (uint32_t index = 0; index < login_data.punish_count; ++index) {
        const SIG_LOGIN_DATA::PunishRow& punish = login_data.punish_rows[index];
        if (punish.end_time < static_cast<uint32_t>(current_second) && punish.end_time != 0) continue;
        switch (punish.punish_type) {
            case 4: user->SetTradePunishType(8); break;
            case 11: user->SetTradePunishType(1); break;
            case 12: user->SetTradePunishType(4); break;
            case 13: user->SetTradePunishType(32); break;
            case 14: user->SetTradePunishType(2); break;
            case 103: user->SetTradePunishType(128); break;
            default: break;
        }
    }
    user->setTotalHackUserTradeCnt(login_data.punish_second_trade_count);
    user->setTotalHackUserTradeGold(login_data.punish_second_trade_gold);
    if (GlobalData::g_autoPunishRuleBlackIPMgr->isBlackIP(login_data.login_ip))
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x32Au), 1, 0, 0);
    user->setRestingUserRestrict(login_data.punish_user_marker != 0);
    user->setHumanCertifyErrorCnt(0);
    user->setHumanCertified(true);

    std::tm current_tm;
    const time_t now = static_cast<time_t>(current_second);
    localtime_r(&now, &current_tm);
    user->dungeon_gained_gold_.setGainedGold(login_data.dungeon_gain_gold);
    user->SetReliablePerson(true);
    user->SetReliablePerson(login_data.reliable_flag != 0);
    current_tm.tm_hour = 6;
    const time_t over_trade_reset = mktime(&current_tm);
    user->SetOverTradeGoldCount(login_data.last_play_time <= static_cast<int32_t>(over_trade_reset)
        ? 0 : login_data.trade_gold_daily);
    std::tm last_play_tm;
    const time_t last_play = static_cast<time_t>(login_data.last_play_time);
    localtime_r(&last_play, &last_play_tm);
    user->SetReportMannerlessUserCount(
        last_play_tm.tm_year == current_tm.tm_year && last_play_tm.tm_mon == current_tm.tm_mon &&
        last_play_tm.tm_mday == current_tm.tm_mday ? login_data.report_count : 0);
    user->SetSchoolNo(login_data.school_id);
    user->SetRating(login_data.rating);
    user->SetSchoolName(login_data.school_name);
    user->SetDailyBadge(reinterpret_cast<short*>(login_data.school_badge));
    user->SetTutorialSkipable(login_data.tutorial_skipable);
    CommonTime login_time;
    login_time.SetCurTime();
    user->SetAge(login_time.year - login_data.birth_year + 2001);
    user->setSecuReward(login_data.reward_daily_reset != 0);
    user->setSecuType(login_data.reward_security_flags);
    user->setRewardType(login_data.reward_state);
    user->setSecuGrade(static_cast<char>(login_data.reward_security_flags & 0x27));
    if (login_data.security_card_loaded) {
        user->getSecurityCard()->setSecurityCardInfo(
            login_data.security_card, login_data.security_card_loaded != 0,
            login_data.security_card_valid != 0, login_data.security_card_fail_count,
            login_data.security_card_cert_flag, login_data.security_card_q_pos,
            login_data.security_card_q_pos_text);
    }

    user->getPad()->setVersionInfo(login_data.passpad_result);
    if (login_data.passpad_enabled) {
        user->getPad()->setActivate(true);
        user->getPad()->setCancelCnt(login_data.passpad_cancel_count);
        user->getPad()->setVersionInfo(login_data.passpad_result);
        user->getPad()->setCertified(login_data.passpad_converted != 0);
        user->getPad()->setCheckTime(login_data.passpad_validity_time);
        if (CheckDailyScheduleTime(environment->get_fatigue_time(), login_data.last_play_time,
                                   current_second)) {
            if (login_data.passpad_fail_count > 0) {
                user->getPad()->setFailCnt(0);
                user->getPad()->setCancelCnt(0);
                DB_PassPadUpdateFailCnt::makeRequest(user->get_acc_id(),
                    user->getPad()->getFailCnt(), user->getWebAddress());
            }
        } else user->getPad()->setFailCnt(login_data.passpad_fail_count);
        user->getPad()->setPassword(login_data.encrypted_password);
        user->getHades()->Send_Login();
        user->getHades()->StdCateUpdate(3u, 0);
        user->getPad()->setAdvideChangePwd(login_data.advice_change_password);
    }
    user->getPad()->setCertified(true);
    if (login_data.punish_second_code == 13) user->setPunishTradeAlert(231);
    else if (login_data.punish_second_code == 14) user->setPunishTradeAlert(232);
    else user->setPunishTradeAlert(0);
    user->setOnlinePreliminaryCharacNo(login_data.online_preliminary_charac_no);
    user->setOnlinePreliminaryServerGroup(
        static_cast<ENUM_SERVER_GROUP>(login_data.online_preliminary_server_group));
    user->setOnlinePreliminaryPlayType(static_cast<int>(login_data.pvp_mode_mask));
    user->setTournamentAccount(login_data.current_charac_no != 0 &&
                               login_data.school_broadcast_server_group != 0);
    if (G_GameWorld()->GetChannelType() == 7) {
        Packet_Monitor_UDP_Login monitor_login;
        monitor_login.uid = user->GetUID();
        monitor_login.account_id = user->get_acc_id();
        monitor_login.channel_no = static_cast<uint8_t>(environment->get_channel_no());
        std::memset(monitor_login.peer_ip, 0, sizeof(monitor_login.peer_ip));
        std::memcpy(monitor_login.peer_ip, peer_ip, sizeof(monitor_login.peer_ip));
        monitor_login.sex = user->getSex();
        std::memset(monitor_login.ssn, 0, sizeof(monitor_login.ssn));
        std::memcpy(monitor_login.ssn, user->GetSsnString(), sizeof(monitor_login.ssn));
        CStreamGuard monitor_guard(GlobalData::stream_pool()->Acquire(
            "InterDispatcher.cpp", 7739), true);
        *monitor_guard << 65;
        *monitor_guard << user->GetUID();
        monitor_guard->put_binary(&monitor_login, sizeof(monitor_login));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, monitor_guard);
    }

    user->setClientVersion(login_data.token_state);
    user->SetAccountLastPlayTime(login_data.last_play_time);
    user->setJoinedSchoolPointEvent(login_data.school_point_joined != 0);
    user->black_ip_private_security_active_ = false;
    user->network_.GetPeerIP2(user->login_session_peer_ip_, sizeof(user->login_session_peer_ip_));
    user->black_ip_punish_type_ =
        GlobalData::g_blackIPMonitor->getPunishType(user->login_session_peer_ip_);
    if ((user->black_ip_punish_type_ & 1) != 0 &&
        GlobalData::g_blackIPMonitor->GetEndTime(user->login_session_peer_ip_, kBlackIpTrade) >=
            current_second) {
        user->SetTradePunishType(16);
        DB_BlackipPrivateSecurityValuation::makeRequest(user->get_acc_id());
    }
    const unsigned int clean_pad_points_before = login_data.daily_clean_pad_point;
    if (CheckDailyScheduleTime(environment->get_fatigue_time(), login_data.last_play_time,
                               current_second)) login_data.daily_clean_pad_point = 0;
    else --login_data.daily_clean_pad_point;
    user->mcap_manager_.incPoint(login_data.daily_clean_pad_point);
    if (user->mcap_manager_.getPoint() == clean_pad_points_before)
        user->mcap_manager_.disableCleanPadPoint();
    if ((user->black_ip_punish_type_ & 2) != 0 &&
        GlobalData::g_blackIPMonitor->GetEndTime(user->login_session_peer_ip_,
                                                  kBlackIpPrivateSecurity) >= current_second) {
        user->black_ip_private_security_active_ = true;
        if (user->mcap_manager_.getPoint() <= 0x251B) user->mcap_manager_.incPoint(9500);
    }

    if (login_data.return_user_loaded) {
        user->SetReturnUser();
        user->SetReturnUserExpireTime(login_data.return_user_value);
        user->SetReturnUserFirstUserLogin(login_data.return_user_first_login != 0);
        Timer_ReturnUserTimeout::registNextTimer(user->GetUID(), user->get_unique_id(),
                                                  user->GetReturnUserExpireTime());
    } else if (login_data.last_play_time != 0 && login_data.last_play_time + 62208000 < current_second) {
        user->SetReturnUser();
        user->SetReturnUserExpireTime(current_second + 1209600);
        user->SetReturnUserFirstUserLogin(true);
        Timer_ReturnUserTimeout::registNextTimer(user->GetUID(), user->get_unique_id(),
                                                  user->GetReturnUserExpireTime());
    } else user->ResetReturnUser();
    user->resetAccountMemberBonusFatigue();
    if (CheckDailyScheduleTime(environment->get_fatigue_time(), login_data.return_user_update_time,
                               current_second)) user->EnableSaveMemberBonusFatigue();
    else user->incAccountMemberBonusFatigue(static_cast<short>(login_data.return_user_bonus_fatigue));
    user->setUserEventCharacterFlag(login_data.event_charac_flag);
    user->SetMileage(login_data.mileage);
    user->setCharacLinkMessageFlag(login_data.charac_link_message != 0);
    if (login_data.break_away_member)
        login_data.break_away_member = static_cast<uint8_t>(
            GlobalData::s_BreakAwaySys->updateUvList(&login_data));
    if (login_data.break_away_luck_point > 0) {
        user->setBreakAwayRewardData(true);
        user->setUserBreakAwayRewardLuckPoint(login_data.break_away_luck_point);
    }
    if (login_data.break_away_add_info > 0) {
        user->setBreakAwayRewardOrder(
            login_data.break_away_last_reward_time <= login_data.break_away_first_reward_time
                ? static_cast<ENUM_BREAK_AWAY_REWARD_ORDER>(3)
                : static_cast<ENUM_BREAK_AWAY_REWARD_ORDER>(1));
        user->setBreakAwayDungeonClearCnt(login_data.break_away_add_info);
        user->setBreakAwayQuestTime(login_data.break_away_quest_time);
    }
    if (login_data.break_away_accrue_cera >= 0)
        user->setBreakAwayAccureCera(login_data.break_away_accrue_cera);
    CEventBase* const break_away_event = GlobalData::s_event_manager->GetRepeatEvent(52);
    if (break_away_event && break_away_event->IsEventing(0) && login_data.break_away_member)
        GlobalData::s_BreakAwaySys->startBreakAwayPreventSystem(&login_data, user);

    DB_LoadAccountCargo::makeRequest(user->GetUID(),
                                     user->get_acc_id(),
                                     user->GetServerGroup());
    // IDA 0x84C7A8C reads original SIG_LOGIN_DATA +0x38DC. The expanded
    // character view moves the same premium flag to +0x39FC.
    user->SetPuUser(login_data.premium_alignment_38dc != 0);
    user->setCharacSlotCountInfo(login_data.charac_slot_effect_count, login_data.charac_slot_limit);
    user->set_ontime_last_recv_idx(login_data.ontime_user_recv_index);
    user->setWebAddress(login_data.sso_login_ip);
    if (login_data.random_option_check_count) user->setCheckPickUpRandomOptionItem(true);
    user->reset_server_fatigue_data();
    user->set_fatigue_count_db(login_data.fatigue_occurrence_day, login_data.fatigue_count);
    user->set_ingame_event_history(reinterpret_cast<SIG_INGAME_EVENT_HISTORY*>(
        login_data.in_game_event_history));
    if (login_data.login_ip_changed)
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x261u), 1, 0, 0);
    user->getAcccountHackInfo()->setLoginTime();
    user->loadInformNoticeFlag(
        reinterpret_cast<char*>(login_data.login_event_state));
    user->ValidateStrikerInfo();
    user->CalcurateManageLevel();
    user->CalcurateUserMaxLevel();
    user->SetBingoData(login_data);
    user->initBingoData();
    user->sendReturnUserFirstLogin();
    user->SetGrowthWeaponEventInfo(*reinterpret_cast<SIG_GROWTH_WEAPON_EVENT*>(
        &login_data.growth_weapon_active));
    user->SetGrowthCreatureEventInfo(login_data.growth_creature_event);
    // Original address +0x4338 is a typed consumer view of the same Stream
    // allocation, not a fabricated payload tail.
    user->setCharacLevelUpGift_AccountOnce(*reinterpret_cast<SIG_LEVELUP_GIFT_ACCOUNT_ONCE*>(
        login_data.account_level_gifts));
    user->sendEventInfo();
    // RECONSTRUCTED_0828: CMD561 reservation state is account-scoped and the
    // 0828 client expects NOTI493 to restore it after entering state 2.
    decide_levelup_event::RequestLoginState(user);
    user->SetEventCreateDnfReward(false);
    DBSelectCreateDnfUserInfo::makeRequest(user->GetUID(), user->get_acc_id());
    user->login_session_active_ = 0;
    if (Taiwan::GarenaCyberCafe(user->getGarenaAuthData()->getGcaType()))
        DBSelectPcroomDailyReward::makeRequest(user);
    return 0;
}

// IDA_EA: 0x84E9F9C
// IDA_NAME: _ZN15Inter_LoginInfoC2Ev
// DEMANGLED: Inter_LoginInfo::Inter_LoginInfo(void)
Inter_LoginInfo::Inter_LoginInfo() {}

// IDA_EA: 0x84E9FB8
// IDA_NAME: _ZN15Inter_LoginInfoD2Ev
// DEMANGLED: Inter_LoginInfo::~Inter_LoginInfo()
// IDA_EA: 0x84E9FE8
// IDA_NAME: _ZN15Inter_LoginInfoD0Ev
// DEMANGLED: Inter_LoginInfo::~Inter_LoginInfo()
Inter_LoginInfo::~Inter_LoginInfo() {}
