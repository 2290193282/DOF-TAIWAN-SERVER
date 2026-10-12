#ifndef OLDSRV_GLOBAL_BLACK_IP_MONITOR_H_
#define OLDSRV_GLOBAL_BLACK_IP_MONITOR_H_

#include <stdint.h>

#include <map>


namespace WongWork {

// IDA unit: 99_misc_global/WongWork_CBlackIPMonitor.
// The old binary uses this enum as a bit-mask value.  Only the representation
// is proven here; individual policy values remain owned by the login flow.
enum ENUM_BLACKIPMONITOR_PUNISH_TYPE : int {
    BLACKIPMONITOR_PUNISH_NONE = 0,
};

// IDA unit: 99_misc_global/WongWork_stBlackIPMonitorEntry_t.
struct stBlackIPMonitorEntry_t {
    // IDA_EA: 0x84EBA2C
    // IDA_NAME: _ZN8WongWork23stBlackIPMonitorEntry_tC2ENS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEEjll
    // DEMANGLED: WongWork::stBlackIPMonitorEntry_t::stBlackIPMonitorEntry_t(WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE,unsigned int,long,long)
    stBlackIPMonitorEntry_t(ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type,
                            uint32_t member_id_count,
                            long start_time,
                            long end_time);

    ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type;
    uint32_t member_id_count;
    long start_time;
    long end_time;
};

class CBlackIPMonitor {
public:
    // IDA_EA: 0x82A6526
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitorC2Ev
    // DEMANGLED: WongWork::CBlackIPMonitor::CBlackIPMonitor(void)
    CBlackIPMonitor();

    // IDA_EA: 0x82A6512
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitor5resetEv
    // DEMANGLED: WongWork::CBlackIPMonitor::reset(void)
    int reset();

    // IDA_EA: 0x84EBA54
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitor11insertNewIPEjNS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEEjll
    // DEMANGLED: WongWork::CBlackIPMonitor::insertNewIP(unsigned int,WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE,unsigned int,long,long)
    int insertNewIP(uint32_t ip,
                    ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type,
                    uint32_t member_id_count,
                    long start_time,
                    long end_time);

    // IDA_EA: 0x84EBACE
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitor8deleteIPEPKcj
    // DEMANGLED: WongWork::CBlackIPMonitor::deleteIP(char const*,unsigned int)
    int deleteIP(const char* ip, uint32_t punish_type);

    // IDA_EA: 0x84EBBA6
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitor13getPunishTypeEPKc
    // DEMANGLED: WongWork::CBlackIPMonitor::getPunishType(char const*)
    int getPunishType(const char* ip);

    // IDA_EA: 0x84EBC8C
    // IDA_NAME: _ZN8WongWork15CBlackIPMonitor10GetEndTimeEPKcNS_31ENUM_BLACKIPMONITOR_PUNISH_TYPEE
    // DEMANGLED: WongWork::CBlackIPMonitor::GetEndTime(char const*,WongWork::ENUM_BLACKIPMONITOR_PUNISH_TYPE)
    long GetEndTime(const char* ip,
                    ENUM_BLACKIPMONITOR_PUNISH_TYPE punish_type);

private:
    std::multimap<uint32_t, stBlackIPMonitorEntry_t> entries_;
};

#if defined(__i386__)
static_assert(sizeof(stBlackIPMonitorEntry_t) == 0x10u,
              "IDA BlackIP monitor entry must be 0x10 bytes");
static_assert(sizeof(CBlackIPMonitor) == 0x18u,
              "IDA CBlackIPMonitor must be a 0x18-byte multimap");
#endif

}  // namespace WongWork


#endif  // OLDSRV_GLOBAL_BLACK_IP_MONITOR_H_
