#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SHOOTING_STAR_DIMENSION_POCKET_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SHOOTING_STAR_DIMENSION_POCKET_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

namespace shooting_star_dimension_pocket {

// Town-ready lifecycle.  BeginSession seeds the proper-clear high watermark
// without backfilling clears that predate this event session.
void BeginSession(CUser* user);
void RefreshSession(CUser* user);
void OnGlobalMinuteTick();
void ResetSession(CUser* user);
void FlushAndErase(CUser* user);

// Authoritative source hooks.  The clear hook belongs immediately after the
// canonical IncProperDungeonClearCount() call.  The reset hook belongs after
// the configured fatigue-time reset has cleared that canonical source.
void OnProperDungeonClear(CUser* user);
// Process-wide generation latch.  Call once from the authoritative daily
// fatigue reset before iterating online users, including when that set is
// empty; per-user reset remains responsible for advancing loaded sessions.
void OnProperClearGlobalReset();
void OnProperClearSerialReset(CUser* user);

// NOTI694 carries the transient visual batch count for an actually completed
// fragment grant, not a persisted total or purchase counter.  The canonical
// inventory insert owner calls this after success; zero is ignored and the
// caller saturates batches larger than the client's single u8 field.
void NotifyShootingStarReceived(CUser* user, uint8_t received_count);

// Exact 140828 NOTI serializers.
void SendShootingStarState(CUser* user, uint8_t active);
void SendShootingStarWishList(CUser* user,
                              uint16_t available_mask);
void SendShootingStarReceived(CUser* user, uint8_t received_count);
void SendDimensionPocketInfo(CUser* user,
                             uint8_t state,
                             uint8_t proper_clear_count);

// Describes implementation ownership.  Runtime PVF/campaign absence remains
// fail-closed and is not a reason to remove the registered packet owner.
bool HasNativeClosure();

}  // namespace tw_event_720_721_native_0828

class Dispatcher_ShootingStarBuyItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ShootingStarBuyItem();
    virtual ~Dispatcher_ShootingStarBuyItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DimensionPocketItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DimensionPocketItem();
    virtual ~Dispatcher_DimensionPocketItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterShootingStarDimensionPocket
    : public TaiwanInterfaceInterDispatcher {
public:
    InterShootingStarDimensionPocket();
    virtual ~InterShootingStarDimensionPocket();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ShootingStarBuyItem) == 0x04u,
              "CMD720 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_DimensionPocketItem) == 0x04u,
              "CMD721 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterShootingStarDimensionPocket) == 0x08u,
              "Taiwan inner-31 Inter owner must preserve base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_TW_EVENT_720_721_NATIVE_0828_H_
