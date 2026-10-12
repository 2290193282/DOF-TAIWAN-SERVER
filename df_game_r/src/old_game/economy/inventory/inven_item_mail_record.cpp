#include "old_game/economy/inventory/inventory.h"

namespace {

void StoreU32LittleEndian(uint8_t* destination, uint32_t value) {
    destination[0] = static_cast<uint8_t>(value);
    destination[1] = static_cast<uint8_t>(value >> 8);
    destination[2] = static_cast<uint8_t>(value >> 16);
    destination[3] = static_cast<uint8_t>(value >> 24);
}

void StoreU16LittleEndian(uint8_t* destination, uint16_t value) {
    destination[0] = static_cast<uint8_t>(value);
    destination[1] = static_cast<uint8_t>(value >> 8);
}

}  // namespace

// The source offsets are the exact field reads in
// CMailBoxHelper::_makeSendMailData @ 0x8555756.  The values below are now
// read from their physical Inven_Item members rather than a byte facade.
void Inven_Item::ExportMailRecord(InvenItemMailRecord& record) const {
    record.seal_flag = seal_or_state_;
    record.item_id = static_cast<uint32_t>(item_id_);
    record.add_info = static_cast<uint32_t>(add_info_);
    record.item_attr = item_attr_;
    record.endurance = avatar_select_;
    record.extend_info = static_cast<uint32_t>(extend_info_);

    StoreU32LittleEndian(record.item_guid,
                          static_cast<uint32_t>(globally_unique_id_.server_id_));
    StoreU32LittleEndian(record.item_guid + 4,
                          static_cast<uint32_t>(globally_unique_id_.datetime_));
    StoreU16LittleEndian(record.item_guid + 8,
                          globally_unique_id_.sequence_no_);

    // stAmplifyOption_t::getValues @ 0x844D490 reads the original ability
    // byte and little-endian word at Inven_Item +0x11/+0x12.
    record.amplify_option =
        amplify_option_.ability_type_and_identification_;
    record.amplify_value = amplify_option_.ability_value_;
    record.chronicle_value = chronicle_random_option_.value_;

    const RandomOption& random_option =
        chronicle_random_option_.random_option_;
    for (int index = 0; index < 3; ++index) {
        const RandomOptionField& field = random_option.option_fields_[index];
        record.random_option[3 * index] = field.option_index_;
        record.random_option[3 * index + 1] = field.first_value_;
        record.random_option[3 * index + 2] = field.second_value_;
    }
    record.random_option[9] = random_option.initial_seed_.seed_;
    record.random_option[10] = random_option.changed_option_.option_index_;
    record.random_option[11] = random_option.changed_option_.first_value_;
    record.random_option[12] = random_option.changed_option_.second_value_;
    record.random_option[13] = random_option.changed_option_seed_.seed_;

    // UpgradeSeparateInfo::GetUpgradeSeparate @ 0x811ED58 masks the low
    // five bits of the original +0x33 byte.
    record.separate_upgrade = static_cast<uint8_t>(
        upgrade_separate_.GetUpgradeSeparate());
    record.avatar_flag = isAvatarItemType();
    record.creature_flag = IsCreatureItemType();
}
