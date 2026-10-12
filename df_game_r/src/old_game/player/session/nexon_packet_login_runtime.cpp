#include "old_game/player/session/nexon_packet_login_runtime.h"

#include <arpa/inet.h>

namespace nexon_packet {

// IDA_EA: 0x86940D2
// IDA_NAME: _ZN12nexon_packet13CPacketHelperC2EPhii
CPacketHelper::CPacketHelper(unsigned char* bytes, int max_size, int offset)
    : offset_(offset), max_size_(max_size), bytes_(bytes) {}

// IDA_EA: 0x86940F2
// IDA_NAME: _ZN12nexon_packet13CPacketHelper10get_offsetEv
int CPacketHelper::get_offset() { return offset_; }

// IDA_EA: 0x86940FC
// IDA_NAME: _ZN12nexon_packet13CPacketHelper7paddingEPhi
int CPacketHelper::padding(unsigned char* input, int size) {
    if (size + offset_ >= max_size_) {
        return 0;
    }
    for (int index = 0; index < size; ++index) {
        bytes_[offset_] = input[index];
        ++offset_;
    }
    return 1;
}

// IDA_EA: 0x8694160
// IDA_NAME: _ZN12nexon_packet13CPacketHelper4scanEPhi
int CPacketHelper::scan(unsigned char* output, int size) {
    if (size + offset_ >= max_size_) {
        return 0;
    }
    for (int index = 0; index < size; ++index) {
        output[index] = bytes_[offset_];
        ++offset_;
    }
    return 1;
}

// IDA_EA: 0x86941CC
// IDA_NAME: _ZN12nexon_packet8stHeaderC2Ev
stHeader::stHeader() : marker(0xAA), packet_length(0), packet_type(0) {}

// IDA_EA: 0x86941E8
// IDA_NAME: _ZN12nexon_packet8stHeader9SetLengthEt
int stHeader::SetLength(uint16_t length) {
    packet_length = htons(length);
    return static_cast<int>(packet_length);
}

// IDA_EA: 0x869420A
// IDA_NAME: _ZN12nexon_packet8stHeader9GetLengthEv
uint16_t stHeader::GetLength() { return ntohs(packet_length); }

// IDA_EA: 0x869423E
// IDA_NAME: _ZN12nexon_packet8stPacketC2Ev
stPacket::stPacket() : stHeader() {}

// IDA_EA: 0x8694224
// IDA_NAME: _ZN12nexon_packet8stPacket9GetLengthEv
int stPacket::GetLength() { return static_cast<int>(stHeader::GetLength()) + 3; }

// IDA_EA: 0x8694252
// IDA_NAME: _ZN12nexon_packet13stLoginPacketC2ERNS_7stLoginE
stLoginPacket::stLoginPacket(stLogin& login) : stPacket() {
    CPacketHelper helper(payload_, sizeof(payload_), 0);
    helper.padding(&login.login_type, 1);
    helper.padding(&login.account_name_length, 1);
    helper.padding(reinterpret_cast<unsigned char*>(login.account_name),
                   login.account_name_length);
    helper.padding(&login.charac_name_length, 1);
    helper.padding(reinterpret_cast<unsigned char*>(login.charac_name),
                   login.charac_name_length);
    helper.padding(reinterpret_cast<unsigned char*>(&login.ip_address), 4);
    helper.padding(&login.can_try, 1);
    helper.padding(reinterpret_cast<unsigned char*>(&login.uid), 8);
    SetLength(static_cast<uint16_t>(helper.get_offset() + 1));
    packet_type = 39;
}

// IDA_EA: 0x86943AF
// IDA_NAME: _ZN12nexon_packet13stLoginPacket10ReadPacketEPKcRNS_7stLoginE
int stLoginPacket::ReadPacket(const char* packet, stLogin& login) {
    const stLoginPacket* const wire =
        reinterpret_cast<const stLoginPacket*>(packet);
    CPacketHelper helper(const_cast<unsigned char*>(wire->payload_),
                         sizeof(wire->payload_),
                         0);
    helper.scan(&login.login_type, 1);
    helper.scan(&login.account_name_length, 1);
    helper.scan(reinterpret_cast<unsigned char*>(login.account_name),
                login.account_name_length);
    helper.scan(&login.charac_name_length, 1);
    helper.scan(reinterpret_cast<unsigned char*>(login.charac_name),
                login.charac_name_length);
    helper.scan(reinterpret_cast<unsigned char*>(&login.ip_address), 4);
    helper.scan(&login.can_try, 1);
    return helper.scan(reinterpret_cast<unsigned char*>(&login.uid), 8);
}

}  // namespace nexon_packet
