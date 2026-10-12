#include "old_game/player/session/inter_character_return_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "old_data/db/anti_cheat/db_hades_punish.h"
#include "old_data/db/auction_cashshop/db_get_premium_selected_cube_info.h"
#include "old_data/db/auction_cashshop/db_mileage.h"
#include "old_data/db/character/character_return_payloads.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"
#include "old_data/db/character/db_select_charac_mid.h"
#include "old_data/db/character/query_charac_info_payload.h"
#include "old_data/db/character/tag_charac_info_payload.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/charac_expand_cdata.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/player/core/character_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

int BoundedCStringLength(const char* text, int maximum_length) {
    int length = 0;
    while (length < maximum_length && text[length]) {
        ++length;
    }
    return length;
}

}  // namespace

// IDA_EA: 0x84E4D4E
// IDA_NAME: _ZN17Inter_LoadMileage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadMileage::dispatch_sig(CUser *,char *,int)
int Inter_LoadMileage::dispatch_sig(CUser* user, char* payload, int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_MILEAGE_STRUCT))) {
        return 0;
    }

    const SIG_MILEAGE_STRUCT& result =
        *reinterpret_cast<const SIG_MILEAGE_STRUCT*>(payload);
    user->SetMileage(result.miles);
    user->SendCashData(false);
    return 0;
}

// IDA_EA: 0x84EE8F0 / 0x84F6288 / 0x84F62B8
Inter_LoadMileage::Inter_LoadMileage() : ISigDispatcher() {}
Inter_LoadMileage::~Inter_LoadMileage() {}

// IDA_EA: 0x84E2A06
// IDA_NAME: _ZN31Inter_ReqQueryCharacInfoMailbox12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReqQueryCharacInfoMailbox::dispatch_sig(CUser *,char *,int)
int Inter_ReqQueryCharacInfoMailbox::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    if (!user || user->get_state() <= 2 || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_QUERY_CHARAC_INFO))) {
        return 0;
    }

    const SIG_QUERY_CHARAC_INFO& result =
        *reinterpret_cast<const SIG_QUERY_CHARAC_INFO*>(payload);
    if (result.result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_QUERY_CHARAC_INFO_MAILBOX,
            static_cast<unsigned char>(result.result));
        return 0;
    }

    const int name_length = BoundedCStringLength(
        result.charac_name, static_cast<int>(sizeof(result.charac_name)));
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_QUERY_CHARAC_INFO_MAILBOX);
    packet.put_byte(1);
    packet.put_int(name_length);
    packet.put_str(result.charac_name, name_length);
    packet.put_short(result.level);
    packet.put_byte(result.job);
    packet.put_byte(result.grow_type);
    packet.put_byte(result.sex);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE89C / 0x84F6AC2 / 0x84F6AF2
Inter_ReqQueryCharacInfoMailbox::Inter_ReqQueryCharacInfoMailbox()
    : ISigDispatcher() {}
Inter_ReqQueryCharacInfoMailbox::~Inter_ReqQueryCharacInfoMailbox() {}

