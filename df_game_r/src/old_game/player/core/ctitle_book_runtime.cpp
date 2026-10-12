#include "old_game/player/core/title_book_runtime.h"
#include "old_game/player/core/title_book_owned_items.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/delete_invalid_item_data.h"
#include "old_data/PVF/etc/title_book_info.h"
#include "old_data/db/inventory/title_book_payload.h"
#include "old_game/economy/inventory/integrated_pvp_item_attr.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/inventory/item_timeout_runtime.h"
#include "old_game/player/core/achievement_title_section.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <new>
#include <stdint.h>
#include <string>

int IsEquipable(CUser* user, const CItem* item, int slot);

namespace {

// The 0828 client/PVF exposes five logical tabs, while the established DB
// payload and CTitleBook ABI still contain four physical banks.  Preserve the
// on-disk/wire persistence layout and carve the new PvP tab from the unused
// tail of the old 200-slot specific bank.
const int kTitleBookPhysicalMax[4] = {70, 200, 120, 60};
const int kTitleBookLogicalMax[5] = {70, 140, 50, 100, 40};

bool ResolveTitleBookSlot(int category,
                          int slot,
                          int& physical_category,
                          int& physical_slot) {
    if (category < 0 || category >= 5 || slot < 0 ||
        slot >= kTitleBookLogicalMax[category]) {
        return false;
    }
    switch (category) {
        case 0:  // general
            physical_category = 0;
            physical_slot = slot;
            return true;
        case 1:  // specific
            physical_category = 1;
            physical_slot = slot;
            return true;
        case 2:  // pvp: tail of the old specific bank
            physical_category = 1;
            physical_slot = 140 + slot;
            return true;
        case 3:  // despair
            physical_category = 2;
            physical_slot = slot;
            return true;
        case 4:  // event
            physical_category = 3;
            physical_slot = slot;
            return true;
        default:
            return false;
    }
}

Inven_Item* LogicalTitleSlot(Inven_Item* const* banks,
                             int category,
                             int slot) {
    int physical_category = 0;
    int physical_slot = 0;
    return ResolveTitleBookSlot(category, slot, physical_category,
                                physical_slot)
               ? banks[physical_category] + physical_slot
               : 0;
}

struct TitleItemPrototypeVTable {
    void (*destructor)(CItem* item);
    void (*deleting_destructor)(CItem* item);
    void (*make_item)(const CItem* item, Inven_Item& inventory_item);
};

#if defined(__i386__)
static_assert(offsetof(TitleItemPrototypeVTable, make_item) == 0x08u,
              "CItem make-item vtable offset must stay 0x08");
#endif

const TitleItemPrototypeVTable* ItemVTable(const CItem* item) {
    const TitleItemPrototypeVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

stTitleBookInfo* TitleBookInfo() {
    return G_CDataManager()->get_title_book_info();
}

uint32_t ItemId(const Inven_Item& item) {
    return static_cast<uint32_t>(item.item_id_);
}

void SetItemId(Inven_Item& item, int item_id) {
    item.item_id_ = item_id;
}

int32_t ItemAddInfo(const Inven_Item& item) {
    return item.add_info_;
}

uint16_t ItemDurability(const Inven_Item& item) {
    return item.avatar_select_;
}

int32_t ItemExpire(const Inven_Item& item) {
    return item.extend_info_;
}

stAmplifyOption_t* AmplifyOption(Inven_Item& item) {
    return &item.amplify_option_;
}

const stAmplifyOption_t* AmplifyOption(const Inven_Item& item) {
    return &item.amplify_option_;
}

CTitleBook* UserTitleBook(CUser* user) {
    return reinterpret_cast<CTitleBook*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)));
}

void TraceInventoryInsertFailure(CUser* user,
                                 int item_id,
                                 const char* function,
                                 int line,
                                 const char* message) {
    cMyTrace trace(function, line, 5);
    trace(message,
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacNo(),
          item_id);
}

}  // namespace

// IDA_EA: 0x848F62A
// IDA_NAME: _ZN11stTitleBookC2Ev
// DEMANGLED: stTitleBook::stTitleBook(void)
stTitleBook::stTitleBook() {
    for (int index = 0; index < 70; ++index) {
        new (category_0[index]) Inven_Item();
    }
    for (int index = 0; index < 200; ++index) {
        new (category_1[index]) Inven_Item();
    }
    for (int index = 0; index < 120; ++index) {
        new (category_2[index]) Inven_Item();
    }
    for (int index = 0; index < 60; ++index) {
        new (category_3[index]) Inven_Item();
    }
    clear();
}

