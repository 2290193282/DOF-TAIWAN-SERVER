#include "old_game/social/party/quick_party_system_manager_runtime.h"

#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/quick_party_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace QuickParty {

// IDA_EA: 0x8269BB6
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManagerC2Ev
// DEMANGLED: QuickParty::CQuickPartySystemManager::CQuickPartySystemManager(void)
CQuickPartySystemManager::CQuickPartySystemManager() : Base() {
    clear();
}

// IDA_EA: 0x8269BF8
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManagerD2Ev
// DEMANGLED: QuickParty::CQuickPartySystemManager::~CQuickPartySystemManager()
CQuickPartySystemManager::~CQuickPartySystemManager() {}

// IDA_EA: 0x8269C0C
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager21init_quick_party_poolEPK8CDungeon
// DEMANGLED: QuickParty::CQuickPartySystemManager::init_quick_party_pool(
//     CDungeon const*)
void CQuickPartySystemManager::init_quick_party_pool(const CDungeon* dungeon) {
    if (!dungeon) {
        return;
    }

    const std::vector<int>* const excluded_dungeons =
        G_CGameManager()->GetQuickPartyRewardManager()->get_dungeon_idx_list();
    for (std::vector<int>::const_iterator it = excluded_dungeons->begin();
         it != excluded_dungeons->end();
         ++it) {
        if (dungeon->get_index() == *it) {
            return;
        }
    }

    if (dungeon->isTowerOfDespairDungeon()) {
        return;
    }

    STQuickPartyPoolMap_Key key;
    STQuickPartyPoolMap_Data data;
    key.pool_id = static_cast<uint16_t>(dungeon->get_index());
    for (int difficulty = 0; difficulty <= 3; ++difficulty) {
        const bool single_pool_only =
            dungeon->getDungeonKind() == 1 ||
            dungeon->get_blood_dungeon_type() > 0 ||
            dungeon->getQuestNpcDungeon() != 0;
        if (!single_pool_only || difficulty == 0) {
            key.channel_type = static_cast<uint8_t>(difficulty);
            insert(std::make_pair(key, data));
        }
    }
}

// IDA_EA: 0x8269E34
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager24check_match_proper_partyERKNS_23STQuickPartyPoolMap_KeyEP6CParty
// DEMANGLED: QuickParty::CQuickPartySystemManager::check_match_proper_party(
//     QuickParty::STQuickPartyPoolMap_Key const&,CParty *)
int CQuickPartySystemManager::check_match_proper_party(
    const STQuickPartyPoolMap_Key& key,
    CParty* party) {
    if (!party) {
        return 1;
    }

    const int member_count = party->get_member_count();
    if (member_count <= 0 || member_count > 3) {
        return 1;
    }
    if (static_cast<int16_t>(key.pool_id) < 0) {
        LogManager::logFormat(
            1, "QuickParty.cpp",
            "ENUM_ERROR QuickParty::CQuickPartySystemManager::check_match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*)",
            0x16e, "QUICK_PARTY_LOG : DUNGEON INDEX ERROR!! (%d)",
            static_cast<int16_t>(key.pool_id));
    }

    CDungeon* const dungeon = G_CDataManager()->find_dungeon(
        static_cast<int16_t>(key.pool_id));
    if (!dungeon) {
        return 1;
    }
    if (dungeon->get_blood_dungeon_type() == 2 &&
        dungeon->get_limit_party_count() <= member_count) {
        return 1;
    }
    return party->checkInoutConditionDungeon(
        dungeon, static_cast<ENUM_DUNGEON_MODE>(0),
        ENUM_CMDPACKET_REGISTER_QUICK_PARTY);
}

