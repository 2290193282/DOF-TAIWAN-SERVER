#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/social/party/hellparty_characlink_abi.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <new>
#include <stdint.h>
#include <vector>

#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/dungeon/instance/monster_move_system.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/dungeon/instance/zombie_itembox_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/world/broadcast/world_messages.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/common_statistics_manager_abi.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"


namespace secretshop {
struct SECRET_SHOP_DATA;
}  // namespace secretshop

extern unsigned int get_rand_int(int limit)
    __asm__("_Z12get_rand_inti");
extern int CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT");
extern int CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT");
namespace {

const int kPartyMaxMemberCount = 4;

uint16_t GetTowerOfDespairSelectFloor(CUser* user) {
    if (!user) {
        return 0u;
    }
    TOD_UserState* const state = static_cast<TOD_UserState*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
    if (!state) {
        return 0u;
    }

    // Selection needs the next layer, not the layer in the clear-reward packet.
    return static_cast<uint16_t>(state->getTodayEnterLayer().GetLayer());
}

struct PartyTeleportEffectData {
    unsigned char village;
    int area;
    std::vector<unsigned short> unique_ids;
};

#pragma pack(push, 1)
struct VillageAttackedFinishPayload {
    uint16_t reserved;
    int32_t remaining_seconds;
    int32_t reward_item_id;
};
#pragma pack(pop)

static_assert(sizeof(VillageAttackedFinishPayload) == 0x0au,
              "village-attacked notification payload ABI");

}  // namespace

// IDA_EA: 0x8145780
// IDA_NAME: _ZN6CParty10getManagerEv
// DEMANGLED: CParty::getManager(void)
CUser* CParty::getManager() {
    return manager_;
}

// IDA_EA: 0x814581C
// IDA_NAME: _ZN6CParty12GetEPLPStateEv
// DEMANGLED: CParty::GetEPLPState(void)
int CParty::GetEPLPState() {
    return eplp_state_;
}

// IDA_EA: 0x84699DA
// IDA_NAME: _ZN6CParty18GetSelectedEPLPCmdEv
// DEMANGLED: CParty::GetSelectedEPLPCmd(void)
int CParty::GetSelectedEPLPCmd() {
    return selected_eplp_command_;
}

// CDungeonEntranceLog members migrated to
// old_game/dungeon_pvp/dungeon_entrance_log_runtime.cpp.  This CParty unit
// intentionally retains no duplicate constructor, destructor, or decrement
// provider.

// IDA_EA: 0x814578C
// IDA_NAME: _ZN6CParty12SetEPLPStateEc
// DEMANGLED: CParty::SetEPLPState(char)
char CParty::SetEPLPState(char state) {
    char result = static_cast<char>(reinterpret_cast<uintptr_t>(this));
    eplp_state_ = static_cast<unsigned char>(state);
    if (state == 2) {
        CDungeon* const dungeon = battle_field_.get_dungeon();
        if (dungeon) {
            for (int seat = 0;; ++seat) {
                result = static_cast<char>(seat <= 3);
                if (seat > 3) {
                    break;
                }
                if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
                    GetInstanceDungeonEntranceLog()->DecrementDungeonEntrance(
                        reinterpret_cast<const CDungeon*>(dungeon)->get_index(), false);
                }
            }
        }
    }
    return result;
}

// IDA_EA: 0x8145828
// IDA_NAME: _ZN6CParty18SetSelectedEPLPCmdEc
// DEMANGLED: CParty::SetSelectedEPLPCmd(char)
CParty* CParty::SetSelectedEPLPCmd(char command) {
    selected_eplp_command_ = static_cast<unsigned char>(command);
    return this;
}

// IDA_EA: 0x814575A
// IDA_NAME: _ZN6CParty16CItemRoutingData15GetRoutingStateEv
// DEMANGLED: CParty::CItemRoutingData::GetRoutingState(void)
int CParty::CItemRoutingData::GetRoutingState() {
    return routing_state;
}

// IDA_EA: 0x8145840
// IDA_NAME: _ZN6CParty14IsRoutingStateEv
// DEMANGLED: CParty::IsRoutingState(void)
int CParty::IsRoutingState() {
    return static_cast<unsigned char>(routing_data_.GetRoutingState()) != 0;
}

// IDA_EA: 0x82A5A34
// IDA_NAME: _ZN6CParty15SetAssaultStateEc
// DEMANGLED: CParty::SetAssaultState(char)
CParty* CParty::SetAssaultState(char state) {
    assault_state_ = static_cast<unsigned char>(state);
    return this;
}

// IDA_EA: 0x85B9F50
// IDA_NAME: _ZN6CParty17IsReturnUserPartyEv
// DEMANGLED: CParty::IsReturnUserParty(void)
int CParty::IsReturnUserParty() {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) == 1u) {
            CUser* const user = members_[seat].user;
            if (user && static_cast<unsigned char>(user->IsReturnUser())) {
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x82A52FA
// IDA_NAME: _ZN6CParty16CItemRoutingData5ResetEv
// DEMANGLED: CParty::CItemRoutingData::Reset(void)
bool CParty::CItemRoutingData::Reset() {
    routing_state = 0;
    routing_inven_item.reset();
    routing_item_unique_id = 0;
    bool result = false;
    for (int seat = 0;; ++seat) {
        result = seat <= 3;
        if (seat > 3) {
            break;
        }
        member_routing_state[seat] = 0;
    }
    return result;
}

// IDA_EA: 0x82A52D2
// IDA_NAME: _ZN6CParty16CItemRoutingDataC2Ev
// DEMANGLED: CParty::CItemRoutingData::CItemRoutingData(void)
CParty::CItemRoutingData::CItemRoutingData()
    : routing_inven_item() {
    Reset();
}

// IDA_EA: 0x82A52F4
// IDA_NAME: _ZN6CParty16CItemRoutingDataD2Ev
// DEMANGLED: CParty::CItemRoutingData::~CItemRoutingData()
CParty::CItemRoutingData::~CItemRoutingData() {}

// IDA_EA: 0x85BF694
// IDA_NAME: _ZN6CParty16CItemRoutingData21GetMemberRoutingStateEi
// DEMANGLED: CParty::CItemRoutingData::GetMemberRoutingState(int)
int CParty::CItemRoutingData::GetMemberRoutingState(int seat) {
    if (static_cast<unsigned int>(seat) >= kPartyMaxMemberCount) {
        return 0;
    }
    return member_routing_state[seat];
}

// IDA_EA: 0x85BF6B8
// IDA_NAME: _ZN6CParty16CItemRoutingData21SetMemberRoutingStateEic
// DEMANGLED: CParty::CItemRoutingData::SetMemberRoutingState(int,char)
unsigned int CParty::CItemRoutingData::SetMemberRoutingState(int seat,
                                                              char state) {
    unsigned int result = static_cast<unsigned int>(static_cast<int>(state));
    if (seat >= 0 && seat < kPartyMaxMemberCount) {
        result = static_cast<unsigned int>(seat);
        member_routing_state[seat] = static_cast<unsigned char>(state);
    }
    return result;
}

// IDA_EA: 0x860B53A
// IDA_NAME: _ZN6CParty16CItemRoutingData19GetRoutingItemIndexEv
// DEMANGLED: CParty::CItemRoutingData::GetRoutingItemIndex(void)
int CParty::CItemRoutingData::GetRoutingItemIndex() {
    return routing_inven_item.item_id_;
}

// IDA_EA: 0x860B546
// IDA_NAME: _ZN6CParty16CItemRoutingData19GetRoutingInvenItemEv
// DEMANGLED: CParty::CItemRoutingData::GetRoutingInvenItem(void)
Inven_Item* CParty::CItemRoutingData::GetRoutingInvenItem() {
    return &routing_inven_item;
}

// IDA_EA: 0x860B552
// IDA_NAME: _ZN6CParty16CItemRoutingData22GetRoutingItemUniqueIDEv
// DEMANGLED: CParty::CItemRoutingData::GetRoutingItemUniqueID(void)
int CParty::CItemRoutingData::GetRoutingItemUniqueID() {
    return routing_item_unique_id;
}

// IDA_EA: 0x860B55E
// IDA_NAME: _ZN6CParty16CItemRoutingData15SetRoutingStateEc
// DEMANGLED: CParty::CItemRoutingData::SetRoutingState(char)
CParty::CItemRoutingData* CParty::CItemRoutingData::SetRoutingState(
    char state) {
    routing_state = static_cast<int>(state);
    return this;
}

// IDA_EA: 0x85A7800
// IDA_NAME: _ZN6CParty7useItemEP5CUserRK10Inven_Item
// DEMANGLED: CParty::useItem(CUser *,Inven_Item const&)
MapInfo* CParty::useItem(CUser* user, const Inven_Item& item) {
    return battle_field_.onUseItem(user, item);
}

// IDA_EA: 0x860B576
// IDA_NAME: _ZN6CParty16CItemRoutingData19SetRoutingItemIndexER10Inven_Itemj
// DEMANGLED: CParty::CItemRoutingData::SetRoutingItemIndex(Inven_Item &,unsigned int)
CParty::CItemRoutingData* CParty::CItemRoutingData::SetRoutingItemIndex(
    Inven_Item& item,
    unsigned int unique_id) {
    routing_inven_item.setCopy(item);
    routing_item_unique_id = static_cast<int>(unique_id);
    return this;
}

// IDA_EA: 0x85BF82A
// IDA_NAME: _ZN6CParty13SetEndRoutingEv
// DEMANGLED: CParty::SetEndRouting(void)
bool CParty::SetEndRouting() {
    return routing_data_.Reset();
}


// IDA_EA: 0x82A59CA
// IDA_NAME: _ZN6CParty6SetIDXEi
// DEMANGLED: CParty::SetIDX(int)
CParty* CParty::SetIDX(int index) {
    party_index_ = index;
    return init();
}

// IDA_EA: 0x85BA574
// IDA_NAME: _ZN6CParty24GetFirstValidMemberStateEv
// DEMANGLED: CParty::GetFirstValidMemberState(void)
int CParty::GetFirstValidMemberState() {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
            CUser* user = members_[seat].user;
            return user->get_state();
        }
    }
    return 0;
}

// IDA_EA: 0x860B616
// IDA_NAME: _ZN6CParty22GetRoutingItemUniqueIDEv
// DEMANGLED: CParty::GetRoutingItemUniqueID(void)
int CParty::GetRoutingItemUniqueID() {
    return routing_data_.GetRoutingItemUniqueID();
}

// IDA_EA: 0x85BF718
// IDA_NAME: _ZN6CParty25ResetPremiumGoldCardPartyEv
// DEMANGLED: CParty::ResetPremiumGoldCardParty(void)
CParty* CParty::ResetPremiumGoldCardParty() {
    premium_gold_card_party_ = 0;
    return this;
}

// IDA_EA: 0x85BF748
// IDA_NAME: _ZN6CParty31ResetPremiumGoldCardDefaultItemEv
// DEMANGLED: CParty::ResetPremiumGoldCardDefaultItem(void)
CParty* CParty::ResetPremiumGoldCardDefaultItem() {
    premium_gold_card_default_item_ = 0;
    return this;
}

// IDA_EA: 0x859B3E4
// IDA_NAME: _ZN6CParty21init_quick_party_dataEv
// DEMANGLED: CParty::init_quick_party_data(void)
CParty* CParty::init_quick_party_data() {
    quick_party_index_ = -1;
    random_buff_type_ = 11;
    return this;
}

// IDA_EA: 0x859B2B6
// IDA_NAME: _ZN6CParty9join_userEP5CUser
// DEMANGLED: CParty::join_user(CUser *)
int CParty::join_user(CUser* user) {
    if (!user) {
        LogManager::logFormat(1, "party.cpp", "int CParty::join_user(CUser*)",
                              0x419, "Null user");
        return -1;
    }
    if (state_ == 2) {
        return -2;
    }
    if (get_member_count() >= user_max_) {
        return -1;
    }
    for (int seat = 0; seat <= 3; ++seat) {
        cMember& member = members_[seat];
        if (member.user) {
            continue;
        }
        member.user = user;
        member.user_uid = user->GetUID();
        user->SetPartyIndex(static_cast<short>(party_index_));
        user->resetStraightVictories();
        return 1;
    }
    return 0;
}

