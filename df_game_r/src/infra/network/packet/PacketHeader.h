#ifndef OLDSRV_PROTOCOL_PACKET_DEFS_PACKETHEADER_H_
#define OLDSRV_PROTOCOL_PACKET_DEFS_PACKETHEADER_H_

#include <stddef.h>
#include <stdint.h>

// IDA unit: 99_misc_global/tagPacketHeader. This raw script-sync header lives
// IDA_EA: setPacketID/getSize/setSize/setCategory/ctor
// @0x8106BBC/0x8106BCC/0x8106BD8/0x8106BE6/0x8106BF6.
// in the global namespace in the recovered 140828 binary.
#pragma pack(push, 1)
struct tagPacketHeader {
    tagPacketHeader();

    tagPacketHeader* setPacketID(int packet_id);
    int getSize();
    tagPacketHeader* setSize(int packet_size);
    tagPacketHeader* setCategory(int category);

    uint8_t category;
    uint8_t packet_id;
    uint32_t size;
};
#pragma pack(pop)

static_assert(sizeof(tagPacketHeader) == 6u,
              "tagPacketHeader IDA ABI size must stay 6");


// IDA type: ENUM_PACKETCLASS.
// GetPacketName(ENUM_PACKETCLASS,unsigned short) checks 0 as NOTIPACKET and
// 1 as CMDPACKET in the recovered 140828 df_game_r pseudocode.
enum ENUM_PACKETCLASS : uint8_t {
    ENUM_PACKETCLASS_NOTIPACKET = 0,
    ENUM_PACKETCLASS_CMDPACKET = 1,
};

#pragma pack(push, 1)

// Native internal server-to-server packet header. Packet classes such as
// Packet_Monitor_UDP_Login construct this 10-byte header before their body.
struct PacketHeader {
    uint16_t type;
    uint16_t size;
    uint16_t reserved;
    uint32_t checksum;

    PacketHeader(uint16_t packet_type, uint16_t packet_size)
        : type(packet_type), size(packet_size), reserved(0), checksum(0) {}
};

// Client-to-server frame header observed in the 140828 service protocol.
// cmd: 0 = NOTIPACKET, 1 = CMDPACKET.
struct CS_Header {
    uint8_t cmd;
    uint16_t type;
    uint32_t length;
    uint32_t checksum;
    uint16_t seq;
};

static_assert(sizeof(CS_Header) == 13u,
              "140828 client-to-server header must stay packed at 13 bytes");
static_assert(offsetof(CS_Header, cmd) == 0u &&
                  offsetof(CS_Header, type) == 1u &&
                  offsetof(CS_Header, length) == 3u &&
                  offsetof(CS_Header, checksum) == 7u &&
                  offsetof(CS_Header, seq) == 11u,
              "140828 client-to-server header field offsets changed");

// Native 140828 server-to-client frame header.  The body begins at byte 15:
// this is corroborated by PacketBuf::put_header/finalize and by the 140828
// captured frames whose declared lengths are exactly 15 plus their bodies.
// A 16-byte variation from another version is not valid for this ABI.
struct SC_Header {
    uint8_t cmd;
    uint16_t type;
    uint32_t length;
    uint32_t checksum1;
    uint32_t checksum2;
};

static_assert(sizeof(SC_Header) == 15u,
              "140828 server-to-client header must stay packed at 15 bytes");
static_assert(offsetof(SC_Header, cmd) == 0u &&
                  offsetof(SC_Header, type) == 1u &&
                  offsetof(SC_Header, length) == 3u &&
                  offsetof(SC_Header, checksum1) == 7u &&
                  offsetof(SC_Header, checksum2) == 11u,
              "140828 server-to-client header field offsets changed");

// The integrity helpers deliberately accept both wire directions.  Their
// common prefix is fixed by the 140828 protocol; keep that relationship checked
// here rather than allowing another hard-coded offset family to emerge.
static_assert(offsetof(CS_Header, cmd) == offsetof(SC_Header, cmd) &&
                  offsetof(CS_Header, type) == offsetof(SC_Header, type) &&
                  offsetof(CS_Header, length) == offsetof(SC_Header, length) &&
                  offsetof(CS_Header, checksum) == offsetof(SC_Header, checksum1),
              "C2S/S2C common wire-header prefix changed");

#pragma pack(pop)


#endif  // OLDSRV_PROTOCOL_PACKET_DEFS_PACKETHEADER_H_
