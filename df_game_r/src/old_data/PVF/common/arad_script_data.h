#ifndef OLDSRV_DATAMANAGER_ARAD_SCRIPT_DATA_H_
#define OLDSRV_DATAMANAGER_ARAD_SCRIPT_DATA_H_

#include <stdint.h>
#include <time.h>

#include <map>
#include <string>
#include <vector>

// The original scripts use this enum as a four-byte value.  The individual
// names are not present in the recovered type library; parser labels select
// the original numeric values in arad_script_data.cpp.
namespace AVATAR_HIDDEN_ABILITY_TYPE {
enum T {
    NONE = 0
};
}

// IDA type: 06_userinfo_stat_inventory/STAvatarHiddenAbilityVariation.
// This is data owned by the avatar PVF parser, even though the game runtime
// consumes the parsed entries.
struct STAvatarHiddenAbilityVariation {
    AVATAR_HIDDEN_ABILITY_TYPE::T ability_type;
    bool is_float;
    float float_value;
    int int_value;
    int variation;

    STAvatarHiddenAbilityVariation(AVATAR_HIDDEN_ABILITY_TYPE::T type,
                                   int value,
                                   int variation_value);
    STAvatarHiddenAbilityVariation(AVATAR_HIDDEN_ABILITY_TYPE::T type,
                                   float value,
                                   int variation_value);
    ~STAvatarHiddenAbilityVariation();
};

namespace ARAD {
namespace SCRIPT {

struct _AradJumpingRewardItem {
    int charac_job;
    int growth_value;
    unsigned long reward_item_index;

    ~_AradJumpingRewardItem();
};

class AradJumpingScriptData
    : public std::vector<_AradJumpingRewardItem> {
public:
    AradJumpingScriptData();
    ~AradJumpingScriptData();
};

class AradJumping_Script : public AradJumpingScriptData {
public:
    AradJumping_Script();
    ~AradJumping_Script();

    static int importScript(char* filename, AradJumping_Script& script);
    bool getCurRewardItem(int charac_job,
                          int growth_value,
                          unsigned long& reward_item_index);
    AradJumping_Script* getRDARScriptData();
};

class AvatarFixedHiddenOption {
public:
    AvatarFixedHiddenOption();
    ~AvatarFixedHiddenOption();

    int loadScript();
    static int parseAbility(
        std::vector<STAvatarHiddenAbilityVariation>& abilities,
        std::string& type);
    bool clearScript();
    int getMaterialItemIndex();
    bool isMaterialItemIndex(int item_index);

    std::vector<STAvatarHiddenAbilityVariation> ability_lists[6];
    int material_item_index;
};

class AvatarRecharge {
public:
    AvatarRecharge();
    ~AvatarRecharge();

    int loadScript();
    int clearScript();
    int getAvatarRecharageLimit();
    std::map<int, int>* getEmblemRechargeStoneInfo();
    int getEmblemRecharge(int grade);
    bool IsEmblemRechargeStone(int item_index);

    int avatar_recharge_limit;
    std::map<int, int> emblem_recharge_stone_info;
    std::map<int, int> emblem_recharge;
    std::map<int, int> recharge_use_emblem_info;
    bool is_loaded;
};

class EmblemCompound {
public:
    struct Result {
        uint16_t list_index;
        uint16_t result_grade;
        uint16_t item_count;
        uint16_t success_rate;
        bool enabled;
    };

    EmblemCompound();
    ~EmblemCompound();

    int loadScript();
    int clearScript();
    int getCost(int grade);
    Result* getResult(int grade);
    std::vector<unsigned long>* getList(int grade);

    std::map<int, int> costs;
    std::map<int, Result*> results;
    std::vector<unsigned long> emblem_lists[7];
};

class AvatarConvert {
public:
    struct GetInfo {
        int item_index;
        int16_t item_count;
        int16_t random_weight;

        ~GetInfo();
    };

    struct ConvertResult {
        int grade;
        int total_random_weight;
        std::vector<GetInfo> result_items;
        std::vector<int> cumulative_random_weights;
        bool is_select;

        ConvertResult();
        ~ConvertResult();
    };

    AvatarConvert();
    ~AvatarConvert();

    int loadScript();
    int clearScript();
    ConvertResult* getConvertResult(int grade);

    std::map<int, ConvertResult*> results;
};

struct SEventPeriodData {
    int event_code;
    unsigned int start_time;
    unsigned int end_time;

    SEventPeriodData(int event_code_value,
                     unsigned int start_time_value,
                     unsigned int end_time_value);
    bool operator==(const SEventPeriodData& other);
    ~SEventPeriodData();
};

struct SRewardPeriodData {
    int event_code;
    unsigned long item_index;
    int16_t item_count;
    unsigned int start_time;
    unsigned int end_time;

    SRewardPeriodData(int event_code_value,
                      unsigned long item_index_value,
                      int16_t item_count_value,
                      unsigned int start_time_value,
                      unsigned int end_time_value);
    bool operator==(const SRewardPeriodData& other);
    ~SRewardPeriodData();
};

bool readTime(int16_t* value);
time_t uint32Time(int16_t* value);
int importAradEventScript(std::vector<SEventPeriodData>& event_periods,
                          std::vector<SRewardPeriodData>& reward_periods);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STAvatarHiddenAbilityVariation) == 0x14,
              "STAvatarHiddenAbilityVariation ABI");
static_assert(sizeof(AvatarFixedHiddenOption) == 0x4c,
              "AvatarFixedHiddenOption ABI");
static_assert(sizeof(AvatarRecharge) == 0x50, "AvatarRecharge ABI");
static_assert(sizeof(EmblemCompound::Result) == 0x0a,
              "EmblemCompound::Result ABI");
static_assert(sizeof(EmblemCompound) == 0x84, "EmblemCompound ABI");
static_assert(sizeof(AvatarConvert::GetInfo) == 0x08,
              "AvatarConvert::GetInfo ABI");
static_assert(sizeof(AvatarConvert::ConvertResult) == 0x24,
              "AvatarConvert::ConvertResult ABI");
static_assert(sizeof(AvatarConvert) == 0x18, "AvatarConvert ABI");
static_assert(sizeof(SEventPeriodData) == 0x0c, "SEventPeriodData ABI");
static_assert(sizeof(SRewardPeriodData) == 0x14,
              "SRewardPeriodData ABI");
static_assert(sizeof(_AradJumpingRewardItem) == 0x0c,
              "_AradJumpingRewardItem ABI");
static_assert(sizeof(AradJumpingScriptData) == 0x0c,
              "AradJumpingScriptData ABI");
#endif

}  // namespace SCRIPT
}  // namespace ARAD

#endif  // OLDSRV_DATAMANAGER_ARAD_SCRIPT_DATA_H_