// IDA_EA: 0x8269F3C
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18match_proper_partyERKNS_23STQuickPartyPoolMap_KeyEP6CPartyP5CUser
// DEMANGLED: QuickParty::CQuickPartySystemManager::match_proper_party(
//     QuickParty::STQuickPartyPoolMap_Key const&,CParty *,CUser *)
int CQuickPartySystemManager::match_proper_party(
    const STQuickPartyPoolMap_Key& key,
    CParty* party,
    CUser* user) {
    if (!party || party->get_quick_party_index() != -1) {
        if (party || !user) {
            LogManager::logFormat(
                1, "QuickParty.cpp",
                "ENUM_ERROR QuickParty::CQuickPartySystemManager::match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*, CUser*)",
                0x188,
                "QUICK_PARTY_LOG : ABNOMAL QUICK PARTY INDEX (%d)!!",
                party ? party->get_quick_party_index() : -1);
        } else {
            LogManager::logFormat(
                1, "QuickParty.cpp",
                "ENUM_ERROR QuickParty::CQuickPartySystemManager::match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*, CUser*)",
                0x186, "QUICK_PARTY_LOG : PARTY IS NULL!! (%d)",
                user->getCurCharacNo());
        }
        return 18;
    }

    const int match_result = check_match_proper_party(key, party);
    if (match_result) {
        return match_result;
    }

    const int member_count = party->get_member_count();
    CDungeon* const dungeon = G_CDataManager()->find_dungeon(
        static_cast<int16_t>(key.pool_id));
    if (!dungeon) {
        return 1;
    }
    int required_members = 4;
    if (dungeon->get_blood_dungeon_type() == 2) {
        required_members = dungeon->get_limit_party_count();
    }

    Base::iterator found = find(key);
    if (found == end()) {
        cMyTrace trace(
            "ENUM_ERROR QuickParty::CQuickPartySystemManager::match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*, CUser*)",
            470, 5);
        trace("QUICK_PARTY_LOG : NO EXIST KEY DATA!!");
        return 1;
    }

    std::list<int>* const pools[4] = {
        &found->second.first_list, &found->second.second_list,
        &found->second.third_list, &found->second.fourth_list};
    CQuickParty* quick_party = 0;
    for (int pool_index = required_members - member_count;; --pool_index) {
        if (pool_index <= 0) {
            return regist_quick_party(key, found->second, 0, party,
                                      member_count) != true;
        }
        if (!pools[pool_index]->empty()) {
            if (pool_index + member_count != required_members) {
                return regist_quick_party(key, found->second, pool_index,
                                          party, member_count) != true;
            }
            quick_party = find_quick_party_in_pool(*pools[pool_index]);
            if (quick_party) {
                break;
            }
        }
    }

    quick_party->set_party_index(party->GetPartyIndex());
    const int quick_party_count = quick_party->get_quick_party_count();
    if (quick_party_count == required_members) {
        start_game(quick_party);
        return 0;
    }

    cMyTrace trace(
        "ENUM_ERROR QuickParty::CQuickPartySystemManager::match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*, CUser*)",
        436, 0);
    trace("QUICK_PARTY_LOG : REPUSH QUICK PARTY IN POOL!! QUICK_PARTY_INDEX(%d), QUICK_PARTY_COUNT(%d)",
          quick_party->get_quick_party_index(), quick_party_count);
    if (push_quick_party_in_pool(key, found->second, party,
                                 quick_party_count, quick_party, true)) {
        return 0;
    }
    LogManager::logFormat(
        1, "QuickParty.cpp",
        "ENUM_ERROR QuickParty::CQuickPartySystemManager::match_proper_party(const QuickParty::STQuickPartyPoolMap_Key&, CParty*, CUser*)",
        0x1bc, "QUICK_PARTY_LOG : ABNOMAL QUICK PARTY!! (%d)",
        quick_party_count);
    return 1;
}

