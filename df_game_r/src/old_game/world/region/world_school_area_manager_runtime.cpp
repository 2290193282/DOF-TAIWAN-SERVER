#include "old_game/world/region/world_area_allocators.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "core/config/env/environment.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <stdio.h>
#include <string.h>


CSchoolMgr g_schoolMgr __asm__("g_schoolMgr");
CGuildAgitManager g_guildAgitMgr __asm__("g_guildAgitMgr");

// IDA_EA: 0x856F2C6
// IDA_NAME: _ZN17CGuildAgitManagerC2Ev
// DEMANGLED: CGuildAgitManager::CGuildAgitManager(void)
CGuildAgitManager::CGuildAgitManager()
    : users_(),
      areas_() {}

// IDA_EA: 0x856F146
// IDA_NAME: _ZN17CGuildAgitManagerD2Ev
// DEMANGLED: CGuildAgitManager::~CGuildAgitManager()
CGuildAgitManager::~CGuildAgitManager() {}

// IDA_EA: 0x82AA41C
// IDA_NAME: _ZN17CGuildAgitManager18AllocGuildAgitAreaEj
// DEMANGLED: CGuildAgitManager::AllocGuildAgitArea(unsigned int)
int CGuildAgitManager::AllocGuildAgitArea(uint32_t guild_key) {
    std::map<uint32_t, uint32_t>::iterator found = users_.find(guild_key);
    if (found != users_.end()) {
        return static_cast<int>(++found->second);
    }

    users_.insert(std::make_pair(guild_key, 1u));
    return areas_.AllocGuildAgitArea(guild_key);
}

// IDA_EA: 0x86D2642
// IDA_NAME: _ZN17CGuildAgitManager22SetGuildAgitAreaDomainEii
// DEMANGLED: CGuildAgitManager::SetGuildAgitAreaDomain(int,int)
bool CGuildAgitManager::SetGuildAgitAreaDomain(int begin_area, int end_area) {
    return areas_.SetGuildAgitAreaDomain(begin_area, end_area);
}

// IDA_EA: 0x84F07C2
// IDA_NAME: _ZN7CBelongC2Ev
// DEMANGLED: CBelong::CBelong(void)
CBelong::CBelong() {
    // The compiler writes CBelong's original single-entry vtable here; the
    // derived recipient constructor then replaces it with its own vtable.
}

// IDA_EA: 0x84F07D0
// IDA_NAME: _ZN9CUserTeenC2Ev
// DEMANGLED: CUserTeen::CUserTeen(void)
CUserTeen::CUserTeen() : CBelong() {}

// IDA_EA: 0x84F078C
// IDA_NAME: _ZN9CUserTeen7IsRightEP5CUser
// DEMANGLED: CUserTeen::IsRight(CUser *)
bool CUserTeen::IsRight(CUser* user) {
    int unique_id;
    return user->CheckLimitMiniumAge(15, unique_id) == 1;
}

// IDA_EA: 0x85690B6
// IDA_NAME: _ZN13CSchoolMemberC2EP5CUser
// DEMANGLED: CSchoolMember::CSchoolMember(CUser *)
CSchoolMember::CSchoolMember(CUser* user)
    : CBelong(),
      user_(user) {}

// IDA_EA: 0x85690DC
// IDA_NAME: _ZN13CSchoolMember7IsRightEP5CUser
// DEMANGLED: CSchoolMember::IsRight(CUser *)
bool CSchoolMember::IsRight(CUser* user) {
    return user_ != user && user_->GetSchoolNo() == user->GetSchoolNo();
}

// IDA_EA: 0x856F206
// IDA_NAME: _ZN10CSchoolMgrC2Ev
// DEMANGLED: CSchoolMgr::CSchoolMgr(void)
CSchoolMgr::CSchoolMgr()
    : users_(),
      areas_() {}

// IDA_EA: 0x856F0BA
// IDA_NAME: _ZN10CSchoolMgrD2Ev
// DEMANGLED: CSchoolMgr::~CSchoolMgr()
CSchoolMgr::~CSchoolMgr() {}

// IDA_EA: 0x84ED394
// IDA_NAME: _ZN10CSchoolMgr12GetUserCountEj
// DEMANGLED: CSchoolMgr::GetUserCount(unsigned int)
int CSchoolMgr::GetUserCount(uint32_t school_key) {
    std::map<uint32_t, uint32_t>::const_iterator found =
        users_.find(school_key);
    return found == users_.end() ? 0 : static_cast<int>(found->second);
}

