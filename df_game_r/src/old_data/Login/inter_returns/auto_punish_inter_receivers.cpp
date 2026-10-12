#include "old_data/Login/inter_returns/auto_punish_inter_receivers.h"

#include <arpa/inet.h>
#include <cstring>

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/security/auto_punish_black_ip_manager.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"


// IDA_EA: 0x84D422A
// IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIP12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser *,char *,int)
int Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_AUTO_PUNISH_RULE_BLACK_IP& reply =
        *reinterpret_cast<const SIG_AUTO_PUNISH_RULE_BLACK_IP*>(payload);
    GlobalData::g_autoPunishRuleBlackIPMgr->reset();

    cMyTrace start_trace(
        "virtual int Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser*, char*, int)",
        13665,
        0);
    start_trace("Black IP Load Start (TotalCount: %d)", reply.entry_count);

    for (uint32_t index = 0; index < reply.entry_count; ++index) {
        const SIG_AUTO_PUNISH_RULE_BLACK_IP_ENTRY& entry = reply.entries[index];
        in_addr start_address;
        start_address.s_addr = entry.c_class_ip |
                                htonl(static_cast<uint32_t>(entry.start_ip));
        const char* const start_text = inet_ntoa(start_address);
        char start_ip_text[16] = {};
        std::strncpy(start_ip_text, start_text, sizeof(start_ip_text));

        if (GlobalData::g_autoPunishRuleBlackIPMgr->addBlackIP(
                entry.c_class_ip,
                entry.start_ip,
                entry.end_ip) != 1) {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser*, char*, int)",
                13677,
                "Black IP Load fail(StartIP:%s -> %d)",
                start_ip_text,
                static_cast<unsigned int>(entry.end_ip));
        }

        cMyTrace entry_trace(
            "virtual int Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser*, char*, int)",
            13678,
            0);
        entry_trace("Black IP Load (StartIP:%s -> %d)",
                    start_ip_text,
                    static_cast<unsigned int>(entry.end_ip));
    }

    const int black_ip_count =
        GlobalData::g_autoPunishRuleBlackIPMgr->getBlackIPCount();
    cMyTrace complete_trace(
        "virtual int Inter_AutoPunishRuleBlackIP::dispatch_sig(CUser*, char*, int)",
        13680,
        0);
    complete_trace("Black IP Load End (TotalCount: %d)", black_ip_count);
    return 0;
}

// IDA_EA: 0x84EDB98
// IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPC2Ev
// DEMANGLED: Inter_AutoPunishRuleBlackIP::Inter_AutoPunishRuleBlackIP(void)
Inter_AutoPunishRuleBlackIP::Inter_AutoPunishRuleBlackIP() {}

// IDA_EA: 0x84F8F04
// IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPD2Ev
// DEMANGLED: Inter_AutoPunishRuleBlackIP::~Inter_AutoPunishRuleBlackIP()
// IDA_EA: 0x84F8F34
// IDA_NAME: _ZN27Inter_AutoPunishRuleBlackIPD0Ev
// DEMANGLED: Inter_AutoPunishRuleBlackIP::~Inter_AutoPunishRuleBlackIP()
Inter_AutoPunishRuleBlackIP::~Inter_AutoPunishRuleBlackIP() {}


// IDA_EA: 0x84D4440
// IDA_NAME: _ZN28Inter_AutoPunishRuleHackType12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AutoPunishRuleHackType::dispatch_sig(CUser *,char *,int)
int Inter_AutoPunishRuleHackType::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_AUTO_PUNISH_RULE_HACK_TYPE& reply =
        *reinterpret_cast<const SIG_AUTO_PUNISH_RULE_HACK_TYPE*>(payload);
    GlobalData::g_autoPunishRuleHackTypeMgr->reset();
    GlobalData::g_autoPunishRuleHackTypeMgr->setTotalTrade(
        reply.total_count,
        reply.total_etc);

    cMyTrace start_trace(
        "virtual int Inter_AutoPunishRuleHackType::dispatch_sig(CUser*, char*, int)",
        13701,
        0);
    start_trace("HackType Load Start. (TotalGold: %llu)(TotalCount: %d)",
                reply.total_etc,
                reply.total_count);

    for (uint32_t index = 0; index < reply.rule_count; ++index) {
        const SIG_AUTO_PUNISH_RULE_HACK_TYPE_ENTRY& entry = reply.entries[index];
        GlobalData::g_autoPunishRuleHackTypeMgr->setHackCnt(
            static_cast<WongWork::ENUM_HACKTYPE>(entry.hack_type),
            entry.count,
            static_cast<WongWork::ENUM_HACKTYPE>(entry.hack_sub_type),
            entry.hack_sub_count,
            entry.apply_flag,
            entry.ip_count,
            entry.etc_low);

        cMyTrace entry_trace(
            "virtual int Inter_AutoPunishRuleHackType::dispatch_sig(CUser*, char*, int)",
            13718,
            0);
        entry_trace("HackType Load (Type: %d -> %d)(SubType: %d -> %d)",
                    entry.hack_type,
                    entry.count,
                    entry.hack_sub_type,
                    entry.hack_sub_count);
    }

    cMyTrace complete_trace(
        "virtual int Inter_AutoPunishRuleHackType::dispatch_sig(CUser*, char*, int)",
        13721,
        0);
    complete_trace("HackType Load End");
    return 0;
}