// IDA_EA: 0x826A762
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager27direct_entrance_quick_partyEP5CUser
// DEMANGLED: QuickParty::CQuickPartySystemManager::direct_entrance_quick_party(CUser *)
int CQuickPartySystemManager::direct_entrance_quick_party(CUser* user) {
    if (!user) {
        return 1;
    }
    CParty* const party = user->GetParty();
    if (!party) {
        return 1;
    }
    CQuickParty* const quick_party = G_CGameManager()->GetQuickParty(
        party->get_quick_party_index());
    if (!quick_party) {
        return 1;
    }

    const int quick_party_count = quick_party->get_quick_party_count();
    if (quick_party_count <= 1 &&
        quick_party->get_first_matching_cnt() == quick_party_count) {
        return 245;
    }

    if (quick_party->get_first_matching_cnt() >= quick_party_count &&
        quick_party->is_change_quick_party_count() != 1) {
        CParty* const quick_party_object = quick_party->get_party_object();
        if (!quick_party_object) {
            return 1;
        }
        CQuickPartySystemManager* const manager =
            G_CGameManager()->GetQuickPartySystemManager();
        if (manager->cancel_quick_party(quick_party_object, true, 0)) {
            PacketGuard packet;
            packet.clear();
            packet.put_header(0, 383);
            packet.finalize(true);
            quick_party_object->send_to_party(packet);
            return 1;
        }
    }

    STQuickPartyPoolMap_Key key;
    key.pool_id = static_cast<uint16_t>(quick_party->get_dungeon_index());
    key.channel_type = static_cast<uint8_t>(quick_party->get_dungeon_diff());
    G_CGameManager()->GetQuickPartySystemManager()->start_game(quick_party,
                                                                 key);
    return 0;
}

// IDA_EA: 0x826A972
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager10start_gameEPNS_11CQuickPartyERKNS_23STQuickPartyPoolMap_KeyE
// DEMANGLED: QuickParty::CQuickPartySystemManager::start_game(
//     QuickParty::CQuickParty *,QuickParty::STQuickPartyPoolMap_Key const&)
int CQuickPartySystemManager::start_game(CQuickParty* quick_party,
                                         const STQuickPartyPoolMap_Key& key) {
    if (!quick_party ||
        !check_quick_party_min_count(key,
                                     quick_party->get_quick_party_count())) {
        return 0;
    }
    if (!G_CGameManager()->GetQuickPartySystemManager()
             ->remove_quick_party_in_pool(quick_party, key)) {
        return 0;
    }
    start_game(quick_party);
    return 1;
}

// IDA_EA: 0x826A9FE
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager24find_quick_party_in_poolERSt4listIiSaIiEE
// DEMANGLED: QuickParty::CQuickPartySystemManager::find_quick_party_in_pool(
//     std::list<int,std::allocator<int> > &)
CQuickParty* CQuickPartySystemManager::find_quick_party_in_pool(
    std::list<int>& pool) {
    while (!pool.empty()) {
        const int quick_party_index = pool.front();
        pool.pop_front();
        CQuickParty* const quick_party =
            G_CGameManager()->GetQuickParty(quick_party_index);
        if (quick_party) {
            return quick_party;
        }
        cMyTrace trace(
            "QuickParty::CQuickParty* QuickParty::CQuickPartySystemManager::find_quick_party_in_pool(QuickParty::LIST_QUICK_PARTY&)",
            668, 5);
        trace("QUICK_PARTY_LOG : NO INTENDED ROUTINE!!");
    }
    return 0;
}

// IDA_EA: 0x826AB9E
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager27check_quick_party_min_countERKNS_23STQuickPartyPoolMap_KeyEi
// DEMANGLED: QuickParty::CQuickPartySystemManager::check_quick_party_min_count(
//     QuickParty::STQuickPartyPoolMap_Key const&,int)
bool CQuickPartySystemManager::check_quick_party_min_count(
    const STQuickPartyPoolMap_Key& key,
    int quick_party_count) {
    CDungeon* const dungeon = G_CDataManager()->find_dungeon(
        static_cast<int16_t>(key.pool_id));
    if (!dungeon) {
        return false;
    }
    if (dungeon->get_blood_dungeon_type() == 2) {
        return dungeon->get_limit_party_count() == quick_party_count;
    }
    if (dungeon->get_dimension_possible() <= 0) {
        int required_count = -1;
        CQuickPartyRewardManager* const rewards =
            G_CGameManager()->GetQuickPartyRewardManager();
        if (rewards) {
            required_count = rewards->getRequireQuickPartyMinCount(
                dungeon->ancient_dungeon_,
                static_cast<signed char>(key.channel_type));
        }
        if (required_count != -1 && required_count > quick_party_count) {
            return false;
        }
        return quick_party_count > 1;
    }
    return dungeon->get_dimension_min_partymem() + 1 <= quick_party_count;
}

