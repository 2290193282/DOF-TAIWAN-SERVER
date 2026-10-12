#ifndef OLDSRV_SECURITY_PACKET_INTEGRITY_H_
#define OLDSRV_SECURITY_PACKET_INTEGRITY_H_

#include <stdint.h>


class CUser;
class PacketBuf;
enum ENUM_PACKETCLASS : uint8_t;

namespace WongWork {
enum ENUM_HACKTYPE : int;
}

// IDA: PI_GetType_CS @0x808C5D8.
char PI_GetType_CS(char packet_class, int16_t packet_id);

// IDA: _NS_PI_Check_CMD @0x8274778.
char _NS_PI_Check_CMD(ENUM_PACKETCLASS packet_class,
                      uint16_t packet_id,
                      PacketBuf& packet,
                      CUser* user);

// IDA: _NS_MakeHash_ @0x808C8B5.
int _NS_MakeHash_(char* src, int16_t n400, int& p_n603);

// IDA: _NS_PI_MakeHash_NOTI @0x82746F3.
void _NS_PI_MakeHash_NOTI(PacketBuf* packet);

// IDA: _NS_PI_2ND_MakeChecksumTo1Byte @0x808CC17.
char* _NS_PI_2ND_MakeChecksumTo1Byte(char* a1);

// IDA: _NS_PI_2ND_MakeChecksumTo1Byte2 @0x808CC4B.
// The old root dispatcher passes the server and client checksum words by
// reference, reduces both to their low bytes, and uses the returned second
// reference for the equality test.
uint32_t& _NS_PI_2ND_MakeChecksumTo1Byte2(uint32_t& server_checksum,
                                          uint32_t& client_checksum);

// IDA: _NS_PI_2ND_ReconstructCheckSumRandomkey @0x808CA60.
uint32_t* _NS_PI_2ND_ReconstructCheckSumRandomkey(uint32_t& a1,
                                                   uint32_t a2);

// IDA: _NS_PI_2ND_GetPacketIndex @0x82772A9.
int _NS_PI_2ND_GetPacketIndex(char* packetEi);

// IDA: receive-side PI state transition and packet-class/id mapping
// @0x8274F73 / 0x8275AF3 / 0x8277359.
int _NS_PI_2ND_GetHackTypeEnum(uint8_t packet_class,
                                int16_t packet_id);
int _NS_PI_2ND_GetHackType(uint32_t extra,
                            uint8_t packet_class,
                            uint16_t packet_id);
int _NS_PI_2ND_RecvHash_CS(char* packet,
                            CUser* user,
                            uint32_t packet_length);

// IDA: PI_GetHackType @0x82758E3 and PIAddServerHackCnt @0x82772B7.
int PI_GetHackType(int16_t packet_integrity_type);
char PIAddServerHackCnt(CUser* user,
                        WongWork::ENUM_HACKTYPE hack_type,
                        int value);

// IDA: _NS_PI_2ND_SendHash_SC @0x82751EE.
uint32_t* _NS_PI_2ND_SendHash_SC(char* packet,
                                  uint16_t length,
                                  CUser* user);

class Encryption {
public:
    // Old direct ELF boundary: libnxencryption.so.  These are declarations of
    // the original imported symbols, not an adapter or runtime library load.
    static uint32_t Adler32(char* data, unsigned int length);
    static const char* EncodeDressString(const char* input,
                                         char* output,
                                         unsigned long key);
    static const char* DecodeDressString(const char* input,
                                         char* output,
                                         unsigned long key);
    static const wchar_t* EncodeDressStringWide(const wchar_t* input,
                                                wchar_t* output,
                                                unsigned long key);
    static const wchar_t* DecodeDressStringWide(const wchar_t* input,
                                                wchar_t* output,
                                                unsigned long key);

    // IDA: Encryption::make_crc_table @0x848D3E4.
    static bool make_crc_table();

    // IDA: Encryption::crc32 @0x848D472.
    static uint32_t crc32(uint32_t crc, const char* buf, unsigned int len);

    // IDA: Encryption::crc32N @0x848D4C8.
    static uint32_t crc32N(void* payload, int payload_size);

    // IDA: Encryption::SeedRand @0x848D51A.
    static int SeedRand(uint32_t& seed);

    // IDA: Encryption::Encrypt @0x848DA70.
    static void Encrypt(void* data, int size, uint32_t& seed);

private:
    static uint32_t crc_table_[256];
    static bool crc_table_initialized_;
};


#endif  // OLDSRV_SECURITY_PACKET_INTEGRITY_H_
