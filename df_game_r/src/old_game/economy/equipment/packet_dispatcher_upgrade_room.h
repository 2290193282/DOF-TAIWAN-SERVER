#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_UPGRADE_ROOM_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_UPGRADE_ROOM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// Exact 140828 Upgrade Room request owners.  The nine commands form one
// cohort: CMD583 is only registrable together with the scoped ProcUpgrade
// escrow adapter documented by upgrade_room_0828.h.
#define DECLARE_UPGRADE_ROOM_DISPATCHER(Type)                              \
class Type                                                               \
    : public IPacketDispatcher<MSG_BASE, ParamBase,                       \
                               static_cast<ch_state>(0)> {                \
public:                                                                  \
    Type();                                                              \
    virtual ~Type();                                                     \
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);            \
    virtual int read(PacketBuf& packet, MSG_BASE& message);              \
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param); \
}

DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_CreateUpgradeRoom);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_DestroyUpgradeRoom);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_JoinSpectatorUpgradeRoom);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_LeaveSpectatorUpgradeRoom);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_UpgradeRoomGiveItemInfo);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_UpgradeRoomPutUpItem);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_UpgradeRoomUpgradeStart);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_UpgradeRoomUpgradeCancel);
DECLARE_UPGRADE_ROOM_DISPATCHER(Dispatcher_UpgradeRoomMasterMessage);

#undef DECLARE_UPGRADE_ROOM_DISPATCHER

#if defined(__i386__)
#define ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Type, Command)                  \
static_assert(sizeof(Type) == 0x04u,                                      \
              "CMD" #Command " dispatcher must remain vptr-only on i386")

ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_CreateUpgradeRoom, 570);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_DestroyUpgradeRoom, 571);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_JoinSpectatorUpgradeRoom,
                                   572);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_LeaveSpectatorUpgradeRoom,
                                   573);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_UpgradeRoomGiveItemInfo,
                                   574);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_UpgradeRoomPutUpItem, 582);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_UpgradeRoomUpgradeStart,
                                   583);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_UpgradeRoomUpgradeCancel,
                                   584);
ASSERT_UPGRADE_ROOM_DISPATCHER_ABI(Dispatcher_UpgradeRoomMasterMessage,
                                   585);

#undef ASSERT_UPGRADE_ROOM_DISPATCHER_ABI
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_PACKET_DISPATCHER_UPGRADE_ROOM_0828_H_
