#ifndef OLDSRV_OLD_GAME_EVENT_BINGO_FATIGUE_PACKET_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_EVENT_BINGO_FATIGUE_PACKET_DISPATCHERS_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD 514 / BINGO_REWARD.
class Dispatcher_BingoReward
    : public IPacketDispatcher<MSG_BASE,
                               stBingoReward,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_BingoReward();
    virtual ~Dispatcher_BingoReward();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
};

// CMD 515 / BINGO_QUIZ.
class Dispatcher_BingoQuiz
    : public IPacketDispatcher<MSG_BINGO_QUIZ,
                               stBingoQuiz,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_BingoQuiz();
    virtual ~Dispatcher_BingoQuiz();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
};

// CMD 429 / FATIGUE_ATTENDANCE.
class Dispatcher_FatigueAttendance
    : public IPacketDispatcher<MSG_FATIGUE_ATTENDANCE,
                               stFatigueAttendance,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_FatigueAttendance();
    virtual ~Dispatcher_FatigueAttendance();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BingoReward) == 0x04u,
              "CMD514 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_BingoQuiz) == 0x04u,
              "CMD515 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_FatigueAttendance) == 0x04u,
              "CMD429 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_BINGO_FATIGUE_PACKET_DISPATCHERS_H_
