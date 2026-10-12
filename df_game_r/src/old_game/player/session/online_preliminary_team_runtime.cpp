#include "old_game/player/session/online_preliminary_team_runtime.h"

#include <cstring>
#include <new>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/player/session/timer_update_online_preliminary_team.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace online_preliminary {

namespace {

int GetBoundedTeamNameLength(const char* team_name) {
    int length = 0;
    while (length < 16 && team_name[length] != '\0') {
        ++length;
    }
    return length;
}

bool QueueLoadTeamMatchList(int team_id) {
    if (!GlobalData::s_stream_pool || !GlobalData::s_msgq_mgr) {
        return false;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "OnlinePreliminaryTeam.cpp", 262);
    if (!stream) {
        return false;
    }

    CStreamGuard guard(stream, true);
    // IDA 0x858894A stores 0x1EC, i.e. DBTask slot 492 (not decimal
    // 236). This request is answered through Inter_LoadOnlinePreliminaryTeamMatchList.
    (*guard) << 492;
    (*guard) << -1;
    (*guard) << team_id;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return true;
}

void BroadcastTeamEnter(CUser* user,
                        const SIG_ONLINE_PRELIMINARY_TEAM& payload) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 280);
    packet.put_short(user->get_unique_id());
    packet.put_int(payload.team_id);
    const int name_length = GetBoundedTeamNameLength(payload.team_name);
    packet.put_int(name_length);
    packet.put_str(payload.team_name, name_length);
    packet.finalize(true);

    GameWorld* const world = G_GameWorld();
    if (world) {
        world->send_all(packet);
    }
}

}  // namespace

// IDA_EA: 0x8588E2E
// IDA_NAME: _ZN18online_preliminary12CTeamFactoryC2Ev
// DEMANGLED: online_preliminary::CTeamFactory::CTeamFactory(void)
CTeamFactory::CTeamFactory()
    : StaticPool<COnlinePreliminaryTeam, 600>() {}

// IDA_EA: 0x8588E42
// IDA_NAME: _ZN18online_preliminary12CTeamFactoryD2Ev
// DEMANGLED: online_preliminary::CTeamFactory::~CTeamFactory()
CTeamFactory::~CTeamFactory() {}

// IDA_EA: 0x8588DD8
// IDA_NAME: _ZN18online_preliminary12CTeamFactory7AcquireEv
// DEMANGLED: online_preliminary::CTeamFactory::Acquire(void)
COnlinePreliminaryTeam* CTeamFactory::Acquire() {
    return StaticPool<COnlinePreliminaryTeam, 600>::Acquire();
}

// IDA_EA: 0x858860C
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgrC2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::COnlinePreliminaryTeamMgr(void)
COnlinePreliminaryTeamMgr::COnlinePreliminaryTeamMgr()
    : factory(new (std::nothrow) CTeamFactory()),
      teams() {}

// IDA_EA: 0x85886FE
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgrD2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::~COnlinePreliminaryTeamMgr()
COnlinePreliminaryTeamMgr::~COnlinePreliminaryTeamMgr() {
    delete factory;
    factory = 0;
}

// IDA_EA: 0x858875E
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr8FindTeamEi
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::FindTeam(int)
COnlinePreliminaryTeam*
COnlinePreliminaryTeamMgr::FindTeam(int team_id) {
    std::map<int, COnlinePreliminaryTeam*>::iterator it = teams.find(team_id);
    return it == teams.end() ? 0 : it->second;
}

