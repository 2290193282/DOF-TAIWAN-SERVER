#ifndef OLDSRV_11_EVENT_RUNTIME_MULTI_BOX_LOTTERY_INFO_H_
#define OLDSRV_11_EVENT_RUNTIME_MULTI_BOX_LOTTERY_INFO_H_

#include <stdint.h>

#include <utility>
#include <vector>

// IDA unit: 11_event_runtime/MultiBoxLotteryInfo.
// CStackableItem::C2/D2 @ 0x8515762/0x85146B4 construct and destroy this
// exact child at +0x1e4.  Keeping the type outside event_value_types.h makes
// that physical ownership available without an inventory/event include cycle.
struct MultiBoxLotteryInfo {
    struct LotteryDefaultArad {
        LotteryDefaultArad(unsigned int item_id_value,
                           unsigned int count_value,
                           unsigned int rate_value);
        ~LotteryDefaultArad();

        unsigned int item_id;
        unsigned int count;
        unsigned int rate;
    };

    struct stDefaultItem {
        stDefaultItem();
        ~stDefaultItem();

        uint32_t first_dword;
        uint32_t second_dword;
        uint32_t third_dword;
        uint8_t flag;
        uint8_t reserved[3];
    };

    struct stDefaultItemInfo {
        stDefaultItemInfo();
        ~stDefaultItemInfo();

        uint32_t first_dword;
        uint32_t second_dword;
        uint32_t third_dword;
        uint8_t flag;
        uint8_t reserved[3];
        std::vector<stDefaultItem> default_items;
    };

    struct LotteryUnit {
        LotteryUnit();
        LotteryUnit(const LotteryUnit& rhs);
        ~LotteryUnit();

        LotteryUnit* clear();
        LotteryUnit& operator=(const LotteryUnit& rhs);

        uint32_t first_dword;
        uint32_t second_dword;
        uint32_t third_dword;
        uint8_t flag;
        uint8_t reserved[3];
        std::vector<std::pair<int, int> > first_pairs;
        std::vector<std::pair<int, int> > second_pairs;
    };

    MultiBoxLotteryInfo();
    ~MultiBoxLotteryInfo();

    MultiBoxLotteryInfo* clear();

    // 0627 CUser::check_locked_multibox_error reads DWORDs 123/124 from
    // CStackableItem.  The child starts at +0x1e4 (DWORD 121), therefore the
    // PVF [sealing removal item] pair lives at child +0x08/+0x0c.  Keep the
    // recovered storage names for ABI stability, but expose the semantics so
    // callers cannot accidentally consume child +0x00/+0x08 as a pair.
    int required_material_item_id() const { return third_dword; }
    uint32_t required_material_count() const { return fourth_dword; }

    int item_id;
    uint32_t second_dword;
    int third_dword;
    uint32_t fourth_dword;
    std::vector<LotteryUnit> lottery_units;
    stDefaultItemInfo default_item_info;
};

#if defined(__i386__)
static_assert(sizeof(MultiBoxLotteryInfo) == 0x38u,
              "MultiBoxLotteryInfo must retain its IDA i386 layout");
#endif

#endif  // OLDSRV_11_EVENT_RUNTIME_MULTI_BOX_LOTTERY_INFO_H_
