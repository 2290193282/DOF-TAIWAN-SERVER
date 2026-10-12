#include "infra/network/dispatcher/frame_lag_statistics_dispatcher.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_FrameLagStatistics::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x822BCF4
// IDA_NAME: _ZN30Packet_Frame_Lag_Statistic_AddC2Ev
// DEMANGLED: Packet_Frame_Lag_Statistic_Add::Packet_Frame_Lag_Statistic_Add(void)
Packet_Frame_Lag_Statistic_Add::Packet_Frame_Lag_Statistic_Add()
    : header(0x0C26u, 0x0186u) {}

// IDA_EA: 0x8596834
// IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsC2Ev
// DEMANGLED: Dispatcher_FrameLagStatistics::Dispatcher_FrameLagStatistics(void)
Dispatcher_FrameLagStatistics::Dispatcher_FrameLagStatistics() {}

// IDA_EA: 0x82419E6
// IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsD2Ev
// DEMANGLED: Dispatcher_FrameLagStatistics::~Dispatcher_FrameLagStatistics()
// IDA_EA: 0x8241A16
// IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsD0Ev
// DEMANGLED: Dispatcher_FrameLagStatistics::~Dispatcher_FrameLagStatistics()
Dispatcher_FrameLagStatistics::~Dispatcher_FrameLagStatistics() {}

// IDA_EA: 0x8217046
// IDA_NAME: _ZN29Dispatcher_FrameLagStatistics12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_FrameLagStatistics::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_FrameLagStatistics::dispatch_sig(CUser* user, PacketBuf& packet) {
    Packet_Frame_Lag_Statistic_Add statistic;
    if (packet.get_int(statistic.raw_0a) != 1) {
        return LineFunc(51453, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(statistic.raw_0e) != 1) {
        return LineFunc(51454, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(statistic.raw_0f) != 1) {
        return LineFunc(51455, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_12) != 1) {
        return LineFunc(51457, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_14) != 1) {
        return LineFunc(51458, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_16) != 1) {
        return LineFunc(51459, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_18) != 1) {
        return LineFunc(51461, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(statistic.raw_1a) != 1) {
        return LineFunc(51462, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(statistic.raw_1b) != 1) {
        return LineFunc(51463, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(statistic.raw_1f) != 1) {
        return LineFunc(51465, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_20) != 1) {
        return LineFunc(51466, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(statistic.raw_22) != 1) {
        return LineFunc(51467, kDispatchSignature, 0, 0);
    }

    for (int row_index = 0; row_index <= 5; ++row_index) {
        FrameLagStatisticRawRow& row = statistic.raw_24[row_index];
        if (packet.get_short(row.raw_00) != 1) {
            return LineFunc(51476, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(row.raw_02) != 1) {
            return LineFunc(51477, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(row.raw_04) != 1) {
            return LineFunc(51478, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(row.raw_06) != 1) {
            return LineFunc(51479, kDispatchSignature, 0, 0);
        }

        for (int cell_index = 0; cell_index <= 5; ++cell_index) {
            FrameLagStatisticRawCell& cell = row.raw_08[cell_index];
            if (packet.get_int(cell.raw_00) != 1) {
                return LineFunc(51487, kDispatchSignature, 0, 0);
            }
            if (packet.get_binary(cell.raw_04, 4) != 1) {
                return LineFunc(51488, kDispatchSignature, 0, 0);
            }
        }
    }

    char raw_tail_count;
    if (packet.get_byte(raw_tail_count) != 1) {
        return LineFunc(51493, kDispatchSignature, 0, 0);
    }
    for (int tail_index = 0; tail_index <= 5; ++tail_index) {
        if (static_cast<signed char>(raw_tail_count) <= tail_index) {
            statistic.raw_174[tail_index] = 0;
            statistic.raw_17a[tail_index] = 0;
        } else {
            if (packet.get_byte(statistic.raw_174[tail_index]) != 1) {
                return LineFunc(51498, kDispatchSignature, 0, 0);
            }
            if (packet.get_short(statistic.raw_17a[tail_index]) != 1) {
                return LineFunc(51499, kDispatchSignature, 0, 0);
            }
        }
    }

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x186);
    return 0;
}

// IDA_EA: 0x8233CC2
// IDA_NAME: _ZN29Dispatcher_FrameLagStatistics4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_FrameLagStatistics::read(PacketBuf &,MSG_BASE &)
int Dispatcher_FrameLagStatistics::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233CCC
// IDA_NAME: _ZN29Dispatcher_FrameLagStatistics7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_FrameLagStatistics::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_FrameLagStatistics::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
