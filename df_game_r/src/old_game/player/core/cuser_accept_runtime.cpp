#include "old_game/player/core/cuser.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_user_action_abi.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/gm/player/game_master_character.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/security/crypto/cipher.h"

#include <cstring>

// IDA_EA: 0x86493C8
// IDA_NAME: _ZN5CUser12InitOnAcceptEi
// DEMANGLED: CUser::InitOnAccept(int)
int CUser::InitOnAccept(int fd) {
    network_.set_socket(fd, false);
    state_ = 1u;
    pending_pvp_win_point_ = 0u;
    packet_verifier_.Init();
    network_.initNetworkBuffer();

    // The original four-byte store begins at CAccountCargo+0xc50.  That is
    // item 51's Chronicle RandomOption subobject, not anonymous cargo state.
    // Spell out the four affected source fields so this reset remains exact
    // without treating CAccountCargo as a byte buffer.
    RandomOption& accept_random_option =
        account_cargo_.items_[51].chronicle_random_option_.random_option_;
    accept_random_option.option_fields_[0].option_index_ = 0u;
    accept_random_option.option_fields_[0].first_value_ = 0u;
    accept_random_option.option_fields_[0].second_value_ = 0u;
    accept_random_option.option_fields_[1].option_index_ = 0u;

    gm_quest_flag_ = 0u;
    play_experience_ = 0;
    break_away_dungeon_clear_ = 0u;
    break_away_dungeon_clear_count_ = 0;
    break_away_accure_cera_ = 0;
    break_away_quest_time_ = 0;
    break_away_reward_order_ = 0;
    multibox_lottery_item_fail_count_ = 0;
    break_away_reward_data_ = 0u;
    used_break_away_luck_point_ = 0u;
    break_away_rewards_.clear();
    charac_link_message_flag_ = 0;
    link_charac_disconnect_flag_ = 0;
    disconnect_link_charac_slot_index_ = -1;
    // Expanded layout writes -1 at +0x7072a after clearing +0x7072b..+0x707f2.
    std::memset(skill_commands_, 0, sizeof(skill_commands_));

    mailbox_ = WongWork::CMailBoxHelper::AssignMailBox();
    mailbox_->Init();

    PacketGuard packet;
    packet.put_header(0, 1);
    unsigned char original_key[1024];
    Cipher* const cipher = G_Cipher();
    const int cipher_key_length = cipher->GetTotalKeyLength();
    cipher->GetOriginalKey(original_key);
    packet.put_binary(reinterpret_cast<const char*>(original_key),
                      cipher_key_length);

    CEnvironment* const environment = G_CEnvironment();
    const char* const channel_name = environment->get_channel_name();
    const int channel_name_length = static_cast<int>(std::strlen(channel_name));
    packet.put_int(channel_name_length);
    packet.put_str(channel_name, channel_name_length);

    accept_seed_from_date_ = static_cast<uint32_t>(get_seed_from_date());
    packet.put_int(static_cast<int>(accept_seed_from_date_));
    accept_random_nonce_ = get_rand_int(0);
    packet.put_int(static_cast<int>(accept_random_nonce_));
    packet.put_byte(environment->get_accept_handshake_flag_888());
    packet.put_byte(environment->get_accept_handshake_flag_432());
    packet.put_byte(environment->get_accept_handshake_flag_424());
    packet.put_int(GlobalData::s_systemTime_.getCurSec());

    const std::vector<SK_IP>& sk_ips = environment->get_sk_ip_list();
    packet.put_int(static_cast<int>(sk_ips.size()));
    if (!sk_ips.empty()) {
        for (size_t index = 0; index < sk_ips.size(); ++index) {
            packet.put_int(static_cast<int>(std::strlen(sk_ips[index].text)));
            // The old sender writes the fixed storage extent, not strlen().
            packet.put_str(sk_ips[index].text, sizeof(sk_ips[index].text));
        }
        const std::vector<int>& sk_ports = environment->get_sk_port_values();
        packet.put_int(sk_ports[0]);
        packet.put_int(sk_ports[1]);
    }
    packet.put_int(ENUM_CMDPACKET_END);
    packet.put_int(ENUM_NOTIPACKET_END);
    packet.finalize(true);
    // Diagnostic-only: the client must receive this initial key packet
    // before it sends encrypted CMD traffic.  Do not log the key or payload.
    LogManager::logFormat(
        1, "accept_diag.cpp", "int CUser::InitOnAccept(int)", 0,
        "[HELLO_SEND] fd=%d key_len=%d channel_len=%d sk_count=%d cmd_end=%d noti_end=%d wire_len=%d",
        fd, cipher_key_length, channel_name_length,
        static_cast<int>(sk_ips.size()), static_cast<int>(ENUM_CMDPACKET_END),
        static_cast<int>(ENUM_NOTIPACKET_END), packet.get_len());
    Send(packet);

    StartRecvEventWaiting();
    setStdDropRate(100);
    trade_punish_type_ = 0;
    tournament_account_ = 0u;

    human_certified_ = 0u;
    secu_user_action_ = new cSecuUserAction();
    secu_user_action_->Init();
    pending_mail_is_multi_ = 0u;
    mail_send_certified_ = 1u;
    std::memset(pending_mail_message_, 0, sizeof(pending_mail_message_));
    std::memset(pending_multi_mail_message_, 0,
                sizeof(pending_multi_mail_message_));
    mcap_manager_.reset();

    money_log_to_accept_random_state_[3] = 0u;
    security_card_.init();
    pad_.init();
    auto_client_rate_ = 0;

    static CGameMasterCharacter game_master_character;
    game_master_character_ = &game_master_character;
    // InitOnAccept clears expanded CUser+0x8ec9c.  This is the
    // clean-pad request timestamp owned by CUser, not storage inside the
    // earlier anonymous security span.
    clean_pad_request_tick_ = 0u;
    hack_ip_counter_saved_ = 0u;
    ars_user_kick_ = 0u;
    hades_.Init(this);
    gold_control_.Init(this);
    hack_log_check_.Init(this);

    clean_pad_verification_first_ = 0u;
    // InitOnAccept clears expanded CUser+0x8ec88 as one scalar store.
    last_lottery_time_ = 0u;
    loading_hack_gold_ = 0u;
    non_client_flag_ = 0u;
    non_client_random_value_ = 0;
    // InitOnAccept clears expanded CUser+0x8e3e0.  The location is the
    // user debug-command scalar after Character, not an Advance Altar child
    // field (the manager ends at expanded +0x8e384).
    debug_command_ = 0;
    account_hacking_.init(this);
    force_drop_flag_ = 0u;
    std::memset(&garena_auth_data_, 0, sizeof(garena_auth_data_));
    return 1;
}
