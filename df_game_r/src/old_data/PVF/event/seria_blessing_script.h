#ifndef OLDSRV_OLD_DATA_PVF_EVENT_SERIA_BLESSING_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_SERIA_BLESSING_SCRIPT_H_

#include <map>
#include <stdint.h>
#include <utility>


// IDA_EA: 0x822AF22, 0x837CE24, 0x837CE38, 0x8693AD6, 0x822AEBE, 0x822AECC, 0x8693AA0, 0x891063B.
// IDA units:
// - 12_pvf_datamanager_scripts/stSeriaBlessingScript
// - 12_pvf_datamanager_scripts/stSeriaBlessingScript_Value
// - 12_pvf_datamanager_scripts/stSeriaBlessingScript_FindOneByLv
class stSeriaBlessingScript {
public:
    struct Value {
        Value();

        int32_t min_level;
        int32_t max_level;
        int32_t value_08;
        int32_t value_0c;
        int32_t value_10;
    };

    struct FindOneByLv {
        explicit FindOneByLv(int32_t charac_level);

        bool operator()(const std::pair<const int, Value>& entry) const;

        int32_t charac_level;
    };

    stSeriaBlessingScript();
    ~stSeriaBlessingScript();

    int is_possible_lv(int32_t charac_level);
    int get_value(int32_t charac_level, Value& value);

    std::map<int, Value> values;
};

int ImportSeriaBlessingScript(const char* filename,
                              stSeriaBlessingScript& script);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_SERIA_BLESSING_SCRIPT_H_
