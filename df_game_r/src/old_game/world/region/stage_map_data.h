#ifndef OLDSRV_CHARACTER_SELECT_STAGE_MAP_DATA_H_
#define OLDSRV_CHARACTER_SELECT_STAGE_MAP_DATA_H_

namespace stagemap {

// IDA unit: 04_character_select_create/stagemap/SlotType.
struct SlotType {
    SlotType();

    SlotType* clear();

    int slots[5];
};

}  // namespace stagemap

#endif  // OLDSRV_CHARACTER_SELECT_STAGE_MAP_DATA_H_
