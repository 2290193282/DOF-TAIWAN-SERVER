#include "old_game/player/core/achievement_runtime.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/etc/title_book_info.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/title_book_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

const std::string& QuestName(const Quest* quest) {
    return quest->name;
}

int QuestIndex(const Quest* quest) {
    return quest->quest_index;
}

int QuestGrade(const Quest* quest) {
    return quest->quest_grade;
}

int QuestMinimumLevel(const Quest* quest) {
    return quest->minimum_level;
}

int QuestClearCondition(const Quest* quest) {
    return quest->trigger_type;
}

const std::vector<int>& QuestClearConditionValues(const Quest* quest) {
    return quest->clear_condition_values;
}

const PostalReward& QuestPostalReward(const Quest* quest) {
    return quest->postal_reward;
}

stTitleBookInfo* TitleBookInfo() {
    return G_CDataManager()->get_title_book_info();
}

CTitleBook* UserTitleBook(CUser* user) {
    return reinterpret_cast<CTitleBook*>(
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)));
}

uint32_t ItemId(const Inven_Item& item) {
    return static_cast<uint32_t>(item.item_id_);
}

void SetItemId(Inven_Item& item, int item_id) {
    item.item_id_ = item_id;
}

uint16_t ItemWord0B(const Inven_Item& item) {
    return item.avatar_select_;
}

struct AchievementItemPrototypeVTable {
    void (*destructor)(CItem* item);
    void (*deleting_destructor)(CItem* item);
    void (*make_item)(const CItem* item, Inven_Item& inventory_item);
};

#if defined(__i386__)
static_assert(offsetof(AchievementItemPrototypeVTable, make_item) == 0x08u,
              "CItem make-item vtable slot must stay +0x08");
#endif

const AchievementItemPrototypeVTable* ItemVTable(const CItem* item) {
    const AchievementItemPrototypeVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

}  // namespace

// IDA_EA: 0x828B614
// IDA_NAME: _ZN12CAchievementC2Ev
// DEMANGLED: CAchievement::CAchievement(void)
CAchievement::CAchievement()
    : owner_(0), achievements_(), no_clear_(0), state_25_(0) {}

// IDA_EA: 0x828F8AA
// IDA_NAME: _ZN12CAchievementD2Ev
// DEMANGLED: CAchievement::~CAchievement()
// IDA_EA: 0x828F90C
// IDA_NAME: _ZN12CAchievementD0Ev
// DEMANGLED: CAchievement::~CAchievement()
CAchievement::~CAchievement() {}

// IDA_EA: 0x828D8A2
// IDA_NAME: _ZN13stAchievement10isCompleteEv
// DEMANGLED: stAchievement::isComplete(void)
bool stAchievement::isComplete() {
    return progress[0] == 0 && progress[1] == 0 && progress[2] == 0;
}

// IDA_EA: 0x828B678
// IDA_NAME: _ZN12CAchievement10setTriggerEjttt
// DEMANGLED: CAchievement::setTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
void CAchievement::setTrigger(unsigned int achievement_id,
                              unsigned short first,
                              unsigned short second,
                              unsigned short third) {
    Quest* const quest =
        G_CDataManager()->find_quest(static_cast<int>(achievement_id));
    if (!quest || QuestGrade(quest) != 7 ||
        owner_->get_charac_level() < QuestMinimumLevel(quest)) {
        _sendSetTrigger(achievement_id, 1);
        return;
    }
    if (isClearedAchievement(achievement_id) ||
        !_verifyTrigger(achievement_id, first, second, third)) {
        _sendSetTrigger(achievement_id, 2);
        return;
    }
    _setTrigger(achievement_id, first, second, third);
    _sendSetTrigger(achievement_id, 0);
    _checkComplete(achievement_id);
}