// IDA_EA: 0x859B52C
// IDA_NAME: _ZN6CParty28delete_party_for_quick_partyEP5CUserRSt4listIS1_SaIS1_EE
// DEMANGLED: CParty::delete_party_for_quick_party(
//     CUser *,std::list<CUser *,std::allocator<CUser *> > &)
int CParty::delete_party_for_quick_party(CUser* user,
                                         std::list<CUser*>& detached_users) {
    if (!user) {
        LogManager::logFormat(
            1, "party.cpp",
            "int CParty::delete_party_for_quick_party(CUser*, std::list<CUser*, std::allocator<CUser*> >&)",
            0x465, "QUICK_PARTY_LOG : NO EXIST USER!!");
        return 1;
    }

    if (get_member_count() == 1) {
        cMyTrace trace(
            "int CParty::delete_party_for_quick_party(CUser*, std::list<CUser*, std::allocator<CUser*> >&)",
            1132, 0);
        trace("<IN_DUNGEON_NULL_PARTY> m_id(%s), user_state(%d), PartyIndex(%d)",
              NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0), user->get_state(),
              user->GetPartyIndex());
        user->SetPartyIndex(-1);
        detached_users.push_back(user);
        destroy();
        return 0;
    }

    if (manager_ == user) {
        change_manager_for_quick_party();
        detached_users.push_back(user);
        return 0;
    }
    for (int seat = 0; seat <= 3; ++seat) {
        cMember& member = members_[seat];
        if (_checkValidUser(seat) == 1 && member.user == user) {
            member.user = 0;
            member.user_uid = 0;
            user->SetPartyIndex(-1);
            detached_users.push_back(user);
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x859B404
// IDA_NAME: _ZN6CParty30change_manager_for_quick_partyEv
// DEMANGLED: CParty::change_manager_for_quick_party(void)
int CParty::change_manager_for_quick_party() {
    for (int seat = 0; seat <= 3; ++seat) {
        cMember& member = members_[seat];
        if (member.user == manager_) {
            manager_->SetPartyIndex(-1);
            member.user = 0;
            member.user_uid = 0;
            break;
        }
    }
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1) {
            manager_ = members_[seat].user;
            set_host();
            send_host_info();
            return 1;
        }
    }
    LogManager::logFormat(1, "party.cpp",
                          "bool CParty::change_manager_for_quick_party()",
                          0x45d, "there is no manager(party empty)");
    return 0;
}

// IDA_EA: 0x8145764
// IDA_NAME: _ZN6CParty8get_userEi
// DEMANGLED: CParty::get_user(int)
CUser* CParty::get_user(int seat) {
    return members_[seat].user;
}

// IDA_EA: 0x822D91C
// IDA_NAME: _ZN6CParty9GetMemberEv
// DEMANGLED: CParty::GetMember(void)
CParty::cMember* CParty::GetMember() {
    return members_;
}

// IDA_EA: 0x822D90E
// IDA_NAME: _ZN6CParty16GetPartyTelePortEv
// DEMANGLED: CParty::GetPartyTelePort(void)
CPartyTelePort* CParty::GetPartyTelePort() {
    return &party_teleport_;
}

// IDA_EA: 0x85BB832
// IDA_NAME: _ZN14CPartyTelePortC2Ev
// DEMANGLED: CPartyTelePort::CPartyTelePort(void)
CPartyTelePort::CPartyTelePort() {
    state_ = 0;
    village_ = 0;
    area_index_ = 0;
    pos_x_ = 0;
    pos_y_ = 0;
    direction_ = 0;
}

// IDA_EA: 0x85BB848
// IDA_NAME: _ZN14CPartyTelePortD2Ev
// DEMANGLED: CPartyTelePort::~CPartyTelePort()
CPartyTelePort::~CPartyTelePort() {}

// IDA_EA: 0x822D7EE
// IDA_NAME: _ZN14CPartyTelePort18get_teleport_stateEv
// DEMANGLED: CPartyTelePort::get_teleport_state(void)
int CPartyTelePort::get_teleport_state() {
    return static_cast<unsigned char>(state_);
}

// IDA_EA: 0x822D7FA
// IDA_NAME: _ZN14CPartyTelePort18set_teleport_stateEc
// DEMANGLED: CPartyTelePort::set_teleport_state(char)
CPartyTelePort* CPartyTelePort::set_teleport_state(char state) {
    state_ = state;
    return this;
}

// IDA_EA: 0x85BB84E
// IDA_NAME: _ZN14CPartyTelePort4initEP6CParty
// DEMANGLED: CPartyTelePort::init(CParty *)
CPartyTelePort* CPartyTelePort::init(CParty* party) {
    party_ = party;
    return this;
}

// IDA_EA: 0x85BB91C
// IDA_NAME: _ZN14CPartyTelePort25set_teleport_member_stateEic
// DEMANGLED: CPartyTelePort::set_teleport_member_state(int,char)
CParty* CPartyTelePort::set_teleport_member_state(int seat, char state) {
    CParty* result = party_;
    if (party_ && seat >= 0 && seat < kPartyMaxMemberCount) {
        party_->members_[seat].teleport_state =
            static_cast<unsigned char>(state);
        result = reinterpret_cast<CParty*>(
            static_cast<uintptr_t>(static_cast<unsigned char>(state)));
    }
    return result;
}

// IDA_EA: 0x85BB85C
// IDA_NAME: _ZN14CPartyTelePort17set_teleport_dataEhhsshc
// DEMANGLED: CPartyTelePort::set_teleport_data(unsigned char,unsigned char,short,short,unsigned char,char)
bool CPartyTelePort::set_teleport_data(unsigned char village,
                                       unsigned char area,
                                       short pos_x,
                                       short pos_y,
                                       unsigned char direction,
                                       char state) {
    state_ = 0;
    village_ = 0;
    area_index_ = 0;
    pos_x_ = 0;
    pos_y_ = 0;
    direction_ = 0;

    village_ = village;
    area_index_ = area;
    pos_x_ = static_cast<uint16_t>(pos_x);
    pos_y_ = static_cast<uint16_t>(pos_y);
    direction_ = direction;
    state_ = state;

    bool result = false;
    for (int seat = 0;; ++seat) {
        result = seat <= 3;
        if (seat > 3) {
            break;
        }
        set_teleport_member_state(seat, 2);
    }
    return result;
}

// IDA_EA: 0x85BC044
// IDA_NAME: _ZN14CPartyTelePort19reset_teleport_dataEv
// DEMANGLED: CPartyTelePort::reset_teleport_data(void)
CPartyTelePort* CPartyTelePort::reset_teleport_data() {
    for (int seat = 0; seat <= 3; ++seat) {
        set_teleport_member_state(seat, static_cast<char>(0xff));
    }
    state_ = 0;
    village_ = 0;
    area_index_ = 0;
    pos_x_ = 0;
    pos_y_ = 0;
    direction_ = 0;
    return set_teleport_state(static_cast<char>(-1));
}

// IDA_EA: 0x859B142
// IDA_NAME: _ZN6CParty15set_single_playEP5CUser
// DEMANGLED: CParty::set_single_play(CUser *)
int CParty::set_single_play(CUser* user) {
    members_[0].user = user;
    members_[0].user_uid = user->GetUID();
    manager_ = user;
    single_play_ = 1;
    member_count_ = 1;
    auto_created_ = 1;
    state_ = 1;
    user->SetPartyIndex(static_cast<short>(party_index_));
    GetPartyTelePort()->reset_teleport_data();
    return 1;
}

// IDA_EA: 0x85B4D12
// IDA_NAME: _ZN6CParty15_checkValidUserEi
// DEMANGLED: CParty::_checkValidUser(int)
int CParty::_checkValidUser(int seat) {
    CUser* user = members_[seat].user;
    if (user) {
        if (user->GetUID() == members_[seat].user_uid &&
            user->getCurCharacR()) {
            return 1;
        }

        CUser* manager = manager_;
        cMyTrace trace_header("bool CParty::_checkValidUser(int)", 14287, 5);
        trace_header("[PARTYPOOL] m_pManager : %x, dungeon_idx : %u, "
                     "m_iIndex : %u, cState : %d",
                     reinterpret_cast<unsigned int>(manager),
                     0u,
                     0u,
                     0);

        for (int index = 0; index < kPartyMaxMemberCount; ++index) {
            CUser* member = members_[index].user;
            if (member) {
                const int charac_no =
                    member->getCurCharacNo();
                const int uid = member->GetUID();
                const int state = member->get_state();
                cMyTrace trace_user("bool CParty::_checkValidUser(int)",
                                    14298,
                                    5);
                trace_user("User[%d] %x, ch_state %d, uid : %d / %d, "
                           "charac_no : %u",
                           index,
                           reinterpret_cast<unsigned int>(member),
                           state,
                           uid,
                           members_[index].user_uid,
                           static_cast<unsigned int>(charac_no));
            } else {
                cMyTrace trace_null("bool CParty::_checkValidUser(int)",
                                    14309,
                                    5);
                trace_null("User[%d] NULL uid : %d",
                           index,
                           members_[index].user_uid);
            }
        }

        const int uid = user->GetUID();
        const char* account_id =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace_mismatch("bool CParty::_checkValidUser(int)",
                                14317,
                                5);
        trace_mismatch("User missmatched in party (m_id: %s)(uid: %d)",
                       account_id,
                       uid);
        user->DisConnSig(static_cast<DISCONN_SIG>(23), false, 0);
        members_[seat].Init();
    }
    return 0;
}

// IDA_EA: 0x8145868
// IDA_NAME: _ZN6CParty14checkValidUserEi
// DEMANGLED: CParty::checkValidUser(int)
int CParty::checkValidUser(int seat) {
    return _checkValidUser(seat);
}

// IDA_EA: 0x860B62A
// IDA_NAME: _ZN6CParty21DeleteDungeonDropItemEi
// DEMANGLED: CParty::DeleteDungeonDropItem(int)
int CParty::DeleteDungeonDropItem(int unique_id) {
    return battle_field_.pickup_item(unique_id);
}

// IDA_EA: 0x8145882
// IDA_NAME: _ZN6CParty14getMemberNamesEPc
// DEMANGLED: CParty::getMemberNames(char *)
char* CParty::getMemberNames(char* buffer) {
    return _getMemberNames(buffer);
}

// IDA_EA: 0x822D812
// IDA_NAME: _ZN6CParty12IsSinglePlayEv
// DEMANGLED: CParty::IsSinglePlay(void)
int CParty::IsSinglePlay() {
    return single_play_;
}

// IDA_EA: 0x822D83A
// IDA_NAME: _ZN6CParty9get_stateEv
// DEMANGLED: CParty::get_state(void)
int CParty::get_state() {
    return state_;
}

// IDA_EA: 0x822D846
// IDA_NAME: _ZN6CParty13GetPartyIndexEv
// DEMANGLED: CParty::GetPartyIndex(void)
int CParty::GetPartyIndex() {
    return party_index_;
}

// IDA_EA: 0x82A59E6
// IDA_NAME: _ZN6CParty8getTitleEv
// DEMANGLED: CParty::getTitle(void)
char* CParty::getTitle() {
    return title_;
}

// IDA_EA: 0x82A59F4
// IDA_NAME: _ZN6CParty13getTitleIndexEv
// DEMANGLED: CParty::getTitleIndex(void)
int CParty::getTitleIndex() {
    return title_index_;
}

// IDA_EA: 0x82A5A04
// IDA_NAME: _ZN6CParty10getUserMaxEv
// DEMANGLED: CParty::getUserMax(void)
int CParty::getUserMax() {
    return user_max_;
}

// IDA_EA: 0x82A5A14
// IDA_NAME: _ZNK6CParty12getDungIndexEv
// DEMANGLED: CParty::getDungIndex(void)const
int CParty::getDungIndex() const {
    return dungeon_index_;
}

// IDA_EA: 0x82A5A24
// IDA_NAME: _ZNK6CParty12getDungDiffiEv
// DEMANGLED: CParty::getDungDiffi(void)const
int CParty::getDungDiffi() const {
    return dungeon_difficulty_;
}

// IDA_EA: 0x822D928
// IDA_NAME: _ZN6CParty21get_quick_party_indexEv
// DEMANGLED: CParty::get_quick_party_index(void)
int CParty::get_quick_party_index() {
    return quick_party_index_;
}

// IDA_EA: 0x822D936
// IDA_NAME: _ZN6CParty15set_quick_partyEb
// DEMANGLED: CParty::set_quick_party(bool)
CParty* CParty::set_quick_party(bool quick_party) {
    quick_party_ = static_cast<unsigned char>(quick_party);
    return this;
}

// IDA_EA: 0x822D952
// IDA_NAME: _ZN6CParty14is_quick_partyEv
// DEMANGLED: CParty::is_quick_party(void)
int CParty::is_quick_party() {
    return quick_party_;
}

// IDA_EA: 0x826B978
// IDA_NAME: _ZN6CParty9set_stateEc
// DEMANGLED: CParty::set_state(char)
CParty* CParty::set_state(char state) {
    state_ = static_cast<unsigned char>(state);
    return this;
}

// IDA_EA: 0x826B9AC
// IDA_NAME: _ZN6CParty21set_quick_party_indexEi
// DEMANGLED: CParty::set_quick_party_index(int)
CParty* CParty::set_quick_party_index(int quick_party_index) {
    quick_party_index_ = quick_party_index;
    return this;
}

// IDA_EA: 0x859AC7C
// IDA_NAME: _ZNK6CParty15GetMemberSlotNoEPK5CUser
// DEMANGLED: CParty::GetMemberSlotNo(CUser const*)const
int CParty::GetMemberSlotNo(const CUser* user) const {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (members_[seat].user == user) {
            return seat;
        }
    }
    return -1;
}

// IDA_EA: 0x859ACC2
// IDA_NAME: _ZN6CParty15GetMemberSlotNoEj
// DEMANGLED: CParty::GetMemberSlotNo(unsigned int)
CUser* CParty::GetMemberSlotNo(unsigned int charac_no) {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                1u &&
            static_cast<unsigned int>(
                get_user(seat)->get_charac_no(-1)) ==
                charac_no) {
            return get_user(seat);
        }
    }
    return 0;
}

