#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_BLACK_IP_MANAGER_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_BLACK_IP_MANAGER_H_

#include <stdint.h>

#include <map>


namespace WongWork {

// IDA unit: 02_login_account/WongWork_CAutoPunishRuleBlackIPMgr.
// This manager is a real 0x18-byte i386 std::multimap aggregate.  It owns
// the rule lookup state consumed by login and by Inter_AutoPunishRuleBlackIP.
class CAutoPunishRuleBlackIPMgr {
public:
    struct stIPInfo_t {
        // IDA_EA: 0x84EB598
        // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr10stIPInfo_tC2Ehh
        // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::stIPInfo_t::stIPInfo_t(unsigned char,unsigned char)
        stIPInfo_t(unsigned char start, unsigned char end);

        unsigned char start_ip;
        unsigned char end_ip;
    };

    // IDA_EA: 0x82A5A88
    // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgrC2Ev
    // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::CAutoPunishRuleBlackIPMgr(void)
    CAutoPunishRuleBlackIPMgr();

    // IDA_EA: 0x82A5A74
    // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr5resetEv
    // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::reset(void)
    int reset();

    // IDA_EA: 0x84EB5C0
    // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr10addBlackIPEmhh
    // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::addBlackIP(unsigned long,unsigned char,unsigned char)
    int addBlackIP(unsigned long c_class_ip,
                   unsigned char start_ip,
                   unsigned char end_ip);

    // IDA_EA: 0x84EB63E
    // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr9isBlackIPEPKc
    // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::isBlackIP(char const*)
    int isBlackIP(const char* ip_address);

    // IDA_EA: 0x84EB7CA
    // IDA_NAME: _ZN8WongWork25CAutoPunishRuleBlackIPMgr15getBlackIPCountEv
    // DEMANGLED: WongWork::CAutoPunishRuleBlackIPMgr::getBlackIPCount(void)
    int getBlackIPCount();

private:
    std::multimap<unsigned long, stIPInfo_t> black_ips_;
};

static_assert(sizeof(CAutoPunishRuleBlackIPMgr::stIPInfo_t) == 2u,
              "old black-IP range entry must stay two bytes");
#if defined(__i386__)
static_assert(sizeof(CAutoPunishRuleBlackIPMgr) == 0x18u,
              "old black-IP manager must stay a 0x18-byte i386 multimap");
#endif

}  // namespace WongWork


#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_BLACK_IP_MANAGER_H_