// IDA_EA: 0x84E312A
// IDA_NAME: _ZN33Inter_Send_Charac_no_from_monitor12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Send_Charac_no_from_monitor::dispatch_sig(CUser *,char *,int)
int Inter_Send_Charac_no_from_monitor::dispatch_sig(CUser* user,
                                                     char* payload,
                                                     int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(
                           offsetof(Pakcet_FindUserName_use_Charac_no,
                                    charac_name))) {
        return 0;
    }

    const Pakcet_FindUserName_use_Charac_no& result =
        *reinterpret_cast<const Pakcet_FindUserName_use_Charac_no*>(payload);
    const int name_length = static_cast<int>(result.charac_name_length);
    const int available_name_bytes = payload_size - static_cast<int>(
        offsetof(Pakcet_FindUserName_use_Charac_no, charac_name));
    if (name_length < 0 || name_length > available_name_bytes ||
        name_length > static_cast<int>(sizeof(result.charac_name))) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_FIND_CHAR_NAME_USE_CHARAC_NO);
    if (name_length) {
        packet.put_byte(1);
        packet.put_int(static_cast<int>(result.charac_no));
        packet.put_int(name_length);
        packet.put_str(result.charac_name, name_length);
    } else {
        packet.put_byte(0);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE944 / 0x84F693C / 0x84F696C
Inter_Send_Charac_no_from_monitor::Inter_Send_Charac_no_from_monitor()
    : ISigDispatcher() {}
Inter_Send_Charac_no_from_monitor::~Inter_Send_Charac_no_from_monitor() {}

// IDA_EA: 0x84E358A
// IDA_NAME: _ZN22Inter_TodayGuildMember12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TodayGuildMember::dispatch_sig(CUser *,char *,int)
int Inter_TodayGuildMember::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    if (!user || user->get_state() <= 2 || !payload ||
        payload_size <
            static_cast<int>(sizeof(Packet_Today_Guild_Member_Reply))) {
        return 0;
    }

    const Packet_Today_Guild_Member_Reply& reply =
        *reinterpret_cast<const Packet_Today_Guild_Member_Reply*>(payload);
    if (static_cast<int>(reply.requester_charac_no) != user->getCurCharacNo()) {
        return 0;
    }

    if (reply.member_charac_no ==
        static_cast<uint32_t>(user->getCurCharacNo())) {
        user->SetTodayGuildMember(true);
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_TODAY_GUILD_MEMBER);
    if (reply.member_charac_no) {
        const int name_length = BoundedCStringLength(
            reply.member_charac_name,
            static_cast<int>(sizeof(reply.member_charac_name)));
        packet.put_byte(1);
        packet.put_int(name_length);
        packet.put_str(reply.member_charac_name, name_length);
        packet.put_byte(reply.job);
        packet.put_byte(reply.grow_type);
        packet.put_byte(reply.level);
        packet.put_byte(reply.sex);
        packet.put_byte(reply.guild_member_grade);
    } else {
        packet.put_byte(0);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE9D0 / 0x84F67B6 / 0x84F67E6
Inter_TodayGuildMember::Inter_TodayGuildMember() : ISigDispatcher() {}
Inter_TodayGuildMember::~Inter_TodayGuildMember() {}

// IDA_EA: 0x84E3F0E
// IDA_NAME: _ZN22Inter_ApproveJoinGuild12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ApproveJoinGuild::dispatch_sig(CUser *,char *,int)
int Inter_ApproveJoinGuild::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    if (!user || user->get_state() <= 2 || !payload ||
        payload_size <
            static_cast<int>(sizeof(Packet_Approve_Join_Guild_Reply))) {
        return 0;
    }

    const Packet_Approve_Join_Guild_Reply& reply =
        *reinterpret_cast<const Packet_Approve_Join_Guild_Reply*>(payload);
    if (static_cast<int>(reply.approver_charac_no) !=
        user->getCurCharacNo()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_APPROVE_JOIN_GUILD);
    if (reply.result_code) {
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(reply.result_code));
    } else {
        packet.put_byte(1);
    }
    packet.put_int(static_cast<int>(reply.applicant_charac_no));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA40 / 0x84F6630 / 0x84F6660
Inter_ApproveJoinGuild::Inter_ApproveJoinGuild() : ISigDispatcher() {}
Inter_ApproveJoinGuild::~Inter_ApproveJoinGuild() {}

// IDA_EA: 0x84E4EE4
// IDA_NAME: _ZN25Inter_LoadKillMonsterInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadKillMonsterInfo::dispatch_sig(CUser *,char *,int)
int Inter_LoadKillMonsterInfo::dispatch_sig(CUser* user,
                                            char* payload,
                                            int) {
    if (!user || user->get_state() == 0) {
        return 25160;
    }

    charac_expand::CData* const kill_monster_info =
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(12));
    kill_monster_info->loadData(user, payload);
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(9));
    return 0;
}

// IDA_EA: 0x84EEBAC / 0x84F6150 / 0x84F6180
Inter_LoadKillMonsterInfo::Inter_LoadKillMonsterInfo() : ISigDispatcher() {}
Inter_LoadKillMonsterInfo::~Inter_LoadKillMonsterInfo() {}

