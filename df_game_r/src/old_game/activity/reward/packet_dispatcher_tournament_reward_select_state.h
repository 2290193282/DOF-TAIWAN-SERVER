#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_STATE_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_STATE_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The generic specialization belongs to the protocol domain.  The concrete
// event handler stays separate from the frozen root constructor table.
template <>
IPacketDispatcher<MSG_BASE,
                  stTournamentRewardSelectState,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stTournamentRewardSelectState,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::dispatch_sig(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet);
template <>
void IPacketDispatcher<MSG_BASE,
                       stTournamentRewardSelectState,
                       static_cast<ch_state>(0)>::send(
    CUser* user,
    ParamBase& param);

// IDA unit: 11_event_runtime/Dispatcher_TournamentRewardSelectState.
struct Dispatcher_TournamentRewardSelectState
    : public IPacketDispatcher<MSG_BASE,
                               stTournamentRewardSelectState,
                               static_cast<ch_state>(0)> {
    // IDA_EA: 0x8597B0A
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectStateC2Ev
    // DEMANGLED: Dispatcher_TournamentRewardSelectState::Dispatcher_TournamentRewardSelectState(void)
    Dispatcher_TournamentRewardSelectState();

    // IDA_EA: 0x82443A4 / 0x82443D4
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectStateD2Ev / D0Ev
    // DEMANGLED: Dispatcher_TournamentRewardSelectState::~Dispatcher_TournamentRewardSelectState()
    virtual ~Dispatcher_TournamentRewardSelectState();

    // IDA_EA: 0x81DFC98
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DFCA2
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DFD74
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81DFF32
    // IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_TournamentRewardSelectState) == 0x04u,
              "Dispatcher_TournamentRewardSelectState must retain its i386 ABI size");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_TOURNAMENT_REWARD_SELECT_STATE_H_
