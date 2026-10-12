#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_DATA_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_DATA_H_

#include <utility>
#include <vector>

struct combo_script_data : public std::vector<std::pair<int, int> > {
    combo_script_data();
    combo_script_data(const combo_script_data& rhs);
    ~combo_script_data();

    combo_script_data* clear();

    std::pair<int, int> value_range;
};

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_DATA_H_
