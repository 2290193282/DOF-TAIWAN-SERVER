#include "old_game/player/session/online_preliminary_match_list_runtime.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/online_preliminary_stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace online_preliminary {

// IDA_EA: 0x8586938
// IDA_NAME: _ZN18online_preliminary13PlayCountInfoC2Ev
// DEMANGLED: online_preliminary::PlayCountInfo::PlayCountInfo(void)
PlayCountInfo::PlayCountInfo() : play_count(0), dirty(false) {}

// IDA_EA: 0x858694E
// IDA_NAME: _ZN18online_preliminary13PlayCountInfoC2Ejb
// DEMANGLED: online_preliminary::PlayCountInfo::PlayCountInfo(unsigned int,bool)
PlayCountInfo::PlayCountInfo(uint32_t play_count_value, bool dirty_value)
    : play_count(play_count_value), dirty(dirty_value) {}

// IDA_EA: 0x8586184
// IDA_NAME: _ZNK18online_preliminary33COnlinePreliminaryMemberMatchList16GetPeerPlayCountEij
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::GetPeerPlayCount(int,unsigned int)const
int COnlinePreliminaryMemberMatchList::GetPeerPlayCount(
    int server_group, uint32_t charac_no) const {
    const CharacterInServer peer(server_group, charac_no);
    std::map<CharacterInServer, PlayCountInfo>::const_iterator found =
        play_counts_.find(peer);
    if (found != play_counts_.end()) {
        return static_cast<int>(found->second.play_count);
    }
    return 0;
}

// IDA_EA: 0x8586208
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchList21IncreasePeerPlayCountEij
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::IncreasePeerPlayCount(int,unsigned int)
charac_expand::CData* COnlinePreliminaryMemberMatchList::IncreasePeerPlayCount(
    int server_group, uint32_t charac_no) {
    const CharacterInServer peer(server_group, charac_no);
    std::map<CharacterInServer, PlayCountInfo>::iterator found =
        play_counts_.find(peer);
    if (found != play_counts_.end()) {
        found->second.dirty = true;
        ++found->second.play_count;
    } else {
        if (play_counts_.size() > 999u) {
            cMyTrace trace(
                "void online_preliminary::COnlinePreliminaryMemberMatchList::"
                "IncreasePeerPlayCount(int, unsigned int)",
                101,
                0);
            return reinterpret_cast<charac_expand::CData*>(
                trace("Online Preliminary peer play count map size over! %d",
                      static_cast<int>(play_counts_.size())));
        }
        play_counts_.insert(
            std::make_pair(CharacterInServer(server_group, charac_no),
                           PlayCountInfo(1u, true)));
    }
    return alter();
}

// IDA_EA: 0x8586398
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchListC2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::COnlinePreliminaryMemberMatchList(void)
COnlinePreliminaryMemberMatchList::COnlinePreliminaryMemberMatchList()
    : play_counts_() {}

// IDA_EA: 0x85863E6
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchListD2Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::~COnlinePreliminaryMemberMatchList()
// IDA_EA: 0x8586448
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchListD0Ev
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::~COnlinePreliminaryMemberMatchList()
COnlinePreliminaryMemberMatchList::~COnlinePreliminaryMemberMatchList() {}

// IDA_EA: 0x8586466
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchList8loadDataEP5CUserPc
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::loadData(CUser *,char *)
bool COnlinePreliminaryMemberMatchList::loadData(CUser* user, char* data) {
    (void)user;
    const SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST* payload =
        reinterpret_cast<const SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST*>(
            data);
    for (uint32_t index = 0; index < payload->count; ++index) {
        const SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY& entry =
            payload->entries[index];
        const CharacterInServer peer(entry.server_group, entry.charac_no);
        play_counts_[peer].play_count = entry.play_count;
    }
    return 1;
}

// IDA_EA: 0x8586510
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchList9_saveDataEP5CUser
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::_saveData(CUser *)
int COnlinePreliminaryMemberMatchList::_saveData(CUser* user) {
    Stream* stream = GlobalData::s_stream_pool->Acquire(
        "OnlinePreliminaryData.cpp", 130);
    CStreamGuard guard(stream, true);
    (*guard) << 0x1eb;
    (*guard) << user->GetUID();
    SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST* payload =
        guard->GetInBuffer<SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>();
    payload->server_group = static_cast<uint16_t>(user->GetServerGroup());
    payload->charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    payload->count = 0;

    std::map<CharacterInServer, PlayCountInfo>::iterator it =
        play_counts_.begin();
    for (; it != play_counts_.end(); ++it) {
        const CharacterInServer& peer = it->first;
        PlayCountInfo& info = it->second;
        if (!info.dirty) {
            continue;
        }

        info.dirty = false;
        SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY& entry =
            payload->entries[payload->count];
        entry.server_group = peer.server_group;
        entry.charac_no = peer.charac_no;
        entry.play_count = info.play_count;
        ++payload->count;

        if (payload->count == 100u) {
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
            {
                Stream* next_stream = GlobalData::s_stream_pool->Acquire(
                    "OnlinePreliminaryData.cpp", 157);
                CStreamGuard next_guard(next_stream, true);
                (*next_guard) << 0x1e7;
                (*next_guard) << user->GetUID();
                payload = next_guard->GetInBuffer<
                    SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST>();
                payload->server_group =
                    static_cast<uint16_t>(user->GetServerGroup());
                payload->charac_no =
                    static_cast<uint32_t>(user->getCurCharacNo());
                payload->count = 0;
            }
        }
    }

    if (payload->count != 0u) {
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else {
        GlobalData::s_stream_pool->Free(guard);
    }
    return 1;
}

// IDA_EA: 0x8586848
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchList6_resetEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::_reset(void)
int COnlinePreliminaryMemberMatchList::_reset() {
    play_counts_.clear();
    return reinterpret_cast<int>(&play_counts_);
}

// IDA_EA: 0x858685E
// IDA_NAME: _ZN18online_preliminary33COnlinePreliminaryMemberMatchList18ResetDailyMidnightEv
// DEMANGLED: online_preliminary::COnlinePreliminaryMemberMatchList::ResetDailyMidnight(void)
void COnlinePreliminaryMemberMatchList::ResetDailyMidnight() {
    _reset();
}

}  // namespace online_preliminary
