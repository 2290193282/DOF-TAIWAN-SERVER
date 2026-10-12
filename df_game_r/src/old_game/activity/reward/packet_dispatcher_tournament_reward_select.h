#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The generic object owns the original typed stack frame.  These declarations
// prevent an accidental generic instantiation in a concrete handler TU.
template <>
IPacketDispatcher<MsgTournamentRewardSelect,
                  stTournamentRewardSelect,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MsgTournamentRewardSelect,
                  stTournamentRewardSelect,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::dispatch_sig(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet);
template <>
void IPacketDispatcher<MsgTournamentRewardSelect,
                       stTournamentRewardSelect,
                       static_cast<ch_state>(0)>::send(
    CUser* user,
    ParamBase& param);

// IDA unit: 11_event_runtime/Dispatcher_TournamentRewardSelect (CMD459).
// The object is vptr-only and remains absent from the frozen 606 table until
// every table entry can be activated as one transaction.
class Dispatcher_TournamentRewardSelect
    : public IPacketDispatcher<MsgTournamentRewardSelect,
                               stTournamentRewardSelect,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597B26
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelectC2Ev
    Dispatcher_TournamentRewardSelect();

    // IDA_EA: 0x8244356 / 0x8244386
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelectD2Ev /
    //           _ZN33Dispatcher_TournamentRewardSelectD0Ev
    virtual ~Dispatcher_TournamentRewardSelect();

    // IDA_EA: 0x81DFFDA
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E00F0
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E02B0
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E046C
    // IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_TournamentRewardSelect) == 0x04u,
              "CMD459 must remain the original vptr-only object");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_H_
