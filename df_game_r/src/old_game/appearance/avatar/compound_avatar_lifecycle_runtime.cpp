#include "old_game/appearance/avatar/compound_avatar.h"

#include "common/config.h"
#include "common/manager/mt_rand.h"

#include <ctime>

namespace WongWork {

// IDA_EA: 0x8331D74
// IDA_NAME: _ZN8WongWork15CCompoundAvatarC2Ev
// DEMANGLED: WongWork::CCompoundAvatar::CCompoundAvatar(void)
CCompoundAvatar::CCompoundAvatar() : tables_(), random_(0) {
    const unsigned long seed =
        static_cast<unsigned long>(std::time(static_cast<time_t*>(0)));
    random_ = new CMTRand(seed);
    for (int index = 0; index != 100; ++index) {
        const unsigned long limit = 10000;
        random_->randInt(limit);
    }
}

// IDA_EA: 0x8331EA2
// IDA_NAME: _ZN8WongWork15CCompoundAvatarD2Ev
// DEMANGLED: WongWork::CCompoundAvatar::~CCompoundAvatar()
CCompoundAvatar::~CCompoundAvatar() {
    delete random_;
    random_ = 0;
}

// IDA_EA: 0x8333EA6
// IDA_NAME: _ZN8WongWork15CCompoundAvatar10LoadScriptEPKcS2_
// DEMANGLED: WongWork::CCompoundAvatar::LoadScript(char const*,char const*)
bool CCompoundAvatar::LoadScript(const char* directory,
                                 const char* filename) {
    return ::importCompoundAvatarTable(directory, filename, &tables_[0]) == 0;
}

// IDA_EA: 0x8334344
// IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfoC2Ev
// DEMANGLED: WongWork::CCompoundAvatar::tagCompoundInfo::tagCompoundInfo(void)
CCompoundAvatar::tagCompoundInfo::tagCompoundInfo()
    : first_avatar(),
      second_avatar(),
      recipe(),
      avatar_mutations() {}

// IDA_EA: 0x8334388
// IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfoD2Ev
// DEMANGLED: WongWork::CCompoundAvatar::tagCompoundInfo::~tagCompoundInfo()
CCompoundAvatar::tagCompoundInfo::~tagCompoundInfo() {}

// IDA_EA: 0x833424A
// IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfo5clearEv
// DEMANGLED: WongWork::CCompoundAvatar::tagCompoundInfo::clear(void)
void CCompoundAvatar::tagCompoundInfo::clear() {
    character_job = oldsrv::config::kCharacterJobInvalid;
    compound_grade = 0;
    first_avatar_slot = 0;
    first_avatar.reset();
    second_avatar_slot = 0;
    second_avatar.reset();
    first_avatar_prototype = 0;
    second_avatar_prototype = 0;
    recipe_slot = 0;
    recipe.reset();
    requested_result_item = 0;
    requested_avatar_option = 100;
    upper_compound = false;
    invalid_request = false;
    new_avatar_slot = 0xffffu;
    avatar_mutations.clear();
    compound_kind = 0;
}

}  // namespace WongWork
