#include "old_data/db/event/reward_event_request.h"

#include <cstring>

namespace arad {

// IDA_EA: 0x8195CD4
// IDA_NAME: _ZN4arad22SigSaveRewardEventItemC2Ev
// DEMANGLED: arad::SigSaveRewardEventItem::SigSaveRewardEventItem(void)
SigSaveRewardEventItem::SigSaveRewardEventItem() {
    init();
}

// IDA_EA: 0x8195CE8
// IDA_NAME: _ZN4arad22SigSaveRewardEventItem4initEv
// DEMANGLED: arad::SigSaveRewardEventItem::init(void)
SigSaveRewardEventItem* SigSaveRewardEventItem::init() {
    return set(0, 0, 0, 0, 0, 0);
}

// IDA_EA: 0x8195D2C
// IDA_NAME: _ZN4arad22SigSaveRewardEventItem3setEijjmsj
// DEMANGLED: arad::SigSaveRewardEventItem::set(int,unsigned int,unsigned int,unsigned long,short,unsigned int)
SigSaveRewardEventItem* SigSaveRewardEventItem::set(
    int event_code_value,
    uint32_t dword_04_value,
    uint32_t dword_08_value,
    unsigned long dword_10_value,
    int16_t word_14_value,
    uint32_t dword_0c_value) {
    event_code = event_code_value;
    dword_04 = dword_04_value;
    dword_08 = dword_08_value;
    dword_10 = dword_10_value;
    word_14 = word_14_value;
    dword_0c = dword_0c_value;
    return this;
}

// IDA_EA: 0x8186686
// IDA_NAME: _ZN4arad22SigLoadRewardEventItemC2Ev
// DEMANGLED: arad::SigLoadRewardEventItem::SigLoadRewardEventItem(void)
SigLoadRewardEventItem::SigLoadRewardEventItem() {
    init();
}

// IDA_EA: 0x818669A
// IDA_NAME: _ZN4arad22SigLoadRewardEventItem4initEv
// DEMANGLED: arad::SigLoadRewardEventItem::init(void)
SigLoadRewardEventItem* SigLoadRewardEventItem::init() {
    return set(0);
}

// IDA_EA: 0x81866B6
// IDA_NAME: _ZN4arad22SigLoadRewardEventItem3setEj
// DEMANGLED: arad::SigLoadRewardEventItem::set(unsigned int)
SigLoadRewardEventItem* SigLoadRewardEventItem::set(
    uint32_t account_id_value) {
    account_id = account_id_value;
    reward_count = 0;
    std::memset(charac_nos, 0, sizeof(charac_nos));
    std::memset(event_codes, 0, sizeof(event_codes));
    return this;
}

// IDA_EA: 0x8694F14
// IDA_NAME: _ZN4arad19SigAradEventItemLogC2Ev
// DEMANGLED: arad::SigAradEventItemLog::SigAradEventItemLog(void)
SigAradEventItemLog::SigAradEventItemLog() {
    init();
}

// IDA_EA: 0x8694F28
// IDA_NAME: _ZN4arad19SigAradEventItemLog4initEv
// DEMANGLED: arad::SigAradEventItemLog::init(void)
SigAradEventItemLog* SigAradEventItemLog::init() {
    item_id = 0;
    item_count = 0;
    return this;
}

}  // namespace arad