// IDA_EA: 0x828B800
// IDA_NAME: _ZN12CAchievement14_verifyTriggerEjttt
// DEMANGLED: CAchievement::_verifyTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
bool CAchievement::_verifyTrigger(unsigned int achievement_id,
                                  unsigned short first,
                                  unsigned short second,
                                  unsigned short third) {
    (void)first;
    (void)second;
    (void)third;
    switch (achievement_id) {
        case 0x1983u:
        case 0x198Bu:
            if (owner_->get_charac_level() <= 69) {
                return false;
            }
            break;
        case 0x19C4u: {
            const time_t now =
                static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
            struct tm local_time;
            localtime_r(&now, &local_time);
            return local_time.tm_hour > 20;
        }
        default:
            break;
    }
    return true;
}

// IDA_EA: 0x828B896
// IDA_NAME: _ZN12CAchievement11_setTriggerEjttt
// DEMANGLED: CAchievement::_setTrigger(unsigned int,unsigned short,unsigned short,unsigned short)
void CAchievement::_setTrigger(unsigned int achievement_id,
                               unsigned short first,
                               unsigned short second,
                               unsigned short third) {
    std::map<unsigned int, stAchievement>::iterator found =
        achievements_.find(achievement_id);
    if (found != achievements_.end()) {
        const unsigned short values[3] = {first, second, third};
        for (unsigned int index = 0; index < 3u; ++index) {
            if (found->second.progress[index] < values[index]) {
                found->second.progress[index] = 0;
            } else {
                found->second.progress[index] -= values[index];
            }
        }
    } else {
        stAchievement achievement;
        achievement.achievement_id = achievement_id;
        Quest* const quest =
            G_CDataManager()->find_quest(static_cast<int>(achievement_id));
        if (!quest) {
            return;
        }
        if (quest->get_init_achievement_trigger(
                achievement.progress[0],
                achievement.progress[1],
                achievement.progress[2]) != 1) {
            _sendSetTrigger(achievement_id, 1);
            return;
        }
        const unsigned short values[3] = {first, second, third};
        for (unsigned int index = 0; index < 3u; ++index) {
            if (achievement.progress[index] < values[index]) {
                achievement.progress[index] = 0;
            } else {
                achievement.progress[index] -= values[index];
            }
        }
        achievements_.insert(
            std::make_pair(achievement_id, achievement));
    }
    alter();
}

// IDA_EA: 0x828BB14
// IDA_NAME: _ZN12CAchievement11_getTriggerEj
// DEMANGLED: CAchievement::_getTrigger(unsigned int)
stAchievement CAchievement::_getTrigger(unsigned int achievement_id) {
    stAchievement result;
    std::map<unsigned int, stAchievement>::iterator found =
        achievements_.find(achievement_id);
    if (found != achievements_.end()) {
        result = found->second;
    }
    return result;
}

// IDA_EA: 0x828BBAE
// IDA_NAME: _ZN12CAchievement14_checkCompleteEj
// DEMANGLED: CAchievement::_checkComplete(unsigned int)
void CAchievement::_checkComplete(unsigned int achievement_id) {
    if (!isClearedAchievement(achievement_id)) {
        return;
    }
    const Quest* const quest =
        G_CDataManager()->find_quest(static_cast<int>(achievement_id));
    if (!quest) {
        return;
    }

    ENUM_TITLE_BOOK_CATEGORY category =
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(0);
    int slot = 0;
    const int title_reward = quest->get_title_reward();
    if (title_reward) {
        CTitleBook::putTitleOnly(owner_, title_reward, category, slot);
    }
    _sendEachAchieveReward(quest);
    _sendCompleteReward(quest, category, slot);
    _sendComplete(achievement_id, title_reward, category, slot);
    CUser_history_log(owner_)->AchievementComplete(
        static_cast<int>(achievement_id));
}

// IDA_EA: 0x828BD5E
// IDA_NAME: _ZN12CAchievement16_getPostalRewardEP14stTitleSection
// DEMANGLED: CAchievement::_getPostalReward(stTitleSection *)
stRewardPostal* CAchievement::_getPostalReward(stTitleSection* section) {
    for (std::vector<stTitleRewardBasis*>::size_type index = 0;
         index < section->rewards.size();
         ++index) {
        stTitleRewardBasis* const reward = section->rewards[index];
        if (reward && reward->reward_kind == 3) {
            return static_cast<stRewardPostal*>(reward);
        }
    }
    return 0;
}

