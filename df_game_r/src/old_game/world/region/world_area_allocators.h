#ifndef OLDSRV_OLD_GAME_WORLD_WORLD_AREA_ALLOCATORS_H_
#define OLDSRV_OLD_GAME_WORLD_WORLD_AREA_ALLOCATORS_H_

#include <map>
#include <stack>
#include <stdint.h>

class CUser;

// IDA unit: CBelong.  The base holds exactly the virtual recipient predicate
// consumed by GameWorld::send_group; individual groups provide that predicate.
class CBelong {
public:
    // IDA_EA: 0x84F07C2
    // IDA_NAME: _ZN7CBelongC2Ev
    // DEMANGLED: CBelong::CBelong(void)
    CBelong();

    virtual bool IsRight(CUser* user) = 0;
};

// IDA unit: 06_userinfo_stat_inventory/CUserTeen. This CBelong predicate is
// constructed by Inter_BroadcastMsg for the minimum-age recipient group.
class CUserTeen : public CBelong {
public:
    // IDA_EA: 0x84F07D0
    // IDA_NAME: _ZN9CUserTeenC2Ev
    // DEMANGLED: CUserTeen::CUserTeen(void)
    CUserTeen();

    // IDA_EA: 0x84F078C
    // IDA_NAME: _ZN9CUserTeen7IsRightEP5CUser
    // DEMANGLED: CUserTeen::IsRight(CUser *)
    bool IsRight(CUser* user);
};

static_assert(sizeof(CUserTeen) == 0x04u,
              "CUserTeen IDA ABI size must stay 0x04");

// IDA unit: 05_world_enter_village/CSchoolArea.
struct CSchoolArea {
    // IDA_EA: 0x856F18C
    // IDA_NAME: _ZN11CSchoolAreaC2Ev
    // DEMANGLED: CSchoolArea::CSchoolArea(void)
    CSchoolArea();
    // IDA_EA: 0x856F074
    // IDA_NAME: _ZN11CSchoolAreaD2Ev
    // DEMANGLED: CSchoolArea::~CSchoolArea()
    ~CSchoolArea();

    // IDA_EA: 0x85689A6
    // IDA_NAME: _ZN11CSchoolArea12GetAreaIndexEv
    // DEMANGLED: CSchoolArea::GetAreaIndex(void)
    int GetAreaIndex();
    // IDA_EA: 0x85689EC
    // IDA_NAME: _ZN11CSchoolArea14SetAreaMappingEji
    // DEMANGLED: CSchoolArea::SetAreaMapping(unsigned int,int)
    int SetAreaMapping(uint32_t school_key, int area_index);
    // IDA_EA: 0x8568A3E
    // IDA_NAME: _ZN11CSchoolArea13AddSchoolAreaEj
    // DEMANGLED: CSchoolArea::AddSchoolArea(unsigned int)
    int AddSchoolArea(uint32_t school_key);
    // IDA_EA: 0x8568A74
    // IDA_NAME: _ZN11CSchoolArea13DelSchoolAreaEj
    // DEMANGLED: CSchoolArea::DelSchoolArea(unsigned int)
    bool DelSchoolArea(uint32_t school_key);
    // IDA_EA: 0x86D23BC
    // IDA_NAME: _ZN11CSchoolArea13SetSchoolAreaEii
    // DEMANGLED: CSchoolArea::SetSchoolArea(int,int)
    bool SetSchoolArea(int begin_area, int end_area);
    // IDA_EA: 0x86D23F8
    // IDA_NAME: _ZN11CSchoolArea12GetAreaIndexEj
    // DEMANGLED: CSchoolArea::GetAreaIndex(unsigned int)
    int GetAreaIndex(uint32_t school_key);

    std::map<uint32_t, int> area_by_school;
    std::stack<int> free_areas;
};

