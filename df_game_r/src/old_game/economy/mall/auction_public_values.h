#ifndef OLDSRV_GLOBAL_AUCTION_PUBLIC_VALUES_H_
#define OLDSRV_GLOBAL_AUCTION_PUBLIC_VALUES_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/equipment/item_option.h"

#pragma pack(push, 1)

// IDA unit: 99_misc_global/ROI_Category.
// IDA_EA: ROI_Category @0x822AC10; _sort @0x822AC30.
struct ROI_Category {
    ROI_Category();

    ROI_Category* _sort();

    // The sort routine directly proves the three signed categories and the
    // byte flags paired with them.  The remaining bytes are initialized but
    // not interpreted by the recovered callers.
    int16_t category_0;
    int16_t category_1;
    int16_t category_2;
    uint8_t reserved_06[0x02];
    uint8_t flag_08;
    uint8_t flag_09;
    uint8_t flag_0a;
    uint8_t reserved_0b;
};

static_assert(sizeof(ROI_Category) == 0x0cu,
              "ROI_Category IDA ABI size must stay 0x0c");

// IDA unit: 99_misc_global/DnfItemInfo.
struct DnfItemInfo {
    DnfItemInfo();

    ReservedCapacity* reset();

    // The complete packed 0627 item-service record.  These are physical
    // members, not a byte-buffer view: guild cargo and auction packets embed
    // this object directly at unaligned wire offsets.
    uint8_t seal_or_state_;                              // +0x00
    uint32_t item_id_;                                   // +0x01
    uint8_t item_attr_;                                  // +0x05
    uint32_t add_info_;                                  // +0x06
    uint16_t avatar_select_;                             // +0x0a
    uint32_t extend_info_;                               // +0x0c
    uint8_t amplify_type_;                               // +0x10
    uint16_t amplify_value_;                             // +0x11
    itemGloballyUniqueIdentifier_t globally_unique_id_;  // +0x13
    RandomOption random_option_;                         // +0x1d
    UpgradeSeparateInfo upgrade_separate_;               // +0x2b
    ReservedCapacity reserved_capacity_;                 // +0x2c
};

static_assert(sizeof(DnfItemInfo) == 0x35u,
              "DnfItemInfo IDA ABI size must stay 0x35");
static_assert(offsetof(DnfItemInfo, item_id_) == 0x01u,
              "DnfItemInfo item id must retain IDA +0x01");
static_assert(offsetof(DnfItemInfo, add_info_) == 0x06u,
              "DnfItemInfo add-info must retain IDA +0x06");
static_assert(offsetof(DnfItemInfo, globally_unique_id_) == 0x13u,
              "DnfItemInfo global id must retain IDA +0x13");
static_assert(offsetof(DnfItemInfo, random_option_) == 0x1du,
              "DnfItemInfo random option must retain IDA +0x1d");
static_assert(offsetof(DnfItemInfo, upgrade_separate_) == 0x2bu,
              "DnfItemInfo upgrade-separate must retain IDA +0x2b");
static_assert(offsetof(DnfItemInfo, reserved_capacity_) == 0x2cu,
              "DnfItemInfo reserved capacity must retain IDA +0x2c");

// IDA unit: 99_misc_global/TSearchByItemId_.
struct TSearchByItemId_ {
    TSearchByItemId_();

    // The old client-to-auction query fields are forwarded in this exact
    // order.  Their business labels are not present in the binary, so the
    // neutral names deliberately avoid inventing semantics while still
    // restoring the real typed object graph.
    uint32_t query_value_;
    uint8_t query_filter_0_;
    uint8_t query_filter_1_;
    uint8_t item_key_count_;
    uint16_t query_filter_2_;
    ROI_Category category_;
    uint8_t category_filter_0_;
    uint8_t category_filter_1_;
    uint32_t item_keys_[20];
};

static_assert(sizeof(TSearchByItemId_) == 0x67u,
              "TSearchByItemId_ public packet span must stay 0x67");
static_assert(offsetof(TSearchByItemId_, category_) == 0x09u,
              "TSearchByItemId_ category must retain IDA +0x09");
static_assert(offsetof(TSearchByItemId_, item_keys_) == 0x17u,
              "TSearchByItemId_ keys must retain IDA +0x17");

// IDA unit: 99_misc_global/TSearchByCategory_.
struct TSearchByCategory_ {
    TSearchByCategory_();

    uint32_t query_value_;
    uint16_t query_filter_0_;
    uint8_t query_filter_1_;
    uint8_t query_filter_2_;
    uint8_t query_filter_3_;
    uint8_t query_filter_4_;
    uint8_t query_filter_5_;
    ROI_Category category_;
    uint8_t category_filter_0_;
    uint8_t category_filter_1_;
};

static_assert(sizeof(TSearchByCategory_) == 0x19u,
              "TSearchByCategory_ public packet span must stay 0x19");
static_assert(offsetof(TSearchByCategory_, category_) == 0x0bu,
              "TSearchByCategory_ category must retain IDA +0x0b");

#pragma pack(pop)


using ::DnfItemInfo;
using ::ROI_Category;
using ::TSearchByCategory_;
using ::TSearchByItemId_;


#endif  // OLDSRV_GLOBAL_AUCTION_PUBLIC_VALUES_H_
