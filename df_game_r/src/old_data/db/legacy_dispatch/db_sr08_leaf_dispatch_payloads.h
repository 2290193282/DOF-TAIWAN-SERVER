#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCH_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCH_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

// The old binary exposes this enum only as a typed four-byte transport
// boundary here. The two observed values are passed by DB_Insert0712...
// (18 to DB_Q and 19 to LOGDB_Q); no global enum table is invented.
enum ENUM_INTERNALPACKET : int;

// IDA_EA: Stream::GetOutBuffer<SIG_SAVE_CONDITION_EVENT> @0x84535E8,
// Stream::GetInBuffer<SIG_SAVE_CONDITION_EVENT> @0x86A19EE.
struct SIG_SAVE_CONDITION_EVENT {
    uint32_t charac_no;       // +0x00
    int16_t current_step;     // +0x04
    int16_t reward_step;      // +0x06
};

// IDA_EA: Stream::GetOutBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT> @0x8453990,
// Stream::GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT> @0x86A1A58.
// The old stream contract is 8 bytes: natural i386 tail alignment is
// observed, rather than a guessed packed six-byte record.
struct SIG_SAVE_BLOOD_CLEAR_COUNT {
    uint32_t charac_no;       // +0x00
    int16_t clear_count;      // +0x04
};

// IDA_EA: Stream GetIn/GetOut @0x8455A5A/@0x8451E30 use 24 bytes.
// Natural i386 tail alignment is intentional; do not pack this record.
struct SIG_GOBLIN_UPDATE_REWARD {
    uint32_t member_id;       // +0x00
    char modification_type;   // +0x04
    bool reward;              // +0x05
    char ip_info[16];         // +0x06
};

// IDA_EA: Stream GetIn/GetOut @0x845660E/@0x84528C6 use 12 bytes.
struct SIG_INSERT_0712_WINTER_EVENT_COUPON {
    uint32_t member_id;       // +0x00
    uint32_t charac_no;       // +0x04
    uint32_t coupon_count;    // +0x08
};

// IDA_EA: Stream GetIn/GetOut @0x8456678/@0x8452930 use 12 bytes.
struct SIG_INSERT_ACHIEVE_EVENT {
    // The named enum type is required by the observed makeRequest ABI.
    // Enumerators are not present in the available old-IDB evidence.
    enum ENUM_ACHIEVE_TYPE : int {};

    uint32_t member_id;                // +0x00
    uint32_t charac_no;                // +0x04
    ENUM_ACHIEVE_TYPE achieve_type;    // +0x08
};

// IDA_EA: Stream GetIn/GetOut @0x86A25B2/@0x8453FB0 use 24 bytes.
struct SIG_STATISTIC_CERASHOP_GIFT_LOG {
    uint32_t from_member_id;   // +0x00
    uint32_t from_charac_no;   // +0x04
    uint32_t to_member_id;     // +0x08
    uint32_t to_charac_no;     // +0x0c
    uint32_t item_index;       // +0x10
    uint32_t item_count;       // +0x14
};

// IDA_EA: Stream::GetOutBuffer<SIG_CERASHOP_ERROR_LOG> @0x845491E.
// Natural i386 layout preserves the observed +0x12 alignment gap and
// 28-byte physical stream record.
struct SIG_CERASHOP_ERROR_LOG {
    uint32_t member_id;        // +0x00
    uint32_t charac_no;        // +0x04
    uint32_t server_id;        // +0x08
    uint32_t channel_no;       // +0x0c
    int16_t type;              // +0x10
    uint32_t error_code;       // +0x14
    int32_t goods_id;          // +0x18

    SIG_CERASHOP_ERROR_LOG();
};

static_assert(sizeof(SIG_SAVE_CONDITION_EVENT) == 8u,
              "SIG_SAVE_CONDITION_EVENT IDA ABI size must stay 8");
static_assert(sizeof(SIG_SAVE_BLOOD_CLEAR_COUNT) == 8u,
              "SIG_SAVE_BLOOD_CLEAR_COUNT IDA ABI size must stay 8");
static_assert(sizeof(SIG_GOBLIN_UPDATE_REWARD) == 24u,
              "SIG_GOBLIN_UPDATE_REWARD IDA ABI size must stay 24");
static_assert(sizeof(SIG_INSERT_0712_WINTER_EVENT_COUPON) == 12u,
              "SIG_INSERT_0712_WINTER_EVENT_COUPON IDA ABI size must stay 12");
static_assert(sizeof(SIG_INSERT_ACHIEVE_EVENT) == 12u,
              "SIG_INSERT_ACHIEVE_EVENT IDA ABI size must stay 12");
static_assert(sizeof(SIG_STATISTIC_CERASHOP_GIFT_LOG) == 24u,
              "SIG_STATISTIC_CERASHOP_GIFT_LOG IDA ABI size must stay 24");
static_assert(sizeof(SIG_CERASHOP_ERROR_LOG) == 28u,
              "SIG_CERASHOP_ERROR_LOG IDA ABI size must stay 28");
static_assert(offsetof(SIG_SAVE_CONDITION_EVENT, current_step) == 4u,
              "SIG_SAVE_CONDITION_EVENT current_step offset must stay 4");
static_assert(offsetof(SIG_SAVE_BLOOD_CLEAR_COUNT, clear_count) == 4u,
              "SIG_SAVE_BLOOD_CLEAR_COUNT clear_count offset must stay 4");
static_assert(offsetof(SIG_GOBLIN_UPDATE_REWARD, ip_info) == 6u,
              "SIG_GOBLIN_UPDATE_REWARD ip_info offset must stay 6");
static_assert(offsetof(SIG_CERASHOP_ERROR_LOG, error_code) == 20u,
              "SIG_CERASHOP_ERROR_LOG error_code offset must stay 20");
static_assert(offsetof(SIG_CERASHOP_ERROR_LOG, goods_id) == 24u,
              "SIG_CERASHOP_ERROR_LOG goods_id offset must stay 24");

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCH_PAYLOADS_H_
