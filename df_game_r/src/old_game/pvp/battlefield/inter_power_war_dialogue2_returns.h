#ifndef OLDSRV_OLD_GAME_WORLD_INTER_POWER_WAR_DIALOGUE2_RETURNS_H_
#define OLDSRV_OLD_GAME_WORLD_INTER_POWER_WAR_DIALOGUE2_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Dialogue-2 PowerWar Inter return leaves.  Every original object is a
// base-only ISigDispatcher derivative; all state belongs to CPowerManager,
// CUser, the guild proxy, or GameWorld.
class Inter_ReplyPowerWarScore : public ISigDispatcher {
public:
    Inter_ReplyPowerWarScore();
    virtual ~Inter_ReplyPowerWarScore();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePowerWarProlong : public ISigDispatcher {
public:
    Inter_NoticePowerWarProlong();
    virtual ~Inter_NoticePowerWarProlong();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReplyJoinPower : public ISigDispatcher {
public:
    Inter_ReplyJoinPower();
    virtual ~Inter_ReplyJoinPower();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SecedePower : public ISigDispatcher {
public:
    Inter_SecedePower();
    virtual ~Inter_SecedePower();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePowerWarStartTime : public ISigDispatcher {
public:
    Inter_NoticePowerWarStartTime();
    virtual ~Inter_NoticePowerWarStartTime();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePowerWarEndTime : public ISigDispatcher {
public:
    Inter_NoticePowerWarEndTime();
    virtual ~Inter_NoticePowerWarEndTime();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePowerWarInfo : public ISigDispatcher {
public:
    Inter_NoticePowerWarInfo();
    virtual ~Inter_NoticePowerWarInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ReplyPowerWarScore) == 0x04u &&
                  sizeof(Inter_NoticePowerWarProlong) == 0x04u &&
                  sizeof(Inter_ReplyJoinPower) == 0x04u &&
                  sizeof(Inter_SecedePower) == 0x04u &&
                  sizeof(Inter_NoticePowerWarStartTime) == 0x04u &&
                  sizeof(Inter_NoticePowerWarEndTime) == 0x04u &&
                  sizeof(Inter_NoticePowerWarInfo) == 0x04u,
              "PowerWar Inter leaves must retain the old base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_INTER_POWER_WAR_DIALOGUE2_RETURNS_H_
