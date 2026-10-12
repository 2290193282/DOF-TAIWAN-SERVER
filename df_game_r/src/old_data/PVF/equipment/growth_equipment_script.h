#ifndef OLDSRV_DATAMANAGER_GROWTH_EQUIPMENT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_GROWTH_EQUIPMENT_SCRIPT_H_

#include <stdint.h>

#include <map>


// IDA_EA: 0x8372FBC, 0x837C3C0, 0x895EB3F.
class growthEquipmentScript {
public:
    growthEquipmentScript();
    ~growthEquipmentScript();

    std::map<int, int> reward_items;
    int32_t gift_growth_weapon_box;
    int32_t growth_material_item;
    uint16_t growth_material_item_max;
    uint16_t event_bubble_max;
    uint16_t event_sand_max;

private:
    growthEquipmentScript(const growthEquipmentScript&);
    growthEquipmentScript& operator=(const growthEquipmentScript&);
};

int importGrowthEquipmentScript(growthEquipmentScript* script,
                                const char* filename);

static_assert(sizeof(growthEquipmentScript) == 0x28u,
              "growthEquipmentScript IDA ABI size must stay 0x28");

#endif  // OLDSRV_DATAMANAGER_GROWTH_EQUIPMENT_SCRIPT_H_
