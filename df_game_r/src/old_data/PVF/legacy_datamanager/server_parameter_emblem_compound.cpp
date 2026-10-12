#include "old_data/PVF/data/datamanager/server_parameter_script.h"

// IDA_EA: importServerParameterScript @0x8A870B7.  The importer writes one
// 0x18-byte [emblem compound info] row per grade beginning at +0x7C0.
// This accessor deliberately stays in the script owner so gameplay never
// reinterprets the enclosing CDataManager layout.
const EmblemCompoundInfo*
ServerParameterScript::get_emblem_compound_info(int grade) const {
    if (grade < 0 || grade > 6) {
        return 0;
    }
    return &emblem_compound_info_[grade];
}

// IDA_EA: CUser::getCompoundUpgrade @0x8682082.
// The enclosing CDataManager embeds ServerParameterScript at +0x68, making
// the four original CDataManager addresses +0x08D0/+0x18D0/+0x28D0/+0x38D0.
// Keep table selection in the physical owner instead of projecting those
// addresses as fictitious CDataManager members.
const CompoundUpgradeRateRow&
ServerParameterScript::get_recipe_compound_upgrade_rates(
    bool use_second_table,
    bool amplified_material,
    unsigned int equipment_upgrade) const {
    if (use_second_table) {
        return amplified_material
                   ? recipe_amplify_table_second_[equipment_upgrade]
                   : recipe_upgrade_table_second_[equipment_upgrade];
    }
    return amplified_material
               ? recipe_amplify_table_[equipment_upgrade]
               : recipe_upgrade_table_[equipment_upgrade];
}