// IDA_EA: 0x828BDC6
// IDA_NAME: _ZN12CAchievement15_getTabCompleteEP14stTitleSection24ENUM_TITLE_BOOK_CATEGORY
// DEMANGLED: CAchievement::_getTabComplete(stTitleSection *,ENUM_TITLE_BOOK_CATEGORY)
bool CAchievement::_getTabComplete(stTitleSection* section,
                                   ENUM_TITLE_BOOK_CATEGORY category) {
    std::map<int, std::vector<stTitleElement> >::iterator found =
        TitleBookInfo()->title_collections.find(static_cast<int>(category));
    if (found == TitleBookInfo()->title_collections.end()) {
        return true;
    }

    const std::vector<stTitleElement> elements(found->second);
    for (unsigned int slot = static_cast<unsigned int>(section->start_slot);
         slot < static_cast<unsigned int>(section->end_slot);
         ++slot) {
        if (elements[slot].title_index != -1 &&
            elements[slot].condition_type != -1 &&
            !isClearedAchievement(
                static_cast<unsigned int>(elements[slot].condition_type))) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x828BF2A
// IDA_NAME: _ZN12CAchievement19_sendCompleteRewardEPK5Quest24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CAchievement::_sendCompleteReward(Quest const*,ENUM_TITLE_BOOK_CATEGORY,int)
void CAchievement::_sendCompleteReward(const Quest* quest,
                                       ENUM_TITLE_BOOK_CATEGORY category,
                                       int slot) {
    (void)quest;
    const int section_index = _getSectionIdx(category, slot);
    if (section_index == -1) {
        return;
    }
    stTitleSection section(TitleBookInfo()->sections[section_index]);
    stRewardPostal* const postal = _getPostalReward(&section);
    if (postal && _getTabComplete(&section, category)) {
        _sendPostal(static_cast<int>(postal->first_dword),
                    static_cast<int>(postal->second_dword),
                    postal->mail_title,
                    postal->mail_text);
    }
}

// IDA_EA: 0x828C086
// IDA_NAME: _ZN12CAchievement22_sendEachAchieveRewardEPK5Quest
// DEMANGLED: CAchievement::_sendEachAchieveReward(Quest const*)
void CAchievement::_sendEachAchieveReward(const Quest* quest) {
    const PostalReward& reward = QuestPostalReward(quest);
    if (reward.first_dword) {
        _sendPostal(static_cast<int>(reward.first_dword),
                    static_cast<int>(reward.second_dword),
                    reward.first_string,
                    reward.second_string);
    }
}

// IDA_EA: 0x828C152
// IDA_NAME: _ZN12CAchievement11_sendPostalEiiSsSs
// DEMANGLED: CAchievement::_sendPostal(int,int,std::string,std::string)
int CAchievement::_sendPostal(int item_id,
                              int add_info,
                              std::string title,
                              std::string text) {
    Inven_Item item;
    CItem* const item_data = G_CDataManager()->find_item(item_id);
    if (!item_data) {
        return 0;
    }
    SetItemId(item, item_id);
    ItemVTable(item_data)->make_item(item_data, item);
    item.set_add_info(add_info);
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title.c_str(),
        item,
        0,
        static_cast<unsigned int>(owner_->getCurCharacNo()),
        text.c_str(),
        static_cast<int>(text.size()),
        0,
        static_cast<ENUM_SERVER_GROUP>(owner_->GetServerGroup()),
        false,
        false);
}

// IDA_EA: 0x828C252
// IDA_NAME: _ZN12CAchievement15_sendSetTriggerEjh
// DEMANGLED: CAchievement::_sendSetTrigger(unsigned int,unsigned char)
void CAchievement::_sendSetTrigger(unsigned int achievement_id,
                                   unsigned char result) {
    const stAchievement achievement = _getTrigger(achievement_id);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_ACHIEVEMENT_TRIGGER);
    if (result) {
        packet.put_byte(0);
        packet.put_byte(result);
        packet.put_int(static_cast<int>(achievement.achievement_id));
    } else {
        packet.put_byte(1);
        packet.put_int(static_cast<int>(achievement.achievement_id));
        packet.put_short(achievement.progress[0]);
        packet.put_short(achievement.progress[1]);
        packet.put_short(achievement.progress[2]);
    }
    packet.finalize(true);
    owner_->Send(packet);
}

