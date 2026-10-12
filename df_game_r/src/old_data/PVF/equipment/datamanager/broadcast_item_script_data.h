#ifndef OLDSRV_DATAMANAGER_BROADCAST_ITEM_SCRIPT_DATA_H_
#define OLDSRV_DATAMANAGER_BROADCAST_ITEM_SCRIPT_DATA_H_

#include <map>
#include <stdint.h>
#include <vector>


// IDA_EA: 0x8371694, 0x837DE92, 0x8918CDC, 0x8918D9C, 0x8918DFE, 0x8918F24.
// IDA unit: 12_pvf_datamanager_scripts/stBroadCastItemScript.
class stBroadCastItemScript {
public:
    stBroadCastItemScript();
    ~stBroadCastItemScript();

    int checkCommonItem(int32_t item_id) const;
    int isUnCommonItem(int32_t item_id) const;
    int checkUnCommonNestedItem(int32_t item_id, int32_t nested_item_id) const;

    std::vector<int> common_items;
    std::map<int, std::vector<int> > uncommon_items;
};

int ImportBroadCastItemScript(const char* filename,
                              stBroadCastItemScript& script);

#endif  // OLDSRV_DATAMANAGER_BROADCAST_ITEM_SCRIPT_DATA_H_