// IDA_EA: 0x84EDBB4
// IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeC2Ev
// DEMANGLED: Inter_AutoPunishRuleHackType::Inter_AutoPunishRuleHackType(void)
Inter_AutoPunishRuleHackType::Inter_AutoPunishRuleHackType() {}

// IDA_EA: 0x84F8EB6
// IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeD2Ev
// DEMANGLED: Inter_AutoPunishRuleHackType::~Inter_AutoPunishRuleHackType()
// IDA_EA: 0x84F8EE6
// IDA_NAME: _ZN28Inter_AutoPunishRuleHackTypeD0Ev
// DEMANGLED: Inter_AutoPunishRuleHackType::~Inter_AutoPunishRuleHackType()
Inter_AutoPunishRuleHackType::~Inter_AutoPunishRuleHackType() {}


// IDA_EA: 0x84DCB18
// IDA_NAME: _ZN16Inter_SetIPCount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SetIPCount::dispatch_sig(CUser *,char *,int)
int Inter_SetIPCount::dispatch_sig(CUser* user, char* payload, int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_AUTO_PUNISH_COUNTER_HEADER& header =
        *reinterpret_cast<const SIG_AUTO_PUNISH_COUNTER_HEADER*>(payload);
    if (!header.keep_existing) {
        GlobalData::g_autoPunishRuleHackTypeMgr->ResetIpCounterData();
    }

    const SIG_AUTO_PUNISH_IP_COUNTER_ENTRY* const entries =
        reinterpret_cast<const SIG_AUTO_PUNISH_IP_COUNTER_ENTRY*>(
            payload + sizeof(SIG_AUTO_PUNISH_COUNTER_HEADER));
    for (uint32_t index = 0; index < header.entry_count; ++index) {
        const SIG_AUTO_PUNISH_IP_COUNTER_ENTRY& entry = entries[index];
        GlobalData::g_autoPunishRuleHackTypeMgr->setIPCounterData(
            static_cast<uint32_t>(entry.hack_type),
            static_cast<uint32_t>(entry.hack_sub_type),
            const_cast<char*>(entry.c_class_ip),
            entry.ip_count);
    }
    return 0;
}

// IDA_EA: 0x84EE2D0
// IDA_NAME: _ZN16Inter_SetIPCountC2Ev
// DEMANGLED: Inter_SetIPCount::Inter_SetIPCount(void)
Inter_SetIPCount::Inter_SetIPCount() {}

// IDA_EA: 0x84F7A9A
// IDA_NAME: _ZN16Inter_SetIPCountD2Ev
// IDA_EA: 0x84F7ACA
// IDA_NAME: _ZN16Inter_SetIPCountD0Ev
// DEMANGLED: Inter_SetIPCount::~Inter_SetIPCount()
Inter_SetIPCount::~Inter_SetIPCount() {}


// IDA_EA: 0x84DCBEA
// IDA_NAME: _ZN20Inter_SetFullIPCount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SetFullIPCount::dispatch_sig(CUser *,char *,int)
int Inter_SetFullIPCount::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_AUTO_PUNISH_COUNTER_HEADER& header =
        *reinterpret_cast<const SIG_AUTO_PUNISH_COUNTER_HEADER*>(payload);
    if (!header.keep_existing) {
        GlobalData::g_autoPunishRuleHackTypeMgr->ResetFullIpCounterData();
    }

    const SIG_AUTO_PUNISH_FULL_IP_COUNTER_ENTRY* const entries =
        reinterpret_cast<const SIG_AUTO_PUNISH_FULL_IP_COUNTER_ENTRY*>(
            payload + sizeof(SIG_AUTO_PUNISH_COUNTER_HEADER));
    for (uint32_t index = 0; index < header.entry_count; ++index) {
        const SIG_AUTO_PUNISH_FULL_IP_COUNTER_ENTRY& entry = entries[index];
        GlobalData::g_autoPunishRuleHackTypeMgr->setFullIPCounterData(
            static_cast<uint32_t>(entry.hack_type),
            static_cast<uint32_t>(entry.hack_sub_type),
            const_cast<char*>(entry.full_ip),
            entry.ip_count);
    }
    return 0;
}

