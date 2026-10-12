#include "infra/security/policy/packet_integrity.h"

#include <map>
#include <utility>

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

typedef std::map<std::pair<char, char>, int> PacketIntegrityCountMap;

}  // namespace

// IDA_EA: 0x827565A
// IDA_NAME: _ZN5CUser10GetPICSMapEv
// DEMANGLED: CUser::GetPICSMap(void)
PacketIntegrityCountMap& CUser::GetPICSMap() {
    return pair_values_;
}

// IDA_EA: 0x808C5D8
// IDA_NAME: _Z13PI_GetType_CScs
// DEMANGLED: PI_GetType_CS(char,short)
char PI_GetType_CS(char packet_class, int16_t packet_id) {
    if (packet_class != 1) {
        return 0;
    }

    switch (packet_id) {
    case ENUM_CMDPACKET_HT_IS:
        return 2;
    case ENUM_CMDPACKET_PI:
        return 3;
    case ENUM_CMDPACKET_NCC:
        return 40;
    case ENUM_CMDPACKET_MI:
        return 38;
    case ENUM_CMDPACKET_PIV:
        return 41;
    case ENUM_CMDPACKET_PID:
        return 42;
    case ENUM_CMDPACKET_HACK_SCRIPT_HASH:
        return 50;
    case ENUM_CMDPACKET_SCAN_BOT_BY_DLL:
        return 46;
    case ENUM_CMDPACKET_SCAN_BOT_BY_DRV:
        return 49;
    case ENUM_CMDPACKET_DIE_MONSTER:
        return 43;
    case ENUM_CMDPACKET_SET_PLAY_RESULT:
        return 45;
    case ENUM_CMDPACKET_RPY_HUMAN_CERTIFY:
        return 33;
    case ENUM_CMDPACKET_CHANGE_ANOTHER_SKILL_TREE:
        return 36;
    case ENUM_CMDPACKET_HT_PS:
        return 1;
    case ENUM_CMDPACKET_BUY_SKILL:
        return 10;
    case ENUM_CMDPACKET_USE_SKILL:
        return 35;
    case ENUM_CMDPACKET_CHANGE_SKILLSLOT:
        return 37;
    default:
        return 0;
    }
}

// IDA_EA: 0x82758E3
// IDA_NAME: _Z14PI_GetHackTypes
// DEMANGLED: PI_GetHackType(short)
int PI_GetHackType(int16_t packet_integrity_type) {
    if (packet_integrity_type >= 1 && packet_integrity_type <= 9) {
        return 1300 + packet_integrity_type;
    }
    if (packet_integrity_type == 10) {
        return 1354;
    }
    if (packet_integrity_type >= 11 && packet_integrity_type <= 54) {
        return 1299 + packet_integrity_type;
    }
    return 1300;
}

// IDA_EA: 0x82772B7
// IDA_NAME: _Z18PIAddServerHackCntP5CUserN8WongWork13ENUM_HACKTYPEEi
// DEMANGLED: PIAddServerHackCnt(CUser *,WongWork::ENUM_HACKTYPE,int)
char PIAddServerHackCnt(CUser* user,
                        WongWork::ENUM_HACKTYPE hack_type,
                        int value) {
    CParty* party = 0;
    if (hack_type != static_cast<WongWork::ENUM_HACKTYPE>(1316) ||
        !static_cast<unsigned char>(user->CheckInParty()) ||
        (party = user->GetParty()) == 0 || party->get_member_count() <= 1) {
        const unsigned int etc_value =
            static_cast<unsigned int>(user->SetETC(hack_type, value));
        return user->getHackAnalyzer()->addServerHackCnt(
            user,
            hack_type,
            1u,
            etc_value,
            0u);
    }
    return 1;
}

// IDA_EA: 0x8274778
// IDA_NAME: _Z16_NS_PI_Check_CMD16ENUM_PACKETCLASStR9PacketBufP5CUser
// DEMANGLED: _NS_PI_Check_CMD(ENUM_PACKETCLASS,unsigned short,PacketBuf &,CUser *)
char _NS_PI_Check_CMD(ENUM_PACKETCLASS packet_class,
                      uint16_t packet_id,
                      PacketBuf& packet,
                      CUser* user) {
    if (!user) {
        return 0;
    }

    const char packet_class_key =
        static_cast<int>(packet_class) == 0
            ? 2
            : static_cast<char>(static_cast<int>(packet_class) == 1);
    const char integrity_type = PI_GetType_CS(
        packet_class_key, static_cast<int16_t>(packet_id));
    if (integrity_type == 0) {
        return 0;
    }

    char* const payload = packet.get_packet(0) + sizeof(CS_Header);
    const int16_t payload_length =
        static_cast<int16_t>(packet.get_index() - sizeof(CS_Header));
    int received_hash = 0;
    packet.get_int(received_hash);

    const std::pair<char, char> key(packet_class_key, integrity_type);
    PacketIntegrityCountMap& counts = user->GetPICSMap();
    if (counts.find(key) == counts.end()) {
        counts[key] = 0;
    }

    int& mismatch_count = counts[key];
    if (mismatch_count > 100) {
        return 0;
    }

    int expected_hash = 0;
    if (_NS_MakeHash_(payload, payload_length, expected_hash) != 1) {
        return 0;
    }
    if (expected_hash == received_hash) {
        return 0;
    }

    ++mismatch_count;
    const int report_value = (packet_class_key % 10) * 10 + 201;
    return PIAddServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(PI_GetHackType(integrity_type)),
        report_value);
}
