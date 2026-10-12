#include "old_data/PVF/event/event_reward_script.h"

#include <stdint.h>


static_assert(sizeof(int) == sizeof(void*),
              "eventReward::StartEndPos targets the old i386 ABI");

// IDA_EA: 0x80F0D22
// IDA_NAME: _ZNK11eventReward11StartEndPosEN10Inven_Item9ITEM_TYPEERiS2_i
// DEMANGLED: eventReward::StartEndPos(Inven_Item::ITEM_TYPE,int &,int &,int)const
int eventReward::StartEndPos(Inven_Item::ITEM_TYPE item_type,
                             int& start,
                             int& end,
                             int capacity) const {
    int result =
        slot_bound_checker.get_slot_bound(item_type, capacity, start, end) ^ 1;
    if (static_cast<unsigned char>(result)) {
        start = 0;
        end = 0;
        result = static_cast<int>(reinterpret_cast<uintptr_t>(&end));
    }
    return result;
}