// IDA_EA: 0x826ADB6
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18regist_quick_partyERKNS_23STQuickPartyPoolMap_KeyERNS_24STQuickPartyPoolMap_DataEiP6CPartyi
// DEMANGLED: QuickParty::CQuickPartySystemManager::regist_quick_party(
//     QuickParty::STQuickPartyPoolMap_Key const&,QuickParty::STQuickPartyPoolMap_Data &,int,CParty *,int)
bool CQuickPartySystemManager::regist_quick_party(
    const STQuickPartyPoolMap_Key& key,
    STQuickPartyPoolMap_Data& pool,
    int pool_index,
    CParty* party,
    int member_count) {
    if (!party) {
        return false;
    }
    int quick_party_count = 0;
    CQuickParty* const quick_party = pop_quick_party_in_pool(
        quick_party_count, pool, pool_index, party->GetPartyIndex(),
        member_count);
    return quick_party && push_quick_party_in_pool(
                              key, pool, party, quick_party_count,
                              quick_party, true);
}

// IDA_EA: 0x826AE64
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager23pop_quick_party_in_poolERiRNS_24STQuickPartyPoolMap_DataEiii
// DEMANGLED: QuickParty::CQuickPartySystemManager::pop_quick_party_in_pool(
//     int &,QuickParty::STQuickPartyPoolMap_Data &,int,int,int)
CQuickParty* CQuickPartySystemManager::pop_quick_party_in_pool(
    int& quick_party_count,
    STQuickPartyPoolMap_Data& pool,
    int pool_index,
    int party_index,
    int member_count) {
    if (pool_index < 0 || pool_index >= 4) {
        return 0;
    }
    CQuickParty* quick_party = 0;
    if (pool_index != 0) {
        std::list<int>* const lists[4] = {&pool.first_list, &pool.second_list,
                                          &pool.third_list, &pool.fourth_list};
        quick_party = find_quick_party_in_pool(*lists[pool_index]);
        if (quick_party) {
            quick_party->set_party_index(party_index);
            quick_party_count = quick_party->get_quick_party_count();
            return quick_party;
        }
        cMyTrace trace(
            "QuickParty::CQuickParty* QuickParty::CQuickPartySystemManager::pop_quick_party_in_pool(int&, QuickParty::STQuickPartyPoolMap_Data&, int, int, int)",
            850, 5);
        trace("QUICK_PARTY_LOG : NO INTENDED ROUTINE!!");
    }
    quick_party = G_CGameManager()->GetQuickParty();
    if (!quick_party) {
        return 0;
    }
    quick_party->set_party_index(party_index);
    quick_party_count = member_count;
    quick_party->set_first_matching_cnt(static_cast<char>(member_count));
    return quick_party;
}