// IDA unit: 05_world_enter_village/CGuildAgitArea.
struct CGuildAgitArea {
    // IDA_EA: 0x856F24C
    // IDA_NAME: _ZN14CGuildAgitAreaC2Ev
    // DEMANGLED: CGuildAgitArea::CGuildAgitArea(void)
    CGuildAgitArea();
    // IDA_EA: 0x856F100
    // IDA_NAME: _ZN14CGuildAgitAreaD2Ev
    // DEMANGLED: CGuildAgitArea::~CGuildAgitArea()
    ~CGuildAgitArea();

    // IDA_EA: 0x82AA324
    // IDA_NAME: _ZN14CGuildAgitArea18AllocGuildAgitAreaEj
    // DEMANGLED: CGuildAgitArea::AllocGuildAgitArea(unsigned int)
    int AllocGuildAgitArea(uint32_t guild_key);
    // IDA_EA: 0x82AA35A
    // IDA_NAME: _ZN14CGuildAgitArea7GetAreaEv
    // DEMANGLED: CGuildAgitArea::GetArea(void)
    int GetArea();
    // IDA_EA: 0x82AA3CA
    // IDA_NAME: _ZN14CGuildAgitArea14SetAreaMappingEji
    // DEMANGLED: CGuildAgitArea::SetAreaMapping(unsigned int,int)
    int SetAreaMapping(uint32_t guild_key, int area_index);
    // IDA_EA: 0x86D249E
    // IDA_NAME: _ZN14CGuildAgitArea22SetGuildAgitAreaDomainEii
    // DEMANGLED: CGuildAgitArea::SetGuildAgitAreaDomain(int,int)
    bool SetGuildAgitAreaDomain(int begin_area, int end_area);
    // IDA_EA: 0x86D24DA
    // IDA_NAME: _ZN14CGuildAgitArea21GetGuildAgitAreaIndexEji
    // DEMANGLED: CGuildAgitArea::GetGuildAgitAreaIndex(unsigned int,int)
    int GetGuildAgitAreaIndex(uint32_t guild_key, int offset);
    // IDA_EA: 0x86D2542
    // IDA_NAME: _ZN14CGuildAgitArea20ReleaseGuildAgitAreaEj
    // DEMANGLED: CGuildAgitArea::ReleaseGuildAgitArea(unsigned int)
    bool ReleaseGuildAgitArea(uint32_t guild_key);

    std::map<uint32_t, int> area_by_guild;
    std::stack<int> free_areas;
};

class CSchoolMember : public CBelong {
public:
    // IDA_EA: 0x85690B6
    // IDA_NAME: _ZN13CSchoolMemberC2EP5CUser
    // DEMANGLED: CSchoolMember::CSchoolMember(CUser *)
    explicit CSchoolMember(CUser* user);
    // IDA_EA: 0x85690DC
    // IDA_NAME: _ZN13CSchoolMember7IsRightEP5CUser
    // DEMANGLED: CSchoolMember::IsRight(CUser *)
    bool IsRight(CUser* user);

private:
    CUser* user_;
};

