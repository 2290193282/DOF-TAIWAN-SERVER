#include "infra/network/dispatcher/lag_statistics_dispatcher.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_Lag_Statistics::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x822C144
// IDA_NAME: _ZN26Packet_Stat_Lag_StatisticsC2Ev
// DEMANGLED: Packet_Stat_Lag_Statistics::Packet_Stat_Lag_Statistics(void)
Packet_Stat_Lag_Statistics::Packet_Stat_Lag_Statistics()
    : header(0x17B9u, 0x014Cu) {
    // The original does not bulk-zero this packet: it writes the module
    // count, initializes eight 10-byte module records, then writes the
    // dungeon count and initializes ten 24-byte dungeon records.
    module_count = 0;
    for (int index = 0; index != 8; ++index) {
        LagStatisticsModuleRaw& module = modules[index];
        // Mirrors stModuleLagInfo::C2's three writes at +0, +4, and +8.
        module.raw_00 = 0;
        module.raw_04 = 0;
        module.raw_08 = 0;
    }
    dungeon_count = 0;
    for (int index = 0; index != 10; ++index) {
        LagStatisticsDungeonRaw& dungeon = dungeons[index];
        // Mirrors stDunGeonLagInfo::C2's exact field-write order.  The packed
        // offsets intentionally include the old unaligned dwords.
        dungeon.raw_00 = 0;
        dungeon.raw_02 = 0;
        dungeon.raw_03 = 0;
        dungeon.raw_07 = 0;
        dungeon.raw_0b = 0;
        dungeon.raw_0d = 0;
        dungeon.raw_0e = 0;
        dungeon.raw_12 = 0;
        dungeon.raw_16 = 0;
    }
}

// IDA_EA: 0x82354EC
// IDA_NAME: _ZN26Packet_Stat_Lag_StatisticsD2Ev
// DEMANGLED: Packet_Stat_Lag_Statistics::~Packet_Stat_Lag_Statistics()
Packet_Stat_Lag_Statistics::~Packet_Stat_Lag_Statistics() {
    // Both raw nested destructors are no-ops.  The original reverse loops
    // therefore have no observable write or ownership effect to reproduce.
}

// IDA_EA: 0x8596F6C
// IDA_NAME: _ZN25Dispatcher_Lag_StatisticsC2Ev
// DEMANGLED: Dispatcher_Lag_Statistics::Dispatcher_Lag_Statistics(void)
Dispatcher_Lag_Statistics::Dispatcher_Lag_Statistics() {}

// IDA_EA: 0x8240AAA
// IDA_NAME: _ZN25Dispatcher_Lag_StatisticsD2Ev
// DEMANGLED: Dispatcher_Lag_Statistics::~Dispatcher_Lag_Statistics()
// IDA_EA: 0x8240ADA
// IDA_NAME: _ZN25Dispatcher_Lag_StatisticsD0Ev
// DEMANGLED: Dispatcher_Lag_Statistics::~Dispatcher_Lag_Statistics()
Dispatcher_Lag_Statistics::~Dispatcher_Lag_Statistics() {}

// IDA_EA: 0x8223DE0
// IDA_NAME: _ZN25Dispatcher_Lag_Statistics12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Lag_Statistics::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Lag_Statistics::dispatch_sig(CUser* user, PacketBuf& packet) {
    Packet_Stat_Lag_Statistics statistic;

    if (packet.get_byte(statistic.module_count) != 1) {
        return LineFunc(56991, kDispatchSignature, 0, 0);
    }
    if (statistic.module_count > 7u) {
        return LineFunc(56993, kDispatchSignature, 0, 0);
    }

    uint8_t module_index = 0;
    for (int index = 0; statistic.module_count > index; ++index) {
        if (packet.get_byte(module_index) != 1) {
            return LineFunc(56999, kDispatchSignature, 0, 0);
        }
        if (module_index > 7u) {
            return LineFunc(57003, kDispatchSignature, 0, 0);
        }

        LagStatisticsModuleRaw& module = statistic.modules[module_index];
        if (packet.get_int(module.raw_00) != 1) {
            return LineFunc(57005, kDispatchSignature, 0, 0);
        }
        if (packet.get_int(module.raw_04) != 1) {
            return LineFunc(57006, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(module.raw_08) != 1) {
            return LineFunc(57007, kDispatchSignature, 0, 0);
        }

        if (module_index != 1u) {
            continue;
        }

        if (packet.get_byte(statistic.dungeon_count) != 1) {
            return LineFunc(57012, kDispatchSignature, 0, 0);
        }
        if (statistic.dungeon_count > 10u) {
            return LineFunc(57014, kDispatchSignature, 0, 0);
        }

        for (int dungeon_index = 0;
             statistic.dungeon_count > dungeon_index;
             ++dungeon_index) {
            LagStatisticsDungeonRaw& dungeon = statistic.dungeons[dungeon_index];
            if (packet.get_short(dungeon.raw_00) != 1) {
                return LineFunc(57020, kDispatchSignature, 0, 0);
            }
            if (packet.get_byte(dungeon.raw_02) != 1) {
                return LineFunc(57021, kDispatchSignature, 0, 0);
            }
            if (packet.get_int(dungeon.raw_03) != 1) {
                return LineFunc(57022, kDispatchSignature, 0, 0);
            }
            if (packet.get_int(dungeon.raw_07) != 1) {
                return LineFunc(57023, kDispatchSignature, 0, 0);
            }
            if (packet.get_short(dungeon.raw_0b) != 1) {
                return LineFunc(57024, kDispatchSignature, 0, 0);
            }
            if (packet.get_byte(dungeon.raw_0d) != 1) {
                return LineFunc(57025, kDispatchSignature, 0, 0);
            }
            if (packet.get_int(dungeon.raw_0e) != 1) {
                return LineFunc(57026, kDispatchSignature, 0, 0);
            }
            if (packet.get_int(dungeon.raw_12) != 1) {
                return LineFunc(57027, kDispatchSignature, 0, 0);
            }
            if (packet.get_short(dungeon.raw_16) != 1) {
                return LineFunc(57028, kDispatchSignature, 0, 0);
            }
        }
    }

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x14C);
    return 0;
}

// IDA_EA: 0x8234096
// IDA_NAME: _ZN25Dispatcher_Lag_Statistics4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Lag_Statistics::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Lag_Statistics::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82340A0
// IDA_NAME: _ZN25Dispatcher_Lag_Statistics7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Lag_Statistics::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Lag_Statistics::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