// IDA_EA: 0x85A277A
// IDA_NAME: _ZN6CParty16get_party_seatnoEP5CUser
// DEMANGLED: CParty::get_party_seatno(CUser *)
int CParty::get_party_seatno(CUser* user) {
    if (!user) {
        return 0;
    }

    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                1 &&
            members_[seat].user == user) {
            return seat;
        }
    }
    return 0;
}

// IDA_EA: 0x85B9E20
// IDA_NAME: _ZN6CParty15GetUserPositionEP5CUser
// DEMANGLED: CParty::GetUserPosition(CUser *)
int CParty::GetUserPosition(CUser* user) {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                1 &&
            members_[seat].user == user) {
            return seat;
        }
    }
    return -1;
}

// IDA_EA: 0x85B9E80
// IDA_NAME: _ZN6CParty17GetUserSecretShopEP5CUser
// DEMANGLED: CParty::GetUserSecretShop(CUser *)
secretshop::SECRET_SHOP_INFO* CParty::GetUserSecretShop(CUser* user) {
    const unsigned int position =
        static_cast<unsigned int>(GetUserPosition(user));
    if (position >= 4u) {
        return 0;
    }
    return &GetSecretShopData()->infos[position];
}

// IDA_EA: 0x85B9ECC
// IDA_NAME: _ZN6CParty22SendSecretShopItemListEP5CUser
// DEMANGLED: CParty::SendSecretShopItemList(CUser *)
void CParty::SendSecretShopItemList(CUser* user) {
    const unsigned int position =
        static_cast<unsigned int>(GetUserPosition(user));
    if (position < 4u) {
        GlobalData::s_secret_shop->SendSecretShopItemList(
            user,
            GetSecretShopData()->infos[position].retailer);
    }
}

// IDA_EA: 0x8694A1A
// IDA_NAME: _ZN6CParty17GetSecretShopDataEv
// DEMANGLED: CParty::GetSecretShopData(void)
secretshop::SECRET_SHOP_DATA* CParty::GetSecretShopData() {
    return &secret_shop_data_;
}

// IDA_EA: 0x859A16A
// IDA_NAME: _ZN6CParty16get_member_countEv
// DEMANGLED: CParty::get_member_count(void)
int CParty::get_member_count() {
    int count = 0;
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat))) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x859D14E
// IDA_NAME: _ZN6CParty13send_to_partyER11PacketGuard
// DEMANGLED: CParty::send_to_party(PacketGuard &)
bool CParty::send_to_party(PacketGuard& packet) {
    const zombie_map_runtime::HookResult zombie_route =
        zombie_map_runtime::RouteAdapterPacket(this, packet);
    if (zombie_route != zombie_map_runtime::kHookNotManaged) {
        // CParty's original return value is false after its complete loop.
        // A rejected managed route is consumed fail-closed as well.
        return false;
    }
    for (int seat = 0;; ++seat) {
        if (seat > 3) {
            return false;
        }
        if (static_cast<unsigned char>(_checkValidUser(seat))) {
            (get_user(seat))->Send(packet);
        }
    }
}

// IDA_EA: 0x859D1AA
// IDA_NAME: _ZN6CParty23send_to_party_cmd_errorE14ENUM_CMDPACKETh
// DEMANGLED: CParty::send_to_party_cmd_error(ENUM_CMDPACKET,unsigned char)
void CParty::send_to_party_cmd_error(ENUM_CMDPACKET command,
                                     unsigned char error) {
    PacketGuard packet;
    packet.put_header(1, command);
    packet.put_byte(0);
    packet.put_byte(error);
    packet.finalize(true);
    send_to_party(packet);
}

// IDA_EA: 0x85B4AE2
// IDA_NAME: _ZN6CParty18_getMemberLevelGapEv
// DEMANGLED: CParty::_getMemberLevelGap(void)
int CParty::_getMemberLevelGap() {
    if (member_level_gap_ == -1) {
        int min_level = 99;
        int max_level = 0;
        for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
            if (static_cast<unsigned char>(
                    _checkValidUser(seat)) == 1u) {
                CUser* member = get_user(seat);
                const int level = member->get_charac_level();
                min_level = std::min(min_level, level);
                max_level = std::max(max_level, level);
            }
        }
        member_level_gap_ = std::abs(max_level - min_level);
    }
    return member_level_gap_;
}

// IDA_EA: 0x85B4BDE
// IDA_NAME: _ZN6CParty15_getMemberNamesEPc
// DEMANGLED: CParty::_getMemberNames(char *)
char* CParty::_getMemberNames(char* buffer) {
    const char* manager_name = "NoMember";
    CUser* manager = manager_;
    if (manager) {
        manager_name = manager->getCurCharacName();
    }

    char member_name[34];
    std::sprintf(buffer, "\"%s\",", manager_name);
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        const char* name = "N/A";
        if (static_cast<unsigned char>(_checkValidUser(seat))) {
            name = (get_user(seat))->getCurCharacName();
        }

        if (seat == 3) {
            std::sprintf(member_name, "\"%s\"", name);
        } else {
            std::sprintf(member_name, "\"%s\",", name);
        }
        std::strncat(buffer, member_name, 0xffu);
    }
    buffer[255] = 0;
    return buffer;
}

// IDA_EA: 0x85B624E
// IDA_NAME: _ZN6CParty16IsExistCharacterEi
// DEMANGLED: CParty::IsExistCharacter(int)
int CParty::IsExistCharacter(int charac_no) {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                1u &&
            (get_user(seat))->getCurCharacNo() ==
                charac_no) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85B62C0
// IDA_NAME: _ZN6CParty12GetPartyTypeEv
// DEMANGLED: CParty::GetPartyType(void)
int CParty::GetPartyType() {
    int guild_key = 0;
    char same_guild = 0;
    int connected_count = 0;
    int member_count = 0;

    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
            1u) {
            CUser* member = get_user(seat);
            const int member_guild_key =
                member->get_charac_guildkey();
            if (member_guild_key) {
                if (guild_key) {
                    if (member_guild_key != guild_key) {
                        same_guild = 0;
                    }
                } else {
                    guild_key = member_guild_key;
                    same_guild = 1;
                }
            }
            const int upper_member = member->get_connect_upper_member();
            if (static_cast<unsigned char>(
                    IsExistCharacter(upper_member))) {
                ++connected_count;
            }
            ++member_count;
        }
    }

    if (same_guild) {
        return 0;
    }
    if (connected_count <= 0) {
        return 2;
    }
    if (member_count - 1 == connected_count) {
        return 1;
    }
    return 4;
}

// IDA_EA: 0x85B6410
// IDA_NAME: _ZN6CParty16IsExistInvisibleEv
// DEMANGLED: CParty::IsExistInvisible(void)
int CParty::IsExistInvisible() {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) &&
            static_cast<unsigned char>(get_user(seat)->IsCurCharacVisible()) !=
                1u) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85BC252
// IDA_NAME: _ZN14CPartyTelePort25get_teleport_member_stateEi
// DEMANGLED: CPartyTelePort::get_teleport_member_state(int)
int CPartyTelePort::get_teleport_member_state(int seat) {
    if (!party_ || seat < 0 || seat >= kPartyMaxMemberCount) {
        return -1;
    }
    return party_->members_[seat].teleport_state;
}

// IDA_EA: 0x85BF658
// IDA_NAME: _ZN14CPartyTelePort20get_teleport_villageEv
// DEMANGLED: CPartyTelePort::get_teleport_village(void)
int CPartyTelePort::get_teleport_village() {
    return village_;
}

// IDA_EA: 0x85BF664
// IDA_NAME: _ZN14CPartyTelePort23get_teleport_area_indexEv
// DEMANGLED: CPartyTelePort::get_teleport_area_index(void)
int CPartyTelePort::get_teleport_area_index() {
    return area_index_;
}

// IDA_EA: 0x85BF670
// IDA_NAME: _ZN14CPartyTelePort17get_teleport_posxEv
// DEMANGLED: CPartyTelePort::get_teleport_posx(void)
int CPartyTelePort::get_teleport_posx() {
    return pos_x_;
}

// IDA_EA: 0x85BF67C
// IDA_NAME: _ZN14CPartyTelePort17get_teleport_posyEv
// DEMANGLED: CPartyTelePort::get_teleport_posy(void)
int CPartyTelePort::get_teleport_posy() {
    return pos_y_;
}

// IDA_EA: 0x85BF688
// IDA_NAME: _ZN14CPartyTelePort22get_teleport_directionEv
// DEMANGLED: CPartyTelePort::get_teleport_direction(void)
int CPartyTelePort::get_teleport_direction() {
    return direction_;
}

// IDA_EA: 0x85BB96E
// IDA_NAME: _ZN14CPartyTelePort20send_teleport_statusEh
// DEMANGLED: CPartyTelePort::send_teleport_status(unsigned char)
void CPartyTelePort::send_teleport_status(unsigned char error) {
    CParty* party = party_;
    if (!party) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 377);
    packet.put_byte(get_teleport_village());
    packet.put_byte(get_teleport_state());
    for (int seat = 0; seat <= 3; ++seat) {
        int unique_id = -1;
        unsigned char member_state = 0;
        if (static_cast<unsigned char>(party->checkValidUser(seat)) == 1u) {
            CUser* user = party->get_user(seat);
            if (user) {
                unique_id = user->get_unique_id();
                member_state = static_cast<unsigned char>(
                    get_teleport_member_state(seat));
            }
        }
        packet.put_short(unique_id);
        packet.put_byte(member_state);
    }
    packet.put_byte(error);
    packet.finalize(true);
    party->send_to_party(packet);
}

// IDA_EA: 0x85BBB08
// IDA_NAME: _ZN14CPartyTelePort24check_cur_teleport_stateEv
// DEMANGLED: CPartyTelePort::check_cur_teleport_state(void)
int CPartyTelePort::check_cur_teleport_state() {
    CParty* party = party_;
    if (!party) {
        return 0;
    }

    unsigned char all_ready = 1;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) ==
                1u &&
            static_cast<unsigned char>(
                get_teleport_member_state(seat)) != 4u &&
            static_cast<unsigned char>(
                get_teleport_member_state(seat)) != 1u) {
            all_ready = 0;
        }
    }
    if (all_ready) {
        set_teleport_state(1);
    }
    return all_ready;
}

// IDA_EA: 0x85BBBB8
// IDA_NAME: _ZN14CPartyTelePort16process_teleportEv
// DEMANGLED: CPartyTelePort::process_teleport(void)
int CPartyTelePort::process_teleport() {
    CParty* party = party_;
    if (!party) {
        return 0;
    }

    const unsigned char ready =
        static_cast<unsigned char>(check_cur_teleport_state());
    int move_error = 0;
    int aura_option_value = 0;

    if (ready) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(party->checkValidUser(seat)) ==
                    1u &&
                static_cast<unsigned char>(
                    get_teleport_member_state(seat)) != 4u) {
                CUser* user = party->get_user(seat);
                if (user) {
                    move_error = G_GameWorld()->check_move_area(
                        user,
                        get_teleport_village(),
                        get_teleport_area_index(),
                        static_cast<short>(
                            get_teleport_posx()),
                        static_cast<short>(
                            get_teleport_posy()),
                        get_teleport_direction(),
                        true,
                        0);
                    if (party->getManager() == user) {
                        move_error =
                            user->is_equip_aura_avatar(0,
                                                       aura_option_value);
                    }
                    if (move_error > 0) {
                        set_teleport_member_state(seat, 0);
                        set_teleport_state(2);
                        break;
                    }
                }
            }
        }

        if (static_cast<unsigned char>(
                get_teleport_state()) == 1u) {
            send_teleport_effect_at_different_place(1);
            for (int seat = 0; seat <= 3; ++seat) {
                if (static_cast<unsigned char>(
                        party->checkValidUser(seat)) == 1u &&
                    static_cast<unsigned char>(
                        get_teleport_member_state(seat)) != 4u) {
                    CUser* user = party->get_user(seat);
                    if (user) {
                        user->set_charac_visible_teleport_(true);
                        if (static_cast<unsigned char>(G_GameWorld()->move_area(
                                user,
                                get_teleport_village(),
                                get_teleport_area_index(),
                                get_teleport_posx(),
                                get_teleport_posy(),
                                get_teleport_direction(),
                                true,
                                1,
                                0,
                                0)) != 1u) {
                            cMyTrace trace(
                                "bool CPartyTelePort::process_teleport()",
                                17401,
                                5);
                            trace("party_teleport_error:(char_no:%d)",
                                  user->getCurCharacNo());
                        }
                        user->set_charac_visible_teleport_(false);
                    }
                }
            }
            send_teleport_effect_at_equal_place(2);
        } else {
            send_teleport_status(static_cast<unsigned char>(move_error));
        }
    }

    if (static_cast<unsigned char>(
            get_teleport_state()) == 1u) {
        reset_teleport_data();
        CUser* manager = party->getManager();
        if (manager) {
            CSystemTime* system_time = GlobalData::system_time();
            const int expires_at =
                aura_option_value +
                (system_time ? system_time->getCurSec() : 0);
            manager->set_aura_avatar_option_value(0, expires_at);
            const int stored =
                manager->get_aura_avatar_option_value(0);
            manager->UpdateAuraAvatarOption(0, stored);
            cMyTrace trace("bool CPartyTelePort::process_teleport()",
                           17433,
                           0);
            trace("party_teleport_success:(char_no:%d)",
                  manager->getCurCharacNo());
        }
    }
    return ready;
}

