#include "infra/security/policy/network_security_scan_runtime.h"

#include <cstdlib>

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/security/policy/packet_integrity.h"

// IDA_EA: 0x8274BC5
// IDA_NAME: _Z21_NS_NonClient_RequestP5CUser
// DEMANGLED: _NS_NonClient_Request(CUser *)
void _NS_NonClient_Request(CUser* user) {
    const int random_value = ::rand();

    PacketGuard packet;
    packet.put_header(0, 272);
    packet.put_int(random_value);
    packet.finalize(true);
    user->Send(packet);

    user->SetNonClientRandInt(random_value);
    G_TimerQueue()->InsertTimer(
        0,
        user->GetUID(),
        140,
        60,
        static_cast<unsigned int>(
            static_cast<uint16_t>(user->get_unique_id())),
        -1);
}

// IDA_EA: 0x82756CF
// IDA_NAME: _Z14HP_GetHackTypeh
// DEMANGLED: HP_GetHackType(unsigned char)
int HP_GetHackType(uint8_t packet_type) {
    if (packet_type >= 1u && packet_type <= 9u) {
        return static_cast<int>(packet_type) + 900;
    }
    if (packet_type >= 0x0au && packet_type <= 0x29u) {
        return static_cast<int>(packet_type) + 1390;
    }
    return 900;
}

// IDA_EA: 0x827585C
// IDA_NAME: _Z15HIP_GetHackTypeh
// DEMANGLED: HIP_GetHackType(unsigned char)
int HIP_GetHackType(uint8_t packet_type) {
    switch (packet_type) {
    case 1u:
    case 2u:
    case 3u:
    case 4u:
    case 6u:
    case 7u:
    case 8u:
        return 1500 + static_cast<int>(packet_type);
    case 9u:
        return 1601;
    case 10u:
        return 1602;
    case 11u:
        return 1603;
    case 13u:
        return 1606;
    case 14u:
        return 1607;
    case 15u:
        return 1608;
    case 16u:
        return 1657;
    default:
        return 1500;
    }
}

// IDA_EA: 0x8274AD7
// IDA_NAME: _Z14MI_GetHackTypec
// DEMANGLED: MI_GetHackType(char)
int MI_GetHackType(char packet_type) {
    int hack_type = static_cast<int>(static_cast<uint8_t>(packet_type)) + 1700;
    if (hack_type > 1799) {
        hack_type = 1799;
    }
    return hack_type;
}

// IDA_EA: 0x8274523
// IDA_NAME: _Z15_NS_ProcessScanP5CUserR9PacketBuf
// DEMANGLED: _NS_ProcessScan(CUser *,PacketBuf &)
int _NS_ProcessScan(CUser* user, PacketBuf& packet) {
    char packet_type = 0;
    char tens_digit = 0;
    char hundreds_digit = 0;
    if (packet.get_byte(packet_type) != 1) {
        return 1;
    }
    if (packet.get_byte(tens_digit) != 1) {
        return 2;
    }
    if (packet.get_byte(hundreds_digit) != 1) {
        return 3;
    }

    const int encoded_value =
        static_cast<int>(static_cast<uint8_t>(tens_digit) % 10u) +
        10 * static_cast<int>(static_cast<uint8_t>(hundreds_digit) % 10u);
    const int hack_type = HP_GetHackType(static_cast<uint8_t>(packet_type));
    const unsigned int previous = static_cast<unsigned int>(
        user->SetETC(static_cast<short>(hack_type), encoded_value));
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, previous,
        0u);
    return 0;
}

// IDA_EA: 0x8274676
// IDA_NAME: _Z10_NS_IPScanP5CUserR9PacketBuf
// DEMANGLED: _NS_IPScan(CUser *,PacketBuf &)
int _NS_IPScan(CUser* user, PacketBuf& packet) {
    char packet_type = 0;
    if (packet.get_byte(packet_type) != 1) {
        return 1;
    }
    const int hack_type = HIP_GetHackType(static_cast<uint8_t>(packet_type));
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, 0u, 0u);
    return 0;
}

