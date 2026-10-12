#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_SYSTEM_MANAGER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_SYSTEM_MANAGER_RUNTIME_H_

#include <list>
#include <map>

#include "old_game/social/party/world_quick_party_pool.h"

class CParty;
class CUser;
class CDungeon;

namespace QuickParty {

class CQuickParty;

// IDA unit: 09_dungeon_pvp_party/QuickParty/CQuickPartySystemManager.
// The original C2 constructs this exact map object at this and immediately
// clears it.  It has no additional state or facade storage.
class CQuickPartySystemManager
    : public std::map<STQuickPartyPoolMap_Key, STQuickPartyPoolMap_Data> {
public:
    typedef std::map<STQuickPartyPoolMap_Key, STQuickPartyPoolMap_Data> Base;

    // IDA_EA: 0x8269BB6
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManagerC2Ev
    // DEMANGLED: QuickParty::CQuickPartySystemManager::CQuickPartySystemManager(void)
    CQuickPartySystemManager();

    // IDA_EA: 0x8269BF8
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManagerD2Ev
    // DEMANGLED: QuickParty::CQuickPartySystemManager::~CQuickPartySystemManager()
    ~CQuickPartySystemManager();

    // The following names are one atomic matchmaking transaction.  In
    // particular, do not substitute CParty's zero-argument predicate for
    // check_match_proper_party: the callee at IDA 0x85ABC80 has the separate
    // three-argument ABI _ZN6CParty26checkInoutConditionDungeonEPK8CDungeon17ENUM_DUNGEON_MODE14ENUM_CMDPACKET.
    // IDA_EA: 0x8269C0C
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager21init_quick_party_poolEPK8CDungeon
    void init_quick_party_pool(const CDungeon* dungeon);

    // IDA_EA: 0x8269E34
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager24check_match_proper_partyERKNS_23STQuickPartyPoolMap_KeyEP6CParty
    int check_match_proper_party(const STQuickPartyPoolMap_Key& key,
                                 CParty* party);

    // IDA_EA: 0x8269F3C
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18match_proper_partyERKNS_23STQuickPartyPoolMap_KeyEP6CPartyP5CUser
    int match_proper_party(const STQuickPartyPoolMap_Key& key,
                           CParty* party,
                           CUser* user);

    // IDA_EA: 0x826A386
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18cancel_quick_partyEP6CPartybP5CUser
    bool cancel_quick_party(CParty* party, bool eplp_checkout, CUser* user);

    // IDA_EA: 0x826A762
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager27direct_entrance_quick_partyEP5CUser
    int direct_entrance_quick_party(CUser* user);

    // IDA_EA: 0x826A972
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager10start_gameEPNS_11CQuickPartyERKNS_23STQuickPartyPoolMap_KeyE
    int start_game(CQuickParty* quick_party,
                   const STQuickPartyPoolMap_Key& key);

    // IDA_EA: 0x826A9FE
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager24find_quick_party_in_poolERNSt4listIiSaIiEEE
    CQuickParty* find_quick_party_in_pool(std::list<int>& pool);

    // IDA_EA: 0x826AA9A / 0x826AFB4
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager26remove_quick_party_in_poolEPNS_11CQuickPartyERKNS_23STQuickPartyPoolMap_KeyE
    //           _ZN10QuickParty24CQuickPartySystemManager24push_quick_party_in_poolERKNS_23STQuickPartyPoolMap_KeyERNS_24STQuickPartyPoolMap_DataEP6CPartyiPNS_11CQuickPartyEb
    bool remove_quick_party_in_pool(CQuickParty* quick_party,
                                    const STQuickPartyPoolMap_Key& key);

    // IDA_EA: 0x826AB9E
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager27check_quick_party_min_countERKNS_23STQuickPartyPoolMap_KeyEi
    bool check_quick_party_min_count(const STQuickPartyPoolMap_Key& key,
                                     int quick_party_count);
    bool push_quick_party_in_pool(const STQuickPartyPoolMap_Key& key,
                                  STQuickPartyPoolMap_Data& pool,
                                  CParty* party,
                                  int quick_party_count,
                                  CQuickParty* quick_party,
                                  bool insert_timer);

    // IDA_EA: 0x826ACEA
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager35insert_timer_waiting_party_matchingEPNS_11CQuickPartyE
    int insert_timer_waiting_party_matching(CQuickParty* quick_party);

    // IDA_EA: 0x826AD50
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager31insert_timer_waiting_start_gameEP6CParty
    int insert_timer_waiting_start_game(CParty* party);

    // IDA_EA: 0x826ADB6
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager18regist_quick_partyERKNS_23STQuickPartyPoolMap_KeyERNS_24STQuickPartyPoolMap_DataEiP6CPartyi
    bool regist_quick_party(const STQuickPartyPoolMap_Key& key,
                            STQuickPartyPoolMap_Data& pool,
                            int pool_index,
                            CParty* party,
                            int member_count);

    // IDA_EA: 0x826AE64
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager23pop_quick_party_in_poolERiRNS_24STQuickPartyPoolMap_DataEiii
    CQuickParty* pop_quick_party_in_pool(int& quick_party_count,
                                         STQuickPartyPoolMap_Data& pool,
                                         int pool_index,
                                         int party_index,
                                         int member_count);

    // IDA_EA: 0x826B222
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager16make_quick_partyEPNS_11CQuickPartyE
    CParty* make_quick_party(CQuickParty* quick_party);

    // IDA_EA: 0x826B58A
    // IDA_NAME: _ZN10QuickParty24CQuickPartySystemManager10start_gameEPNS_11CQuickPartyE
    void start_game(CQuickParty* quick_party);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CQuickPartySystemManager) == 0x18u,
              "CQuickPartySystemManager must remain the original map extent");
#endif

}  // namespace QuickParty

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_SYSTEM_MANAGER_RUNTIME_H_
