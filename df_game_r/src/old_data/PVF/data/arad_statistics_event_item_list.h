#ifndef OLDSRV_OLD_DATA_PVF_DATA_ARAD_STATISTICS_EVENT_ITEM_LIST_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ARAD_STATISTICS_EVENT_ITEM_LIST_H_

#include "old_data/PVF/data/arad_data_manager.h"

#include <vector>

namespace ARAD {

// IDA type: ARAD::Arad_StatisticsEventItemList (0x14 on i386).  The values
// are the fixed old-server event-item table recovered from loadScriptData.
class Arad_StatisticsEventItemList : public Arad_InterfaceGameScript {
public:
    Arad_StatisticsEventItemList();
    virtual ~Arad_StatisticsEventItemList();

    virtual bool loadScriptData();
    virtual void clearScriptData();
    bool needStatistics(unsigned long item_index) const;

private:
    std::vector<unsigned long> item_indexes_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_StatisticsEventItemList) == 0x14u,
              "Arad_StatisticsEventItemList IDA ABI size must stay 0x14");
#endif

}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ARAD_STATISTICS_EVENT_ITEM_LIST_H_
