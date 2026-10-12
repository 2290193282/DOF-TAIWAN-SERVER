#include "old_game/pvp/arena/pvp_room.h"

#include <algorithm>
#include <cstring>

#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"

// IDA_EA: 0x82B91BA
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE6cVoterC2Ev
cElection<int, 8, 8>::cVoter::cVoter() {
    Init();
}

// IDA_EA: 0x82BEC16
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE6cVoter4InitEv
cElection<int, 8, 8>::cVoter* cElection<int, 8, 8>::cVoter::Init() {
    std::fill(votes_, votes_ + 8, 0);
    return this;
}

// IDA_EA: 0x82AB00C
// IDA_NAME: _ZN9cElectionIiLi8ELi8EEC2Ev
cElection<int, 8, 8>::cElection() {
    Init();
}

// IDA_EA: 0x82B58B0
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE4InitEv
bool cElection<int, 8, 8>::Init() {
    for (int voter = 0; voter != 8; ++voter) {
        voters_[voter].Init();
    }
    return true;
}

// IDA_EA: 0x85D6C70
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE4VoteEii
cElection<int, 8, 8>* cElection<int, 8, 8>::Vote(int voter, int candidate) {
    voters_[voter].votes_[candidate] = 1;
    return this;
}

// IDA_EA: 0x85D6C8C
// IDA_NAME: _ZNK9cElectionIiLi8ELi8EE10GetElectedEv
int cElection<int, 8, 8>::GetElected() const {
    int vote_count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int voter = 0; voter != 8; ++voter) {
        for (int candidate = 0; candidate != 8; ++candidate) {
            vote_count[candidate] += voters_[voter].votes_[candidate];
        }
    }
    int elected = -1;
    for (int candidate = 0; candidate != 8; ++candidate) {
        if (vote_count[candidate] > 4) {
            elected = candidate;
            break;
        }
    }
    return elected;
}

// IDA_EA: 0x85D6D88
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE15CancelCandidateEi
bool cElection<int, 8, 8>::CancelCandidate(int candidate) {
    for (int voter = 0; voter != 8; ++voter) {
        voters_[voter].votes_[candidate] = 0;
    }
    return true;
}

// IDA_EA: 0x85D6DBE
// IDA_NAME: _ZN9cElectionIiLi8ELi8EE11CancelVoterEi
cElection<int, 8, 8>::cVoter*
cElection<int, 8, 8>::CancelVoter(int voter) {
    return voters_[voter].Init();
}

// IDA_EA: 0x85DF8E2
// IDA_NAME: _ZN20CDeathMatchBattleMgr5ResetEv
CDeathMatchBattleMgr* CDeathMatchBattleMgr::Reset() {
    std::memset(kill_count_, 0, sizeof(kill_count_));
    std::memset(death_count_, 0, sizeof(death_count_));
    return this;
}

// IDA_EA: 0x85DF90A
// IDA_NAME: _ZN20CDeathMatchBattleMgr9SetParentEP8PvP_Room
CDeathMatchBattleMgr* CDeathMatchBattleMgr::SetParent(PvP_Room* parent) {
    parent_ = parent;
    return this;
}

// IDA_EA: 0x85D6012
// IDA_NAME: _ZN8PvP_RoomC2Ev
PvP_Room::PvP_Room()
    : guild_war_log_(),
      election_(),
      user_table_(),
      recv_pvp_rank_(),
      recv_pvp_end_result_(),
      mutex_(),
      matching_system_(new CMatchingSystem()),
      current_match_(0) {
    std::fill(pvp_timer_keys_, pvp_timer_keys_ + 15, 0);
    std::fill(users_, users_ + 8, static_cast<CUser*>(0));
    current_match_ = matching_system_->GetMatchingSystem(
        ENUM_PVP_MATCHING_NORMAL);
    rematchable_ = 0;
}

// IDA_EA: 0x85D61A6
// IDA_NAME: _ZN8PvP_RoomD2Ev
PvP_Room::~PvP_Room() {
    channel_zombie_matching_authority::OnRoomReset(this);
    zombie_mode::ResetRoom(this);
    delete matching_system_;
}
