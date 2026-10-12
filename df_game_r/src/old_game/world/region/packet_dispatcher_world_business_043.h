#ifndef OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_BUSINESS_043_H_
#define OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_BUSINESS_043_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// The original parameter has three integer members.  The 140828 CMD 15
// error-68 response consumes an additional u32 after response_detail; reuse
// the third member for the authoritative forbidden-move time instead of
// growing the internal dispatcher ABI.
struct stStartGameParam : ParamBase {
    stStartGameParam();
    virtual ~stStartGameParam();

    int result;
    int response_detail;
    uint32_t forbidden_move_left_seconds;
};

#if defined(__i386__)
static_assert(sizeof(stStartGameParam) == 0x10u,
              "stStartGameParam must stay ParamBase plus three ints");
#endif

// These A-owned explicit member specializations provide the original CMD 15
// dispatch ABI.  Declare all of them before the derived class below can
// instantiate the primary-template virtual member set.
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&);

// CMD 15's original class uses this exact specialized dispatcher base.  The
// generic template implementation is A-owned; the world-map path below uses
// CDataManager's recovered typed CWorldMapList subobject at +0x8780.
class DisPatcher_StartGame
    : public IPacketDispatcher<MSG_BASE,
                               stStartGameParam,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_StartGame();
    virtual ~DisPatcher_StartGame();

    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    bool excludeAddHackCount(CUser* user);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

// CMD 91's original object is the state-0 MSG_BASE/ParamBase dispatcher
// specialization.  `dispatch_sig` uses the recovered private_store::ISeller
// contract; MoveToGate is its verified vtable slot +0x0c.
class Dispatcher_MoveToGate
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_MoveToGate();
    virtual ~Dispatcher_MoveToGate();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};


#endif  // OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_BUSINESS_043_H_
