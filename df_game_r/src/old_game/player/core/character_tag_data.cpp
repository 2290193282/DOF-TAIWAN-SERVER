#include "old_game/player/core/character_tag_data.h"

#include <cstring>


typedef char TAG_CHARAC_DATA_Size_Must_Be_0x78[
    (sizeof(TAG_CHARAC_DATA) == 0x78) ? 1 : -1];
typedef char TAG_CHARAC_INVEN_DATA_Size_Must_Be_0x40b0[
    (sizeof(TAG_CHARAC_INVEN_DATA) == 0x40b0) ? 1 : -1];
typedef char TAG_CHARAC_ETC_DATA_Size_Must_Be_0x330[
    (sizeof(TAG_CHARAC_ETC_DATA) == 0x330) ? 1 : -1];
typedef char CHARAC_QUEST_SHOP_DATA_Size_Must_Be_0xaa[
    (sizeof(CHARAC_QUEST_SHOP_DATA) == 0xaa) ? 1 : -1];

// IDA_EA: 0x822ED84
// IDA_NAME: _ZN15TAG_CHARAC_DATA5clearEv
// DEMANGLED: TAG_CHARAC_DATA::clear(void)
TAG_CHARAC_DATA* TAG_CHARAC_DATA::clear() {
    std::memset(this, 0, 0x78u);
    skill_tree_index = -1;
    return this;
}

// IDA_EA: 0x822EDAE
// IDA_NAME: _ZN15TAG_CHARAC_DATAC2Ev
// DEMANGLED: TAG_CHARAC_DATA::TAG_CHARAC_DATA(void)
TAG_CHARAC_DATA::TAG_CHARAC_DATA() : additional_info() {
    clear();
}

// IDA_EA: 0x822EDD0
// IDA_NAME: _ZN21TAG_CHARAC_INVEN_DATA5clearEv
// DEMANGLED: TAG_CHARAC_INVEN_DATA::clear(void)
void* TAG_CHARAC_INVEN_DATA::clear() {
    return std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x848F774
// IDA_NAME: _ZN21TAG_CHARAC_INVEN_DATAC2Ev
// DEMANGLED: TAG_CHARAC_INVEN_DATA::TAG_CHARAC_INVEN_DATA(void)
TAG_CHARAC_INVEN_DATA::TAG_CHARAC_INVEN_DATA() {
    clear();
}

// IDA_EA: 0x822EDF4
// IDA_NAME: _ZN19TAG_CHARAC_ETC_DATA5clearEv
// DEMANGLED: TAG_CHARAC_ETC_DATA::clear(void)
void* TAG_CHARAC_ETC_DATA::clear() {
    return std::memset(this, 0, 0x330u);
}

// IDA_EA: 0x848F7DC
// IDA_NAME: _ZN19TAG_CHARAC_ETC_DATAC2Ev
// DEMANGLED: TAG_CHARAC_ETC_DATA::TAG_CHARAC_ETC_DATA(void)
TAG_CHARAC_ETC_DATA::TAG_CHARAC_ETC_DATA() {
    clear();
}

// IDA_EA: 0x848F83E
// IDA_NAME: _ZN22CHARAC_QUEST_SHOP_DATA5clearEv
// DEMANGLED: CHARAC_QUEST_SHOP_DATA::clear(void)
void* CHARAC_QUEST_SHOP_DATA::clear() {
    charac_no = 0;
    quest_point = 0;
    init_count = 0;
    for (unsigned int index = 0;
         index < sizeof(buying_count) / sizeof(buying_count[0]);
         ++index) {
        buying_count[index] = 0;
    }
    quest_piece = 0;
    return this;
}

// IDA_EA: 0x848F862
// IDA_NAME: _ZN22CHARAC_QUEST_SHOP_DATAC2Ev
// DEMANGLED: CHARAC_QUEST_SHOP_DATA::CHARAC_QUEST_SHOP_DATA(void)
CHARAC_QUEST_SHOP_DATA::CHARAC_QUEST_SHOP_DATA() {
    clear();
}
