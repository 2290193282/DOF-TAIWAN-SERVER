#include "old_data/PVF/npc/need_material_data_manager.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"

typedef char NeedMaterialDataManager_Size_Must_Be_0x20[
    (sizeof(NeedMaterialDataManager) == 0x20) ? 1 : -1];

// IDA_EA: 0x81B5A3C
// IDA_NAME: _ZN23NeedMaterialDataManagerC2Ev
// DEMANGLED: NeedMaterialDataManager::NeedMaterialDataManager(void)
NeedMaterialDataManager::NeedMaterialDataManager()
    : need_materials_(), default_need_material_(-1, 0) {}

// IDA_EA: 0x81B5A9E
// IDA_NAME: _ZN23NeedMaterialDataManagerD2Ev
// DEMANGLED: NeedMaterialDataManager::~NeedMaterialDataManager()
NeedMaterialDataManager::~NeedMaterialDataManager() {}

// IDA_EA: 0x81B5AB2
// IDA_NAME: _ZN23NeedMaterialDataManager11getInstanceEv
// DEMANGLED: NeedMaterialDataManager::getInstance(void)
NeedMaterialDataManager* NeedMaterialDataManager::getInstance() {
    static NeedMaterialDataManager inst;
    return &inst;
}

// IDA_EA: 0x81B5B46
// IDA_NAME: _ZN23NeedMaterialDataManager19setNeedMaterialInfoEiRKSt4pairIiiE
// DEMANGLED: NeedMaterialDataManager::setNeedMaterialInfo(int,std::pair<int,int> const&)
int NeedMaterialDataManager::setNeedMaterialInfo(
    int item_id,
    const NeedMaterialPair& need_material) {
    need_materials_[item_id] = need_material;
    return need_material.first;
}

// IDA_EA: 0x81B5B70
// IDA_NAME: _ZNK23NeedMaterialDataManager15getNeedMaterialEPK5CItem
// DEMANGLED: NeedMaterialDataManager::getNeedMaterial(CItem const*)const
const NeedMaterialDataManager::NeedMaterialPair*
NeedMaterialDataManager::getNeedMaterial(const CItem* item) const {
    if (item == 0) {
        return &default_need_material_;
    }

    const int item_index = item->get_index();
    std::map<int, NeedMaterialPair>::const_iterator found =
        need_materials_.find(item_index);
    if (found != need_materials_.end()) {
        return &found->second;
    }
    return &default_need_material_;
}

// IDA_EA: 0x81B5BF2
// IDA_NAME: _ZNK23NeedMaterialDataManager15getNeedMaterialEi
// DEMANGLED: NeedMaterialDataManager::getNeedMaterial(int)const
const NeedMaterialDataManager::NeedMaterialPair*
NeedMaterialDataManager::getNeedMaterial(int item_id) const {
    CDataManager* data_manager = G_CDataManager();
    CItem* item = data_manager->find_item(item_id);
    return getNeedMaterial(item);
}
