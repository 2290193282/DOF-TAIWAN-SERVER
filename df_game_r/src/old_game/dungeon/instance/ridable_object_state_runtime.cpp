#include "old_game/pvp/battlefield/battle_field.h"

#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

// IDA_EA: 0x82FE9C8
// IDA_NAME: _ZN8CRidable24CheckAppearRidableObjectEP6CPartyiiP11PacketGuard
// DEMANGLED: CRidable::CheckAppearRidableObject(CParty *,int,int,PacketGuard *)
int CRidable::CheckAppearRidableObject(CParty*,
                                        int pos_x,
                                        int pos_y,
                                        PacketGuard* packet) {
    int count = 0;
    int count_index = packet->get_index();
    packet->put_byte(count);

    for (std::map<int, DungeonRidableObject>::iterator it = begin();
         it != end();
         ++it) {
        DungeonRidableObject& object = it->second;
        if (object.value_00 != pos_x || object.value_04 != pos_y ||
            (object.state != 4 && !(object.state == 3 && reveal_pending_))) {
            continue;
        }

        object.state = 1;
        ++count;
        packet->put_int(object.value_0c);
        packet->put_int(object.value_10);
        packet->put_int(object.value_08);
        packet->put_int(object.value_14);
    }
    return packet->put_byte(count_index, count);
}

// IDA_EA: 0x82FEC2C
// IDA_NAME: _ZN8CRidable12GetObjectPosERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CRidable::GetObjectPos(std::vector<std::pair<int,int>> &)
int CRidable::GetObjectPos(std::vector<std::pair<int, int> >& positions) {
    for (std::map<int, DungeonRidableObject>::const_iterator it = begin();
         it != end();
         ++it) {
        positions.push_back(std::make_pair(it->second.value_00,
                                           it->second.value_04));
    }
    return static_cast<int>(positions.size());
}

// IDA_EA: 0x830EA0A
// IDA_NAME: _ZN8CRidable12GetObjectCntEv
// DEMANGLED: CRidable::GetObjectCnt(void)
int CRidable::GetObjectCnt() {
    return static_cast<int>(size());
}

// IDA_EA: 0x82FEB5E
// IDA_NAME: _ZN8CRidable21SetRidableObjectStateEi19eRidableObjectState
// DEMANGLED: CRidable::SetRidableObjectState(int,eRidableObjectState)
int CRidable::SetRidableObjectState(int object_id,
                                    eRidableObjectState requested_state) {
    std::map<int, DungeonRidableObject>::iterator found = find(object_id);
    if (found == end()) {
        return 0;
    }

    int& current_state = found->second.state;
    if (current_state == 0 || current_state == 4 || current_state == 3 ||
        current_state > 4) {
        return 0;
    }

    current_state = static_cast<int>(requested_state);
    return 1;
}

// IDA_EA: 0x85BF228
// IDA_NAME: _ZN13CBattle_Field21SetRidableObjectStateEi19eRidableObjectState
// DEMANGLED: CBattle_Field::SetRidableObjectState(int,eRidableObjectState)
int CBattle_Field::SetRidableObjectState(
    int object_id,
    eRidableObjectState requested_state) {
    return ridable_.SetRidableObjectState(object_id, requested_state);
}
