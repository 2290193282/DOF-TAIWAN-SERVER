#include "old_data/Login/security/auto_punish_black_ip_manager.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>


namespace WongWork {

// IDA_EA: 0x82A5A74
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr5resetEv
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::reset(void)
int CAutoPunishRuleBlackIPMgr::reset() {
    black_ips_.clear();
    return 0;
}

// IDA_EA: 0x82A5A88
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgrC2Ev
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::CAutoPunishRuleBlackIPMgr(void)
CAutoPunishRuleBlackIPMgr::CAutoPunishRuleBlackIPMgr()
    : black_ips_() {}

// IDA_EA: 0x84EB598
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr10stIPInfo_tC2Ehh
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::stIPInfo_t::stIPInfo_t(unsigned char,unsigned char)
CAutoPunishRuleBlackIPMgr::stIPInfo_t::stIPInfo_t(unsigned char start,
                                                   unsigned char end)
    : start_ip(start),
      end_ip(end) {}

// IDA_EA: 0x84EB5C0
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr10addBlackIPEmhh
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::addBlackIP(unsigned long,unsigned char,unsigned char)
int CAutoPunishRuleBlackIPMgr::addBlackIP(unsigned long c_class_ip,
                                          unsigned char start_ip,
                                          unsigned char end_ip) {
    black_ips_.insert(std::make_pair(c_class_ip,
                                     stIPInfo_t(start_ip, end_ip)));
    return 1;
}

// IDA_EA: 0x84EB63E
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr9isBlackIPEPKc
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::isBlackIP(char const*)
int CAutoPunishRuleBlackIPMgr::isBlackIP(const char* ip_address) {
    char c_class_text[18];
    ::memset(c_class_text, 0, sizeof(c_class_text));
    ::strncpy(c_class_text, ip_address, 16);

    in_addr_t c_class_ip = 0;
    unsigned char host_ip = 0;
    for (int index = 15; index >= 0; --index) {
        if (c_class_text[index] == '.') {
            char host_text[4];
            ::memset(host_text, 0, sizeof(host_text));
            ::strcpy(host_text, c_class_text + index + 1);
            host_ip = static_cast<unsigned char>(::atoi(host_text));
            c_class_text[index + 1] = '0';
            c_class_text[index + 2] = 0;
            c_class_ip = ::inet_addr(c_class_text);
            break;
        }
    }

    const std::pair<std::multimap<unsigned long, stIPInfo_t>::const_iterator,
                    std::multimap<unsigned long, stIPInfo_t>::const_iterator>
        range = black_ips_.equal_range(c_class_ip);
    for (std::multimap<unsigned long, stIPInfo_t>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        if (it->second.start_ip <= host_ip && it->second.end_ip >= host_ip) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x84EB7CA
// IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr15getBlackIPCountEv
// DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::getBlackIPCount(void)
int CAutoPunishRuleBlackIPMgr::getBlackIPCount() {
    return static_cast<int>(black_ips_.size());
}

}  // namespace WongWork
