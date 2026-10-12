#ifndef OLDSRV_PROTOCOL_ASSERT_MANAGER_DISPATCHER_H_
#define OLDSRV_PROTOCOL_ASSERT_MANAGER_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// The fixed-size statistic body emitted by CMD 219. Its only original member
// is the header constructor below; the dispatcher fills the remaining raw
// fields directly and sends all 0x214 bytes through the statistic proxy.
#pragma pack(push, 1)
struct Packet_Assert_Manager_Info {
    PacketHeader header;
    int raw_0a;
    char raw_0e[0x100];
    uint16_t word_10e;
    int raw_110;
    char raw_114[0x100];

    // IDA_EA: 0x822BD18
    // IDA_NAME: _ZN26Packet_Assert_Manager_InfoC2Ev
    // DEMANGLED: Packet_Assert_Manager_Info::Packet_Assert_Manager_Info(void)
    Packet_Assert_Manager_Info();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Assert_Manager_Info) == 0x214u,
              "CMD219 assert-manager statistic packet must stay 0x214 bytes");
static_assert(offsetof(Packet_Assert_Manager_Info, raw_0a) == 0x0Au &&
                  offsetof(Packet_Assert_Manager_Info, raw_0e) == 0x0Eu &&
                  offsetof(Packet_Assert_Manager_Info, word_10e) == 0x10Eu &&
                  offsetof(Packet_Assert_Manager_Info, raw_110) == 0x110u &&
                  offsetof(Packet_Assert_Manager_Info, raw_114) == 0x114u,
              "CMD219 assert-manager statistic packet offsets changed");

// CMD 219 / ASSERT_INFO. This exact state-0 handler remains unregistered
// until the complete original 606-slot PacketDispatcher transaction is
// available; defining this class alone must not activate a partial route.
class Dispatcher_AssertManager
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596A80
    // IDA_NAME: _ZN24Dispatcher_AssertManagerC2Ev
    // DEMANGLED: Dispatcher_AssertManager::Dispatcher_AssertManager(void)
    Dispatcher_AssertManager();

    // IDA_EA: 0x8241380
    // IDA_NAME: _ZN24Dispatcher_AssertManagerD2Ev
    // DEMANGLED: Dispatcher_AssertManager::~Dispatcher_AssertManager()
    // IDA_EA: 0x82413B0
    // IDA_NAME: _ZN24Dispatcher_AssertManagerD0Ev
    // DEMANGLED: Dispatcher_AssertManager::~Dispatcher_AssertManager()
    virtual ~Dispatcher_AssertManager();

    // IDA_EA: 0x821E1F4
    // IDA_NAME: _ZN24Dispatcher_AssertManager12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_AssertManager::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233E3E
    // IDA_NAME: _ZN24Dispatcher_AssertManager4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AssertManager::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233E48
    // IDA_NAME: _ZN24Dispatcher_AssertManager7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AssertManager::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_ASSERT_MANAGER_DISPATCHER_H_
