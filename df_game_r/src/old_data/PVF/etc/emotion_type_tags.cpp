#include "old_data/PVF/etc/emotion_type_tags.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

typedef char stOneEmotionData_Size_Must_Be_0x24[
    (sizeof(stOneEmotionData) == 0x24) ? 1 : -1];
typedef char stEmotionTypeTagsInfo_Size_Must_Be_0x18[
    (sizeof(stEmotionTypeTagsInfo) == 0x18) ? 1 : -1];

// IDA_EA: 0x8370BB0
// IDA_NAME: _ZN16stOneEmotionDataD2Ev
// DEMANGLED: stOneEmotionData::~stOneEmotionData()
stOneEmotionData::~stOneEmotionData() {}

// IDA_EA: 0x891B560
// IDA_NAME: _ZN16stOneEmotionDataC2Ev
// DEMANGLED: stOneEmotionData::stOneEmotionData(void)
stOneEmotionData::stOneEmotionData()
    : value_08(), value_0c(), value_14(), values_18() {
    clear();
}

// IDA_EA: 0x891B61A
// IDA_NAME: _ZN16stOneEmotionData5clearEv
// DEMANGLED: stOneEmotionData::clear(void)
int stOneEmotionData::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08.clear();
    value_0c.clear();
    value_10 = false;
    value_11 = false;
    value_12 = false;
    value_14.clear();
    values_18.clear();
    // The old function returns after vector::clear without assigning EAX.
}

// IDA_EA: 0x892CE7C
// IDA_NAME: _ZN16stOneEmotionDataC2ERKS_
// DEMANGLED: stOneEmotionData::stOneEmotionData(stOneEmotionData const&)
stOneEmotionData::stOneEmotionData(const stOneEmotionData& other)
    : value_00(other.value_00),
      value_04(other.value_04),
      value_08(other.value_08),
      value_0c(other.value_0c),
      value_10(other.value_10),
      value_11(other.value_11),
      value_12(other.value_12),
      value_14(other.value_14),
      values_18(other.values_18) {}

// IDA_EA: 0x892CFFC
// IDA_NAME: _ZN16stOneEmotionDataaSERKS_
// DEMANGLED: stOneEmotionData::operator=(stOneEmotionData const&)
stOneEmotionData& stOneEmotionData::operator=(
    const stOneEmotionData& other) {
    value_00 = other.value_00;
    value_04 = other.value_04;
    value_08 = other.value_08;
    value_0c = other.value_0c;
    value_10 = other.value_10;
    value_11 = other.value_11;
    value_12 = other.value_12;
    value_14 = other.value_14;
    values_18 = other.values_18;
    return *this;
}

// IDA_EA: 0x8370C4C
// IDA_NAME: _ZN21stEmotionTypeTagsInfoC2Ev
// DEMANGLED: stEmotionTypeTagsInfo::stEmotionTypeTagsInfo(void)
stEmotionTypeTagsInfo::stEmotionTypeTagsInfo()
    : emotion_types(), emotion_data() {
    clear();
}

// IDA_EA: 0x8370CB4
// IDA_NAME: _ZN21stEmotionTypeTagsInfoD2Ev
// DEMANGLED: stEmotionTypeTagsInfo::~stEmotionTypeTagsInfo()
stEmotionTypeTagsInfo::~stEmotionTypeTagsInfo() {}

// IDA_EA: 0x8370CFA
// IDA_NAME: _ZN21stEmotionTypeTagsInfo5clearEv
// DEMANGLED: stEmotionTypeTagsInfo::clear(void)
int stEmotionTypeTagsInfo::clear() {
    emotion_types.clear();
    emotion_data.clear();
    // The old function returns after vector::clear without assigning EAX.
}

// IDA_EA: 0x890DE85
// IDA_NAME: _Z27importEmotionTypeTagsScriptPKcP21stEmotionTypeTagsInfo
// DEMANGLED: importEmotionTypeTagsScript(char const*,stEmotionTypeTagsInfo *)
int importEmotionTypeTagsScript(const char* filename,
                                stEmotionTypeTagsInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string unused;
    (void)unused;
    bool success = false;
    if (!info) {
        return 0;
    }

    while (ScanType(tag, true) == 1) {
        if (tag == "[EMOTION INFO]") {
            int row_ok = 0;
            do {
                stOneEmotionData data;
                std::string text;

                int value = ScanInt(&success);
                if (success) {
                    info->emotion_types.push_back(value);
                    data.value_00 = value;

                    data.value_04 = ScanInt(&success);
                    if (success) {
                        if (ScanStr(&data.value_08) == 1) {
                            if (ScanStr(&data.value_0c) == 1) {
                                value = ScanInt(&success);
                                if (success) {
                                    data.value_10 = value == 1;
                                    value = ScanInt(&success);
                                    if (success) {
                                        data.value_11 = value == 1;
                                        value = ScanInt(&success);
                                        if (success) {
                                            data.value_12 = value == 1;
                                            if (ScanStr(&data.value_14) == 1) {
                                                while (ScanStr(&text) == 1 &&
                                                       text != "$END") {
                                                    data.values_18.push_back(
                                                        text);
                                                }
                                                info->emotion_data.push_back(
                                                    data);
                                                row_ok = 1;
                                            } else {
                                                row_ok = 0;
                                            }
                                        } else {
                                            row_ok = 0;
                                        }
                                    } else {
                                        row_ok = 0;
                                    }
                                } else {
                                    row_ok = 0;
                                }
                            } else {
                                row_ok = 0;
                            }
                        } else {
                            row_ok = 0;
                        }
                    } else {
                        row_ok = 0;
                    }
                } else {
                    row_ok = 0;
                }
            } while (row_ok);
        }
    }

    return 1;
}
