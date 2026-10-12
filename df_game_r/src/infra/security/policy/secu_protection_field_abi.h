#ifndef OLDSRV_GLOBAL_SECU_PROTECTION_FIELD_ABI_H_
#define OLDSRV_GLOBAL_SECU_PROTECTION_FIELD_ABI_H_

#include <stdint.h>

#include <map>

class CUser;
class CItem;
struct Inven_Item;
enum SECURITY_PROTCTION : int;

// IDA unit: 99_misc_global/CSecu_ProtectionField.
// IDA_EA: ctor @0x82AA6D4, init @0x828824E.
class CSecu_ProtectionField {
public:
    CSecu_ProtectionField();
    ~CSecu_ProtectionField();

    bool init();

    // IDA_EA: 0x8288A02
    // IDA_NAME: _ZN21CSecu_ProtectionField5CheckEP5CUser18SECURITY_PROTCTION
    // DEMANGLED: CSecu_ProtectionField::Check(CUser *,SECURITY_PROTCTION)
    int Check(CUser* user, SECURITY_PROTCTION protection);
    int GetProtectionType(SECURITY_PROTCTION protection);
    int GetCashShopErr(int error);
    int CheckRestoreType(CUser* user, unsigned long protection_type);
    int CheckTradingType(CUser* user,
                         unsigned long protection_type,
                         SECURITY_PROTCTION protection);
    int CheckEtcType(CUser* user, SECURITY_PROTCTION protection);

    // IDA_EA: 0x8288AC8
    // IDA_NAME: _ZN21CSecu_ProtectionField16CheckAccountTypeEP5CUserm
    // DEMANGLED: CSecu_ProtectionField::CheckAccountType(CUser *,unsigned long)
    int CheckAccountType(CUser* user, unsigned long protection_type);

    // IDA_EA: 0x8288994
    // IDA_NAME: _ZN21CSecu_ProtectionField13CheckOppositeEP5CUser18SECURITY_PROTCTIONS1_
    // DEMANGLED: CSecu_ProtectionField::CheckOpposite(CUser *,SECURITY_PROTCTION,CUser *)
    int CheckOpposite(CUser* user,
                      SECURITY_PROTCTION protection,
                      CUser* opposite_user);

    // IDA_EA: 0x8288140
    // IDA_NAME: _ZN21CSecu_ProtectionField14GetOppositeErrEi
    // DEMANGLED: CSecu_ProtectionField::GetOppositeErr(int)
    int GetOppositeErr(int error);

    // IDA_EA: 0x82884D6
    // IDA_NAME: _ZN21CSecu_ProtectionField10GetItemPtrEP5CUsercs
    // DEMANGLED: CSecu_ProtectionField::GetItemPtr(CUser *,char,short)
    // The concrete typed inventory provider remains link debt.  This is the
    // one canonical member declaration used by CMD032; no raw layout or
    // fallback object is introduced here.
    CItem* GetItemPtr(CUser* user, char inventory_kind, short slot);

    // IDA_EA: 0x8288786
    // IDA_NAME: _ZN21CSecu_ProtectionField9CheckItemEP5CUser18SECURITY_PROTCTIONPK5CItem
    // DEMANGLED: CSecu_ProtectionField::CheckItem(CUser *,SECURITY_PROTCTION,CItem const *)
    // The original typed item validation remains a separate provider debt.
    int CheckItem(CUser* user,
                  SECURITY_PROTCTION protection,
                  const CItem* item);

    // IDA_EA: 0x82886AE
    // IDA_NAME: _ZN21CSecu_ProtectionField15GetInvenItemPtrEP5CUsercs
    // DEMANGLED: CSecu_ProtectionField::GetInvenItemPtr(CUser *,char,short)
    // This is deliberately distinct from GetItemPtr: CMD024 needs the live
    // per-slot record as well as its immutable CItem prototype for the
    // rarity/upgrade security decision.
    Inven_Item* GetInvenItemPtr(CUser* user,
                                char inventory_kind,
                                short slot);

    // IDA_EA: 0x82887D0
    // IDA_NAME: _ZN21CSecu_ProtectionField15CheckItemRarityEP5CUser18SECURITY_PROTCTIONPK5CItemPK10Inven_Item
    // DEMANGLED: CSecu_ProtectionField::CheckItemRarity(CUser *,SECURITY_PROTCTION,CItem const *,Inven_Item const *)
    int CheckItemRarity(CUser* user,
                        SECURITY_PROTCTION protection,
                        const CItem* item,
                        const Inven_Item* inventory_item);

    std::map<int, uint32_t> protection_types;
};

static_assert(sizeof(CSecu_ProtectionField) == 0x18u,
              "CSecu_ProtectionField IDA ABI size must stay 0x18");

#endif  // OLDSRV_GLOBAL_SECU_PROTECTION_FIELD_ABI_H_
