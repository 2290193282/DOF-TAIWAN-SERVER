#include "old_game/appearance/avatar/avatar_roulette.h"

#include <cstddef>
#include <stdint.h>

#include "common/config.h"

typedef char AvatarRoulette_Size_Must_Be_0x134[
    (sizeof(AvatarRoulette) == 0x134) ? 1 : -1];

// IDA_EA: 0x837E484
// IDA_NAME: _ZN14AvatarRouletteD2Ev
// DEMANGLED: AvatarRoulette::~AvatarRoulette()
AvatarRoulette::~AvatarRoulette() {}

// IDA_EA: 0x8911B98
// IDA_NAME: _ZN14AvatarRouletteC2Ev
// DEMANGLED: AvatarRoulette::AvatarRoulette(void)
AvatarRoulette::AvatarRoulette() {
    clear();
}

// IDA_EA: 0x8911CF6
// IDA_NAME: _ZN14AvatarRoulette5clearEv
// DEMANGLED: AvatarRoulette::clear(void)
bool AvatarRoulette::clear() {
    for (int job = 0;; ++job) {
        const bool result = job < oldsrv::config::kCharacterJobCount;
        if (job >= oldsrv::config::kCharacterJobCount) {
            return result;
        }

        for (size_t index = 0; index < avatar_lists_by_job[job].size();
             ++index) {
            avatar_lists_by_job[job][index].clear();
        }
        avatar_lists_by_job[job].clear();
    }
}

// IDA_EA: 0x8911DA0
// IDA_NAME: _ZN14AvatarRoulette12isAvatarCoinEm
// DEMANGLED: AvatarRoulette::isAvatarCoin(unsigned long)
bool AvatarRoulette::isAvatarCoin(unsigned long item_no) {
    return avatar_coin_item_no == item_no;
}

// IDA_EA: 0x8911DB0
// IDA_NAME: _ZN14AvatarRoulette13getAvatarListERb17ENUM_CHARACTERJOBRSt6vectorIS2_ISt4pairImiESaIS4_EESaIS6_EE
// DEMANGLED: AvatarRoulette::getAvatarList(bool &,ENUM_CHARACTERJOB,std::vector<std::vector<std::pair<unsigned long,int> > > &)
bool* AvatarRoulette::getAvatarList(bool& result,
                                    ENUM_CHARACTERJOB character_job,
                                    AvatarRateGroup& out) {
    const int job = static_cast<int>(character_job);
    result = false;
    bool* return_value = reinterpret_cast<bool*>(static_cast<intptr_t>(job));
    if (job >= 0 && job < oldsrv::config::kCharacterJobCount) {
        out = avatar_lists_by_job[job];
        result = true;
        return_value = &result;
    }
    return return_value;
}

// IDA_EA: 0x8911DFE
// IDA_NAME: _ZN14AvatarRoulette13getStikerListERb17ENUM_CHARACTERJOBRSt6vectorIiSaIiEE
// DEMANGLED: AvatarRoulette::getStikerList(bool &,ENUM_CHARACTERJOB,std::vector<int> &)
bool* AvatarRoulette::getStikerList(bool& result,
                                    ENUM_CHARACTERJOB character_job,
                                    std::vector<int>& out) {
    const int job = static_cast<int>(character_job);
    result = false;
    bool* return_value = reinterpret_cast<bool*>(static_cast<intptr_t>(job));
    if (job >= 0 && job < oldsrv::config::kCharacterJobCount) {
        out = sticker_lists_by_job[job];
        result = true;
        return_value = &result;
    }
    return return_value;
}

// IDA_EA: 0x8911E4E
// IDA_NAME: _ZN14AvatarRoulette4swapEPS_
// DEMANGLED: AvatarRoulette::swap(AvatarRoulette *)
int AvatarRoulette::swap(AvatarRoulette* other) {
    avatar_coin_item_no = other->avatar_coin_item_no;
    rare_clone_avatar_rate = other->rare_clone_avatar_rate;
    rare_avatar_rate = other->rare_avatar_rate;
    upper_avatar_rate = other->upper_avatar_rate;
    normal_avatar_rate = other->normal_avatar_rate;
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        avatar_lists_by_job[job].swap(other->avatar_lists_by_job[job]);
    }
    return 1;
}