// IDA_EA: 0x84EE2EC
// IDA_NAME: _ZN20Inter_SetFullIPCountC2Ev
// DEMANGLED: Inter_SetFullIPCount::Inter_SetFullIPCount(void)
Inter_SetFullIPCount::Inter_SetFullIPCount() {}

// IDA_EA: 0x84F7A4C
// IDA_NAME: _ZN20Inter_SetFullIPCountD2Ev
// IDA_EA: 0x84F7A7C
// IDA_NAME: _ZN20Inter_SetFullIPCountD0Ev
// DEMANGLED: Inter_SetFullIPCount::~Inter_SetFullIPCount()
Inter_SetFullIPCount::~Inter_SetFullIPCount() {}


// IDA_EA: 0x84E09CE
// IDA_NAME: _ZN16Inter_SetARSInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SetARSInfo::dispatch_sig(CUser *,char *,int)
int Inter_SetARSInfo::dispatch_sig(CUser* user, char* payload, int payload_size) {
    (void)user;
    (void)payload_size;

    const SIG_AUTO_PUNISH_ARS_HEADER& header =
        *reinterpret_cast<const SIG_AUTO_PUNISH_ARS_HEADER*>(payload);
    if (!header.keep_existing) {
        GlobalData::g_autoPunishRuleHackTypeMgr->reset();
        cMyTrace reset_trace(
            "virtual int Inter_SetARSInfo::dispatch_sig(CUser*, char*, int)",
            20318,
            0);
        reset_trace("[ARS_INFO] Reset");
    }

    const int32_t entry_count = header.entry_count;
    cMyTrace count_trace(
        "virtual int Inter_SetARSInfo::dispatch_sig(CUser*, char*, int)",
        20324,
        0);
    count_trace("[ARS_INFO] Cnt : %d, State : %d",
                entry_count,
                static_cast<int>(header.keep_existing));
    if (entry_count > 100) {
        cMyTrace over_count_trace(
            "virtual int Inter_SetARSInfo::dispatch_sig(CUser*, char*, int)",
            20331,
            0);
        over_count_trace("[ARS_INFO] Cnt OverFlag(100)");
        return 0;
    }

    const SIG_AUTO_PUNISH_ARS_ENTRY* const entries =
        reinterpret_cast<const SIG_AUTO_PUNISH_ARS_ENTRY*>(
            payload + sizeof(SIG_AUTO_PUNISH_ARS_HEADER));
    for (int32_t index = 0; index < entry_count; ++index) {
        const SIG_AUTO_PUNISH_ARS_ENTRY& entry = entries[index];
        if (entry.hack_type) {
            GlobalData::g_autoPunishRuleHackTypeMgr->setHackCnt(
                static_cast<WongWork::ENUM_HACKTYPE>(entry.hack_type),
                static_cast<uint32_t>(entry.count),
                static_cast<WongWork::ENUM_HACKTYPE>(entry.hack_sub_type),
                static_cast<uint32_t>(entry.hack_sub_count),
                static_cast<uint32_t>(entry.apply_flag),
                static_cast<uint32_t>(entry.ip_count),
                static_cast<uint32_t>(entry.etc_low));
            cMyTrace entry_trace(
                "virtual int Inter_SetARSInfo::dispatch_sig(CUser*, char*, int)",
                20366,
                0);
            entry_trace(
                "[ARS_INFO] MainType : %d (%d), SubType : %d(%d), ApplyFlag %d, IpCnt : %d",
                static_cast<int>(entry.hack_type),
                static_cast<int>(entry.count),
                static_cast<int>(entry.hack_sub_type),
                static_cast<int>(entry.hack_sub_count),
                static_cast<int>(entry.apply_flag),
                static_cast<int>(entry.ip_count));
        } else {
            GlobalData::g_autoPunishRuleHackTypeMgr->setTotalTrade(
                static_cast<uint32_t>(entry.count),
                static_cast<uint64_t>(entry.etc_low));
            cMyTrace total_trace(
                "virtual int Inter_SetARSInfo::dispatch_sig(CUser*, char*, int)",
                20354,
                0);
            total_trace("[ARS_INFO] TotalGold: %llu, TotalCount: %d",
                        static_cast<unsigned long long>(entry.etc_low),
                        static_cast<int>(entry.count));
        }
    }
    return 0;
}

// IDA_EA: 0x84EE634
// IDA_NAME: _ZN16Inter_SetARSInfoC2Ev
// DEMANGLED: Inter_SetARSInfo::Inter_SetARSInfo(void)
Inter_SetARSInfo::Inter_SetARSInfo() {}

// IDA_EA: 0x84F7128
// IDA_NAME: _ZN16Inter_SetARSInfoD2Ev
// IDA_EA: 0x84F7158
// IDA_NAME: _ZN16Inter_SetARSInfoD0Ev
// DEMANGLED: Inter_SetARSInfo::~Inter_SetARSInfo()
Inter_SetARSInfo::~Inter_SetARSInfo() {}