// IDA_EA: 0x828C3A4
// IDA_NAME: _ZN12CAchievement13_sendCompleteEji24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CAchievement::_sendComplete(unsigned int,int,ENUM_TITLE_BOOK_CATEGORY,int)
void CAchievement::_sendComplete(unsigned int achievement_id,
                                 int title_reward,
                                 ENUM_TITLE_BOOK_CATEGORY category,
                                 int slot) {
    const stAchievement achievement = _getTrigger(achievement_id);
    PacketGuard packet;
    packet.put_header(0, 359);
    packet.put_int(static_cast<int>(achievement.achievement_id));
    packet.put_int(static_cast<int>(category));
    packet.put_int(slot);
    packet.put_int(title_reward);
    const Inven_Item title =
        UserTitleBook(owner_)->getTitleSlotR(category, slot);
    packet.put_short(ItemWord0B(title));
    packet.finalize(true);
    owner_->Send(packet);
}

// IDA_EA: 0x828C4DE
// IDA_NAME: _ZN12CAchievement8sendListEv
// DEMANGLED: CAchievement::sendList(void)
void CAchievement::sendList() {
    stAchievement_DBData data;
    getData(reinterpret_cast<char*>(&data));
    PacketGuard packet;
    packet.put_header(0, 358);
    packet.put_int(static_cast<int>(data.count));
    for (unsigned int index = 0; index < data.count; ++index) {
        packet.put_int(static_cast<int>(data.achievements[index].achievement_id));
        packet.put_short(data.achievements[index].progress[0]);
        packet.put_short(data.achievements[index].progress[1]);
        packet.put_short(data.achievements[index].progress[2]);
    }
    packet.finalize(true);
    owner_->Send(packet);
}

// IDA_EA: 0x828C676
// IDA_NAME: _ZN12CAchievement6_resetEv
// DEMANGLED: CAchievement::_reset(void)
int CAchievement::_reset() {
    owner_ = 0;
    achievements_.clear();
    no_clear_ = 0;
    state_25_ = 0;
    return static_cast<int>(reinterpret_cast<intptr_t>(this));
}

// IDA_EA: 0x828C6A4
// IDA_NAME: _ZNK12CAchievement7getDataEPc
// DEMANGLED: CAchievement::getData(char *)const
void CAchievement::getData(char* data) const {
    stAchievement_DBData* const output =
        reinterpret_cast<stAchievement_DBData*>(data);
    unsigned int index = 0;
    for (std::map<unsigned int, stAchievement>::const_iterator it =
             achievements_.begin();
         it != achievements_.end();
         ++it, ++index) {
        output->achievements[index] = it->second;
    }
    output->count = static_cast<uint32_t>(achievements_.size());
    output->last_update_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
}