namespace {

void SendSchoolBroadcast(CUser* user, const char* string_key, bool append_channel) {
    const char* const format =
        g_scriptStringManager_.findString(4, string_key, 0);
    if (!format) {
        return;
    }

    char message[255];
    memset(message, 0, sizeof(message));
    if (append_channel) {
        sprintf(message,
                format,
                user->getCurCharacName(),
                G_CEnvironment()->get_channel_no());
    } else {
        sprintf(message, format, user->getCurCharacName());
    }

    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    const int message_length = static_cast<int>(strlen(message));
    packet.put_int(message_length);
    packet.put_str(message, message_length);
    packet.finalize(true);

    CSchoolMember member(user);
    G_GameWorld()->send_group(packet, &member);
}

}  // namespace

// IDA_EA: 0x8567FE8
// IDA_NAME: _ZN10CSchoolMgr19SendBroadcastUserInEP5CUser
// DEMANGLED: CSchoolMgr::SendBroadcastUserIn(CUser *)
void CSchoolMgr::SendBroadcastUserIn(CUser* user) {
    SendSchoolBroadcast(user, "channel_connect", true);
}

// IDA_EA: 0x85681CE
// IDA_NAME: _ZN10CSchoolMgr20SendBroadcastUserOutEP5CUser
// DEMANGLED: CSchoolMgr::SendBroadcastUserOut(CUser *)
void CSchoolMgr::SendBroadcastUserOut(CUser* user) {
    SendSchoolBroadcast(user, "channel_disconnect", false);
}

// IDA_EA: 0x85683A0
// IDA_NAME: _ZN10CSchoolMgr7AddUserEP5CUser
// DEMANGLED: CSchoolMgr::AddUser(CUser *)
int CSchoolMgr::AddUser(CUser* user) {
    if (!G_GameWorld()->IsSchoolPvPChannel()) {
        return 1;
    }

    SendBroadcastUserIn(user);
    const uint32_t school_key = static_cast<uint32_t>(user->GetSchoolNo());
    std::map<uint32_t, uint32_t>::iterator found = users_.find(school_key);
    if (found != users_.end()) {
        return static_cast<int>(++found->second);
    }

    users_.insert(std::make_pair(school_key, 1u));
    return areas_.AddSchoolArea(school_key);
}

// IDA_EA: 0x85684BC
// IDA_NAME: _ZN10CSchoolMgr7DelUserEP5CUser
// DEMANGLED: CSchoolMgr::DelUser(CUser *)
bool CSchoolMgr::DelUser(CUser* user) {
    if (!G_GameWorld()->IsSchoolPvPChannel()) {
        return true;
    }

    SendBroadcastUserOut(user);
    const uint32_t school_key = static_cast<uint32_t>(user->GetSchoolNo());
    std::map<uint32_t, uint32_t>::iterator found = users_.find(school_key);
    if (found == users_.end()) {
        return false;
    }

    --found->second;
    if (found->second != 0) {
        return false;
    }

    users_.erase(found);
    return areas_.DelSchoolArea(school_key);
}

// IDA_EA: 0x86D245C
// IDA_NAME: _ZN10CSchoolMgr13SetSchoolAreaEii
// DEMANGLED: CSchoolMgr::SetSchoolArea(int,int)
bool CSchoolMgr::SetSchoolArea(int begin_area, int end_area) {
    return areas_.SetSchoolArea(begin_area, end_area);
}

// IDA_EA: 0x86D2480
// IDA_NAME: _ZN10CSchoolMgr13GetSchoolAreaEj
// DEMANGLED: CSchoolMgr::GetSchoolArea(unsigned int)
int CSchoolMgr::GetSchoolArea(uint32_t school_key) {
    return areas_.GetAreaIndex(school_key);
}

// IDA_EA: 0x86D2666
// IDA_NAME: _ZN17CGuildAgitManager21GetGuildAgitAreaIndexEji
// DEMANGLED: CGuildAgitManager::GetGuildAgitAreaIndex(unsigned int,int)
int CGuildAgitManager::GetGuildAgitAreaIndex(uint32_t guild_key,
                                              int offset) {
    return areas_.GetGuildAgitAreaIndex(guild_key, offset);
}

// IDA_EA: 0x86D268A
// IDA_NAME: _ZN17CGuildAgitManager20ReleaseGuildAgitAreaEj
// DEMANGLED: CGuildAgitManager::ReleaseGuildAgitArea(unsigned int)
bool CGuildAgitManager::ReleaseGuildAgitArea(uint32_t guild_key) {
    std::map<uint32_t, uint32_t>::iterator found = users_.find(guild_key);
    if (found == users_.end()) {
        return false;
    }

    --found->second;
    if (found->second != 0) {
        return false;
    }

    users_.erase(found);
    return areas_.ReleaseGuildAgitArea(guild_key);
}
