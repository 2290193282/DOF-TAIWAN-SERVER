#include "old_game/social/party/quick_party_runtime.h"

#include <string.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace QuickParty {

// IDA_EA: 0x826952C
// IDA_NAME: _ZN10QuickParty11CQuickPartyC2Ev
// DEMANGLED: QuickParty::CQuickParty::CQuickParty(void)
CQuickParty::CQuickParty() {
    init();
}

// IDA_EA: 0x82695AE
// IDA_NAME: _ZN10QuickParty11CQuickPartyD2Ev
// DEMANGLED: QuickParty::CQuickParty::~CQuickParty()
CQuickParty::~CQuickParty() {}

// IDA_EA: 0x8269546
// IDA_NAME: _ZN10QuickParty11CQuickParty4initEv
// DEMANGLED: QuickParty::CQuickParty::init(void)
CQuickParty* CQuickParty::init() {
    memset(party_indices_, -1, sizeof(party_indices_));
    quick_party_index_ = -1;
    first_matching_count_ = 0;
    dungeon_index_ = 0;
    dungeon_difficulty_ = -1;
    pool_index_ = -1;
    timer_keys_[0] = 0;
    timer_keys_[1] = 0;
    change_quick_party_count_ = 0;
    return this;
}

// IDA_EA: 0x82695CA
// IDA_NAME: _ZN10QuickParty11CQuickParty7destroyEv
// DEMANGLED: QuickParty::CQuickParty::destroy(void)
int CQuickParty::destroy() {
    G_CGameManager()->PutQuickParty(this);
    return 0;
}

// IDA_EA: 0x82696D0
// IDA_NAME: _ZN10QuickParty11CQuickParty16get_party_objectEi
// DEMANGLED: QuickParty::CQuickParty::get_party_object(int)
CParty* CQuickParty::get_party_object(int slot) {
    if (slot < 0 || slot >= 4 || party_indices_[slot] == -1) {
        return 0;
    }
    return G_CGameManager()->GetParty(party_indices_[slot]);
}

// IDA_EA: 0x826972C
// IDA_NAME: _ZN10QuickParty11CQuickParty16get_party_objectEv
// DEMANGLED: QuickParty::CQuickParty::get_party_object(void)
CParty* CQuickParty::get_party_object() {
    for (int slot = 0; slot <= 3; ++slot) {
        CParty* const party = get_party_object(slot);
        if (party) {
            return party;
        }
    }
    return 0;
}

// IDA_EA: 0x826978C
// IDA_NAME: _ZN10QuickParty11CQuickParty15set_party_indexEi
// DEMANGLED: QuickParty::CQuickParty::set_party_index(int)
int CQuickParty::set_party_index(int party_index) {
    for (int slot = 0; slot <= 3; ++slot) {
        if (party_indices_[slot] == -1) {
            party_indices_[slot] = party_index;
            return reinterpret_cast<intptr_t>(this);
        }
    }
    return 0;
}

// IDA_EA: 0x82697C8
// IDA_NAME: _ZN10QuickParty11CQuickParty15set_party_indexEii
// DEMANGLED: QuickParty::CQuickParty::set_party_index(int,int)
CQuickParty* CQuickParty::set_party_index(int slot, int party_index) {
    if (slot >= 0 && slot < 4) {
        party_indices_[slot] = party_index;
    }
    return this;
}

// IDA_EA: 0x8269668
// IDA_NAME: _ZN10QuickParty11CQuickParty13get_timer_keyE13TIMER_MESSAGE
// DEMANGLED: QuickParty::CQuickParty::get_timer_key(TIMER_MESSAGE)
int CQuickParty::get_timer_key(TIMER_MESSAGE message) {
    return timer_keys_[static_cast<int>(message) - 157];
}

// IDA_EA: 0x8269680
// IDA_NAME: _ZN10QuickParty11CQuickParty13gen_timer_keyE13TIMER_MESSAGE
// DEMANGLED: QuickParty::CQuickParty::gen_timer_key(TIMER_MESSAGE)
int CQuickParty::gen_timer_key(TIMER_MESSAGE message) {
    return ++timer_keys_[static_cast<int>(message) - 157];
}

