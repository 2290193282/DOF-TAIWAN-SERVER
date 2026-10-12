// Canonical CParty physical-subobject lifecycle provider.  The C2/D2 bodies
// are activated with StaticPool<CParty,600>; constructing a pool node must
// construct every named CParty subobject rather than a placeholder extent.

#include "old_game/social/party/party.h"
#include "old_game/dungeon/instance/instant_reinforce.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/dungeon/mission/dungeon_mission.h"

#include <algorithm>

// IDA_EA: 0x82B5846
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE6cVoterC2Ev
// DEMANGLED: cElection<int,4,4>::cVoter::cVoter(void)
cElection<int, 4, 4>::cVoter::cVoter() {
    Init();
}

// IDA_EA: 0x82BEB62
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE6cVoter4InitEv
// DEMANGLED: cElection<int,4,4>::cVoter::Init(void)
cElection<int, 4, 4>::cVoter* cElection<int, 4, 4>::cVoter::Init() {
    std::fill(votes_, votes_ + 4, 0);
    return this;
}

// IDA_EA: 0x82AAFC4
// IDA_NAME: _ZN9cElectionIiLi4ELi4EEC2Ev
// DEMANGLED: cElection<int,4,4>::cElection(void)
cElection<int, 4, 4>::cElection() {
    Init();
}

// IDA_EA: 0x82B585A
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE4InitEv
// DEMANGLED: cElection<int,4,4>::Init(void)
bool cElection<int, 4, 4>::Init() {
    for (int voter = 0; voter != 4; ++voter) {
        voters_[voter].Init();
    }
    return true;
}

// IDA_EA: 0x85C038C
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE4VoteEii
// DEMANGLED: cElection<int,4,4>::Vote(int,int)
cElection<int, 4, 4>* cElection<int, 4, 4>::Vote(int voter,
                                                   int candidate) {
    voters_[voter].votes_[candidate] = 1;
    return this;
}

// IDA_EA: 0x85C03A8
// IDA_NAME: _ZNK9cElectionIiLi4ELi4EE10GetElectedEv
// DEMANGLED: cElection<int,4,4>::GetElected(void)const
int cElection<int, 4, 4>::GetElected() const {
    int vote_count[4] = {0, 0, 0, 0};
    for (int voter = 0; voter != 4; ++voter) {
        for (int candidate = 0; candidate != 4; ++candidate) {
            vote_count[candidate] += voters_[voter].votes_[candidate];
        }
    }

    int elected = -1;
    int high_vote_count = 0;
    for (int candidate = 0; candidate != 4; ++candidate) {
        if (vote_count[candidate] > high_vote_count) {
            high_vote_count = vote_count[candidate];
            elected = candidate;
        }
    }
    return elected;
}

// IDA_EA: 0x85C047C
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE15CancelCandidateEi
// DEMANGLED: cElection<int,4,4>::CancelCandidate(int)
bool cElection<int, 4, 4>::CancelCandidate(int candidate) {
    for (int voter = 0; voter != 4; ++voter) {
        voters_[voter].votes_[candidate] = 0;
    }
    return true;
}

// IDA_EA: 0x85C04B2
// IDA_NAME: _ZN9cElectionIiLi4ELi4EE11CancelVoterEi
// DEMANGLED: cElection<int,4,4>::CancelVoter(int)
cElection<int, 4, 4>::cVoter* cElection<int, 4, 4>::CancelVoter(
    int voter) {
    return voters_[voter].Init();
}

// IDA_EA: 0x8279D30
// IDA_NAME: _ZN18SecKillMonsterInfoC2Ejjjj
// DEMANGLED: SecKillMonsterInfo::SecKillMonsterInfo(unsigned int,unsigned int,unsigned int,unsigned int)
SecKillMonsterInfo::SecKillMonsterInfo(unsigned int monster_id,
                                       unsigned int kill_count,
                                       unsigned int hp_total,
                                       unsigned int exp_total)
    : monster_id_(monster_id),
      kill_count_(kill_count),
      hp_total_(hp_total),
      exp_total_(exp_total),
      hp_outlier_count_(0),
      exp_outlier_count_(0) {}

