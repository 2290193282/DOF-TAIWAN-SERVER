#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_DYE_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_DYE_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
struct Inven_Item;

// IDA 0x822E3B2 calls MSG_BASE::C2 then initializes only the two payload
// shorts.  Byte +0x0d remains an actual untouched wire-layout byte.
#pragma pack(push, 1)
struct MSG_USE_DYE : MSG_BASE {
    uint8_t raw_0d;
    int16_t dye_slot;
    int16_t avatar_slot;

    // IDA_EA: 0x822E3B2
    // IDA_NAME: _ZN11MSG_USE_DYEC2Ev
    // DEMANGLED: MSG_USE_DYE::MSG_USE_DYE(void)
    MSG_USE_DYE();
};
#pragma pack(pop)

static_assert(sizeof(MSG_USE_DYE) == 0x12u,
              "CMD499 request must retain its old 18-byte extent");
static_assert(offsetof(MSG_USE_DYE, dye_slot) == 0x0eu &&
                  offsetof(MSG_USE_DYE, avatar_slot) == 0x10u,
              "CMD499 request field offsets changed");

struct stUseDye : ParamBase {
    // IDA_EA: 0x8231DE4
    // IDA_NAME: _ZN8stUseDyeC2Ev
    // DEMANGLED: stUseDye::stUseDye(void)
    stUseDye();

    // IDA_EA: 0x8246576 / 0x82465A6
    // IDA_NAME: _ZN8stUseDyeD2Ev / _ZN8stUseDyeD0Ev
    virtual ~stUseDye();

    int result;
};

#if defined(__i386__)
static_assert(sizeof(stUseDye) == 0x08u,
              "stUseDye must retain its old eight-byte ABI");
static_assert(offsetof(stUseDye, result) == 0x04u,
              "stUseDye result must stay at IDA +0x04");
#endif

// CMD499 is restored as a source-complete derived handler only.  It is not
// entered in PacketDispatcher's root 606-slot registration until that owner
// restores the complete original registry lifecycle.
class Dispatcher_UseDye
    : public IPacketDispatcher<MSG_USE_DYE, stUseDye, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597DFE
    // IDA_NAME: _ZN17Dispatcher_UseDyeC2Ev
    // DEMANGLED: Dispatcher_UseDye::Dispatcher_UseDye(void)
    Dispatcher_UseDye();

    // IDA_EA: 0x8243C06 / 0x8243C36
    // IDA_NAME: _ZN17Dispatcher_UseDyeD2Ev / _ZN17Dispatcher_UseDyeD0Ev
    virtual ~Dispatcher_UseDye();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    int check_error(CUser* user, MSG_BASE& message);
    void notifyUpdatedInventoryItemInfo(CUser& user, short slot) const;
    bool getItem(const CUser& user,
                 int inventory_type,
                 short slot,
                 Inven_Item& item) const;
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseDye) == 0x04u,
              "CMD499 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_DYE_DISPATCHER_H_