// IDA_EA: 0x82696B0
// IDA_NAME: _ZN10QuickParty11CQuickParty15check_timer_keyE13TIMER_MESSAGEi
// DEMANGLED: QuickParty::CQuickParty::check_timer_key(TIMER_MESSAGE,int)
bool CQuickParty::check_timer_key(TIMER_MESSAGE message, int key) {
    return timer_keys_[static_cast<int>(message) - 157] == key;
}

// IDA_EA: 0x82697EC
// IDA_NAME: _ZN10QuickParty11CQuickParty15get_party_indexEi
// DEMANGLED: QuickParty::CQuickParty::get_party_index(int)
int CQuickParty::get_party_index(int slot) {
    return static_cast<unsigned int>(slot) < 4u ? party_indices_[slot] : -1;
}

// IDA_EA: 0x826980E
// IDA_NAME: _ZN10QuickParty11CQuickParty18delete_party_indexEi
// DEMANGLED: QuickParty::CQuickParty::delete_party_index(int)
int CQuickParty::delete_party_index(int party_index) {
    for (int slot = 0; slot < 4; ++slot) {
        if (party_indices_[slot] == party_index) {
            party_indices_[slot] = -1;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8269856
// IDA_NAME: _ZN10QuickParty11CQuickParty18delete_party_indexEv
// DEMANGLED: QuickParty::CQuickParty::delete_party_index(void)
bool CQuickParty::delete_party_index() {
    memset(party_indices_, -1, sizeof(party_indices_));
    return false;
}

// IDA_EA: 0x8269884
// IDA_NAME: _ZN10QuickParty11CQuickParty20is_empty_quick_partyEv
// DEMANGLED: QuickParty::CQuickParty::is_empty_quick_party(void)
int CQuickParty::is_empty_quick_party() {
    for (int slot = 0; slot < 4; ++slot) {
        if (party_indices_[slot] != -1)
            return 0;
    }
    return 1;
}

// IDA_EA: 0x82698BE
// IDA_NAME: _ZN10QuickParty11CQuickParty21get_quick_party_countEv
// DEMANGLED: QuickParty::CQuickParty::get_quick_party_count(void)
int CQuickParty::get_quick_party_count() {
    int count = 0;
    for (int slot = 0; slot <= 3; ++slot) {
        CParty* const party = get_party_object(slot);
        if (party) {
            count += party->get_member_count();
        }
    }
    return count;
}

// IDA_EA: 0x8269936
// IDA_NAME: _ZN10QuickParty11CQuickParty41get_matching_quick_party_master_unique_idEv
// DEMANGLED: QuickParty::CQuickParty::get_matching_quick_party_master_unique_id(void)
int CQuickParty::get_matching_quick_party_master_unique_id() {
    int selected_slot = -1;
    for (int slot = 0; slot <= 3; ++slot) {
        CParty* const party = get_party_object(slot);
        if (!party) {
            continue;
        }
        if (static_cast<signed char>(party->GetEPLPState()) > 1) {
            selected_slot = slot;
            break;
        }
        if (party->is_quick_party()) {
            selected_slot = slot;
        }
    }
    if (selected_slot < 0) {
        for (int slot = 0; slot <= 3; ++slot) {
            if (get_party_object(slot)) {
                selected_slot = slot;
                break;
            }
        }
    }
    CParty* const party =
        selected_slot >= 0 ? get_party_object(selected_slot) : 0;
    return party ? static_cast<unsigned short>(party->getManager()->get_unique_id())
                 : 0xffff;
}

// IDA_EA: 0x8269A8E
// IDA_NAME: _ZN10QuickParty11CQuickParty32send_matching_update_quick_partyEctb
// DEMANGLED: QuickParty::CQuickParty::send_matching_update_quick_party(char,unsigned short,bool)
bool CQuickParty::send_matching_update_quick_party(char quick_party_count,
                                                   unsigned short master_uid,
                                                   bool changed) {
    for (int slot = 0; slot <= 3; ++slot) {
        CParty* const party = get_party_object(slot);
        if (!party) {
            continue;
        }
        PacketGuard packet;
        packet.put_header(0, 384);
        packet.put_byte(quick_party_count);
        packet.put_short(master_uid);
        packet.put_byte(changed ? 1 : 0);
        packet.finalize(true);
        party->send_to_party(packet);
    }
    return false;
}

// IDA_EA: 0x826B74E
// IDA_NAME: _ZNK10QuickParty11CQuickParty21get_quick_party_indexEv
// DEMANGLED: QuickParty::CQuickParty::get_quick_party_index(void)const
int CQuickParty::get_quick_party_index() const {
    return quick_party_index_;
}

// IDA_EA: 0x826B75A
// IDA_NAME: _ZN10QuickParty11CQuickParty22get_first_matching_cntEv
// DEMANGLED: QuickParty::CQuickParty::get_first_matching_cnt(void)
int CQuickParty::get_first_matching_cnt() {
    return first_matching_count_;
}

// IDA_EA: 0x826B766
// IDA_NAME: _ZN10QuickParty11CQuickParty22set_first_matching_cntEc
// DEMANGLED: QuickParty::CQuickParty::set_first_matching_cnt(char)
CQuickParty* CQuickParty::set_first_matching_cnt(char count) {
    first_matching_count_ = static_cast<uint8_t>(count);
    return this;
}

// IDA_EA: 0x826B77E
// IDA_NAME: _ZN10QuickParty11CQuickParty17set_dungeon_indexEs
// DEMANGLED: QuickParty::CQuickParty::set_dungeon_index(short)
CQuickParty* CQuickParty::set_dungeon_index(int16_t dungeon_index) {
    dungeon_index_ = static_cast<uint16_t>(dungeon_index);
    return this;
}

// IDA_EA: 0x826B798
// IDA_NAME: _ZN10QuickParty11CQuickParty17get_dungeon_indexEv
// DEMANGLED: QuickParty::CQuickParty::get_dungeon_index(void)
int CQuickParty::get_dungeon_index() {
    return dungeon_index_;
}

// IDA_EA: 0x826B7A4
// IDA_NAME: _ZN10QuickParty11CQuickParty16set_dungeon_diffEi
// DEMANGLED: QuickParty::CQuickParty::set_dungeon_diff(int)
CQuickParty* CQuickParty::set_dungeon_diff(int difficulty) {
    dungeon_difficulty_ = static_cast<int8_t>(difficulty);
    return this;
}

// IDA_EA: 0x826B7B4
// IDA_NAME: _ZN10QuickParty11CQuickParty16get_dungeon_diffEv
// DEMANGLED: QuickParty::CQuickParty::get_dungeon_diff(void)
int CQuickParty::get_dungeon_diff() {
    return dungeon_difficulty_;
}

// IDA_EA: 0x826B7C4
// IDA_NAME: _ZN10QuickParty11CQuickParty14set_pool_indexEi
// DEMANGLED: QuickParty::CQuickParty::set_pool_index(int)
CQuickParty* CQuickParty::set_pool_index(int pool_index) {
    pool_index_ = static_cast<int8_t>(pool_index);
    return this;
}

// IDA_EA: 0x826B7D4
// IDA_NAME: _ZN10QuickParty11CQuickParty14get_pool_indexEv
// DEMANGLED: QuickParty::CQuickParty::get_pool_index(void)
int CQuickParty::get_pool_index() {
    return pool_index_;
}

// IDA_EA: 0x826B7E4
// IDA_NAME: _ZN10QuickParty11CQuickParty27is_change_quick_party_countEv
// DEMANGLED: QuickParty::CQuickParty::is_change_quick_party_count(void)
int CQuickParty::is_change_quick_party_count() {
    return change_quick_party_count_;
}

// IDA_EA: 0x826B7F0
// IDA_NAME: _ZN10QuickParty11CQuickParty28set_change_quick_party_countEb
// DEMANGLED: QuickParty::CQuickParty::set_change_quick_party_count(bool)
CQuickParty* CQuickParty::set_change_quick_party_count(bool changed) {
    change_quick_party_count_ = changed;
    return this;
}

// IDA_EA: 0x82A69BE
// IDA_NAME: _ZN10QuickParty11CQuickParty21set_quick_party_indexEi
// DEMANGLED: QuickParty::CQuickParty::set_quick_party_index(int)
CQuickParty* CQuickParty::set_quick_party_index(int quick_party_index) {
    quick_party_index_ = quick_party_index;
    return this;
}

}  // namespace QuickParty