// IDA_EA: 0x828C774
// IDA_NAME: _ZN12CAchievement9_saveDataEP5CUser
// DEMANGLED: CAchievement::_saveData(CUser *)
int CAchievement::_saveData(CUser* user) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("Achievement.cpp", 468);
    CStreamGuard guard(stream, true);
    *guard << 0x286;
    *guard << user->GetUID();
    stAchievement_DBSaveData* const payload =
        guard->GetInBuffer<stAchievement_DBSaveData>();
    if (!payload) {
        return 0;
    }
    std::memset(payload, 0, sizeof(*payload));
    getData(reinterpret_cast<char*>(payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x828C8B8
// IDA_NAME: _ZN12CAchievement8loadDataEP5CUserPc
// DEMANGLED: CAchievement::loadData(CUser *,char *)
bool CAchievement::loadData(CUser* user, char* data) {
    stAchievement_DBData* const input =
        reinterpret_cast<stAchievement_DBData*>(data);
    for (unsigned int index = 0; index < input->count; ++index) {
        achievements_.insert(
            std::make_pair(input->achievements[index].achievement_id,
                           input->achievements[index]));
    }
    owner_ = user;
    _verifyAchievement(input->last_update_time);
    return true;
}

// IDA_EA: 0x828C976
// IDA_NAME: _ZN12CAchievement6adjustEv
// DEMANGLED: CAchievement::adjust(void)
charac_expand::CData* CAchievement::adjust() {
    ENUM_TITLE_BOOK_CATEGORY category =
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(0);
    int slot = 0;
    const std::vector<stAdjust>& adjusts = TitleBookInfo()->adjusts;
    for (std::vector<stAdjust>::size_type index = 0;
         index < adjusts.size();
         ++index) {
        const stAdjust& adjust = adjusts[index];
        if (owner_->getCurCharacQuestR()->clear_quest.isClearedQuest(
                static_cast<unsigned int>(adjust.title_index))) {
            Quest* const quest =
                G_CDataManager()->find_quest(adjust.adjust_value);
            if (quest && QuestGrade(quest) == 7) {
                stAchievement achievement;
                quest->get_init_achievement_trigger(
                    achievement.progress[0],
                    achievement.progress[1],
                    achievement.progress[2]);
                _setTrigger(static_cast<unsigned int>(adjust.adjust_value),
                            achievement.progress[0],
                            achievement.progress[1],
                            achievement.progress[2]);
                CTitleBook::putTitleOnly(owner_,
                                         quest->get_title_reward(),
                                         category,
                                         slot);
            }
        }
    }

    if (owner_->get_charac_level() ==
        oldsrv::config::kCharacterLevelMaximum) {
        Quest* const quest = G_CDataManager()->find_quest(131);
        if (!quest) {
            return 0;
        }
        stAchievement achievement;
        quest->get_init_achievement_trigger(achievement.progress[0],
                                            achievement.progress[1],
                                            achievement.progress[2]);
        _setTrigger(0x1983u,
                    achievement.progress[0],
                    achievement.progress[1],
                    achievement.progress[2]);
        CTitleBook::putTitleOnly(owner_,
                                 quest->get_title_reward(),
                                 category,
                                 slot);
    }
    return alter();
}

// IDA_EA: 0x828CBE4
// IDA_NAME: _ZN12CAchievement20isClearedAchievementEj
// DEMANGLED: CAchievement::isClearedAchievement(unsigned int)
bool CAchievement::isClearedAchievement(unsigned int achievement_id) {
    std::map<unsigned int, stAchievement>::iterator found =
        achievements_.find(achievement_id);
    return found != achievements_.end() && found->second.isComplete();
}

// IDA_EA: 0x828CC56
// IDA_NAME: _ZN12CAchievement11_forceClearEj
// DEMANGLED: CAchievement::_forceClear(unsigned int)
charac_expand::CData* CAchievement::_forceClear(
    unsigned int achievement_id) {
    Quest* const quest =
        G_CDataManager()->find_quest(static_cast<int>(achievement_id));
    if (!quest) {
        return 0;
    }
    if (QuestGrade(quest) != 7) {
        return reinterpret_cast<charac_expand::CData*>(
            static_cast<intptr_t>(QuestGrade(quest)));
    }
    const int title_reward = quest->get_title_reward();
    if (!title_reward) {
        return 0;
    }

    if (isClearedAchievement(achievement_id)) {
        ENUM_TITLE_BOOK_CATEGORY category =
            static_cast<ENUM_TITLE_BOOK_CATEGORY>(5);
        int slot = 0;
        CTitleBook::_getFitSlot(title_reward, category, slot);
        const Inven_Item title =
            UserTitleBook(owner_)->getTitleSlotR(category, slot);
        if (!ItemId(title)) {
            const Inven_Item inventory_title =
                owner_->getCurCharacInvenR()->GetInvenSlot(
                    0, oldsrv::config::kEquipmentTypeTitle);
            if (ItemId(inventory_title) !=
                static_cast<uint32_t>(title_reward)) {
                CTitleBook::putTitleOnly(owner_, title_reward);
            }
        }
    } else {
        stAchievement achievement;
        quest->get_init_achievement_trigger(achievement.progress[0],
                                            achievement.progress[1],
                                            achievement.progress[2]);
        _setTrigger(achievement_id,
                    achievement.progress[0],
                    achievement.progress[1],
                    achievement.progress[2]);
        CTitleBook::putTitleOnly(owner_, title_reward);
    }
    return alter();
}

// IDA_EA: 0x828CE40
// IDA_NAME: _ZN12CAchievement11_forceResetEj
// DEMANGLED: CAchievement::_forceReset(unsigned int)
intptr_t CAchievement::_forceReset(unsigned int achievement_id) {
    Quest* const quest =
        G_CDataManager()->find_quest(static_cast<int>(achievement_id));
    if (!quest) {
        return 0;
    }
    if (QuestGrade(quest) != 7) {
        return static_cast<intptr_t>(QuestGrade(quest));
    }
    const int title_reward = quest->get_title_reward();
    if (!title_reward) {
        return 0;
    }

    if (isClearedAchievement(achievement_id)) {
        std::map<unsigned int, stAchievement>::iterator found =
            achievements_.find(achievement_id);
        if (found != achievements_.end()) {
            stAchievement achievement;
            const int trigger_result =
                quest->get_init_achievement_trigger(
                    achievement.progress[0],
                    achievement.progress[1],
                    achievement.progress[2]);
            if (trigger_result != 1) {
                return static_cast<intptr_t>(trigger_result ^ 1);
            }
            found->second.progress[0] = achievement.progress[0];
            found->second.progress[1] = achievement.progress[1];
            found->second.progress[2] = achievement.progress[2];
        }
        alter();
    }

    ENUM_TITLE_BOOK_CATEGORY category =
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(5);
    int slot = 0;
    CTitleBook::_getFitSlot(title_reward, category, slot);
    Inven_Item* const title =
        UserTitleBook(owner_)->getTitleSlot(category, slot);
    if (title) {
        if (ItemId(*title)) {
            return reinterpret_cast<intptr_t>(title->reset());
        }
        Inven_Item* const inventory_title =
            owner_->getCurCharacInvenW()->GetInvenRef(
                0, oldsrv::config::kEquipmentTypeTitle);
        if (inventory_title &&
            ItemId(*inventory_title) ==
                static_cast<uint32_t>(title_reward)) {
            inventory_title->reset();
            return static_cast<intptr_t>(owner_->SendUpdateItem(
                static_cast<CUser::eSendTarget>(1),
                static_cast<ENUM_ITEMSPACE>(0),
                oldsrv::config::kEquipmentTypeTitle));
        }
        if (inventory_title) {
            return static_cast<intptr_t>(title_reward);
        }
    }
    return 0;
}

// IDA_EA: 0x828D092
// IDA_NAME: _ZN12CAchievement18_verifyAchievementEj
// DEMANGLED: CAchievement::_verifyAchievement(unsigned int)
charac_expand::CData* CAchievement::_verifyAchievement(
    unsigned int last_update_time) {
    (void)last_update_time;
    const std::map<int, std::vector<stTitleElement> >& collections =
        TitleBookInfo()->title_collections;
    for (std::map<int, std::vector<stTitleElement> >::const_iterator
             collection = collections.begin();
         collection != collections.end();
         ++collection) {
        const std::vector<stTitleElement>& elements = collection->second;
        for (std::vector<stTitleElement>::size_type index = 0;
             index < elements.size();
             ++index) {
            const int achievement_id = elements[index].condition_type;
            if (achievement_id == -1) {
                continue;
            }
            Quest* const quest =
                G_CDataManager()->find_quest(achievement_id);
            if (!quest || QuestGrade(quest) != 7 ||
                QuestClearCondition(quest) != 12 ||
                QuestClearConditionValues(quest).empty()) {
                continue;
            }
            const unsigned int clear_quest = static_cast<unsigned int>(
                QuestClearConditionValues(quest)[0]);
            if (owner_->getCurCharacQuestR()->clear_quest.isClearedQuest(
                    clear_quest)) {
                _forceClear(static_cast<unsigned int>(achievement_id));
            }
        }
    }

    const unsigned int level_achievements[2] = {6531u, 6539u};
    if (owner_->get_charac_level() > 69) {
        _forceClear(0x1983u);
    } else {
        for (unsigned int index = 0; index < 2u; ++index) {
            _forceReset(level_achievements[index]);
        }
    }

    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            owner_->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (missions->isClearMission(131) ||
        missions->isClearMission(135)) {
        return _forceClear(6592u);
    }
    return 0;
}

// IDA_EA: 0x828D338
// IDA_NAME: _ZN12CAchievement8_gm_initEv
// DEMANGLED: CAchievement::_gm_init(void)
charac_expand::CData* CAchievement::_gm_init() {
    for (std::map<unsigned int, stAchievement>::iterator it =
             achievements_.begin();
         it != achievements_.end();
         ++it) {
        Quest* const quest =
            G_CDataManager()->find_quest(static_cast<int>(it->first));
        if (!quest ||
            quest->get_init_achievement_trigger(
                it->second.progress[0],
                it->second.progress[1],
                it->second.progress[2]) != 1) {
            break;
        }
        _sendSetTrigger(it->first, 0);
    }
    return alter();
}

// IDA_EA: 0x828D450
// IDA_NAME: _ZN12CAchievement10_gm_reportERSs
// DEMANGLED: CAchievement::_gm_report(std::string &)
bool CAchievement::_gm_report(std::string& report) {
    char line[1024];
    std::memset(line, 0, sizeof(line));
    for (int achievement_id = 6501; achievement_id <= 7099;
         ++achievement_id) {
        Quest* const quest =
            G_CDataManager()->find_quest(achievement_id);
        if (!quest || QuestGrade(quest) != 7) {
            continue;
        }
        stAchievement initial;
        quest->get_init_achievement_trigger(initial.progress[0],
                                            initial.progress[1],
                                            initial.progress[2]);
        std::map<unsigned int, stAchievement>::iterator found =
            achievements_.find(static_cast<unsigned int>(achievement_id));
        const stAchievement& current =
            found != achievements_.end() ? found->second : initial;
        std::sprintf(line,
                     "%d\t\"%s\"\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                     QuestIndex(quest),
                     QuestName(quest).c_str(),
                     quest->get_title_reward(),
                     initial.progress[0],
                     initial.progress[1],
                     initial.progress[2],
                     current.progress[0],
                     current.progress[1],
                     current.progress[2]);
        report += line;
        std::memset(line, 0, sizeof(line));
    }
    return false;
}

// IDA_EA: 0x828D738
// IDA_NAME: _ZN12CAchievement12_gm_clearAllEv
// DEMANGLED: CAchievement::_gm_clearAll(void)
bool CAchievement::_gm_clearAll() {
    for (int achievement_id = 6501; achievement_id <= 7099;
         ++achievement_id) {
        Quest* const quest =
            G_CDataManager()->find_quest(achievement_id);
        if (quest && QuestGrade(quest) == 7) {
            stAchievement achievement;
            quest->get_init_achievement_trigger(
                achievement.progress[0],
                achievement.progress[1],
                achievement.progress[2]);
            setTrigger(static_cast<unsigned int>(achievement_id),
                       achievement.progress[0],
                       achievement.progress[1],
                       achievement.progress[2]);
        }
    }
    return false;
}
