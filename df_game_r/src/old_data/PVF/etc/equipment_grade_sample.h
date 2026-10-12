#ifndef OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GRADE_SAMPLE_H_
#define OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GRADE_SAMPLE_H_

#include <stddef.h>

#include <vector>


// IDA_EA: 0x837C810, 0x837C884, 0x890E18D.
// CDataManager embeds this value at +0x4D5C in the old server.
struct stEquipmentGradeSample {
    stEquipmentGradeSample();
    ~stEquipmentGradeSample();

    std::vector<unsigned int> grades[5];
};

// IDA_NAME: _Z30importEquipmentGradeSampleInfoPKcP22stEquipmentGradeSample
// DEMANGLED: importEquipmentGradeSampleInfo(char const*,stEquipmentGradeSample *)
int importEquipmentGradeSampleInfo(const char* filename,
                                   stEquipmentGradeSample* sample);

static_assert(sizeof(stEquipmentGradeSample) == 0x3cu,
              "stEquipmentGradeSample IDA ABI size must stay 0x3c");
static_assert(offsetof(stEquipmentGradeSample, grades) == 0x00u,
              "stEquipmentGradeSample grade array must start at +0x00");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GRADE_SAMPLE_H_
