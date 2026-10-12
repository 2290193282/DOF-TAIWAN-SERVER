#ifndef OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_MANAGER_H_
#define OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "infra/security/policy/hack_analyzer_public_abi.h"

struct stLookOuts;
struct tyIPCounters;


namespace WongWork {

struct CAutoPunishRuleHackTypeMgrLayoutVerifier;

// IDA_EA: 0x80F982A
// IDA_NAME: _ZN8WongWork25stHacktypeExceptionInfo_tC2Ess
// DEMANGLED: WongWork::stHacktypeExceptionInfo_t::stHacktypeExceptionInfo_t(short,short)
struct stHacktypeExceptionInfo_t {
    stHacktypeExceptionInfo_t(int16_t exception_count,
                              int16_t accumulated_count);

    int16_t exception_count;   // +0x00
    int16_t accumulated_count; // +0x02
};

// IDA_EA: 0x80F9892
// IDA_NAME: _ZN8WongWork15HackIPCounter_tC2EjjPcj
// DEMANGLED: WongWork::HackIPCounter_t::HackIPCounter_t(unsigned int,unsigned int,char *,unsigned int)
struct HackIPCounter_t {
    HackIPCounter_t(uint32_t hack_type,
                    uint32_t hack_sub_type,
                    char* c_class_ip,
                    uint32_t ip_count);

    uint16_t hack_type;       // +0x00, constructor truncates the uint input
    uint16_t hack_sub_type;   // +0x02, constructor truncates the uint input
    char c_class_ip[13];      // +0x04, C2 zeroes/copies exactly 13 bytes
    uint32_t ip_count;        // +0x14
};

// IDA_EA: 0x80F98F6
// IDA_NAME: _ZN8WongWork19HackFullIPCounter_tC2EjjPcj
// DEMANGLED: WongWork::HackFullIPCounter_t::HackFullIPCounter_t(unsigned int,unsigned int,char *,unsigned int)
struct HackFullIPCounter_t {
    HackFullIPCounter_t(uint32_t hack_type,
                        uint32_t hack_sub_type,
                        char* full_ip,
                        uint32_t ip_count);

    uint16_t hack_type;       // +0x00, constructor truncates the uint input
    uint16_t hack_sub_type;   // +0x02, constructor truncates the uint input
    char full_ip[16];         // +0x04, C2 zeroes/copies exactly 16 bytes
    uint32_t ip_count;        // +0x14
};

struct HackTypeInfo_t {
    // IDA_EA: 0x84EB7DE
    // IDA_NAME: _ZN8WongWork14HackTypeInfo_tC2ENS_13ENUM_HACKTYPEEtS1_tttj
    HackTypeInfo_t(ENUM_HACKTYPE hack_type,
                   uint16_t count,
                   ENUM_HACKTYPE hack_sub_type,
                   uint16_t hack_sub_count,
                   uint16_t apply_flag,
                   uint16_t ip_count,
                   uint32_t etc_low);

    // IDA_EA: 0x86952B8
    // IDA_NAME: _ZN8WongWork14HackTypeInfo_tC2Ev
    HackTypeInfo_t();

    // IDA_EA: 0x86952FC
    // IDA_NAME: _ZN8WongWork14HackTypeInfo_t10CopyStructERKS0_
    HackTypeInfo_t* CopyStruct(const HackTypeInfo_t& other);

    uint16_t hack_type;       // +0x00
    uint16_t count;           // +0x02
    uint16_t hack_sub_type;   // +0x04
    uint16_t hack_sub_count;  // +0x06
    uint16_t apply_flag;      // +0x08
    uint16_t ip_count;        // +0x0a
    uint32_t etc_low;         // +0x0c
};

// Old GlobalData::Init @0x829B685 allocates exactly 0x70 bytes, constructs
// this object, stores it at @0x941F7D4, then calls reset().  The primitive
// total members deliberately remain untouched by C2: reset() owns their
// initialization, exactly as in the old code.
class CAutoPunishRuleHackTypeMgr {
public:
    // IDA_EA: 0x82A5B1A
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgrC2Ev
    CAutoPunishRuleHackTypeMgr();

    // IDA_EA: 0x82A5A9C
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr5resetEv
    CAutoPunishRuleHackTypeMgr* reset();

    // IDA_EA: 0x84EB854
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr10setHackCntENS_13ENUM_HACKTYPEEjS1_jjjj
    void setHackCnt(ENUM_HACKTYPE hack_type,
                    uint32_t count,
                    ENUM_HACKTYPE hack_sub_type,
                    uint32_t hack_sub_count,
                    uint32_t apply_flag,
                    uint32_t ip_count,
                    uint32_t etc_low);

    // IDA_EA: 0x84EB9D4
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr13setTotalTradeEjy
    uint64_t setTotalTrade(uint32_t total_count, uint64_t total_etc);

    // IDA_EA: 0x8531FD2
    // IDA_NAME: _ZNK8WongWork26CAutoPunishRuleHackTypeMgr23checkTotalTradeRuleOverEjy
    bool checkTotalTradeRuleOver(uint32_t total_count,
                                 uint64_t total_etc) const;

    // IDA_EA: 0x80F9218
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr16setIPCounterDataEjjPcj
    void setIPCounterData(uint32_t hack_type,
                          uint32_t hack_sub_type,
                          char* c_class_ip,
                          uint32_t ip_count);

    // IDA_EA: 0x80F9322
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr12_findIPCountEjjPKc
    int _findIPCount(uint32_t hack_type,
                     uint32_t hack_sub_type,
                     const char* ip_address);