// IDA_EA: 0x85BC0A0
// IDA_NAME: _ZN14CPartyTelePort30process_leave_user_at_teleportEP5CUser
// DEMANGLED: CPartyTelePort::process_leave_user_at_teleport(CUser *)
char CPartyTelePort::process_leave_user_at_teleport(CUser* user) {
    CParty* party = party_;
    if (!party) {
        return 0;
    }
    const unsigned char teleport_state =
        static_cast<unsigned char>(get_teleport_state());
    if (teleport_state >> 7) {
        return 1;
    }
    if (static_cast<signed char>(teleport_state) > 3) {
        return 1;
    }
    if (user->get_state() != 3) {
        return 1;
    }
    if (teleport_state == 0xffu) {
        return 0;
    }

    const unsigned int seat =
        static_cast<unsigned int>(party->GetMemberSlotNo(user));
    if (seat >= kPartyMaxMemberCount) {
        return static_cast<char>(seat);
    }
    if (static_cast<unsigned char>(party->checkValidUser(seat)) != 1u) {
        return 1;
    }

    if (party->getManager() == user) {
        set_teleport_member_state(seat, 0);
        set_teleport_state(2);
        send_teleport_status(8u);
        return static_cast<char>(
            reinterpret_cast<uintptr_t>(
                reset_teleport_data()) &
            0xffu);
    }

    set_teleport_member_state(seat, 4);
    check_cur_teleport_state();
    set_teleport_member_state(seat, 0);
    send_teleport_status(0);
    set_teleport_member_state(seat, 4);
    return static_cast<char>(process_teleport());
}

// IDA_EA: 0x85BC2B0
// IDA_NAME: _ZN14CPartyTelePort39send_teleport_effect_at_different_placeEi
// DEMANGLED: CPartyTelePort::send_teleport_effect_at_different_place(int)
CParty* CPartyTelePort::send_teleport_effect_at_different_place(
    int effect_type) {
    CParty* party = party_;
    if (!party) {
        return party;
    }

    std::vector<PartyTeleportEffectData> effects;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) ==
            1u) {
            CUser* user = party->get_user(seat);
            if (user) {
                const int area = user->get_area(false);
                const unsigned char village = static_cast<unsigned char>(
                    user->getCurCharacVill());
                bool found = false;
                for (std::vector<PartyTeleportEffectData>::iterator it =
                         effects.begin();
                     it != effects.end();
                     ++it) {
                    if (it->area == area && it->village == village) {
                        it->unique_ids.push_back(
                            static_cast<unsigned short>(
                                user->get_unique_id()));
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    PartyTeleportEffectData effect;
                    effect.village = village;
                    effect.area = area;
                    effect.unique_ids.push_back(static_cast<unsigned short>(
                        user->get_unique_id()));
                    effects.push_back(effect);
                }
            }
        }
    }

    for (std::vector<PartyTeleportEffectData>::iterator it = effects.begin();
         it != effects.end();
         ++it) {
        PacketGuard packet;
        packet.put_header(0, 379);
        packet.put_byte(effect_type);
        packet.put_byte(static_cast<unsigned char>(it->unique_ids.size()));
        for (std::vector<unsigned short>::iterator uid = it->unique_ids.begin();
             uid != it->unique_ids.end();
             ++uid) {
            packet.put_short(*uid);
        }
        packet.finalize(true);
        G_GameWorld()->send_to_area(it->village, it->area, packet);
    }
    return party;
}

// IDA_EA: 0x85BC64C
// IDA_NAME: _ZN14CPartyTelePort35send_teleport_effect_at_equal_placeEi
// DEMANGLED: CPartyTelePort::send_teleport_effect_at_equal_place(int)
void CPartyTelePort::send_teleport_effect_at_equal_place(int effect_type) {
    CParty* party = party_;
    if (!party) {
        return;
    }

    CUser* manager = party->getManager();
    if (!manager || G_GameWorld()->IsPVPChannel()) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 379);
    packet.put_byte(effect_type);

    std::vector<unsigned short> unique_ids;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) ==
            1u) {
            CUser* user = party->get_user(seat);
            if (user) {
                unique_ids.push_back(
                    static_cast<unsigned short>(user->get_unique_id()));
            }
        }
    }

    packet.put_byte(static_cast<unsigned char>(unique_ids.size()));
    for (std::vector<unsigned short>::iterator it = unique_ids.begin();
         it != unique_ids.end();
         ++it) {
        packet.put_short(*it);
    }
    packet.finalize(true);
    if (manager->get_state() == 3) {
        G_GameWorld()->send_to_area(
            static_cast<signed char>(manager->getCurCharacVill()),
            manager->get_area(false),
            packet);
    }
}

// IDA_EA: 0x859C114
// IDA_NAME: _ZN6CParty10leave_userEP5CUser20ENUM_PARTY_INFO_TYPE
// DEMANGLED: CParty::leave_user(CUser *,ENUM_PARTY_INFO_TYPE)
int CParty::leave_user(CUser* user, ENUM_PARTY_INFO_TYPE info_type) {
    if (!user) {
        LogManager::logFormat(1,
                              "party.cpp",
                              "int CParty::leave_user(CUser*, ENUM_PARTY_INFO_TYPE)",
                              1389,
                              "CParty::leave_user pUser is NULL");
        return 1;
    }
    if (channel_zombie_matching_authority::
            AbortMapOwnerBeforeLifecycle(this, user)) {
        return 0;
    }

    dual_raid::OnPartyMemberLeave(this, user);
    racing_dungeon::OnPartyMemberLeaving(this, user);

    party_teleport_.process_leave_user_at_teleport(user);

    secretshop::SECRET_SHOP_DATA* secret_shop_data = &secret_shop_data_;
    if (static_cast<unsigned char>(
            secret_shop_data->IsOpen())) {
        secret_shop_data->SetBuying(user->GetUserPosInParty(), false);
    }

    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (dungeon) {
        char member_names[256];
        std::memset(member_names, 0, sizeof(member_names));
        const int dungeon_type = battle_field_.instance_dungeon_mode();
        const char* names = _getMemberNames(member_names);
        const int dungeon_diff = battle_field_.get_dungeon_diff();
        const char* dungeon_name = reinterpret_cast<const CDungeon*>(dungeon)->GetDungeonName();
        CUser_history_log(user)->LeaveDungeon(dungeon_name,
                                              dungeon_diff,
                                              names,
                                              dungeon_type);
        if (static_cast<unsigned char>(IsSinglePlay()) &&
            static_cast<unsigned char>(user->isDungeonClearHackCheck())) {
            reinterpret_cast<WongWork::CHackAnalyzer*>(
                user->getHackAnalyzer())
                ->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x19c),
                    1,
                    0,
                    0);
        }
    }

    if (get_quick_party_index() != -1) {
        (G_CGameManager())->CheckOutQuickParty(this, false);
    }
    if (static_cast<unsigned char>(is_quick_party())) {
        send_quick_party_matching_result(false, user);
        if (get_member_count() == 2) {
            set_quick_party(false);
        }
    }

    const int member_count = get_member_count();
    if (member_count > 1) {
        reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->reportHackInfo();
        reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->resetHackInfo();
        if (getManager() == user) {
            change_manager();
            G_GameWorld()->send_party_info_to_all(
                this, info_type);
            send_party_realtime_info();
            const int reason =
                checkInoutConditionDungeon();
            sendInoutConditionDungeon(
                static_cast<RetryFailReason::T>(reason));
            return 0;
        }

        int seat = 0;
        for (;; ++seat) {
            if (seat > 3) {
                return 1;
            }
            if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                    1u &&
                members_[seat].user == user) {
                break;
            }
        }
        members_[seat].user = 0;
        members_[seat].user_uid = 0;
        user->SetPartyIndex(-1);
        user->set_charac_party_bonus_exp(0);
        user->set_charac_member_bonus_exp(0);
        user->set_charac_fatigue_buf_bonus_exp(0);
        user->set_charac_seria_buf_bonus_exp(0);
        if (user->getMoveSpace() != 1) {
            user->set_state(3);
        }
        G_GameWorld()->send_party_info_to_all(
            this, info_type);
        send_party_realtime_info();
        const int reason =
            checkInoutConditionDungeon();
        sendInoutConditionDungeon(
            static_cast<RetryFailReason::T>(reason));
        return 0;
    }

    const int party_index = user->GetPartyIndex();
    const int user_state = user->get_state();
    const char* account_id =
        NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0);
    cMyTrace trace("int CParty::leave_user(CUser*, ENUM_PARTY_INFO_TYPE)",
                   1464,
                   0);
    trace("<IN_DUNGEON_NULL_PARTY> m_id(%s), user_state(%d), PartyIndex(%d)",
          account_id,
          user_state,
          party_index);

    if (member_count <= 0) {
        LogManager::logFormat(
            1,
            "party.cpp",
            "int CParty::leave_user(CUser*, ENUM_PARTY_INFO_TYPE)",
            1477,
            "[PARTYPOOL] member_count : %d, m_pManager : %x, dungeon_idx : %u, m_iIndex : %u, cState : %d",
            member_count,
            reinterpret_cast<unsigned int>(getManager()),
            dungeon_index_,
            static_cast<unsigned int>(party_index_),
            state_);
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* member = members_[seat].user;
            if (member) {
                LogManager::logFormat(
                    1,
                    "party.cpp",
                    "int CParty::leave_user(CUser*, ENUM_PARTY_INFO_TYPE)",
                    1496,
                    "User[%d] %x, ch_state %d, uid : %d / %d, charac_no : %u",
                    seat,
                    reinterpret_cast<unsigned int>(member),
                    member->get_state(),
                    member->GetUID(),
                    members_[seat].user_uid,
                    member->getCurCharacNo());
            } else {
                LogManager::logFormat(
                    1,
                    "party.cpp",
                    "int CParty::leave_user(CUser*, ENUM_PARTY_INFO_TYPE)",
                    1485,
                    "User[%d] NULL, ch_state NONE, uid : %d, charac_no : 0",
                    seat,
                    members_[seat].user_uid);
            }
        }
    }

    user->SetPartyIndex(-1);
    user->set_charac_party_bonus_exp(0);
    user->set_charac_member_bonus_exp(0);
    user->set_charac_fatigue_buf_bonus_exp(0);
    user->set_charac_seria_buf_bonus_exp(0);
    if (user->getMoveSpace() != 1) {
        user->set_state(3);
    }
    destroy();
    return 0;
}

// IDA_EA: 0x859AE76
// IDA_NAME: _ZN6CParty13gen_timer_keyE13TIMER_MESSAGE
// DEMANGLED: CParty::gen_timer_key(TIMER_MESSAGE)
int CParty::gen_timer_key(TIMER_MESSAGE message) {
    return ++timer_keys_[static_cast<int>(message) -
                         TIMER_MESSAGE_MAP_LOADING];
}

// IDA_EA: 0x85B1FE0
// IDA_NAME: _ZN6CParty21send_loadingcheck_msgEv
// DEMANGLED: CParty::send_loadingcheck_msg(void)
int CParty::send_loadingcheck_msg() {
    loading_check_pending_ = 0;
    const int timeout = (get_member_count() == 1)
                            ? CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT
                            : CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT;
    const unsigned int timer_key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_MAP_LOADING));
    return G_TimerQueue()->InsertTimer(
        1,
        party_index_,
        11,
        timeout,
        timer_key,
        0);
}

// IDA_EA: 0x859AEA4
// IDA_NAME: _ZN6CParty15check_timer_keyE13TIMER_MESSAGEi
// DEMANGLED: CParty::check_timer_key(TIMER_MESSAGE,int)
bool CParty::check_timer_key(TIMER_MESSAGE message, int key) {
    return timer_keys_[static_cast<int>(message) -
                       TIMER_MESSAGE_MAP_LOADING] == key;
}

// IDA_EA: 0x859AD4C
// IDA_NAME: _ZN6CParty7destroyEv
// DEMANGLED: CParty::destroy(void)
CParty* CParty::destroy() {
    if (channel_zombie_matching_authority::
            AbortMapOwnerBeforeLifecycle(this, manager_)) {
        return this;
    }
    time_spiral::ResetParty(this);
    dual_raid::OnPartyDestroyed(this);
    racing_dungeon::OnPartyDestroyed(this);

    if (auto_created_) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_PARTY_INFO);
        packet.put_short(1);
        packet.put_short(GetPartyIndex());
        packet.put_byte(3);
        packet.finalize(true);
        manager_->SendPacket(static_cast<CUser::eSendTarget>(1), packet);
    } else {
        G_GameWorld()->send_party_info_to_all(this, 3);
    }

    init();
    (G_CGameManager())->PutParty(this);
    used_coin_count_ = 0;
    return this;
}

