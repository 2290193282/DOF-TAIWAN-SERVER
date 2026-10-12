#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_RIGHT_OF_CHANGE_GROW_TYPE_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_RIGHT_OF_CHANGE_GROW_TYPE_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/MSG_RIGHT_OF_CHANGE_GROW_TYPE.
#pragma pack(push, 1)
struct MSG_RIGHT_OF_CHANGE_GROW_TYPE : public MSG_BASE {
    // IDA_EA: 0x826031A
    // IDA_NAME: _ZN29MSG_RIGHT_OF_CHANGE_GROW_TYPEC2Ev
    // DEMANGLED: MSG_RIGHT_OF_CHANGE_GROW_TYPE::MSG_RIGHT_OF_CHANGE_GROW_TYPE(void)
    MSG_RIGHT_OF_CHANGE_GROW_TYPE();

    uint16_t item_slot;       // +0x0d
    uint8_t first_grow_type;  // +0x0f
};
#pragma pack(pop)

// IDA unit: 99_misc_global/stRightOfChangeGrowType.
struct stRightOfChangeGrowType : public ParamBase {
    // IDA_EA: 0x82620E0
    // IDA_NAME: _ZN23stRightOfChangeGrowTypeC2Ev
    // DEMANGLED: stRightOfChangeGrowType::stRightOfChangeGrowType(void)
    stRightOfChangeGrowType();

    // IDA_EA: 0x82620FC / 0x826212C
    // IDA_NAME: _ZN23stRightOfChangeGrowTypeD2Ev /
    //           _ZN23stRightOfChangeGrowTypeD0Ev
    virtual ~stRightOfChangeGrowType();

    // The original C2 constructs only ParamBase; process() always writes the
    // result before send(), so this scalar is intentionally not initialized.
    int result;  // +0x04
};

// CMD503 is supplied as an unregistered concrete provider.  Its root slot is
// deliberately left untouched until the original 606-slot constructor and
// teardown transaction are restored as one unit.
class Dispatcher_UseRightOfChangeGrowType
    : public IPacketDispatcher<MSG_RIGHT_OF_CHANGE_GROW_TYPE,
                               stRightOfChangeGrowType,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8595820
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowTypeC2Ev
    Dispatcher_UseRightOfChangeGrowType();

    // IDA_EA: 0x8260BC6 / 0x8260BF6
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowTypeD2Ev /
    //           _ZN35Dispatcher_UseRightOfChangeGrowTypeD0Ev
    virtual ~Dispatcher_UseRightOfChangeGrowType();

    // IDA_EA: 0x825DBCC
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x825DC64
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x825DF24
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x825DD84
    // IDA_NAME: _ZN35Dispatcher_UseRightOfChangeGrowType11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(MSG_RIGHT_OF_CHANGE_GROW_TYPE) == 0x10u,
              "CMD503 request must retain the old 16-byte packet ABI");
static_assert(sizeof(stRightOfChangeGrowType) == 0x08u,
              "CMD503 result must retain the old eight-byte ABI");
static_assert(sizeof(Dispatcher_UseRightOfChangeGrowType) == 0x04u,
              "CMD503 handler must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_USE_RIGHT_OF_CHANGE_GROW_TYPE_DISPATCHER_H_
