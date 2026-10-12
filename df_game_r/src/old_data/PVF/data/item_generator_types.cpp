#include "old_data/PVF/data/item_generator_types.h"

#include <ctime>

#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "common/manager/mt_rand.h"

extern "C++" int get_rand_int(int limit);

namespace WongWork {
namespace {

unsigned int g_generate_random_seed_increment = 0;

// Both GenerateRandomUpgrade methods use the same (int,float) table at
// CDataManager+0x540.  The original signed comparisons intentionally choose
// the first entry below the range and the last entry above it; preserve that
// ordering rather than normalizing the table interpretation.
float get_upgrade_dungeon_level_multiplier(
    const IntFloatPairVector& adjustments, int dungeon_level) {
    float multiplier = 1.0f;
    if (adjustments.empty()) {
        return multiplier;
    }

    const int last_index = static_cast<int>(adjustments.size()) - 1;
    if (adjustments[0].first < dungeon_level) {
        multiplier = adjustments[0].second;
    } else if (adjustments[last_index].first > dungeon_level) {
        multiplier = adjustments[last_index].second;
    } else {
        for (int index = 0; index <= last_index; ++index) {
            if (adjustments[index].first == dungeon_level) {
                multiplier = adjustments[index].second;
            }
        }
    }
    return multiplier;
}

int scale_upgrade_weight(int weight, float multiplier) {
    // 0x8533853/0x8533AAC explicitly change the x87 control word to truncate
    // the product toward zero before storing its signed 32-bit result.
    return static_cast<int>(static_cast<long double>(weight) * multiplier);
}

}  // namespace

// IDA_EA: 0x8374EBC
// IDA_NAME: _ZN8WongWork20CGeneratorCommonDataC2Ev
// DEMANGLED: WongWork::CGeneratorCommonData::CGeneratorCommonData(void)
CGeneratorCommonData::CGeneratorCommonData() {}

// IDA_EA: 0x8374EC2
// IDA_NAME: _ZN8WongWork20CGeneratorCommonDataD2Ev
// DEMANGLED: WongWork::CGeneratorCommonData::~CGeneratorCommonData()
CGeneratorCommonData::~CGeneratorCommonData() {}

// IDA_EA: 0x8534456
// IDA_NAME: _ZN8WongWork20CGeneratorCommonData10loadScriptEPKc
// DEMANGLED: WongWork::CGeneratorCommonData::loadScript(char const*)
int CGeneratorCommonData::loadScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 472;
    }

    bool success = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag != "[gold drop ref table]") {
            return 504;
        }

        gold_generate_range_[0].minimum = 0;
        gold_generate_range_[0].maximum = 0;
        for (int row = 1; row <= 200; ++row) {
            const int level = ScanInt(&success);
            if (!success) {
                return 492;
            }
            gold_generate_range_[level].minimum = ScanInt(&success);
            if (!success) {
                return 496;
            }
            gold_generate_range_[level].maximum = ScanInt(&success);
            if (!success) {
                return 500;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8533424
// IDA_NAME: _ZN8WongWork20CGeneratorCommonData15getGoldGenTableEh
// DEMANGLED: WongWork::CGeneratorCommonData::getGoldGenTable(unsigned char)
GoldGenerateRange* CGeneratorCommonData::getGoldGenTable(
    unsigned char character_level) {
    const unsigned char level = character_level <= 200u ? character_level : 0u;
    static GoldGenerateRange adjusted_range;
    adjusted_range = gold_generate_range_[level];

    CAutoMarketConditionsControl* const market =
        G_CDataManager()->GetAutoMarketContitionsControl();
    if (market) {
        adjusted_range.minimum = market->GetGoldDropAmountRate(
            level, adjusted_range.minimum);
    }
    return &adjusted_range;
}

// IDA_EA: 0x85334EA
// IDA_NAME: _ZN8WongWork21CGenerateRandomNumberC2Ei
// DEMANGLED: WongWork::CGenerateRandomNumber::CGenerateRandomNumber(int)
CGenerateRandomNumber::CGenerateRandomNumber(int stream_count)
    : streams_(0), stream_count_(stream_count) {
    if (stream_count_ == 0) {
        return;
    }

    streams_ = new CMTRand[stream_count_];
    const unsigned long current_time =
        static_cast<unsigned long>(std::time(0));
    for (int index = 0; index < stream_count_; ++index) {
        streams_[index].seed(current_time +
                             g_generate_random_seed_increment + index);
    }
    g_generate_random_seed_increment +=
        static_cast<unsigned int>(stream_count_);
}

// IDA_EA: 0x85335D0
// IDA_NAME: _ZN8WongWork21CGenerateRandomNumberD2Ev
// DEMANGLED: WongWork::CGenerateRandomNumber::~CGenerateRandomNumber()
CGenerateRandomNumber::~CGenerateRandomNumber() {
    delete[] streams_;
}

// IDA_EA: 0x85334A4
// IDA_NAME: _ZN8WongWork21CGenerateRandomNumber14generateNumberEii
// DEMANGLED: WongWork::CGenerateRandomNumber::generateNumber(int,int)
unsigned int CGenerateRandomNumber::generateNumber(int stream_index,
                                                    int limit) {
    if (stream_count_ < stream_index) {
        return static_cast<unsigned int>(get_rand_int(limit));
    }
    const unsigned long bounded_limit = static_cast<unsigned long>(limit);
    return streams_[stream_index].randInt(bounded_limit);
}

// IDA_EA: 0x85335EE
// IDA_NAME: _ZN8WongWork21GenerateRandomUpgradeC2Ev
// DEMANGLED: WongWork::GenerateRandomUpgrade::GenerateRandomUpgrade(void)
GenerateRandomUpgrade::GenerateRandomUpgrade()
    : random_(new CMTRand(static_cast<unsigned long>(std::time(0)))) {}

// IDA_EA: 0x853364E
// IDA_NAME: _ZN8WongWork21GenerateRandomUpgradeD2Ev
// DEMANGLED: WongWork::GenerateRandomUpgrade::~GenerateRandomUpgrade()
GenerateRandomUpgrade::~GenerateRandomUpgrade() {
    delete random_;
    random_ = 0;
}

// IDA_EA: 0x8533676.
// IDA_NAME: _ZN8WongWork21GenerateRandomUpgrade15generateUpgradeE11ENUM_RARITYi
// DEMANGLED: WongWork::GenerateRandomUpgrade::generateUpgrade(ENUM_RARITY,int)
int GenerateRandomUpgrade::generateUpgrade(ENUM_RARITY rarity,
                                           int dungeon_level) {
    ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    const ItemFarmingDataMap& item_farming_data =
        parameters->get_item_farming_data();
    const ItemFarmingDataMap::const_iterator found =
        item_farming_data.find(static_cast<int>(rarity));
    if (found == item_farming_data.end() || found->second.total_rate == 0) {
        return 0;
    }

    const IntFloatPairVector& adjustments =
        parameters->get_drop_bonus_of_dungeon_level_increase();
    const float multiplier =
        get_upgrade_dungeon_level_multiplier(adjustments, dungeon_level);

    ItemFarmingData selected(found->second);
    const int original_first_weight = selected.list[0].second;
    selected.list[0].second =
        scale_upgrade_weight(original_first_weight, multiplier);

    unsigned long total_weight =
        static_cast<unsigned long>(selected.total_rate);
    total_weight -= static_cast<unsigned long>(original_first_weight);
    total_weight += static_cast<unsigned long>(selected.list[0].second);

    const int random_value =
        static_cast<int>(random_->randInt(total_weight));
    const int item_count = static_cast<int>(selected.list.size());
    int accumulated_weight = 0;
    for (int index = 0; index < item_count; ++index) {
        accumulated_weight += selected.list[index].second;
        if (accumulated_weight >= random_value) {
            return selected.list[index].first;
        }
    }
    return 0;
}

// IDA_EA: 0x8533932.
// IDA_NAME: _ZN8WongWork21GenerateRandomUpgrade18generateEquipGradeEi
// DEMANGLED: WongWork::GenerateRandomUpgrade::generateEquipGrade(int)
int GenerateRandomUpgrade::generateEquipGrade(int dungeon_level) {
    const int grades[5] = {23207, 4006, 27171, 24048, 29296};
    int weights[5] = {1000, 1000, 1000, 1000, 1000};
    unsigned long total_weight = 5000u;

    const IntFloatPairVector& adjustments = G_CDataManager()
                                                 ->get_server_parameter_script()
                                                 ->get_drop_bonus_of_dungeon_level_increase();
    if (adjustments.empty()) {
        return grades[0];
    }

    const float multiplier =
        get_upgrade_dungeon_level_multiplier(adjustments, dungeon_level);
    const int original_first_weight = weights[0];
    weights[0] = scale_upgrade_weight(weights[0], multiplier);
    total_weight -= static_cast<unsigned long>(original_first_weight);
    total_weight += static_cast<unsigned long>(weights[0]);

    const int random_value =
        static_cast<int>(random_->randInt(total_weight));
    int accumulated_weight = 0;
    for (int index = 0; index <= 4; ++index) {
        accumulated_weight += weights[index];
        if (accumulated_weight >= random_value) {
            return grades[index];
        }
    }
    return grades[0];
}

// IDA_EA: 0x8533B3E
// IDA_NAME: _ZN8WongWork14CItemGeneratorC2Ei
// DEMANGLED: WongWork::CItemGenerator::CItemGenerator(int)
CItemGenerator::CItemGenerator(int random_stream_count)
    : common_data_(0),
      random_number_(random_stream_count),
      random_upgrade_(),
      generator_type_(0) {}

// IDA_EA: 0x8533B9A
// IDA_NAME: _ZN8WongWork14CItemGeneratorD2Ev
// DEMANGLED: WongWork::CItemGenerator::~CItemGenerator()
CItemGenerator::~CItemGenerator() {}

// IDA_EA: 0x853A3E2
// IDA_NAME: _ZN8WongWork14CItemGenerator13setCommonDataEPNS_20CGeneratorCommonDataE
// DEMANGLED: WongWork::CItemGenerator::setCommonData(WongWork::CGeneratorCommonData *)
CItemGenerator* CItemGenerator::setCommonData(
    CGeneratorCommonData* common_data) {
    common_data_ = common_data;
    return this;
}

// IDA_EA: 0x853A3F0
// IDA_NAME: _ZN8WongWork14CItemGenerator12setDropRatioEf
// DEMANGLED: WongWork::CItemGenerator::setDropRatio(float)
void CItemGenerator::setDropRatio(float /*ratio*/) {}

// IDA_EA: 0x853A3F6
// IDA_NAME: _ZN8WongWork14CItemGenerator10loadScriptEPKc
// DEMANGLED: WongWork::CItemGenerator::loadScript(char const*)
int CItemGenerator::loadScript(const char* /*filename*/) {
    return 0;
}

// IDA_EA: 0x853A400
// IDA_NAME: _ZN8WongWork14CItemGenerator18generateRandomItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CItemGenerator::generateRandomItem(WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CItemGenerator::generateRandomItem(const stGenerateRefData_t& /*reference*/,
                                        stGenerateResult_t& /*result*/) {}

// IDA_EA: 0x853A406
// IDA_NAME: _ZN8WongWork14CItemGenerator20generateSpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CItemGenerator::generateSpecificItem(WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CItemGenerator::generateSpecificItem(
    const stGenerateRefData_t& /*reference*/, stGenerateResult_t& /*result*/) {}

// IDA_EA: 0x853A40C
// IDA_NAME: _ZN8WongWork14CItemGenerator16setGeneratorTypeEi
// DEMANGLED: WongWork::CItemGenerator::setGeneratorType(int)
CItemGenerator* CItemGenerator::setGeneratorType(int generator_type) {
    generator_type_ = generator_type;
    return this;
}

// IDA_EA: 0x853A41A
// IDA_NAME: _ZN8WongWork14CItemGenerator16getGeneratorTypeEv
// DEMANGLED: WongWork::CItemGenerator::getGeneratorType(void)
int CItemGenerator::getGeneratorType() {
    return generator_type_;
}

// IDA_EA: 0x85BEFF4
// IDA_NAME: _ZN8WongWork14CItemGenerator22getGeneratorCommonDataEv
// DEMANGLED: WongWork::CItemGenerator::getGeneratorCommonData(void)
CGeneratorCommonData* CItemGenerator::getGeneratorCommonData() {
    return common_data_;
}

}  // namespace WongWork