// IDA_EA: 0x859B73E
// IDA_NAME: _ZN6CParty32send_quick_party_matching_resultEbP5CUser
// DEMANGLED: CParty::send_quick_party_matching_result(bool,CUser *)
void CParty::send_quick_party_matching_result(bool success, CUser* user) {
    void* const party = this;
    PacketGuard packet;
    packet.put_header(0, 382);
    packet.put_byte(success);
    packet.finalize(true);
    if (reinterpret_cast<CParty*>(party)->get_member_count() > 2 && user) {
        user->Send(packet);
    } else {
        reinterpret_cast<CParty*>(party)->send_to_party(packet);
    }
}

// IDA_EA: 0x859C832
// IDA_NAME: _ZN6CParty14change_managerEv
// DEMANGLED: CParty::change_manager(void)
int CParty::change_manager() {
    CUser* manager = manager_;
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (members_[seat].user == manager) {
            manager->SetPartyIndex(-1);
            manager->set_state(3);
            members_[seat].user = 0;
            members_[seat].user_uid = 0;
            break;
        }
    }

    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat))) {
            manager_ = members_[seat].user;
            set_host();
            send_host_info();
            return 1;
        }
    }

    LogManager::logFormat(1,
                          "party.cpp",
                          "bool CParty::change_manager()",
                          0x689,
                          "there is no manager(party empty)");
    return 0;
}

// IDA_EA: 0x859CBAC
// IDA_NAME: _ZN6CParty23get_party_realtime_infoER11PacketGuard
// DEMANGLED: CParty::get_party_realtime_info(PacketGuard &)
int CParty::get_party_realtime_info(PacketGuard& packet) {
    int count_index = packet.get_index();
    int member_count = 0;
    packet.put_byte(0);
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat))) {
            CUser* member = members_[seat].user;
            packet.put_short(member->get_unique_id());
            packet.put_byte(member->getCurCharacR()
                                ? member->getCurCharacStamina()
                                : 0);
            packet.put_byte(
                isHelpAbuseParty(member));
            packet.put_byte(seat);
            ++member_count;
        }
    }
    packet.put_byte(count_index, member_count);
    return 1;
}

// IDA_EA: 0x859CD24
// IDA_NAME: _ZN6CParty24send_party_realtime_infoEv
// DEMANGLED: CParty::send_party_realtime_info(void)
void CParty::send_party_realtime_info() {
    void* const party = this;
    PacketGuard packet;
    packet.put_header(0, 153);
    if (static_cast<unsigned char>(
            reinterpret_cast<CParty*>(party)->get_party_realtime_info(packet))) {
        packet.finalize(true);
        reinterpret_cast<CParty*>(party)->send_to_party(packet);
    }
}

// IDA_EA: 0x859CDBC
// IDA_NAME: _ZN6CParty8set_hostEv
// DEMANGLED: CParty::set_host(void)
int CParty::set_host() {
    host_ = manager_;
    return 1;
}

// IDA_EA: 0x859CDD2
// IDA_NAME: _ZN6CParty14send_host_infoEv
// DEMANGLED: CParty::send_host_info(void)
void CParty::send_host_info() {
    PacketGuard packet;
    packet.put_header(0, 26);
    CUser* const host = host_;
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (members_[seat].user == host) {
            packet.put_byte(seat);
            packet.finalize(true);
            send_to_party(packet);
            break;
        }
    }
}

// IDA_EA: 0x85A21CA
// IDA_NAME: _ZN6CParty16isHelpAbusePartyEP5CUser
// DEMANGLED: CParty::isHelpAbuseParty(CUser *)
int CParty::isHelpAbuseParty(CUser* /*user*/) {
    return 0;
}

// IDA_EA: 0x85A21D4
// IDA_NAME: _ZN6CParty16isHelpAbusePartyEv
// DEMANGLED: CParty::isHelpAbuseParty(void)
int CParty::isHelpAbuseParty() {
    void* const party = this;
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        if (static_cast<unsigned char>(reinterpret_cast<CParty*>(party)->_checkValidUser(seat)) ==
            1u) {
            if (static_cast<unsigned char>(
                    reinterpret_cast<CParty*>(party)->isHelpAbuseParty(
                        reinterpret_cast<CParty*>(party)->get_user(seat)))) {
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x85AC95E
// IDA_NAME: _ZN6CParty25sendInoutConditionDungeonEN15RetryFailReason1TE
// DEMANGLED: CParty::sendInoutConditionDungeon(RetryFailReason::T)
void CParty::sendInoutConditionDungeon(RetryFailReason::T reason) {
    void* const party = this;
    if (reason != 10) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 260);
        packet.put_byte(reason);
        packet.finalize(true);
        reinterpret_cast<CParty*>(party)->send_to_party(packet);
    }
}

// IDA_EA: 0x859A974
// IDA_NAME: _ZN6CParty4initEv
// DEMANGLED: CParty::init(void)
CParty* CParty::init() {
    time_spiral::ResetParty(this);
    golden_commerce::ReleaseParty(this);
    ZombieItemBox::Reset(this);
    monster_move_system::ResetParty(this);
    trace_mob_die_hack_.reportHackInfo();
    trace_mob_die_hack_.reset();

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
            1u) {
            CUser* member = get_user(seat);
            reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->reportHackInfo();
            reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->resetHackInfo();
        }
    }

    member_level_gap_ = -1;
    recv_result_flag_ = 0;
    std::memset(title_, 0, sizeof(title_));
    title_index_ = static_cast<unsigned char>(get_rand_int(255) + 1);
    user_max_ = 4;
    dungeon_index_ = 0xFFFFu;
    dungeon_difficulty_ = 0xFFu;
    member_count_ = 0;
    state_ = 0;
    single_play_ = 0;
    auto_created_ = 0;
    SetEPLPState(0);
    selected_eplp_command_ = 0xFFu;
    SetAssaultState(0);

    for (int seat = 0; seat <= 3; ++seat) {
        members_[seat].Init();
        selected_cards_[0][seat] = 0xFFu;
        selected_cards_[1][seat] = 0xFFu;
    }
    dungeon_clear_state_ = 0;

    for (int group = 0; group <= 1; ++group) {
        for (int seat = 0; seat <= 3; ++seat) {
            play_result_values_[group][seat] = 0;
        }
    }

    stHellParty_t* hellparty_script_values =
        G_CDataManager()->get_hellparty_script_values();
    battle_data_.SetHellPartyValueTotal(hellparty_script_values->base_value);
    battle_field_.reset_hell_party_value();
    battle_data_.Reset();
    battle_field_.reset_field();
    result_recv_flag_.Clear();
    used_coin_count_ = 0;
    world_map_ = 0;
    straight_victories_ = 0;
    secret_shop_data_.clear();
    ResetPremiumGoldCardParty();
    ResetPremiumGoldCardDefaultItem();
    weekend_event_ = 0;
    gm_random_buff_type_ = 11;
    init_quick_party_data();
    tournament_dungeon_clear_state_ = 0;
    dungeon_map_saving_ = 0;
    return this;
}
// IDA_EA: 0x85A7A72
// IDA_NAME: _ZN6CParty24CheckClearQuestHellPartyERSt6vectorIiSaIiEE
// DEMANGLED: CParty::CheckClearQuestHellParty(std::vector<int,std::allocator<int>> &)
bool CParty::CheckClearQuestHellParty(std::vector<int>& quests) {
    quests.clear();
    if (!world_map_) {
        return false;
    }

    const std::vector<int>& hell_quests = world_map_->GetHellDungeonList();
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) !=
            1u) {
            continue;
        }

        for (std::vector<int>::const_iterator it = hell_quests.begin();
             it != hell_quests.end();
             ++it) {
            UserQuest* quest = get_user(seat)->getCurCharacQuestR();
            if (!quest || static_cast<unsigned char>(quest->isClearQuest(*it)) != 1u) {
                quests.push_back(seat);
                break;
            }
        }
    }
    return false;
}

// IDA_EA: 0x859D5E8
// IDA_NAME: _ZN6CParty18CheckMemberFatigueEv
// DEMANGLED: CParty::CheckMemberFatigue(void)
int CParty::CheckMemberFatigue() {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
                1u &&
            static_cast<unsigned char>(get_user(seat)->CheckFatigue()) != 1u) {
            return seat + 1;
        }
    }
    return 0;
}

// IDA_EA: 0x859F75E
// IDA_NAME: _ZN6CParty31get_blood_dungeon_admission_feeEP5CUser
// DEMANGLED: CParty::get_blood_dungeon_admission_fee(CUser *)
int CParty::get_blood_dungeon_admission_fee(CUser* user) {
    CCharacBloodDungeon* blood_dungeon =
        static_cast<CCharacBloodDungeon*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
    const int enter_count = blood_dungeon->GetEnterCount();
    const int charac_level = user->get_charac_level();
    ServerParameterScript* server_parameter =
        G_CDataManager()->get_server_parameter_script();
    const int price_average = server_parameter->GetPriceAverage(charac_level);
    const int admission_increase =
        price_average * server_parameter->GetAdmissionIncrease(enter_count) /
        100;
    return admission_increase + price_average +
           server_parameter->GetLottoCost();
}

// IDA_EA: 0x859E0D2
// IDA_NAME: _ZN6CParty23make_blood_dungeon_infoEP5CUserR11PacketGuard
// DEMANGLED: CParty::make_blood_dungeon_info(CUser *,PacketGuard &)
int CParty::make_blood_dungeon_info(CUser* manager, PacketGuard& packet) {
    if (!world_map_) {
        const int world_map_index =
            G_GameWorld()->GetWorldMapIndex(manager);
        if (world_map_index > 0) {
            CWorldMapList* const world_map_list =
                CDataManagerWorldMapList(G_CDataManager());
            world_map_ = world_map_list
                             ? world_map_list->find_world_map(world_map_index)
                             : 0;
        }
    }

    if (!world_map_) {
        return 0;
    }
    if (static_cast<unsigned char>(world_map_->hasDeathTower()) != 1u) {
        return 0;
    }

    packet.put_byte(get_member_count());
    for (int seat = 0; seat <= 3; ++seat) {
        if (!static_cast<unsigned char>(_checkValidUser(seat))) {
            continue;
        }

        CUser* member = get_user(seat);
        packet.put_short(static_cast<short>(member->get_unique_id()));
        int count_index = packet.get_index();
        int blood_dungeon_count = 0;
        packet.put_byte(0);

        std::vector<int>* dungeon_list =
            world_map_->GetDungeonList();
        for (std::vector<int>::iterator it = dungeon_list->begin();
             it != dungeon_list->end();
             ++it) {
            CDungeon* dungeon = static_cast<CDungeon*>(
                (G_CDataManager())->find_dungeon(*it));
            if (!dungeon || dungeon->get_blood_dungeon_type() != 1) {
                continue;
            }

            packet.put_short(*it);
            packet.put_int(
                get_blood_dungeon_admission_fee(member));
            CCharacBloodDungeon* blood_dungeon =
                static_cast<CCharacBloodDungeon*>(
                    member->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
            packet.put_int(blood_dungeon->GetEnterCount());
            // 0828 reader 0x5AC5D9 consumes u16 dungeon + u32 fee +
            // u32 enter count only; a weekly count shifts jackpot and floor.
            ++blood_dungeon_count;
        }
        packet.put_byte(count_index, blood_dungeon_count);
    }

    CEventBase* second_event =
        GlobalData::s_event_manager->GetRepeatEvent(46);
    if (second_event->IsEventing(0)) {
        packet.put_int(
            static_cast<CBloodDungeonRewardSecondEvent*>(second_event)
                ->GetGold());
    } else {
        CEventBase* first_event =
            GlobalData::s_event_manager->GetRepeatEvent(45);
        if (first_event->IsEventing(0)) {
            packet.put_int(
                static_cast<CBloodDungeonRewardFirstEvent*>(first_event)
                    ->GetGold());
        } else {
            ServerParameterScript* server_parameter =
                G_CDataManager()->get_server_parameter_script();
            packet.put_int(server_parameter->GetJackpot());
        }
    }
    return 1;
}

// IDA_EA: 0x859E480
// IDA_NAME: _ZN6CParty29make_blood_dungeon_info_blankER11PacketGuard
// DEMANGLED: CParty::make_blood_dungeon_info_blank(PacketGuard &)
int CParty::make_blood_dungeon_info_blank(PacketGuard& packet) {
    void* const party = this;
    packet.put_byte(reinterpret_cast<CParty*>(party)->get_member_count());
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(reinterpret_cast<CParty*>(party)->_checkValidUser(seat))) {
            packet.put_short(static_cast<short>(
                (reinterpret_cast<CParty*>(party)->get_user(seat))->get_unique_id()));
            packet.put_byte(0);
        }
    }
    return packet.put_int(0);
}

