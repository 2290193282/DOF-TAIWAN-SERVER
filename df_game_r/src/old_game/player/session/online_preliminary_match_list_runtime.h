#ifndef OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MATCH_LIST_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MATCH_LIST_RUNTIME_H_

#include <map>
#include <stddef.h>
#include <stdint.h>

#include "old_game/activity/schedule/character_event_records.h"
#include "old_game/player/core/charac_expand_cdata.h"


class CUser;

struct SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY {
    uint16_t server_group;  // IDA +0x00
    uint16_t raw_02;        // IDA +0x02; not written by _saveData
    uint32_t charac_no;     // IDA +0x04
    uint32_t play_count;    // IDA +0x08
};

// IDA payload used by CStreamGuard::GetInBuffer at 0x8586CB4 and by
// COnlinePreliminaryMemberMatchList::{loadData,_saveData}.  The old stream
// specialization reserves exactly 0x4BC bytes.
struct SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST {
    uint16_t server_group;  // IDA +0x000
    uint16_t raw_02;        // IDA +0x002; not written by _saveData
    uint32_t charac_no;     // IDA +0x004
    uint32_t count;         // IDA +0x008
    SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY entries[100]; // +0x00C
};

static_assert(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY) == 0x0cu,
              "online preliminary match-list entry ABI must stay 0x0C");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY,
                       charac_no) == 0x04u,
              "online preliminary match-list charac_no offset must stay 0x04");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST_ENTRY,
                       play_count) == 0x08u,
              "online preliminary match-list play_count offset must stay 0x08");
static_assert(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST) == 0x4bcu,
              "SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST ABI must stay 0x4BC");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST, count) ==
                  0x08u,
              "online preliminary match-list count offset must stay 0x08");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER_MATCH_LIST, entries) ==
                  0x0cu,
              "online preliminary match-list entries offset must stay 0x0C");

namespace online_preliminary {

struct PlayCountInfo {
    uint32_t play_count;  // IDA +0x00
    bool dirty;           // IDA +0x04
    uint8_t raw_05[3];    // ABI alignment, untouched by the old constructors

    PlayCountInfo();
    PlayCountInfo(uint32_t play_count_value, bool dirty_value);
};

static_assert(sizeof(PlayCountInfo) == 0x08u,
              "online_preliminary::PlayCountInfo ABI must stay 0x08");
static_assert(offsetof(PlayCountInfo, dirty) == 0x04u,
              "online_preliminary::PlayCountInfo dirty offset must stay 0x04");

// Canonical CDataMgr slot 5 persisted state.  The old CDataMgr constructor
// allocates 0x20 bytes and installs this object in slots_[5].
class COnlinePreliminaryMemberMatchList : public charac_expand::CData {
public:
    COnlinePreliminaryMemberMatchList();
    virtual ~COnlinePreliminaryMemberMatchList();

    virtual void ResetDailyMidnight();
    virtual bool loadData(CUser* user, char* data);

    int GetPeerPlayCount(int server_group, uint32_t charac_no) const;
    // IDA 0x8586208 returns the CData dirty-state handle from alter().
    // COnlinePreliminary forwards that result through its own update chain.
    charac_expand::CData* IncreasePeerPlayCount(int server_group,
                                                uint32_t charac_no);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    std::map<CharacterInServer, PlayCountInfo> play_counts_; // IDA +0x08
};

#if defined(__i386__)
static_assert(sizeof(COnlinePreliminaryMemberMatchList) == 0x20u,
              "COnlinePreliminaryMemberMatchList ABI must stay 0x20");
#endif

}  // namespace online_preliminary

#endif  // OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MATCH_LIST_RUNTIME_H_
