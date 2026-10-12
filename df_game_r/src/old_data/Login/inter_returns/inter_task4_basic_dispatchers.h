#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TASK4_BASIC_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TASK4_BASIC_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Base-only InterDispatcher leaves from the original C2 table.  They are kept
// here because each is an Inter return consumer, not a DBTask provider.
class Inter_GoblinPadSamePwdCheck : public ISigDispatcher {
public:
    Inter_GoblinPadSamePwdCheck();
    virtual ~Inter_GoblinPadSamePwdCheck();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LevelUpReward : public ISigDispatcher {
public:
    Inter_LevelUpReward();
    virtual ~Inter_LevelUpReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SendPeriodicMessage : public ISigDispatcher {
public:
    Inter_SendPeriodicMessage();
    virtual ~Inter_SendPeriodicMessage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SendTimeSync : public ISigDispatcher {
public:
    Inter_SendTimeSync();
    virtual ~Inter_SendTimeSync();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SendSecuReward : public ISigDispatcher {
public:
    Inter_SendSecuReward();
    virtual ~Inter_SendSecuReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadPuUserCheck : public ISigDispatcher {
public:
    Inter_LoadPuUserCheck();
    virtual ~Inter_LoadPuUserCheck();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GuildAttendanceInfo : public ISigDispatcher {
public:
    Inter_GuildAttendanceInfo();
    virtual ~Inter_GuildAttendanceInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_AchieveGuildAttendance : public ISigDispatcher {
public:
    Inter_AchieveGuildAttendance();
    virtual ~Inter_AchieveGuildAttendance();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_EmergencyPatchMessage : public ISigDispatcher {
public:
    Inter_EmergencyPatchMessage();
    virtual ~Inter_EmergencyPatchMessage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_UpdateServerSnapShot : public ISigDispatcher {
public:
    Inter_UpdateServerSnapShot();
    virtual ~Inter_UpdateServerSnapShot();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_UpdateMiniCraneSeed : public ISigDispatcher {
public:
    Inter_UpdateMiniCraneSeed();
    virtual ~Inter_UpdateMiniCraneSeed();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SelectAllowProxyUser : public ISigDispatcher {
public:
    Inter_SelectAllowProxyUser();
    virtual ~Inter_SelectAllowProxyUser();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_GoblinPadSamePwdCheck) == 0x04u,
              "Inter_GoblinPadSamePwdCheck must remain base-only");
static_assert(sizeof(Inter_LevelUpReward) == 0x04u,
              "Inter_LevelUpReward must remain base-only");
static_assert(sizeof(Inter_SendPeriodicMessage) == 0x04u,
              "Inter_SendPeriodicMessage must remain base-only");
static_assert(sizeof(Inter_SendTimeSync) == 0x04u,
              "Inter_SendTimeSync must remain base-only");
static_assert(sizeof(Inter_SendSecuReward) == 0x04u,
              "Inter_SendSecuReward must remain base-only");
static_assert(sizeof(Inter_LoadPuUserCheck) == 0x04u,
              "Inter_LoadPuUserCheck must remain base-only");
static_assert(sizeof(Inter_GuildAttendanceInfo) == 0x04u,
              "Inter_GuildAttendanceInfo must remain base-only");
static_assert(sizeof(Inter_AchieveGuildAttendance) == 0x04u,
              "Inter_AchieveGuildAttendance must remain base-only");
static_assert(sizeof(Inter_EmergencyPatchMessage) == 0x04u,
              "Inter_EmergencyPatchMessage must remain base-only");
static_assert(sizeof(Inter_UpdateServerSnapShot) == 0x04u,
              "Inter_UpdateServerSnapShot must remain base-only");
static_assert(sizeof(Inter_UpdateMiniCraneSeed) == 0x04u,
              "Inter_UpdateMiniCraneSeed must remain base-only");
static_assert(sizeof(Inter_SelectAllowProxyUser) == 0x04u,
              "Inter_SelectAllowProxyUser must remain base-only");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_TASK4_BASIC_DISPATCHERS_H_
