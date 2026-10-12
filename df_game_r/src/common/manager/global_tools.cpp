#include "common/manager/global_tools.h"

#include "common/config.h"
#include "common/manager/character_script_public_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_data/db/core/mysql.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/play_info_public_abi.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <cstdarg>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <execinfo.h>
#include <cstring>
#include <cwchar>
#include <ctime>
#include <map>

// IDA_DATA_EA: 0x93F4D40
const char* CHARACTER_JOB_TAG_NAMES[
    oldsrv::config::kCharacterJobWithCommonCount] = {
    "[swordman]",
    "[fighter]",
    "[gunner]",
    "[mage]",
    "[priest]",
    "[at gunner]",
    "[thief]",
    "[at fighter]",
    "[at mage]",
    "[demonic swordman]",
    "[creator mage]",
    "[at swordman]",
    "[all]",
};


// IDA_DATA_EA: 0x9370180
unsigned int CREATECHARACTEREQUIP[
    oldsrv::config::kCharacterJobCount * 2]
    __asm__("CREATECHARACTEREQUIP") = {
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
    10400u, 12400u,
};
// IDA_DATA_EA: 0x949B298
int g_new_scriptVersion = -1;

// IDA_DATA_EA: 0x948B2B0
int MAX_VILLAGE_NUM = -1;

// IDA_EA: 0x89BC76E
// IDA_NAME: _Z27getEquipmentTypeToEnumValueRSs
// DEMANGLED: getEquipmentTypeToEnumValue(std::string &)
int getEquipmentTypeToEnumValue(std::string& text) {
    using namespace oldsrv::config;
    if (text == "hat avatar") return kEquipmentTypeHatAvatar;
    if (text == "hair avatar") return kEquipmentTypeHairAvatar;
    if (text == "face avatar") return kEquipmentTypeFaceAvatar;
    if (text == "coat avatar") return kEquipmentTypeCoatAvatar;
    if (text == "pants avatar") return kEquipmentTypePantsAvatar;
    if (text == "shoes avatar") return kEquipmentTypeShoesAvatar;
    if (text == "breast avatar") return kEquipmentTypeBreastAvatar;
    if (text == "waist avatar") return kEquipmentTypeWaistAvatar;
    if (text == "skin avatar") return kEquipmentTypeSkinAvatar;
    if (text == "aurora avatar") return kEquipmentTypeAuroraAvatar;
    if (text == "weapon avatar") return kEquipmentTypeWeaponAvatar;
    if (text == "weapon") return kEquipmentTypeWeapon;
    if (text == "title name") return kEquipmentTypeTitle;
    if (text == "coat") return kEquipmentTypeCoat;
    if (text == "shoulder") return kEquipmentTypeShoulder;
    if (text == "pants") return kEquipmentTypePants;
    if (text == "shoes") return kEquipmentTypeShoes;
    if (text == "waist") return kEquipmentTypeWaist;
    if (text == "amulet") return kEquipmentTypeAmulet;
    if (text == "wrist") return kEquipmentTypeWrist;
    if (text == "ring") return kEquipmentTypeRing;
    if (text == "support") return kEquipmentTypeSupport;
    if (text == "magic stone") return kEquipmentTypeMagicStone;
    return kEquipmentTypeCreature;
}

// IDA_EA: 0x8A9AFA2
// IDA_NAME: _Z22getSkillSpendSPOnStealii
int getSkillSpendSPOnSteal(int spend, int penalty_percent) {
    const int computed = static_cast<int>(
        static_cast<long double>(spend) *
        (static_cast<long double>(penalty_percent) / 100.0L));
    return computed > 1 ? computed : 1;
}

// IDA_NAME: _ZL15s_bodyImagePath
std::map<ENUM_CHARACTERJOB, std::string> s_bodyImagePath
    __asm__("_ZL15s_bodyImagePath");

// IDA_NAME: upgradeWeaponAttackPowerRate_
float upgradeWeaponAttackPowerRate_[
    oldsrv::config::kCharacterJobCount * 12] = {};

// IDA_NAME: g_weaponDurabilityDecreaseRate_
float g_weaponDurabilityDecreaseRate_[
    oldsrv::config::kCharacterJobCount * 6] = {};

namespace {

__thread char tstring_from_wide_buffers[10][0x1000];
__thread int tstring_from_wide_index = 0;
__thread wchar_t unicode_from_mbcs_buffers[10][0x1000];
__thread int unicode_from_mbcs_index = 0;

struct IgnoreCaseCompare {
    bool operator()(const std::string& left, const std::string& right) const {
        const size_t compare_size = left.size() < right.size()
            ? left.size() : right.size();
        for (size_t index = 0; index < compare_size; ++index) {
            const unsigned char left_ch = static_cast<unsigned char>(left[index]);
            const unsigned char right_ch = static_cast<unsigned char>(right[index]);
            const int folded_left = std::tolower(left_ch);
            const int folded_right = std::tolower(right_ch);
            if (folded_left != folded_right) {
                return folded_left < folded_right;
            }
        }
        return left.size() < right.size();
    }
};

typedef std::map<std::string, int, IgnoreCaseCompare> ObjectCategoryMap;
ObjectCategoryMap s_objCategoryString;

const char kInitializeCharacStatPrettyFunction[] =
    "static void global::tools::InitializeCharacStat::initCharacStat"
    "(_Additioal_info&, unsigned int, int, int, int, int)";

}  // namespace

// IDA_EA: 0x822AD3A
// IDA_NAME: _Z18GetLightServerFlagv
// DEMANGLED: GetLightServerFlag(void)
unsigned char* GetLightServerFlag() {
    static unsigned char light_server_flag = 0;
    return &light_server_flag;
}

// IDA_EA: 0x822AD44
// IDA_NAME: _Z13IsLightServerv
// DEMANGLED: IsLightServer(void)
int IsLightServer() {
    return *GetLightServerFlag();
}

// IDA_EA: 0x86B1D25
// IDA_NAME: _Z18get_seed_from_datev
// DEMANGLED: get_seed_from_date(void)
int get_seed_from_date() {
    const time_t timer = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    tm* const current_time = localtime_r(&timer, &local_time);
    return 30 * current_time->tm_mon + 365 * current_time->tm_year +
           current_time->tm_mday;
}

// IDA_EA: 0x8A8CBA5
// IDA_NAME: _Z18SetLightServerFlagb
// DEMANGLED: SetLightServerFlag(bool)
unsigned char* SetLightServerFlag(bool flag) {
    unsigned char* result = GetLightServerFlag();
    *result = static_cast<unsigned char>(flag);
    return result;
}