// IDA_EA: 0x85887C8
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr11OnEnterUserEP5CUserPc
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::OnEnterUser(CUser *,char *)
int COnlinePreliminaryTeamMgr::OnEnterUser(CUser* user, char* data) {
    if (!user || !data) {
        return 0;
    }

    const SIG_ONLINE_PRELIMINARY_TEAM& payload =
        *reinterpret_cast<const SIG_ONLINE_PRELIMINARY_TEAM*>(data);
    COnlinePreliminaryTeam* team = FindTeam(payload.team_id);
    if (team) {
        user->SetCurCharacOnlinePreliminaryTeam(team);
        team->IncreaseCurrentUserCount();
        BroadcastTeamEnter(user, payload);
        return 1;
    }

    if (!factory) {
        return 0;
    }

    team = factory->Acquire();
    if (!team) {
        return 0;
    }

    team->Reset();
    user->SetCurCharacOnlinePreliminaryTeam(team);
    team->SetCurrentUserCount(1);
    team->loadDataPreliminaryTeam(data);
    teams.insert(std::make_pair(payload.team_id, team));

    if (!QueueLoadTeamMatchList(payload.team_id)) {
        teams.erase(payload.team_id);
        user->SetCurCharacOnlinePreliminaryTeam(0);
        factory->Free(team);
        return 0;
    }

    Timer_UpdateOnlinePreliminaryTeam::RegistNextTimer(payload.team_id);
    BroadcastTeamEnter(user, payload);
    return 1;
}

// IDA_EA: 0x8588BE4
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr14OnLoadTeamInfoEP5CUserPc
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::OnLoadTeamInfo(CUser *,char *)
int COnlinePreliminaryTeamMgr::OnLoadTeamInfo(CUser* user, char* data) {
    (void)user;
    if (!data) {
        return 0;
    }

    const int team_id =
        reinterpret_cast<const SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*>(data)
            ->team_id;
    COnlinePreliminaryTeam* const team = FindTeam(team_id);
    return team ? team->loadDataPreliminaryTeamMatchList(data) : 0;
}

// IDA_EA: 0x8588C5C
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr18ResetDailyMidnightEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::ResetDailyMidnight(void)
int COnlinePreliminaryTeamMgr::ResetDailyMidnight() {
    for (std::map<int, COnlinePreliminaryTeam*>::iterator it = teams.begin();
         it != teams.end();
         ++it) {
        it->second->ResetDailyMidnight();
    }
    return 0;
}

// IDA_EA: 0x8588D2A
// IDA_NAME: _ZN18online_preliminary15PlayCountInfoExC2Ev
// DEMANGLED: online_preliminary::PlayCountInfoEx::PlayCountInfoEx(void)
PlayCountInfoEx::PlayCountInfoEx() : play_count(0), dirty(false) {}

// IDA_EA: 0x8588D40
// IDA_NAME: _ZN18online_preliminary15PlayCountInfoExC2Ejb
// DEMANGLED: online_preliminary::PlayCountInfoEx::PlayCountInfoEx(unsigned int,bool)
PlayCountInfoEx::PlayCountInfoEx(unsigned int play_count_value,
                                 bool dirty_value)
    : play_count(play_count_value), dirty(dirty_value) {}

// IDA_EA: 0x8568EEE
// IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam9GetRatingEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetRating(void)const
float COnlinePreliminaryTeam::GetRating() const {
    return rating_;
}

// IDA_EA: 0x8587EB8
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeamC2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::COnlinePreliminaryTeam(void)
COnlinePreliminaryTeam::COnlinePreliminaryTeam() : play_counts_() {}

// IDA_EA: 0x8587ECE
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeamD2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::~COnlinePreliminaryTeam()
COnlinePreliminaryTeam::~COnlinePreliminaryTeam() {}

// IDA_EA: 0x8587EE4
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam9_saveTeamEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::_saveTeam(void)
int COnlinePreliminaryTeam::_saveTeam() {
    if (team_dirty_) {
        Stream* const stream = GlobalData::s_stream_pool->Acquire(
            "OnlinePreliminaryTeam.cpp", 56);
        CStreamGuard guard(stream, true);
        // IDA 0x8587F43 stores 0x1E9: DB_UpdateOnlinePreliminaryTeam.
        (*guard) << 489;
        (*guard) << -1;
        SIG_ONLINE_PRELIMINARY_TEAM* const payload =
            reinterpret_cast<SIG_ONLINE_PRELIMINARY_TEAM*>(
                guard->GetInBufferUntyped(sizeof(SIG_ONLINE_PRELIMINARY_TEAM)));
        payload->team_id = team_id_;
        payload->rating = rating_;
        payload->win_count = static_cast<int>(win_count_);
        payload->lose_count = static_cast<int>(lose_count_);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
        team_dirty_ = false;
    }
    return 1;
}

