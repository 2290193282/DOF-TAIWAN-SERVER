#include "old_game/player/session/inter_online_preliminary_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_game/player/core/charac_expand_cdata.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/session/online_preliminary_team_runtime.h"


namespace {

int LoadOnlinePreliminaryCData(CUser* user,
                               char* payload,
                               ENUM_CHARAC_EXPAND_TYPE expand_type) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    // CDataMgr constructs both original slots during CUser construction; the
    // historical leaf dispatches through the CData vtable without a separate
    // payload decoder at this boundary.
    charac_expand::CData* const data = user->GetCharacExpandData(expand_type);
    data->loadData(user, payload);
    return 0;
}

bool IsTeamPayloadValid(const char* payload, int payload_size) {
    return payload &&
           payload_size >= static_cast<int>(sizeof(SIG_ONLINE_PRELIMINARY_TEAM));
}

bool IsTeamMatchListPayloadValid(const char* payload, int payload_size) {
    if (!payload || payload_size < 8) {
        return false;
    }

    uint32_t count = 0;
    std::memcpy(&count, payload + 4, sizeof(count));
    const uint32_t kMaximumEntries = 100u;
    if (count > kMaximumEntries) {
        return false;
    }

    const uint32_t required =
        8u + count * static_cast<uint32_t>(
                         sizeof(SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY));
    return required <= static_cast<uint32_t>(payload_size);
}

}  // namespace

// IDA_EA: 0x84E263E
// IDA_NAME: _ZN33Inter_LoadOnlinePreliminaryMember12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadOnlinePreliminaryMember::dispatch_sig(CUser *,char *,int)
int Inter_LoadOnlinePreliminaryMember::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int) {
    return LoadOnlinePreliminaryCData(
        user, payload, static_cast<ENUM_CHARAC_EXPAND_TYPE>(4));
}

// IDA_EA: 0x84EE7A0 / 0x84F6C48 / 0x84F6C78
Inter_LoadOnlinePreliminaryMember::Inter_LoadOnlinePreliminaryMember()
    : ISigDispatcher() {}
Inter_LoadOnlinePreliminaryMember::~Inter_LoadOnlinePreliminaryMember() {}

// IDA_EA: 0x84E271E
// IDA_NAME: _ZN42Inter_LoadOnlinePreliminaryMemberMatchList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadOnlinePreliminaryMemberMatchList::dispatch_sig(CUser *,char *,int)
int Inter_LoadOnlinePreliminaryMemberMatchList::dispatch_sig(CUser* user,
                                                             char* payload,
                                                             int) {
    return LoadOnlinePreliminaryCData(
        user, payload, static_cast<ENUM_CHARAC_EXPAND_TYPE>(5));
}

// IDA_EA: 0x84EE7D8 / 0x84F6BAC / 0x84F6BDC
Inter_LoadOnlinePreliminaryMemberMatchList::
    Inter_LoadOnlinePreliminaryMemberMatchList()
    : ISigDispatcher() {}
Inter_LoadOnlinePreliminaryMemberMatchList::
    ~Inter_LoadOnlinePreliminaryMemberMatchList() {}

// IDA_EA: 0x84E26B0
// IDA_NAME: _ZN31Inter_LoadOnlinePreliminaryTeam12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadOnlinePreliminaryTeam::dispatch_sig(CUser *,char *,int)
int Inter_LoadOnlinePreliminaryTeam::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR() ||
        !IsTeamPayloadValid(payload, payload_size)) {
        return 0;
    }

    online_preliminary::COnlinePreliminaryTeamMgr* const manager =
        GlobalData::s_onlinePreliminaryTeamMgr;
    if (manager && manager->OnEnterUser(user, payload) == 1) {
        return 0;
    }
    return 21017;
}

// IDA_EA: 0x84EE7BC / 0x84F6BFA / 0x84F6C2A
Inter_LoadOnlinePreliminaryTeam::Inter_LoadOnlinePreliminaryTeam()
    : ISigDispatcher() {}
Inter_LoadOnlinePreliminaryTeam::~Inter_LoadOnlinePreliminaryTeam() {}

// IDA_EA: 0x84E2790
// IDA_NAME: _ZN40Inter_LoadOnlinePreliminaryTeamMatchList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadOnlinePreliminaryTeamMatchList::dispatch_sig(CUser *,char *,int)
int Inter_LoadOnlinePreliminaryTeamMatchList::dispatch_sig(CUser* user,
                                                           char* payload,
                                                           int payload_size) {
    if (!IsTeamMatchListPayloadValid(payload, payload_size)) {
        return 0;
    }

    online_preliminary::COnlinePreliminaryTeamMgr* const manager =
        GlobalData::s_onlinePreliminaryTeamMgr;
    if (manager) {
        manager->OnLoadTeamInfo(user, payload);
    }
    return 0;
}

// IDA_EA: 0x84EE7F4 / 0x84F6B5E / 0x84F6B8E
Inter_LoadOnlinePreliminaryTeamMatchList::
    Inter_LoadOnlinePreliminaryTeamMatchList()
    : ISigDispatcher() {}
Inter_LoadOnlinePreliminaryTeamMatchList::
    ~Inter_LoadOnlinePreliminaryTeamMatchList() {}
