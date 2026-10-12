#include "old_data/PVF/pvp_mission/combo_script_data.h"

typedef char combo_script_data_Size_Must_Be_0x14[
    (sizeof(combo_script_data) == 0x14) ? 1 : -1];

// IDA_EA: 0x8A5BD94
// IDA_NAME: _ZN17combo_script_dataC2Ev
// DEMANGLED: combo_script_data::combo_script_data(void)
combo_script_data::combo_script_data()
    : std::vector<std::pair<int, int> >(), value_range() {
    clear();
}

// IDA_EA: 0x85EBEB2
// IDA_NAME: _ZN17combo_script_dataC2ERKS_
// DEMANGLED: combo_script_data::combo_script_data(combo_script_data const&)
combo_script_data::combo_script_data(const combo_script_data& rhs)
    : std::vector<std::pair<int, int> >(rhs), value_range(rhs.value_range) {}

// IDA_EA: 0x85EAA88
// IDA_NAME: _ZN17combo_script_dataD2Ev
// DEMANGLED: combo_script_data::~combo_script_data()
combo_script_data::~combo_script_data() {}

// IDA_EA: 0x8A5A7F4
// IDA_NAME: _ZN17combo_script_data5clearEv
// DEMANGLED: combo_script_data::clear(void)
combo_script_data* combo_script_data::clear() {
    std::vector<std::pair<int, int> >::clear();
    value_range.first = 0;
    value_range.second = 0;
    return this;
}
