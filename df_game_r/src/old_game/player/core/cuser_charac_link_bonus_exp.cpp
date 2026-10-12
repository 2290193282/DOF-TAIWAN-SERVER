#include "old_game/player/core/cuser.h"

#include <cstring>

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/db/taiwan/inter_slots_331_353.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x8652564
// IDA_NAME: _ZN5CUser22giveCharacLinkBonusExpEj
// DEMANGLED: CUser::giveCharacLinkBonusExp(unsigned int)
void CUser::giveCharacLinkBonusExp(unsigned int exp) {
    PacketGuard packet;
    int gained_sp = 0;
    int gained_sfp = 0;

    const ENUM_SKILL_TREE_KIND skill_tree =
        static_cast<ENUM_SKILL_TREE_KIND>(GetCurCharacSkillTreeIndex());
    const int previous_sp =
        getCurCharacSkillR()->get_remain_sp_at_index(skill_tree);

    calcHelpAbuseRatio(exp, false);
    const bool level_changed = gain_exp_sp(
        static_cast<int>(exp), gained_sp, gained_sfp,
        static_cast<eExpAddReason>(3), 0, true);

    const ENUM_SKILL_TREE_KIND current_skill_tree =
        static_cast<ENUM_SKILL_TREE_KIND>(GetCurCharacSkillTreeIndex());
    const int current_sp =
        getCurCharacSkillR()->get_remain_sp_at_index(current_skill_tree);

    if (level_changed) {
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(1);
        make_basic_info(packet, 0);
        packet.finalize(true);
        G_GameWorld()->send_all(packet, this);
    }

    char link_character_name[30] = {};
    const int link_slot = static_cast<char>(getLinkCharacSlotIndex());
    if (link_slot >= 0 &&
        static_cast<int>(characters_.size()) > link_slot) {
        std::strcpy(link_character_name,
                    characters_[link_slot].fields_.character_name_);
    }

    packet.clear();
    packet.put_header(0, 291);
    packet.put_int(static_cast<int>(std::strlen(link_character_name)));
    packet.put_str(link_character_name, std::strlen(link_character_name));
    packet.put_int(0);
    packet.put_int(static_cast<int>(exp));
    packet.put_short(static_cast<short>(current_sp - previous_sp));
    packet.put_short(static_cast<short>(gained_sfp));
    packet.finalize(true);
    Send(packet);
    SendNotiPacket(static_cast<eSendTarget>(1),
                   static_cast<ENUM_NOTIPACKET>(2), 1);
    send_skill_info();
}


// IDA_EA: 0x84E1F02
// IDA_NAME: _ZN25Inter_LoadCharacLinkBonus12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadCharacLinkBonus::dispatch_sig(CUser *,char *,int)
int Inter_LoadCharacLinkBonus::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;

    const SIG_LOAD_CHARAC_LINK_BONUS* const reply =
        reinterpret_cast<const SIG_LOAD_CHARAC_LINK_BONUS*>(payload);

    char mail_text[256] = {};
    char mail_title[21] = {};
    std::strncpy(mail_title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_149", 0),
                 0x14u);
    std::strncpy(mail_text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_150", 0),
                 0xffu);
    Inven_Item empty_mail_item;

    const unsigned int reward_exp = reply->reward_exp;
    const unsigned int mail_item_id = reply->mail_item_id;
    if (!reward_exp && !mail_item_id) {
        return 0;
    }

    if (mail_item_id) {
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            mail_title, empty_mail_item, mail_item_id,
            static_cast<unsigned int>(user->getCurCharacNo()), mail_text,
            static_cast<int>(std::strlen(mail_text)), 0,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false,
            false);

        char link_character_name[30] = {};
        const int link_slot =
            static_cast<char>(user->getLinkCharacSlotIndex());
        if (link_slot >= 0 &&
            static_cast<int>(user->characters_.size()) > link_slot) {
            std::strcpy(link_character_name,
                        user->characters_[link_slot].fields_.character_name_);
        }

        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 291);
        packet.put_int(static_cast<int>(std::strlen(link_character_name)));
        packet.put_str(link_character_name, std::strlen(link_character_name));
        packet.put_int(static_cast<int>(mail_item_id));
        packet.put_int(0);
        packet.put_short(0);
        packet.put_short(0);
        packet.finalize(true);
        user->Send(packet);

        GetInstanceCirculationStatistic()->AddCirculationStatistic(
            static_cast<CIRCULATION_STATISTIC_FIELD>(17), user,
            mail_item_id);
    }

    if (reward_exp && user->get_charac_level() !=
                          oldsrv::config::kCharacterLevelMaximum) {
        user->giveCharacLinkBonusExp(reward_exp);
    }
    user->UpdateCharacLinkBonus(
        0, 0, static_cast<unsigned int>(user->getCurCharacNo()),
        static_cast<ENUM_CHARAC_LINK_BONUS_STATE>(1));
    user->setCharacLinkBonusSetting(
        user->get_charac_idx(user->getCurCharacNo()), false);
    return 0;
}


// IDA_EA: 0x84EE810
// IDA_NAME: _ZN25Inter_LoadCharacLinkBonusC2Ev
// DEMANGLED: Inter_LoadCharacLinkBonus::Inter_LoadCharacLinkBonus(void)
Inter_LoadCharacLinkBonus::Inter_LoadCharacLinkBonus() {}

// IDA_EA: 0x84F6DCE (D2), 0x84F6DFE (D0)
Inter_LoadCharacLinkBonus::~Inter_LoadCharacLinkBonus() {}