// IDA_EA: 0x826B222
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager16make_quick_partyEPNS_11CQuickPartyE
// DEMANGLED: QuickParty::CQuickPartySystemManager::make_quick_party(QuickParty::CQuickParty *)
CParty* CQuickPartySystemManager::make_quick_party(CQuickParty* quick_party) {
    if (!quick_party) {
        return 0;
    }

    std::list<CUser*> detached_users;
    CParty* destination = 0;
    for (int slot = 0; slot <= 3; ++slot) {
        CParty* const candidate = G_CGameManager()->GetParty(
            quick_party->get_party_index(slot));
        if (!candidate) {
            continue;
        }
        if (candidate->GetEPLPState() > 1 || candidate->is_quick_party()) {
            destination = candidate;
            break;
        }
    }
    if (!destination) {
        for (int slot = 0; slot <= 3; ++slot) {
            destination = G_CGameManager()->GetParty(
                quick_party->get_party_index(slot));
            if (destination) {
                break;
            }
        }
    }
    if (!destination) {
        quick_party->destroy();
        LogManager::logFormat(
            1, "QuickParty.cpp",
            "CParty* QuickParty::CQuickPartySystemManager::make_quick_party(QuickParty::CQuickParty*)",
            0x3d0, "QUICK_PARTY_LOG : PARTY IS NULL!!");
        return 0;
    }

    destination->set_state(1);
    destination->set_quick_party(true);
    destination->setDungIndex(1);
    destination->setDungDiffi(0);
    if (destination->IsAutoCreated()) {
        destination->SetSinglePlay(false);
        destination->SetAutoCreated(false);
        G_GameWorld()->send_party_info_to_all(destination, 0);
    }
    for (int party_slot = 0; party_slot <= 3; ++party_slot) {
        const int party_index = quick_party->get_party_index(party_slot);
        if (party_index == -1) {
            continue;
        }
        CParty* const source = G_CGameManager()->GetParty(party_index);
        if (!source || source == destination) {
            continue;
        }
        for (int user_slot = 0; user_slot <= 3; ++user_slot) {
            if (source->checkValidUser(user_slot) == 1) {
                source->delete_party_for_quick_party(
                    source->get_user(user_slot), detached_users);
            }
        }
    }
    for (std::list<CUser*>::iterator it = detached_users.begin();
         it != detached_users.end(); ++it) {
        destination->join_user(*it);
    }
    quick_party->destroy();
    destination->init_quick_party_data();
    return destination;
}

// IDA_EA: 0x826B58A
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager10start_gameEPNS_11CQuickPartyE
// DEMANGLED: QuickParty::CQuickPartySystemManager::start_game(QuickParty::CQuickParty *)
void CQuickPartySystemManager::start_game(CQuickParty* quick_party) {
    if (!quick_party) {
        LogManager::logFormat(
            1, "QuickParty.cpp",
            "void QuickParty::CQuickPartySystemManager::start_game(QuickParty::CQuickParty*)",
            0x40c, "QUICK_PARTY_LOG : QUICK PARTY IS NULL!!");
        return;
    }
    if (quick_party->get_quick_party_count() <= 1) {
        LogManager::logFormat(
            1, "QuickParty.cpp",
            "void QuickParty::CQuickPartySystemManager::start_game(QuickParty::CQuickParty*)",
            0x412,
            "QUICK_PARTY_LOG : NO START!! BECAUSE OF QUICK PARTY COUNT 1 !!");
        return;
    }
    CParty* const party = make_quick_party(quick_party);
    if (!party) {
        LogManager::logFormat(
            1, "QuickParty.cpp",
            "void QuickParty::CQuickPartySystemManager::start_game(QuickParty::CQuickParty*)",
            0x41e, "QUICK_PARTY_LOG : NO EXIST PARTY!!");
        return;
    }
    insert_timer_waiting_start_game(party);
    party->send_party_realtime_info();
    party->send_party_ipinfo();
    G_GameWorld()->send_party_info_to_all(party, 2);
    party->send_quick_party_matching_result(true, 0);
    if (party->GetEPLPState() <= 1) {
        party->game_start(party->getManager());
    }
}

// IDA_EA: 0x826ACEA
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager35insert_timer_waiting_party_matchingEPNS_11CQuickPartyE
// DEMANGLED: QuickParty::CQuickPartySystemManager::insert_timer_waiting_party_matching(QuickParty::CQuickParty *)
int CQuickPartySystemManager::insert_timer_waiting_party_matching(
    CQuickParty* quick_party) {
    const int key = quick_party->gen_timer_key(
        static_cast<TIMER_MESSAGE>(158));
    return G_TimerQueue()->InsertTimer(1,
                                       quick_party->get_quick_party_index(),
                                       158,
                                       20,
                                       key,
                                       158);
}