// IDA_EA: 0x8484A47
// IDA_NAME: _Z26GetCollectDungeonStatisticv
// DEMANGLED: GetCollectDungeonStatistic(void)
unsigned char* GetCollectDungeonStatistic() {
    static unsigned char collect_dungeon_statistic = 0;
    return &collect_dungeon_statistic;
}

// IDA_EA: 0x8484A51
// IDA_NAME: _Z25IsCollectDungeonStatisticv
// DEMANGLED: IsCollectDungeonStatistic(void)
int IsCollectDungeonStatistic() {
    return *GetCollectDungeonStatistic();
}

// IDA_EA: 0x8484A67
// IDA_NAME: _Z26SetCollectDungeonStatisticb
// DEMANGLED: SetCollectDungeonStatistic(bool)
unsigned char* SetCollectDungeonStatistic(bool flag) {
    unsigned char* result = GetCollectDungeonStatistic();
    *result = static_cast<unsigned char>(flag);
    return result;
}

// IDA_EA: 0x8115BF4
// IDA_NAME: _Z11filterEventi
// DEMANGLED: filterEvent(int)
int filterEvent(int event_id) {
    if (event_id == 2) {
        return 0;
    }

    GameWorld* world = G_GameWorld();
    const unsigned int channel_type =
        static_cast<unsigned int>(world->GetChannelType());
    if (channel_type > 0x10u) {
        return 0;
    }

    const int channel_mask = 1 << channel_type;
    if ((channel_mask & 0x1A100) != 0) {
        if (event_id > 57) {
            if (event_id >= 61 && (event_id <= 62 || event_id == 92)) {
                return 0;
            }
        } else if (event_id >= 54 || event_id == 11) {
            return 0;
        }
        return 1;
    }

    if ((channel_mask & 0x4000) == 0) {
        return (channel_mask & 0x600) != 0 && event_id != 9;
    }

    if (event_id != 57) {
        if (event_id > 57) {
            if (static_cast<unsigned int>(event_id - 61) > 1u) {
                return 1;
            }
        } else if (event_id != 11) {
            return 1;
        }
    }
    return 0;
}

namespace Global {

// IDA_EA: 0x81466F0
// IDA_NAME: _ZN6Global11CGlobalFunc15Get_State_CheckE8ch_state
// DEMANGLED: Global::CGlobalFunc::Get_State_Check(ch_state)
int CGlobalFunc::Get_State_Check(unsigned int state) {
    return state <= 0xDu && ((1u << state) & 0x3420u) != 0;
}

}  // namespace Global

// IDA_EA: 0x8ADEAB0
// IDA_NAME: _Z9toTStringRKSs
// DEMANGLED: toTString(std::string const&)
const char* toTString(const std::string& source) {
    return source.c_str();
}

// IDA_EA: 0x8ADEAC3
// IDA_NAME: _Z9toTStringRKSbIwSt11char_traitsIwESaIwEE
// DEMANGLED: toTString(std::basic_string<wchar_t> const&)
const char* toTString(const std::wstring& source) {
    const int index = tstring_from_wide_index;
    char* buffer = tstring_from_wide_buffers[index];
    const std::size_t converted =
        std::wcstombs(buffer, source.c_str(), 0x1000u);
    buffer[converted] = '\0';
    tstring_from_wide_index = (tstring_from_wide_index + 1) % 10;
    return buffer;
}

// IDA_EA: 0x8ADEB7D
// IDA_NAME: _Z9toTStringPKw
// DEMANGLED: toTString(wchar_t const*)
const char* toTString(const wchar_t* source) {
    const int index = tstring_from_wide_index;
    char* buffer = tstring_from_wide_buffers[index];
    const std::size_t converted = std::wcstombs(buffer, source, 0x1000u);
    buffer[converted] = '\0';
    tstring_from_wide_index = (tstring_from_wide_index + 1) % 10;
    return buffer;
}

// IDA_EA: 0x8ADF0F6
// IDA_NAME: _Z9toUnicodeRKSs
// DEMANGLED: toUnicode(std::string const&)
const wchar_t* toUnicode(const std::string& source) {
    const int index = unicode_from_mbcs_index;
    wchar_t* buffer = unicode_from_mbcs_buffers[index];
    const std::size_t converted =
        std::mbstowcs(buffer, source.c_str(), 0x1000u);
    buffer[converted] = L'\0';
    unicode_from_mbcs_index = (unicode_from_mbcs_index + 1) % 10;
    return buffer;
}

// IDA_EA: 0x8ADF1B3
// IDA_NAME: _Z9toUnicodePKc
// DEMANGLED: toUnicode(char const*)
const wchar_t* toUnicode(const char* source) {
    const int index = unicode_from_mbcs_index;
    wchar_t* buffer = unicode_from_mbcs_buffers[index];
    const std::size_t converted = std::mbstowcs(buffer, source, 0x1000u);
    buffer[converted] = L'\0';
    unicode_from_mbcs_index = (unicode_from_mbcs_index + 1) % 10;
    return buffer;
}

// IDA_EA: 0x8ADEE98
// IDA_NAME: _Z6toMbcsPKc
// DEMANGLED: toMbcs(char const*)
const char* toMbcs(const char* source) {
    return source;
}

// IDA_EA: 0x83FF968
// IDA_NAME: _Z15getList2inQueryPjPc
// DEMANGLED: getList2inQuery(unsigned int*, char*)
char* getList2inQuery(unsigned int* buf, char* dest) {
    unsigned int i;

    *dest = 0;
    std::memcpy(dest, "in (", 5u);
    for (i = 1; *buf > i; ++i) {
        std::sprintf(dest, "%s%d,", dest, buf[i]);
    }
    std::sprintf(dest, "%s%d)", dest, buf[i]);
    return dest;
}

// IDA_EA: 0x83FFA02
// IDA_NAME: _Z15getList2inQueryRKSt6vectorIiSaIiEEPc
// DEMANGLED: getList2inQuery(std::vector<int,std::allocator<int>> const&,char *)
char* getList2inQuery(const std::vector<int>& values, char* dest) {
    unsigned int index;

    *dest = 0;
    std::memcpy(dest, "in (", 5u);
    for (index = 0; values.size() - 1 > index; ++index) {
        std::sprintf(dest, "%s%d,", dest, values[index]);
    }
    std::sprintf(dest, "%s%d)", dest, values[index]);
    return dest;
}

