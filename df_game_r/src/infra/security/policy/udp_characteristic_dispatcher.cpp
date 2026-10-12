#include "infra/security/policy/udp_characteristic_dispatcher.h"

#include <cstring>

#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

const char kDispatchSignature[] =
    "virtual int DisPatcher_UdpCharacteristic::dispatch_sig(CUser*, PacketBuf&)";

// Raw 0x821ABE0 constructs Packet_Abnormal_Exit_Statistic at stack offset
// +0x00, then overlays its two body words at +0x0A and +0x0C before sending
// exactly 0x0E bytes.  The shared packet constructor has no active public
// provider outside the raw export, so preserve the proven wire ABI locally
// rather than introduce a second owner for that unrelated packet class.
#pragma pack(push, 1)
struct AbnormalExitStatisticPacket {
    AbnormalExitStatisticPacket() : header(0x0FA9u, 0x000Eu) {}

    PacketHeader header;
    unsigned short first;
    unsigned short total;
};
#pragma pack(pop)

static_assert(sizeof(AbnormalExitStatisticPacket) == 0x0Eu,
              "CMD212 abnormal-exit statistic packet must remain 14 bytes");
static_assert(sizeof(Packet_Party_Result_Statistic) == 0x0Eu,
              "CMD212 party-result statistic packet must remain 14 bytes");
static_assert(sizeof(Packet_Party_Ping_Statistic) == 0x0Eu,
              "CMD212 party-ping statistic packet must remain 14 bytes");
static_assert(sizeof(Packet_Pvp_Ping_Statistic) == 0x0Eu,
              "CMD212 PvP-ping statistic packet must remain 14 bytes");
static_assert(sizeof(Packet_Fair_Pvp_Ping_Statistic) == 0x0Eu,
              "CMD212 fair-PvP-ping statistic packet must remain 14 bytes");

unsigned short AddCounter(unsigned short first, unsigned short second) {
    // The original writes into a 16-bit stack slot (`second += first`), so
    // retain its modulo-2^16 result rather than widening the statistic body.
    return static_cast<unsigned short>(first + second);
}

void SetRawStatisticCounters(unsigned char* body,
                             unsigned short first,
                             unsigned short total) {
    std::memcpy(body, &first, sizeof(first));
    std::memcpy(body + sizeof(first), &total, sizeof(total));
}

}  // namespace

// IDA_EA: 0x8596968
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristicC2Ev
// DEMANGLED: DisPatcher_UdpCharacteristic::DisPatcher_UdpCharacteristic(void)
DisPatcher_UdpCharacteristic::DisPatcher_UdpCharacteristic() {}

// IDA_EA: 0x8241728
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristicD2Ev
// DEMANGLED: DisPatcher_UdpCharacteristic::~DisPatcher_UdpCharacteristic()
// IDA_EA: 0x8241758
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristicD0Ev
// DEMANGLED: DisPatcher_UdpCharacteristic::~DisPatcher_UdpCharacteristic()
DisPatcher_UdpCharacteristic::~DisPatcher_UdpCharacteristic() {}

// IDA_EA: 0x821ABE0
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_UdpCharacteristic::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_UdpCharacteristic::dispatch_sig(CUser* user, PacketBuf& packet) {
    unsigned short characteristic = 0;
    unsigned char statistic_type = 0;
    if (packet.get_short(characteristic) != 1) {
        return LineFunc(52754, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(statistic_type) != 1) {
        return LineFunc(52756, kDispatchSignature, 0, 0);
    }

    if (characteristic == 1) {
        unsigned short first = 0;
        unsigned short second = 0;
        if (packet.get_short(first) != 1) {
            return LineFunc(52784, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(second) != 1) {
            return LineFunc(52785, kDispatchSignature, 0, 0);
        }

        if (statistic_type == 0) {
            return partyPingStat(user, first, second);
        }
        if (statistic_type == 1) {
            return pvpPingStat(user, first, second);
        }
        if (statistic_type == 2) {
            return fairPvpPingStat(user, first, second);
        }
        return LineFunc(52804, kDispatchSignature, 0, 0);
    }

    if (characteristic == 2) {
        unsigned short first = 0;
        unsigned short second = 0;
        AbnormalExitStatisticPacket statistic;
        if (packet.get_short(first) != 1) {
            return LineFunc(52840, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(second) != 1) {
            return LineFunc(52841, kDispatchSignature, 0, 0);
        }
        statistic.first = first;
        statistic.total = AddCounter(first, second);
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
        return 0;
    }

    if (characteristic == 0) {
        unsigned short first = 0;
        unsigned short second = 0;
        Packet_Party_Result_Statistic statistic;
        if (packet.get_short(first) != 1) {
            return LineFunc(52765, kDispatchSignature, 0, 0);
        }
        if (packet.get_short(second) != 1) {
            return LineFunc(52766, kDispatchSignature, 0, 0);
        }
        SetRawStatisticCounters(statistic.raw_0a,
                                first,
                                AddCounter(first, second));
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
        return 0;
    }

    return LineFunc(52854, kDispatchSignature, 0, 0);
}

// IDA_EA: 0x821AFBC
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic11pvpPingStatEP5CUserRtS2_
// DEMANGLED: DisPatcher_UdpCharacteristic::pvpPingStat(CUser *,unsigned short &,unsigned short &)
int DisPatcher_UdpCharacteristic::pvpPingStat(CUser* user,
                                               unsigned short& first,
                                               unsigned short& second) {
    Packet_Pvp_Ping_Statistic statistic;
    SetRawStatisticCounters(statistic.raw_0a,
                            first,
                            AddCounter(first, second));
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
    return 0;
}

// IDA_EA: 0x821B02A
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic15fairPvpPingStatEP5CUserRtS2_
// DEMANGLED: DisPatcher_UdpCharacteristic::fairPvpPingStat(CUser *,unsigned short &,unsigned short &)
int DisPatcher_UdpCharacteristic::fairPvpPingStat(
    CUser* user,
    unsigned short& first,
    unsigned short& second) {
    Packet_Fair_Pvp_Ping_Statistic statistic;
    statistic.word_0a = first;
    statistic.word_0c = AddCounter(first, second);
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
    return 0;
}

// IDA_EA: 0x821B098
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic13partyPingStatEP5CUserRtS2_
// DEMANGLED: DisPatcher_UdpCharacteristic::partyPingStat(CUser *,unsigned short &,unsigned short &)
int DisPatcher_UdpCharacteristic::partyPingStat(CUser* user,
                                                 unsigned short& first,
                                                 unsigned short& second) {
    Packet_Party_Ping_Statistic statistic;
    SetRawStatisticCounters(statistic.raw_0a,
                            first,
                            AddCounter(first, second));
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
    return 0;
}

// IDA_EA: 0x8233D62
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_UdpCharacteristic::read(PacketBuf &,MSG_BASE &)
int DisPatcher_UdpCharacteristic::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D6C
// IDA_NAME: _ZN28DisPatcher_UdpCharacteristic7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UdpCharacteristic::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_UdpCharacteristic::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