// IDA_EA: 0x8274975
// IDA_NAME: _Z19_NS_PacketIntegrityP5CUserR9PacketBuf
// DEMANGLED: _NS_PacketIntegrity(CUser *,PacketBuf &)
int _NS_PacketIntegrity(CUser* user, PacketBuf& packet) {
    char hundreds_digit = 0;
    char tens_digit = 0;
    char integrity_type = 0;
    char ones_digit = 0;
    if (packet.get_byte(hundreds_digit) != 1) {
        return 1;
    }
    if (packet.get_byte(tens_digit) != 1) {
        return 2;
    }
    if (packet.get_byte(integrity_type) != 1) {
        return 3;
    }
    if (packet.get_byte(ones_digit) != 1 || !user) {
        return 4;
    }

    const int encoded_value =
        static_cast<int>(static_cast<uint8_t>(ones_digit) % 10u) +
        100 * static_cast<int>(static_cast<uint8_t>(hundreds_digit) % 10u) +
        10 * static_cast<int>(static_cast<uint8_t>(tens_digit) % 10u);
    PIAddServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(
            PI_GetHackType(static_cast<int16_t>(static_cast<uint8_t>(integrity_type)))),
        encoded_value);
    return 0;
}

// IDA_EA: 0x8274CC3
// IDA_NAME: _Z25_NS_NonClient_GetResponseP5CUserR9PacketBuf
// DEMANGLED: _NS_NonClient_GetResponse(CUser *,PacketBuf &)
int _NS_NonClient_GetResponse(CUser* user, PacketBuf& packet) {
    if (!user) {
        return 0;
    }

    user->SetNonClientFlag(true);
    int response_hash = 0;
    char response_type = 0;
    if (packet.get_int(response_hash) != 1) {
        return 1;
    }
    if (packet.get_byte(response_type) != 1) {
        return 2;
    }

    if (response_type != 0) {
        const int hack_type = response_type == 1 ? 0x33d : 0x33e;
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, 0u,
            0u);
        return 0;
    }

    int random_value = user->GetNonClientRandInt();
    int expected_hash = 0;
    if (_NS_MakeHash_(reinterpret_cast<char*>(&random_value), 4,
                      expected_hash) != 1) {
        return 0;
    }
    if (expected_hash != response_hash) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x33b), 1u, 0u, 0u);
    }
    return 0;
}

// IDA_EA: 0x8274B09
// IDA_NAME: _Z36_NS_MI_RecvMemoryIntegrityModulationP5CUserR9PacketBuf
// DEMANGLED: _NS_MI_RecvMemoryIntegrityModulation(CUser *,PacketBuf &)
int _NS_MI_RecvMemoryIntegrityModulation(CUser* user, PacketBuf& packet) {
    char packet_type = 0;
    char value = 0;
    if (packet.get_byte(packet_type) != 1) {
        return 1;
    }
    if (packet.get_byte(value) != 1) {
        return 2;
    }

    const int hack_type = MI_GetHackType(packet_type);
    const unsigned int previous = static_cast<unsigned int>(
        user->SetETC(static_cast<short>(hack_type), static_cast<int>(value)));
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, previous,
        0u);
    return 0;
}

// IDA_EA: 0x8275140
// IDA_NAME: _Z44_NS_PI_2ND_RecvPacketIntegrityModulation_S2CP5CUserR9PacketBuf
// DEMANGLED: _NS_PI_2ND_RecvPacketIntegrityModulation_S2C(CUser *,PacketBuf &)
int _NS_PI_2ND_RecvPacketIntegrityModulation_S2C(CUser* user,
                                                  PacketBuf& packet) {
    char packet_class = 0;
    uint16_t packet_index = 0;
    if (packet.get_byte(packet_class) != 1) {
        return 1;
    }
    if (packet.get_short(packet_index) != 1) {
        return 2;
    }
    const int hack_type = _NS_PI_2ND_GetHackType(
        1u, static_cast<uint8_t>(packet_class), packet_index);
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, 0u, 0u);
    return 0;
}

// IDA_EA: 0x8275290
// IDA_NAME: _Z44_NS_PI_2ND_RecvPacketIntegrityModulation_C2CP5CUserR9PacketBuf
// DEMANGLED: _NS_PI_2ND_RecvPacketIntegrityModulation_C2C(CUser *,PacketBuf &)
int _NS_PI_2ND_RecvPacketIntegrityModulation_C2C(CUser* user,
                                                  PacketBuf& packet) {
    uint16_t packet_index = 0;
    if (packet.get_short(packet_index) != 1) {
        return 1;
    }
    int hack_type = static_cast<int>(packet_index) + 3601;
    if (static_cast<unsigned int>(hack_type) > 0xf9eu) {
        hack_type = 3999;
    }
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, 0u, 0u);
    return 0;
}
