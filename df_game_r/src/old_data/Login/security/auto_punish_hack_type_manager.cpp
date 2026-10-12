#include "old_data/Login/security/auto_punish_hack_type_manager.h"

#include <cstring>
#include <map>
#include <utility>

#include "infra/network/transport/ip_address_split_abi.h"


namespace WongWork {

// IDA_EA: 0x80F982A
// IDA_NAME: _ZN8WongWork25stHacktypeExceptionInfo_tC2Ess
// DEMANGLED: WongWork::stHacktypeExceptionInfo_t::stHacktypeExceptionInfo_t(short,short)
stHacktypeExceptionInfo_t::stHacktypeExceptionInfo_t(
    int16_t exception_count_value,
    int16_t accumulated_count_value)
    : exception_count(exception_count_value),
      accumulated_count(accumulated_count_value) {}

// IDA_EA: 0x80F9892
// IDA_NAME: _ZN8WongWork15HackIPCounter_tC2EjjPcj
// DEMANGLED: WongWork::HackIPCounter_t::HackIPCounter_t(unsigned int,unsigned int,char *,unsigned int)
HackIPCounter_t::HackIPCounter_t(uint32_t hack_type_value,
                                 uint32_t hack_sub_type_value,
                                 char* c_class_ip_value,
                                 uint32_t ip_count_value)
    : hack_type(static_cast<uint16_t>(hack_type_value)),
      hack_sub_type(static_cast<uint16_t>(hack_sub_type_value)),
      ip_count(ip_count_value) {
    std::memset(c_class_ip, 0, sizeof(c_class_ip));
    std::strncpy(c_class_ip, c_class_ip_value, sizeof(c_class_ip));
}

// IDA_EA: 0x80F98F6
// IDA_NAME: _ZN8WongWork19HackFullIPCounter_tC2EjjPcj
// DEMANGLED: WongWork::HackFullIPCounter_t::HackFullIPCounter_t(unsigned int,unsigned int,char *,unsigned int)
HackFullIPCounter_t::HackFullIPCounter_t(uint32_t hack_type_value,
                                         uint32_t hack_sub_type_value,
                                         char* full_ip_value,
                                         uint32_t ip_count_value)
    : hack_type(static_cast<uint16_t>(hack_type_value)),
      hack_sub_type(static_cast<uint16_t>(hack_sub_type_value)),
      ip_count(ip_count_value) {
    std::memset(full_ip, 0, sizeof(full_ip));
    std::strncpy(full_ip, full_ip_value, sizeof(full_ip));
}

// IDA_EA: 0x84EB7DE
// IDA_NAME: _ZN8WongWork14HackTypeInfo_tC2ENS_13ENUM_HACKTYPEEtS1_tttj
// DEMANGLED: WongWork::HackTypeInfo_t::HackTypeInfo_t(WongWork::ENUM_HACKTYPE,unsigned short,WongWork::ENUM_HACKTYPE,unsigned short,unsigned short,unsigned short,unsigned int)
HackTypeInfo_t::HackTypeInfo_t(ENUM_HACKTYPE hack_type_value,
                               uint16_t count_value,
                               ENUM_HACKTYPE hack_sub_type_value,
                               uint16_t hack_sub_count_value,
                               uint16_t apply_flag_value,
                               uint16_t ip_count_value,
                               uint32_t etc_low_value)
    : hack_type(static_cast<uint16_t>(hack_type_value)),
      count(count_value),
      hack_sub_type(static_cast<uint16_t>(hack_sub_type_value)),
      hack_sub_count(hack_sub_count_value),
      apply_flag(apply_flag_value),
      ip_count(ip_count_value),
      etc_low(etc_low_value) {}

// IDA_EA: 0x86952B8
// IDA_NAME: _ZN8WongWork14HackTypeInfo_tC2Ev
// DEMANGLED: WongWork::HackTypeInfo_t::HackTypeInfo_t(void)
HackTypeInfo_t::HackTypeInfo_t()
    : hack_type(0),
      count(0),
      hack_sub_type(0),
      hack_sub_count(0),
      apply_flag(0),
      ip_count(0),
      etc_low(0) {}

// IDA_EA: 0x86952FC
// IDA_NAME: _ZN8WongWork14HackTypeInfo_t10CopyStructERKS0_
// DEMANGLED: WongWork::HackTypeInfo_t::CopyStruct(WongWork::HackTypeInfo_t const&)
HackTypeInfo_t* HackTypeInfo_t::CopyStruct(const HackTypeInfo_t& other) {
    hack_type = other.hack_type;
    count = other.count;
    hack_sub_type = other.hack_sub_type;
    hack_sub_count = other.hack_sub_count;
    apply_flag = other.apply_flag;
    ip_count = other.ip_count;
    etc_low = other.etc_low;
    return this;
}

// IDA_EA: 0x82A5B1A
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgrC2Ev
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::CAutoPunishRuleHackTypeMgr(void)
CAutoPunishRuleHackTypeMgr::CAutoPunishRuleHackTypeMgr()
    : hack_rules_(),
      ip_counters_(),
      full_ip_counters_(),
      exception_rules_() {
    exception_rules_.insert(std::make_pair(131, stHacktypeExceptionInfo_t(20, 0)));
    exception_rules_.insert(std::make_pair(202, stHacktypeExceptionInfo_t(40, 0)));
    exception_rules_.insert(std::make_pair(205, stHacktypeExceptionInfo_t(10, 0)));
    exception_rules_.insert(std::make_pair(206, stHacktypeExceptionInfo_t(5, 0)));
    exception_rules_.insert(std::make_pair(208, stHacktypeExceptionInfo_t(200, 0)));
    exception_rules_.insert(std::make_pair(209, stHacktypeExceptionInfo_t(150, 0)));
    exception_rules_.insert(std::make_pair(211, stHacktypeExceptionInfo_t(100, 0)));
    exception_rules_.insert(std::make_pair(215, stHacktypeExceptionInfo_t(50, 1000)));
    exception_rules_.insert(std::make_pair(216, stHacktypeExceptionInfo_t(200, 1000)));
    exception_rules_.insert(std::make_pair(305, stHacktypeExceptionInfo_t(30, 0)));
    exception_rules_.insert(std::make_pair(403, stHacktypeExceptionInfo_t(10, 0)));
    exception_rules_.insert(std::make_pair(411, stHacktypeExceptionInfo_t(50, 0)));
    exception_rules_.insert(std::make_pair(412, stHacktypeExceptionInfo_t(20, 0)));
    exception_rules_.insert(std::make_pair(832, stHacktypeExceptionInfo_t(5, 0)));
    exception_rules_.insert(std::make_pair(220, stHacktypeExceptionInfo_t(100, 0)));
    exception_rules_.insert(std::make_pair(134, stHacktypeExceptionInfo_t(50, 0)));
    exception_rules_.insert(std::make_pair(224, stHacktypeExceptionInfo_t(0, 20)));
    exception_rules_.insert(std::make_pair(225, stHacktypeExceptionInfo_t(0, 50)));
}

// IDA_EA: 0x82A5A9C
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr5resetEv
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::reset(void)
CAutoPunishRuleHackTypeMgr* CAutoPunishRuleHackTypeMgr::reset() {
    hack_rules_.clear();
    total_trade_count_ = 0;
    total_trade_etc_ = 0;
    return this;
}

// IDA_EA: 0x84EB854
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr10setHackCntENS_13ENUM_HACKTYPEEjS1_jjjj
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::setHackCnt(WongWork::ENUM_HACKTYPE,unsigned int,WongWork::ENUM_HACKTYPE,unsigned int,unsigned int,unsigned int,unsigned int)
void CAutoPunishRuleHackTypeMgr::setHackCnt(ENUM_HACKTYPE hack_type_value,
                                            uint32_t count_value,
                                            ENUM_HACKTYPE hack_sub_type_value,
                                            uint32_t hack_sub_count_value,
                                            uint32_t apply_flag_value,
                                            uint32_t ip_count_value,
                                            uint32_t etc_low_value) {
    hack_rules_.insert(std::make_pair(
        hack_type_value,
        HackTypeInfo_t(hack_type_value,
                       static_cast<uint16_t>(count_value),
                       hack_sub_type_value,
                       static_cast<uint16_t>(hack_sub_count_value),
                       static_cast<uint16_t>(apply_flag_value),
                       static_cast<uint16_t>(ip_count_value),
                       etc_low_value)));
    if (hack_sub_type_value) {
        hack_rules_.insert(std::make_pair(
            hack_sub_type_value,
            HackTypeInfo_t(hack_type_value,
                           static_cast<uint16_t>(count_value),
                           hack_sub_type_value,
                           static_cast<uint16_t>(hack_sub_count_value),
                           static_cast<uint16_t>(apply_flag_value),
                           static_cast<uint16_t>(ip_count_value),
                           etc_low_value)));
    }
}

// IDA_EA: 0x84EB9D4
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr13setTotalTradeEjy
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::setTotalTrade(unsigned int,unsigned long long)
uint64_t CAutoPunishRuleHackTypeMgr::setTotalTrade(uint32_t total_count,
                                                    uint64_t total_etc) {
    total_trade_count_ = total_count;
    total_trade_etc_ = total_etc;
    return total_etc;
}

// IDA_EA: 0x8531FD2
// IDA_NAME: _ZNK8WongWork26CAutoPunishRuleHackTypeMgr23checkTotalTradeRuleOverEjy
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::checkTotalTradeRuleOver(unsigned int,unsigned long long)const
bool CAutoPunishRuleHackTypeMgr::checkTotalTradeRuleOver(
    uint32_t total_count,
    uint64_t total_etc) const {
    if (!total_trade_count_ && !total_trade_etc_) {
        return false;
    }
    return total_trade_count_ <= total_count && total_trade_etc_ <= total_etc;
}

// IDA_EA: 0x80F9218
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr16setIPCounterDataEjjPcj
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::setIPCounterData(unsigned int,unsigned int,char *,unsigned int)
void CAutoPunishRuleHackTypeMgr::setIPCounterData(uint32_t hack_type_value,
                                                  uint32_t hack_sub_type_value,
                                                  char* c_class_ip_value,
                                                  uint32_t ip_count_value) {
    ip_counters_.insert(std::make_pair(
        static_cast<uint16_t>(hack_type_value),
        HackIPCounter_t(hack_type_value,
                        hack_sub_type_value,
                        c_class_ip_value,
                        ip_count_value)));
    if (hack_sub_type_value) {
        ip_counters_.insert(std::make_pair(
            static_cast<uint16_t>(hack_sub_type_value),
            HackIPCounter_t(hack_type_value,
                            hack_sub_type_value,
                            c_class_ip_value,
                            ip_count_value)));
    }
}

// IDA_EA: 0x80F9322
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr12_findIPCountEjjPKc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::_findIPCount(unsigned int,unsigned int,char const*)
int CAutoPunishRuleHackTypeMgr::_findIPCount(uint32_t hack_type_value,
                                             uint32_t hack_sub_type_value,
                                             const char* ip_address) {
    const uint16_t key = static_cast<uint16_t>(hack_type_value);
    const std::pair<std::multimap<uint16_t, HackIPCounter_t>::const_iterator,
                    std::multimap<uint16_t, HackIPCounter_t>::const_iterator>
        range = ip_counters_.equal_range(key);
    for (std::multimap<uint16_t, HackIPCounter_t>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        const HackIPCounter_t& counter = it->second;
        if (counter.hack_sub_type != static_cast<uint16_t>(hack_sub_type_value)) {
            continue;
        }
        char c_class_ip[16] = {};
        Divide_C_ClassIP_EndIP(c_class_ip, 0, const_cast<char*>(ip_address));
        if (c_class_ip[0] && std::strncmp(c_class_ip, counter.c_class_ip, 12) == 0) {
            return static_cast<int>(counter.ip_count);
        }
    }
    return 0;
}

// IDA_EA: 0x80F946A
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr20setFullIPCounterDataEjjPcj
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::setFullIPCounterData(unsigned int,unsigned int,char *,unsigned int)
void CAutoPunishRuleHackTypeMgr::setFullIPCounterData(uint32_t hack_type_value,
                                                      uint32_t hack_sub_type_value,
                                                      char* full_ip_value,
                                                      uint32_t ip_count_value) {
    full_ip_counters_.insert(std::make_pair(
        static_cast<uint16_t>(hack_type_value),
        HackFullIPCounter_t(hack_type_value,
                            hack_sub_type_value,
                            full_ip_value,
                            ip_count_value)));
    if (hack_sub_type_value) {
        full_ip_counters_.insert(std::make_pair(
            static_cast<uint16_t>(hack_sub_type_value),
            HackFullIPCounter_t(hack_type_value,
                                hack_sub_type_value,
                                full_ip_value,
                                ip_count_value)));
    }
}

// IDA_EA: 0x80F9574
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr16_findFullIPCountEjjPKc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::_findFullIPCount(unsigned int,unsigned int,char const*)
int CAutoPunishRuleHackTypeMgr::_findFullIPCount(uint32_t hack_type_value,
                                                 uint32_t hack_sub_type_value,
                                                 const char* full_ip_value) {
    const uint16_t key = static_cast<uint16_t>(hack_type_value);
    const std::pair<std::multimap<uint16_t, HackFullIPCounter_t>::const_iterator,
                    std::multimap<uint16_t, HackFullIPCounter_t>::const_iterator>
        range = full_ip_counters_.equal_range(key);
    for (std::multimap<uint16_t, HackFullIPCounter_t>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        const HackFullIPCounter_t& counter = it->second;
        if (counter.hack_sub_type == static_cast<uint16_t>(hack_sub_type_value) &&
            std::strncmp(full_ip_value, counter.full_ip, 16) == 0) {
            return static_cast<int>(counter.ip_count);
        }
    }
    return 0;
}

// IDA_EA: 0x84EBA00
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr18ResetIpCounterDataEv
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::ResetIpCounterData(void)
int CAutoPunishRuleHackTypeMgr::ResetIpCounterData() {
    ip_counters_.clear();
    return static_cast<int>(reinterpret_cast<uintptr_t>(&ip_counters_));
}

// IDA_EA: 0x84EBA16
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr22ResetFullIpCounterDataEv
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::ResetFullIpCounterData(void)
int CAutoPunishRuleHackTypeMgr::ResetFullIpCounterData() {
    full_ip_counters_.clear();
    return static_cast<int>(reinterpret_cast<uintptr_t>(&full_ip_counters_));
}

// IDA_EA: 0x80F972A
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr15GetExceptionCntEi
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::GetExceptionCnt(int)
stHacktypeExceptionInfo_t CAutoPunishRuleHackTypeMgr::GetExceptionCnt(
    int hack_type_value) {
    const std::map<int, stHacktypeExceptionInfo_t>::const_iterator found =
        exception_rules_.find(hack_type_value);
    if (found != exception_rules_.end()) {
        return found->second;
    }
    return stHacktypeExceptionInfo_t(0, 0);
}

}  // namespace WongWork
