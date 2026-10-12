#ifndef OLDSRV_DATAMANAGER_ARAD_REWARD_FUNCTOR_H_
#define OLDSRV_DATAMANAGER_ARAD_REWARD_FUNCTOR_H_

#include "old_data/PVF/arad/datamanager/arad_event_reward.h"

#include <stdint.h>

class CUser;

namespace ARAD {

// IDA type: ARAD::FnRewardInterface (0x0c on i386).  Its only virtual slot is
// the RDAR-backed mail text provider; it intentionally has no virtual dtor.
class FnRewardInterface {
public:
    FnRewardInterface(CUser* user, bool character_scoped);

    virtual char* GetMailString(char* text, char* title) = 0;

    int operator()(int event_code,
                   unsigned long item_index,
                   int16_t item_count,
                   uint32_t expire_time);
    EventRewardItemArad make_data(int event_code);

protected:
    CUser* user_;
    bool character_scoped_;
    uint8_t reserved_09_[3];
};

// IDA type: ARAD::FnRewardLethe (0x04 on i386).  This is intentionally a
// separate non-polymorphic functor in the original event reward path.
class FnRewardLethe {
public:
    explicit FnRewardLethe(CUser* user);

    int operator()(int event_code,
                   unsigned long item_index,
                   int16_t item_count,
                   uint32_t expire_time);
    EventRewardItemArad make_data(int event_code);

private:
    CUser* user_;
};

// IDA type: ARAD::FnRewardMomoji.  It reuses the interface's save and mail
// flow, with only its real RDAR title/body keys differing.
class FnRewardMomoji : public FnRewardInterface {
public:
    explicit FnRewardMomoji(CUser* user);

    virtual char* GetMailString(char* text, char* title);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(FnRewardInterface) == 0x0cu,
              "FnRewardInterface IDA ABI size must stay 0x0c");
static_assert(sizeof(FnRewardLethe) == 0x04u,
              "FnRewardLethe IDA ABI size must stay 0x04");
static_assert(sizeof(FnRewardMomoji) == 0x0cu,
              "FnRewardMomoji IDA ABI size must stay 0x0c");
#endif

}  // namespace ARAD

#endif  // OLDSRV_DATAMANAGER_ARAD_REWARD_FUNCTOR_H_