// IDA_EA: 0x826AD50
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager31insert_timer_waiting_start_gameEP6CParty
// DEMANGLED: QuickParty::CQuickPartySystemManager::insert_timer_waiting_start_game(CParty *)
int CQuickPartySystemManager::insert_timer_waiting_start_game(CParty* party) {
    const int key = party->gen_timer_key(static_cast<TIMER_MESSAGE>(45));
    return G_TimerQueue()->InsertTimer(1, party->GetPartyIndex(), 45, 90,
                                       key, 45);
}

// IDA_EA: 0x826AA9A
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager26remove_quick_party_in_poolEPNS_11CQuickPartyERKNS_23STQuickPartyPoolMap_KeyE
// DEMANGLED: QuickParty::CQuickPartySystemManager::remove_quick_party_in_pool(QuickParty::CQuickParty *,QuickParty::STQuickPartyPoolMap_Key const&)
bool CQuickPartySystemManager::remove_quick_party_in_pool(
    CQuickParty* quick_party,
    const STQuickPartyPoolMap_Key& key) {
    if (!quick_party) {
        return false;
    }
    const unsigned int pool_index =
        static_cast<unsigned char>(quick_party->get_pool_index());
    if (pool_index >= 4u) {
        return false;
    }
    Base::iterator found = find(key);
    if (found == end()) {
        quick_party->destroy();
        cMyTrace trace(
            "bool QuickParty::CQuickPartySystemManager::remove_quick_party_in_pool(QuickParty::CQuickParty*, const QuickParty::STQuickPartyPoolMap_Key&)",
            698,
            5);
        trace("QUICK_PARTY_LOG : NO EXIST KEY DATA!!");
        return false;
    }
    std::list<int>* const pools[] = {
        &found->second.first_list,
        &found->second.second_list,
        &found->second.third_list,
        &found->second.fourth_list,
    };
    pools[pool_index]->remove(quick_party->get_quick_party_index());
    return true;
}

// IDA_EA: 0x826AFB4
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager24push_quick_party_in_poolERKNS_23STQuickPartyPoolMap_KeyERNS_24STQuickPartyPoolMap_DataEP6CPartyiPNS_11CQuickPartyEb
// DEMANGLED: QuickParty::CQuickPartySystemManager::push_quick_party_in_pool(QuickParty::STQuickPartyPoolMap_Key const&,QuickParty::STQuickPartyPoolMap_Data &,CParty *,int,QuickParty::CQuickParty *,bool)
bool CQuickPartySystemManager::push_quick_party_in_pool(
    const STQuickPartyPoolMap_Key& key,
    STQuickPartyPoolMap_Data& pool,
    CParty* party,
    int quick_party_count,
    CQuickParty* quick_party,
    bool insert_timer) {
    if (quick_party && quick_party_count > 0 && quick_party_count <= 3) {
        quick_party->set_dungeon_index(static_cast<int16_t>(key.pool_id));
        quick_party->set_dungeon_diff(static_cast<signed char>(key.channel_type));
        quick_party->set_pool_index(quick_party_count);
        if (party) {
            party->set_quick_party_index(
                quick_party->get_quick_party_index());
        }

        std::list<int>* const lists[4] = {&pool.first_list,
                                          &pool.second_list,
                                          &pool.third_list,
                                          &pool.fourth_list};
        lists[quick_party_count]->push_back(
            quick_party->get_quick_party_index());
        if (insert_timer) {
            insert_timer_waiting_party_matching(quick_party);
        }
        const unsigned short master_uid = static_cast<unsigned short>(
            quick_party->get_matching_quick_party_master_unique_id());
        const int current_count = quick_party->get_quick_party_count();
        const bool changed =
            quick_party->get_first_matching_cnt() < current_count ||
            quick_party->is_change_quick_party_count() != 0;
        quick_party->send_matching_update_quick_party(
            static_cast<char>(quick_party_count), master_uid, changed);
        return true;
    }

    if (quick_party) {
        for (int slot = 0; slot <= 3; ++slot) {
            CParty* const current_party = quick_party->get_party_object(slot);
            if (!current_party) {
                continue;
            }
            current_party->init_quick_party_data();
            PacketGuard packet;
            packet.put_header(0, 383);
            packet.finalize(true);
            current_party->send_to_party(packet);
        }
        quick_party->destroy();
    }
    LogManager::logFormat(
        1,
        "QuickParty.cpp",
        "bool QuickParty::CQuickPartySystemManager::push_quick_party_in_pool(const QuickParty::STQuickPartyPoolMap_Key&, QuickParty::STQuickPartyPoolMap_Data&, CParty*, int, QuickParty::CQuickParty*, bool)",
        0x383,
        "QUICK_PARTY_LOG : ABNOMAL QUICK PARTY!! REGIST POOL INDEX(%d)",
        quick_party_count);
    return false;
}