// IDA_EA: 0x829F068
// IDA_NAME: _Z4trimRSs
// DEMANGLED: trim(std::string &)
int trim(std::string& text) {
    const std::string::size_type first_not_space =
        text.find_first_not_of(" ", 0);
    if (first_not_space != std::string::npos) {
        text.erase(0, first_not_space);
    }

    const std::string::size_type last_not_space =
        text.find_last_not_of(" ", std::string::npos);
    if (last_not_space != std::string::npos) {
        text.erase(last_not_space + 1, std::string::npos);
        return static_cast<int>(reinterpret_cast<uintptr_t>(&text));
    }
    return -1;
}

// IDA_EA: 0x829F0EF
// IDA_NAME: _Z4trimPc
// DEMANGLED: trim(char *)
char* trim(char* text) {
    char dest[1024];
    const int length = static_cast<int>(std::strlen(text));
    int begin = 0;
    for (int index = 0; index < length; ++index) {
        begin = index;
        if (static_cast<unsigned char>(text[index]) > 0x20u) {
            break;
        }
    }

    int end = length;
    for (int index = length - 1;
         (index >= 0 && end > begin) &&
         !(static_cast<unsigned char>(text[index]) > 0x20u);
         --index) {
        --end;
    }

    dest[end - begin] = '\0';
    std::memcpy(dest, text + begin, static_cast<std::size_t>(end - begin));
    std::strcpy(text, dest);
    return text;
}

