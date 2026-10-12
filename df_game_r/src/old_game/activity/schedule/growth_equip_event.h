#ifndef OLDSRV_OLD_GAME_EVENT_GROWTH_EQUIP_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_GROWTH_EQUIP_EVENT_H_

#include "old_game/activity/reward/event_base.h"


class CUser;
struct Inven_Item;

// IDA unit: 06_userinfo_stat_inventory/GrowthEquipEvent.  The raw C2 has
// only the CEventBase construction and vptr installation: there is no
// derived storage to model here.  Retaining the real inheritance makes the
// CEventManager repeat-event downcast explicit without introducing an offset
// bridge.
class GrowthEquipEvent : public CEventBase {
public:
    // IDA_EA: 0x80F6EF0
    // IDA_NAME: _ZN16GrowthEquipEventC2Ev
    // DEMANGLED: GrowthEquipEvent::GrowthEquipEvent(void)
    GrowthEquipEvent();

    // IDA_EA: 0x80F6F0C / 0x80F6F3C (D2 / deleting D0)
    // IDA_NAME: _ZN16GrowthEquipEventD2Ev / _ZN16GrowthEquipEventD0Ev
    virtual ~GrowthEquipEvent();

    // The event-table object has no derived data, but its five virtual
    // overrides are required for the original vtable and lifecycle.
    // IDA_EA: 0x80F6F5A / 0x80F6FFE / 0x80F6FAC / 0x80F7710 / 0x80F7820
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();
    virtual int IsEventing(CUser* user) const;

    // IDA_EA: 0x80F7050
    // IDA_NAME: _ZNK16GrowthEquipEvent19GiftGrowthWeaponBoxER5CUser
    // DEMANGLED: GrowthEquipEvent::GiftGrowthWeaponBox(CUser &)const
    int GiftGrowthWeaponBox(CUser& user) const;

    // IDA_EA: 0x80F7226 / 0x80F73BE
    // IDA_NAME: _ZN16GrowthEquipEvent25SendGrowthWeaponEventInfoER5CUser /
    //           _ZN16GrowthEquipEvent18SelectGrowthWeaponER5CUserii
    void SendGrowthWeaponEventInfo(CUser& user);
    void SelectGrowthWeapon(CUser& user, int source_item_id, int result_item_id);

    // IDA_EA: 0x80F74B2
    // IDA_NAME: _ZNK16GrowthEquipEvent24SendInfinityGrowthWeaponEP5CUserPK10Inven_Itemii
    // DEMANGLED: GrowthEquipEvent::SendInfinityGrowthWeapon(CUser *,Inven_Item const*,int,int)const
    int SendInfinityGrowthWeapon(CUser* user,
                                 const Inven_Item* source,
                                 int item_space,
                                 int slot) const;
};

#if defined(__i386__)
static_assert(sizeof(GrowthEquipEvent) == 0x0Cu,
              "GrowthEquipEvent must retain its IDA 0x0C event ABI");
#endif


#endif  // OLDSRV_OLD_GAME_EVENT_GROWTH_EQUIP_EVENT_H_