// IDA_EA: 0x826A386
// IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18cancel_quick_partyEP6CPartybP5CUser
// DEMANGLED: QuickParty::CQuickPartySystemManager::cancel_quick_party(CParty *,bool,CUser *)
bool CQuickPartySystemManager::cancel_quick_party(CParty* party,
                                                  bool eplp_checkout,
                                                  CUser* user) {
    if (!party || party->get_quick_party_index() == -1) {
        if (party || !user) {
            LogManager::logFormat(
                1,
                "QuickParty.cpp",
                "bool QuickParty::CQuickPartySystemManager::cancel_quick_party(CParty*, bool, CUser*)",
                0x1e5,
                "QUICK_PARTY_LOG : ABNOMAL QUICK PARTY INDEX (%d)!!",
                party ? party->get_quick_party_index() : -1);
        } else {
            LogManager::logFormat(
                1,
                "QuickParty.cpp",
                "bool QuickParty::CQuickPartySystemManager::cancel_quick_party(CParty*, bool, CUser*)",
                0x1e3,
                "QUICK_PARTY_LOG : PARTY IS NULL!! (%d)",
                user->get_charac_no(-1));
        }
        return false;
    }

    CQuickParty* const quick_party =
        G_CGameManager()->GetQuickParty(party->get_quick_party_index());
    if (!quick_party) {
        LogManager::logFormat(
            1,
            "QuickParty.cpp",
            "bool QuickParty::CQuickPartySystemManager::cancel_quick_party(CParty*, bool, CUser*)",
            0x1ec,
            "QUICK_PARTY_LOG : NO EXIST QUICK PARTY!!");
        return false;
    }

    if (static_cast<signed char>(party->GetEPLPState()) > 1 &&
        eplp_checkout) {
        party->SetSelectedEPLPCmd(-1);
        CBattle_Field& field = party->battle_field();
        field.setEplpWaitingTimer(
            static_cast<RandomBuffDungeon>(
                field.getRandomBuffDungeonType(
                    quick_party->get_dungeon_index())));
    }

    STQuickPartyPoolMap_Key key;
    key.pool_id = static_cast<uint16_t>(quick_party->get_dungeon_index());
    key.channel_type = static_cast<uint8_t>(quick_party->get_dungeon_diff());
    if (!remove_quick_party_in_pool(quick_party, key)) {
        return false;
    }
    if (!eplp_checkout) {
        quick_party->set_change_quick_party_count(true);
    }

    int quick_party_count = 0;
    if (party->get_member_count() == 1 || eplp_checkout) {
        quick_party->delete_party_index(party->GetPartyIndex());
        quick_party_count = quick_party->get_quick_party_count();
        party->init_quick_party_data();
        if (quick_party_count <= 0) {
            quick_party->destroy();
            return true;
        }
    } else {
        quick_party_count = quick_party->get_quick_party_count() - 1;
        if (quick_party_count <= 0) {
            quick_party->delete_party_index(party->GetPartyIndex());
            party->init_quick_party_data();
            quick_party->destroy();
            return false;
        }
    }

    Base::iterator found = find(key);
    if (found == end()) {
        return true;
    }
    return push_quick_party_in_pool(
        key, found->second, 0, quick_party_count, quick_party, true);
}

}  // namespace QuickParty
