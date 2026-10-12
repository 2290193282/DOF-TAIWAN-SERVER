#ifndef OLDSRV_DATAMANAGER_TITLE_BOOK_INFO_H_
#define OLDSRV_DATAMANAGER_TITLE_BOOK_INFO_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>


struct stTitleRewardBasis;

// IDA_EA: 0x828D926, 0x828F43E, 0x8AA95AE, 0x8AA9E2A.
struct stTitleElement {
    stTitleElement();
    ~stTitleElement();
    stTitleElement(const stTitleElement& other);

    stTitleElement& operator=(const stTitleElement& other);

    int32_t title_index;
    std::vector<int> condition_values;
    int32_t value_10;
    int32_t value_14;
    std::string value_18;
    int32_t condition_type;
    int32_t value_20;
    int32_t value_24;
};

// IDA_EA: 0x828D972, 0x828DAC2, 0x8AA979E, 0x8AAA242.
struct stTitleSection {
    stTitleSection();
    ~stTitleSection();
    stTitleSection(const stTitleSection& other);

    stTitleSection& operator=(const stTitleSection& other);

    int32_t category;
    std::string section_name;
    std::string value_08;
    int32_t start_slot;
    int32_t end_slot;
    std::string name;
    std::string image_path;
    int32_t image_frame;
    std::string effect_path;
    std::string section_icon;
    int32_t section_icon_frame;
    int32_t frame_type;
    std::string lock_message;
    std::vector<stTitleRewardBasis*> rewards;
};

// IDA_EA: 0x83E629C, 0x8AA9928, 0x8AAA67C, 0x8AAA736.
struct stCustomConditionMsg {
    stCustomConditionMsg();
    ~stCustomConditionMsg();
    stCustomConditionMsg(const stCustomConditionMsg& other);

    stCustomConditionMsg& operator=(const stCustomConditionMsg& other);

    int32_t category;
    int32_t condition_id;
    std::string message;
};

// IDA_EA: 0x8AA9906, 0x8AABE5E.
struct stAdjust {
    stAdjust();
    ~stAdjust();

    int32_t title_index;
    int32_t adjust_type;
    int32_t adjust_value;
};

// IDA_EA: 0x837D828, 0x837D906.
// CDataManager embeds stTitleBookInfo at +0xA788 in the old server.
struct stTitleBookInfo {
    stTitleBookInfo();
    ~stTitleBookInfo();

    std::vector<int> default_reward_values;
    std::vector<stTitleSection> sections;
    std::map<int, std::vector<stTitleElement> > title_collections;
    std::vector<stCustomConditionMsg> custom_condition_messages;
    std::vector<stAdjust> adjusts;
    std::vector<stTitleRewardBasis*> default_rewards;
    uint8_t view_titlebook_eventmark;
    uint8_t padding_55[3];
};

int convertTabTypeStringToEnum(const char* text);
int importRewards(std::vector<stTitleRewardBasis*>* rewards,
                  std::string reward_tag);
int importTitleSection(stTitleSection* section, int section_index);
std::vector<stTitleElement>* getTitleCollection(stTitleBookInfo* info,
                                                int category);
int importTitleBookEtcScript(const char* filename, stTitleBookInfo* info);

#endif  // OLDSRV_DATAMANAGER_TITLE_BOOK_INFO_H_
