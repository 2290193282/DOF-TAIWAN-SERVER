#ifndef OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_PARAMETER_SCRIPT_H_

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <vector>

#include "old_data/PVF/common/script_file_list.h"
#include "old_data/PVF/etc/independent_drop_types.h"


// IDA_EA: IndependentDropListInfo @0x897CBAE/@0x83DD4B4.
struct IndependentDropListInfo {
    IndependentDropListInfo();
    IndependentDropListInfo(const IndependentDropListInfo& other);
    ~IndependentDropListInfo();
    IndependentDropListInfo& operator=(const IndependentDropListInfo& other);

    int32_t file_index;
    std::vector<std::pair<int, int> > drops;
};

// IDA_EA: stIndependentDropInfo @0x897CBDA/@0x83DD4E4.
struct stIndependentDropInfo {
    stIndependentDropInfo();
    stIndependentDropInfo(const stIndependentDropInfo& other);
    ~stIndependentDropInfo();
    stIndependentDropInfo& operator=(const stIndependentDropInfo& other);

    int32_t dungeon_index;
    int32_t start_level;
    stMonsterIndependentDrop_t drop;
};

// IDA_EA: stIndependentDropRateControl @0x897CBF0/@0x83DD514.
struct stIndependentDropRateControl {
    stIndependentDropRateControl();
    stIndependentDropRateControl(const stIndependentDropRateControl& other);
    ~stIndependentDropRateControl();
    stIndependentDropRateControl& operator=(
        const stIndependentDropRateControl& other);

    int32_t dungeon_index;
    int32_t rate_min;
    int32_t rate_max;
    stMonsterIndependentDrop_t drop;
};

// IDA_EA: IndependentDropParameterScript @0x8372D46/@0x8372DFA/@0x837DAAC/@0x897C0D8/@0x897C1C6/@0x897C398.
class IndependentDropParameterScript {
public:
    IndependentDropParameterScript();
    ~IndependentDropParameterScript();

    void clear();
    int load_independent_drop_list();
    int importIndependentDropParameterScript(const char* filename);
    int ImportIndependentDrop(const char* filename);

    // IDA consumers address these four contiguous container members directly;
    // no getter exists in df_game_r. Keep the proven shared layout public so
    // consumers do not reproduce member offsets.
    std::vector<IndependentDropListInfo> independent_drop_list_;
    std::vector<stIndependentDropInfo> independent_drop_info_;
    std::vector<stIndependentDropRateControl> rate_control_;
    std::map<int, std::pair<int, int> > file_index_range_;
};

// IDA_EA: initIndependentDropScript @0x897C01C.
int initIndependentDropScript(char* src, char* filename);

// IDA_EA: 0x94F7840 / 0x94F7940.
extern char g_independentDropScriptBaseDirectory[256];
extern STScriptFileList g_independentDropScriptFileList;

static_assert(sizeof(IndependentDropListInfo) == 0x10u,
              "IndependentDropListInfo IDA ABI size must stay 0x10");
static_assert(sizeof(stIndependentDropInfo) == 0x50u,
              "stIndependentDropInfo IDA ABI size must stay 0x50");
static_assert(sizeof(stIndependentDropRateControl) == 0x54u,
              "stIndependentDropRateControl IDA ABI size must stay 0x54");
static_assert(sizeof(IndependentDropParameterScript) == 0x3cu,
              "IndependentDropParameterScript IDA ABI size must stay 0x3c");
static_assert(offsetof(IndependentDropParameterScript,
                       independent_drop_list_) == 0x00u,
              "IndependentDrop list vector must stay at +0x00");
static_assert(offsetof(IndependentDropParameterScript,
                       independent_drop_info_) == 0x0cu,
              "IndependentDrop info vector must stay at +0x0C");
static_assert(offsetof(IndependentDropParameterScript, rate_control_) == 0x18u,
              "IndependentDrop rate vector must stay at +0x18");
static_assert(offsetof(IndependentDropParameterScript, file_index_range_) ==
                  0x24u,
              "IndependentDrop range map must stay at +0x24");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_INDEPENDENT_DROP_PARAMETER_SCRIPT_H_
