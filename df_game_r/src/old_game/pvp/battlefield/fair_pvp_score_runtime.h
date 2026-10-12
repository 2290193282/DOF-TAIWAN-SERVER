#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_SCORE_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_SCORE_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/charac_expand_cdata.h"

class CUser;
class PacketGuard;

// IDA type used by CFairPvPScore's map value.  The three dwords are copied
// directly to/from each 16-byte job-score record in SIG_FAIR_PVP_SCORE.
struct FAIR_PVP_SCORE_MODE_COUNT {
    int win;
    int lose;
    int draw;
};

static_assert(sizeof(FAIR_PVP_SCORE_MODE_COUNT) == 0x0cu,
              "FAIR_PVP_SCORE_MODE_COUNT IDA ABI size must stay 0x0c");

// IDA payload consumed by CFairPvPScore::loadData @0x84A48F8 and produced by
// _saveData @0x84A57FE.  Stream/CStreamGuard reserve exactly 0x688 bytes at
// 0x84A6E98/0x84A67F2.
struct SIG_FAIR_PVP_SCORE {
    struct JobScoreRecord {
        uint8_t job;
        uint8_t grow_type;
        uint8_t reserved_02[2];
        FAIR_PVP_SCORE_MODE_COUNT score;
    };

    struct JobScoreBlob {
        uint8_t count;
        uint8_t reserved_001[3];
        JobScoreRecord records[100];
        uint8_t recent_results[10];
        uint8_t reserved_64e[2];
    };

    uint32_t charac_no;                         // +0x000
    FAIR_PVP_SCORE_MODE_COUNT mode_totals[2];   // +0x004
    uint32_t relay_two_kill;                    // +0x01c
    uint32_t relay_three_kill;                  // +0x020
    uint32_t successive_win;                    // +0x024
    uint32_t maximum_successive_win;            // +0x028
    JobScoreBlob job_score;                     // +0x02c
    uint32_t daily_play_count;                  // +0x67c
    int32_t last_play_time;                     // +0x680
    uint8_t give_item;                          // +0x684
    uint8_t reserved_685[3];
};

static_assert(sizeof(SIG_FAIR_PVP_SCORE::JobScoreRecord) == 0x10u,
              "fair PvP job-score record IDA ABI size must stay 0x10");
static_assert(sizeof(SIG_FAIR_PVP_SCORE::JobScoreBlob) == 0x650u,
              "fair PvP job-score blob IDA ABI size must stay 0x650");
static_assert(offsetof(SIG_FAIR_PVP_SCORE::JobScoreBlob, records) == 0x04u,
              "fair PvP job-score records offset must stay 0x04");
static_assert(
    offsetof(SIG_FAIR_PVP_SCORE::JobScoreBlob, recent_results) == 0x644u,
    "fair PvP recent-result offset must stay 0x644");
static_assert(sizeof(SIG_FAIR_PVP_SCORE) == 0x688u,
              "SIG_FAIR_PVP_SCORE IDA ABI size must stay 0x688");
static_assert(offsetof(SIG_FAIR_PVP_SCORE, job_score) == 0x02cu,
              "SIG_FAIR_PVP_SCORE job_score offset must stay 0x02c");
static_assert(offsetof(SIG_FAIR_PVP_SCORE, daily_play_count) == 0x67cu,
              "SIG_FAIR_PVP_SCORE daily_play_count offset must stay 0x67c");
static_assert(offsetof(SIG_FAIR_PVP_SCORE, last_play_time) == 0x680u,
              "SIG_FAIR_PVP_SCORE last_play_time offset must stay 0x680");
static_assert(offsetof(SIG_FAIR_PVP_SCORE, give_item) == 0x684u,
              "SIG_FAIR_PVP_SCORE give_item offset must stay 0x684");

namespace fair_pvp {

// IDA unit: 09_dungeon_pvp_party/fair_pvp/CFairPvPScore.
// CDataMgr slot 7 owns this persisted fair-PvP score state.
class CFairPvPScore : public charac_expand::CData {
public:
    // IDA_EA: 0x84A47DC
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScoreC2Ev
    // DEMANGLED: fair_pvp::CFairPvPScore::CFairPvPScore(void)
    CFairPvPScore();

    // IDA_EA: 0x84A4850
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScoreD2Ev
    // DEMANGLED: fair_pvp::CFairPvPScore::~CFairPvPScore()
    // IDA_EA: 0x84A48DA
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScoreD0Ev
    // DEMANGLED: fair_pvp::CFairPvPScore::~CFairPvPScore()
    virtual ~CFairPvPScore();

    // IDA_EA: 0x84A48F8
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore8loadDataEP5CUserPc
    // DEMANGLED: fair_pvp::CFairPvPScore::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x84A55A8
    // IDA_NAME: _ZNK8fair_pvp13CFairPvPScore7getDataEPc
    // DEMANGLED: fair_pvp::CFairPvPScore::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x84A645A
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore10ResetDailyEv
    // DEMANGLED: fair_pvp::CFairPvPScore::ResetDaily(void)
    virtual void ResetDaily();

