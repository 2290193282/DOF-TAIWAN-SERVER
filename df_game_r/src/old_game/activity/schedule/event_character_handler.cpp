#include "old_game/activity/schedule/event_character_handler.h"

#include <algorithm>

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_character_parameter_script.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x848E258
// IDA_NAME: _Z30CEventCharacterHandlerInstancev
// DEMANGLED: CEventCharacterHandlerInstance(void)
CEventCharacterHandler* CEventCharacterHandlerInstance() {
    static CEventCharacterHandler instance;
    return &instance;
}

// IDA_EA: 0x848E29E
// IDA_NAME: _ZN22CEventCharacterHandlerC2Ev
// DEMANGLED: CEventCharacterHandler::CEventCharacterHandler(void)
CEventCharacterHandler::CEventCharacterHandler() {}

// IDA_EA: 0x848E2A4
// IDA_NAME: _ZN22CEventCharacterHandler19_makeEventCharacterEP5CUseri
// DEMANGLED: CEventCharacterHandler::_makeEventCharacter(CUser *,int)
int CEventCharacterHandler::_makeEventCharacter(CUser* user,
                                                int target_level) {
    int event_job = -1;
    if (_checkMakeCommand(user, target_level, event_job) != 1) {
        return 0;
    }

    cMyTrace trace(
        "bool CEventCharacterHandler::_makeEventCharacter(CUser*, int)",
        43,
        0);
    trace("%s:'%s' Make Event Character(%d) level(%d).",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName(),
          event_job,
          target_level);

    STEventCharacterInfo info;
    if (G_CDataManager()->get_event_character_parameter_script()
            ->GetEventCharacterInfo(event_job, target_level, info) != 1) {
        return 0;
    }

    if (target_level <= 14) {
        _SetLevel(user, target_level);
    } else {
        _SetLevel(user, 15);
        if (info.grow_type) {
            _SetGrowType(user, 1, static_cast<int>(info.grow_type));
        }
        if (target_level > 15) {
            _SetLevel(user, target_level);
            if (target_level > 49 && info.awakening_type) {
                _SetGrowType(user, 2, static_cast<int>(info.awakening_type));
            }
        }
    }

    user->AddDungeonClear(2, 1);
    user->AddDungeonClear(26, 1);
    _SetClearedQuest(user);

    for (std::vector<std::pair<int, int> >::const_iterator it =
             info.skills.begin();
         it != info.skills.end();
         ++it) {
        _MasterNewSkill(user, it->first, it->second);
    }
    for (std::vector<STEventCharacterItem>::const_iterator it =
             info.items.begin();
         it != info.items.end();
         ++it) {
        _AddItem(user,
                 static_cast<unsigned long>(static_cast<unsigned int>(
                     it->item_index)),
                 it->count,
                 it->upgrade);
    }

    _UpdateEventCharacInfo(user);
    user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_NOTIPACKET>(37),
                         0);
    return 1;
}

// IDA_EA: 0x848E636
// IDA_NAME: _ZN22CEventCharacterHandler22_UpdateEventCharacInfoEP5CUser
// DEMANGLED: CEventCharacterHandler::_UpdateEventCharacInfo(CUser *)
int CEventCharacterHandler::_UpdateEventCharacInfo(CUser* user) {
    return user->setCurCharacEventCharacterGrowtype(static_cast<char>(-1));
}

// IDA_EA: 0x848E652
// IDA_NAME: _ZN22CEventCharacterHandler16_SetClearedQuestEP5CUser
// DEMANGLED: CEventCharacterHandler::_SetClearedQuest(CUser *)
char CEventCharacterHandler::_SetClearedQuest(CUser* user) {
    unsigned int quest_index = 0;
    switch (user->get_charac_job()) {
        case oldsrv::config::kCharacterJobSwordman:
            quest_index = 0x37f;
            break;
        case oldsrv::config::kCharacterJobFighter:
            quest_index = 0x382;
            break;
        case oldsrv::config::kCharacterJobGunner:
            quest_index = 0x380;
            break;
        case oldsrv::config::kCharacterJobMage:
            quest_index = 0x383;
            break;
        case oldsrv::config::kCharacterJobPriest:
            quest_index = 0x381;
            break;
        case oldsrv::config::kCharacterJobAtGunner:
            quest_index = 0x13f9;
            break;
        case oldsrv::config::kCharacterJobThief:
            quest_index = 0x384;
            break;
        default: return 0;
    }
    return static_cast<char>(
        user->getCurCharacQuestW()->clear_quest.setClearedQuest(quest_index));
}

