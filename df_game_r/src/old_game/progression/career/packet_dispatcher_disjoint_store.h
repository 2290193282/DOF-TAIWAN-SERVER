#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_STORE_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_STORE_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA packet extent/offsets from Dispatcher_CreateDisjointStore::read
// @0x81CF750.  MSG_BASE remains its 13-byte packed outer prefix; the native
// i386 alignment before title_length is part of the original message ABI.
struct MSG_CREATE_DISJOINT_STORE : public MSG_BASE {
    char title[256];             // +0x0d
    uint32_t title_length;       // +0x110
    int32_t cost;                // +0x114
    uint16_t pos_x;              // +0x118
    uint16_t pos_y;              // +0x11a
};

// IDA packet extent/offset from Dispatcher_EnterDisjointStore::read
// @0x81D0082.  The single native alignment byte at +0x0d is compiler ABI
// padding, not an invented protocol field.
struct MSG_ENTER_DISJOINT_STORE : public MSG_BASE {
    uint16_t owner_unique_id;  // +0x0e
};

#if defined(__i386__)
static_assert(offsetof(MSG_CREATE_DISJOINT_STORE, title) == 0x0du &&
                  offsetof(MSG_CREATE_DISJOINT_STORE, title_length) == 0x110u &&
                  offsetof(MSG_CREATE_DISJOINT_STORE, cost) == 0x114u &&
                  offsetof(MSG_CREATE_DISJOINT_STORE, pos_x) == 0x118u &&
                  offsetof(MSG_CREATE_DISJOINT_STORE, pos_y) == 0x11au &&
                  sizeof(MSG_CREATE_DISJOINT_STORE) == 0x11cu,
              "CMD238 request must retain the IDA i386 layout");
static_assert(offsetof(MSG_ENTER_DISJOINT_STORE, owner_unique_id) == 0x0eu &&
                  sizeof(MSG_ENTER_DISJOINT_STORE) == 0x10u,
              "CMD245 request must retain the IDA i386 layout");
#endif

struct stCreateDisjointStoreParam : public ParamBase {
    // IDA_EA: 0x823127E
    // IDA_NAME: _ZN26stCreateDisjointStoreParamC2Ev
    stCreateDisjointStoreParam();

    // IDA_EA: 0x8247654 / 0x8247684
    // IDA_NAME: _ZN26stCreateDisjointStoreParamD2Ev / D0Ev
    virtual ~stCreateDisjointStoreParam();

    int error_code;  // +0x04
};

struct stEnterDisjointStoreParam : public ParamBase {
    // IDA_EA: 0x8231316
    // IDA_NAME: _ZN25stEnterDisjointStoreParamC2Ev
    stEnterDisjointStoreParam();

    // IDA_EA: 0x824751C / 0x824754C
    // IDA_NAME: _ZN25stEnterDisjointStoreParamD2Ev / D0Ev
    virtual ~stEnterDisjointStoreParam();

    int error_code;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stCreateDisjointStoreParam) == 0x08u &&
                  sizeof(stEnterDisjointStoreParam) == 0x08u,
              "disjoint-store parameters must retain the i386 ABI");
#endif

extern template class IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,
                                        stCreateDisjointStoreParam,
                                        static_cast<ch_state>(3)>;
extern template class IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,
                                        stEnterDisjointStoreParam,
                                        static_cast<ch_state>(3)>;

// IDA unit: 13_database_dispatch/Dispatcher_CreateDisjointStore.
class Dispatcher_CreateDisjointStore
    : public IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,
                               stCreateDisjointStoreParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_CreateDisjointStore();
    virtual ~Dispatcher_CreateDisjointStore();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_EnterDisjointStore.
class Dispatcher_EnterDisjointStore
    : public IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,
                               stEnterDisjointStoreParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_EnterDisjointStore();
    virtual ~Dispatcher_EnterDisjointStore();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CreateDisjointStore) == 0x04u &&
                  sizeof(Dispatcher_EnterDisjointStore) == 0x04u,
              "disjoint-store dispatchers must retain base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_STORE_H_
