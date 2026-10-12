#include "old_game/player/core/cuser.h"

#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/progression/quest/quest_runtime.h"

#include <map>
#include <stdint.h>

namespace {
CQuestShop* CurrentQuestShop(CUser* user) {
    return static_cast<CQuestShop*>(user->GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
}

}  // namespace

// IDA_EA: 0x868AC66
// IDA_NAME: _ZN5CUser13incQuestPointEiib
// DEMANGLED: CUser::incQuestPoint(int,int,bool)
CQuestShop* CUser::incQuestPoint(int quest_point,
                                  int quest_index,
                                  bool suppress_packet) {
    CQuestShop* const quest_shop = CurrentQuestShop(this);
    if (!quest_shop) {
        return quest_shop;
    }

    const int previous_quest_point = quest_shop->getQp();
    quest_shop->addQp(quest_point);
    if (!suppress_packet) {
        quest_shop->sendCharacQp(this, 0);
    }

    // The old i386 function returns QPAdd's EAX value through its historical
    // CQuestShop* signature. No caller dereferences that value.
    return reinterpret_cast<CQuestShop*>(
        static_cast<uintptr_t>(history_log().QPAdd(
            previous_quest_point, quest_index, quest_point)));
}

// IDA_EA: 0x868AD88
// IDA_NAME: _ZN5CUser19clearInitQuestPointEv
// DEMANGLED: CUser::clearInitQuestPoint(void)
CQuestShop* CUser::clearInitQuestPoint() {
    CQuestShop* const quest_shop = CurrentQuestShop(this);
    if (quest_shop) {
        return quest_shop->setInitQp(0);
    }
    return quest_shop;
}

// IDA_EA: 0x868ADC2
// IDA_NAME: _ZN5CUser17incInitQuestPointEi
// DEMANGLED: CUser::incInitQuestPoint(int)
CQuestShop* CUser::incInitQuestPoint(int quest_point) {
    CQuestShop* const quest_shop = CurrentQuestShop(this);
    if (quest_shop) {
        return quest_shop->incInitQp(quest_point);
    }
    return quest_shop;
}

// IDA_EA: 0x868ADFC
// IDA_NAME: _ZN5CUser17setInitQuestPointEv
// DEMANGLED: CUser::setInitQuestPoint(void)
bool CUser::setInitQuestPoint() {
    clearInitQuestPoint();
    clearQuestPiece();

    CDataManager* const data_manager = G_CDataManager();
    std::multimap<int, int>* const quest_npcs =
        data_manager->get_arad_jumping_quest_list()->getQuestNPCList(
            static_cast<ENUM_QUEST_GRADE>(0));
    std::multimap<int, int>::const_iterator it = quest_npcs->begin();
    const std::multimap<int, int>::const_iterator end = quest_npcs->end();
    bool result = false;
    while (true) {
        result = it != end;
        if (!result) {
            break;
        }

        const unsigned int quest_index = static_cast<unsigned int>(it->second);
        Quest* const quest = data_manager->find_quest(
            static_cast<int>(quest_index));
        if (quest && getCurCharacQuestR()->clear_quest.isClearedQuest(
                         quest_index)) {
            incInitQuestPoint(quest->quest_point);
            incQuestPiece(quest->quest_piece, quest->quest_index, true);
        }
        ++it;
    }
    return result;
}

// IDA_EA: 0x868AF66
// IDA_NAME: _ZN5CUser13incQuestPieceEiib
// DEMANGLED: CUser::incQuestPiece(int,int,bool)
void CUser::incQuestPiece(int quest_piece,
                          int quest_index,
                          bool suppress_packet) {
    CQuestShop* const quest_shop = CurrentQuestShop(this);
    if (quest_shop) {
        quest_shop->getPiece();
        quest_shop->addPiece(quest_piece);
        if (!suppress_packet) {
            quest_shop->sendCharacQuestPiece(this);
        }
    }
    (void)quest_index;
}

// IDA_EA: 0x868AFD0
// IDA_NAME: _ZN5CUser15clearQuestPieceEv
// DEMANGLED: CUser::clearQuestPiece(void)
CQuestShop* CUser::clearQuestPiece() {
    CQuestShop* const quest_shop = CurrentQuestShop(this);
    if (quest_shop) {
        return quest_shop->setPiece(0);
    }
    return quest_shop;
}
