#ifndef OLDSRV_SECURITY_UDP_CHARACTERISTIC_DISPATCHER_H_
#define OLDSRV_SECURITY_UDP_CHARACTERISTIC_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 212 / UDP_CHARACTERISTIC.  This is an exact concrete state-0 packet
// dispatcher provider, but remains deliberately unregistered until the old
// PacketDispatcher's complete 606-slot construction transaction is restored.
class DisPatcher_UdpCharacteristic
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596968
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristicC2Ev
    // DEMANGLED: DisPatcher_UdpCharacteristic::DisPatcher_UdpCharacteristic(void)
    DisPatcher_UdpCharacteristic();

    // IDA_EA: 0x8241728
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristicD2Ev
    // DEMANGLED: DisPatcher_UdpCharacteristic::~DisPatcher_UdpCharacteristic()
    // IDA_EA: 0x8241758
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristicD0Ev
    // DEMANGLED: DisPatcher_UdpCharacteristic::~DisPatcher_UdpCharacteristic()
    virtual ~DisPatcher_UdpCharacteristic();

    // IDA_EA: 0x821ABE0
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_UdpCharacteristic::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x821AFBC
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic11pvpPingStatEP5CUserRtS2_
    // DEMANGLED: DisPatcher_UdpCharacteristic::pvpPingStat(CUser *,unsigned short &,unsigned short &)
    int pvpPingStat(CUser* user,
                    unsigned short& first,
                    unsigned short& second);

    // IDA_EA: 0x821B02A
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic15fairPvpPingStatEP5CUserRtS2_
    // DEMANGLED: DisPatcher_UdpCharacteristic::fairPvpPingStat(CUser *,unsigned short &,unsigned short &)
    int fairPvpPingStat(CUser* user,
                        unsigned short& first,
                        unsigned short& second);

    // IDA_EA: 0x821B098
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic13partyPingStatEP5CUserRtS2_
    // DEMANGLED: DisPatcher_UdpCharacteristic::partyPingStat(CUser *,unsigned short &,unsigned short &)
    int partyPingStat(CUser* user,
                      unsigned short& first,
                      unsigned short& second);

    // IDA_EA: 0x8233D62
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_UdpCharacteristic::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233D6C
    // IDA_NAME: _ZN28DisPatcher_UdpCharacteristic7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_UdpCharacteristic::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_SECURITY_UDP_CHARACTERISTIC_DISPATCHER_H_