// IDA_EA: 0x858800C
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam18_saveTeamMatchListEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::_saveTeamMatchList(void)
int COnlinePreliminaryTeam::_saveTeamMatchList() {
    if (!play_counts_dirty_) {
        return 1;
    }

    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "OnlinePreliminaryTeam.cpp", 78);
    CStreamGuard guard(stream, false);
    // IDA 0x858806B stores 0x1ED: DB_UpdateOnlinePreliminaryTeamMatchList.
    (*guard) << 493;
    (*guard) << -1;
    SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST* payload =
        reinterpret_cast<SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST)));
    payload->team_id = team_id_;
    payload->count = 0;

    for (std::map<int, PlayCountInfoEx>::iterator it = play_counts_.begin();
         it != play_counts_.end();
         ++it) {
        PlayCountInfoEx& info = it->second;
        if (!info.dirty) {
            continue;
        }

        info.dirty = false;
        SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY& entry =
            payload->entries[payload->count];
        entry.other_team_id = it->first;
        entry.play_count = info.play_count;
        ++payload->count;

        if (payload->count == 100u) {
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
            stream = GlobalData::s_stream_pool->Acquire(
                "OnlinePreliminaryTeam.cpp", 102);
            guard.Set(stream);
            (*guard) << 493;
            (*guard) << -1;
            payload = reinterpret_cast<SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*>(
                guard->GetInBufferUntyped(
                    sizeof(SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST)));
            payload->team_id = team_id_;
            payload->count = 0;
        }
    }

    if (payload->count != 0u) {
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else {
        GlobalData::s_stream_pool->Free(guard);
    }
    play_counts_dirty_ = false;
    return 1;
}

// IDA_EA: 0x85882B8
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam23loadDataPreliminaryTeamEPc
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::loadDataPreliminaryTeam(char *)
int COnlinePreliminaryTeam::loadDataPreliminaryTeam(char* data) {
    const SIG_ONLINE_PRELIMINARY_TEAM& payload =
        *reinterpret_cast<const SIG_ONLINE_PRELIMINARY_TEAM*>(data);
    team_id_ = payload.team_id;
    std::strncpy(team_name_, payload.team_name, 15u);
    rating_ = payload.rating;
    win_count_ = static_cast<unsigned int>(payload.win_count);
    lose_count_ = static_cast<unsigned int>(payload.lose_count);
    return 1;
}

// IDA_EA: 0x858831A
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam32loadDataPreliminaryTeamMatchListEPc
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::loadDataPreliminaryTeamMatchList(char *)
int COnlinePreliminaryTeam::loadDataPreliminaryTeamMatchList(char* data) {
    const SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST& payload =
        *reinterpret_cast<const SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST*>(data);
    for (unsigned int index = 0; index < payload.count; ++index) {
        const SIG_ONLINE_PRELIMINARY_TEAM_MATCH_LIST_ENTRY& entry =
            payload.entries[index];
        play_counts_[entry.other_team_id].play_count = entry.play_count;
    }
    return 1;
}

// IDA_EA: 0x8588374
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam18ResetDailyMidnightEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::ResetDailyMidnight(void)
void COnlinePreliminaryTeam::ResetDailyMidnight() {
    play_counts_.clear();
}

// IDA_EA: 0x858838A
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam5ResetEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::Reset(void)
COnlinePreliminaryTeam* COnlinePreliminaryTeam::Reset() {
    current_user_count_ = 0;
    team_id_ = 0;
    std::memset(team_name_, 0, sizeof(team_name_));
    rating_ = 0.0f;
    win_count_ = 0;
    lose_count_ = 0;
    play_counts_.clear();
    team_dirty_ = false;
    play_counts_dirty_ = false;
    return this;
}

// IDA_EA: 0x85883FE
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam16IncreaseWinCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseWinCount(void)
COnlinePreliminaryTeam* COnlinePreliminaryTeam::IncreaseWinCount() {
    ++win_count_;
    team_dirty_ = true;
    return this;
}

