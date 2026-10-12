#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x85056CA
// IDA_NAME: _ZN10CInventory19erase_repeated_itemERKSt6vectorISt4pairIiiESaIS2_EERS4_
// DEMANGLED: CInventory::erase_repeated_item(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
int CInventory::erase_repeated_item(
    const std::vector<std::pair<int, int> >& requested,
    std::vector<std::pair<int, int> >& remaining) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             requested.begin();
         it != requested.end();
         ++it) {
        const int item_id = it->first;
        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start, end);

        int residual_count = it->second;
        for (int slot = start; slot <= end; ++slot) {
            if (IsCreatureItemType(item_type)) {
                const Inven_Item* const item = GetInvenRef(3, slot);
                if (item->getKey() == item_id) {
                    if (isEquipableItemType(item_type)) {
                        --residual_count;
                    } else {
                        residual_count -= item->get_add_info();
                    }
                }
            } else {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item->getKey() == item_id) {
                    // The original branch is the exact stored ITEM_TYPE == 1
                    // test.  The canonical Inven_Item predicates express that
                    // same value without reopening CInventory's raw storage.
                    if (item->isEquipableItemType() &&
                        !item->IsCreatureItemType() &&
                        !item->isAvatarItemType()) {
                        --residual_count;
                    } else {
                        residual_count -= item->get_add_info();
                    }
                }
            }
        }

        // IDA tests the original request, not the calculated residual: it
        // intentionally retains zero and negative residual entries here.
        if (it->second > 0) {
            remaining.push_back(std::make_pair(item_id, residual_count));
        }
    }
    return static_cast<int>(remaining.size());
}

// IDA_EA: 0x8508536
// IDA_NAME: _ZNK10CInventory12CalBlankSlotEPiRKSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: CInventory::CalBlankSlot(int *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)const
bool CInventory::CalBlankSlot(
    int* blank_slots,
    const std::vector<std::pair<int, int> >& requested) const {
    if (requested.empty()) {
        return true;
    }

    for (std::vector<std::pair<int, int> >::const_iterator it =
             requested.begin();
         it != requested.end();
         ++it) {
        const int item_id = it->first;
        const unsigned int requested_count =
            static_cast<unsigned int>(it->second);
        if (!item_id || item_id == 2) {
            continue;
        }

        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start, end);

        if (item_type == 1) {
            unsigned int matched_count = 0;
            for (int slot = start; slot <= end; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item->getKey() == item_id) {
                    ++matched_count;
                    ++blank_slots[item_type];
                    if (matched_count == requested_count) {
                        break;
                    }
                }
            }
            if (matched_count < requested_count) {
                for (int slot = 3; slot <= 8; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (item->getKey() == item_id) {
                        ++matched_count;
                        ++blank_slots[0];
                        if (matched_count == requested_count) {
                            break;
                        }
                    }
                }
            }
        } else if (IsCreatureItemType(item_type)) {
            // IDA reads the normal inventory table in this branch even for a
            // creature item type.  Preserve that table selection exactly.
            if (isEquipableItemType(item_type)) {
                unsigned int matched_count = 0;
                for (int slot = start; slot <= end; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (item->getKey() == item_id) {
                        ++matched_count;
                        ++blank_slots[item_type];
                        if (matched_count == requested_count) {
                            break;
                        }
                    }
                }
            } else {
                for (int slot = start; slot <= end; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (item->getKey() == item_id &&
                        static_cast<unsigned int>(item->get_add_info()) ==
                            requested_count) {
                        ++blank_slots[item_type];
                        break;
                    }
                }
            }
        } else {
            bool found_in_inventory = false;
            for (int slot = start; slot <= end; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item->getKey() == item_id &&
                    static_cast<unsigned int>(item->get_add_info()) ==
                        requested_count) {
                    found_in_inventory = true;
                    ++blank_slots[item_type];
                    break;
                }
            }
            if (!found_in_inventory) {
                for (int slot = 3; slot <= 8; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (item->getKey() == item_id &&
                        static_cast<unsigned int>(item->get_add_info()) ==
                            requested_count) {
                        ++blank_slots[0];
                        break;
                    }
                }
            }
        }
    }
    return false;
}

// IDA_EA: 0x850891A
// IDA_NAME: _ZNK10CInventory11CalNeedSlotEPiRKSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: CInventory::CalNeedSlot(int *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)const
bool CInventory::CalNeedSlot(
    int* needed_slots,
    const std::vector<std::pair<int, int> >& requested) const {
    if (requested.empty()) {
        return true;
    }

    for (std::vector<std::pair<int, int> >::const_iterator it =
             requested.begin();
         it != requested.end();
         ++it) {
        const int item_id = it->first;
        const int requested_count = it->second;
        if (!item_id) {
            continue;
        }

        const int item_type = GetItemType(item_id);
        if (!item_type) {
            continue;
        }

        if (item_type == 1) {
            needed_slots[item_type] += requested_count;
        } else if (IsCreatureItemType(item_type)) {
            if (isEquipableItemType(item_type)) {
                needed_slots[item_type] += requested_count;
            } else {
                int start = 0;
                int end = 0;
                StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type),
                            start, end);

                bool found_in_creature_inventory = false;
                for (int slot = start; slot <= end; ++slot) {
                    const Inven_Item* const item = GetInvenRef(3, slot);
                    if (item->getKey() == item_id) {
                        found_in_creature_inventory = true;
                        break;
                    }
                }
                if (!found_in_creature_inventory) {
                    ++needed_slots[item_type];
                }
            }
        } else if (item_type == 8) {
            if (getAvatarEmptySlot(0, 104) != -1) {
                ++needed_slots[item_type];
            }
        } else {
            int start = 0;
            int end = 0;
            StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type),
                        start, end);

            bool found_in_inventory = false;
            for (int slot = start; slot <= end; ++slot) {
                const Inven_Item* const item = GetInvenRef(1, slot);
                if (item->getKey() == item_id) {
                    found_in_inventory = true;
                    break;
                }
            }
            if (!found_in_inventory) {
                for (int slot = 3; slot <= 8; ++slot) {
                    const Inven_Item* const item = GetInvenRef(1, slot);
                    if (item->getKey() == item_id) {
                        found_in_inventory = true;
                        break;
                    }
                }
            }
            if (!found_in_inventory) {
                ++needed_slots[item_type];
            }
        }
    }
    return false;
}

// IDA_EA: 0x850D159
// IDA_NAME: _Z18IsCreatureItemTypei
// DEMANGLED: IsCreatureItemType(int)
int IsCreatureItemType(int item_type) {
    return item_type == 5 || item_type == 7 || item_type == 6;
}

// IDA_EA: 0x850D17C
// IDA_NAME: _Z19isEquipableItemTypei
// DEMANGLED: isEquipableItemType(int)
int isEquipableItemType(int item_type) {
    return item_type == 1 || item_type == 5 || item_type == 6;
}
