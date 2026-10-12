#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_ITEMBOX_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_ITEMBOX_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/concurrent/timer/timer_sig.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CParty;
class PvP_Room;

namespace ZombieItemBox {

enum { kGenerateTimerMessage = 48 };

// Imports Etc/GameMode/ZombieMode.etc.  A missing 0828 block disables this
// feature without failing the common 0627 data-manager startup.
bool LoadConfig();

// The runtime ownership is deliberately outside CParty so no restored party
// ABI offsets are consumed by this 0828-only feature.
void Reset(CParty* party);
void Start(CParty* party);
// Matching-authority integration point: called exactly once after the final
// five/six room is ACTIVE.  Hidden adapters remain fail-closed until then.
bool StartRoom(PvP_Room* room, uint32_t generation);
int OnGenerateTimer(CParty* party, uint32_t timer_key);
int Claim(CParty* party, uint32_t runtime_uid);

}  // namespace ZombieItemBox

// CLIENT_EA: CMD517 sender sub_C9E480 writes exactly one u32 after the
// inherited 13-byte C2S header.
#pragma pack(push, 1)
struct MSG_GET_ITEMBOX : MSG_BASE {
    uint32_t runtime_uid;  // +0x0d
};
#pragma pack(pop)

static_assert(sizeof(MSG_GET_ITEMBOX) == 0x11u,
              "CMD517 zombie item-box request must stay 17 bytes");

struct stGetItemBoxParam : ParamBase {
    stGetItemBoxParam();
    virtual ~stGetItemBoxParam() {}

    int error;
};

#if defined(__i386__)
static_assert(sizeof(stGetItemBoxParam) == 0x08u,
              "CMD517 parameter must retain its i386 ABI");
static_assert(offsetof(stGetItemBoxParam, error) == 0x04u,
              "CMD517 parameter error offset mismatch");
#endif

class DisPatcher_GetItemBox
    : public IPacketDispatcher<MSG_GET_ITEMBOX,
                               stGetItemBoxParam,
                               static_cast<ch_state>(5)> {
public:
    DisPatcher_GetItemBox();
    virtual ~DisPatcher_GetItemBox();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_GetItemBox) == 0x04u,
              "CMD517 dispatcher must retain its base-only i386 ABI");
#endif

// CLIENT_EA: CMD535 sender sub_5351B0 writes one fixed u16(0).  Its
// success handler is nullsub_6; NOTI474 separately broadcasts
// {u8 count, count * u32 party-seat}.
#pragma pack(push, 1)
struct MSG_SELECT_ZOMBIE : MSG_BASE {
    uint16_t reserved_zero;
};
#pragma pack(pop)

static_assert(sizeof(MSG_SELECT_ZOMBIE) == 0x0fu,
              "CMD535 select-zombie request must stay 15 bytes");

struct stSelectZombieParam : ParamBase {
    stSelectZombieParam();
    virtual ~stSelectZombieParam() {}

    int error;
};

#if defined(__i386__)
static_assert(sizeof(stSelectZombieParam) == 0x08u,
              "CMD535 parameter must retain its i386 ABI");
static_assert(offsetof(stSelectZombieParam, error) == 0x04u,
              "CMD535 parameter error offset mismatch");
#endif

class DisPatcher_SelectZombie
    : public IPacketDispatcher<MSG_SELECT_ZOMBIE,
                               stSelectZombieParam,
                               static_cast<ch_state>(5)> {
public:
    DisPatcher_SelectZombie();
    virtual ~DisPatcher_SelectZombie();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SelectZombie) == 0x04u,
              "CMD535 dispatcher must retain its base-only i386 ABI");
#endif

// TimerDispatcher slot 48 is the 0828 periodic zombie item-box generator.
class TimerZombieItemBoxGenerate : public TimerSig {
public:
    TimerZombieItemBoxGenerate();
    virtual ~TimerZombieItemBoxGenerate();

    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);
};

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_ITEMBOX_RUNTIME_H_
