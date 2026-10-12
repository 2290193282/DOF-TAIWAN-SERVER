#ifndef OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_COMPOUND_CREATURE_H_
#define OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_COMPOUND_CREATURE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
struct SIG_CREATURE_ITEM;


// 0828 CMD226: consume one creature compounder and two distinct creature
// records, then publish the DB-assigned result egg through NOTI103.
class Dispatcher_CompoundCreature
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_CompoundCreature();
    virtual ~Dispatcher_CompoundCreature();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// Inter_InsertCreatureItemResult routes only marker-tagged signal136 replies
// here.  Returns true when the marker belongs to CMD226 (matched or stale).
bool HandleCompoundCreatureInsertResult(CUser* user,
                                        SIG_CREATURE_ITEM* signal);

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CompoundCreature) == 0x04u,
              "CMD226 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_APPEARANCE_PET_PACKET_DISPATCHER_COMPOUND_CREATURE_H_
