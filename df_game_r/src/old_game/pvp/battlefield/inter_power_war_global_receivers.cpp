#include "old_data/db/taiwan/inter_slots_357_382.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"

// IDA_EA: 0x84DA394
// IDA_NAME: _ZN30Inter_LoadPowerWarStatueRanker12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadPowerWarStatueRanker::dispatch_sig(CUser *,char *,int)
int Inter_LoadPowerWarStatueRanker::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int payload_size) {
    (void)user;
    (void)payload_size;
    SIG_LOAD_POWER_WAR_STATUE_RANKER& input =
        *reinterpret_cast<SIG_LOAD_POWER_WAR_STATUE_RANKER*>(payload);
    GlobalData::s_power_manager->SetPowerWarUserRankingInfo(
        input.charac_no);
    return 0;
}

// IDA_EA: 0x84EE0A0 / 0x84F80B2 / 0x84F80E2
// IDA_NAME: _ZN30Inter_LoadPowerWarStatueRankerC2Ev /
//           _ZN30Inter_LoadPowerWarStatueRankerD2Ev /
//           _ZN30Inter_LoadPowerWarStatueRankerD0Ev
Inter_LoadPowerWarStatueRanker::Inter_LoadPowerWarStatueRanker() {}
Inter_LoadPowerWarStatueRanker::~Inter_LoadPowerWarStatueRanker() {}

// IDA_EA: 0x84DA3BE
// IDA_NAME: _ZN28Inter_LoadPowerWarStatueInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadPowerWarStatueInfo::dispatch_sig(CUser *,char *,int)
int Inter_LoadPowerWarStatueInfo::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)user;
    (void)payload_size;
    // The manager reads the complete 0x80-byte DB reply; do not reduce it to
    // a ranker id or a locally reconstructed projection.
    GlobalData::s_power_manager->SetPowerWarUserStatueInfo(
        reinterpret_cast<SIG_LOAD_POWER_WAR_STATUE_INFO*>(payload));
    return 0;
}

// IDA_EA: 0x84EE0BC / 0x84F8064 / 0x84F8094
// IDA_NAME: _ZN28Inter_LoadPowerWarStatueInfoC2Ev /
//           _ZN28Inter_LoadPowerWarStatueInfoD2Ev /
//           _ZN28Inter_LoadPowerWarStatueInfoD0Ev
Inter_LoadPowerWarStatueInfo::Inter_LoadPowerWarStatueInfo() {}
Inter_LoadPowerWarStatueInfo::~Inter_LoadPowerWarStatueInfo() {}

// IDA_EA: 0x84D4F46
// IDA_NAME: _ZN24Inter_NoticePowerWarRank12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticePowerWarRank::dispatch_sig(CUser *,char *,int)
int Inter_NoticePowerWarRank::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    (void)user;
    (void)payload_size;
    SIG_NOTICE_POWER_WAR_RANK& input =
        *reinterpret_cast<SIG_NOTICE_POWER_WAR_RANK*>(payload);
    cMyTrace trace(
        "virtual int Inter_NoticePowerWarRank::dispatch_sig(CUser*, char*, int)",
        13966, 0);
    trace("RECV Packet_Notice_Power_War_Rank ranker 1:%d, 2:%d, 3:%d",
          static_cast<int>(input.charac_no[0]),
          static_cast<int>(input.charac_no[1]),
          static_cast<int>(input.charac_no[2]));
    GlobalData::s_power_manager->SetPowerWarUserRankingInfo(
        input.charac_no);
    return 0;
}

// IDA_EA: 0x84EE084 / 0x84F8C94 / 0x84F8CC4
// IDA_NAME: _ZN24Inter_NoticePowerWarRankC2Ev /
//           _ZN24Inter_NoticePowerWarRankD2Ev /
//           _ZN24Inter_NoticePowerWarRankD0Ev
Inter_NoticePowerWarRank::Inter_NoticePowerWarRank() {}
Inter_NoticePowerWarRank::~Inter_NoticePowerWarRank() {}
