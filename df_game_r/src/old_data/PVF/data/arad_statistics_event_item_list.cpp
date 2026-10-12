#include "old_data/PVF/data/arad_statistics_event_item_list.h"

#include <algorithm>

namespace ARAD {

// IDA_EA: 0x819CBE8
// IDA_NAME: _ZN4ARAD28Arad_StatisticsEventItemListC2Ev
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::Arad_StatisticsEventItemList(void)
Arad_StatisticsEventItemList::Arad_StatisticsEventItemList()
    : Arad_InterfaceGameScript("Arad_StatisticsEventItemList"),
      item_indexes_() {}

// IDA_EA: 0x819CCAC
// IDA_NAME: _ZN4ARAD28Arad_StatisticsEventItemListD2Ev
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::~Arad_StatisticsEventItemList()
Arad_StatisticsEventItemList::~Arad_StatisticsEventItemList() {}

// IDA_EA: 0x819CD0E
// IDA_NAME: _ZN4ARAD28Arad_StatisticsEventItemListD0Ev
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::~Arad_StatisticsEventItemList()
// The virtual destructor above emits this deleting-destructor entry.

// IDA_EA: 0x819CD2C
// IDA_NAME: _ZN4ARAD28Arad_StatisticsEventItemList14loadScriptDataEv
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::loadScriptData(void)
bool Arad_StatisticsEventItemList::loadScriptData() {
    static const unsigned long kStatisticItemIndexes[] = {
        2675626u, 2675620u, 26710u,   26711u,   26712u,   26713u,
        26714u,   26715u,   26716u,   7576u,    2675765u, 2675766u,
        2675767u, 2675768u, 2675769u, 7573u,    2675021u, 2674093u,
        2674092u, 2675847u, 2675903u, 2770012u, 2771612u, 2781222u,
        2778012u, 2773212u, 2774812u, 2782816u, 2776419u, 2779613u,
    };

    for (unsigned int index = 0;
         index < sizeof(kStatisticItemIndexes) /
                     sizeof(kStatisticItemIndexes[0]);
         ++index) {
        item_indexes_.push_back(kStatisticItemIndexes[index]);
    }
    return true;
}

// IDA_EA: 0x819D084
// IDA_NAME: _ZN4ARAD28Arad_StatisticsEventItemList15clearScriptDataEv
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::clearScriptData(void)
void Arad_StatisticsEventItemList::clearScriptData() {
    item_indexes_.clear();
}

// IDA_EA: 0x819D09A
// IDA_NAME: _ZNK4ARAD28Arad_StatisticsEventItemList14needStatisticsEm
// DEMANGLED: ARAD::Arad_StatisticsEventItemList::needStatistics(unsigned long)const
bool Arad_StatisticsEventItemList::needStatistics(
    unsigned long item_index) const {
    return std::find(item_indexes_.begin(), item_indexes_.end(), item_index) !=
           item_indexes_.end();
}

}  // namespace ARAD
