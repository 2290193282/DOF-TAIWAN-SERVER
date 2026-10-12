#ifndef OLDSRV_OLD_DATA_PVF_EQUIPMENT_ITEM_UPGRADE_H_
#define OLDSRV_OLD_DATA_PVF_EQUIPMENT_ITEM_UPGRADE_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "old_data/PVF/equipment/upgrade_table.h"
#include "infra/network/packet/message_base.h"

class CMTRand;
class CItem;
class CUser;
class PacketGuard;
enum ENUM_ERROR : int;
struct EquipmentSkillDataUp;
struct Inven_Item;
struct STEnchantSystemMapKey;

// IDA_EA: MSG_ITEM_UPGRADE::C2 @0x822DCEA.  The original constructor first
// invokes MSG_BASE::C2, then writes only +0x25, +0x27 and +0x28.  Therefore
// the 13-byte prelude below is a real, deliberately unconstructed portion of
// this request object; it is not a synthetic tail or a replacement for a
// typed packet body.  Every byte that participates in the two item-upgrade
// transactions is named below with its observed wire meaning.
#pragma pack(push, 1)
struct MSG_ITEM_UPGRADE_BODY {
    uint8_t unconstructed_0d_19[0x0d];  // IDA +0x0d..+0x19, C2 leaves intact
    uint8_t mode;                       // IDA +0x1a
    uint16_t target_slot;               // IDA +0x1b
    uint32_t target_item_id;            // IDA +0x1d
    uint16_t ticket_slot;               // IDA +0x21
    uint16_t material_slot;             // IDA +0x23
    uint16_t upgrade_count;             // IDA +0x25
    uint8_t upgrade_rate;               // IDA +0x27
    char guild_item_name[0x81];         // IDA +0x28
};

struct MSG_ITEM_UPGRADE : public MSG_BASE {
    // IDA_EA: 0x822DCEA
    // IDA_NAME: _ZN16MSG_ITEM_UPGRADEC2Ev
    // DEMANGLED: MSG_ITEM_UPGRADE::MSG_ITEM_UPGRADE(void)
    MSG_ITEM_UPGRADE();

    MSG_ITEM_UPGRADE_BODY body_;  // starts at IDA +0x0d
};

// IDA_EA: 0x822DC04.  CMD371 deliberately uses this compact 0x1a-byte
// request, rather than the 0xa9-byte normal upgrade request above.  The
// third-chronicle CItemUpgrade overload consumes exactly these five fields;
// keeping the two wire records distinct prevents callers from reading the
// normal upgrade's protection / fixed-rate tail from an unrelated packet.
struct MSG_ENCHANT : public MSG_BASE {
    MSG_ENCHANT();

    uint16_t target_slot;       // IDA +0x0d
    uint32_t target_item_id;    // IDA +0x0f
    uint16_t material_slot;     // IDA +0x13
    uint32_t material_item_id;  // IDA +0x15
    uint8_t enchant_type;       // IDA +0x19
};

struct STUdateItemParam : public ParamBase {
    // IDA_EA: 0x8231048
    // IDA_NAME: _ZN16STUdateItemParamC2Ev
    // DEMANGLED: STUdateItemParam::STUdateItemParam(void)
    STUdateItemParam();

    // IDA_EA: 0x8247A98 / 0x8247AC8
    // IDA_NAME: _ZN16STUdateItemParamD2Ev / _ZN16STUdateItemParamD0Ev
    virtual ~STUdateItemParam();

    int result;  // IDA +0x04
};
#pragma pack(pop)

static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, mode) == 0x0du,
              "upgrade message mode must stay at IDA +0x1a");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, target_slot) == 0x0eu,
              "upgrade message target slot must stay at IDA +0x1b");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, target_item_id) == 0x10u,
              "upgrade message target item id must stay at IDA +0x1d");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, ticket_slot) == 0x14u,
              "upgrade message protection slot must stay at IDA +0x21");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, material_slot) == 0x16u,
              "upgrade message material slot must stay at IDA +0x23");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, upgrade_count) == 0x18u,
              "upgrade message fixed count must stay at IDA +0x25");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, upgrade_rate) == 0x1au,
              "upgrade message fixed rate must stay at IDA +0x27");
