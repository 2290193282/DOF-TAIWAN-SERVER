#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_MONSTER_MOVE_SYSTEM_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_MONSTER_MOVE_SYSTEM_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CParty;

namespace monster_move_system {

// The 0828 owner is intentionally sidecar-only: the original CParty/MapInfo
// ABI has no room for the client-script movement cursor.
void ResetParty(CParty* party);

// NOTI476 is also the client-side map-move entity constructor.  The first
// action-5 record for a room carries the monster identity, level, world
// position and team.  CLIENT_IDA_EVIDENCE: sub_4F10B0 passes the byte at
// record+0x20 to sub_9CCE80, whose vtable+0x270 setter stores the team value;
// it is not an HP percentage.  Golden-Commerce carts use player/friendly
// team 0 while ordinary enemy monsters use team 100.
bool SendBootstrap(CParty* party,
                   uint16_t entity_unique_id,
                   uint32_t monster_id,
                   uint16_t monster_level,
                   uint32_t position_x,
                   uint32_t position_y,
                   uint32_t position_z,
                   uint8_t team);

}  // namespace monster_move_system

// CLIENT_EA:
//   sub_4F0CF0 @0x004F0CF0 (scripted action 5)
//   sub_4F0E60 @0x004F0E60 (one-cell actions 0..3)
// Both finalize exactly
//   {u16 entity_unique_id,u8 grid_x,u8 grid_y,
//    u32 value_0,u32 value_1,u32 value_2,u8 action}.
class Dispatcher_MonsterMoveSystem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_MonsterMoveSystem();
    virtual ~Dispatcher_MonsterMoveSystem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MonsterMoveSystem) == 0x04u,
              "CMD541 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_MONSTER_MOVE_SYSTEM_0828_H_
