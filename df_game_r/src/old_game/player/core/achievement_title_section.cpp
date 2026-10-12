#include "old_game/player/core/achievement_title_section.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/etc/title_book_info.h"

#include <stdint.h>

stTitleBookInfo* GetTitleBookInfo() {
    return G_CDataManager()->get_title_book_info();
}

// IDA_EA: 0x828BCB8
// IDA_NAME: _ZN12CAchievement14_getSectionIdxE24ENUM_TITLE_BOOK_CATEGORYi
// DEMANGLED: CAchievement::_getSectionIdx(ENUM_TITLE_BOOK_CATEGORY,int)
int CAchievement::_getSectionIdx(ENUM_TITLE_BOOK_CATEGORY category,
                                 int slot) {
    const std::vector<stTitleSection>& sections = GetTitleBookInfo()->sections;
    for (std::vector<stTitleSection>::size_type index = 0;
         index < sections.size();
         ++index) {
        const stTitleSection section(sections[index]);
        if (section.category == static_cast<int>(category) &&
            section.start_slot <= slot && section.end_slot >= slot) {
            return static_cast<int>(index);
        }
    }
    return -1;
}
