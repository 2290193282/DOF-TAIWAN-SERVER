#include "old_game/economy/inventory/limit_item_usage_info_abi.h"


// IDA_EA: 0x89352AC
// IDA_NAME: _ZN20stLimitItemUsageInfo9LimitInfoD2Ev
// DEMANGLED: stLimitItemUsageInfo::LimitInfo::~LimitInfo()
stLimitItemUsageInfo::LimitInfo::~LimitInfo() {
}

// IDA_EA: 0x837C6BE
// IDA_NAME: _ZN20stLimitItemUsageInfoC2Ev
// DEMANGLED: stLimitItemUsageInfo::stLimitItemUsageInfo(void)
stLimitItemUsageInfo::stLimitItemUsageInfo() : values() {
}

// IDA_EA: 0x837C6D2
// IDA_NAME: _ZN20stLimitItemUsageInfoD2Ev
// DEMANGLED: stLimitItemUsageInfo::~stLimitItemUsageInfo()
stLimitItemUsageInfo::~stLimitItemUsageInfo() {
}

// IDA_EA: 0x891B4A8
// IDA_NAME: _ZN20stLimitItemUsageInfo3AddEcii
// DEMANGLED: stLimitItemUsageInfo::Add(char,int,int)
void stLimitItemUsageInfo::Add(char flag, int item_id, int usage_count) {
    LimitInfo info;
    info.flag = static_cast<uint8_t>(flag);
    info.pad_01[0] = 0;
    info.pad_01[1] = 0;
    info.pad_01[2] = 0;
    info.item_id = item_id;
    info.usage_count = usage_count;
    values.push_back(info);
}

// IDA_EA: 0x89354EC
// IDA_NAME: _ZN15stResetItemInfo9ResetItemD2Ev
// DEMANGLED: stResetItemInfo::ResetItem::~ResetItem()
stResetItemInfo::ResetItem::~ResetItem() {
}

// IDA_EA: 0x837C6E6
// IDA_NAME: _ZN15stResetItemInfoC2Ev
// DEMANGLED: stResetItemInfo::stResetItemInfo(void)
stResetItemInfo::stResetItemInfo() : values() {
}

// IDA_EA: 0x837C6FA
// IDA_NAME: _ZN15stResetItemInfoD2Ev
// DEMANGLED: stResetItemInfo::~stResetItemInfo()
stResetItemInfo::~stResetItemInfo() {
}

// IDA_EA: 0x891B4DC
// IDA_NAME: _ZN15stResetItemInfo3AddEil
// DEMANGLED: stResetItemInfo::Add(int,long)
void stResetItemInfo::Add(int item_id, int reset_time) {
    ResetItem info;
    info.item_id = item_id;
    info.reset_time = reset_time;
    values.push_back(info);
}

// IDA_EA: 0x8935742
// IDA_NAME: _ZN16stRefillItemInfo10RefillItemD2Ev
// DEMANGLED: stRefillItemInfo::RefillItem::~RefillItem()
stRefillItemInfo::RefillItem::~RefillItem() {
}

// IDA_EA: 0x837C70E
// IDA_NAME: _ZN16stRefillItemInfoC2Ev
// DEMANGLED: stRefillItemInfo::stRefillItemInfo(void)
stRefillItemInfo::stRefillItemInfo() : values() {
}

// IDA_EA: 0x837C722
// IDA_NAME: _ZN16stRefillItemInfoD2Ev
// DEMANGLED: stRefillItemInfo::~stRefillItemInfo()
stRefillItemInfo::~stRefillItemInfo() {
}

// IDA_EA: 0x891B502
// IDA_NAME: _ZN16stRefillItemInfo3AddEiil
// DEMANGLED: stRefillItemInfo::Add(int,int,long)
void stRefillItemInfo::Add(int item_id,
                           int refill_count,
                           int refill_time) {
    RefillItem info;
    info.item_id = item_id;
    info.refill_count = refill_count;
    info.refill_time = refill_time;
    values.push_back(info);
}

// IDA_EA: 0x837C736
// IDA_NAME: _ZN22stLimitItemUsageInfoExC2Ev
// DEMANGLED: stLimitItemUsageInfoEx::stLimitItemUsageInfoEx(void)
stLimitItemUsageInfoEx::stLimitItemUsageInfoEx()
    : limit_item_usage_info(), reset_item_info(), refill_item_info() {
}

// IDA_EA: 0x837C7A2
// IDA_NAME: _ZN22stLimitItemUsageInfoExD2Ev
// DEMANGLED: stLimitItemUsageInfoEx::~stLimitItemUsageInfoEx()
stLimitItemUsageInfoEx::~stLimitItemUsageInfoEx() {
}
