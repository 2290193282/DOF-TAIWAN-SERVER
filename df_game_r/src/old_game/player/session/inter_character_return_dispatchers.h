#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_CHARACTER_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_CHARACTER_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class SkillSlot;
struct _Mastered_skill;

class Inter_LoadMileage : public ISigDispatcher {
public:
    Inter_LoadMileage();
    virtual ~Inter_LoadMileage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReqQueryCharacInfoMailbox : public ISigDispatcher {
public:
    Inter_ReqQueryCharacInfoMailbox();
    virtual ~Inter_ReqQueryCharacInfoMailbox();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Send_Charac_no_from_monitor : public ISigDispatcher {
public:
    Inter_Send_Charac_no_from_monitor();
    virtual ~Inter_Send_Charac_no_from_monitor();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_TodayGuildMember : public ISigDispatcher {
public:
    Inter_TodayGuildMember();
    virtual ~Inter_TodayGuildMember();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ApproveJoinGuild : public ISigDispatcher {
public:
    Inter_ApproveJoinGuild();
    virtual ~Inter_ApproveJoinGuild();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadKillMonsterInfo : public ISigDispatcher {
public:
    Inter_LoadKillMonsterInfo();
    virtual ~Inter_LoadKillMonsterInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_MonitorReplyCharacInfo : public ISigDispatcher {
public:
    Inter_MonitorReplyCharacInfo();
    virtual ~Inter_MonitorReplyCharacInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadCharacTypeHotKey : public ISigDispatcher {
public:
    Inter_LoadCharacTypeHotKey();
    virtual ~Inter_LoadCharacTypeHotKey();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_GetPremiumSelectedCubeInfo : public ISigDispatcher {
public:
    Inter_GetPremiumSelectedCubeInfo();
    virtual ~Inter_GetPremiumSelectedCubeInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_RequestCharacSkillInfo : public ISigDispatcher {
public:
    Inter_RequestCharacSkillInfo();
    virtual ~Inter_RequestCharacSkillInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84E7820
    // IDA_NAME: _ZN28Inter_RequestCharacSkillInfo21_PutSkillInfoToCharacEcPK15_Mastered_skillP9SkillSlot
    // DEMANGLED: Inter_RequestCharacSkillInfo::_PutSkillInfoToCharac(char,_Mastered_skill const*,SkillSlot *)
    void* _PutSkillInfoToCharac(char skill_tree_index,
                                const _Mastered_skill* source,
                                SkillSlot* skill_slot);
};

class Inter_RequestTagCharacInfo : public ISigDispatcher {
public:
    Inter_RequestTagCharacInfo();
    virtual ~Inter_RequestTagCharacInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_HadesAction : public ISigDispatcher {
public:
    Inter_HadesAction();
    virtual ~Inter_HadesAction();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_SelectCharacMid : public ISigDispatcher {
public:
    Inter_SelectCharacMid();
    virtual ~Inter_SelectCharacMid();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadMileage) == 0x04u,
              "Inter_LoadMileage must remain base-only");
static_assert(sizeof(Inter_ReqQueryCharacInfoMailbox) == 0x04u,
              "Inter_ReqQueryCharacInfoMailbox must remain base-only");
static_assert(sizeof(Inter_Send_Charac_no_from_monitor) == 0x04u,
              "Inter_Send_Charac_no_from_monitor must remain base-only");
static_assert(sizeof(Inter_TodayGuildMember) == 0x04u,
              "Inter_TodayGuildMember must remain base-only");
static_assert(sizeof(Inter_ApproveJoinGuild) == 0x04u,
              "Inter_ApproveJoinGuild must remain base-only");
static_assert(sizeof(Inter_LoadKillMonsterInfo) == 0x04u,
              "Inter_LoadKillMonsterInfo must remain base-only");
static_assert(sizeof(Inter_MonitorReplyCharacInfo) == 0x04u,
              "Inter_MonitorReplyCharacInfo must remain base-only");
static_assert(sizeof(Inter_LoadCharacTypeHotKey) == 0x04u,
              "Inter_LoadCharacTypeHotKey must remain base-only");
static_assert(sizeof(Inter_GetPremiumSelectedCubeInfo) == 0x04u,
              "Inter_GetPremiumSelectedCubeInfo must remain base-only");
static_assert(sizeof(Inter_RequestCharacSkillInfo) == 0x04u,
              "Inter_RequestCharacSkillInfo must remain base-only");
static_assert(sizeof(Inter_RequestTagCharacInfo) == 0x04u,
              "Inter_RequestTagCharacInfo must remain base-only");
static_assert(sizeof(Inter_HadesAction) == 0x04u,
              "Inter_HadesAction must remain base-only");
static_assert(sizeof(Inter_SelectCharacMid) == 0x04u,
              "Inter_SelectCharacMid must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_CHARACTER_RETURN_DISPATCHERS_H_