static_assert(offsetof(MSG_ITEM_UPGRADE_BODY, guild_item_name) == 0x1bu,
              "upgrade message guild name must stay at IDA +0x28");
static_assert(sizeof(MSG_ITEM_UPGRADE_BODY) == 0x9cu,
              "upgrade message body must retain its 0x9c-byte IDA extent");
static_assert(sizeof(MSG_ITEM_UPGRADE) == 0xa9u,
              "upgrade message ABI size must stay 0xa9 bytes");
static_assert(offsetof(MSG_ENCHANT, target_slot) == 0x0du &&
                  offsetof(MSG_ENCHANT, target_item_id) == 0x0fu &&
                  offsetof(MSG_ENCHANT, material_slot) == 0x13u &&
                  offsetof(MSG_ENCHANT, material_item_id) == 0x15u &&
                  offsetof(MSG_ENCHANT, enchant_type) == 0x19u &&
                  sizeof(MSG_ENCHANT) == 0x1au,
              "third-chronicle enchant message must retain IDA layout");
#if defined(__i386__)
static_assert(sizeof(STUdateItemParam) == 0x08u,
              "STUdateItemParam must retain its i386 ABI size");
#endif

namespace WongWork {

// IDA_EA: ctor @0x80FC834, dtor @0x80FC842, dispatch @0x80FC850.
// The two entries beginning at each derived vptr are the item transaction and
// PVF-load functions.  No virtual destructor precedes them: item_upgrade_system
// dispatches the first entry directly at offset zero.
class IItemUpgrade {
public:
    IItemUpgrade();
    ~IItemUpgrade();

    virtual int ProcUpgrade(CUser* user,
                            MSG_ITEM_UPGRADE* message,
                            PacketGuard* packet) = 0;
    virtual int LoadUpgradeScript(const char* filename) = 0;

    void item_upgrade_system(CUser* user,
                             MSG_ITEM_UPGRADE* message,
                             int packet_id);
};

// IDA_EA: ctor @0x85461C8, dtor @0x85462C6.
// The RTTI names CItemUpgradeTable as the first declared base at +0x04 and
// IItemUpgrade as the second declared base and primary dynamic base at +0x00.
// The restored prefix remains unchanged. The target PVF appends four typed
// reinforcement vectors to CItemUpgradeTable, so CMTRand* moves to +0x520 and
// its limit to +0x524, for an overall i386 size of 0x528.
class CItemUpgrade : public CItemUpgradeTable, public IItemUpgrade {
public:
    CItemUpgrade();
    ~CItemUpgrade();