// IDA_EA: 0x84E7024
// IDA_NAME: _ZN28Inter_MonitorReplyCharacInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorReplyCharacInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorReplyCharacInfo::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || !payload ||
        payload_size <
            static_cast<int>(sizeof(Packet_Monitor_Reply_Charac_Info))) {
        return 0;
    }

    const Packet_Monitor_Reply_Charac_Info& reply =
        *reinterpret_cast<const Packet_Monitor_Reply_Charac_Info*>(payload);
    CUser* const resolved_user = G_CGameManager()->getUser(
        static_cast<int>(reply.target_user_uid), 0);
    const int unique_id = resolved_user ? resolved_user->get_unique_id() : 0;

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_PRECHECK_SOLO_TELEPOART);
    if (reply.result_code) {
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(reply.result_code));
    } else {
        const int name_length = BoundedCStringLength(
            reply.target_charac_name,
            static_cast<int>(sizeof(reply.target_charac_name)));
        packet.put_byte(1);
        packet.put_byte(reply.channel_no);
        packet.put_int(unique_id);
        packet.put_int(name_length);
        packet.put_str(reply.target_charac_name, name_length);
        packet.put_byte(reply.level);
        packet.put_byte(reply.job);
        packet.put_byte(reply.grow_type);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EED50 / 0x84F5CBE / 0x84F5CEE
Inter_MonitorReplyCharacInfo::Inter_MonitorReplyCharacInfo()
    : ISigDispatcher() {}
Inter_MonitorReplyCharacInfo::~Inter_MonitorReplyCharacInfo() {}

// IDA_EA: 0x84E87DE
// IDA_NAME: _ZN26Inter_LoadCharacTypeHotKey12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadCharacTypeHotKey::dispatch_sig(CUser *,char *,int)
int Inter_LoadCharacTypeHotKey::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (!user || user->get_state() <= 1 || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION))) {
        return 0;
    }

    SIG_LOAD_CHARAC_HOTKEY_OPTION& result =
        *reinterpret_cast<SIG_LOAD_CHARAC_HOTKEY_OPTION*>(payload);
    if (result.key_type > 1u ||
        result.option_length > sizeof(result.hotkey_option.bytes_)) {
        return 0;
    }
    const unsigned int bounded_length =
        result.hotkey_option.getTextLength();
    const bool is_empty = result.option_length == 0 &&
                          result.hotkey_option.isEmpty();
    if (!is_empty &&
        (bounded_length != result.option_length ||
         !result.hotkey_option.isValidText(result.option_length))) {
        return 0;
    }
    user->GetGameOptionRef()->SetHotKeyOption(result.key_type,
                                               result.hotkey_option);

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_HOTKEY_OPTION);
    packet.put_byte(result.key_type);
    packet.put_int(result.option_length);
    if (result.option_length != 0) {
        packet.put_binary(result.hotkey_option.bytes_, result.option_length);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEE84 / 0x84F58E4 / 0x84F5914
Inter_LoadCharacTypeHotKey::Inter_LoadCharacTypeHotKey() : ISigDispatcher() {}
Inter_LoadCharacTypeHotKey::~Inter_LoadCharacTypeHotKey() {}

// IDA_EA: 0x84E8936
// IDA_NAME: _ZN32Inter_GetPremiumSelectedCubeInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GetPremiumSelectedCubeInfo::dispatch_sig(CUser *,char *,int)
int Inter_GetPremiumSelectedCubeInfo::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(
                           sizeof(SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES)) ||
        !user->getCurCharacR()) {
        return 0;
    }

    const SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES& result =
        *reinterpret_cast<const SIG_GET_PREMIUM_SELECTED_CUBE_INFO_RES*>(
            payload);
    user->initContractOfCubePremiumInfo(result.selected != 0,
                                        result.cube_type);

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_CONTRACT_OF_CUBE_INFO);
    packet.put_byte(result.selected);
    packet.put_byte(result.cube_type);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEEBC / 0x84F5848 / 0x84F5878
Inter_GetPremiumSelectedCubeInfo::Inter_GetPremiumSelectedCubeInfo()
    : ISigDispatcher() {}
Inter_GetPremiumSelectedCubeInfo::~Inter_GetPremiumSelectedCubeInfo() {}

