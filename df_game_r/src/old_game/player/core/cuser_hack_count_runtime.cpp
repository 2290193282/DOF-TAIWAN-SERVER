#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"


// IDA_EA: 0x867FD0A
// IDA_NAME: _ZN5CUser17onUpdateHackCountEN8WongWork13ENUM_HACKTYPEEijj
// DEMANGLED: CUser::onUpdateHackCount(WongWork::ENUM_HACKTYPE,int,unsigned int,unsigned int)
int CUser::onUpdateHackCount(WongWork::ENUM_HACKTYPE hack_type,
                             int count,
                             unsigned int value1,
                             unsigned int value2) {
    const int point_per_hack_type =
        WongWork::CMCAPManager::getPointPerHackType(hack_type);
    mcap_manager_.incPoint(count * point_per_hack_type);

    if (mcap_manager_.getPoint() > 1000u) {
        const unsigned int point = mcap_manager_.getPoint();
        setStdDropRate(static_cast<int>(100u - point / 100u));
    }

    if (isHackUserTimer() != 1) {
        getHades()->AddHackTypeCnt(static_cast<unsigned long>(hack_type),
                                   static_cast<unsigned long>(count),
                                   static_cast<unsigned long>(value1),
                                   static_cast<unsigned long>(value2));
        getHackAnalyzer()->setUserP(this);

        WongWork::HackTypeInfo_t first_rule;
        char peer_ip[16];
        network_.GetPeerIP2(peer_ip, sizeof(peer_ip));

        WongWork::CHackAnalyzer* analyzer = getHackAnalyzer();
        const unsigned int accumulated_count = static_cast<unsigned int>(
            getHackAnalyzer()->getServerHackAccumulatedCnt(hack_type));
        if (GlobalData::g_autoPunishRuleHackTypeMgr->checkMaxHackCntOver(
                hack_type,
                accumulated_count,
                analyzer,
                first_rule,
                lookouts_,
                ip_counters_,
                lookout_history_,
                peer_ip)) {
            const WongWork::ENUM_HACKTYPE sub_type =
                static_cast<WongWork::ENUM_HACKTYPE>(first_rule.hack_sub_type);
            const unsigned int sub_accumulated_count =
                static_cast<unsigned int>(
                    getHackAnalyzer()->getServerHackAccumulatedCnt(sub_type));
            const WongWork::ENUM_HACKTYPE main_type =
                static_cast<WongWork::ENUM_HACKTYPE>(first_rule.hack_type);
            const unsigned int main_accumulated_count =
                static_cast<unsigned int>(
                    getHackAnalyzer()->getServerHackAccumulatedCnt(main_type));
            setHackUser(first_rule.hack_type,
                        main_accumulated_count,
                        first_rule.hack_sub_type,
                        sub_accumulated_count,
                        first_rule.apply_flag,
                        first_rule.etc_low);
        }

        WongWork::HackTypeInfo_t second_rule;
        if (first_rule.apply_flag == 3u) {
            analyzer = getHackAnalyzer();
            const unsigned int second_accumulated_count =
                static_cast<unsigned int>(
                    getHackAnalyzer()->getServerHackAccumulatedCnt(hack_type));
            if (GlobalData::g_autoPunishRuleHackTypeMgr->checkMaxHackCntOver(
                    hack_type,
                    second_accumulated_count,
                    analyzer,
                    second_rule,
                    lookouts_,
                    ip_counters_,
                    lookout_history_,
                    peer_ip)) {
                const WongWork::ENUM_HACKTYPE sub_type =
                    static_cast<WongWork::ENUM_HACKTYPE>(
                        second_rule.hack_sub_type);
                const unsigned int sub_accumulated_count =
                    static_cast<unsigned int>(
                        getHackAnalyzer()->getServerHackAccumulatedCnt(
                            sub_type));
                const WongWork::ENUM_HACKTYPE main_type =
                    static_cast<WongWork::ENUM_HACKTYPE>(second_rule.hack_type);
                const unsigned int main_accumulated_count =
                    static_cast<unsigned int>(
                        getHackAnalyzer()->getServerHackAccumulatedCnt(
                            main_type));
                setHackUser(second_rule.hack_type,
                            main_accumulated_count,
                            second_rule.hack_sub_type,
                            sub_accumulated_count,
                            second_rule.apply_flag,
                            first_rule.etc_low);
            }
        }
    }

    const unsigned int accumulated_count = static_cast<unsigned int>(
        getHackAnalyzer()->getServerHackAccumulatedCnt(hack_type));
    const unsigned int server_count = static_cast<unsigned int>(
        getHackAnalyzer()->getServerHackCnt(hack_type));
    return history_log_.HackDetected(
        static_cast<unsigned int>(hack_type),
        static_cast<unsigned int>(count),
        server_count,
        accumulated_count,
        value1,
        value2);
}