// IDA_EA: 0x85BE8A0
// IDA_NAME: _ZN6CParty31isDungeonDifficultyInfromNoticeEb
// DEMANGLED: CParty::isDungeonDifficultyInfromNotice(bool)
int CParty::isDungeonDifficultyInfromNotice(bool tower) {
    CUser* manager = getManager();
    if (static_cast<unsigned char>(manager->getMemberDungeonFlag())) {
        return 0;
    }
    if (!manager->getCurCharacTutorialFlag()) {
        return 0;
    }
    if (tower) {
        return 0;
    }
    if (battle_field_.village_monster_timer_token()) {
        return 0;
    }
    return getManager()->getMoveSpace() != 1;
}
// IDA_EA: 0x859AEC2
// IDA_NAME: _ZN6CParty17battle_data_resetEv
// DEMANGLED: CParty::battle_data_reset(void)
int CParty::battle_data_reset() {
    golden_commerce::ResetParty(this);
    ZombieItemBox::Reset(this);
    monster_move_system::ResetParty(this);
    trace_mob_die_hack_.reportHackInfo();
    trace_mob_die_hack_.reset();

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
            1u) {
            CUser* member = get_user(seat);
            void* analyzer = member->getHackAnalyzer();
            reinterpret_cast<WongWork::CHackAnalyzer*>(analyzer)->reportHackInfo();
            reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->resetHackInfo();
        }
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
            1u) {
            CUser* member = get_user(seat);
            member->set_state(3);
            member->set_charac_party_bonus_exp(0);
            member->set_charac_member_bonus_exp(0);
            member->set_charac_fatigue_buf_bonus_exp(0);
            member->set_charac_seria_buf_bonus_exp(0);
        }
    }

    battle_data_.Reset();
    battle_field_.reset_field();
    state_ = 1;
    SetEPLPState(0);
    SetSelectedEPLPCmd(-1);
    dungeon_clear_state_ = 0;
    member_level_gap_ = -1;
    map_int_int_.clear();
    secret_shop_data_.clear();
    SetEndRouting();
    ResetPremiumGoldCardParty();
    ResetPremiumGoldCardDefaultItem();
    tournament_dungeon_clear_state_ = 0;
    init_quick_party_data();
    dungeon_map_saving_ = 0;

    passed_map_info_list_.clear();
    return 0;
}

// Phase-2 Zombie owner adapter.  The ordinary game_start body below remains
// byte-for-byte source-identical for ABI/behavior compatibility.  This
// specialized entry is intentionally non-virtual and restricted to the
// singleton map adapter installed for one authoritative PvP_Room member.
bool CParty::game_start_zombie_owner(CUser* manager) {
    if (!manager || state_ == 2u || get_member_count() != 1 ||
        !static_cast<unsigned char>(_checkValidUser(0)) ||
        get_user(0) != manager || getManager() != manager) {
        return false;
    }

    tournament_victory_ = 0;
    random_member_index_ = static_cast<uint32_t>(get_rand_int(1));
    CSystemTime* const system_time = GlobalData::system_time();
    game_start_sec_ = static_cast<uint32_t>(
        system_time ? system_time->getCurSec() : 0);

    battle_data_.member_live(0) = 1;
    battle_data_.member_enter_map(0) = 1;
    battle_data_.member_state_flag(0) = 0;
    battle_data_.card_info_member_state(0) = 0;

    // The complete cohort NOTI2 is emitted separately by the central owner.
    // This call performs only the authoritative world transition.
    G_GameWorld()->goto_dungeon(manager);
    if (manager->get_state() != 5) return false;

    const int trade_space_index = manager->GetTradeSpace();
    if (trade_space_index != -1) {
        CTradeSpace* const trade_space =
            G_CGameManager()->GetTradeSpace(trade_space_index);
        if (trade_space &&
            !static_cast<unsigned char>(
                trade_space->check_trade_possibility())) {
            trade_space->cancel_trade();
            G_CGameManager()->PutTradeSpace(trade_space);
        }
    }

    UserQuest* const quest = manager->getCurCharacQuestW();
    if (quest) quest->authen_data.reset();

    // Preserve the ordinary authoritative state transition notification, but
    // do not emit the singleton NOTI2 or elect/send a singleton NOTI26 host.
    PacketGuard state_packet;
    state_packet.put_header(0, 3);
    state_packet.put_byte(1);
    state_packet.put_short(static_cast<short>(manager->get_unique_id()));
    state_packet.put_byte(1);
    state_packet.finalize(true);
    if (static_cast<unsigned char>(IsExistInvisible()) ||
        manager->getMoveSpace() == 1) {
        send_to_party(state_packet);
    } else {
        G_GameWorld()->send_all(state_packet);
    }

    hell_quest_notice_ = 0;
    world_map_ = 0;
    int world_map_index = G_GameWorld()->GetWorldMapIndex(manager);
    if (world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        world_map_ = world_map_list
                         ? world_map_list->find_world_map(world_map_index)
                         : 0;
    }

    // This CParty is an invisible server-side map owner, not an ordinary
    // selectable-dungeon party.  NOTI27 opens the town's normal dungeon
    // picker; on the Zombie channel that exposes Golden Commerce 3054/3056
    // and lets a completed Zombie match escape into the cart dungeon.  The
    // authority selects DGN3101 directly, so no selection payload belongs on
    // this path.

    recv_result_flag_ = 0;
    party_teleport_.reset_teleport_data();
    ZombieItemBox::Start(this);
    return true;
}

