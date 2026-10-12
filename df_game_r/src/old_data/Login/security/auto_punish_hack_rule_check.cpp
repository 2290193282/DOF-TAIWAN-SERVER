#include "old_game/player/core/cuser.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"

#include <map>
#include <utility>
#include <vector>


namespace WongWork {

// IDA_EA: 0x80F967A
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr17WriteIPCounterLogEPNS_13CHackAnalyzerEiiiiiPc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::WriteIPCounterLog(WongWork::CHackAnalyzer *,int,int,int,int,int,char *)
int CAutoPunishRuleHackTypeMgr::WriteIPCounterLog(
    CHackAnalyzer* analyzer,
    int passed,
    int hack_type,
    int hack_sub_type,
    int required_count,
    int current_count,
    char* peer_ip) {
    CUser* const user = analyzer->getUserP();
    if (!user) {
        return 0;
    }
    return user->history_log().IPCounterLog(passed,
                                            hack_type,
                                            hack_sub_type,
                                            required_count,
                                            current_count,
                                            peer_ip);
}

// IDA_EA: 0x80F96D2
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr21WriteFullIPCounterLogEPNS_13CHackAnalyzerEiiiiiPc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::WriteFullIPCounterLog(WongWork::CHackAnalyzer *,int,int,int,int,int,char *)
int CAutoPunishRuleHackTypeMgr::WriteFullIPCounterLog(
    CHackAnalyzer* analyzer,
    int passed,
    int hack_type,
    int hack_sub_type,
    int required_count,
    int current_count,
    char* peer_ip) {
    CUser* const user = analyzer->getUserP();
    if (!user) {
        return 0;
    }
    return user->history_log().FullIPCounterLog(passed,
                                                hack_type,
                                                hack_sub_type,
                                                required_count,
                                                current_count,
                                                peer_ip);
}

// IDA_EA: 0x869550C
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr15_checkApplyFlagERKNS_14HackTypeInfo_tEPNS_13CHackAnalyzerERS1_RSt6vectorI10stLookOutsSaIS8_EERS7_I12tyIPCountersSaISC_EESB_Pc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::_checkApplyFlag(WongWork::HackTypeInfo_t const&,WongWork::CHackAnalyzer *,WongWork::HackTypeInfo_t&,std::vector<stLookOuts> &,std::vector<tyIPCounters> &,std::vector<stLookOuts> &,char *)
bool CAutoPunishRuleHackTypeMgr::_checkApplyFlag(
    const HackTypeInfo_t& rule,
    CHackAnalyzer* analyzer,
    HackTypeInfo_t& matched_rule,
    std::vector<stLookOuts>& lookouts,
    std::vector<tyIPCounters>& ip_counters,
    std::vector<stLookOuts>& lookout_history,
    char* peer_ip) {
    if (rule.apply_flag != 1u && rule.apply_flag != 5u) {
        const int lookout_count = static_cast<int>(lookouts.size());
        if (rule.apply_flag == 2u && lookout_count > 0) {
            for (int index = 0; index < lookout_count; ++index) {
                if (rule.hack_type == lookouts[index].first_ &&
                    rule.hack_sub_type == lookouts[index].second_) {
                    return false;
                }
            }
        }

        const int history_count =
            static_cast<int>(lookout_history.size());
        if (rule.apply_flag == 10u && history_count > 0) {
            for (int index = 0; index < history_count; ++index) {
                if (rule.hack_type == lookout_history[index].first_ &&
                    rule.hack_sub_type == lookout_history[index].second_) {
                    return false;
                }
            }
        }

        const int counter_count = static_cast<int>(ip_counters.size());
        if (rule.apply_flag == 3u && counter_count > 0) {
            for (int index = 0; index < counter_count; ++index) {
                if (rule.hack_type == ip_counters[index].first_ &&
                    rule.hack_sub_type == ip_counters[index].second_) {
                    return false;
                }
            }
        }

        if (rule.apply_flag == 4u || rule.apply_flag == 9u) {
            const unsigned int ip_count = static_cast<unsigned int>(
                _findIPCount(rule.hack_type, rule.hack_sub_type, peer_ip));
            if (rule.ip_count > ip_count) {
                WriteIPCounterLog(analyzer,
                                  0,
                                  rule.hack_type,
                                  rule.hack_sub_type,
                                  rule.ip_count,
                                  static_cast<int>(ip_count),
                                  peer_ip);
                return false;
            }
            WriteIPCounterLog(analyzer,
                              1,
                              rule.hack_type,
                              rule.hack_sub_type,
                              rule.ip_count,
                              static_cast<int>(ip_count),
                              peer_ip);
        }

        if (rule.apply_flag == 11u || rule.apply_flag == 12u) {
            const unsigned int full_ip_count = static_cast<unsigned int>(
                _findFullIPCount(rule.hack_type,
                                 rule.hack_sub_type,
                                 peer_ip));
            if (rule.ip_count > full_ip_count) {
                WriteFullIPCounterLog(analyzer,
                                      0,
                                      rule.hack_type,
                                      rule.hack_sub_type,
                                      rule.ip_count,
                                      static_cast<int>(full_ip_count),
                                      peer_ip);
                return false;
            }
            WriteFullIPCounterLog(analyzer,
                                  1,
                                  rule.hack_type,
                                  rule.hack_sub_type,
                                  rule.ip_count,
                                  static_cast<int>(full_ip_count),
                                  peer_ip);
        }
    }

    matched_rule.CopyStruct(rule);
    return true;
}

// IDA_EA: 0x8695360
// IDA_NAME: _ZN8WongWork26CAutoPunishRuleHackTypeMgr19checkMaxHackCntOverENS_13ENUM_HACKTYPEEjPNS_13CHackAnalyzerERNS_14HackTypeInfo_tERSt6vectorI10stLookOutsSaIS7_EERS6_I12tyIPCountersSaISB_EESA_Pc
// DEMANGLED: WongWork::CAutoPunishRuleHackTypeMgr::checkMaxHackCntOver(WongWork::ENUM_HACKTYPE,unsigned int,WongWork::CHackAnalyzer *,WongWork::HackTypeInfo_t &,std::vector<stLookOuts> &,std::vector<tyIPCounters> &,std::vector<stLookOuts> &,char *)
bool CAutoPunishRuleHackTypeMgr::checkMaxHackCntOver(
    ENUM_HACKTYPE hack_type,
    unsigned int accumulated_count,
    CHackAnalyzer* analyzer,
    HackTypeInfo_t& matched_rule,
    std::vector<stLookOuts>& lookouts,
    std::vector<tyIPCounters>& ip_counters,
    std::vector<stLookOuts>& lookout_history,
    char* peer_ip) {
    typedef std::multimap<ENUM_HACKTYPE, HackTypeInfo_t> RuleMap;
    const std::pair<RuleMap::const_iterator, RuleMap::const_iterator> range =
        hack_rules_.equal_range(hack_type);
    for (RuleMap::const_iterator it = range.first; it != range.second; ++it) {
        const HackTypeInfo_t rule = it->second;
        bool count_over = false;
        if (static_cast<int>(rule.hack_type) ==
                static_cast<int>(hack_type) &&
            rule.count <= accumulated_count &&
            static_cast<int>(rule.hack_sub_count) <=
                analyzer->getServerHackAccumulatedCnt(
                    static_cast<ENUM_HACKTYPE>(rule.hack_sub_type))) {
            count_over = true;
        } else if (static_cast<int>(rule.hack_sub_type) ==
                       static_cast<int>(hack_type) &&
                   rule.hack_sub_count <= accumulated_count &&
                   static_cast<int>(rule.count) <=
                       analyzer->getServerHackAccumulatedCnt(
                           static_cast<ENUM_HACKTYPE>(rule.hack_type))) {
            count_over = true;
        }

        if (count_over &&
            _checkApplyFlag(rule,
                            analyzer,
                            matched_rule,
                            lookouts,
                            ip_counters,
                            lookout_history,
                            peer_ip)) {
            return true;
        }
    }
    return false;
}

}  // namespace WongWork
