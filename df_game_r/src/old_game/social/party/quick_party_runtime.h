#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/social/party/party.h"

namespace QuickParty {

struct CQuickPartyLayoutChecks;

// IDA unit: 09_dungeon_pvp_party/QuickParty/CQuickParty.  This is the
// matchmaking record owned by the gameplay runtime, not the PVF-backed reward
// manager in dungeon_pvp_value_types.h.
class CQuickParty {
public:
    // IDA_EA: 0x826952C
    // IDA_NAME: _ZN10QuickParty11CQuickPartyC2Ev
    // DEMANGLED: QuickParty::CQuickParty::CQuickParty(void)
    CQuickParty();

    // IDA_EA: 0x82695AE
    // IDA_NAME: _ZN10QuickParty11CQuickPartyD2Ev
    // DEMANGLED: QuickParty::CQuickParty::~CQuickParty()
    ~CQuickParty();

    // IDA_EA: 0x8269546
    // IDA_NAME: _ZN10QuickParty11CQuickParty4initEv
    // DEMANGLED: QuickParty::CQuickParty::init(void)
    CQuickParty* init();

    // IDA_EA: 0x82695CA
    // IDA_NAME: _ZN10QuickParty11CQuickParty7destroyEv
    // DEMANGLED: QuickParty::CQuickParty::destroy(void)
    int destroy();

    // IDA_EA: 0x82696D0 / 0x826972C
    // IDA_NAME: _ZN10QuickParty11CQuickParty16get_party_objectEi
    //           _ZN10QuickParty11CQuickParty16get_party_objectEv
    CParty* get_party_object(int slot);
    CParty* get_party_object();

    // IDA_EA: 0x826978C / 0x82697C8
    // IDA_NAME: _ZN10QuickParty11CQuickParty15set_party_indexEi
    //           _ZN10QuickParty11CQuickParty15set_party_indexEii
    int set_party_index(int party_index);
    CQuickParty* set_party_index(int slot, int party_index);

    // IDA_EA: 0x8269668
    // IDA_NAME: _ZN10QuickParty11CQuickParty13get_timer_keyE13TIMER_MESSAGE
    // DEMANGLED: QuickParty::CQuickParty::get_timer_key(TIMER_MESSAGE)
    int get_timer_key(TIMER_MESSAGE message);

    // IDA_EA: 0x8269680
    // IDA_NAME: _ZN10QuickParty11CQuickParty13gen_timer_keyE13TIMER_MESSAGE
    // DEMANGLED: QuickParty::CQuickParty::gen_timer_key(TIMER_MESSAGE)
    int gen_timer_key(TIMER_MESSAGE message);

    // IDA_EA: 0x82696B0
    // IDA_NAME: _ZN10QuickParty11CQuickParty15check_timer_keyE13TIMER_MESSAGEi
    // DEMANGLED: QuickParty::CQuickParty::check_timer_key(TIMER_MESSAGE,int)
    bool check_timer_key(TIMER_MESSAGE message, int key);

    // IDA_EA: 0x82697EC
    // IDA_NAME: _ZN10QuickParty11CQuickParty15get_party_indexEi
    // DEMANGLED: QuickParty::CQuickParty::get_party_index(int)
    int get_party_index(int slot);

    // IDA_EA: 0x826980E
    // IDA_NAME: _ZN10QuickParty11CQuickParty18delete_party_indexEi
    // DEMANGLED: QuickParty::CQuickParty::delete_party_index(int)
    int delete_party_index(int party_index);

    // IDA_EA: 0x8269856
    // IDA_NAME: _ZN10QuickParty11CQuickParty18delete_party_indexEv
    bool delete_party_index();

    // IDA_EA: 0x8269884
    // IDA_NAME: _ZN10QuickParty11CQuickParty20is_empty_quick_partyEv
    // DEMANGLED: QuickParty::CQuickParty::is_empty_quick_party(void)
    int is_empty_quick_party();

    // IDA_EA: 0x82698BE / 0x8269936 / 0x8269A8E
    // IDA_NAME: _ZN10QuickParty11CQuickParty21get_quick_party_countEv
    //           _ZN10QuickParty11CQuickParty41get_matching_quick_party_master_unique_idEv
    //           _ZN10QuickParty11CQuickParty32send_matching_update_quick_partyEctb
    int get_quick_party_count();
    int get_matching_quick_party_master_unique_id();
    bool send_matching_update_quick_party(char quick_party_count,
                                          unsigned short master_uid,
                                          bool changed);

