#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/instant_reinforce.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85B29BC
// IDA_NAME: _ZN6CParty15set_charac_liveEP5CUser23ENUM_USER_DUNGEON_STATE
// DEMANGLED: CParty::set_charac_live(CUser *,ENUM_USER_DUNGEON_STATE)
void CParty::set_charac_live(CUser* user, ENUM_USER_DUNGEON_STATE state) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 && members_[seat].user == user) {
            const int previous_live = battle_data_.member_live(seat);
            if (state && state <= 2) {
                battle_data_.member_live(seat) = 1;
                battle_data_.member_enter_map(seat) = 1;
                battle_data_.member_state_flag(seat) = 1;
                if (state == 1) {
                    ++members_[seat].used_coin_count;
                }
            } else if (!state) {
                battle_data_.member_live(seat) = 0;
            }

            instant_reinforce::ObserveAuthoritativeLifeTransition(
                user, this, previous_live, static_cast<int>(state));

            PacketGuard packet;
            packet.put_header(0, 32);
            packet.put_short(user->get_unique_id());
            packet.put_byte(state);
            packet.finalize(true);
            send_to_party(packet);
            return;
        }
    }
}

// IDA_EA: 0x85B2B52
// IDA_NAME: _ZN6CParty19check_allmember_dieEv
// DEMANGLED: CParty::check_allmember_die(void)
int CParty::check_allmember_die() {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 && battle_data_.member_live(seat)) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x85B3520
// IDA_NAME: _ZN6CParty8SaveCoinE14eCoinSubReason
// DEMANGLED: CParty::SaveCoin(eCoinSubReason)
bool CParty::SaveCoin(eCoinSubReason reason) {
    (void)reason;
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1) {
            members_[seat].user->SaveMoneyCoin();
        }
    }
    return false;
}