// IDA_EA: 0x82A4C4A
// IDA_NAME: _ZN24Secu_DungeonAverageCheckC2Ev
// DEMANGLED: Secu_DungeonAverageCheck::Secu_DungeonAverageCheck(void)
Secu_DungeonAverageCheck::Secu_DungeonAverageCheck() : kill_monsters_() {
    // Old C2 constructs the vector at +0x04 before it stores the null party
    // pointer at +0x00.  Do not use a `party_(0)` initializer: declaration
    // order would write the pointer before the vector constructor.
    party_ = 0;
}

// IDA_EA: 0x82A4C6A
// IDA_NAME: _ZN24Secu_DungeonAverageCheckD2Ev
// DEMANGLED: Secu_DungeonAverageCheck::~Secu_DungeonAverageCheck()
Secu_DungeonAverageCheck::~Secu_DungeonAverageCheck() {}

// IDA_EA: 0x82794A6
// IDA_NAME: _ZN24Secu_DungeonAverageCheck11SetPartyPtrEP6CParty
// DEMANGLED: Secu_DungeonAverageCheck::SetPartyPtr(CParty *)
void Secu_DungeonAverageCheck::SetPartyPtr(CParty* party) {
    party_ = party;
    reset();
}

// IDA_EA: 0x82794C2
// IDA_NAME: _ZN24Secu_DungeonAverageCheck12startDungeonEv
// DEMANGLED: Secu_DungeonAverageCheck::startDungeon(void)
void Secu_DungeonAverageCheck::startDungeon() {
    reset();
}

// IDA_EA: 0x82A4C80
// IDA_NAME: _ZN24Secu_HackLogCheckByPartyC2Ev
// DEMANGLED: Secu_HackLogCheckByParty::Secu_HackLogCheckByParty(void)
Secu_HackLogCheckByParty::Secu_HackLogCheckByParty() : party_for_check_(0) {}

// IDA_EA: 0x82A4C9E
// IDA_NAME: _ZN24Secu_HackLogCheckByPartyD2Ev
// DEMANGLED: Secu_HackLogCheckByParty::~Secu_HackLogCheckByParty()
Secu_HackLogCheckByParty::~Secu_HackLogCheckByParty() {}

// IDA_EA: 0x827A908
// IDA_NAME: _ZN24Secu_HackLogCheckByParty4InitEP6CParty
// DEMANGLED: Secu_HackLogCheckByParty::Init(CParty *)
void Secu_HackLogCheckByParty::Init(CParty* party) {
    party_for_check_ = party;
    SetPartyPtr(party);
}

// IDA_EA: 0x827A92C
// IDA_NAME: _ZN24Secu_HackLogCheckByParty12startDungeonEv
// DEMANGLED: Secu_HackLogCheckByParty::startDungeon(void)
// The derived entry is only a non-virtual forward to the embedded base
// detector; it must not reset party_for_check_ or create a second tracker.
void Secu_HackLogCheckByParty::startDungeon() {
    Secu_DungeonAverageCheck::startDungeon();
}

// IDA_EA: 0x82A5516
// IDA_NAME: _ZN6CPartyC2Ev
// DEMANGLED: CParty::CParty(void)
CParty::CParty() {
    time_spiral::ResetParty(this);
    dungeon_mission::ResetParty(this);
    for (int timer_message = 11; timer_message <= 47; ++timer_message) {
        timer_keys_[timer_message - 11] = 0;
    }
    used_coin_count_ = 0;
    battle_field_.SetParty(this);
    SetAssaultState(0);
    routing_data_.Reset();
    party_teleport_.init(this);
    gm_random_buff_type_ = 11;
    quick_party_ = 0;
    init_quick_party_data();
    secu_.Init(this);
    party_member_coin_limit_ = 0;
}

// IDA_EA: 0x82A57FE
// IDA_NAME: _ZN6CPartyD2Ev
// DEMANGLED: CParty::~CParty()
CParty::~CParty() {
    time_spiral::ResetParty(this);
    instant_reinforce::ResetParty(this);
    dungeon_mission::ResetParty(this);
}