    // IDA_EA: 0x826B74E
    // IDA_NAME: _ZNK10QuickParty11CQuickParty21get_quick_party_indexEv
    // DEMANGLED: QuickParty::CQuickParty::get_quick_party_index(void)const
    int get_quick_party_index() const;

    // IDA_EA: 0x826B75A
    // IDA_NAME: _ZN10QuickParty11CQuickParty22get_first_matching_cntEv
    // DEMANGLED: QuickParty::CQuickParty::get_first_matching_cnt(void)
    int get_first_matching_cnt();

    // IDA_EA: 0x826B766
    // IDA_NAME: _ZN10QuickParty11CQuickParty22set_first_matching_cntEc
    // DEMANGLED: QuickParty::CQuickParty::set_first_matching_cnt(char)
    CQuickParty* set_first_matching_cnt(char count);

    // IDA_EA: 0x826B77E
    // IDA_NAME: _ZN10QuickParty11CQuickParty17set_dungeon_indexEs
    // DEMANGLED: QuickParty::CQuickParty::set_dungeon_index(short)
    CQuickParty* set_dungeon_index(int16_t dungeon_index);

    // IDA_EA: 0x826B798
    // IDA_NAME: _ZN10QuickParty11CQuickParty17get_dungeon_indexEv
    // DEMANGLED: QuickParty::CQuickParty::get_dungeon_index(void)
    int get_dungeon_index();

    // IDA_EA: 0x826B7A4
    // IDA_NAME: _ZN10QuickParty11CQuickParty16set_dungeon_diffEi
    // DEMANGLED: QuickParty::CQuickParty::set_dungeon_diff(int)
    CQuickParty* set_dungeon_diff(int difficulty);

    // IDA_EA: 0x826B7B4
    // IDA_NAME: _ZN10QuickParty11CQuickParty16get_dungeon_diffEv
    // DEMANGLED: QuickParty::CQuickParty::get_dungeon_diff(void)
    int get_dungeon_diff();

    // IDA_EA: 0x826B7C4
    // IDA_NAME: _ZN10QuickParty11CQuickParty14set_pool_indexEi
    // DEMANGLED: QuickParty::CQuickParty::set_pool_index(int)
    CQuickParty* set_pool_index(int pool_index);

    // IDA_EA: 0x826B7D4
    // IDA_NAME: _ZN10QuickParty11CQuickParty14get_pool_indexEv
    // DEMANGLED: QuickParty::CQuickParty::get_pool_index(void)
    int get_pool_index();

    // IDA_EA: 0x826B7E4
    // IDA_NAME: _ZN10QuickParty11CQuickParty27is_change_quick_party_countEv
    // DEMANGLED: QuickParty::CQuickParty::is_change_quick_party_count(void)
    int is_change_quick_party_count();

    // IDA_EA: 0x826B7F0
    // IDA_NAME: _ZN10QuickParty11CQuickParty28set_change_quick_party_countEb
    // DEMANGLED: QuickParty::CQuickParty::set_change_quick_party_count(bool)
    CQuickParty* set_change_quick_party_count(bool changed);

    // IDA_EA: 0x82A69BE
    // IDA_NAME: _ZN10QuickParty11CQuickParty21set_quick_party_indexEi
    // DEMANGLED: QuickParty::CQuickParty::set_quick_party_index(int)
    CQuickParty* set_quick_party_index(int quick_party_index);

private:
    int party_indices_[4];                 // +0x00
    int quick_party_index_;                // +0x10
    uint8_t first_matching_count_;         // +0x14
    uint8_t pad_15_;
    uint16_t dungeon_index_;               // +0x16
    int8_t dungeon_difficulty_;            // +0x18
    int8_t pool_index_;                    // +0x19
    uint8_t pad_1a_1b_[2];
    int timer_keys_[2];                    // +0x1C, messages 157..158
    uint8_t change_quick_party_count_;     // +0x24
    uint8_t pad_25_27_[3];

    friend struct CQuickPartyLayoutChecks;
};

struct CQuickPartyLayoutChecks {
    static_assert(offsetof(CQuickParty, timer_keys_) == 0x1cu,
                  "CQuickParty timer-key table must stay at +0x1c");
    static_assert(sizeof(CQuickParty) == 0x28u,
                  "CQuickParty IDA ABI size must stay 0x28");
};

}  // namespace QuickParty

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_RUNTIME_H_