class CSchoolMgr {
public:
    // IDA_EA: 0x856F206
    // IDA_NAME: _ZN10CSchoolMgrC2Ev
    // DEMANGLED: CSchoolMgr::CSchoolMgr(void)
    CSchoolMgr();
    // IDA_EA: 0x856F0BA
    // IDA_NAME: _ZN10CSchoolMgrD2Ev
    // DEMANGLED: CSchoolMgr::~CSchoolMgr()
    ~CSchoolMgr();
    // IDA_EA: 0x84ED394
    // IDA_NAME: _ZN10CSchoolMgr12GetUserCountEj
    // DEMANGLED: CSchoolMgr::GetUserCount(unsigned int)
    int GetUserCount(uint32_t school_key);
    // IDA_EA: 0x8567FE8
    // IDA_NAME: _ZN10CSchoolMgr19SendBroadcastUserInEP5CUser
    // DEMANGLED: CSchoolMgr::SendBroadcastUserIn(CUser *)
    void SendBroadcastUserIn(CUser* user);
    // IDA_EA: 0x85681CE
    // IDA_NAME: _ZN10CSchoolMgr20SendBroadcastUserOutEP5CUser
    // DEMANGLED: CSchoolMgr::SendBroadcastUserOut(CUser *)
    void SendBroadcastUserOut(CUser* user);
    // IDA_EA: 0x85683A0
    // IDA_NAME: _ZN10CSchoolMgr7AddUserEP5CUser
    // DEMANGLED: CSchoolMgr::AddUser(CUser *)
    int AddUser(CUser* user);
    // IDA_EA: 0x85684BC
    // IDA_NAME: _ZN10CSchoolMgr7DelUserEP5CUser
    // DEMANGLED: CSchoolMgr::DelUser(CUser *)
    bool DelUser(CUser* user);
    // IDA_EA: 0x86D245C
    // IDA_NAME: _ZN10CSchoolMgr13SetSchoolAreaEii
    // DEMANGLED: CSchoolMgr::SetSchoolArea(int,int)
    bool SetSchoolArea(int begin_area, int end_area);
    // IDA_EA: 0x86D2480
    // IDA_NAME: _ZN10CSchoolMgr13GetSchoolAreaEj
    // DEMANGLED: CSchoolMgr::GetSchoolArea(unsigned int)
    int GetSchoolArea(uint32_t school_key);

private:
    std::map<uint32_t, uint32_t> users_;
    CSchoolArea areas_;
};

class CGuildAgitManager {
public:
    // IDA_EA: 0x856F2C6
    // IDA_NAME: _ZN17CGuildAgitManagerC2Ev
    // DEMANGLED: CGuildAgitManager::CGuildAgitManager(void)
    CGuildAgitManager();
    // IDA_EA: 0x856F146
    // IDA_NAME: _ZN17CGuildAgitManagerD2Ev
    // DEMANGLED: CGuildAgitManager::~CGuildAgitManager()
    ~CGuildAgitManager();
    // IDA_EA: 0x82AA41C
    // IDA_NAME: _ZN17CGuildAgitManager18AllocGuildAgitAreaEj
    // DEMANGLED: CGuildAgitManager::AllocGuildAgitArea(unsigned int)
    int AllocGuildAgitArea(uint32_t guild_key);
    // IDA_EA: 0x86D2642
    // IDA_NAME: _ZN17CGuildAgitManager22SetGuildAgitAreaDomainEii
    // DEMANGLED: CGuildAgitManager::SetGuildAgitAreaDomain(int,int)
    bool SetGuildAgitAreaDomain(int begin_area, int end_area);
    // IDA_EA: 0x86D2666
    // IDA_NAME: _ZN17CGuildAgitManager21GetGuildAgitAreaIndexEji
    // DEMANGLED: CGuildAgitManager::GetGuildAgitAreaIndex(unsigned int,int)
    int GetGuildAgitAreaIndex(uint32_t guild_key, int offset);
    // IDA_EA: 0x86D268A
    // IDA_NAME: _ZN17CGuildAgitManager20ReleaseGuildAgitAreaEj
    // DEMANGLED: CGuildAgitManager::ReleaseGuildAgitArea(unsigned int)
    bool ReleaseGuildAgitArea(uint32_t guild_key);

private:
    std::map<uint32_t, uint32_t> users_;
    CGuildAgitArea areas_;
};

extern CSchoolMgr g_schoolMgr __asm__("g_schoolMgr");
extern CGuildAgitManager g_guildAgitMgr __asm__("g_guildAgitMgr");

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CBelong) == 0x04u, "CBelong IDA ABI size must stay 0x04");
static_assert(sizeof(CSchoolMember) == 0x08u,
              "CSchoolMember IDA ABI size must stay 0x08");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_WORLD_AREA_ALLOCATORS_H_
