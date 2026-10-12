#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include <cstring>
#include <map>

#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/arad_script_data.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/arad_jumping_character_packet_dispatcher.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A636
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::Arad_INTER_kAradJumpingCharacter(void)
Arad_INTER_kAradJumpingCharacter::Arad_INTER_kAradJumpingCharacter()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(45)) {}

// IDA_EA: 0x819A65A
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterD2Ev
// IDA_EA: 0x819A68A
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::~Arad_INTER_kAradJumpingCharacter()
Arad_INTER_kAradJumpingCharacter::~Arad_INTER_kAradJumpingCharacter() {}

// IDA_EA: 0x81993C4
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter14clearEpicQuestEP5CUser
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::clearEpicQuest(CUser *)
int Arad_INTER_kAradJumpingCharacter::clearEpicQuest(CUser* user) {
    int any_cleared = 0;
    CDataManager* const data_manager = G_CDataManager();

    // The direct old code reads CDataManager+0x18 and then calls this grade-0
    // selector.  The bridge retains that pointer's B-owned named ABI; this
    // C leaf intentionally does not reopen the DataManager layout.
    std::multimap<int, int>* const quest_npc_list =
        data_manager->get_arad_jumping_quest_list()->getQuestNPCList(
            static_cast<ENUM_QUEST_GRADE>(0));
    if (quest_npc_list == 0) {
        return any_cleared;
    }

    int cleared_count = 0;  // Old local +0x54; retained although not sent.
    const int character_level = user->get_charac_level();

    // First, clear the twenty currently accepted normal-grade quests whose
    // minimum level is below the character's pre-reward level.
    for (int slot = 0; slot <= 19; ++slot) {
        if (user->getCurCharacQuestW()->play_quest[slot] == 0) {
            continue;
        }

        const uint32_t accepted_quest =
            user->getCurCharacQuestR()->play_quest[slot];
        Quest* const quest = data_manager->find_quest(
            static_cast<int>(accepted_quest));
        if (quest == 0 || quest->quest_grade != 0) {
            continue;
        }

        // quest_basic_reward @0x866E7A8 writes all four output references
        // before inspecting them.  The old caller initializes only the first
        // three stack locals; retain that exact call-side contract here.
        int reward_exp = 0;
        int reward_gold = 0;
        int reward_qp = 0;
        int reward_piece;
        if (quest->minimum_level < character_level) {
            const uint32_t quest_to_clear =
                user->getCurCharacQuestR()->play_quest[slot];
            user->getCurCharacQuestW()->clear_quest.setClearedQuest(
                quest_to_clear);
            user->getCurCharacQuestW()->play_quest[slot] = 0;
            user->getCurCharacQuestW()->play_quest_trigger[slot] = 0;
            user->quest_basic_reward(quest,
                                     reward_exp,
                                     reward_gold,
                                     reward_qp,
                                     reward_piece,
                                     true);
            any_cleared = 1;
            ++cleared_count;
        }
    }

    // The original obtains end before begin.  Neither operation mutates the
    // script list, but preserving the order documents the observed routine.
    const std::multimap<int, int>::const_iterator end =
        quest_npc_list->end();
    std::multimap<int, int>::const_iterator it = quest_npc_list->begin();
    while (it != end) {
        Quest* const quest = data_manager->find_quest(it->second);
        if (quest != 0 && quest->quest_index != 3588 &&
            quest->quest_index != 3589 && quest->quest_index != 3590 &&
            quest->quest_index != 3591 && quest->quest_index != 3592 &&
            quest->checkPossibleJob(
                static_cast<ENUM_CHARACTERJOB>(user->get_charac_job())) ==
                1) {
            int reward_exp = 0;
            int reward_gold = 0;
            int reward_qp = 0;
            int reward_piece;
            if (quest->minimum_level < character_level &&
                user->getCurCharacQuestW()->clear_quest.isClearedQuest(
                    static_cast<unsigned int>(quest->quest_index)) != 1) {
                user->getCurCharacQuestW()->clear_quest.setClearedQuest(
                    static_cast<unsigned int>(quest->quest_index));
                user->quest_basic_reward(quest,
                                         reward_exp,
                                         reward_gold,
                                         reward_qp,
                                         reward_piece,
                                         true);
                any_cleared = 1;
                ++cleared_count;
            }
        }
        ++it;
    }

    (void)cleared_count;
    return any_cleared;
}

