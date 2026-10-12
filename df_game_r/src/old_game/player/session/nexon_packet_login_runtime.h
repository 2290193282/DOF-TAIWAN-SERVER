#ifndef OLDSRV_OLD_GAME_USERINFO_NEXON_PACKET_LOGIN_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_NEXON_PACKET_LOGIN_RUNTIME_H_

#include <stdint.h>

namespace nexon_packet {

// IDA type recovered from the padding sequence in stLoginPacket @0x8694252.
// These packed offsets are protocol offsets, not a CUser overlay.
#pragma pack(push, 1)
struct stHeader {
    unsigned char marker;          // +0x00
    uint16_t packet_length;        // +0x01, network byte order
    unsigned char packet_type;     // +0x03

    // IDA_EA: 0x86941CC
    // IDA_NAME: _ZN12nexon_packet8stHeaderC2Ev
    stHeader();

    // IDA_EA: 0x86941E8 / 0x869420A
    int SetLength(uint16_t length);
    uint16_t GetLength();
};

struct stPacket : public stHeader {
    // IDA_EA: 0x869423E / 0x8694224
    stPacket();
    int GetLength();
};

struct stLogin {
    unsigned char login_type;           // +0x000
    unsigned char account_name_length;  // +0x001
    char account_name[255];             // +0x002
    unsigned char charac_name_length;   // +0x101
    char charac_name[255];              // +0x102
    int ip_address;                     // +0x201
    unsigned char can_try;              // +0x205
    int uid;                            // +0x206
    int account_id;                     // +0x20A
};
#pragma pack(pop)

class CPacketHelper {
public:
    // IDA_EA: 0x86940D2
    // IDA_NAME: _ZN12nexon_packet13CPacketHelperC2EPhii
    CPacketHelper(unsigned char* bytes, int max_size, int offset);

    // IDA_EA: 0x86940F2 / 0x86940FC / 0x8694160
    int get_offset();
    int padding(unsigned char* input, int size);
    int scan(unsigned char* output, int size);

private:
    int offset_;
    int max_size_;
    unsigned char* bytes_;
};

class stLoginPacket : public stPacket {
public:
    // IDA_EA: 0x8694252
    // IDA_NAME: _ZN12nexon_packet13stLoginPacketC2ERNS_7stLoginE
    explicit stLoginPacket(stLogin& login);

    // IDA_EA: 0x86943AF
    // IDA_NAME: _ZN12nexon_packet13stLoginPacket10ReadPacketEPKcRNS_7stLoginE
    static int ReadPacket(const char* packet, stLogin& login);

private:
    // The two original callers reserve 4100 bytes: the four-byte header plus
    // CPacketHelper's 4096-byte payload window.
    unsigned char payload_[4096];
};

static_assert(sizeof(stHeader) == 4u, "stHeader must retain the old wire prefix");
static_assert(sizeof(stLogin) == 526u, "stLogin must retain the recovered wire extent");
static_assert(sizeof(stLoginPacket) == 4100u,
              "stLoginPacket must retain the old 4100-byte wire storage");

}  // namespace nexon_packet

#endif  // OLDSRV_OLD_GAME_USERINFO_NEXON_PACKET_LOGIN_RUNTIME_H_