// IDA_EA: 0x84E7884
// IDA_NAME: _ZN28Inter_RequestCharacSkillInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_RequestCharacSkillInfo::dispatch_sig(CUser *,char *,int)
int Inter_RequestCharacSkillInfo::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || user->get_state() <= 2 || !payload ||
        payload_size < static_cast<int>(
                           offsetof(SIG_REQUEST_CHARAC_SKILL_INFO,
                                    skill_tree_index) +
                           sizeof(SIG_REQUEST_CHARAC_SKILL_INFO::
                                      skill_tree_index))) {
        return 0;
    }

    const SIG_REQUEST_CHARAC_SKILL_INFO& result =
        *reinterpret_cast<const SIG_REQUEST_CHARAC_SKILL_INFO*>(payload);
    const int charac_index = user->get_charac_idx(result.charac_no);
    if (charac_index == -1) {
        return 0;
    }

    _Charac_info& character = user->characters_[
        static_cast<std::vector<_Charac_info>::size_type>(charac_index)];
    const unsigned char skill_tree_index =
        static_cast<unsigned char>(result.skill_tree_index);
    character.fields_.skill_tree_index_ =
        static_cast<int8_t>(skill_tree_index);

    SkillSlot* const skill_slot = &character.fields_.skill_slot_;
    skill_slot->set_parent(user);

    _PutSkillInfoToCharac(static_cast<char>(skill_tree_index),
                          result.mastered_skills,
                          skill_slot);

    skill_slot->SendSkillInfo(
        charac_index,
        static_cast<ENUM_SKILL_TREE_KIND>(
            static_cast<int8_t>(skill_tree_index)));
    skill_slot->SetLoadStrikerSkill(true);
    return 0;
}

// IDA_EA: 0x84E7820
// IDA_NAME: _ZN28Inter_RequestCharacSkillInfo21_PutSkillInfoToCharacEcPK15_Mastered_skillP9SkillSlot
// DEMANGLED: Inter_RequestCharacSkillInfo::_PutSkillInfoToCharac(char,_Mastered_skill const*,SkillSlot *)
void* Inter_RequestCharacSkillInfo::_PutSkillInfoToCharac(
    char skill_tree_index,
    const _Mastered_skill* source,
    SkillSlot* skill_slot) {
    const unsigned char tree = static_cast<unsigned char>(skill_tree_index);
    if (tree == 0xffu || tree == 0u) {
        return std::memcpy(
            skill_slot->get_mastered_skill_tree(ENUM_SKILL_TREE_0),
            source,
            sizeof(SIG_REQUEST_CHARAC_SKILL_INFO::mastered_skills));
    }
    if (tree == 1u) {
        return std::memcpy(
            skill_slot->get_mastered_skill_tree(ENUM_SKILL_TREE_1),
            source,
            sizeof(SIG_REQUEST_CHARAC_SKILL_INFO::mastered_skills));
    }
    return reinterpret_cast<void*>(static_cast<uintptr_t>(tree));
}

// IDA_EA: 0x84EEDF8 / 0x84F5B86 / 0x84F5BB6
Inter_RequestCharacSkillInfo::Inter_RequestCharacSkillInfo()
    : ISigDispatcher() {}
Inter_RequestCharacSkillInfo::~Inter_RequestCharacSkillInfo() {}

// IDA_EA: 0x84E79F2
// IDA_NAME: _ZN26Inter_RequestTagCharacInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_RequestTagCharacInfo::dispatch_sig(CUser *,char *,int)
int Inter_RequestTagCharacInfo::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (!user || user->get_state() <= 2 || !payload ||
        payload_size !=
            static_cast<int>(sizeof(SIG_REQUEST_TAG_CHARAC_INFO))) {
        return 0;
    }

    SIG_REQUEST_TAG_CHARAC_INFO& result =
        *reinterpret_cast<SIG_REQUEST_TAG_CHARAC_INFO*>(payload);
    const int charac_index = user->get_charac_idx(result.charac_no);
    if (charac_index == -1) {
        return 0;
    }

    _Charac_info& character = user->characters_[
        static_cast<std::vector<_Charac_info>::size_type>(charac_index)];
    user->setTagCharac(&character);
    user->getTagCharacInvenR()->set_parent(user);
    user->getTagCharacSkillR()->set_parent(user);

    user->setTagCharacInfo(
        charac_index,
        result.charac_no,
        &result.tag_charac_data);
    if (user->setTagCharacInven(&result.inventory) != 1) {
        return 0;
    }

    CQuestShop* const quest_shop = static_cast<CQuestShop*>(
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (quest_shop) {
        quest_shop->SetTagCharacQuestShopData(
            &result.quest_shop);
    }

    user->setCurCharacTagCharacNo(result.charac_no);
    user->setCurCharacStrikerSkillIndex(result.striker_skill_index);
    user->character_view().enableSaveCharacView();
    character.fields_.tag_charac_info_loaded_ = 1;
    user->SendTagCharacInfo();
    return 0;
}