// IDA_EA: 0x859D718
// IDA_NAME: _ZN6CParty10game_startEP5CUser
// DEMANGLED: CParty::game_start(CUser *)
void CParty::game_start(CUser* manager) {
    if (state_ == 2u) {
        return;
    }

    PacketGuard basic_packet;
    basic_packet.clear();
    basic_packet.put_header(0, 2);
    basic_packet.put_byte(1);
    basic_packet.put_short(static_cast<short>(get_member_count()));

    tournament_victory_ = 0;
    const int member_count_for_random = get_member_count();
    random_member_index_ = get_rand_int(member_count_for_random);
    CSystemTime* system_time = GlobalData::system_time();
    game_start_sec_ = static_cast<uint32_t>(
        system_time ? system_time->getCurSec() : 0);

    PacketGuard state_packet;
    state_packet.put_header(0, 3);
    int state_count = 0;
    int count_index = state_packet.get_index();
    state_packet.put_byte(state_count);

    bool hack_village_area = false;
    const unsigned char manager_village =
        static_cast<unsigned char>(manager->getCurCharacVill());
    const int manager_area = manager->get_area(false);

    for (int seat = 0; seat <= 3; ++seat) {
        if (!static_cast<unsigned char>(_checkValidUser(seat))) {
            continue;
        }

        CUser* member = get_user(seat);
        const unsigned char member_village =
            static_cast<unsigned char>(member->getCurCharacVill());
        const int member_area = member->get_area(false);
        if ((manager_village != member_village || manager_area != member_area) &&
            static_cast<unsigned char>(is_quick_party()) != 1u &&
            battle_field_.village_monster_timer_token() == 0u) {
            LogManager::logFormat(
                1,
                "party.cpp",
                "void CParty::game_start(CUser*)",
                0x84C,
                "[HACK_VILL_AREA] m_id : %s, Vill(%d,%d), Area(%d,%d)",
                NumberToString(static_cast<unsigned int>(member->get_acc_id()),
                               0),
                manager_village,
                member_village,
                manager_area,
                member_area);
            hack_village_area = true;
            break;
        }

        battle_data_.member_live(seat) = 1;
        battle_data_.member_enter_map(seat) = 1;
        battle_data_.member_state_flag(seat) = 0;
        battle_data_.card_info_member_state(seat) = 0;

        member->make_basic_info(basic_packet, 1);
        G_GameWorld()->goto_dungeon(member);
        if (member->get_state() == 5) {
            state_packet.put_short(static_cast<short>(member->get_unique_id()));
            state_packet.put_byte(1);
            ++state_count;
        }

        const int trade_space_index = member->GetTradeSpace();
        if (trade_space_index != -1) {
            CTradeSpace* trade_space =
                G_CGameManager()->GetTradeSpace(trade_space_index);
            if (trade_space &&
                !static_cast<unsigned char>(
                    trade_space->check_trade_possibility())) {
                trade_space->cancel_trade();
                G_CGameManager()->PutTradeSpace(trade_space);
            }
        }

        UserQuest* quest = member->getCurCharacQuestW();
        if (quest) {
            quest->authen_data.reset();
        }
    }

    if (hack_village_area) {
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* member = get_user(seat);
            if (!member) {
                continue;
            }
            LogManager::logFormat(
                1,
                "party.cpp",
                "void CParty::game_start(CUser*)",
                0x88A,
                "[HACK_VILL_AREA] m_id : %s, Vill(%d), Area(%d)",
                NumberToString(static_cast<unsigned int>(member->get_acc_id()),
                               0),
                manager_village,
                manager_area);
            member->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
        }
        return;
    }

    state_packet.put_byte(count_index, state_count);
    state_packet.finalize(true);
    if (static_cast<unsigned char>(IsExistInvisible()) ||
        getManager()->getMoveSpace() == 1) {
        send_to_party(state_packet);
    } else {
        G_GameWorld()->send_all(state_packet);
    }

    basic_packet.finalize(true);
    send_to_party(basic_packet);
    set_host();
    send_host_info();

    hell_quest_notice_ = 0;
    world_map_ = 0;
    int world_map_index = G_GameWorld()->GetWorldMapIndex(manager);
    if (world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        world_map_ =
            world_map_list
                ? world_map_list->find_world_map(world_map_index)
                : 0;
    }

    basic_packet.clear();
    basic_packet.put_header(0, ENUM_NOTIPACKET_ENTER_SELECT_DUNGEON);
    if (get_member_count() == 1) {
        stHellParty_t* hellparty_script_values =
            G_CDataManager()->get_hellparty_script_values();
        if (hellparty_script_values) {
            battle_data_.hell_party_initial_value() =
                hellparty_script_values->base_value;
        }
    }

    const float hp_rate = battle_data_.hell_party_value_total();
    const int hp_max = battle_field_.hell_party_value_max();
    const int hp_percent = hp_max ? static_cast<int>(hp_rate * 100.0 / hp_max) : 0;
    basic_packet.put_int(hp_percent);

    std::vector<int> clear_quest_hell_party;
    if (world_map_ &&
        static_cast<unsigned char>(world_map_->IsInHellDungeon()) &&
        static_cast<unsigned char>(CBattle_Field::IsEnableHellDungeon())) {
        CheckClearQuestHellParty(clear_quest_hell_party);
        if (clear_quest_hell_party.empty()) {
            basic_packet.put_byte(0);
            hell_quest_notice_ = 1;
        } else {
            basic_packet.put_byte(
                static_cast<unsigned char>(clear_quest_hell_party.size()));
            for (std::vector<int>::iterator it = clear_quest_hell_party.begin();
                 it != clear_quest_hell_party.end();
                 ++it) {
                basic_packet.put_short(static_cast<short>(*it));
            }
        }
    } else {
        basic_packet.put_byte(0);
    }

    // 0828 reader 0x5AC245 consumes a second u8-counted member list before
    // the blood-dungeon member block.  There is no producer for that list
    // here; keep it empty without moving the existing hell-quest list.
    basic_packet.put_byte(0);
    if (static_cast<unsigned char>(
            make_blood_dungeon_info(manager, basic_packet)) != 1u) {
        make_blood_dungeon_info_blank(basic_packet);
    }

    bool death_tower = false;
    world_map_index = G_GameWorld()->GetWorldMapIndex(manager);
    if (world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        CWorldMap* world_map = world_map_list
                                   ? world_map_list->find_world_map(world_map_index)
                                   : 0;
        (void)world_map;
        if (world_map_ &&
            static_cast<unsigned char>(world_map_->hasDeathTower())) {
            death_tower = true;
        }
    }

    // 0828 NOTI27: the client at 0x5AC070 reads u16 floor before u8 notice.
    // Non-tower maps still require the field.  Unlike CN, there is no
    // optional-record gate between these two fields in the 0828 reader.
    const uint16_t tower_floor =
        death_tower ? GetTowerOfDespairSelectFloor(manager) : 0u;
    basic_packet.put_short(tower_floor);
    if (static_cast<unsigned char>(
            isDungeonDifficultyInfromNotice(death_tower))) {
        basic_packet.put_byte(1);
        getManager()->setMemberDungeonFlag(true);
    } else {
        basic_packet.put_byte(0);
    }

    LogManager::logFormat(
        1, "party.cpp", "void CParty::game_start(CUser*)", 0,
        "[TOD_SELECT][NOTI27] charac=%d world_map=%d tower=%u floor=%u bytes=%d",
        manager->get_charac_no(-1), world_map_index,
        static_cast<unsigned int>(death_tower),
        static_cast<unsigned int>(tower_floor), basic_packet.get_index());

    if (death_tower) {
        battle_field_.setWorldMapTowerArea(death_tower);
    }

    basic_packet.finalize(true);
    send_to_party(basic_packet);
    recv_result_flag_ = 0;
    party_teleport_.reset_teleport_data();
    ZombieItemBox::Start(this);
}
// IDA_EA: 0x85ACA60
// IDA_NAME: _ZN6CParty15ReturnToVillageEv
// DEMANGLED: CParty::ReturnToVillage(void)
void CParty::ReturnToVillage() {
    if (channel_zombie_matching_authority::
            AbortMapOwnerBeforeLifecycle(this, manager_)) {
        return;
    }
    battle_field_.setBloodState(
        static_cast<ENUM_BLOOD_STATE>(0));

    PacketGuard packet;
    packet.put_header(0, 2);

    if (auto_created_ == 0u) {
        packet.put_byte(0);
        packet.put_short(get_member_count());

        char member_names[256];
        std::memset(member_names, 0, sizeof(member_names));
        _getMemberNames(member_names);

        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(_checkValidUser(seat))) {
                CDungeon* const dungeon = battle_field_.get_dungeon();
                CUser* member = get_user(seat);
                if (dungeon) {
                    CUser_history_log(member)->LeaveDungeon(
                        reinterpret_cast<const CDungeon*>(dungeon)->GetDungeonName(),
                        battle_field_.get_dungeon_diff(),
                        member_names,
                        battle_field_.instance_dungeon_mode());
                }
                if (dungeon) {
                    CUser* manager = manager_;
                    if (static_cast<unsigned char>(IsSinglePlay()) &&
                        static_cast<unsigned char>(
                            manager->isDungeonClearHackCheck())) {
                        reinterpret_cast<WongWork::CHackAnalyzer*>((manager->getHackAnalyzer()))->addServerHackCnt(
                            manager,
                            static_cast<WongWork::ENUM_HACKTYPE>(412),
                            1,
                            0,
                            0);
                    }
                }
                G_GameWorld()->out_from_dungeon(member);
                member->make_basic_info(packet, 0);
            }
        }

        set_state(1);
        battle_data_reset();
        packet.finalize(true);
        if (!static_cast<unsigned char>(IsExistInvisible())) {
            G_GameWorld()->send_all(packet);
        } else {
            send_to_party(packet);
        }
        SetEPLPState(0);
        return;
    }

    CUser* manager = manager_;
    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (dungeon) {
        char member_names[256];
        std::memset(member_names, 0, sizeof(member_names));
        const char* names = _getMemberNames(member_names);
        CUser_history_log(manager)->LeaveDungeon(
            reinterpret_cast<const CDungeon*>(dungeon)->GetDungeonName(),
            battle_field_.get_dungeon_diff(),
            names,
            battle_field_.instance_dungeon_mode());
        if (static_cast<unsigned char>(IsSinglePlay()) &&
            static_cast<unsigned char>(
                manager->isDungeonClearHackCheck())) {
            reinterpret_cast<WongWork::CHackAnalyzer*>(
                manager->getHackAnalyzer())
                ->addServerHackCnt(
                    manager,
                    static_cast<WongWork::ENUM_HACKTYPE>(412),
                    1,
                    0,
                    0);
        }
    }

    G_GameWorld()->out_from_dungeon(manager);
    packet.put_byte(0);
    packet.put_short(1);

    const int party_index = manager->GetPartyIndex();
    const int user_state = manager->get_state();
    const char* account_id =
        NumberToString(static_cast<unsigned int>(manager->get_acc_id()),
                       0);
    cMyTrace trace("void CParty::ReturnToVillage()", 10004, 0);
    trace("<IN_DUNGEON_NULL_PARTY> m_id(%s), user_state(%d), PartyIndex(%d)",
          account_id,
          user_state,
          party_index);

    manager->SetPartyIndex(-1);
    if (manager->getMoveSpace() != 1) {
        manager->set_state(3);
    }
    manager->make_basic_info(packet, 0);
    destroy();
    packet.finalize(true);

    CUser* current_manager = getManager();
    if (current_manager->getMoveSpace() == 1) {
        send_to_party(packet);
    } else {
        CUser* except_user = manager_;
        G_GameWorld()->send_all(packet, except_user);
    }
    SetEPLPState(0);
}
// IDA_EA: 0x85B9596
// IDA_NAME: _ZN6CParty21OnFightVillageMonsterEP5CUseri
// DEMANGLED: CParty::OnFightVillageMonster(CUser *,int)
int CParty::OnFightVillageMonster(CUser* user, int monster_index) {
    const int fight_result =
        reinterpret_cast<village_attacked::CVillageMonsterMgr*>(
            GlobalData::s_villageMonsterMgr)->OnFightVillageMonster(
                user, static_cast<unsigned short>(monster_index));
    if (fight_result) {
        return fight_result;
    }

    game_start(user);

    const int dungeon_index = battle_field_.get_dungeon_index();
    const int cleared_dungeon_diff = 0;
    CDungeon* dungeon = static_cast<CDungeon*>(
        (G_CDataManager())->find_dungeon(dungeon_index));
    if (!dungeon) {
        return 21;
    }

    CWorldMap* world_map = 0;
    const int world_map_index = G_GameWorld()->GetWorldMapIndex(user);
    if (world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        world_map = world_map_list
                        ? world_map_list->find_world_map(world_map_index)
                        : 0;
        world_map_ = world_map;
    }

    const int member_count = get_member_count();
    if (static_cast<unsigned char>(battle_field_.SelectDungeon(
            dungeon_index,
            dungeon,
            cleared_dungeon_diff,
            static_cast<ENUM_DUNGEON_TYPE>(0),
            member_count,
            world_map,
            false,
            0)) != 1u) {
        return 1;
    }

    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    _getMemberNames(member_names);
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) ==
            1u) {
            char* names = _getMemberNames(member_names);
            CUser_history_log(get_user(seat))
                ->EnterDungeon(
                reinterpret_cast<const CDungeon*>(dungeon)->GetDungeonName(),
                cleared_dungeon_diff,
                names,
                0);
        }
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, ENUM_NOTIPACKET_DUNGEON_INFO);
    packet.put_short(static_cast<short>(dungeon_index));
    packet.put_byte(cleared_dungeon_diff);
    battle_field_.MakePacket(ENUM_NOTIPACKET_DUNGEON_INFO, &packet);
    send_to_party(packet);

    packet.clear();
    battle_field_.MakePacket(ENUM_NOTIPACKET_START_MAP, &packet);
    send_to_party(packet);

    const uint32_t cur_tick = static_cast<uint32_t>(
        GlobalData::system_time() ?
            GlobalData::system_time()->getCurTickCount() :
            0);
    battle_data_.map_tick_211() = cur_tick;
    battle_data_.map_tick_212() = 0;
    battle_data_.map_pause_time() = 0;
    battle_data_.map_start_time() = 0;
    battle_data_.map_tick_214() = 0;
    battle_data_.map_tick_215() = 0;
    battle_data_.map_tick_216() = battle_data_.map_tick_211();
    battle_data_.map_tick_217() = battle_data_.map_tick_212();

    set_state(2);
    SetEPLPState(1);
    send_loadingcheck_msg();

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_FIGHT_VILLAGE_MONSTER);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}
// IDA_EA: 0x85B99AC
// IDA_NAME: _ZN6CParty30OnFinishVillageMonsterFightingEP5CUserb
// DEMANGLED: CParty::OnFinishVillageMonsterFighting(CUser *,bool)
int CParty::OnFinishVillageMonsterFighting(CUser* user, bool arg3) {
    CUser* members[4];
    std::memset(members, 0, sizeof(members));
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) ==
            1u) {
            members[seat] = get_user(seat);
        }
    }

    if (!user) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(
                    checkValidUser(seat)) == 1u) {
                user = get_user(seat);
                break;
            }
        }
    }
    if (!user) {
        return 3;
    }

    bool clear = false;
    if (static_cast<unsigned char>(battle_field_.check_grid_clear())) {
        clear = true;
    }

    const int kill_result =
        reinterpret_cast<village_attacked::CVillageMonsterMgr*>(
            GlobalData::s_villageMonsterMgr)->OnKillVillageMonster(user, clear);
    if (kill_result) {
        return kill_result;
    }

    giveup_game(user, false, clear, arg3);
    battle_field_.village_monster_uid_ = 0;

    PacketGuard finish_packet;
    finish_packet.clear();
    finish_packet.put_header(
        1, ENUM_CMDPACKET_FINISH_VILLAGE_MONSTER_FIGHTING);
    finish_packet.put_byte(1);
    finish_packet.finalize(true);
    user->Send(finish_packet);

    if (!static_cast<unsigned char>(
            reinterpret_cast<village_attacked::CVillageMonsterMgr*>(
                GlobalData::s_villageMonsterMgr)->OnEvent()) &&
        reinterpret_cast<village_attacked::CVillageMonsterMgr*>(
            GlobalData::s_villageMonsterMgr)->GetRewardType() == 1) {
        CSystemTime* system_time = GlobalData::system_time();
        const unsigned int cur_sec =
            system_time ? system_time->getCurSec() : 0;
        const unsigned int close_time =
            GlobalData::s_revengeDungeonMgr->GetCloseTime();
        const int remaining_sec = close_time < cur_sec
                                      ? 0
                                      : static_cast<int>(close_time - cur_sec);

        for (int seat = 0; seat <= 3; ++seat) {
            CUser* member = members[seat];
            if (!member) {
                continue;
            }

            VillageAttackedFinishPayload payload;
            std::memset(&payload, 0, sizeof(payload));
            payload.remaining_seconds = remaining_sec;

            const ConditionEnterRevengeVector& conditions =
                G_CDataManager()->get_server_parameter_script()
                    ->get_condition_enter_revenge();
            const unsigned int attack_count =
                static_cast<unsigned char>(
                    member->GetCurVillageAttackCount());
            if (attack_count > 2) {
                payload.reward_item_id = 7745;
            }

            PacketGuard reward_packet;
            reward_packet.put_header(
                0, ENUM_NOTIPACKET_VILLAGE_ATTACKED_COMBINE);
            reward_packet.put_binary(
                reinterpret_cast<const char*>(&payload), sizeof(payload));
            reward_packet.finalize(true);
            member->Send(reward_packet);

            member->SetVillageAttackedReward(false);
            for (ConditionEnterRevengeVector::const_iterator it =
                     conditions.begin();
                 it != conditions.end();
                 ++it) {
                if (attack_count >=
                        static_cast<unsigned int>(it->minimum_attack_count) &&
                    attack_count <=
                        static_cast<unsigned int>(it->maximum_attack_count)) {
                    member->SetCurRevengeDungeonCount(
                        static_cast<unsigned char>(it->revenge_dungeon_count));
                    member->VillageAttack_DBUpdate();
                    break;
                }
            }
        }
    }

    return 0;
}
// IDA_EA: 0x85B6768
// IDA_NAME: _ZN6CParty17GetMapPlayingTimeEv
// DEMANGLED: CParty::GetMapPlayingTime(void)
uint64_t CParty::GetMapPlayingTime() {
    CSystemTime* system_time = GlobalData::system_time();
    const uint64_t cur_tick =
        static_cast<uint32_t>(system_time ? system_time->getCurTickCount() : 0);
    battle_data_.map_playing_time() =
        cur_tick - battle_data_.map_start_time() - battle_data_.map_pause_time();
    return battle_data_.map_playing_time();
}

