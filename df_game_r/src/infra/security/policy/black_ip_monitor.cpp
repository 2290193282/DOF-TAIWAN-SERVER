#include "infra/security/policy/black_ip_monitor.h"

#include <arpa/inet.h>


namespace WongWork {

// IDA_EA: 0x84EBA2C
// IDA_NAME: _ZN8WongWork23stBlackIPMonitorEntry_tC2ENS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEEjll
// DEMANGLED: WongWork::stBlackIPMonitorEntry_t::stBlackIPMonitorEntry_t(WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE,unsigned int,long,long)
stBlackIPMonitorEntry_t::stBlackIPMonitorEntry_t(
        ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type_value,
        uint32_t member_id_count_value,
        long start_time_value,
        long end_time_value)
    : punish_type(punish_type_value),
      member_id_count(member_id_count_value),
      start_time(start_time_value),
      end_time(end_time_value) {
}

// IDA_EA: 0x82A6526
// IDA_NAME: _ZN8WongWork15CBlackIPMonitorC2Ev
// DEMANGLED: WongWork::CBlackIPMonitor::CBlackIPMonitor(void)
CBlackIPMonitor::CBlackIPMonitor()
    : entries_() {
}

// IDA_EA: 0x82A6512
// IDA_NAME: _ZN8WongWork15CBlackIPMonitor5resetEv
// DEMANGLED: WongWork::CBlackIPMonitor::reset(void)
int CBlackIPMonitor::reset() {
    entries_.clear();
    return 0;
}

// IDA_EA: 0x84EBA54
// IDA_NAME: _ZN8WongWork15CBlackIPMonitor11insertNewIPEjNS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEEjll
// DEMANGLED: WongWork::CBlackIPMonitor::insertNewIP(unsigned int,WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE,unsigned int,long,long)
int CBlackIPMonitor::insertNewIP(
        uint32_t ip,
        ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type,
        uint32_t member_id_count,
        long start_time,
        long end_time) {
    entries_.insert(std::make_pair(
        ip,
        stBlackIPMonitorEntry_t(punish_type, member_id_count, start_time,
                                end_time)));
    return 1;
}

// IDA_EA: 0x84EBACE
// IDA_NAME: _ZN8WongWork15CBlackIPMonitor8deleteIPEPKcj
// DEMANGLED: WongWork::CBlackIPMonitor::deleteIP(char const*,unsigned int)
int CBlackIPMonitor::deleteIP(const char* ip, uint32_t punish_type) {
    if (ip == 0) {
        return 0;
    }

    const uint32_t key = static_cast<uint32_t>(::inet_addr(ip));
    std::pair<std::multimap<uint32_t, stBlackIPMonitorEntry_t>::iterator,
              std::multimap<uint32_t, stBlackIPMonitorEntry_t>::iterator>
        range = entries_.equal_range(key);
    for (std::multimap<uint32_t, stBlackIPMonitorEntry_t>::iterator it =
             range.first;
         it != range.second;
         ++it) {
        if (static_cast<uint32_t>(it->second.punish_type) == punish_type) {
            entries_.erase(it);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x84EBBA6
// IDA_NAME: _ZN8WongWork15CBlackIPMonitor13getPunishTypeEPKc
// DEMANGLED: WongWork::CBlackIPMonitor::getPunishType(char const*)
int CBlackIPMonitor::getPunishType(const char* ip) {
    if (ip == 0) {
        return 0;
    }

    const uint32_t key = static_cast<uint32_t>(::inet_addr(ip));
    int punish_types = 0;
    const std::pair<
        std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator,
        std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator>
        range = entries_.equal_range(key);
    for (std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        punish_types |= static_cast<int>(it->second.punish_type);
    }
    return punish_types;
}

// IDA_EA: 0x84EBC8C
// IDA_NAME: _ZN8WongWork15CBlackIPMonitor10GetEndTimeEPKcNS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEE
// DEMANGLED: WongWork::CBlackIPMonitor::GetEndTime(char const*,WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE)
long CBlackIPMonitor::GetEndTime(
        const char* ip,
        ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type) {
    if (ip == 0) {
        return 0;
    }

    const uint32_t key = static_cast<uint32_t>(::inet_addr(ip));
    const std::pair<
        std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator,
        std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator>
        range = entries_.equal_range(key);
    for (std::multimap<uint32_t, stBlackIPMonitorEntry_t>::const_iterator it =
             range.first;
         it != range.second;
         ++it) {
        if (it->second.punish_type == punish_type) {
            return it->second.end_time;
        }
    }
    return 0;
}

}  // namespace WongWork