// IDA_EA: 0x84EEE14 / 0x84F5B38 / 0x84F5B68
Inter_RequestTagCharacInfo::Inter_RequestTagCharacInfo()
    : ISigDispatcher() {}
Inter_RequestTagCharacInfo::~Inter_RequestTagCharacInfo() {}

namespace {

void SendHadesAlert(CUser* user, uint32_t action_flags) {
    PacketGuard packet;
    packet.put_header(0, 447);
    packet.put_int(static_cast<int>(action_flags));
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x84E5F02
// IDA_NAME: _ZN17Inter_HadesAction12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_HadesAction::dispatch_sig(CUser *,char *,int)
int Inter_HadesAction::dispatch_sig(CUser* user,
                                    char* payload,
                                    int payload_size) {
    if (!payload ||
        payload_size < static_cast<int>(sizeof(Packet_Hades_Action))) {
        return 0;
    }

    const Packet_Hades_Action& action =
        *reinterpret_cast<const Packet_Hades_Action*>(payload);

    if (user &&
        static_cast<uint32_t>(user->get_acc_id()) == action.account_id) {
        char peer_ip[16] = {};
        user->getHades()->GetIP(peer_ip);
        user->getHades()->RunHadesAction(action.action_flags,
                                          action.punish_days,
                                          action.apply_switch,
                                          action.reserved,
                                          peer_ip);
        SendHadesAlert(user, action.action_flags);
    } else if (!user && action.account_id) {
        if ((action.action_flags & 0x02u) != 0u) {
            DB_HadesPunish::makeRequest(action.account_id,
                                        2u,
                                        action.punish_days,
                                        action.apply_switch,
                                        action.reserved,
                                        0);
        }
        if ((action.action_flags & 0x01u) != 0u) {
            DB_HadesPunish::makeRequest(action.account_id,
                                        1u,
                                        action.punish_days,
                                        action.apply_switch,
                                        action.reserved,
                                        0);
        }
    }
    return 0;
}

// IDA_EA: 0x84EECA8 / 0x84F5E92 / 0x84F5EC2
Inter_HadesAction::Inter_HadesAction() : ISigDispatcher() {}
Inter_HadesAction::~Inter_HadesAction() {}

// IDA_EA: 0x84E6170
// IDA_NAME: _ZN21Inter_SelectCharacMid12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SelectCharacMid::dispatch_sig(CUser *,char *,int)
int Inter_SelectCharacMid::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_CHECK_CHAR_MID_RESULT)) ||
        !user->getCurCharacR()) {
        return 0;
    }

    const SIG_CHECK_CHAR_MID_RESULT& result =
        *reinterpret_cast<const SIG_CHECK_CHAR_MID_RESULT*>(payload);
    if (result.request_id != 0u) {
        return 0;
    }

    const uint32_t hack_type = result.value1;
    const uint32_t value2 = result.value2;
    const uint32_t value1 = result.account_id;
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
        1u,
        value1,
        value2);

    if (hack_type == 718u) {
        // IDA 0x84E622F passes `src__12`, the global empty string, not the
        // client's peer IP.  Keep that exact DB-return behavior.
        char empty_ip[1] = {};
        user->getHades()->GoldTracking(
            2u,
            value1,
            empty_ip,
            value2,
            static_cast<uint32_t>(user->getCurCharacUsedFatigue()));
    }
    return 0;
}

// IDA_EA: 0x84EECE0 / 0x84F5DF6 / 0x84F5E26
Inter_SelectCharacMid::Inter_SelectCharacMid() : ISigDispatcher() {}
Inter_SelectCharacMid::~Inter_SelectCharacMid() {}
