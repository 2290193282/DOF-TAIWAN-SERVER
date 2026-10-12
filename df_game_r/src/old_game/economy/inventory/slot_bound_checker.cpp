#include "old_game/economy/inventory/slot_bound_checker.h"

#include <utility>


CSlotBoundChecker g_slotBoundChecker;

// IDA_EA: 0x80F1028
// IDA_NAME: _ZNK12SlotBoundKeyltERKS_
// DEMANGLED: SlotBoundKey::operator<(SlotBoundKey const&) const
bool SlotBoundKey::operator<(const SlotBoundKey& rhs) const {
    if (item_id < rhs.item_id) {
        return true;
    }
    return item_id == rhs.item_id && item_type < rhs.item_type;
}

// IDA_EA: 0x80F1074
// IDA_NAME: _ZN12SlotBoundKeyC2EtN10Inven_Item9ITEM_TYPEE
// DEMANGLED: SlotBoundKey::SlotBoundKey(unsigned short,Inven_Item::ITEM_TYPE)
SlotBoundKey::SlotBoundKey(unsigned short item_id_value,
                           Inven_Item::ITEM_TYPE item_type_value)
    : item_id(item_id_value),
      item_type(item_type_value) {}

// IDA_EA: 0x837351C
// IDA_NAME: _ZN9SlotBoundC2Eiib
// DEMANGLED: SlotBound::SlotBound(int,int,bool)
SlotBound::SlotBound(int first_value, int second_value, bool flag_value)
    : first(first_value),
      second(second_value),
      flag(flag_value) {}

// IDA_EA: 0x80F1096
// IDA_NAME: _ZNK17CSlotBoundChecker14get_slot_boundEN10Inven_Item9ITEM_TYPEEiRiS2_
// DEMANGLED: CSlotBoundChecker::get_slot_bound(Inven_Item::ITEM_TYPE,int,int&,int&) const
int CSlotBoundChecker::get_slot_bound(Inven_Item::ITEM_TYPE item_type,
                                      int item_id,
                                      int& first,
                                      int& second) const {
    SlotBoundKey key(static_cast<unsigned short>(item_id), item_type);
    std::map<SlotBoundKey, SlotBound>::const_iterator it =
        slot_bounds.find(key);
    if (it == slot_bounds.end()) {
        return 0;
    }
    first = it->second.first;
    second = it->second.second;
    return 1;
}

// IDA_EA: 0x837355A
// IDA_NAME: _ZN17CSlotBoundCheckerC2Ev
// DEMANGLED: CSlotBoundChecker::CSlotBoundChecker(void)
CSlotBoundChecker::CSlotBoundChecker()
    : slot_bounds() {
    init();
}

// IDA_EA: 0x837359C
// IDA_NAME: _ZN17CSlotBoundChecker6insertEiiN10Inven_Item9ITEM_TYPEEbb
// DEMANGLED: CSlotBoundChecker::insert(int,int,Inven_Item::ITEM_TYPE,bool,bool)
bool CSlotBoundChecker::insert(int first,
                               int second,
                               Inven_Item::ITEM_TYPE item_type,
                               bool shifted_second,
                               bool flag) {
    bool result = false;
    for (int item_id = 0;; item_id += 8) {
        result = item_id <= 16;
        if (item_id > 16) {
            break;
        }

        const int bounded_second =
            shifted_second ? item_id + second - 16 : second;
        SlotBound bound(first, bounded_second, flag);
        SlotBoundKey key(static_cast<unsigned short>(item_id), item_type);
        slot_bounds.insert(std::make_pair(key, bound));
    }
    return result;
}

// IDA_EA: 0x8373674
// IDA_NAME: _ZN17CSlotBoundChecker4initEv
// DEMANGLED: CSlotBoundChecker::init(void)
bool CSlotBoundChecker::init() {
    insert(3, 8, static_cast<Inven_Item::ITEM_TYPE>(0), false, true);
    insert(9, 64, static_cast<Inven_Item::ITEM_TYPE>(1), true, true);
    insert(65, 120, static_cast<Inven_Item::ITEM_TYPE>(2), true, true);
    insert(121, 176, static_cast<Inven_Item::ITEM_TYPE>(3), true, true);
    insert(177, 232, static_cast<Inven_Item::ITEM_TYPE>(4), true, true);
    insert(233, 288, static_cast<Inven_Item::ITEM_TYPE>(10), true, true);
    insert(289, 351, static_cast<Inven_Item::ITEM_TYPE>(9), false, true);
    insert(0, 139, static_cast<Inven_Item::ITEM_TYPE>(5), false, false);
    insert(140, 188, static_cast<Inven_Item::ITEM_TYPE>(6), false, false);
    return insert(189, 237, static_cast<Inven_Item::ITEM_TYPE>(7), false,
                  false);
}

// IDA_EA: 0x837388E
// IDA_NAME: _ZN17CSlotBoundCheckerD2Ev
// DEMANGLED: CSlotBoundChecker::~CSlotBoundChecker()
CSlotBoundChecker::~CSlotBoundChecker() {}

// IDA_EA: 0x850D1A0
// IDA_NAME: _ZNK17CSlotBoundChecker13get_item_typeEiiRN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CSlotBoundChecker::get_item_type(int,int,Inven_Item::ITEM_TYPE&) const
int CSlotBoundChecker::get_item_type(
    int slot,
    int item_id,
    Inven_Item::ITEM_TYPE& item_type) const {
    std::map<SlotBoundKey, SlotBound>::const_iterator it = slot_bounds.begin();
    while (it != slot_bounds.end()) {
        if (it->second.flag && it->first.item_id == item_id &&
            it->second.first <= slot && it->second.second >= slot) {
            item_type = it->first.item_type;
            return 1;
        }
        ++it;
    }
    return 0;
}
