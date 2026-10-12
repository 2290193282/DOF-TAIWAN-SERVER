#ifndef OLDSRV_OLD_GAME_PROGRESSION_CAREER_PACKET_DISPATCHER_MONSTERCARD_BIND_H_
#define OLDSRV_OLD_GAME_PROGRESSION_CAREER_PACKET_DISPATCHER_MONSTERCARD_BIND_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// 140828 CMD411.  The native sender writes one catalyst slot and then writes
// two card slots from a two-entry loop; the matching response reader consumes
// the same three slots before its counted reward records.
class Dispatcher_MonstercardBind
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_MonstercardBind();
    virtual ~Dispatcher_MonstercardBind();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MonstercardBind) == 0x04u,
              "CMD411 native dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PROGRESSION_CAREER_PACKET_DISPATCHER_MONSTERCARD_BIND_0828_H_
