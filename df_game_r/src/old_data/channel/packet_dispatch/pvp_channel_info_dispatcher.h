#ifndef OLDSRV_OLD_DATA_CHANNEL_PACKET_DISPATCH_PVP_CHANNEL_INFO_DISPATCHER_H_
#define OLDSRV_OLD_DATA_CHANNEL_PACKET_DISPATCH_PVP_CHANNEL_INFO_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// The monitor-side reply handler consumes each tail record at a 16-byte
// stride.  The names intentionally state only the observed transport role:
// the old IDA does not prove a richer semantic name for these four dwords.
#pragma pack(push, 1)
struct PvPChannelInfoEntry {
    uint32_t value_0;       // packet +0x18 + entry * 0x10
    uint32_t value_1;       // packet +0x1C + entry * 0x10
    uint32_t client_state;  // packet +0x20 + entry * 0x10
    uint32_t server_state;  // packet +0x24 + entry * 0x10
};

// Old monitor-channel packet type 0xFA2.  Its constructor writes only the
// PacketHeader; count and entries intentionally remain indeterminate until a
// remote monitor reply supplies them.  CMD198 sends only the first 0x18 bytes
// after it writes the request fields, exactly as the raw code does.
struct Packet_PvPChannelInfo {
    enum : uint16_t {
        kPacketType = 0x0FA2u,
        kObjectBytes = 0x17F8u,
        kRequestBytes = 0x18u,
        kEntryCount = (kObjectBytes - kRequestBytes) / sizeof(PvPChannelInfoEntry),
    };

    // IDA_EA: 0x822BD3C
    // IDA_NAME: _ZN21Packet_PvPChannelInfoC2Ev
    // DEMANGLED: Packet_PvPChannelInfo::Packet_PvPChannelInfo(void)
    Packet_PvPChannelInfo();

    PacketHeader header;            // +0x00
    uint32_t charac_no;             // +0x0A
    uint32_t uid;                   // +0x0E
    uint32_t school_no;             // +0x12
    uint8_t request_value;          // +0x16, raw CMD198 writes 9
    uint8_t channel_count;          // +0x17, raw C2/request leaves untouched
    PvPChannelInfoEntry entries[kEntryCount];  // +0x18
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0Au,
              "PacketHeader base ABI must stay packed at 10 bytes");
static_assert(sizeof(PvPChannelInfoEntry) == 0x10u,
              "PvP channel entry stride must stay 16 bytes");
static_assert(offsetof(Packet_PvPChannelInfo, charac_no) == 0x0Au &&
                  offsetof(Packet_PvPChannelInfo, uid) == 0x0Eu &&
                  offsetof(Packet_PvPChannelInfo, school_no) == 0x12u &&
                  offsetof(Packet_PvPChannelInfo, request_value) == 0x16u &&
                  offsetof(Packet_PvPChannelInfo, channel_count) == 0x17u &&
                  offsetof(Packet_PvPChannelInfo, entries) == 0x18u,
              "Packet_PvPChannelInfo observed offsets changed");
static_assert(Packet_PvPChannelInfo::kEntryCount == 382u,
              "Packet_PvPChannelInfo fixed entry capacity changed");
static_assert(sizeof(Packet_PvPChannelInfo) == 0x17F8u,
              "Packet_PvPChannelInfo old i386 object size changed");

// CMD 198 / PVP_CHANNEL_INFO.  The raw class selects only the state-0
// IPacketDispatcher base and has no concrete members of its own.
class Dispatcher_PvPChannelInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859686C
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfoC2Ev
    Dispatcher_PvPChannelInfo();

    // IDA_EA: 0x824194A
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfoD2Ev
    // IDA_EA: 0x824197A
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfoD0Ev
    virtual ~Dispatcher_PvPChannelInfo();

    // IDA_EA: 0x82178C0
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfo12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233CEA
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfo4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233CF4
    // IDA_NAME: _ZN25Dispatcher_PvPChannelInfo7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PvPChannelInfo) == 0x04u,
              "CMD198 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_CHANNEL_PACKET_DISPATCH_PVP_CHANNEL_INFO_DISPATCHER_H_
