#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CLOSE_DISJOINT_STORE_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CLOSE_DISJOINT_STORE_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/stCloseDisjointStoreParam.
// IDA_EA: 0x823133C, 0x82474CE, 0x82474FE.
struct stCloseDisjointStoreParam : public ParamBase {
    // IDA_EA: 0x823133C
    // IDA_NAME: _ZN25stCloseDisjointStoreParamC2Ev
    // DEMANGLED: stCloseDisjointStoreParam::stCloseDisjointStoreParam(void)
    stCloseDisjointStoreParam();

    // IDA_EA: 0x82474CE / 0x82474FE
    // IDA_NAME: _ZN25stCloseDisjointStoreParamD2Ev / D0Ev
    // DEMANGLED: stCloseDisjointStoreParam::~stCloseDisjointStoreParam()
    virtual ~stCloseDisjointStoreParam();

    int error_code;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stCloseDisjointStoreParam) == 0x08u,
              "CMD246 parameter must retain its i386 ABI size");
#endif

// The complete generic dispatcher is emitted once by the A protocol source.
// This must be an extern instantiation, not six member-specialization
// declarations: the latter would leave the provider object without bodies.
extern template class IPacketDispatcher<MSG_BASE,
                                        stCloseDisjointStoreParam,
                                        static_cast<ch_state>(3)>;

// IDA unit: 13_database_dispatch/Dispatcher_CloseDisjointStore.
class Dispatcher_CloseDisjointStore
    : public IPacketDispatcher<MSG_BASE,
                               stCloseDisjointStoreParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596C24
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStoreC2Ev
    // DEMANGLED: Dispatcher_CloseDisjointStore::Dispatcher_CloseDisjointStore(void)
    Dispatcher_CloseDisjointStore();

    // IDA_EA: 0x8245946 / 0x8245976
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStoreD2Ev / D0Ev
    // DEMANGLED: Dispatcher_CloseDisjointStore::~Dispatcher_CloseDisjointStore()
    virtual ~Dispatcher_CloseDisjointStore();

    // IDA_EA: 0x81D02BE
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStore4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D0366
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStore7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D02C8
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStore11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81D0412
    // IDA_NAME: _ZN29Dispatcher_CloseDisjointStore4sendEP5CUserR9ParamBase
    // The IDA pseudocode propagates SendCmdErrorPacket's eax value, but this
    // occupies the generic IPacketDispatcher::send vtable slot, whose old
    // C++ declaration is void.  Return type is not encoded in the raw symbol;
    // retain the typed virtual ABI so the slot actually overrides the base.
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CloseDisjointStore) == 0x04u,
              "CMD246 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CLOSE_DISJOINT_STORE_H_