// IDA_EA: 0x848F6E4
// IDA_NAME: _ZN11stTitleBook5clearEv
// DEMANGLED: stTitleBook::clear(void)
uint8_t* stTitleBook::clear() {
    std::memset(category_0, 0, sizeof(category_0));
    std::memset(category_1, 0, sizeof(category_1));
    std::memset(category_2, 0, sizeof(category_2));
    std::memset(category_3, 0, sizeof(category_3));
    return category_3[0];
}

// IDA_EA: 0x864103C
// IDA_NAME: _ZN10CTitleBookC2Ev
// DEMANGLED: CTitleBook::CTitleBook(void)
CTitleBook::CTitleBook()
    : charac_expand::CData(),
      owner_(0),
      title_book_() {
    category_slots_[0] =
        reinterpret_cast<Inven_Item*>(title_book_.category_0);
    category_slots_[1] =
        reinterpret_cast<Inven_Item*>(title_book_.category_1);
    category_slots_[2] =
        reinterpret_cast<Inven_Item*>(title_book_.category_2);
    category_slots_[3] =
        reinterpret_cast<Inven_Item*>(title_book_.category_3);
}

// IDA_EA: 0x86433FE
// IDA_NAME: _ZN10CTitleBookD2Ev
// DEMANGLED: CTitleBook::~CTitleBook()
// IDA_EA: 0x864342E
// IDA_NAME: _ZN10CTitleBookD0Ev
// DEMANGLED: CTitleBook::~CTitleBook()
CTitleBook::~CTitleBook() {}

// IDA_EA: 0x86410B6
// IDA_NAME: _ZN10CTitleBook6_resetEv
// DEMANGLED: CTitleBook::_reset(void)
int CTitleBook::_reset() {
    owner_ = 0;
    return static_cast<int>(
        reinterpret_cast<intptr_t>(title_book_.clear()));
}

// IDA_EA: 0x86410D6
// IDA_NAME: _ZN10CTitleBook9_gm_clearEv
// DEMANGLED: CTitleBook::_gm_clear(void)
void CTitleBook::_gm_clear() {
    alter();
    title_book_.clear();
}

// IDA_EA: 0x86410F8
// IDA_NAME: _ZNK10CTitleBook7getDataEPc
// DEMANGLED: CTitleBook::getData(char *)const
void CTitleBook::getData(char* dest) const {
    std::memcpy(dest, &title_book_, sizeof(title_book_));
}

// CTitleBook::findItemLock @0x8641124 remains in the canonical shared
// item-lock lookup source because CInventory and CCargo use the same closure.

// IDA_EA: 0x86411BA
// IDA_NAME: _ZN10CTitleBook15getItemLockListERSt6vectorIN9item_lock13stItemLockRefESaIS2_EE
// DEMANGLED: CTitleBook::getItemLockList(std::vector<item_lock::stItemLockRef,std::allocator<item_lock::stItemLockRef>> &)
bool CTitleBook::getItemLockList(
    std::vector<item_lock::stItemLockRef>& locks) {
    for (int category = 0; category < 5; ++category) {
        for (int slot = 0; slot < kTitleBookLogicalMax[category]; ++slot) {
            Inven_Item* const item =
                LogicalTitleSlot(category_slots_, category, slot);
            const unsigned char lock = static_cast<unsigned char>(
                AmplifyOption(*item)->GetLock());
            if (lock) {
                item_lock::stItemLockRef reference;
                reference.lock = lock;
                reference.item_space =
                    static_cast<unsigned char>(category + 19);
                reference.slot = static_cast<uint16_t>(slot);
                locks.push_back(reference);
            }
        }
    }
    return false;
}

