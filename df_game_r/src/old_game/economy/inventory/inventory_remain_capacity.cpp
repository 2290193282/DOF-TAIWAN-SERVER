#include "old_game/economy/inventory/inventory.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x84FB67A
// IDA_NAME: _ZNK10CInventory17GetRemainCapacityE10INVEN_TYPEN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CInventory::GetRemainCapacity(INVEN_TYPE,Inven_Item::ITEM_TYPE)const
int CInventory::GetRemainCapacity(
    INVEN_TYPE inventory_type, Inven_Item::ITEM_TYPE item_type) const {
    int remain_capacity = 0;

    switch (static_cast<int>(inventory_type)) {
    case 0:
    case 6:
        LogManager::logFormat(
            1, "inventory.cpp",
            "int CInventory::GetRemainCapacity(INVEN_TYPE, Inven_Item::ITEM_TYPE) const",
            0x4c3, "Invalid inventory type");
        break;

    case 1: {
        int start_pos = 0;
        int end_pos = 0;
        StartEndPos(item_type, start_pos, end_pos);

        switch (static_cast<int>(item_type)) {
        case 0:
        case 1:
        case 3:
        case 4:
        case 10:
            return remain_capacity;

        case 2: {
            Inven_Item* const items = GetInvenStart(1);
            for (int pos = start_pos; pos <= end_pos; ++pos) {
                if (items[pos].isEmpty()) {
                    ++remain_capacity;
                }
            }
            break;
        }

        default:
            LogManager::logFormat(
                1, "inventory.cpp",
                "int CInventory::GetRemainCapacity(INVEN_TYPE, Inven_Item::ITEM_TYPE) const",
                0x493, "Invalid Item Type");
            break;
        }
        break;
    }

    case 2: {
        Inven_Item* const items = GetInvenStart(2);
        for (int pos = 0; pos <= 104; ++pos) {
            if (items[pos].isEmpty()) {
                ++remain_capacity;
            }
        }
        break;
    }

    case 3: {
        int start_pos = 0;
        int end_pos = 0;
        StartEndPos(item_type, start_pos, end_pos);

        Inven_Item* const items = GetInvenStart(3);
        for (int pos = start_pos; pos <= end_pos; ++pos) {
            if (items[pos].isEmpty()) {
                ++remain_capacity;
            }
        }
        break;
    }

    default:
        break;
    }

    return remain_capacity;
}
