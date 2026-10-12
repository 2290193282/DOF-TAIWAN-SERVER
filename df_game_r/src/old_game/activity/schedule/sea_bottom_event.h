#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SEA_BOTTOM_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SEA_BOTTOM_EVENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

namespace sea_bottom_event {

// Town-ready lifecycle.  State is character/campaign scoped and has no daily
// 06:00 reset in the official Sea Bottom PVF; a new campaign_key is the only
// state reset boundary.
void BeginSession(CUser* user);
void RefreshSession(CUser* user);
void OnGlobalMinuteTick();
void ResetSession(CUser* user);
void FlushAndErase(CUser* user);

// Exact 140828 NOTI702 body:
// u32 current_depth, u32 count, repeat(count){u32 claimed_spec_depth}.
void SendSeaBottomInfo(CUser* user,
                       uint32_t current_depth,
                       uint32_t claimed_mask);

// Business ownership is complete only when strict official-PVF validation,
// inner-32 persistence, reserve/consume/commit recovery and mail projection
// are all linked.  Runtime campaign absence still fails closed.
bool HasNativeClosure();

}  // namespace tw_seabottom_native_0828

class Dispatcher_TheSeaBottom2000Use
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_TheSeaBottom2000Use();
    virtual ~Dispatcher_TheSeaBottom2000Use();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterSeaBottomEvent : public TaiwanInterfaceInterDispatcher {
public:
    InterSeaBottomEvent();
    virtual ~InterSeaBottomEvent();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_TheSeaBottom2000Use) == 0x04u,
              "CMD727 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterSeaBottomEvent) == 0x08u,
              "Taiwan inner-32 Inter owner must preserve the base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_TW_SEABOTTOM_NATIVE_0828_H_