// IDA_EA: 0x858841A
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam17IncreaseLoseCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseLoseCount(void)
COnlinePreliminaryTeam* COnlinePreliminaryTeam::IncreaseLoseCount() {
    ++lose_count_;
    team_dirty_ = true;
    return this;
}

// IDA_EA: 0x8588436
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam9SetRatingEf
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::SetRating(float)
COnlinePreliminaryTeam* COnlinePreliminaryTeam::SetRating(float rating) {
    rating_ = rating;
    team_dirty_ = true;
    return this;
}

// IDA_EA: 0x858844C
// IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam25GetEachOtherTeamPlayCountEi
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetEachOtherTeamPlayCount(int)const
int COnlinePreliminaryTeam::GetEachOtherTeamPlayCount(int other_team_id) const {
    const std::map<int, PlayCountInfoEx>::const_iterator it =
        play_counts_.find(other_team_id);
    return it == play_counts_.end() ? 0 : static_cast<int>(it->second.play_count);
}

// IDA_EA: 0x85884B6
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam30IncreaseEachOtherTeamPlayCountEi
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseEachOtherTeamPlayCount(int)
COnlinePreliminaryTeam*
COnlinePreliminaryTeam::IncreaseEachOtherTeamPlayCount(int other_team_id) {
    std::map<int, PlayCountInfoEx>::iterator it =
        play_counts_.find(other_team_id);
    if (it != play_counts_.end()) {
        it->second.dirty = true;
        ++it->second.play_count;
    } else {
        if (play_counts_.size() > 999u) {
            cMyTrace trace(
                "void online_preliminary::COnlinePreliminaryTeam::IncreaseEachOtherTeamPlayCount(int)",
                202,
                0);
            trace("Online Preliminary each other team play count map size over! %d",
                  static_cast<int>(play_counts_.size()));
            return this;
        }
        play_counts_.insert(std::make_pair(
            other_team_id, PlayCountInfoEx(1u, true)));
    }
    play_counts_dirty_ = true;
    return this;
}

// IDA_EA: 0x8588D88
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam19SetCurrentUserCountEi
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::SetCurrentUserCount(int)
COnlinePreliminaryTeam*
COnlinePreliminaryTeam::SetCurrentUserCount(int count) {
    current_user_count_ = count;
    return this;
}

// IDA_EA: 0x8588D96
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam24IncreaseCurrentUserCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::IncreaseCurrentUserCount(void)
COnlinePreliminaryTeam* COnlinePreliminaryTeam::IncreaseCurrentUserCount() {
    ++current_user_count_;
    return this;
}

// IDA_EA: 0x8697892
// IDA_NAME: _ZNK18online_preliminary22COnlinePreliminaryTeam9GetTeamIdEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetTeamId(void)const
int COnlinePreliminaryTeam::GetTeamId() const {
    return team_id_;
}

// IDA_EA: 0x869789E
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam11GetTeamNameEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetTeamName(void)
char* COnlinePreliminaryTeam::GetTeamName() {
    return team_name_;
}

// IDA_EA: 0x8588AE8
// IDA_NAME: _ZN18online_preliminary25COnlinePreliminaryTeamMgr11OnLeaveUserEP5CUser
// DEMANGLED: online_preliminary::COnlinePreliminaryTeamMgr::OnLeaveUser(CUser *)
int COnlinePreliminaryTeamMgr::OnLeaveUser(CUser* user) {
    const int team_id = user->GetCurCharacOnlinePreliminaryTeamId();
    std::map<int, COnlinePreliminaryTeam*>::iterator it = teams.find(team_id);
    if (it == teams.end()) {
        return 0;
    }

    COnlinePreliminaryTeam* const team = it->second;
    user->SetCurCharacOnlinePreliminaryTeam(0);
    if (team->GetCurrentUserCount() == 1) {
        team->saveData();
        factory->Free(team);
        teams.erase(it);
        return 0;
    }

    return reinterpret_cast<int>(team->DecreaseCurrentUserCount());
}

}  // namespace online_preliminary
