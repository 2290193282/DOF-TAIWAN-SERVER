#include "old_game/social/party/party.h"

#include "common/manager/global_data.h"
#include "common/manager/history_log_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85A7828
// IDA_NAME: _ZN6CParty8die_userEP5CUser
// DEMANGLED: CParty::die_user(CUser *)
void CParty::die_user(CUser* user) {
    int member_slot = -1;
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 && members_[seat].user == user) {
            ++user->character_statistic_.rating_storage_;
            member_slot = seat;
            HistoryLog::WriteDie(user->history_stream());
            break;
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_DIE_CHARACTER);
    packet.put_byte(0);
    if (member_slot == -1) {
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    if (battle_data_.member_live(member_slot) == 1) {
        set_charac_live(user, static_cast<ENUM_USER_DUNGEON_STATE>(0));
        if (user->GetRevivalTime() > GlobalData::system_time()->getCurSec()) {
            set_charac_live(user, static_cast<ENUM_USER_DUNGEON_STATE>(2));
            user->SetRevivalTime(0);
        }
        user->SetUseAPCPotionTime(0);
        if (check_allmember_die() && tournament_victory_ != 1 &&
            battle_field_.instance_dungeon_mode() != 1) {
            send_dungeon_fail_msg();
        }
        return;
    }

    packet.put_byte(18);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85B8E5C
// IDA_NAME: _ZN6CParty18BloodHandleDieUserEP5CUser
// DEMANGLED: CParty::BloodHandleDieUser(CUser *)
int CParty::BloodHandleDieUser(CUser* user) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) == 1 && get_user(seat) == user) {
            if (battle_data_.member_live(seat) != 1) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_DIE_CHARACTER, 18);
                return 0;
            }
            set_charac_live(user, static_cast<ENUM_USER_DUNGEON_STATE>(0));
            break;
        }
    }

    if (check_allmember_die() && battle_field_.instance_dungeon_mode() != 1) {
        const int timer_key = gen_timer_key(static_cast<TIMER_MESSAGE>(34));
        G_TimerQueue()->InsertTimer(1,
                                    GetPartyIndex(),
                                    34,
                                    10,
                                    static_cast<unsigned int>(timer_key),
                                    34);
    }
    return 1;
}
