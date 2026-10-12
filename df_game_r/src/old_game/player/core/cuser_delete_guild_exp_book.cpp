#include "old_game/player/core/cuser.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <utility>
#include <vector>

// IDA_EA: 0x867E898
// IDA_NAME: _ZN5CUser18deleteGuildExpBookEv
// DEMANGLED: CUser::deleteGuildExpBook(void)
void CUser::deleteGuildExpBook() {
    // The original pushes ENUM_HARDCODE_ITEM 1200 with a zero count into the
    // deleteSpecificItem request.  The public deletion transaction accepts
    // its ABI-equivalent int pair, preserving the "delete all matching" zero.
    std::vector<std::pair<int, int> > matched_items;
    std::vector<std::pair<int, int> > changed_slots;
    matched_items.push_back(std::make_pair(1200, 0));
    deleteSpecificItem(matched_items, changed_slots);

    if (static_cast<unsigned char>(send_itemspace(0)) == 1u) {
        if (static_cast<unsigned char>(send_itemspace(2)) == 1u) {
            send_RedeemInfo();
            return;
        }

        LogManager::logFormat(
            1,
            "user.cpp",
            "void CUser::deleteGuildExpBook()",
            0x7C5C,
            "deleteGuildExpBook, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_CARGO ) return FAILED",
            NumberToString(static_cast<unsigned int>(get_acc_id()), 0));
        return;
    }

    LogManager::logFormat(
        1,
        "user.cpp",
        "void CUser::deleteGuildExpBook()",
        0x7C57,
        "deleteGuildExpBook, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_INVENTORY ) return FAILED",
        NumberToString(static_cast<unsigned int>(get_acc_id()), 0));
}
