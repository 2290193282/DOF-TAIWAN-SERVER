#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_COLLECT_BOX_DISPATCHER_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_COLLECT_BOX_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace collect_box_db {
struct Message;
}

namespace collect_box {
void BeginSession(CUser* user);
void ResetSession(CUser* user);
void ApplyDatabaseResult(CUser* user,
                         const collect_box_db::Message& response);
}


class Dispatcher_SelectCollectBox
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SelectCollectBox();
    virtual ~Dispatcher_SelectCollectBox();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_AddCollectBoxItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_AddCollectBoxItem();
    virtual ~Dispatcher_AddCollectBoxItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RemoveCollectBoxItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RemoveCollectBoxItem();
    virtual ~Dispatcher_RemoveCollectBoxItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ExtendCollectBoxExpiryDate
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ExtendCollectBoxExpiryDate();
    virtual ~Dispatcher_ExtendCollectBoxExpiryDate();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SelectCollectBox) == 0x04u,
              "CMD690 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_AddCollectBoxItem) == 0x04u,
              "CMD691 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_RemoveCollectBoxItem) == 0x04u,
              "CMD692 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_ExtendCollectBoxExpiryDate) == 0x04u,
              "CMD693 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_COLLECT_BOX_DISPATCHER_H_
