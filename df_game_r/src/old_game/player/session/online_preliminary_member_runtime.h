#ifndef OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MEMBER_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MEMBER_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/player/core/charac_expand_cdata.h"


class CUser;

// IDA payload used by CStreamGuard::GetInBuffer at 0x8586982 and by
// COnlinePreliminaryMember::_saveData at 0x8585FD6.  The old stream
// specialization reserves exactly 0x14 bytes.
struct SIG_ONLINE_PRELIMINARY_MEMBER {
    uint16_t server_group;  // IDA +0x00
    uint16_t raw_02;        // IDA +0x02; not written by _saveData
    int32_t charac_no;      // IDA +0x04
    float rating;           // IDA +0x08
    int32_t win_count;      // IDA +0x0c
    int32_t lose_count;     // IDA +0x10
};

static_assert(sizeof(SIG_ONLINE_PRELIMINARY_MEMBER) == 0x14u,
              "SIG_ONLINE_PRELIMINARY_MEMBER IDA ABI size must stay 0x14");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER, charac_no) == 0x04u,
              "SIG_ONLINE_PRELIMINARY_MEMBER charac_no offset must stay 0x04");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER, rating) == 0x08u,
              "SIG_ONLINE_PRELIMINARY_MEMBER rating offset must stay 0x08");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER, win_count) == 0x0cu,
              "SIG_ONLINE_PRELIMINARY_MEMBER win_count offset must stay 0x0c");
static_assert(offsetof(SIG_ONLINE_PRELIMINARY_MEMBER, lose_count) == 0x10u,
              "SIG_ONLINE_PRELIMINARY_MEMBER lose_count offset must stay 0x10");

namespace online_preliminary {

// Canonical CData slot 4 persisted state.  IDA's CDataMgr constructor calls
// operator new(0x14) at 0x832A870 and installs the object in slots_[4]
// (+0x10) at 0x832A8A2.
class COnlinePreliminaryMember : public charac_expand::CData {
public:
    COnlinePreliminaryMember();
    virtual ~COnlinePreliminaryMember();

    virtual bool loadData(CUser* user, char* data);

    float GetRating() const;
    charac_expand::CData* IncreaseWinCount();
    charac_expand::CData* IncreaseLoseCount();
    charac_expand::CData* SetRating(float rating);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    float rating_;       // IDA +0x08
    int32_t win_count_;  // IDA +0x0c
    int32_t lose_count_; // IDA +0x10
};

static_assert(sizeof(COnlinePreliminaryMember) == 0x14u,
              "COnlinePreliminaryMember IDA ABI size must stay 0x14");

}  // namespace online_preliminary

#endif  // OLDSRV_OLD_GAME_USERINFO_ONLINE_PRELIMINARY_MEMBER_RUNTIME_H_