    virtual int ProcUpgrade(CUser* user,
                            MSG_ITEM_UPGRADE* message,
                            PacketGuard* packet);
    virtual int LoadUpgradeScript(const char* filename);
    int Load3rdChronicleUpgradeScript(const char* filename);
    float getUpgradeItemRepairCostRate(int upgrade, bool has_ability) const;
    // IDA_EA: 0x85491D6
    // IDA_NAME: _ZN8WongWork12CItemUpgrade20ProcUpgradeChronicleEP5CUsersjsjcRSt6vectorIiSaIiEE
    // DEMANGLED: WongWork::CItemUpgrade::ProcUpgradeChronicle(CUser *,short,unsigned int,short,unsigned int,char,std::vector<int,std::allocator<int>> &)
    int ProcUpgradeChronicle(CUser* user,
                             int16_t material_slot,
                             uint32_t material_item_id,
                             int16_t target_slot,
                             uint32_t target_item_id,
                             uint8_t grade_sample,
                             std::vector<int>& changed_slots);
    // IDA_EA: 0x854A9AA
    // IDA_NAME: _ZN8WongWork12CItemUpgrade22make3rdChroniclePacketEP5CUserRK10Inven_ItemP11PacketGuard
    // DEMANGLED: WongWork::CItemUpgrade::make3rdChroniclePacket(CUser *,Inven_Item const&,PacketGuard *)
    int make3rdChroniclePacket(CUser* user,
                               const Inven_Item& item,
                               PacketGuard* packet);
    // IDA_EA: 0x854A1BA.  CMD371's wire record has its own full transaction;
    // it is not ABI-compatible with MSG_ITEM_UPGRADE.
    int ProcUpgrade(CUser* user,
                    MSG_ENCHANT* message,
                    PacketGuard* packet);
    // IDA_EA: 0x854AC8A / 0x854AF38. CMD208's random-invest branch keeps
    // using this CItemUpgrade object and its real PVF upgrade table.
    int ProcAmplifyRandomUpgrade(CUser* user,
                                 Inven_Item& target,
                                 Inven_Item& material,
                                 int upgrade,
                                 const char* guild_item_name);
    int enableAmplifyRandomUpgrade(CUser* user,
                                   Inven_Item& target,
                                   Inven_Item& material,
                                   int& upgrade);
    // Pure CMD205 preparation on the dispatcher's inventory copy.  It does
    // not draw RNG and does not publish to the live inventory.
    int prepareAmplifyRandomUpgrade(Inven_Item& target, int upgrade);
    // Emit the official logging/notice side effects only after the prepared
    // copy has been committed to the live inventory.
    void commitAmplifyRandomUpgrade(CUser* user,
                                    const Inven_Item& target,
                                    int old_upgrade,
                                    const char* guild_item_name);

private:
    void _getAdditionalDisjointItem(
        unsigned int upgrade,
        const CItem* item,
        std::vector<std::pair<int, int> >& result_items,
        const upgrade_table_t& table);
    int _ProcPenalty(CUser* user,
                     Inven_Item* target,
                     const CItem* item,
                     uint16_t target_slot,
                     const upgrade_table_t& table,
                     const upgrade_info_t* info,
                     PacketGuard* packet,
                     Inven_Item* alternative_material,
                     char forced_penalty);
    int _DoProcUpgrade(CUser* user,
                       Inven_Item* target,
                       const upgrade_info_t* info);
    int _CheckCondition(const CUser* user,
                        const Inven_Item& target,
                        const CItem* item,
                        const Inven_Item& material,
                        const upgrade_table_t& table,
                        const upgrade_info_t& info);
    void _NoticeUpgrade(const CUser* user,
                        const Inven_Item& target,
                        bool succeeded);
    int _GetUpgradeInfo(const Inven_Item* target,
                        const CItem* item,
                        const upgrade_table_t& table,
                        upgrade_info_t* output,
                        int charac_level);
    bool _CheckTargetUsableLv(const Inven_Item& upgrade_item,
                              const Inven_Item& target) const;
    int _ProcPortableUpgradeItem(const Inven_Item& upgrade_item,
                                 const Inven_Item& target,
                                 upgrade_info_t& output) const;
    int _getUpgradeCountByUpgradeItem(const Inven_Item& target,
                                      const Inven_Item& upgrade_item,
                                      ENUM_ERROR& error);
    int _getUpgradeRateByUpgradeItem(const Inven_Item& target,
                                     const Inven_Item& upgrade_item,
                                     ENUM_ERROR& error);

    // Three named helper identities used exclusively by the CMD371
    // third-chronicle transaction.
    bool isDoEnchant(const Inven_Item& target, char* rate_data);
    void _getAdditionalDisjointItem(
        unsigned int upgrade,
        const CItem* item,
        std::vector<std::pair<int, int> >& result_items,
        const upgrade_table_t& table,
        bool has_ability);
    int _CheckCondition(const CUser* user,
                        const Inven_Item& target,
                        const CItem* item,
                        const Inven_Item& material,
                        uint8_t enchant_type);
    int _ProcPenalty(CUser* user,
                     Inven_Item* target,
                     const CItem* item,
                     uint16_t target_slot,
                     PacketGuard* packet);
    int ProcUpgrade_3rdChronicle(CUser* user,
                                 ENUM_EQUIPMENTTYPE equipment_type,
                                 uint8_t enchant_type,
                                 Inven_Item& target,
                                 const CItem* material_item);
    std::vector<EquipmentSkillDataUp>* find3rdChronicleEnchantData(
        STEnchantSystemMapKey key,
        int item_id);

    CMTRand* random_;
    uint32_t random_limit_;

    CItemUpgrade(const CItemUpgrade&);
    CItemUpgrade& operator=(const CItemUpgrade&);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(IItemUpgrade) == 0x04u,
              "IItemUpgrade IDA ABI size must stay 0x04");
static_assert(sizeof(CItemUpgrade) == 0x528u,
              "typed-table CItemUpgrade size must stay 0x528");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_DATA_PVF_EQUIPMENT_ITEM_UPGRADE_H_