    // IDA_EA: 0x8234F64
    // IDA_NAME: _ZNK8fair_pvp13CFairPvPScore11GetGiveItemEv
    // DEMANGLED: fair_pvp::CFairPvPScore::GetGiveItem(void)const
    int GetGiveItem() const;

    // IDA_EA: 0x8234F74
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore11SetGiveItemEi
    // DEMANGLED: fair_pvp::CFairPvPScore::SetGiveItem(int)
    void SetGiveItem(int give_item);

    // IDA_EA: 0x8234F92
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore17SetDailyPlayCountEi
    // DEMANGLED: fair_pvp::CFairPvPScore::SetDailyPlayCount(int)
    void SetDailyPlayCount(int play_count);

    // IDA_EA: 0x8234FAE
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore19SetRemainBonusCountEi
    // DEMANGLED: fair_pvp::CFairPvPScore::SetRemainBonusCount(int)
    void SetRemainBonusCount(int remain_count);

    // IDA_EA: 0x84A5A28
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore11UpdateScoreEiiiiiii
    // DEMANGLED: fair_pvp::CFairPvPScore::UpdateScore(int,int,int,int,int,int,int)
    void UpdateScore(int job,
                     int grow_type,
                     int mode,
                     int win,
                     int lose,
                     int draw,
                     int relay_kill_count);

    // IDA_EA: 0x84A5CC2
    // IDA_NAME: _ZNK8fair_pvp13CFairPvPScore15MakePacketSocreER11PacketGuardi
    // DEMANGLED: fair_pvp::CFairPvPScore::MakePacketSocre(PacketGuard &,int)const
    void MakePacketSocre(PacketGuard& packet, int detail_mode) const;

    // IDA_EA: 0x8568EC2
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore17GetDailyPlayCountEv
    // DEMANGLED: fair_pvp::CFairPvPScore::GetDailyPlayCount(void)
    int GetDailyPlayCount();

    // IDA_EA: 0x8568ECE
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore24SetLastTrainingSealCountEi
    // DEMANGLED: fair_pvp::CFairPvPScore::SetLastTrainingSealCount(int)
    void SetLastTrainingSealCount(int count);

    // IDA_EA: 0x8568EDE
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore29SetLastTrainingSealBonusCountEi
    // DEMANGLED: fair_pvp::CFairPvPScore::SetLastTrainingSealBonusCount(int)
    void SetLastTrainingSealBonusCount(int count);

    // IDA_EA: 0x85DFDAC
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore24GetLastTrainingSealCountEv
    // DEMANGLED: fair_pvp::CFairPvPScore::GetLastTrainingSealCount(void)
    int GetLastTrainingSealCount();

    // IDA_EA: 0x85DFDBC
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore29GetLastTrainingSealBonusCountEv
    // DEMANGLED: fair_pvp::CFairPvPScore::GetLastTrainingSealBonusCount(void)
    int GetLastTrainingSealBonusCount();

    // IDA_EA: 0x85DFDCC
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore19GetRemainBonusCountEv
    // DEMANGLED: fair_pvp::CFairPvPScore::GetRemainBonusCount(void)
    int GetRemainBonusCount();

protected:
    // IDA_EA: 0x84A57FE
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore9_saveDataEP5CUser
    // DEMANGLED: fair_pvp::CFairPvPScore::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x84A5942
    // IDA_NAME: _ZN8fair_pvp13CFairPvPScore6_resetEv
    // DEMANGLED: fair_pvp::CFairPvPScore::_reset(void)
    virtual int _reset();

private:
    typedef std::map<JobKey, FAIR_PVP_SCORE_MODE_COUNT> JobScoreMap;

    JobScoreMap job_score_;                       // IDA +0x08
    std::list<uint8_t> recent_results_;            // IDA +0x20
    FAIR_PVP_SCORE_MODE_COUNT mode_totals_[2];     // IDA +0x28
    int relay_two_kill_;                           // IDA +0x40
    int relay_three_kill_;                         // IDA +0x44
    int successive_win_;                          // IDA +0x48
    int maximum_successive_win_;                  // IDA +0x4c
    int daily_play_count_;                        // IDA +0x50
    int last_play_time_;                          // IDA +0x54
    uint8_t last_training_seal_count_;            // IDA +0x58
    uint8_t last_training_seal_bonus_count_;      // IDA +0x59
    uint8_t remain_bonus_count_;                  // IDA +0x5a
    uint8_t give_item_;                           // IDA +0x5b
};

static_assert(sizeof(CFairPvPScore) == 0x5cu,
              "CFairPvPScore IDA ABI size must stay 0x5c");

}  // namespace fair_pvp

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_SCORE_RUNTIME_H_
