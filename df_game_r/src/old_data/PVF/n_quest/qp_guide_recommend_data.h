#ifndef OLDSRV_OLD_DATA_PVF_N_QUEST_QP_GUIDE_RECOMMEND_DATA_H_
#define OLDSRV_OLD_DATA_PVF_N_QUEST_QP_GUIDE_RECOMMEND_DATA_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

class stQPGuideScript {
public:
    struct stRecommendQPData {
        stRecommendQPData();
        ~stRecommendQPData();
        stRecommendQPData(const stRecommendQPData& other);

        stRecommendQPData& operator=(const stRecommendQPData& other);
        bool clear();

        int32_t value_00;
        int32_t value_04;
        std::vector<int> values;
    };

    std::vector<std::vector<stRecommendQPData> > recommendations;
    std::map<int, std::string> tooltip_mappings;
};

static_assert(sizeof(stQPGuideScript) == 0x24u,
              "stQPGuideScript IDA ABI size must stay 0x24");

// IDA_EA: 0x890FF93
// IDA_NAME: _Z19importQPGuideScriptPKcR15stQPGuideScript
// DEMANGLED: importQPGuideScript(char const*,stQPGuideScript &)
int importQPGuideScript(const char* filename, stQPGuideScript& script);

#endif  // OLDSRV_OLD_DATA_PVF_N_QUEST_QP_GUIDE_RECOMMEND_DATA_H_
