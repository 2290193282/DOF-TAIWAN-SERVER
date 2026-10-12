#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_ROULETTE_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_ROULETTE_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "common/config.h"

// IDA_EA: 0x8911DB0 / 0x8911DFE reference this global enum by name in the
// old C++ ABI. The public owned enum definition is still requested from A.
enum ENUM_CHARACTERJOB : int;

// IDA unit: 06_userinfo_stat_inventory/AvatarRoulette.
// This class intentionally stays in the global namespace because B-owned
// CDataManager init currently consumes the old global C++ symbols directly.
class AvatarRoulette {
public:
    typedef std::pair<unsigned long, int> AvatarRatePair;
    typedef std::vector<AvatarRatePair> AvatarRateList;
    typedef std::vector<AvatarRateList> AvatarRateGroup;

    // IDA_EA: 0x8911B98.
    AvatarRoulette();
    // IDA_EA: 0x837E484.
    ~AvatarRoulette();

    bool clear();
    bool isAvatarCoin(unsigned long item_no);
    bool* getAvatarList(bool& result,
                        ENUM_CHARACTERJOB character_job,
                        AvatarRateGroup& out);
    bool* getStikerList(bool& result,
                        ENUM_CHARACTERJOB character_job,
                        std::vector<int>& out);
    int swap(AvatarRoulette* other);

    uint32_t avatar_coin_item_no;
    uint32_t rare_clone_avatar_rate;
    uint32_t rare_avatar_rate;
    uint32_t upper_avatar_rate;
    uint32_t normal_avatar_rate;
    AvatarRateGroup
        avatar_lists_by_job[oldsrv::config::kCharacterJobCount];
    std::vector<int>
        sticker_lists_by_job[oldsrv::config::kCharacterJobCount];
};

#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_ROULETTE_H_
