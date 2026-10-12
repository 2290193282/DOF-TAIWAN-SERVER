#include "old_data/PVF/arad/datamanager/arad_event_reward.h"

namespace ARAD {

// IDA_EA: 0x8195D72
// IDA_NAME: _ZN4ARAD17SearchEventPeriodC2E22ENUM_REPEAT_EVENT_CODE
// DEMANGLED: ARAD::SearchEventPeriod::SearchEventPeriod(ENUM_REPEAT_EVENT_CODE)
SearchEventPeriod::SearchEventPeriod(ENUM_REPEAT_EVENT_CODE event_code_value)
    : event_code(event_code_value) {}

// IDA_EA: 0x8195D80
// IDA_NAME: _ZNK4ARAD17SearchEventPeriodclERKNS_6SCRIPT16SEventPeriodDataE
// DEMANGLED: ARAD::SearchEventPeriod::operator()(ARAD::SCRIPT::SEventPeriodData const&)const
bool SearchEventPeriod::operator()(const SCRIPT::SEventPeriodData& data) const {
    return data.event_code == static_cast<int>(event_code);
}

// IDA_EA: 0x819A8B4
// IDA_NAME: _ZNK4ARAD17SearchEventPeriodclERKNS_6SCRIPT17SRewardPeriodDataE
// DEMANGLED: ARAD::SearchEventPeriod::operator()(ARAD::SCRIPT::SRewardPeriodData const&)const
bool SearchEventPeriod::operator()(const SCRIPT::SRewardPeriodData& data) const {
    return data.event_code == static_cast<int>(event_code);
}

// IDA_EA: 0x8195D94
// IDA_NAME: _ZN4ARAD19EventRewardItemAradC2Eijj
// DEMANGLED: ARAD::EventRewardItemArad::EventRewardItemArad(int,unsigned int,unsigned int)
EventRewardItemArad::EventRewardItemArad(int event_code_value,
                                         uint32_t account_id_value,
                                         uint32_t charac_no_value)
    : event_code(event_code_value),
      account_id(account_id_value),
      charac_no(charac_no_value) {}

// IDA_EA: 0x8195DB4
// IDA_NAME: _ZNK4ARAD19EventRewardItemAradeqERKS0_
// DEMANGLED: ARAD::EventRewardItemArad::operator==(ARAD::EventRewardItemArad const&)const
bool EventRewardItemArad::operator==(const EventRewardItemArad& rhs) const {
    return event_code == rhs.event_code &&
           account_id == rhs.account_id &&
           charac_no == rhs.charac_no;
}

}  // namespace ARAD
