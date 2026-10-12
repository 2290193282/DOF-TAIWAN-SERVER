#ifndef OLDSRV_OLD_DATA_PVF_ETC_EMOTION_TYPE_TAGS_H_
#define OLDSRV_OLD_DATA_PVF_ETC_EMOTION_TYPE_TAGS_H_

#include <stdint.h>

#include <string>
#include <vector>

struct stOneEmotionData {
    stOneEmotionData();
    ~stOneEmotionData();
    stOneEmotionData(const stOneEmotionData& other);

    int clear();
    stOneEmotionData& operator=(const stOneEmotionData& other);

    int32_t value_00;
    int32_t value_04;
    std::string value_08;
    std::string value_0c;
    bool value_10;
    bool value_11;
    bool value_12;
    char padding_13;
    std::string value_14;
    std::vector<std::string> values_18;
};

struct stEmotionTypeTagsInfo {
    stEmotionTypeTagsInfo();
    ~stEmotionTypeTagsInfo();

    int clear();

    std::vector<int> emotion_types;
    std::vector<stOneEmotionData> emotion_data;
};

int importEmotionTypeTagsScript(const char* filename,
                                stEmotionTypeTagsInfo* info);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_EMOTION_TYPE_TAGS_H_
