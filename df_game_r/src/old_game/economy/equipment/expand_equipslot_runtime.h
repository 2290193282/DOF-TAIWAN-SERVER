#ifndef OLDSRV_OLD_GAME_INVENTORY_EXPAND_EQUIPSLOT_RUNTIME_H_
#define OLDSRV_OLD_GAME_INVENTORY_EXPAND_EQUIPSLOT_RUNTIME_H_

#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// IDA unit: 04_character_select_create/CExpandEquipslot.
// The old i386 layout reuses charac_expand::CData's tail padding: the three
// twelve-item tables begin at +0x005, +0x2E1, and +0x5BD respectively.
class CExpandEquipslot : public charac_expand::CData {
public:
    CExpandEquipslot();
    virtual ~CExpandEquipslot();

    const Inven_Item* getExpandEquipslotR(
        ENUM_EQUIPSLOT equip_slot) const;
    Inven_Item* getExpandEquipslotW(ENUM_EQUIPSLOT equip_slot);

    int EquipslotSwitch(CUser* user,
                        char requested_switch,
                        char channel_switch,
                        char expand_switch);
    int Send_Equip_Info(CUser* user);
    charac_expand::CData* SetExpandEquipslot(int equip_slot,
                                             void* source,
                                             int bytes);
    int Change_Equip(CUser* user,
                     int source_space,
                     int source_slot,
                     int target_space,
                     int target_slot);
    Inven_Item GetInvenSlot(INVEN_TYPE inventory_type, int slot) const;
    const Inven_Item* GetInvenRefR(INVEN_TYPE inventory_type,
                                    int slot) const;
    Inven_Item* GetInvenRefW(INVEN_TYPE inventory_type, int slot);
    int FindItemLock(unsigned char lock,
                     ENUM_ITEMSPACE& item_space,
                     int& slot) const;
    int Move_Item(CUser* user,
                  int source_space,
                  int source_slot,
                  int target_space,
                  int target_slot);

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;

    int Send_Expand_Equip_Info(CUser* user);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    // The anonymous union suppresses automatic array destruction, matching
    // old D2 @0x849935C, while retaining a typed Inven_Item layout.  The old
    // constructor explicitly constructs all three twelve-item ranges.
    union {
        Inven_Item expand_equipslots_[36];
    };
};

static_assert(sizeof(CExpandEquipslot) == 0x89Cu,
              "CExpandEquipslot old i386 ABI size must stay 0x89C");

#endif  // OLDSRV_OLD_GAME_INVENTORY_EXPAND_EQUIPSLOT_RUNTIME_H_
