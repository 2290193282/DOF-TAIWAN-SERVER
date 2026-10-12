#include "old_game/player/core/cuser.h"

#include <cstring>

#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_data/db/account/login_payloads.h"
#include "old_game/activity/schedule/character_event_records.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8691BAE
// IDA_NAME: _ZN5CUser24SetGrowthWeaponEventInfoERK23SIG_GROWTH_WEAPON_EVENT
// DEMANGLED: CUser::SetGrowthWeaponEventInfo(SIG_GROWTH_WEAPON_EVENT const&)
void CUser::SetGrowthWeaponEventInfo(
    const SIG_GROWTH_WEAPON_EVENT& event_info) {
    growth_weapon_event_.enabled_ = event_info.enabled;
    growth_weapon_event_.charac_no_ = event_info.charac_no;
    growth_weapon_event_.event_value_ = event_info.weapon_id;
    growth_weapon_event_.timepiece_ = event_info.timepiece;
    growth_weapon_event_.infinity_weapon_id_ = event_info.infinity_weapon_id;
    growth_weapon_event_.server_group_ = event_info.server_id;

    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(100);
    if (event->IsEventing(0) == 1 && growth_weapon_event_.enabled_ != 1) {
        PacketGuard packet;
        packet.put_header(0, 442);
        packet.finalize(true);
        Send(packet);
    }
}

// IDA_EA: 0x8691CDE
// IDA_NAME: _ZN5CUser26SetGrowthCreatureEventInfoERK30SIG_GIVE_GROWTH_CREATURE_EVENT
// DEMANGLED: CUser::SetGrowthCreatureEventInfo(SIG_GIVE_GROWTH_CREATURE_EVENT const&)
void CUser::SetGrowthCreatureEventInfo(
    const SIG_GIVE_GROWTH_CREATURE_EVENT& event_info) {
    for (int index = 0; index <= 10; ++index) {
        const SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& source =
            event_info.entries[index];
        if (source.job == 0 || source.charac_no == 0) {
            break;
        }

        GrowthCreatureEvent::eventCharacInfo destination;
        destination.job = source.job;
        destination.charac_no = source.charac_no;
        destination.creature_id = source.creature_id;
        std::memcpy(destination.creature_name,
                    source.creature_name,
                    sizeof(destination.creature_name));
        growth_creature_event_.event_characters.push_back(destination);
    }
}

// IDA_EA: 0x8691E6A
// IDA_NAME: _ZN5CUser13sendEventInfoEv
// DEMANGLED: CUser::sendEventInfo(void)
void CUser::sendEventInfo() {
    // The old body is exactly prologue/epilogue.  Preserve this named ABI so
    // Inter_LoginInfo can call the original no-op event notification point.
}