// IDA_EA: 0x819973E
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter13sendHelperBoxEP5CUserhh
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::sendHelperBox(CUser *,unsigned char,unsigned char)
int Arad_INTER_kAradJumpingCharacter::sendHelperBox(
    CUser* user,
    unsigned char ticket_type,
    unsigned char grow_type) {
    Inven_Item item;
    char title[21];
    char text[256];
    int reward_growth_value;

    if (ticket_type == 1u) {
        reward_growth_value = static_cast<int>(grow_type);
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "jumping_character_title_01", 0),
                     0x14u);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "jumping_character_mail_01", 0),
                     0xffu);
    } else {
        reward_growth_value = static_cast<int>(grow_type) + 16;
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "jumping_character_title_02", 0),
                     0x14u);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "jumping_character_mail_02", 0),
                     0xffu);
    }

    unsigned long reward_item_index;
    if (!G_CDataManager()->get_arad_jumping_script()->getCurRewardItem(
            user->get_charac_job(), reward_growth_value, reward_item_index)) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "bool ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::sendHelperBox(CUser*, byte, byte)",
            0x5D5,
            "JUMPING CHARACTER : REWARD ERROR - m_id:%u, charac_no:%u, tickettype:%d, job:%d, growValue:%d",
            user->get_acc_id(),
            user->get_charac_no(-1),
            ticket_type,
            user->get_charac_job(),
            reward_growth_value);
        return 0;
    }

    // IDA passes `item + 2` as the unsigned-long reward output to the
    // script, then calls CItemList::create_item and writes item + 7 = 1.
    item.item_id_ = static_cast<int>(reward_item_index);
    G_CDataManager()->get_arad_jumping_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, item, 1);
    item.set_add_info(1);

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        100000u,
        static_cast<unsigned int>(user->getCurCharacNo()),
        text,
        static_cast<int>(std::strlen(text)),
        0u,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
    return 1;
}

// IDA_EA: 0x81999F2
// IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kAradJumpingCharacter::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int length) {
    (void)length;  // The old leaf tests only the pointer before parsing it.

    if (user == 0) {
        return 1513;
    }
    if (user->getCurCharacR() == 0) {
        return 1518;
    }

    arad::SigAradJumpingCharacter* const signal =
        reinterpret_cast<arad::SigAradJumpingCharacter*>(payload);
    if (signal == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kAradJumpingCharacter::dispatch_sig(CUser*, char*, int)",
            0x5F3,
            "JUMPING_CHARACTER INTER_DSP(SigAradJumpingCharacter) null.");
        return 1524;
    }

    // 0x8199A71 reads this field before asking CUser for the writable
    // inventory.  Keep that ordering explicit because the signal buffer is
    // an asynchronously queued internal record.
    const uint16_t material_slot = signal->material_slot;
    CInventory* const inventory = user->getCurCharacInvenW();
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                               static_cast<int>(material_slot),
                               1,
                               static_cast<eItemDelReason>(3),
                               true) != 1) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET, 19);
        return 0;
    }

    // 0x8199AFB passes target 1, item-space 0, and the consumed slot.
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             static_cast<int>(material_slot));

    while (user->get_charac_level() <= 19) {
        user->SetCharacLevel(user->get_charac_level() + 1);
    }

    user->ChangeGrowType_GM(static_cast<int>(signal->grow_type), 0);
    if (signal->ticket_type == 2u) {
        while (user->get_charac_level() <= 49) {
            user->SetCharacLevel(user->get_charac_level() + 1);
        }
        user->ChangeGrowType_GM(user->getCurCharFirstGrowType(), 1);
        while (user->get_charac_level() <= 54) {
            user->SetCharacLevel(user->get_charac_level() + 1);
        }
    }

    user->send_skill_info();
    clearEpicQuest(user);

    // The original PacketGuard remains alive through all of the following
    // notifications, rather than being destroyed immediately after Send.
    PacketGuard quest_packet;
    user->getCurCharacQuestR()->get_quest_info(
        reinterpret_cast<char*>(&quest_packet));
    user->Send(quest_packet);
    user->send_clear_quest_list();
    user->sendCharacQp();

    user->setCurCharacExp(
        G_CDataManager()->get_level_exp(user->get_charac_level()));
    sendHelperBox(user, signal->ticket_type, signal->grow_type);

    user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                         ENUM_NOTIPACKET_EXP,
                         0);
    user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                         ENUM_NOTIPACKET_USERINFO,
                         0);
    user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                         ENUM_NOTIPACKET_USERINFO,
                         1);
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
