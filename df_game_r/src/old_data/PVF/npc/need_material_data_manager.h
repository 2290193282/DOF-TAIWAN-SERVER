#ifndef OLDSRV_OLD_DATA_PVF_NPC_NEED_MATERIAL_DATA_MANAGER_H_
#define OLDSRV_OLD_DATA_PVF_NPC_NEED_MATERIAL_DATA_MANAGER_H_

#include <map>
#include <utility>

#include "old_game/economy/inventory/item.h"

class NeedMaterialDataManager {
public:
    typedef std::pair<int, int> NeedMaterialPair;

    NeedMaterialDataManager();
    ~NeedMaterialDataManager();

    static NeedMaterialDataManager* getInstance();

    int setNeedMaterialInfo(int item_id,
                            const NeedMaterialPair& need_material);
    const NeedMaterialPair* getNeedMaterial(const CItem* item) const;
    const NeedMaterialPair* getNeedMaterial(int item_id) const;

private:
    std::map<int, NeedMaterialPair> need_materials_;
    NeedMaterialPair default_need_material_;

    NeedMaterialDataManager(const NeedMaterialDataManager&);
    NeedMaterialDataManager& operator=(const NeedMaterialDataManager&);
};

#endif  // OLDSRV_OLD_DATA_PVF_NPC_NEED_MATERIAL_DATA_MANAGER_H_