    // IDA_EA: 0x80F946A
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr20setFullIPCounterDataEjjPcj
    void setFullIPCounterData(uint32_t hack_type,
                              uint32_t hack_sub_type,
                              char* full_ip,
                              uint32_t ip_count);

    // IDA_EA: 0x80F9574
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr16_findFullIPCountEjjPKc
    int _findFullIPCount(uint32_t hack_type,
                         uint32_t hack_sub_type,
                         const char* full_ip);

    // IDA_EA: 0x84EBA00
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr18ResetIpCounterDataEv
    int ResetIpCounterData();

    // IDA_EA: 0x84EBA16
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr22ResetFullIpCounterDataEv
    int ResetFullIpCounterData();

    // IDA_EA: 0x80F972A
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr15GetExceptionCntEi
    stHacktypeExceptionInfo_t GetExceptionCnt(int hack_type);

    // IDA_EA: 0x8695360
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr19checkMaxHackCntOverENS_13ENUM_HACKTYPEEjPNS_13CHackAnalyzerERNS_14HackTypeInfo_tERSt6vectorI10stLookOutsSaIS7_EERS6_I12tyIPCountersSaISB_EESA_Pc
    // DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::checkMaxHackCntOver(WongWork::ENUM_HACKTYPE,unsigned int,WongWork::CHackAnalyzer *,WongWork::HackTypeInfo_t &,std::vector<stLookOuts> &,std::vector<tyIPCounters> &,std::vector<stLookOuts> &,char *)
    bool checkMaxHackCntOver(ENUM_HACKTYPE hack_type,
                             unsigned int accumulated_count,
                             CHackAnalyzer* analyzer,
                             HackTypeInfo_t& matched_rule,
                             std::vector<stLookOuts>& lookouts,
                             std::vector<tyIPCounters>& ip_counters,
                             std::vector<stLookOuts>& lookout_history,
                             char* peer_ip);

private:
    friend struct CAutoPunishRuleHackTypeMgrLayoutVerifier;

    // IDA_EA: 0x869550C
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr15_checkApplyFlagERKNS_14HackTypeInfo_tEPNS_13CHackAnalyzerERS1_RSt6vectorI10stLookOutsSaIS8_EERS7_I12tyIPCountersSaISC_EESB_Pc
    bool _checkApplyFlag(const HackTypeInfo_t& rule,
                         CHackAnalyzer* analyzer,
                         HackTypeInfo_t& matched_rule,
                         std::vector<stLookOuts>& lookouts,
                         std::vector<tyIPCounters>& ip_counters,
                         std::vector<stLookOuts>& lookout_history,
                         char* peer_ip);

    // IDA_EA: 0x80F967A
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr17WriteIPCounterLogEPNS_13CHackAnalyzerEiiiiiPc
    int WriteIPCounterLog(CHackAnalyzer* analyzer,
                          int passed,
                          int hack_type,
                          int hack_sub_type,
                          int required_count,
                          int current_count,
                          char* peer_ip);

    // IDA_EA: 0x80F96D2
    // IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr21WriteFullIPCounterLogEPNS_13CHackAnalyzerEiiiiiPc
    int WriteFullIPCounterLog(CHackAnalyzer* analyzer,
                              int passed,
                              int hack_type,
                              int hack_sub_type,
                              int required_count,
                              int current_count,
                              char* peer_ip);

    std::multimap<ENUM_HACKTYPE, HackTypeInfo_t> hack_rules_;  // +0x00
    uint32_t total_trade_count_;                                // +0x18
    uint64_t total_trade_etc_;                                  // +0x1c
    std::multimap<uint16_t, HackIPCounter_t> ip_counters_;      // +0x24
    // C2 constructs the next tree at +0x40 and issues no write at +0x3c.
    // This explicit uninitialised word preserves that observed ABI gap.
    uint32_t ip_counter_gap_;                                   // +0x3c
    std::multimap<uint16_t, HackFullIPCounter_t> full_ip_counters_; // +0x40
    std::map<int, stHacktypeExceptionInfo_t> exception_rules_;  // +0x58
};

static_assert(sizeof(stHacktypeExceptionInfo_t) == 0x04u,
              "old exception rule record must stay four bytes");
static_assert(sizeof(HackIPCounter_t) == 0x18u,
              "old class-IP counter record must stay 24 bytes");
static_assert(sizeof(HackFullIPCounter_t) == 0x18u,
              "old full-IP counter record must stay 24 bytes");
static_assert(sizeof(HackTypeInfo_t) == 0x10u,
              "old hack-type rule record must stay 16 bytes");

#if defined(__i386__)
struct CAutoPunishRuleHackTypeMgrLayoutVerifier {
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, hack_rules_) == 0x00u,
                  "old hack-rule tree must start at +0x00");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, total_trade_count_) == 0x18u,
                  "old total trade count must stay at +0x18");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, total_trade_etc_) == 0x1cu,
                  "old total trade value must stay at +0x1c");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, ip_counters_) == 0x24u,
                  "old class-IP tree must start at +0x24");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, ip_counter_gap_) == 0x3cu,
                  "old class-IP tree leaves one observed word before +0x40");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, full_ip_counters_) == 0x40u,
                  "old full-IP tree must start at +0x40");
    static_assert(offsetof(CAutoPunishRuleHackTypeMgr, exception_rules_) == 0x58u,
                  "old exception tree must start at +0x58");
};

static_assert(sizeof(CAutoPunishRuleHackTypeMgr) == 0x70u,
              "old AutoPunish hack-type manager must stay 0x70 bytes on i386");
#endif

}  // namespace WongWork


#endif  // OLDSRV_OLD_DATA_LOGIN_SECURITY_AUTO_PUNISH_HACK_TYPE_MANAGER_H_
