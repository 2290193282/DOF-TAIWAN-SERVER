#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x830AAA4
// IDA_NAME: _ZN13CBattle_Field24getRandomBuffDungeonTypeEi
// DEMANGLED: CBattle_Field::getRandomBuffDungeonType(int)
int CBattle_Field::getRandomBuffDungeonType(int dungeon_index) {
    if (!dungeon_index) {
        CDungeon* const current = GetDungeon();
        if (!current) {
            return 7;
        }
        dungeon_index = current->get_index();
    }

    CDungeon* const dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon) {
        return 7;
    }

    QuickParty::CQuickPartyRewardManager* const rewards =
        G_CGameManager()->GetQuickPartyRewardManager();
    if (rewards) {
        const std::vector<int>* const excluded = rewards->get_dungeon_idx_list();
        for (std::vector<int>::const_iterator it = excluded->begin();
             it != excluded->end();
             ++it) {
            if (*it == dungeon_index) {
                return 7;
            }
        }
    }

    if (static_cast<signed char>(dungeon->get_dimension_possible()) > 0) {
        return 2;
    }
    if (dungeon->hell_dungeon_) {
        return 1;
    }
    if (static_cast<unsigned char>(dungeon->get_blood_dungeon_type()) == 1u) {
        return 4;
    }
    if (static_cast<unsigned char>(dungeon->get_blood_dungeon_type()) == 2u) {
        return 5;
    }
    if (dungeon->getDungeonKind() == 1) {
        return dungeon->limitOfStackableItemInTower() ? 3 : 6;
    }
    return 0;
}

// IDA_EA: 0x830AC7A
// IDA_NAME: _ZN13CBattle_Field19setEplpWaitingTimerEN10QuickParty17RandomBuffDungeonE
// DEMANGLED: CBattle_Field::setEplpWaitingTimer(QuickParty::RandomBuffDungeon)
WongWork::CDeathTower* CBattle_Field::setEplpWaitingTimer(
    QuickParty::RandomBuffDungeon random_buff_dungeon) {
    CParty* const party = party_;
    if (!party) {
        return 0;
    }
    const int kind = static_cast<int>(random_buff_dungeon);
    if (kind >= 4 && kind <= 5) {
        const int key = party->gen_timer_key(static_cast<TIMER_MESSAGE>(37));
        G_TimerQueue()->InsertTimer(1,
                                    party->GetPartyIndex(),
                                    37,
                                    30,
                                    key,
                                    37);
        return 0;
    }
    if (kind == 3 || kind == 6) {
        WongWork::CDeathTower* const tower =
            party->getManager()->getDeathTower();
        if (!tower) {
            return 0;
        }
        const int key = tower->genTimerKey(static_cast<TIMER_MESSAGE>(77));
        G_TimerQueue()->InsertTimer(6,
                                    tower->getIdx(),
                                    77,
                                    30,
                                    key,
                                    77);
        return tower;
    }
    const int key = party->gen_timer_key(static_cast<TIMER_MESSAGE>(21));
    G_TimerQueue()->InsertTimer(1,
                                party->GetPartyIndex(),
                                21,
                                30,
                                key,
                                21);
    return 0;
}
