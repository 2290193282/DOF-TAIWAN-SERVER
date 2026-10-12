#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_START_GAME_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_START_GAME_H_

#include "old_game/activity/schedule/advance_altar_dispatchers.h"

namespace advancealtar {

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_StartGame.
class Dispatcher_StartGame
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85956D0
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGameC2Ev
    // DEMANGLED: advancealtar::Dispatcher_StartGame::Dispatcher_StartGame(void)
    Dispatcher_StartGame();
    // IDA_EA: 0x814218A / 0x81421BA
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGameD2Ev /
    //           _ZN12advancealtar20Dispatcher_StartGameD0Ev
    // DEMANGLED: advancealtar::Dispatcher_StartGame::~Dispatcher_StartGame()
    virtual ~Dispatcher_StartGame();

    // IDA_EA: 0x813F7B0
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x813F8CE
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x813F92A
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x813F7DE
    // IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(ParamBase) == 0x04u,
              "CMD641 must retain the four-byte ParamBase response ABI");
static_assert(sizeof(Dispatcher_StartGame) == 0x04u,
              "CMD641 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_START_GAME_H_
