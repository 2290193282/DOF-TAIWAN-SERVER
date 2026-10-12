#ifndef OLDSRV_OLD_DATA_PVF_EVENT_BINGO_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_BINGO_SCRIPT_H_

#include <map>
#include <stdint.h>
#include <utility>
#include <vector>


// IDA boundary: CDataManager embedded Bingo script table at +0xB568.
// Runtime BingoEvent/CBingo consumers remain 11_event_runtime-owned.
// IDA_EA: 0x83715B6, 0x837DDFC. Old i386 size: 0x54.
class stBingoScript {
public:
    stBingoScript();
    ~stBingoScript();

    std::map<int, int> quiz_values;
    std::map<int, std::vector<int> > boards;
    std::map<int, std::pair<int, int> > rewards;
    std::vector<int> use_items;
};

int ImportBingoScript(const char* filename, stBingoScript& script);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_BINGO_SCRIPT_H_
