#include "old_data/PVF/stackable/stackable_script_types.h"

#include <string.h>

// IDA_EA: 0x898C770
// IDA_NAME: _ZN21CountinuousItemOptionC2Ev
// DEMANGLED: CountinuousItemOption::CountinuousItemOption(void)
CountinuousItemOption::CountinuousItemOption() { clear(); }

// IDA_EA: 0x898C784
// IDA_NAME: _ZN21CountinuousItemOption5clearEv
// DEMANGLED: CountinuousItemOption::clear(void)
CountinuousItemOption* CountinuousItemOption::clear() {
    option_type = 0;
    option_value = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C328A
// IDA_NAME: _ZN12STStatChangeC2Ev
// DEMANGLED: STStatChange::STStatChange(void)
STStatChange::STStatChange() { clear(); }

// IDA_EA: 0x89C329E
// IDA_NAME: _ZN12STStatChange5clearEv
// DEMANGLED: STStatChange::clear(void)
STStatChange* STStatChange::clear() {
    stat_type = 0;
    value = 0;
    enabled = 0;
    duration = 0;
    return this;
}

// IDA_EA: 0x89D3F1A
// IDA_NAME: _ZN12STStatChangeD2Ev
// DEMANGLED: STStatChange::~STStatChange()
STStatChange::~STStatChange() {}

// IDA_EA: 0x89C32C8
// IDA_NAME: _ZN18STStatChangeVectorC2Ev
// DEMANGLED: STStatChangeVector::STStatChangeVector(void)
STStatChangeVector::STStatChangeVector() : values() { clear(); }

// IDA_EA: 0x89C330A
// IDA_NAME: _ZN18STStatChangeVector5clearEv
// DEMANGLED: STStatChangeVector::clear(void)
STStatChangeVector* STStatChangeVector::clear() {
    values.clear();
    value_0c = 0;
    value_10 = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C3ACE
// IDA_NAME: _ZN18STStatChangeVectorD2Ev
// DEMANGLED: STStatChangeVector::~STStatChangeVector()
STStatChangeVector::~STStatChangeVector() {}

// IDA_EA: 0x89C40FE
// IDA_NAME: _ZN18STStatChangeVectoraSERKS_
// DEMANGLED: STStatChangeVector::operator=(STStatChangeVector const&)
STStatChangeVector& STStatChangeVector::operator=(
    const STStatChangeVector& other) {
    values = other.values;
    value_0c = other.value_0c;
    value_10 = other.value_10;
    enabled = other.enabled;
    return *this;
}

// IDA_EA: 0x89C3124
// IDA_NAME: _ZN16STStatusGenerateC2Ev
// DEMANGLED: STStatusGenerate::STStatusGenerate(void)
STStatusGenerate::STStatusGenerate() : values() { clear(); }

// IDA_EA: 0x89C3166
// IDA_NAME: _ZN16STStatusGenerate5clearEv
// DEMANGLED: STStatusGenerate::clear(void)
STStatusGenerate* STStatusGenerate::clear() {
    values.clear();
    value_0c = 0;
    value_10 = 0;
    enabled = 0;
    return this;
}

// IDA_EA: 0x89C3ABA
// IDA_NAME: _ZN16STStatusGenerateD2Ev
// DEMANGLED: STStatusGenerate::~STStatusGenerate()
STStatusGenerate::~STStatusGenerate() {}

// IDA_EA: 0x89C40BC
// IDA_NAME: _ZN16STStatusGenerateaSERKS_
// DEMANGLED: STStatusGenerate::operator=(STStatusGenerate const&)
STStatusGenerate& STStatusGenerate::operator=(
    const STStatusGenerate& other) {
    values = other.values;
    value_0c = other.value_0c;
    value_10 = other.value_10;
    enabled = other.enabled;
    return *this;
}

// IDA_EA: 0x8197E1E
// IDA_NAME: _ZN10CRabbitBoxC2Ev
// DEMANGLED: CRabbitBox::CRabbitBox(void)
CRabbitBox::CRabbitBox()
    : option_type_(-1), item_index_(-1), item_count_(-1), drop_rate_(-1) {}

// IDA_EA: 0x852488C
// IDA_NAME: _ZN10CRabbitBoxC2ERKS_
// DEMANGLED: CRabbitBox::CRabbitBox(CRabbitBox const&)
CRabbitBox::CRabbitBox(const CRabbitBox& other)
    : option_type_(other.option_type_),
      item_index_(other.item_index_),
      item_count_(other.item_count_),
      drop_rate_(other.drop_rate_) {}

// IDA_EA: 0x8197E5A
// IDA_NAME: _ZN10CRabbitBoxD2Ev
// DEMANGLED: CRabbitBox::~CRabbitBox()
CRabbitBox::~CRabbitBox() {}

// IDA_EA: 0x8197EC2
// IDA_NAME: _ZN10CRabbitBoxaSERKS_
// DEMANGLED: CRabbitBox::operator=(CRabbitBox const&)
CRabbitBox& CRabbitBox::operator=(const CRabbitBox& other) {
    option_type_ = other.option_type_;
    item_index_ = other.item_index_;
    item_count_ = other.item_count_;
    drop_rate_ = other.drop_rate_;
    return *this;
}

// IDA_EA: 0x8197E9E
// IDA_NAME: _ZNK10CRabbitBox12getItemIndexEv
// DEMANGLED: CRabbitBox::getItemIndex(void)const
int CRabbitBox::getItemIndex() const { return item_index_; }

// IDA_EA: 0x8197EAA
// IDA_NAME: _ZNK10CRabbitBox12getItemCountEv
// DEMANGLED: CRabbitBox::getItemCount(void)const
int CRabbitBox::getItemCount() const { return item_count_; }

// IDA_EA: 0x89C3338
// IDA_NAME: _ZN10CRabbitBox12setItemIndexEi
// DEMANGLED: CRabbitBox::setItemIndex(int)
CRabbitBox* CRabbitBox::setItemIndex(int item_index) {
    item_index_ = item_index;
    return this;
}

// IDA_EA: 0x89C3346
// IDA_NAME: _ZN10CRabbitBox12SetItemCountEi
// DEMANGLED: CRabbitBox::SetItemCount(int)
CRabbitBox* CRabbitBox::SetItemCount(int item_count) {
    item_count_ = item_count;
    return this;
}

// IDA_EA: 0x89C3354
// IDA_NAME: _ZN10CRabbitBox11SetDropRateEi
// DEMANGLED: CRabbitBox::SetDropRate(int)
CRabbitBox* CRabbitBox::SetDropRate(int drop_rate) {
    drop_rate_ = drop_rate;
    return this;
}

// IDA_EA: 0x89C3362
// IDA_NAME: _ZN17stLimitCubeResultC2Ev
// DEMANGLED: stLimitCubeResult::stLimitCubeResult(void)
stLimitCubeResult::stLimitCubeResult()
    : item_index(0), item_count(0), probability(0) {}

// IDA_EA: 0x89D4262
// IDA_NAME: _ZN17stLimitCubeResultD2Ev
// DEMANGLED: stLimitCubeResult::~stLimitCubeResult()
stLimitCubeResult::~stLimitCubeResult() {}

// IDA_EA: 0x851571C
// IDA_NAME: _ZN15stLimitCubeInfoC2Ev
// DEMANGLED: stLimitCubeInfo::stLimitCubeInfo(void)
stLimitCubeInfo::stLimitCubeInfo()
    : condition_items(), value_0c(0), value_10(0), results() {}

// IDA_EA: 0x851466E
// IDA_NAME: _ZN15stLimitCubeInfoD2Ev
// DEMANGLED: stLimitCubeInfo::~stLimitCubeInfo()
stLimitCubeInfo::~stLimitCubeInfo() {}

// IDA_EA: 0x89C4140
// IDA_NAME: _ZN15stLimitCubeInfoaSERKS_
// DEMANGLED: stLimitCubeInfo::operator=(stLimitCubeInfo const&)
stLimitCubeInfo& stLimitCubeInfo::operator=(const stLimitCubeInfo& other) {
    condition_items = other.condition_items;
    value_0c = other.value_0c;
    value_10 = other.value_10;
    results = other.results;
    return *this;
}

// IDA_EA: 0x8513B82
// IDA_NAME: _ZN17BoosterSelectInfoC2Ev
// DEMANGLED: BoosterSelectInfo::BoosterSelectInfo(void)
BoosterSelectInfo::BoosterSelectInfo()
    : value_00(0), value_04(0), value_08(0), filenames(), enabled(0) {
    filenames.clear();
}

// IDA_EA: 0x8514658
// IDA_NAME: _ZN17BoosterSelectInfoD2Ev
// DEMANGLED: BoosterSelectInfo::~BoosterSelectInfo()
BoosterSelectInfo::~BoosterSelectInfo() {}

// IDA_EA: 0x8515128
// IDA_NAME: _ZN17BoosterSelectInfoaSERKS_
// DEMANGLED: BoosterSelectInfo::operator=(BoosterSelectInfo const&)
BoosterSelectInfo& BoosterSelectInfo::operator=(
    const BoosterSelectInfo& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    value_08 = other.value_08;
    filenames = other.filenames;
    enabled = other.enabled;
    return *this;
}

// IDA_EA: 0x8513B6E
// IDA_NAME: _ZN22AvatarOptionChangeTypeC2Ev
// DEMANGLED: AvatarOptionChangeType::AvatarOptionChangeType(void)
AvatarOptionChangeType::AvatarOptionChangeType() { clear(); }

// IDA_EA: 0x8513B48
// IDA_NAME: _ZN22AvatarOptionChangeType5clearEv
// DEMANGLED: AvatarOptionChangeType::clear(void)
AvatarOptionChangeType* AvatarOptionChangeType::clear() {
    option_2 = -1;
    option_1 = option_2;
    option_0 = option_1;
    return this;
}

// IDA_EA: 0x854B268
// IDA_NAME: _ZN21STEnchantSystemMapKeyC2Ev
// DEMANGLED: STEnchantSystemMapKey::STEnchantSystemMapKey(void)
STEnchantSystemMapKey::STEnchantSystemMapKey()
    : STEnchantSystemData(), job(0) {}

// IDA_EA: 0x8513B2E
// IDA_NAME: _ZN21STEnchantSystemMapKeyD2Ev
// DEMANGLED: STEnchantSystemMapKey::~STEnchantSystemMapKey()
STEnchantSystemMapKey::~STEnchantSystemMapKey() {}

// IDA_EA: 0x854B1C8
// IDA_NAME: _ZNK21STEnchantSystemMapKeyltERKS_
// DEMANGLED: STEnchantSystemMapKey::operator<(STEnchantSystemMapKey const&)const
bool STEnchantSystemMapKey::operator<(
    const STEnchantSystemMapKey& other) const {
    if (value_00 != other.value_00) {
        return value_00 < other.value_00;
    }
    if (value_04 != other.value_04) {
        return value_04 < other.value_04;
    }
    if (value_08 != other.value_08) {
        return value_08 < other.value_08;
    }
    return job < other.job;
}

// IDA_EA: 0x89C3384
// IDA_NAME: _ZN22STEnchantSystemMapDataC2Ev
// DEMANGLED: STEnchantSystemMapData::STEnchantSystemMapData(void)
STEnchantSystemMapData::STEnchantSystemMapData()
    : skill_data(), skill_explain() {
    skill_data.clear();
    skill_explain.clear();
}

// IDA_EA: 0x8527D54
// IDA_NAME: _ZN22STEnchantSystemMapDataC2ERKS_
// DEMANGLED: STEnchantSystemMapData::STEnchantSystemMapData(STEnchantSystemMapData const&)
STEnchantSystemMapData::STEnchantSystemMapData(
    const STEnchantSystemMapData& other)
    : skill_data(other.skill_data), skill_explain(other.skill_explain) {}

// IDA_EA: 0x85218CC
// IDA_NAME: _ZN22STEnchantSystemMapDataD2Ev
// DEMANGLED: STEnchantSystemMapData::~STEnchantSystemMapData()
STEnchantSystemMapData::~STEnchantSystemMapData() {}

// IDA_EA: 0x8328062
// IDA_NAME: _ZN28stStackableBoosterItemInfo_tD2Ev
// DEMANGLED: stStackableBoosterItemInfo_t::~stStackableBoosterItemInfo_t()
stStackableBoosterItemInfo_t::~stStackableBoosterItemInfo_t() {}

// IDA_EA: 0x89AEE9C
// IDA_NAME: _ZN27stStackableBoosterElement_tC2Ev
// DEMANGLED: stStackableBoosterElement_t::stStackableBoosterElement_t(void)
stStackableBoosterElement_t::stStackableBoosterElement_t() : items() {}

// IDA_EA: 0x89C2AAC
// IDA_NAME: _ZN27stStackableBoosterElement_tC2ERKS_
// DEMANGLED: stStackableBoosterElement_t::stStackableBoosterElement_t(stStackableBoosterElement_t const&)
stStackableBoosterElement_t::stStackableBoosterElement_t(
    const stStackableBoosterElement_t& other)
    : category(other.category),
      item_count(other.item_count),
      total_rate(other.total_rate),
      items(other.items) {}

// IDA_EA: 0x8513930
// IDA_NAME: _ZN27stStackableBoosterElement_tD2Ev
// DEMANGLED: stStackableBoosterElement_t::~stStackableBoosterElement_t()
stStackableBoosterElement_t::~stStackableBoosterElement_t() {}

// IDA_EA: 0x89C2AEE
// IDA_NAME: _ZN27stStackableBoosterElement_taSERKS_
// DEMANGLED: stStackableBoosterElement_t::operator=(stStackableBoosterElement_t const&)
stStackableBoosterElement_t& stStackableBoosterElement_t::operator=(
    const stStackableBoosterElement_t& other) {
    category = other.category;
    item_count = other.item_count;
    total_rate = other.total_rate;
    items = other.items;
    return *this;
}

// IDA_EA: 0x85156D6
// IDA_NAME: _ZN20stStackableBooster_tC2Ev
// DEMANGLED: stStackableBooster_t::stStackableBooster_t(void)
stStackableBooster_t::stStackableBooster_t() : elements(), category_items() {}

// IDA_EA: 0x8514612
// IDA_NAME: _ZN20stStackableBooster_tD2Ev
// DEMANGLED: stStackableBooster_t::~stStackableBooster_t()
stStackableBooster_t::~stStackableBooster_t() {}

// IDA_EA: 0x85150F2
// IDA_NAME: _ZN20stStackableBooster_taSERKS_
// DEMANGLED: stStackableBooster_t::operator=(stStackableBooster_t const&)
stStackableBooster_t& stStackableBooster_t::operator=(
    const stStackableBooster_t& other) {
    elements = other.elements;
    category_items = other.category_items;
    return *this;
}

// IDA_EA: 0x89C2832
// IDA_NAME: _ZN20stStackableBooster_t8tag2TypeEPKc
// DEMANGLED: stStackableBooster_t::tag2Type(char const*)
int stStackableBooster_t::tag2Type(const char* tag) {
    if (strcmp(tag, "[avatar]") == 0) return 0;
    if (strcmp(tag, "[special avatar]") == 0) return 1;
    if (strcmp(tag, "[cera]") == 0) return 2;
    if (strcmp(tag, "[creature]") == 0) return 3;
    if (strcmp(tag, "[equipment]") == 0) return 4;
    if (strcmp(tag, "[stackable]") == 0) return 5;
    if (strcmp(tag, "[etc]") == 0) return 6;
    if (strcmp(tag, "[special creature]") == 0) return 7;
    if (strcmp(tag, "[emblem]") == 0) return 8;
    return 9;
}

// IDA_EA: 0x89C295A
// IDA_NAME: _ZN20stStackableBooster_t11tag2EndTypeEPKc
// DEMANGLED: stStackableBooster_t::tag2EndType(char const*)
int stStackableBooster_t::tag2EndType(const char* tag) {
    if (strcmp(tag, "[/avatar]") == 0) return 0;
    if (strcmp(tag, "[/special avatar]") == 0) return 1;
    if (strcmp(tag, "[/cera]") == 0) return 2;
    if (strcmp(tag, "[/creature]") == 0) return 3;
    if (strcmp(tag, "[/equipment]") == 0) return 4;
    if (strcmp(tag, "[/stackable]") == 0) return 5;
    if (strcmp(tag, "[/etc]") == 0) return 6;
    if (strcmp(tag, "[/special creature]") == 0) return 7;
    if (strcmp(tag, "[/emblem]") == 0) return 8;
    return 9;
}

// IDA_EA: 0x89C3076
// IDA_NAME: _ZN20stStackableBooster_t5clearEv
// DEMANGLED: stStackableBooster_t::clear(void)
stStackableBooster_t* stStackableBooster_t::clear() {
    elements.clear();
    category_items.clear();
    return this;
}