// IDA_EA: 0x864125A
// IDA_NAME: _ZN10CTitleBook17deleteTimeoutItemERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CTitleBook::deleteTimeoutItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
void CTitleBook::deleteTimeoutItem(
    const std::vector<std::pair<int, int> >& matched_items) {
    std::vector<std::pair<int, int> > invalid_items;
    for (int category = 0; category <= 3; ++category) {
        for (int slot = 0; slot < kTitleBookPhysicalMax[category]; ++slot) {
            Inven_Item& item = category_slots_[category][slot];
            const int item_id = static_cast<int>(ItemId(item));
            if (!item_id) {
                continue;
            }
            if (_checkTimeoutItem(item) ||
                _isMatchedItem(item_id, matched_items) != -1) {
                item.reset();
                alter();
            } else if (static_cast<unsigned char>(
                           G_CDataManager()->get_delete_invalid_item_script()
                               ->IsInvalidItem2Delete(
                               item_id)) == 1u) {
                invalid_items.push_back(
                    std::make_pair(item_id, ItemAddInfo(item)));
                item.reset();
                alter();
            }
        }
    }
    if (owner_) {
        owner_->RewardItem2DeleteInvalidItem(
            std::string("TitleBook"), invalid_items);
    }
}

// IDA_EA: 0x86414A8
// IDA_NAME: _ZN10CTitleBook9_saveDataEP5CUser
// DEMANGLED: CTitleBook::_saveData(CUser *)
int CTitleBook::_saveData(CUser* user) {
    Stream* const stream = GlobalData::big_stream_pool()->Acquire(
        "TitleBook.cpp", 156);
    CStreamGuard guard(stream, true);
    *guard << 0x285;
    *guard << user->GetUID();

    stTitleBook_Save* const payload =
        guard->GetInBuffer<stTitleBook_Save>();
    if (!payload) {
        return 0;
    }
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    std::memcpy(payload->general_section,
                title_book_.category_0,
                sizeof(payload->general_section));
    std::memcpy(payload->specific_section,
                title_book_.category_1,
                sizeof(payload->specific_section));
    std::memcpy(payload->despair,
                title_book_.category_2,
                sizeof(payload->despair));
    std::memcpy(payload->event,
                title_book_.category_3,
                sizeof(payload->event));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x864165E
// IDA_NAME: _ZN10CTitleBook8loadDataEP5CUserPc
// DEMANGLED: CTitleBook::loadData(CUser *,char *)
bool CTitleBook::loadData(CUser* user, char* src) {
    _reset();
    owner_ = user;
    std::memcpy(&title_book_, src, sizeof(title_book_));
    return true;
}

// IDA_EA: 0x86416A4
// IDA_NAME: _ZN10CTitleBook13getTitleSlotRE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::getTitleSlotR(ENUM_TITLE_BOOK_CATEGORY,int)
Inven_Item CTitleBook::getTitleSlotR(ENUM_TITLE_BOOK_CATEGORY category,
                                     int slot) {
    return _getTitleR(category, slot);
}

bool title_book::ContainsOwnedTitle(CTitleBook* book, uint32_t item_id) {
    if (!book || !item_id) return false;
    for (int category = 0; category < 5; ++category) {
        for (int slot = 0; slot < kTitleBookLogicalMax[category]; ++slot) {
            const Inven_Item item = book->getTitleSlotR(
                static_cast<ENUM_TITLE_BOOK_CATEGORY>(category), slot);
            if (ItemId(item) == item_id) return true;
        }
    }
    return false;
}

// IDA_EA: 0x86416D8
// IDA_NAME: _ZN10CTitleBook12getTitleSlotE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::getTitleSlot(ENUM_TITLE_BOOK_CATEGORY,int)
Inven_Item* CTitleBook::getTitleSlot(ENUM_TITLE_BOOK_CATEGORY category,
                                     int slot) {
    Inven_Item* const item = LogicalTitleSlot(
        category_slots_, static_cast<int>(category), slot);
    if (!item) {
        return 0;
    }
    alter();
    return item;
}

// IDA_EA: 0x8641704
// IDA_NAME: _ZN10CTitleBook8sendListEv
// DEMANGLED: CTitleBook::sendList(void)
bool CTitleBook::sendList() {
    for (int category = 0; category < 5; ++category) {
        _sendCategoryData(
            owner_,
            0,
            static_cast<ENUM_TITLE_BOOK_CATEGORY>(category),
            static_cast<ENUM_TITLE_BOOK_INFO_TYPE>(0));
    }
    return false;
}

// IDA_EA: 0x8641750
// IDA_NAME: _ZN10CTitleBook17sendListOtherUserEP5CUser25ENUM_TITLE_BOOK_INFO_TYPE
// DEMANGLED: CTitleBook::sendListOtherUser(CUser *,ENUM_TITLE_BOOK_INFO_TYPE)
bool CTitleBook::sendListOtherUser(CUser* user,
                                   ENUM_TITLE_BOOK_INFO_TYPE type) {
    for (int category = 0; category < 5; ++category) {
        _sendCategoryData(
            user,
            static_cast<unsigned short>(owner_->get_unique_id()),
            static_cast<ENUM_TITLE_BOOK_CATEGORY>(category),
            type);
    }
    return false;
}

// IDA_EA: 0x86417AA
// IDA_NAME: _ZN10CTitleBook17_sendCategoryDataEP5CUsert24ENUM_TITLE_BOOK_CATEGORY25ENUM_TITLE_BOOK_INFO_TYPE
// DEMANGLED: CTitleBook::_sendCategoryData(CUser *,unsigned short,ENUM_TITLE_BOOK_CATEGORY,ENUM_TITLE_BOOK_INFO_TYPE)
void CTitleBook::_sendCategoryData(CUser* user,
                                   unsigned short unique_id,
                                   ENUM_TITLE_BOOK_CATEGORY category,
                                   ENUM_TITLE_BOOK_INFO_TYPE type) {
    Inven_Item owner_item;
    owner_item.reset();
    ENUM_TITLE_BOOK_CATEGORY owner_category =
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(5);
    int owner_slot = 0;
    bool append_owner_title = false;

    PacketGuard packet;
    packet.put_header(0, 357);
    packet.put_byte(static_cast<int>(type));
    packet.put_short(unique_id);
    packet.put_int(static_cast<int>(category));

    int count = _getCategoryItemCount(category);
    if (unique_id &&
        _getOwnerTitleInfo(owner_category, owner_slot, &owner_item) &&
        category == owner_category) {
        ++count;
        append_owner_title = true;
    }
    packet.put_int(count);

    const int category_index = static_cast<int>(category);
    for (int slot = 0;
         category_index >= 0 && category_index < 5 &&
         slot < kTitleBookLogicalMax[category_index];
         ++slot) {
        Inven_Item* const item =
            LogicalTitleSlot(category_slots_, category_index, slot);
        if (item && ItemId(*item)) {
            _putItemData(packet, static_cast<short>(slot), *item);
        }
    }
    if (append_owner_title) {
        _putItemData(packet, static_cast<short>(owner_slot), owner_item);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8641994
// IDA_NAME: _ZN10CTitleBook18_getOwnerTitleInfoER24ENUM_TITLE_BOOK_CATEGORYRiP10Inven_Item
// DEMANGLED: CTitleBook::_getOwnerTitleInfo(ENUM_TITLE_BOOK_CATEGORY &,int &,Inven_Item *)
int CTitleBook::_getOwnerTitleInfo(ENUM_TITLE_BOOK_CATEGORY& category,
                                   int& slot,
                                   Inven_Item* out) {
    *out = owner_->getCurCharacInvenR()->GetInvenSlot(
        0, oldsrv::config::kEquipmentTypeTitle);
    return ItemId(*out) && _getFitSlot(ItemId(*out), category, slot);
}

// IDA_EA: 0x8641A6A
// IDA_NAME: _ZN10CTitleBook12_putItemDataER11PacketGuardsR10Inven_Item
// DEMANGLED: CTitleBook::_putItemData(PacketGuard &,short,Inven_Item &)
int CTitleBook::_putItemData(PacketGuard& packet,
                             short slot,
                             Inven_Item& item) {
    packet.put_short(slot);
    packet.put_int(static_cast<int>(ItemId(item)));
    packet.put_int(ItemAddInfo(item));
    packet.put_byte(GetIntegratedPvPItemAttr(item));
    packet.put_short(ItemDurability(item));
    packet.put_byte(item.seal_or_state_);
    packet.put_int(G_GameWorld()->IsEnchantRevisionChannel()
                       ? 0
                       : ItemExpire(item));
    packet.put_byte(AmplifyOption(item)->getAbilityType());
    return packet.put_short(AmplifyOption(item)->getAbilityValue());
}

// IDA_EA: 0x8641B84
// IDA_NAME: _ZN10CTitleBook21_getCategoryItemCountE24ENUM_TITLE_BOOK_CATEGORY
// DEMANGLED: CTitleBook::_getCategoryItemCount(ENUM_TITLE_BOOK_CATEGORY)
int CTitleBook::_getCategoryItemCount(
    ENUM_TITLE_BOOK_CATEGORY category) {
    const int category_index = static_cast<int>(category);
    if (category_index < 0 || category_index >= 5) {
        return 0;
    }
    int count = 0;
    for (int slot = 0; slot < kTitleBookLogicalMax[category_index]; ++slot) {
        Inven_Item* const item =
            LogicalTitleSlot(category_slots_, category_index, slot);
        if (item && ItemId(*item)) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x8641BDC
// IDA_NAME: _ZN10CTitleBook17_checkInsertTitleE24ENUM_TITLE_BOOK_CATEGORYiP10Inven_Item
// DEMANGLED: CTitleBook::_checkInsertTitle(ENUM_TITLE_BOOK_CATEGORY,int,Inven_Item *)
bool CTitleBook::_checkInsertTitle(ENUM_TITLE_BOOK_CATEGORY category,
                                   int slot,
                                   Inven_Item* item) {
    (void)slot;
    if (!item) {
        return false;
    }
    CItem* const item_data =
        G_CDataManager()->find_item(static_cast<int>(ItemId(*item)));
    if (!item_data ||
        static_cast<unsigned char>(item_data->is_stackable()) != 0u) {
        return false;
    }
    return static_cast<int>(category) != 4 || !item->seal_or_state_;
}

// IDA_EA: 0x8641C52
// IDA_NAME: _ZN10CTitleBook17_checkMovetoInvenE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::_checkMovetoInven(ENUM_TITLE_BOOK_CATEGORY,int)
bool CTitleBook::_checkMovetoInven(ENUM_TITLE_BOOK_CATEGORY category,
                                   int slot) {
    stTitleBookInfo* const info = TitleBookInfo();
    std::map<int, std::vector<stTitleElement> >::iterator found =
        info->title_collections.find(static_cast<int>(category));
    if (found == info->title_collections.end()) {
        return false;
    }
    return slot >= 0 &&
           found->second.size() > static_cast<unsigned int>(slot) &&
           found->second[slot].condition_type == -1;
}

// IDA_EA: 0x8641D16
// IDA_NAME: _ZN10CTitleBook12_insertTitleE24ENUM_TITLE_BOOK_CATEGORYiP10Inven_Item
// DEMANGLED: CTitleBook::_insertTitle(ENUM_TITLE_BOOK_CATEGORY,int,Inven_Item *)
Inven_Item* CTitleBook::_insertTitle(ENUM_TITLE_BOOK_CATEGORY category,
                                     int slot,
                                     Inven_Item* item) {
    Inven_Item* const destination = LogicalTitleSlot(
        category_slots_, static_cast<int>(category), slot);
    if (!destination || !item) {
        return 0;
    }
    alter();
    return destination->setCopy(*item);
}

// IDA_EA: 0x8641D50
// IDA_NAME: _ZN10CTitleBook12_deleteTitleE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::_deleteTitle(ENUM_TITLE_BOOK_CATEGORY,int)
Inven_Item* CTitleBook::_deleteTitle(ENUM_TITLE_BOOK_CATEGORY category,
                                     int slot) {
    Inven_Item* const item = LogicalTitleSlot(
        category_slots_, static_cast<int>(category), slot);
    if (!item) {
        return 0;
    }
    alter();
    return item->reset();
}

// IDA_EA: 0x8641D84
// IDA_NAME: _ZN10CTitleBook9_getTitleE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::_getTitle(ENUM_TITLE_BOOK_CATEGORY,int)
Inven_Item* CTitleBook::_getTitle(ENUM_TITLE_BOOK_CATEGORY category,
                                  int slot) {
    Inven_Item* const item = LogicalTitleSlot(
        category_slots_, static_cast<int>(category), slot);
    if (!item) {
        return 0;
    }
    alter();
    return item;
}

// IDA_EA: 0x8641DB0
// IDA_NAME: _ZN10CTitleBook10_getTitleRE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::_getTitleR(ENUM_TITLE_BOOK_CATEGORY,int)
Inven_Item CTitleBook::_getTitleR(ENUM_TITLE_BOOK_CATEGORY category,
                                  int slot) {
    Inven_Item* const item = LogicalTitleSlot(
        category_slots_, static_cast<int>(category), slot);
    if (item) {
        return *item;
    }
    Inven_Item empty;
    empty.reset();
    return empty;
}

// IDA_EA: 0x8641DF2
// IDA_NAME: _ZN10CTitleBook6_emptyEv
// DEMANGLED: CTitleBook::_empty(void)
int CTitleBook::_empty() {
    for (int category = 0; category <= 3; ++category) {
        for (int slot = 0; slot < kTitleBookPhysicalMax[category]; ++slot) {
            if (ItemId(category_slots_[category][slot])) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8641E60
// IDA_NAME: _ZN10CTitleBook12putTitleOnlyEP5CUseri
// DEMANGLED: CTitleBook::putTitleOnly(CUser *,int)
int CTitleBook::putTitleOnly(CUser* user, int title_reward) {
    ENUM_TITLE_BOOK_CATEGORY category =
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(5);
    int slot = 0;
    return putTitleOnly(user, title_reward, category, slot);
}

// IDA_EA: 0x8641E88
// IDA_NAME: _ZN10CTitleBook12putTitleOnlyEP5CUseriR24ENUM_TITLE_BOOK_CATEGORYRi
// DEMANGLED: CTitleBook::putTitleOnly(CUser *,int,ENUM_TITLE_BOOK_CATEGORY &,int &)
int CTitleBook::putTitleOnly(CUser* user,
                            int title_reward,
                            ENUM_TITLE_BOOK_CATEGORY& category,
                            int& slot) {
    CTitleBook* const title_book = UserTitleBook(user);
    if (!title_book || !title_reward) {
        return 0;
    }

    Inven_Item item;
    SetItemId(item, title_reward);
    const CItem* const item_data = G_CDataManager()->find_item(title_reward);
    if (!item_data) {
        return 0;
    }
    ItemVTable(item_data)->make_item(item_data, item);
    item.set_add_info(1);
    if (_getFitSlot(title_reward, category, slot) != 1 ||
        !title_book->_checkInsertTitle(category, slot, &item)) {
        return 0;
    }
    title_book->_insertTitle(category, slot, &item);
    return 1;
}

// IDA_EA: 0x8641FAC
// IDA_NAME: _ZN10CTitleBook10checkErrorEP5CUser24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::checkError(CUser *,ENUM_TITLE_BOOK_CATEGORY,int)
int CTitleBook::checkError(CUser* user,
                           ENUM_TITLE_BOOK_CATEGORY category,
                           int slot) {
    if (!user || slot < 0) {
        return 3;
    }
    const int category_index = static_cast<int>(category);
    if (category_index < 0 || category_index >= 5 ||
        slot >= kTitleBookLogicalMax[category_index]) {
        return 3;
    }
    return 0;
}

// IDA_EA: 0x8642058
// IDA_NAME: _ZN10CTitleBook8putTitleEP5CUser14ENUM_ITEMSPACEii24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::putTitle(CUser *,ENUM_ITEMSPACE,int,int,ENUM_TITLE_BOOK_CATEGORY,int)
int CTitleBook::putTitle(CUser* user,
                         ENUM_ITEMSPACE item_space,
                         int source_slot,
                         int item_id,
                         ENUM_TITLE_BOOK_CATEGORY category,
                         int title_slot) {
    CTitleBook* const title_book = UserTitleBook(user);
    if (!title_book) {
        return 3;
    }
    // Validate the five logical 0828 tabs before inventory, achievement, or
    // persistent title-book state can be observed or changed.
    const int error = checkError(user, category, title_slot);
    if (error) {
        return error;
    }
    if (static_cast<int>(item_space) == 18) {
        item_space = static_cast<ENUM_ITEMSPACE>(3);
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const int inventory_type = GetInvenTypeFromItemSpace(item_space);
    Inven_Item source_item =
        inventory->GetInvenSlot(inventory_type, source_slot);
    if (!item_id || ItemId(source_item) != static_cast<uint32_t>(item_id)) {
        return 2;
    }

    const int section_index =
        CAchievement::_getSectionIdx(category, title_slot);
    if (section_index != -1) {
        const stTitleSection section(
            TitleBookInfo()->sections[section_index]);
        if (!section.lock_message.empty()) {
            return 12;
        }
    }
    if (user->get_charac_level() <= 9) {
        return 8;
    }
    if (static_cast<int>(category) == 3 &&
        user->get_charac_level() <= 69) {
        return 9;
    }
    if (_checkFitSlot(item_id, category, title_slot) != 1 ||
        !title_book->_checkInsertTitle(category, title_slot, &source_item)) {
        return 2;
    }
    if (inventory->delete_item(
            static_cast<INVEN_TYPE>(inventory_type),
            source_slot,
            1,
            static_cast<eItemDelReason>(43),
            true) != 1) {
        return 2;
    }
    if (!inventory_type &&
        source_slot == oldsrv::config::kEquipmentTypeTitle) {
        user->send_equip(oldsrv::config::kEquipmentTypeTitle);
    }

    Inven_Item previous =
        title_book->getTitleSlotR(category, title_slot);
    if (ItemId(previous)) {
        if (inventory_type == 1) {
            const int inserted = inventory->insertItemIntoInventory(
                previous,
                static_cast<eItemAddReason>(7),
                true,
                false);
            if (inserted < 0) {
                TraceInventoryInsertFailure(
                    user,
                    static_cast<int>(ItemId(previous)),
                    "static int CTitleBook::putTitle(CUser*, ENUM_ITEMSPACE, int, int, ENUM_TITLE_BOOK_CATEGORY, int)",
                    591,
                    "insertItemIntoInventory failed(title) %s %d %d");
            }
        } else if (!inventory_type) {
            const int inserted = inventory->insertItemIntoEquipment(
                previous,
                static_cast<eItemAddReason>(7),
                true,
                false);
            if (inserted >= 0) {
                user->send_equip(source_slot);
            } else {
                TraceInventoryInsertFailure(
                    user,
                    static_cast<int>(ItemId(previous)),
                    "static int CTitleBook::putTitle(CUser*, ENUM_ITEMSPACE, int, int, ENUM_TITLE_BOOK_CATEGORY, int)",
                    598,
                    "insertItemIntoEquipment failed(title) %s %d %d");
            }
        }
    }
    title_book->_insertTitle(category, title_slot, &source_item);
    return 0;
}

// IDA_EA: 0x86425C4
// IDA_NAME: _ZN10CTitleBook8getTitleEP5CUser24ENUM_TITLE_BOOK_CATEGORYii14ENUM_ITEMSPACERi
// DEMANGLED: CTitleBook::getTitle(CUser *,ENUM_TITLE_BOOK_CATEGORY,int,int,ENUM_ITEMSPACE,int &)
int CTitleBook::getTitle(CUser* user,
                         ENUM_TITLE_BOOK_CATEGORY category,
                         int title_slot,
                         int item_id,
                         ENUM_ITEMSPACE item_space,
                         int& output_slot) {
    CTitleBook* const title_book = UserTitleBook(user);
    if (!title_book) {
        return 3;
    }
    if (static_cast<int>(item_space) == 18) {
        item_space = static_cast<ENUM_ITEMSPACE>(3);
    }
    const int error = checkError(user, category, title_slot);
    if (error) {
        return error;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const int inventory_type = GetInvenTypeFromItemSpace(item_space);
    Inven_Item item = *title_book->_getTitle(category, title_slot);
    if (!item_id || ItemId(item) != static_cast<uint32_t>(item_id)) {
        return 2;
    }
    const CItem* const item_data = G_CDataManager()->find_item(item_id);
    if (!item_data) {
        return 2;
    }

    if (inventory_type == 1) {
        if (_checkMovetoInven(category, title_slot) != 1) {
            return 5;
        }
        if (inventory->tryInsertItemIntoInventory(item) < 0) {
            return 11;
        }
        title_book->_deleteTitle(category, title_slot);
        const int inserted = inventory->insertItemIntoInventory(
            item,
            static_cast<eItemAddReason>(7),
            true,
            false);
        if (inserted < 0) {
            TraceInventoryInsertFailure(
                user,
                item_id,
                "static int CTitleBook::getTitle(CUser*, ENUM_TITLE_BOOK_CATEGORY, int, int, ENUM_ITEMSPACE, int&)",
                677,
                "insertItemIntoInventory failed(title) %s %d %d");
        }
        output_slot = inserted;
    } else if (!inventory_type) {
        if (output_slot != oldsrv::config::kEquipmentTypeTitle) {
            return 7;
        }
        if (inventory->tryInsertItemIntoEquipment(item) < 0) {
            return 7;
        }
        if (IsEquipable(user, item_data, output_slot)) {
            return 6;
        }
        if (AmplifyOption(item)->GetLock() && item.seal_or_state_) {
            return 1;
        }
        item.seal_or_state_ = 0;
        title_book->_deleteTitle(category, title_slot);
        const int inserted = inventory->insertItemIntoEquipment(
            item,
            static_cast<eItemAddReason>(7),
            true,
            false);
        if (inserted < 0) {
            TraceInventoryInsertFailure(
                user,
                item_id,
                "static int CTitleBook::getTitle(CUser*, ENUM_TITLE_BOOK_CATEGORY, int, int, ENUM_ITEMSPACE, int&)",
                711,
                "insertItemIntoEquipment failed(title) %s %d %d");
        }
        user->send_equip(inserted);
    }
    return 0;
}

// IDA_EA: 0x8642BAA
// IDA_NAME: _ZN10CTitleBook11_getFitSlotEiR24ENUM_TITLE_BOOK_CATEGORYRi
// DEMANGLED: CTitleBook::_getFitSlot(int,ENUM_TITLE_BOOK_CATEGORY &,int &)
int CTitleBook::_getFitSlot(int title_reward,
                            ENUM_TITLE_BOOK_CATEGORY& category,
                            int& slot) {
    stTitleBookInfo* const info = TitleBookInfo();
    for (std::map<int, std::vector<stTitleElement> >::iterator category_it =
             info->title_collections.begin();
         category_it != info->title_collections.end();
         ++category_it) {
        std::vector<stTitleElement>& elements = category_it->second;
        for (std::vector<stTitleElement>::size_type index = 0;
             index < elements.size();
             ++index) {
            if (std::find(elements[index].condition_values.begin(),
                          elements[index].condition_values.end(),
                          title_reward) !=
                elements[index].condition_values.end()) {
                category = static_cast<ENUM_TITLE_BOOK_CATEGORY>(
                    category_it->first);
                slot = static_cast<int>(index);
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8642D42
// IDA_NAME: _ZN10CTitleBook13_checkFitSlotEi24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CTitleBook::_checkFitSlot(int,ENUM_TITLE_BOOK_CATEGORY,int)
bool CTitleBook::_checkFitSlot(int title_reward,
                               ENUM_TITLE_BOOK_CATEGORY category,
                               int slot) {
    stTitleBookInfo* const info = TitleBookInfo();
    std::map<int, std::vector<stTitleElement> >::const_iterator found =
        info->title_collections.find(static_cast<int>(category));
    if (found == info->title_collections.end() || slot < 0 ||
        found->second.size() <= static_cast<unsigned int>(slot)) {
        return false;
    }
    const stTitleElement& element = found->second[slot];
    return std::find(element.condition_values.begin(),
                     element.condition_values.end(),
                     title_reward) != element.condition_values.end() ||
           element.condition_type == -1;
}

// IDA_EA: 0x8642EC2
// IDA_NAME: _ZN10CTitleBook14isAbleItemLockEm
// DEMANGLED: CTitleBook::isAbleItemLock(unsigned long)
bool CTitleBook::isAbleItemLock(unsigned long item_id) {
    stTitleBookInfo* const info = TitleBookInfo();
    for (std::map<int, std::vector<stTitleElement> >::const_iterator
             category_it = info->title_collections.begin();
         category_it != info->title_collections.end();
         ++category_it) {
        const std::vector<stTitleElement>& elements = category_it->second;
        for (std::vector<stTitleElement>::size_type index = 0;
             index < elements.size();
             ++index) {
            const stTitleElement& element = elements[index];
            if (std::find(element.condition_values.begin(),
                          element.condition_values.end(),
                          static_cast<int>(item_id)) !=
                element.condition_values.end()) {
                return element.condition_type == -1;
            }
        }
    }
    return true;
}

// IDA_EA: 0x864306E
// IDA_NAME: _ZN10CTitleBook14_isMatchedItemEiRKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CTitleBook::_isMatchedItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
int CTitleBook::_isMatchedItem(
    int item_id,
    const std::vector<std::pair<int, int> >& matched_items) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             matched_items.begin();
         it != matched_items.end();
         ++it) {
        if (it->first == item_id) {
            return it->second;
        }
    }
    return -1;
}