// IDA_EA: 0x85A83A2
// IDA_NAME: _ZN6CParty19checkMoveMapHackCntER12MSG_MOVE_MAP
// DEMANGLED: CParty::checkMoveMapHackCnt(MSG_MOVE_MAP &)
int CParty::checkMoveMapHackCnt(MSG_MOVE_MAP& message) {
    int result = battle_data_.monster_drop_limit_candidate_count_;
    result = hit_time_divisor_;
    const int divisor_for_stats = result;
    hit_time_divisor_ = 0;
    battle_data_.monster_drop_limit_candidate_count_ = 0;

    CSystemTime* system_time = GlobalData::system_time();
    const uint64_t cur_tick =
        static_cast<uint32_t>(system_time ? system_time->getCurTickCount() : 0);
    battle_data_.map_playing_time() =
        cur_tick - battle_data_.map_start_time() - battle_data_.map_pause_time();

    result = battle_field_.IsVisitedCurrentMap() ^ 1;
    if (static_cast<unsigned char>(result)) {
        int total_hit_count = 0;
        const int divisor = divisor_for_stats;
        if (divisor) {
            const int map_hit_index = map_hit_statistics_.map_count_;
            int& current_hit =
                map_hit_statistics_.monster_field_a1e_[map_hit_index];
            current_hit /= divisor;
            map_hit_statistics_.total_monster_field_a1e_ += current_hit;

            int& current_time =
                map_hit_statistics_.monster_field_a1a_[map_hit_index];
            current_time /= divisor;
            map_hit_statistics_.total_monster_field_a1a_ += current_time;
        }

        for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
            const unsigned short hit_count = message.clear_counts[seat];
            if (hit_count != 0xffffu &&
                static_cast<unsigned char>(checkValidUser(seat)) &&
                (!battle_field_.get_dungeon() ||
                 static_cast<unsigned char>(
                     battle_field_.get_dungeon()->IsEnterEachMap()) != 1u ||
                 static_cast<signed char>(message.party_seat) == seat)) {
                CUser* member = get_user(seat);
                MapInfo* const last_map_info = battle_field_.GetLastMapInfo();
                CUser_history_log(member)->MapClear(
                    hit_count,
                    static_cast<int>(
                        battle_data_.map_playing_time() /
                        0x3E8u),
                    last_map_info->map_index_);

                if (!hit_count) {
                    reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->addServerHackCnt(
                        member,
                        static_cast<WongWork::ENUM_HACKTYPE>(411),
                        1,
                        0,
                        0);
                }

                const unsigned int playing_time_sec =
                    static_cast<unsigned int>(
                        battle_data_.map_playing_time() /
                        0x3E8u);
                if (playing_time_sec) {
                    const unsigned int hit_per_sec =
                        hit_count / playing_time_sec;
                    if (hit_per_sec > 4u) {
                        const unsigned int capped =
                            std::min<unsigned int>(hit_per_sec - 4u, 100u);
                        reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->addServerHackCnt(
                            member,
                            static_cast<WongWork::ENUM_HACKTYPE>(832),
                            capped,
                            0,
                            0);
                    }
                }

                total_hit_count += hit_count;
                CDungeon* const dungeon = battle_field_.get_dungeon();
                if (dungeon &&
                    dungeon->ancient_dungeon_ != 1u &&
                    static_cast<unsigned char>(
                        reinterpret_cast<const CDungeon*>(dungeon)->get_dimension_possible()) == 0u &&
                    divisor &&
                    map_hit_statistics_.has_a1e_tenth_multiple_ != 1u) {
                    ++member->move_map_hack_count_;
                    if (member->move_map_hack_count_ > 9) {
                        member->move_map_hack_count_ = 0;
                        reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->addServerHackCnt(
                            member,
                            static_cast<WongWork::ENUM_HACKTYPE>(220),
                            1,
                            0,
                            0);
                    }
                }
                map_hit_statistics_.has_a1e_tenth_multiple_ = 0;

                const unsigned int elapsed =
                    static_cast<unsigned int>(
                        battle_data_.map_playing_time() /
                        0x3E8u);
                int* clear_time = &skill_quest_use_counts_[seat];
                if (member->get_charac_level() > 19 &&
                    battle_data_.member_live(seat) &&
                    (!*clear_time ||
                     (!(elapsed % 5u) &&
                      static_cast<unsigned int>(*clear_time) <= elapsed / 5u) ||
                     ((elapsed % 5u) &&
                      static_cast<unsigned int>(*clear_time) < elapsed / 5u))) {
                    UserQuest* quest = member->getCurCharacQuestR();
                    if (static_cast<unsigned char>(
                            quest->IsDoingQuest(4307)) != 1u ||
                        battle_field_.get_dungeon_index() != 11 ||
                        battle_field_.get_dungeon_diff() != 2) {
                        quest = member->getCurCharacQuestR();
                        if (static_cast<unsigned char>(
                                quest->IsDoingQuest(4308)) != 1u ||
                            battle_field_.get_dungeon_index() != 12 ||
                            battle_field_.get_dungeon_diff() != 2) {
                            quest = member->getCurCharacQuestR();
                            if (static_cast<unsigned char>(
                                    quest->IsDoingQuest(4312)) != 1u ||
                                battle_field_.get_dungeon_index() != 15 ||
                                battle_field_.get_dungeon_diff() <= 0) {
                                const int etc =
                                    member->SetETC(132, *clear_time);
                                reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->addServerHackCnt(
                                    member,
                                    static_cast<WongWork::ENUM_HACKTYPE>(132),
                                    1,
                                    static_cast<unsigned int>(etc),
                                    0);
                            }
                        }
                    }
                }
                *clear_time = 0;

                if (!message.dword_4d &&
                    static_cast<unsigned char>(IsSinglePlay())) {
                    reinterpret_cast<WongWork::CHackAnalyzer*>((member->getHackAnalyzer()))->addServerHackCnt(
                        member,
                        static_cast<WongWork::ENUM_HACKTYPE>(416),
                        1,
                        0,
                        0);
                }
            }
        }

        const int map_hit_index = map_hit_statistics_.map_count_;
        map_hit_statistics_.client_hit_counts_[map_hit_index] = total_hit_count;
        map_hit_statistics_.total_client_hit_count_ += total_hit_count;
        result = map_hit_index;
        if (result <= 43) {
            ++map_hit_statistics_.map_count_;
            result = reinterpret_cast<int>(this);
        }
    }
    return result;
}

// Moved to src/old_game/group/cparty_dungeon_admission.cpp.
// Canonical provider: CParty::canFatigueUp(unsigned char), IDA_EA 0x85A8C88.

// IDA_EA: 0x85A8D1E
// IDA_NAME: _ZN6CParty21changePartyMemberInfoER12MSG_MOVE_MAPc
// DEMANGLED: CParty::changePartyMemberInfo(MSG_MOVE_MAP &,char)
int CParty::changePartyMemberInfo(MSG_MOVE_MAP& /*message*/, char seat) {
    int result = _checkValidUser(seat);
    if (static_cast<unsigned char>(result)) {
        if (battle_data_.member_live(seat) != 1u) {
            if (battle_data_.member_enter_map(seat) == 1u ||
                battle_data_.member_state_flag(seat) == 1u) {
                battle_data_.member_enter_map(seat) = 0;
            } else {
                ++battle_data_.member_map_move_count(seat);
            }
        }
        battle_data_.member_state_flag(seat) = 0;
        result = canFatigueUp(static_cast<unsigned char>(seat));
        if (static_cast<unsigned char>(result)) {
            CUser* const user = get_user(seat);
            return fatigue_acceleration::ConsumeFatigue(
                user,
                battle_field_.get_dungeon(),
                getDungIndex());
        }
    }
    return result;
}

// IDA_EA: 0x85A8E0E
// IDA_NAME: _ZN6CParty21resetFinishLoadingVarEv
// DEMANGLED: CParty::resetFinishLoadingVar(void)
bool CParty::resetFinishLoadingVar() {
    for (int seat = 0;; ++seat) {
        if (seat > 3) {
            return false;
        }
        battle_data_.member_finish_loading(seat) = 0;
    }
}

// IDA_EA: 0x85A8E3C
// IDA_NAME: _ZN6CParty30changePartyMemberInfoOnMoveMapER12MSG_MOVE_MAPb
// DEMANGLED: CParty::changePartyMemberInfoOnMoveMap(MSG_MOVE_MAP &,bool)
char CParty::changePartyMemberInfoOnMoveMap(MSG_MOVE_MAP& message,
                                            bool layered) {
    char result = static_cast<char>(layered);
    if (!layered) {
        CDungeon* const dungeon = battle_field_.get_dungeon();
        if (dungeon &&
            static_cast<unsigned char>(reinterpret_cast<const CDungeon*>(dungeon)->IsEnterEachMap())) {
            return static_cast<char>(
                changePartyMemberInfo(
                    message, static_cast<signed char>(message.party_seat)));
        }
        for (int seat = 0;; ++seat) {
            result = static_cast<char>(seat <= 3);
            if (seat > 3) {
                break;
            }
            changePartyMemberInfo(message, static_cast<char>(seat));
        }
    }
    return result;
}

// IDA_EA: 0x85A8EE0
// IDA_NAME: _ZN6CParty25changeBattleDataOnMoveMapEb
// DEMANGLED: CParty::changeBattleDataOnMoveMap(bool)
void* CParty::changeBattleDataOnMoveMap(bool layered) {
    void* const party = this;
    CSystemTime* system_time = GlobalData::system_time();
    const uint32_t cur_tick =
        static_cast<uint32_t>(system_time ? system_time->getCurTickCount() : 0);
    battle_data_.map_tick_216() = cur_tick;
    battle_data_.map_tick_217() = 0;
    battle_data_.map_start_time() = cur_tick;
    if (!layered) {
        ++battle_data_.map_move_count();
        return party;
    }
    return 0;
}

// IDA_EA: 0x85A8F4E
// IDA_NAME: _ZN6CParty8move_mapER12MSG_MOVE_MAP
// DEMANGLED: CParty::move_map(MSG_MOVE_MAP &)
int CParty::move_map(MSG_MOVE_MAP& message) {
    PacketGuard packet;

    if (state_ != 2u) {
        send_to_party_cmd_error(
            ENUM_CMDPACKET_MOVE_MAP, 19);
        return 0;
    }

    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (dungeon &&
        static_cast<unsigned char>(reinterpret_cast<const CDungeon*>(dungeon)->IsEnterEachMap())) {
        battle_field_.SetCurSeatNo(static_cast<signed char>(message.party_seat));
    } else if (static_cast<unsigned char>(
                   battle_field_.IsClearDungeon())) {
        send_to_party_cmd_error(
            ENUM_CMDPACKET_MOVE_MAP, 19);
        return 0;
    }

    int resolved_x = static_cast<signed char>(message.map_x);
    int resolved_y = static_cast<signed char>(message.map_y);
    const time_spiral::MoveRequestResolution time_spiral_resolution =
        time_spiral::ResolveMoveRequest(
            this,
            resolved_x,
            resolved_y,
            &resolved_x,
            &resolved_y);
    if (time_spiral_resolution == time_spiral::kMoveRequestRejected) {
        send_to_party_cmd_error(ENUM_CMDPACKET_MOVE_MAP, 19);
        return 0;
    }
    const int cur_pos_x = battle_field_.GetCurPosX();
    const int cur_pos_y = battle_field_.GetCurPosY();
    const int old_map_index =
        battle_field_.get_map_index(cur_pos_x, cur_pos_y);
    if (message.layered_map == 1u) {
        battle_field_.checkLayeredMap(
            resolved_x,
            resolved_y);
    }
    const int target_map_index = battle_field_.get_map_index(
        resolved_x,
        resolved_y);
    (void)target_map_index;

    CUser* hack_user = host_;
    secu_.CheckMoveMapFitInPathHackCnt(
        hack_user,
        this,
        message.dword_0f,
        message.dword_13,
        static_cast<signed char>(old_map_index & 0xff));

    bool layered = false;
    const int move_result = battle_field_.MoveMap(
        resolved_x,
        resolved_y,
        layered);
    if (move_result <= 0) {
        const zombie_map_runtime::HookResult zombie_commit =
            zombie_map_runtime::CommitMoveMap(
                this, resolved_x, resolved_y);
        if (zombie_commit == zombie_map_runtime::kHookRejected) {
            send_to_party_cmd_error(ENUM_CMDPACKET_MOVE_MAP, 19);
            return 0;
        }
        checkMoveMapHackCnt(message);
        if (dungeon->get_blood_dungeon_type() <= 0) {
            battle_field_.MakePacket(ENUM_NOTIPACKET_START_MAP, &packet);
        } else {
            battle_field_.MakePacket(211, &packet);
        }
        send_to_party(packet);
        check_passive_skill();
        send_loadingcheck_msg();
        resetFinishLoadingVar();
        changePartyMemberInfoOnMoveMap(message, layered);

        const int member_level_gap = _getMemberLevelGap();
        if (get_member_count() > 1) {
            if (member_level_gap <= 6) {
                GetInstanceCommonStatisticsMgr()->IncreaseQuantity(
                    CCommonStatisticsMgr::ENUM_HELPED_PARTY_COUNT, 3);
            } else {
                const char party_type =
                    static_cast<char>(GetPartyType());
                GetInstanceCommonStatisticsMgr()->IncreaseQuantity(
                    CCommonStatisticsMgr::ENUM_HELPED_PARTY_COUNT,
                    party_type);
            }
        }
        changeBattleDataOnMoveMap(layered);
        dual_raid::OnPartyMoveMap(
            this,
            static_cast<unsigned int>(resolved_x),
            static_cast<unsigned int>(resolved_y));
        time_spiral::ConsumeMoveSuccess(
            this,
            resolved_x,
            resolved_y);
        if (static_cast<unsigned char>(
                getDungeonDropTestFlag()) &&
            !layered) {
            battle_field_.GetCurrentMapInfo();
        }
        return 1;
    }

    send_to_party_cmd_error(
        ENUM_CMDPACKET_MOVE_MAP,
        static_cast<unsigned char>(move_result));
    return 0;
}

// IDA_EA: 0x85A932A
// IDA_NAME: _ZN6CParty19check_passive_skillEv
// DEMANGLED: CParty::check_passive_skill(void)
void CParty::check_passive_skill() {}

// IDA_EA: 0x85BDC76
// IDA_NAME: _ZN6CParty22getDungeonDropTestFlagEv
// DEMANGLED: CParty::getDungeonDropTestFlag(void)
int CParty::getDungeonDropTestFlag() {
    for (int seat = 0; seat < kPartyMaxMemberCount; ++seat) {
        CUser* user = get_user(seat);
        if (user && static_cast<unsigned char>(user->isGMUser()) &&
            static_cast<unsigned char>(getDungeonMapSaving())) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85BF930
// IDA_NAME: _ZN6CParty19getDungeonMapSavingEv
// DEMANGLED: CParty::getDungeonMapSaving(void)
int CParty::getDungeonMapSaving() {
    return dungeon_map_saving_;
}

// IDA_EA: 0x859D388
// IDA_NAME: _ZN6CParty19send_invite_warroomEi
// DEMANGLED: CParty::send_invite_warroom(int)
void CParty::send_invite_warroom(int unique_id) {
    PacketGuard packet;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 0u) {
            continue;
        }

        CUser* const member = members_[seat].user;
        packet.clear();
        packet.put_header(0, 7);
        packet.put_short(unique_id);
        packet.put_byte(3);
        packet.put_int(member->get_unique_id());
        packet.finalize(true);
        member->Send(packet);
    }
}

// IDA_EA: 0x85A3B6A
// IDA_NAME: _ZN6CParty18kill_blood_monsterEP5CUseriPt
// DEMANGLED: CParty::kill_blood_monster(CUser *,int,unsigned short *)
int CParty::kill_blood_monster(CUser* user,
                               int value,
                               unsigned short* result) {
    return (&battle_field_)->kill_blood_monster(user, value,
                                             result);
}
