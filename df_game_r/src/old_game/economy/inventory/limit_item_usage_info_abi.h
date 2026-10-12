#ifndef OLDSRV_GLOBAL_LIMIT_ITEM_USAGE_INFO_ABI_H_
#define OLDSRV_GLOBAL_LIMIT_ITEM_USAGE_INFO_ABI_H_

#include <stdint.h>

#include <vector>


// IDA unit: 99_misc_global/stLimitItemUsageInfo and nested row values.
// IDA_EA: stLimitItemUsageInfo @0x837C6BE/0x837C6D2; Add @0x891B4A8.
struct stLimitItemUsageInfo {
    struct LimitInfo {
        ~LimitInfo();

        uint8_t flag;
        uint8_t pad_01[3];
        int32_t item_id;
        int32_t usage_count;
    };

    stLimitItemUsageInfo();
    ~stLimitItemUsageInfo();

    void Add(char flag, int item_id, int usage_count);

    std::vector<LimitInfo> values;
};

static_assert(sizeof(stLimitItemUsageInfo::LimitInfo) == 0x0Cu,
              "LimitInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(stLimitItemUsageInfo) == 0x0Cu,
              "stLimitItemUsageInfo IDA ABI size must stay 0x0C");

// IDA unit: 99_misc_global/stResetItemInfo and nested row values.
struct stResetItemInfo {
    struct ResetItem {
        ~ResetItem();

        int32_t item_id;
        int32_t reset_time;
    };

    stResetItemInfo();
    ~stResetItemInfo();

    void Add(int item_id, int reset_time);

    std::vector<ResetItem> values;
};

static_assert(sizeof(stResetItemInfo::ResetItem) == 0x08u,
              "ResetItem IDA ABI size must stay 0x08");
static_assert(sizeof(stResetItemInfo) == 0x0Cu,
              "stResetItemInfo IDA ABI size must stay 0x0C");

// IDA unit: 99_misc_global/stRefillItemInfo and nested row values.
struct stRefillItemInfo {
    struct RefillItem {
        ~RefillItem();

        int32_t item_id;
        int32_t refill_count;
        int32_t refill_time;
    };

    stRefillItemInfo();
    ~stRefillItemInfo();

    void Add(int item_id, int refill_count, int refill_time);

    std::vector<RefillItem> values;
};

static_assert(sizeof(stRefillItemInfo::RefillItem) == 0x0Cu,
              "RefillItem IDA ABI size must stay 0x0C");
static_assert(sizeof(stRefillItemInfo) == 0x0Cu,
              "stRefillItemInfo IDA ABI size must stay 0x0C");

// IDA unit: 99_misc_global/stLimitItemUsageInfoEx.
struct stLimitItemUsageInfoEx {
    stLimitItemUsageInfoEx();
    ~stLimitItemUsageInfoEx();

    stLimitItemUsageInfo limit_item_usage_info;
    stResetItemInfo reset_item_info;
    stRefillItemInfo refill_item_info;
};

static_assert(sizeof(stLimitItemUsageInfoEx) == 0x24u,
              "stLimitItemUsageInfoEx IDA ABI size must stay 0x24");


#endif  // OLDSRV_GLOBAL_LIMIT_ITEM_USAGE_INFO_ABI_H_
