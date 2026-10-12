#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/etc/equipment_grade_sample.h"

const stEquipmentGradeSample* CDataManager::get_equipment_grade_sample() const {
    return &equipment_grade_sample_;
}
