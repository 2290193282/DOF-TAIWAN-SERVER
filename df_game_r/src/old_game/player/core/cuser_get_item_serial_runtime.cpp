#include "old_game/player/core/cuser.h"

#include "old_game/social/party/party.h"

// IDA_EA: 0x823066A
// IDA_NAME: _ZN5CUser20GetItemSerialCheckerEtti
// DEMANGLED: CUser::GetItemSerialChecker(unsigned short,unsigned short,int)
int CUser::GetItemSerialChecker(unsigned short x,
                                unsigned short y,
                                int unique_id) {
    const short signed_x = static_cast<short>(x);
    const short signed_y = static_cast<short>(y);

    if (signed_x - item_serial_check_.previous_x < -23 ||
        signed_x - item_serial_check_.previous_x > 23 ||
        signed_y - item_serial_check_.previous_y < -17 ||
        signed_y - item_serial_check_.previous_y > 17) {
        item_serial_check_.serial_count = 0;
        item_serial_check_.previous_x = signed_x;
        item_serial_check_.previous_y = signed_y;
        return item_serial_check_.serial_count;
    }

    CParty* const party = GetParty();
    if (party && party->isMonsterDrop_item(unique_id)) {
        const CBattle_Field& field = party->battle_field();
        if (field.monster_drop_event_flag() != 1 ||
            field.monster_drop_source_map() !=
                const_cast<CBattle_Field&>(field).get_cur_map()) {
            ++item_serial_check_.serial_count;
        }
    }
    return item_serial_check_.serial_count;
}
