#include "infra/network/dispatcher/loading_time_report_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_LoadingTimeReport::dispatch_sig(CUser*, PacketBuf&)";

void StoreUnsignedShort(unsigned char* destination, unsigned short value) {
    std::memcpy(destination, &value, sizeof(value));
}

void StoreUnsignedInt(unsigned char* destination, unsigned int value) {
    std::memcpy(destination, &value, sizeof(value));
}

bool ReadUnsignedShort(PacketBuf& packet, unsigned char* destination) {
    unsigned short value = 0;
    if (packet.get_short(value) != 1) {
        return false;
    }
    StoreUnsignedShort(destination, value);
    return true;
}

bool ReadUnsignedByte(PacketBuf& packet, unsigned char* destination) {
    unsigned char value = 0;
    if (packet.get_byte(value) != 1) {
        return false;
    }
    *destination = value;
    return true;
}

bool ReadUnsignedInt(PacketBuf& packet, unsigned char* destination) {
    unsigned int value = 0;
    if (packet.get_int(value) != 1) {
        return false;
    }
    StoreUnsignedInt(destination, value);
    return true;
}

}  // namespace

// IDA_EA: 0x822C040
// IDA_NAME: _ZN37Packet_Loading_Time_Report_StatisticsC2Ev
// DEMANGLED: Packet_Loading_Time_Report_Statistics::Packet_Loading_Time_Report_Statistics(void)
Packet_Loading_Time_Report_Statistics::Packet_Loading_Time_Report_Statistics()
    : header(0x0FB0u, 0x0806u) {}

// IDA_EA: 0x8596CE8
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReportC2Ev
// DEMANGLED: Dispatcher_LoadingTimeReport::Dispatcher_LoadingTimeReport(void)
Dispatcher_LoadingTimeReport::Dispatcher_LoadingTimeReport() {}

// IDA_EA: 0x82410C2
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReportD2Ev
// DEMANGLED: Dispatcher_LoadingTimeReport::~Dispatcher_LoadingTimeReport()
// IDA_EA: 0x82410F2
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReportD0Ev
// DEMANGLED: Dispatcher_LoadingTimeReport::~Dispatcher_LoadingTimeReport()
Dispatcher_LoadingTimeReport::~Dispatcher_LoadingTimeReport() {}

// IDA_EA: 0x821EFEC
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReport12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_LoadingTimeReport::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_LoadingTimeReport::dispatch_sig(CUser* user, PacketBuf& packet) {
    Packet_Loading_Time_Report_Statistics statistic;
    unsigned short code = 0;

    for (int index = 0; index <= 8; ++index) {
        if (packet.get_short(code) != 1) {
            return LineFunc(55294, kDispatchSignature, 0, 0);
        }
        if (code > 8u) {
            return LineFunc(55296, kDispatchSignature, 0, 0);
        }
        if (code <= 6u &&
            !ReadUnsignedInt(packet, statistic.raw_0a + 4u * code)) {
            return LineFunc(55302, kDispatchSignature, 0, 0);
        }

        // The original invokes this typed CUser accessor on every one of the
        // nine records even though Hex-Rays shows the returned value unused.
        // Retain the call and its ordering; do not replace it with a field
        // read or omit it as a presumed optimizer artifact.
        (void)user->get_acc_id();

        if (code == 7u) {
            short entry_count = 0;
            if (packet.get_short(entry_count) != 1) {
                return LineFunc(55308, kDispatchSignature, 0, 0);
            }
            if (entry_count > 99) {
                return LineFunc(55309, kDispatchSignature, 0, 0);
            }
            for (int entry = 0; entry_count > entry; ++entry) {
                unsigned char* const destination =
                    statistic.raw_0a + 42u + 9u * entry;
                if (!ReadUnsignedShort(packet, destination)) {
                    return LineFunc(55313, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedByte(packet, destination + 2u)) {
                    return LineFunc(55314, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedShort(packet, destination + 3u)) {
                    return LineFunc(55315, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedShort(packet, destination + 5u)) {
                    return LineFunc(55316, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedShort(packet, destination + 7u)) {
                    return LineFunc(55317, kDispatchSignature, 0, 0);
                }
            }
            StoreUnsignedInt(statistic.raw_0a + 28u, 0u);
        }

        if (code == 8u) {
            short entry_count = 0;
            if (packet.get_short(entry_count) != 1) {
                return LineFunc(55324, kDispatchSignature, 0, 0);
            }
            if (entry_count > 99) {
                return LineFunc(55325, kDispatchSignature, 0, 0);
            }
            for (int entry = 0; entry_count > entry; ++entry) {
                unsigned char* const destination =
                    statistic.raw_0a + 944u + 11u * entry;
                if (!ReadUnsignedShort(packet, destination)) {
                    return LineFunc(55329, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedByte(packet, destination + 2u)) {
                    return LineFunc(55330, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedInt(packet, destination + 3u)) {
                    return LineFunc(55331, kDispatchSignature, 0, 0);
                }
                if (!ReadUnsignedInt(packet, destination + 7u)) {
                    return LineFunc(55332, kDispatchSignature, 0, 0);
                }
            }
            StoreUnsignedInt(statistic.raw_0a + 32u, 0u);
        }
    }

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x806);
    return 0;
}

// IDA_EA: 0x8233F06
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReport4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_LoadingTimeReport::read(PacketBuf &,MSG_BASE &)
int Dispatcher_LoadingTimeReport::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F10
// IDA_NAME: _ZN28Dispatcher_LoadingTimeReport7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_LoadingTimeReport::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_LoadingTimeReport::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