// IDA_EA: 0x829EFAE
// IDA_NAME: _Z16checkSpecialCharPc
// DEMANGLED: checkSpecialChar(char *)
int checkSpecialChar(char* text) {
    CDataManager* data_manager = G_CDataManager();
    if (data_manager->getCharacNameFilterPass()) {
        G_CDataManager()->setCharacNameFilterPass(false);
        return 0;
    }

    if (!G_CDataManager()->hasSpecialCharacter(text)) {
        if (!G_CDataManager()->hasPreventString(text)) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x829F01F
// IDA_NAME: _Z20checkSpecialCharNamePc
// DEMANGLED: checkSpecialCharName(char *)
int checkSpecialCharName(char* text) {
    CDataManager* data_manager = G_CDataManager();
    if (data_manager->getCharacNameFilterPass()) {
        G_CDataManager()->setCharacNameFilterPass(false);
        return 0;
    }
    return G_CDataManager()->hasPreventStringName(text);
}

// IDA_EA: 0x85908D7
// IDA_NAME: _Z8LineFunciPKcij
// DEMANGLED: LineFunc(int,char const*,int,unsigned int)
int LineFunc(int line,
             const char* function,
             int error_no,
             unsigned int mid) {
    LogManager::logFormat(
        1,
        "PacketDispatcher.cpp",
        "int LineFunc(int, const char*, int, memberIdentificationNumber_t)",
        0x221,
        "return __LINE__:func(%s) line(%d) errNo(%d) mid(%u)",
        function,
        line,
        error_no,
        mid);
    return line;
}

// IDA_EA: 0x822A98C
// IDA_NAME: _Z22getAvatarEmblemTypeIdxt
// DEMANGLED: getAvatarEmblemTypeIdx(unsigned short)
int getAvatarEmblemTypeIdx(unsigned short type_mask) {
    if (type_mask > 0x10u) {
        return -1;
    }
    for (int index = 4; index >= 0; --index) {
        if ((type_mask >> index) == 1u) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x822A9E5
// IDA_NAME: _Z39getAvatarEmblemTypeIdxForCompountEmblemtPiRi
// DEMANGLED: getAvatarEmblemTypeIdxForCompountEmblem(unsigned short,int *,int &)
int getAvatarEmblemTypeIdxForCompountEmblem(unsigned short type_mask,
                                            int* type_counts,
                                            int& total_count) {
    bool found = false;
    for (int type = 0; type <= 4; ++type) {
        if ((type_mask & static_cast<unsigned short>(1u << type)) != 0u) {
            ++type_counts[type];
            ++total_count;
            found = true;
        }
    }
    return found ? 1 : -1;
}

// IDA_EA: 0x8332E4E
// IDA_NAME: _Z12getItemIndexRKSt6vectorISt4pairIiiESaIS1_EEi
// DEMANGLED: getItemIndex(std::vector<std::pair<int,int>> const&,int)
int getItemIndex(const std::vector<std::pair<int, int> >& items,
                 int sampled_weight) {
    for (std::vector<std::pair<int, int> >::const_iterator item =
             items.begin();
         item != items.end();
         ++item) {
        if (item->second > sampled_weight) {
            return item->first;
        }
    }
    return 0;
}

// IDA_EA: 0x8550FC9
// IDA_NAME: _Z24GetTenThousandPercentageii
// DEMANGLED: GetTenThousandPercentage(int,int)
int GetTenThousandPercentage(int value, int deduction_rate) {
    const uint32_t product_bits =
        static_cast<uint32_t>(deduction_rate) * static_cast<uint32_t>(value);
    const int32_t product = static_cast<int32_t>(product_bits);
    return product / 10000;
}

// IDA_EA: 0x8A86D28
// IDA_NAME: _Z28GetEquipmentTypeStringToEnumRSs
// DEMANGLED: GetEquipmentTypeStringToEnum(std::string &)
int GetEquipmentTypeStringToEnum(std::string& text) {
    using namespace oldsrv::config;
    if (text == "[hat avatar]") {
        return kEquipmentTypeHatAvatar;
    }
    if (text == "[hair avatar]") {
        return kEquipmentTypeHairAvatar;
    }
    if (text == "[face avatar]") {
        return kEquipmentTypeFaceAvatar;
    }
    if (text == "[coat avatar]") {
        return kEquipmentTypeCoatAvatar;
    }
    if (text == "[pants avatar]") {
        return kEquipmentTypePantsAvatar;
    }
    if (text == "[shoes avatar]") {
        return kEquipmentTypeShoesAvatar;
    }
    if (text == "[breast avatar]") {
        return kEquipmentTypeBreastAvatar;
    }
    if (text == "[waist avatar]") {
        return kEquipmentTypeWaistAvatar;
    }
    if (text == "[skin avatar]") {
        return kEquipmentTypeSkinAvatar;
    }
    if (text == "[aurora avatar]") {
        return kEquipmentTypeAuroraAvatar;
    }
    if (text == "[weapon avatar]") {
        return kEquipmentTypeWeaponAvatar;
    }
    if (text == "[weapon]") {
        return kEquipmentTypeWeapon;
    }
    if (text == "[title name]") {
        return kEquipmentTypeTitle;
    }
    if (text == "[coat]") {
        return kEquipmentTypeCoat;
    }
    if (text == "[shoulder]") {
        return kEquipmentTypeShoulder;
    }
    if (text == "[pants]") {
        return kEquipmentTypePants;
    }
    if (text == "[shoes]") {
        return kEquipmentTypeShoes;
    }
    if (text == "[waist]") {
        return kEquipmentTypeWaist;
    }
    if (text == "[amulet]") {
        return kEquipmentTypeAmulet;
    }
    if (text == "[wrist]") {
        return kEquipmentTypeWrist;
    }
    if (text == "[ring]") {
        return kEquipmentTypeRing;
    }
    if (text == "[support]") {
        return kEquipmentTypeSupport;
    }
    if (text == "[magic stone]") {
        return kEquipmentTypeMagicStone;
    }
    if (text == "[creature]") {
        return kEquipmentTypeCreature;
    }
    if (text == "[artifact red]") {
        return kEquipmentTypeArtifactRed;
    }
    if (text == "[artifact blue]") {
        return kEquipmentTypeArtifactBlue;
    }
    if (text == "[artifact green]") {
        return kEquipmentTypeArtifactGreen;
    }
    return kEquipmentTypeInvalid;
}

// IDA_EA: 0x88BE57E
// IDA_NAME: _Z15getCharacterJobPKc
// DEMANGLED: getCharacterJob(char const*)
int getCharacterJob(const char* text) {
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        if (std::strcmp(text, CHARACTER_JOB_TAG_NAMES[job]) == 0) {
            return job;
        }
    }
    return oldsrv::config::kCharacterJobCommon;
}

// IDA_EA: 0x88BE5C5
// IDA_NAME: _Z18checkValiedJobNamePKc
// DEMANGLED: checkValiedJobName(char const*)
int checkValiedJobName(const char* text) {
    for (int job = 0; job <= oldsrv::config::kCharacterJobCommon; ++job) {
        if (std::strcmp(text, CHARACTER_JOB_TAG_NAMES[job]) == 0) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x88BE60E
// IDA_NAME: _Z19getCharacterJobName17ENUM_CHARACTERJOB
// DEMANGLED: getCharacterJobName(ENUM_CHARACTERJOB)
char* getCharacterJobName(ENUM_CHARACTERJOB job) {
    static char buffer[17];
    std::strncpy(buffer, CHARACTER_JOB_TAG_NAMES[static_cast<int>(job)], 16);
    return buffer;
}

// IDA_EA: 0x88BE04C
// IDA_NAME: _Z19convertBinaryToTextPKc
// DEMANGLED: convertBinaryToText(char const*)
int convertBinaryToText(const char* source) {
    static char buffer[373761];
    FILE* input = std::fopen(toMbcs(source), "rb");
    int size = 0;
    for (;; ++size) {
        const int value = std::fgetc(input);
        if (value == -1 || size > 373759) {
            break;
        }
        if (value == 10 && (!size || buffer[size - 1] != 13)) {
            buffer[size++] = 13;
        }
        buffer[size] = static_cast<char>(value);
    }
    std::fclose(input);
    FILE* output = std::fopen(toMbcs(source), "wb");
    std::fwrite(buffer, size, 1u, output);
    return std::fclose(output);
}

// IDA_EA: 0x8A3DF64
// IDA_NAME: _Z11GetRoundIntf
// DEMANGLED: GetRoundInt(float)
int GetRoundInt(float value) {
    return value < 0.0f ? static_cast<int>(value - 0.5f)
                        : static_cast<int>(value + 0.5f);
}

// IDA_EA: 0x8A39866
// IDA_NAME: _Z31getUpgradeWeaponAttackPowerRate17ENUM_CHARACTERJOBjj
// DEMANGLED: getUpgradeWeaponAttackPowerRate(ENUM_CHARACTERJOB,unsigned int,unsigned int)
float getUpgradeWeaponAttackPowerRate(ENUM_CHARACTERJOB job,
                                      unsigned int grade,
                                      unsigned int value_type) {
    if (grade <= 5u && value_type <= 1u) {
        return upgradeWeaponAttackPowerRate_[
            12 * static_cast<int>(job) + 2 * grade + value_type];
    }
    return 0.0f;
}

// IDA_EA: 0x8A0AEB8
// IDA_NAME: _Z20initObjectScriptBasev
// DEMANGLED: initObjectScriptBase(void)
int initObjectCategory();

bool initObjectScriptBase() {
    return initObjectCategory() == 1;
}

// IDA_EA: 0x8A0AED8
// IDA_NAME: _Z18initObjectCategoryv
// DEMANGLED: initObjectCategory(void)
int initObjectCategory() {
    static const char* const kCategoryTags[] = {
        "[human]", "[beast]", "[plant]", "[insect]", "[machine]",
        "[organic]", "[hybrid]", "[undead]", "[devil]", "[spirit]",
        "[dragon]", "[angel]", "[melee combat]", "[range combat]",
        "[close-passive]", "[close-carefully]", "[full-superarmor]",
        "[close-tough]", "[close-manace]", "[oneside move]",
        "[human armor]", "[fort armor]", "[divine armor]", "[goblin]",
        "[tau]", "[lugaru]", "[fixture]", "[stay]", "[melee]",
        "[missile]", "[energy]", "[physical]", "[magical]", "[bullet]",
        "[cannon]", "[explosion]", "[inflame]", "[non inflame]",
        "[light]", "[dark]", "[water]", "[fire]", "[fort]",
        "[unbeatable]"
    };
    static const int kCategoryValues[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43
    };
    for (unsigned int index = 0; index < 44u; ++index) {
        s_objCategoryString[kCategoryTags[index]] = kCategoryValues[index];
    }
    return 1;
}

// IDA_EA: 0x8A3CA1B
// IDA_NAME: _Z17getObjectCategorySs
// DEMANGLED: getObjectCategory(std::string)
int getObjectCategory(std::string category) {
    const ObjectCategoryMap::const_iterator found =
        s_objCategoryString.find(category);
    return found == s_objCategoryString.end() ? 44 : found->second;
}

// IDA_EA: 0x8A3CA81
// IDA_NAME: _Z23getObjectCategoryString13ENUM_CATEGORY
// DEMANGLED: getObjectCategoryString(ENUM_CATEGORY)
std::string getObjectCategoryString(int category) {
    for (ObjectCategoryMap::const_iterator it = s_objCategoryString.begin();
         it != s_objCategoryString.end();
         ++it) {
        if (it->second == category) {
            return it->first;
        }
    }
    return std::string();
}

// IDA_EA: 0x861CC47
// IDA_NAME: _Z15GetEscapeStringP5MySQLPKcPc
// DEMANGLED: GetEscapeString(MySQL *,char const*,char *)
char* GetEscapeString(MySQL* mysql, const char* source, char* dest) {
    mysql->escape_string(dest, source);
    return dest;
}

// IDA_EA: 0x841FFF8
// IDA_NAME: _Z14AddFieldStringRbPciiPKcz
// DEMANGLED: AddFieldString(bool &,char *,int,int,char const*,...)
char* AddFieldString(bool& first,
                     char* dest,
                     int flags,
                     int mask,
                     const char* format,
                     ...) {
    char text[100];
    std::memset(text, 0, sizeof(text));
    if ((flags & mask) == 0) {
        return reinterpret_cast<char*>(flags & mask);
    }

    va_list args;
    va_start(args, format);
    std::vsprintf(text, format, args);
    va_end(args);

    if (first) {
        first = false;
    } else {
        std::strncat(dest, ",", std::strlen(","));
    }
    return std::strncat(dest, text, std::strlen(text));
}

// IDA_EA: 0x89BBC0F
// IDA_NAME: _Z26getCharacterJobToEnumValueRSs
// DEMANGLED: getCharacterJobToEnumValue(std::string &)
int getCharacterJobToEnumValue(std::string& text) {
    if (text == "swordman") {
        return oldsrv::config::kCharacterJobSwordman;
    }
    if (text == "fighter") {
        return oldsrv::config::kCharacterJobFighter;
    }
    if (text == "gunner") {
        return oldsrv::config::kCharacterJobGunner;
    }
    if (text == "mage") {
        return oldsrv::config::kCharacterJobMage;
    }
    if (text == "priest") {
        return oldsrv::config::kCharacterJobPriest;
    }
    if (text == "at gunner") {
        return oldsrv::config::kCharacterJobAtGunner;
    }
    if (text == "thief") {
        return oldsrv::config::kCharacterJobThief;
    }
    if (text == "at fighter") {
        return oldsrv::config::kCharacterJobAtFighter;
    }
    if (text == "at mage") {
        return oldsrv::config::kCharacterJobAtMage;
    }
    if (text == "demonic swordman") {
        return oldsrv::config::kCharacterJobDemonicSwordman;
    }
    if (text == "creator mage") {
        return oldsrv::config::kCharacterJobCreatorMage;
    }
    if (text == "at swordman") {
        return oldsrv::config::kCharacterJobAtSwordman;
    }
    if (text == "common") {
        return oldsrv::config::kCharacterJobCommon;
    }
    return -1;
}

// IDA_EA: 0x88BE63D
// IDA_NAME: _Z16getGrowTypeIndexPKc
// DEMANGLED: getGrowTypeIndex(char const*)
int getGrowTypeIndex(const char* text) {
    if (std::strcmp(text, "none") == 0) {
        return 0;
    }

    if (std::strcmp(text, "weaponmaster") == 0 ||
        std::strcmp(text, "nenmaster") == 0 ||
        std::strcmp(text, "ranger") == 0 ||
        std::strcmp(text, "elementalmaster") == 0 ||
        std::strcmp(text, "crusader") == 0 ||
        std::strcmp(text, "rogue") == 0 ||
        std::strcmp(text, "elementalbomber") == 0) {
        return 1;
    }

    if (std::strcmp(text, "soulbringer") == 0 ||
        std::strcmp(text, "striker") == 0 ||
        std::strcmp(text, "launcher") == 0 ||
        std::strcmp(text, "summoner") == 0 ||
        std::strcmp(text, "infighter") == 0 ||
        std::strcmp(text, "necromancer") == 0 ||
        std::strcmp(text, "glacialmaster") == 0) {
        return 2;
    }

    if (std::strcmp(text, "berserker") == 0 ||
        std::strcmp(text, "streetfighter") == 0 ||
        std::strcmp(text, "mechanic") == 0 ||
        std::strcmp(text, "battlemage") == 0 ||
        std::strcmp(text, "exorcist") == 0) {
        return 3;
    }

    if (std::strcmp(text, "asura") == 0 ||
        std::strcmp(text, "grappler") == 0 ||
        std::strcmp(text, "spitfire") == 0 ||
        std::strcmp(text, "witch") == 0 ||
        std::strcmp(text, "avenger") == 0) {
        return 4;
    }

    return 0xFFFF;
}

// IDA_EA: 0x82F843F
// IDA_NAME: _Z22GetMarketConditionRateddPdiRiRd
// DEMANGLED: GetMarketConditionRate(double,double,double *,int,int &,double &)
double* GetMarketConditionRate(double base_amount,
                               double current_amount,
                               double* threshold_rates,
                               int previous_state,
                               int& out_state,
                               double& out_rate) {
    out_state = 0;
    out_rate = 1.0;
    for (out_state = 0;
         out_state <= 4 &&
         threshold_rates[out_state] * base_amount <= current_amount;
         ++out_state) {
    }
    if (previous_state + 1 < out_state) {
        out_state = previous_state + 1;
    }
    if (out_state) {
        out_rate = threshold_rates[out_state + 4];
        return &out_rate;
    }
    return reinterpret_cast<double*>(out_state);
}

// IDA_EA: 0x82F84F8
// IDA_NAME: _Z21MakeNotiSetDurabilityR11PacketGuardi
// DEMANGLED: MakeNotiSetDurability(PacketGuard &,int)
int MakeNotiSetDurability(PacketGuard& packet, int durability_rate) {
    packet.put_header(0, 179);
    packet.put_int(durability_rate);
    return packet.finalize(true);
}

// IDA_EA: 0x842AE3F
// IDA_NAME: _Z23saveSecurityCardHistoryP5MySQLj26ENUM_SECURITYCARD_MOD_TYPE
// DEMANGLED: saveSecurityCardHistory(MySQL *,unsigned int,ENUM_SECURITYCARD_MOD_TYPE)
int saveSecurityCardHistory(MySQL* db,
                            unsigned int account_id,
                            int modify_type) {
    db->set_query(
        "inSert into member_security_card_history(occ_time,m_id,modify_type) "
        "values(now(),%s,%d)",
        NumberToString(account_id, 0),
        modify_type);
    return db->exec(true);
}

// IDA_EA: 0x842AF12
// IDA_NAME: _Z19turnOffSecurityFlagP5MySQLj
// DEMANGLED: turnOffSecurityFlag(MySQL *,unsigned int)
int turnOffSecurityFlag(MySQL* db, unsigned int account_id) {
    char table_name[28];
    std::memset(table_name, 0, 0x14u);
    std::memcpy(table_name, "member_login", 0x0Du);

    db->set_query(
        "upDate %s set security_flag=0 where m_id=%s and security_flag=1",
        table_name,
        NumberToString(account_id, 0));
    return db->exec(true);
}

// IDA_EA: 0x842AE90
// IDA_NAME: _Z18turnOnSecurityFlagP5MySQLj
// DEMANGLED: turnOnSecurityFlag(MySQL *,unsigned int)
int turnOnSecurityFlag(MySQL* db, unsigned int account_id) {
    char table_name[28];
    std::memset(table_name, 0, 0x14u);
    std::memcpy(table_name, "member_login", 0x0Du);

    db->set_query(
        "upDate %s set security_flag=1 where m_id=%s and security_flag=0",
        table_name,
        NumberToString(account_id, 0));
    return db->exec(true);
}

// IDA_EA: 0x86B216D
// IDA_NAME: _Z28generateSecurityCardQuestionPc
// DEMANGLED: generateSecurityCardQuestion(char *)
unsigned int generateSecurityCardQuestion(char* p_src) {
    unsigned int first_kind = get_rand_int(4);
    int first_digit =
        first_kind == 3 ? static_cast<int>(get_rand_int(8))
                        : static_cast<int>(get_rand_int(10));
    unsigned int second_kind;
    int second_digit;
    do {
        second_kind = get_rand_int(4);
        second_digit =
            second_kind == 3 ? static_cast<int>(get_rand_int(8))
                             : static_cast<int>(get_rand_int(10));
    } while (first_digit / 2 == second_digit / 2 &&
             first_kind == second_kind);

    p_src[0] = static_cast<char>(first_digit + 48);
    p_src[1] = static_cast<char>(first_kind + 48);
    p_src[2] = static_cast<char>(second_digit + 48);
    p_src[3] = static_cast<char>(second_kind + 48);
    return second_kind + 48;
}

// IDA_EA: 0x8A5A86C
// IDA_NAME: _Z11BitsetToStrRKSt6bitsetILj256EERSs
// DEMANGLED: BitsetToStr(std::bitset<256u> const&,std::string &)
bool BitsetToStr(const std::bitset<256>& bits, std::string& text) {
    text.resize(0x20u);
    for (int byte_index = 0; byte_index <= 31; ++byte_index) {
        unsigned char value = 0;
        for (int bit_index = 0; bit_index <= 7; ++bit_index) {
            value = static_cast<unsigned char>(
                value +
                (static_cast<unsigned char>(
                     bits[static_cast<std::size_t>(bit_index + 8 * byte_index)])
                 << bit_index));
        }
        text[static_cast<std::size_t>(byte_index)] =
            static_cast<char>(value);
    }
    return false;
}

// IDA_EA: 0x8A5A90A
// IDA_NAME: _Z11BitsetToStrRKSt6bitsetILj256EEPh
// DEMANGLED: BitsetToStr(std::bitset<256u> const&,unsigned char *)
bool BitsetToStr(const std::bitset<256>& bits, unsigned char* text) {
    for (int byte_index = 0; byte_index <= 31; ++byte_index) {
        unsigned char value = 0;
        for (int bit_index = 0; bit_index <= 7; ++bit_index) {
            value = static_cast<unsigned char>(
                value +
                (static_cast<unsigned char>(
                     bits[static_cast<std::size_t>(bit_index + 8 * byte_index)])
                 << bit_index));
        }
        text[byte_index] = value;
    }
    return false;
}

// IDA_EA: 0x8A5A989
// IDA_NAME: _Z11StrToBitsetPhRSt6bitsetILj256EE
// DEMANGLED: StrToBitset(unsigned char *,std::bitset<256u> &)
bool StrToBitset(unsigned char* text, std::bitset<256>& bits) {
    for (int byte_index = 0; byte_index <= 31; ++byte_index) {
        for (int bit_index = 0; bit_index <= 7; ++bit_index) {
            bits[static_cast<std::size_t>(bit_index + 8 * byte_index)] =
                ((static_cast<int>(text[byte_index]) >> bit_index) & 1) != 0;
        }
    }
    return false;
}

// IDA_EA: 0x89BAB64
// IDA_NAME: _Z17createLotteryInfoP11LotteryInfoPSt6vectorIiSaIiEE
// DEMANGLED: createLotteryInfo(LotteryInfo *,std::vector<int,std::allocator<int>> *)
bool createLotteryInfo(LotteryInfo* info, std::vector<int>* values) {
    info->clear();
    std::vector<int>::iterator it = values->begin();
    info->first_dword = static_cast<uint32_t>(*it);
    ++it;
    info->second_dword = 1;

    while (it != values->end()) {
        std::pair<int, int> item_rate;
        item_rate.first = *it;
        ++it;
        if (it == values->end()) {
            return true;
        }
        item_rate.second = *it;
        ++it;
        info->pair_values.push_back(item_rate);
        info->int_values.push_back(1);
    }
    return false;
}

// IDA_EA: 0x846C0A8
// IDA_NAME: _Z22CheckDailyScheduleTimeill
// DEMANGLED: CheckDailyScheduleTime(int,long,long)
bool CheckDailyScheduleTime(int tm_hour, long last_time, long cur_time) {
    std::time_t timer = static_cast<std::time_t>(cur_time);
    std::tm current_tm;
    localtime_r(&timer, &current_tm);

    std::tm reset_tm = current_tm;
    reset_tm.tm_hour = tm_hour;
    reset_tm.tm_min = 0;
    reset_tm.tm_sec = 0;

    std::time_t reset_time = std::mktime(&reset_tm);
    if (current_tm.tm_hour < tm_hour) {
        reset_time -= 86400;
    }
    return static_cast<std::time_t>(last_time) <= reset_time;
}

// IDA_EA: 0x846C144
// IDA_NAME: _Z23ComputeDaysScheduleTimeill
// DEMANGLED: ComputeDaysScheduleTime(int,long,long)
int ComputeDaysScheduleTime(int tm_hour, long last_time, long cur_time) {
    std::time_t current_timer = static_cast<std::time_t>(cur_time);
    std::tm current_tm;
    localtime_r(&current_timer, &current_tm);

    std::tm current_schedule_tm = current_tm;
    current_schedule_tm.tm_hour = tm_hour;
    current_schedule_tm.tm_min = 0;
    current_schedule_tm.tm_sec = 0;
    std::time_t current_schedule_time = std::mktime(&current_schedule_tm);
    if (current_tm.tm_hour < tm_hour) {
        current_schedule_time -= 86400;
    }

    std::time_t last_timer = static_cast<std::time_t>(last_time);
    std::tm last_tm;
    localtime_r(&last_timer, &last_tm);

    std::tm last_schedule_tm = last_tm;
    last_schedule_tm.tm_hour = tm_hour;
    last_schedule_tm.tm_min = 0;
    last_schedule_tm.tm_sec = 0;
    std::time_t last_schedule_time = std::mktime(&last_schedule_tm);
    if (last_tm.tm_hour < tm_hour) {
        last_schedule_time -= 86400;
    }

    const int days = static_cast<int>(
        (current_schedule_time - last_schedule_time) / 86400);
    return days < 0 ? 0 : days;
}

// IDA_EA: 0x846C2E9
// IDA_NAME: _Z19getPrevScheduleTimeil
// DEMANGLED: getPrevScheduleTime(int,long)
long getPrevScheduleTime(int tm_hour, long timestamp) {
    const int seconds_per_hour = 3600;
    const int reset_seconds = seconds_per_hour * tm_hour;
    std::time_t timer =
        static_cast<std::time_t>(timestamp - reset_seconds);

    std::tm reset_tm;
    localtime_r(&timer, &reset_tm);
    reset_tm.tm_hour = tm_hour;
    reset_tm.tm_min = 0;
    reset_tm.tm_sec = 0;
    return static_cast<long>(std::mktime(&reset_tm));
}

// IDA_EA: 0x8145E64
// IDA_NAME: _ZN6global5tools20InitializeCharacStat14initCharacStatER15_Additioal_infojiiii
// DEMANGLED: global::tools::InitializeCharacStat::initCharacStat(_Additioal_info &,unsigned int,int,int,int,int)
int global::tools::InitializeCharacStat::initCharacStat(
    _Additioal_info& stat,
    unsigned int charac_no,
    int job,
    int level,
    int first_grow_type,
    int second_grow_type) {
    int result = stat.physical_attack;
    if (result != 0) {
        return result;
    }
    result = stat.magical_attack;
    if (result != 0) {
        return result;
    }
    result = stat.physical_defense;
    if (result != 0) {
        return result;
    }
    result = stat.magical_defense;
    if (result != 0) {
        return result;
    }
    result = static_cast<int32_t>(stat.max_hp);
    if (result != 0) {
        return result;
    }
    result = static_cast<int32_t>(stat.max_mp);
    if (result != 0) {
        return result;
    }
    result = static_cast<int32_t>(stat.move_speed);
    if (result != 0) {
        return result;
    }

    stat.clear();
    if (job < 0 || job >= oldsrv::config::kCharacterJobCount ||
        level > oldsrv::config::kCharacterLevelMaximum ||
        level < oldsrv::config::kCharacterLevelMinimum) {
        cMyTrace trace(kInitializeCharacStatPrettyFunction, 31, 5);
        return trace("[ARAD,STAT,RESET] Invalid character job. "
                     "charac %u, job %d, level %d",
                     charac_no,
                     job,
                     level);
    }

    const CCharacter* character =
        &G_CDataManager()->get_character_list()->characters_[job];
    const _Additioal_info base_stat = character->base_status_[0];

    const _Additioal_info* base_grow_stat =
        character->get_growtype_stat(0, 0);
    const _Additioal_info* first_grow_stat =
        character->get_growtype_stat(first_grow_type, 0);
    const _Additioal_info* second_grow_stat =
        character->get_growtype_stat(first_grow_type, second_grow_type);
    if (base_grow_stat == 0 || first_grow_stat == 0 ||
        second_grow_stat == 0) {
        cMyTrace trace(kInitializeCharacStatPrettyFunction, 45, 5);
        return trace("[ARAD,STAT,RESET] Invalid character grow type. "
                     "charac %u, first %d, second %d",
                     charac_no,
                     first_grow_type,
                     second_grow_type);
    }

    stat = base_stat;
    for (int index = 1; index < level; ++index) {
        if (index > 14) {
            if (index > 49) {
                stat.add(*second_grow_stat);
            } else {
                stat.add(*first_grow_stat);
            }
        } else {
            stat.add(*base_grow_stat);
        }
    }

    cMyTrace trace(kInitializeCharacStatPrettyFunction, 65, 0);
    return trace("[ARAD][RESET STAT] Init Character : %d, stat : "
                 "hp %d, mp %d, pyatt %d, pydef %d, mgatt %d, "
                 "mgdef %d, speed %d, element %d, element %d, "
                 "element %d, element %d)",
                 charac_no,
                 static_cast<int32_t>(stat.max_hp),
                 static_cast<int32_t>(stat.max_mp),
                 stat.physical_attack,
                 stat.physical_defense,
                 stat.magical_attack,
                 stat.magical_defense,
                 static_cast<int32_t>(stat.move_speed),
                 static_cast<int>(static_cast<int16_t>(
                     stat.element_resist[0])),
                 static_cast<int>(static_cast<int16_t>(
                     stat.element_resist[1])),
                 static_cast<int>(static_cast<int16_t>(
                     stat.element_resist[2])),
                 static_cast<int>(static_cast<int16_t>(
                     stat.element_resist[3])));
}

// IDA_EA: 0x86B192D
// IDA_NAME: _Z16get_str_datetimePci
// DEMANGLED: get_str_datetime(char *,int)
int get_str_datetime(char* text, int length) {
    (void)length;
    std::time_t timer = 0;
    CSystemTime* system_time = GlobalData::system_time();
    if (system_time) {
        timer = static_cast<std::time_t>(system_time->getCurSec());
    } else {
        std::time(&timer);
    }

    std::tm tm_value;
    std::tm* local = localtime_r(&timer, &tm_value);
    return std::sprintf(text,
                        "%04d%02d%02d%02d%02d%02d",
                        local->tm_year + 1900,
                        local->tm_mon + 1,
                        local->tm_mday,
                        local->tm_hour,
                        local->tm_min,
                        local->tm_sec);
}


namespace {

__thread char number_to_string_buffers[8][64];

}  // namespace

// IDA_EA: 0x8108750
// IDA_NAME: _Z8Char2HexhPc
// DEMANGLED: Char2Hex(unsigned char,char *)
char* Char2Hex(unsigned char value, char* output) {
    static const char saucHex[] = "0123456789abcdef";
    output[0] = saucHex[value >> 4];
    output[1] = saucHex[value & 0x0F];
    output[2] = '\0';
    return output + 2;
}

// IDA_EA: 0x8108792
// IDA_NAME: _Z8Hex2CharPKcRh
// DEMANGLED: Hex2Char(char const*,unsigned char &)
int Hex2Char(const char* input, unsigned char& output) {
    if (input[0] > '0' - 1 && input[0] <= '9') {
        output = static_cast<unsigned char>(input[0] - '0');
    } else {
        if (input[0] <= 'a' - 1 || input[0] > 'f') {
            return 0;
        }
        output = static_cast<unsigned char>(input[0] - 'a' + 10);
    }

    if (input[1] > '0' - 1 && input[1] <= '9') {
        output = static_cast<unsigned char>(output * 16 + input[1] - '0');
    } else {
        if (input[1] <= 'a' - 1 || input[1] > 'f') {
            return 0;
        }
        output = static_cast<unsigned char>(output * 16 + input[1] - 'a' + 10);
    }

    return 1;
}

// IDA_EA: 0x8108F9C
// IDA_NAME: _Z14PrintBackTracev
// DEMANGLED: PrintBackTrace(void)
void PrintBackTrace() {
    void* array[100];
    const int size = backtrace(array, 100);
    char** symbols = backtrace_symbols(array, size);
    if (symbols != 0) {
        for (int index = 0; index < size; ++index) {
            CMyFileLog log("PrintBackTrace", 465);
            log("./log/BackTrace", symbols[index]);
        }
        std::free(symbols);
    }
}

// IDA_EA: 0x810904B
// IDA_NAME: _Z14NumberToStringji
// DEMANGLED: NumberToString(unsigned int,int)
const char* NumberToString(unsigned int value, int slot) {
    const unsigned int index = static_cast<unsigned int>(slot) & 7U;
    std::sprintf(number_to_string_buffers[index], "%u", value);
    return number_to_string_buffers[index];
}

// IDA_EA: 0x8109098
// IDA_NAME: _Z14NumberToStringyi
// DEMANGLED: NumberToString(unsigned long long,int)
const char* NumberToString(unsigned long long value, int slot) {
    const unsigned int index = static_cast<unsigned int>(slot) & 7U;
    std::sprintf(number_to_string_buffers[index], "%llu", value);
    return number_to_string_buffers[index];
}

// IDA_EA: 0x81090FA
// IDA_NAME: _Z14StringToNumberPKc
// DEMANGLED: StringToNumber(char const*)
unsigned int StringToNumber(const char* text) {
    if (text != 0) {
        return static_cast<unsigned int>(std::strtoul(text, 0, 10));
    }
    return 0;
}

// IDA_EA: 0x810912A
// IDA_NAME: _Z23GetCurrentResetBaseTimeli
// DEMANGLED: GetCurrentResetBaseTime(long,int)
std::time_t GetCurrentResetBaseTime(std::time_t timer, int reset_hour) {
    std::time_t previous_timer = timer - 86400;
    std::tm current_tm;
    std::tm previous_tm;
    std::tm* current = localtime_r(&timer, &current_tm);
    std::tm* previous = localtime_r(&previous_timer, &previous_tm);

    if (current->tm_hour < 0 || current->tm_hour >= reset_hour) {
        current->tm_hour = reset_hour;
        current->tm_min = 0;
        current->tm_sec = 0;
        return std::mktime(current);
    }

    previous->tm_hour = reset_hour;
    previous->tm_min = 0;
    previous->tm_sec = 0;
    return std::mktime(previous);
}

// IDA_EA: 0x81091DC
// IDA_NAME: _Z14checkTimeTodayli
// DEMANGLED: checkTimeToday(long,int)
bool checkTimeToday(std::time_t timer, int reset_hour) {
    std::time_t previous_timer = timer - 86400;
    std::tm current_tm;
    std::tm previous_tm;
    std::tm* current = localtime_r(&timer, &current_tm);
    localtime_r(&previous_timer, &previous_tm);
    return current->tm_hour < 0 || current->tm_hour >= reset_hour;
}

// IDA_EA: 0x820582B
// IDA_NAME: _Z20checkValidCharacNamePKc
// DEMANGLED: checkValidCharacName(char const*)
int checkValidCharacName(const char* text) {
    bool invalid = true;
    if (!G_CDataManager()->hasSpecialCharacter(text)) {
        if (!G_CDataManager()->hasPreventString(text)) {
            if (!G_CDataManager()->hasPreventStringName(text)) {
                invalid = false;
            }
        }
    }
    return !invalid;
}

// IDA_EA: 0x8275315
// IDA_NAME: _Z21checkNewMemberHacklogjP5CUserj
// DEMANGLED: checkNewMemberHacklog(unsigned int,CUser *,unsigned int)
void checkNewMemberHacklog(unsigned int registration_date,
                           CUser* user,
                           unsigned int registration_seconds) {
    if (!user) {
        return;
    }

    std::tm registration_tm;
    std::memset(&registration_tm, 0, sizeof(registration_tm));
    registration_tm.tm_year = static_cast<int>(registration_date / 10000u) -
                              1900;
    registration_tm.tm_mon =
        static_cast<int>((registration_date / 100u) % 100u) - 1;
    registration_tm.tm_mday = static_cast<int>(registration_date % 100u);
    registration_tm.tm_isdst = -1;

    const std::time_t registration_time = std::mktime(&registration_tm);
    const int elapsed_days = static_cast<int>(
        std::difftime(std::time(0), registration_time) / 86400.0);

    short etc_key = 308;
    WongWork::ENUM_HACKTYPE hack_type =
        static_cast<WongWork::ENUM_HACKTYPE>(0x134u);
    if (elapsed_days > 0) {
        if (elapsed_days <= 3) {
            etc_key = 309;
            hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x135u);
        } else if (elapsed_days <= 7) {
            etc_key = 310;
            hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x136u);
        } else if (elapsed_days <= 30) {
            etc_key = 311;
            hack_type = static_cast<WongWork::ENUM_HACKTYPE>(0x137u);
        } else {
            return;
        }
    }

    const unsigned int previous = static_cast<unsigned int>(
        user->SetETC(etc_key, static_cast<int>(registration_seconds)));
    user->getHackAnalyzer()->addServerHackCnt(
        user, hack_type, 1u, previous, registration_date);
}

// IDA_EA: 0x82138F2
// IDA_NAME: _Z27IsAuctionServerDisconnectedh
// DEMANGLED: IsAuctionServerDisconnected(unsigned char)
int IsAuctionServerDisconnected(uint8_t payment_type) {
    return (!payment_type &&
            GlobalData::s_auction_proxy->IsRunning() != 1) ||
           (payment_type == 1 &&
            GlobalData::s_cera_auction_proxy->IsRunning() != 1);
}

// IDA_EA: 0x8213E15
// IDA_NAME: _Z12IsPriceErrorii
// DEMANGLED: IsPriceError(int,int)
int IsPriceError(int first_price, int second_price) {
    return first_price < -1 || second_price < -1 ||
           (second_price != -1 && first_price >= second_price);
}

// IDA_EA: 0x8213E40
// IDA_NAME: _Z21IsValidMultiplyResultii
// DEMANGLED: IsValidMultiplyResult(int,int)
int IsValidMultiplyResult(int first_value, int multiplier) {
    return static_cast<long long>(first_value) * multiplier <= 0x7fffffffLL;
}