// IDA_EA: 0x848E7D6
// IDA_NAME: _ZN22CEventCharacterHandler15_MasterNewSkillEP5CUserii
// DEMANGLED: CEventCharacterHandler::_MasterNewSkill(CUser *,int,int)
int CEventCharacterHandler::_MasterNewSkill(CUser* user,
                                            int skill_index,
                                            int target_level) {
    CSkill* const skill =
        G_CDataManager()->find_skill(user->get_charac_job(), skill_index);
    if (!skill) {
        return 0;
    }

    SkillSlot* const skills = user->getCurCharacSkillR();
    const int slot_index = skills->get_skillslot_no(
        skill_index, skill->get_group(), ENUM_SKILL_TREE_0, true);
    _Mastered_skill* const skill_tree =
        skills->get_mastered_skill_tree(ENUM_SKILL_TREE_0);
    if (!skill_tree) {
        return 0;
    }

    stBuySkillInfo buy_info;
    buy_info.skill_tree_index = 0;
    buy_info.skill_index = static_cast<uint8_t>(skill_index);
    buy_info.skill_level_delta = static_cast<int8_t>(
        target_level - static_cast<int>(skill_tree[slot_index].skill_level));
    if (buy_info.skill_level_delta <= 0) {
        return 0;
    }
    user->master_new_skill(buy_info, true);
    return 1;
}

// IDA_EA: 0x848E8E2
// IDA_NAME: _ZN22CEventCharacterHandler8_AddItemEP5CUsermii
// DEMANGLED: CEventCharacterHandler::_AddItem(CUser *,unsigned long,int,int)
int CEventCharacterHandler::_AddItem(CUser* user,
                                     unsigned long item_index,
                                     int count,
                                     int upgrade) {
    if (item_index == static_cast<unsigned long>(-1)) {
        const long end_time =
            static_cast<long>(GlobalData::s_systemTime_.getCurSec()) +
            86400;
        WongWork::CHandlePremium::handleSetUserPCRoom(
            user,
            static_cast<long>(GlobalData::s_systemTime_.getCurSec()),
            end_time);
    }

    CItem* const item =
        G_CDataManager()->find_item(static_cast<int>(item_index));
    if (item && item->is_stackable() &&
        static_cast<CStackableItem*>(item)->getStackableLimit() < count) {
        count = static_cast<CStackableItem*>(item)->getStackableLimit();
    }

    ENUM_ITEMSPACE item_space;
    const int slot = user->AddItem(
        static_cast<int>(item_index),
        count,
        static_cast<eItemAddReason>(6),
        item_space,
        upgrade);
    if (slot < 0) {
        return 0;
    }

    cMyTrace trace(
        "bool CEventCharacterHandler::_AddItem(CUser*, itemIndexOnlyServer_t, int, int)",
        218,
        0);
    trace("%s:'%s' GM Mode Add Item(%d).",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName(),
          static_cast<int>(item_index));
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1), item_space,
                             slot);
    return 1;
}

// IDA_EA: 0x848EA50
// IDA_NAME: _ZN22CEventCharacterHandler12_SetGrowTypeEP5CUserii
// DEMANGLED: CEventCharacterHandler::_SetGrowType(CUser *,int,int)
int CEventCharacterHandler::_SetGrowType(CUser* user,
                                         int grow_type_kind,
                                         int grow_type) {
    if (grow_type_kind == 1) {
        return user->ChangeGrowType_GM(grow_type, 0) != 0;
    }
    if (grow_type_kind != 2 || !user->getCurCharFirstGrowType()) {
        return 0;
    }
    return user->ChangeGrowType_GM(user->getCurCharFirstGrowType(),
                                   grow_type) != 0;
}

// IDA_EA: 0x848EADE
// IDA_NAME: _ZN22CEventCharacterHandler9_SetLevelEP5CUseri
// DEMANGLED: CEventCharacterHandler::_SetLevel(CUser *,int)
int CEventCharacterHandler::_SetLevel(CUser* user, int target_level) {
    if (target_level > oldsrv::config::kCharacterLevelMaximum) {
        return 0;
    }

    const int level_delta = target_level - user->get_charac_level();
    if (!level_delta) {
        return 0;
    }
    if (level_delta <= 0) {
        for (int offset = 0; offset > level_delta; --offset) {
            user->SetCharacLevel(std::min<int>(
                oldsrv::config::kCharacterLevelMaximum,
                user->get_charac_level() - 1));
        }
    } else {
        for (int offset = 0; offset < level_delta; ++offset) {
            user->SetCharacLevel(std::min<int>(
                oldsrv::config::kCharacterLevelMaximum,
                user->get_charac_level() + 1));
        }
    }
    return 1;
}

// IDA_EA: 0x848EBD6
// IDA_NAME: _ZN22CEventCharacterHandler17_checkMakeCommandEP5CUseriRi
// DEMANGLED: CEventCharacterHandler::_checkMakeCommand(CUser *,int,int &)
int CEventCharacterHandler::_checkMakeCommand(CUser* user,
                                              int target_level,
                                              int& event_job) {
    if (GlobalData::s_systemTime_.getCurSec() <= 1258642799 ||
        GlobalData::s_systemTime_.getCurSec() > 1262206800) {
        return 0;
    }
    if (target_level > oldsrv::config::kCharacterLevelMaximum) {
        return 0;
    }

    const int event_grow_type = static_cast<char>(
        user->getCurCharacEventCharacterGrowtype());
    if (event_grow_type > 6) {
        return 0;
    }
    event_job = event_grow_type + 10 * user->get_charac_job();
    return 1;
}
