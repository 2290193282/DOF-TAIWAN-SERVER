#include "old_game/player/core/cuser.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"

// IDA_EA: 0x8656C12
// IDA_NAME: _ZN5CUser20DimensionInoutUpdateEbb
// DEMANGLED: CUser::DimensionInoutUpdate(bool,bool)
char CUser::DimensionInoutUpdate(bool update_dimension, bool update_ultimate) {
    char result = 0;
    for (int dimension = 0;; ++dimension) {
        result = static_cast<char>(dimension <= 5);
        if (dimension > 5) {
            break;
        }

        if (update_dimension) {
            CDataManager* const data_manager = G_CDataManager();
            const char inout_value = static_cast<char>(
                data_manager->get_dimensionInout(dimension));
            setDemensionInoutValue(static_cast<char>(dimension), inout_value);
        }
    }

    if (update_ultimate) {
        CDataManager* const data_manager = G_CDataManager();
        CDungeon* const black_dragon = data_manager
            ? data_manager->find_dungeon(122)
            : 0;
        setUltimateInoutValue(
            0,
            static_cast<char>(black_dragon
                                  ? black_dragon->get_limit_Inout_count()
                                  : 0));
        const char limit = static_cast<char>(
            data_manager->get_limit_inout_count(2));
        return static_cast<char>(setUltimateInoutValue(2, limit));
    }

    return result;
}
