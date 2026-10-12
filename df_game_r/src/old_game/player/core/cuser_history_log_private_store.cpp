#include "old_game/player/core/cuser.h"

#include "old_game/economy/equipment/item_option.h"

namespace {

const char* GetItemUniqueId(const Inven_Item& item,
                            CItemGloballyUniqueIdentifierPrint& printer) {
    return printer(item.globally_unique_id_);
}

}  // namespace

// IDA_EA: 0x868455A
// IDA_NAME: _ZN15cUserHistoryLog18CreatePrivateStoreEiiPKc
// DEMANGLED: cUserHistoryLog::CreatePrivateStore(int,int,char const*)
int cUserHistoryLog::CreatePrivateStore(int village_index,
                                        int area_index,
                                        const char*) {
    return owner_->LogHistory("Private Store+,%d,%d", village_index,
                              area_index);
}

// IDA_EA: 0x8684586
// IDA_NAME: _ZN15cUserHistoryLog18RemovePrivateStoreEiiPKc
// DEMANGLED: cUserHistoryLog::RemovePrivateStore(int,int,char const*)
int cUserHistoryLog::RemovePrivateStore(int village_index,
                                        int area_index,
                                        const char*) {
    return owner_->LogHistory("Private Store-,%d,%d", village_index,
                              area_index);
}

// IDA_EA: 0x868412A
// IDA_NAME: _ZN15cUserHistoryLog19PrivateStoreItemDelE10INVEN_TYPERK10Inven_Itemii14eItemDelReasonPKcS6_
// DEMANGLED: cUserHistoryLog::PrivateStoreItemDel(INVEN_TYPE,Inven_Item const&,int,int,eItemDelReason,char const*,char const*)
void cUserHistoryLog::PrivateStoreItemDel(INVEN_TYPE inventory_type,
                                          const Inven_Item& item,
                                          int value_1,
                                          int value_2,
                                          eItemDelReason reason,
                                          const char*,
                                          const char* character_name) {
    CItemGloballyUniqueIdentifierPrint printer;
    const ::RandomOption& random_option =
        item.chronicle_random_option_.random_option();
    owner_->LogHistory(
        "Item-,%d,%d,%d,%d,%d,\"%s\",%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
        static_cast<int>(inventory_type), item.item_id_, value_1,
        value_2, static_cast<int>(reason), character_name,
        static_cast<unsigned char>(item.GetUpgrade()), item.avatar_select_,
        random_option.option_fields_[0].option_index_,
        random_option.option_fields_[0].first_value_,
        random_option.option_fields_[0].second_value_,
        random_option.option_fields_[1].option_index_,
        random_option.option_fields_[1].first_value_,
        random_option.option_fields_[1].second_value_,
        random_option.option_fields_[2].option_index_,
        random_option.option_fields_[2].first_value_,
        random_option.option_fields_[2].second_value_,
        random_option.initial_seed_.seed_,
        random_option.changed_option_.option_index_,
        random_option.changed_option_.first_value_,
        random_option.changed_option_.second_value_,
        random_option.changed_option_seed_.seed_,
        static_cast<unsigned char>(item.GetReSealCount()),
        static_cast<int>(item.seal_or_state_), GetItemUniqueId(item, printer));
}

// IDA_EA: 0x868433E
// IDA_NAME: _ZN15cUserHistoryLog19PrivateStoreItemAddE10INVEN_TYPERK10Inven_Itemii14eItemAddReasonPKcS6_
// DEMANGLED: cUserHistoryLog::PrivateStoreItemAdd(INVEN_TYPE,Inven_Item const&,int,int,eItemAddReason,char const*,char const*)
void cUserHistoryLog::PrivateStoreItemAdd(INVEN_TYPE inventory_type,
                                          const Inven_Item& item,
                                          int value_1,
                                          int value_2,
                                          eItemAddReason reason,
                                          const char* trader_name,
                                          const char* character_name) {
    CItemGloballyUniqueIdentifierPrint printer;
    const ::RandomOption& random_option =
        item.chronicle_random_option_.random_option();
    owner_->LogHistory(
        "Item+,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
        static_cast<int>(inventory_type), item.item_id_, value_1,
        value_2, static_cast<int>(reason), trader_name, character_name,
        static_cast<unsigned char>(item.GetUpgrade()), item.avatar_select_,
        random_option.option_fields_[0].option_index_,
        random_option.option_fields_[0].first_value_,
        random_option.option_fields_[0].second_value_,
        random_option.option_fields_[1].option_index_,
        random_option.option_fields_[1].first_value_,
        random_option.option_fields_[1].second_value_,
        random_option.option_fields_[2].option_index_,
        random_option.option_fields_[2].first_value_,
        random_option.option_fields_[2].second_value_,
        random_option.initial_seed_.seed_,
        random_option.changed_option_.option_index_,
        random_option.changed_option_.first_value_,
        random_option.changed_option_.second_value_,
        random_option.changed_option_seed_.seed_,
        static_cast<unsigned char>(item.GetReSealCount()),
        static_cast<int>(item.seal_or_state_), GetItemUniqueId(item, printer));
}
