#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_TIMER_CALLBACKS_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_TIMER_CALLBACKS_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA_EA: 0x8636555
// IDA_NAME: _Z16PvPDieUserReviveiiji
// DEMANGLED: PvPDieUserRevive(int,int,unsigned int,int)
int PvPDieUserRevive(int pvp_index,
                     int timer_key,
                     unsigned int payload,
                     int seat);

class TimerPvpRankRes : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPVPWalkOutLackUser : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive0 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive1 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive2 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive3 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive4 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive5 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive6 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerPvPDieUserRevive7 : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerCheckPvPRoomUser : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

class TimerCheckRecvPvpEndResult : public TimerSig {
public:
    virtual int dispatch_sig(int pvp_index, int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(TimerPvpRankRes) == 4u,
              "TimerPvpRankRes must remain a TimerSig-only object");
static_assert(sizeof(TimerPVPWalkOutLackUser) == 4u,
              "TimerPVPWalkOutLackUser must remain a TimerSig-only object");
static_assert(sizeof(TimerPvPDieUserRevive0) == 4u,
              "TimerPvPDieUserRevive0 must remain a TimerSig-only object");
static_assert(sizeof(TimerPvPDieUserRevive7) == 4u,
              "TimerPvPDieUserRevive7 must remain a TimerSig-only object");
static_assert(sizeof(TimerCheckPvPRoomUser) == 4u,
              "TimerCheckPvPRoomUser must remain a TimerSig-only object");
static_assert(sizeof(TimerCheckRecvPvpEndResult) == 4u,
              "TimerCheckRecvPvpEndResult must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_TIMER_CALLBACKS_RUNTIME_H_
