#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_GLASS_BOTTLE_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_GLASS_BOTTLE_EVENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

namespace glass_bottle_event {

struct ShootingStarWishState {
    uint8_t reward_index;
    uint8_t available;
};

struct ShootingStarSceneObject {
    uint32_t actor_key;
    uint8_t variant;
};

// Character lifecycle for the native-closed glass-bottle owner.  BeginSession
// belongs at the first state-3 town-ready boundary after inventory load.
void BeginSession(CUser* user);
void ResetSession(CUser* user);
void FlushAndErase(CUser* user);

// Exact 140828 serializers proven from the client readers.  The shooting-star
// and dimension functions are intentionally not called by this feature until
// their authoritative producers are restored.
void SendShootingStarState(CUser* user, uint8_t active);
void SendShootingStarWishList(CUser* user,
                              const ShootingStarWishState* states,
                              uint8_t count);
void SendShootingStarReceiveStar(CUser* user, uint8_t received_count);
void SendDimensionPocketInfo(CUser* user,
                             uint8_t state,
                             uint8_t proper_clear_count);
void SendGlassBottleEvent(CUser* user, uint16_t sent_count);

// Registration gates.  They make it impossible for shared wiring to confuse
// a proven packet shape with a complete business owner.
bool HasNativeShootingStarClosure();
bool HasNativeDimensionPocketClosure();
bool HasNativeGlassBottleClosure();

}  // namespace tw_event_0828

class Dispatcher_SendGlassBottleLetter
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SendGlassBottleLetter();
    virtual ~Dispatcher_SendGlassBottleLetter();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterGlassBottleEvent : public TaiwanInterfaceInterDispatcher {
public:
    InterGlassBottleEvent();
    virtual ~InterGlassBottleEvent();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SendGlassBottleLetter) == 0x04u,
              "CMD724 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterGlassBottleEvent) == 0x08u,
              "Taiwan inner-28 Inter owner must preserve the base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_TW_EVENT_720_721_724_0828_H_
