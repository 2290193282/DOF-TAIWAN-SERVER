#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UNSEAL_RANDOM_OPTION_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UNSEAL_RANDOM_OPTION_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
struct Inven_Item;

// PacketDispatcher slot 401.  The old service deliberately bypasses the
// generic MSG/PARAM transaction template: dispatch_sig consumes the two-slot
// request and performs the complete inventory/PVF transaction itself.
class Dispatcher_UnsealRandomOption
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859771A
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOptionC2Ev
    // DEMANGLED: Dispatcher_UnsealRandomOption::Dispatcher_UnsealRandomOption(void)
    Dispatcher_UnsealRandomOption();

    // IDA_EA: 0x824009C
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOptionD2Ev
    // DEMANGLED: Dispatcher_UnsealRandomOption::~Dispatcher_UnsealRandomOption()
    virtual ~Dispatcher_UnsealRandomOption();

    // IDA_EA: 0x8228E5E
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOption12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_UnsealRandomOption::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8229644
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOption25_check_unseal_scroll_itemEP5CUserPK10Inven_Itemt
    // DEMANGLED: Dispatcher_UnsealRandomOption::_check_unseal_scroll_item(CUser *,Inven_Item const*,unsigned short)
    int _check_unseal_scroll_item(CUser* user,
                                  const Inven_Item* target,
                                  uint16_t scroll_slot);

    // IDA_EA: 0x823432A
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOption4readER9PacketBufR8MSG_BASE
    // IDA_EA: 0x8234334
    // IDA_NAME: _ZN29Dispatcher_UnsealRandomOption7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UnsealRandomOption) == 0x04u,
              "CMD401 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UNSEAL_RANDOM_OPTION_H_
