#ifndef OLDSRV_PROTOCOL_CLIENT_SPEC_STATISTIC_DISPATCHER_H_
#define OLDSRV_PROTOCOL_CLIENT_SPEC_STATISTIC_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/HWSpec.  The three bytes after raw_00 are ABI
// padding: the old constructor writes +0, +4 and +8 only.
struct HWSpec {
    uint8_t raw_00;
    uint32_t raw_04;
    uint32_t raw_08;

    // IDA_EA: 0x822B890
    // IDA_NAME: _ZN6HWSpecC2Ev
    // DEMANGLED: HWSpec::HWSpec(void)
    HWSpec();
};

static_assert(sizeof(HWSpec) == 0x0cu &&
                  offsetof(HWSpec, raw_00) == 0x00u &&
                  offsetof(HWSpec, raw_04) == 0x04u &&
                  offsetof(HWSpec, raw_08) == 0x08u,
              "CMD174 HWSpec must retain the old 12-byte layout");

// IDA unit: 99_misc_global/Client_Spec.  Its four front bytes are raw
// storage: only +0/+1 are read by CMD174; +2/+3 are copied without a
// reconstructed semantic meaning.
struct Client_Spec {
    uint8_t raw_00;
    uint8_t spec_count;
    uint8_t raw_02[2];
    HWSpec specs[255];

    // IDA_EA: 0x822B8B0
    // IDA_NAME: _ZN11Client_SpecC2Ev
    // DEMANGLED: Client_Spec::Client_Spec(void)
    Client_Spec();
};

static_assert(sizeof(Client_Spec) == 0xbf8u &&
                  offsetof(Client_Spec, raw_00) == 0x00u &&
                  offsetof(Client_Spec, spec_count) == 0x01u &&
                  offsetof(Client_Spec, raw_02) == 0x02u &&
                  offsetof(Client_Spec, specs) == 0x04u,
              "CMD174 Client_Spec must retain the old 0xbf8-byte layout");

// The C2S message is deliberately packed around Client_Spec.  The old
// constructor invokes MSG_BASE at +0 then Client_Spec at +0x0e, so the latter
// is intentionally unaligned even though each HWSpec has dwords at +4/+8.
#pragma pack(push, 1)
struct MSG_CLIENT_SPEC_STATISTIC : MSG_BASE {
    uint8_t raw_0d;
    Client_Spec client_spec;

    // IDA_EA: 0x82556A0
    // IDA_NAME: _ZN25MSG_CLIENT_SPEC_STATISTICC2Ev
    // DEMANGLED: MSG_CLIENT_SPEC_STATISTIC::MSG_CLIENT_SPEC_STATISTIC(void)
    MSG_CLIENT_SPEC_STATISTIC();
};
#pragma pack(pop)

static_assert(sizeof(MSG_CLIENT_SPEC_STATISTIC) == 0x0c06u,
              "CMD174 input message must retain its old 0xc06-byte extent");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_CLIENT_SPEC_STATISTIC, raw_0d) == 0x0du &&
                  offsetof(MSG_CLIENT_SPEC_STATISTIC, client_spec) == 0x0eu,
              "CMD174 input message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The original template stack object is a packed ParamBase plus one byte.
// Packing is intentional: the i386 raw function reserves five bytes, not an
// eight-byte naturally aligned derived C++ object.
#pragma pack(push, 1)
struct stClientSpecStatisticParam : ParamBase {
    uint8_t raw_04;

    // IDA_EA: 0x8231218
    // IDA_NAME: _ZN26stClientSpecStatisticParamC2Ev
    // DEMANGLED: stClientSpecStatisticParam::stClientSpecStatisticParam(void)
    stClientSpecStatisticParam();

    // IDA_EA: 0x824773E
    // IDA_NAME: _ZN26stClientSpecStatisticParamD2Ev
    // DEMANGLED: stClientSpecStatisticParam::~stClientSpecStatisticParam()
    // IDA_EA: 0x824776E
    // IDA_NAME: _ZN26stClientSpecStatisticParamD0Ev
    // DEMANGLED: stClientSpecStatisticParam::~stClientSpecStatisticParam()
    virtual ~stClientSpecStatisticParam();
};
#pragma pack(pop)

#if defined(__i386__)
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(stClientSpecStatisticParam) == 0x05u &&
                  offsetof(stClientSpecStatisticParam, raw_04) == 0x04u,
              "CMD174 parameter must retain the old five-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

// The statistic-server packet has a maximum allocation of 0xc06 bytes, but
// CMD174 overwrites header.size with 18 + 12 * raw_0b before SendPacket().
// raw_0a and raw_0c are intentionally not initialized by this constructor.
#pragma pack(push, 1)
struct Packet_Client_Spec_Statistic {
    PacketHeader header;
    uint8_t raw_0a;
    // Volatile keeps the old one-byte constructor store at +0x0b from being
    // widened into the deliberately unwritten neighboring +0x0a byte.
    volatile uint8_t raw_0b;
    uint8_t raw_0c[6];
    HWSpec specs[255];

    // IDA_EA: 0x822BC18
    // IDA_NAME: _ZN28Packet_Client_Spec_StatisticC2Ev
    // DEMANGLED: Packet_Client_Spec_Statistic::Packet_Client_Spec_Statistic(void)
    Packet_Client_Spec_Statistic();
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0au,
              "CMD174 statistic packet requires the old 10-byte PacketHeader");
static_assert(sizeof(Packet_Client_Spec_Statistic) == 0x0c06u &&
                  offsetof(Packet_Client_Spec_Statistic, header) == 0x00u &&
                  offsetof(Packet_Client_Spec_Statistic, raw_0a) == 0x0au &&
                  offsetof(Packet_Client_Spec_Statistic, raw_0b) == 0x0bu &&
                  offsetof(Packet_Client_Spec_Statistic, raw_0c) == 0x0cu &&
                  offsetof(Packet_Client_Spec_Statistic, specs) == 0x12u,
              "CMD174 statistic packet offsets changed");

// CMD 174 / CLIENT_SPEC_STATISTIC. This typed state-2 provider is deliberately
// unregistered until PacketDispatcher's complete original 606-slot root,
// ingress, validation and return chains are restored atomically.
class Dispatcher_Client_Spec_Statistic
    : public IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,
                               stClientSpecStatisticParam,
                               static_cast<ch_state>(2)> {
public:
    // IDA_EA: 0x8596594
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticC2Ev
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::Dispatcher_Client_Spec_Statistic(void)
    Dispatcher_Client_Spec_Statistic();

    // IDA_EA: 0x8245CA0
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticD2Ev
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::~Dispatcher_Client_Spec_Statistic()
    // IDA_EA: 0x8245CD0
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticD0Ev
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::~Dispatcher_Client_Spec_Statistic()
    virtual ~Dispatcher_Client_Spec_Statistic();

    // IDA_EA: 0x81CE5FE
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CE6D4
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CE75A
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81CE794
    // IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Client_Spec_Statistic::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Client_Spec_Statistic) == 0x04u,
              "CMD174 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_PROTOCOL_CLIENT_SPEC_STATISTIC_DISPATCHER_H_
