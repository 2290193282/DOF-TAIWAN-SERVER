#include "old_data/PVF/n_quest/qp_guide_recommend_data.h"

typedef char stRecommendQPData_Size_Must_Be_0x14[
    (sizeof(stQPGuideScript::stRecommendQPData) == 0x14) ? 1 : -1];

// IDA_EA: 0x891BA22
// IDA_NAME: _ZN15stQPGuideScript17stRecommendQPDataC2Ev
// DEMANGLED: stQPGuideScript::stRecommendQPData::stRecommendQPData(void)
stQPGuideScript::stRecommendQPData::stRecommendQPData()
    : values() {
    clear();
}

// IDA_EA: 0x891BA6A
// IDA_NAME: _ZN15stQPGuideScript17stRecommendQPDataD2Ev
// DEMANGLED: stQPGuideScript::stRecommendQPData::~stRecommendQPData()
stQPGuideScript::stRecommendQPData::~stRecommendQPData() {}

// IDA_EA: 0x891BA80
// IDA_NAME: _ZN15stQPGuideScript17stRecommendQPData5clearEv
// DEMANGLED: stQPGuideScript::stRecommendQPData::clear(void)
bool stQPGuideScript::stRecommendQPData::clear() {
    value_00 = -1;
    value_04 = -1;
    values.clear();
    values.reserve(4);
    // Preserve the official mutations while making the reconstructed C++
    // return value deterministic for the strict 0828 importer.
    return true;
}

// IDA_EA: 0x892F730
// IDA_NAME: _ZN15stQPGuideScript17stRecommendQPDataC2ERKS0_
// DEMANGLED: stQPGuideScript::stRecommendQPData::stRecommendQPData(stQPGuideScript::stRecommendQPData const&)
stQPGuideScript::stRecommendQPData::stRecommendQPData(
    const stRecommendQPData& other)
    : value_00(other.value_00), value_04(other.value_04), values(other.values) {}

// IDA_EA: 0x892F7EA
// IDA_NAME: _ZN15stQPGuideScript17stRecommendQPDataaSERKS0_
// DEMANGLED: stQPGuideScript::stRecommendQPData::operator=(stQPGuideScript::stRecommendQPData const&)
stQPGuideScript::stRecommendQPData&
stQPGuideScript::stRecommendQPData::operator=(
    const stRecommendQPData& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    values = other.values;
    return *this;
}
