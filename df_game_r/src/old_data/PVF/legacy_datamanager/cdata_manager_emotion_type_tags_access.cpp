#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/etc/emotion_type_tags.h"

// This is deliberately a non-counted typed reconstruction boundary.  The old
// server directly addressed the embedded member and exports no corresponding
// CDataManager accessor symbol.
const stEmotionTypeTagsInfo* CDataManager::get_emotion_type_tags_info() const {
    return &emotion_type_tags_info_;
}
