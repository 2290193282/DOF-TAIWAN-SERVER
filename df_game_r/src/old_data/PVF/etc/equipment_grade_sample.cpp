#include "old_data/PVF/etc/equipment_grade_sample.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>


// IDA_EA: 0x837C810
// IDA_NAME: _ZN22stEquipmentGradeSampleC2Ev
// DEMANGLED: stEquipmentGradeSample::stEquipmentGradeSample(void)
stEquipmentGradeSample::stEquipmentGradeSample() {}

// IDA_EA: 0x837C884
// IDA_NAME: _ZN22stEquipmentGradeSampleD2Ev
// DEMANGLED: stEquipmentGradeSample::~stEquipmentGradeSample()
stEquipmentGradeSample::~stEquipmentGradeSample() {}

// IDA_EA: 0x890E18D
// IDA_NAME: _Z30importEquipmentGradeSampleInfoPKcP22stEquipmentGradeSample
// DEMANGLED: importEquipmentGradeSampleInfo(char const*,stEquipmentGradeSample *)
int importEquipmentGradeSampleInfo(const char* filename,
                                   stEquipmentGradeSample* sample) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string unused;
    (void)unused;
    bool success = false;
    if (sample) {
        while (ScanType(tag, true) == 1) {
            if (tag == "[grade_0]") {
                while (1) {
                    const unsigned int value =
                        static_cast<unsigned int>(ScanInt(&success));
                    if (!success) {
                        break;
                    }
                    sample->grades[0].push_back(value);
                }
            } else if (tag == "[grade_1]") {
                while (1) {
                    const unsigned int value =
                        static_cast<unsigned int>(ScanInt(&success));
                    if (!success) {
                        break;
                    }
                    sample->grades[1].push_back(value);
                }
            } else if (tag == "[grade_2]") {
                while (1) {
                    const unsigned int value =
                        static_cast<unsigned int>(ScanInt(&success));
                    if (!success) {
                        break;
                    }
                    sample->grades[2].push_back(value);
                }
            } else if (tag == "[grade_3]") {
                while (1) {
                    const unsigned int value =
                        static_cast<unsigned int>(ScanInt(&success));
                    if (!success) {
                        break;
                    }
                    sample->grades[3].push_back(value);
                }
            } else if (tag == "[grade_4]") {
                while (1) {
                    const unsigned int value =
                        static_cast<unsigned int>(ScanInt(&success));
                    if (!success) {
                        break;
                    }
                    sample->grades[4].push_back(value);
                }
            }
        }
        return 1;
    }

    return 0;
}
