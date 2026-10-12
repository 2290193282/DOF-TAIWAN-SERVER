#include "infra/security/policy/secu_protection_field_abi.h"

#include <stdint.h>
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: referenced by CSecu_ProtectionField::GetItemPtr @0x8288573.
// IDA_NAME: _Z25GetInvenTypeFromItemSpace14ENUM_ITEMSPACE
// DEMANGLED: GetInvenTypeFromItemSpace(ENUM_ITEMSPACE)
extern int GetInvenTypeFromItemSpace(ENUM_ITEMSPACE item_space)
    __asm__("_Z25GetInvenTypeFromItemSpace14ENUM_ITEMSPACE");

namespace {

const unsigned int kSecurityProtectionMax = 70u;

const int kRestoreExceptionProtections[37] = {
    1,  2,  3,  4,  5,  12, 13, 14, 17, 18,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    34, 36, 31, 39, 37, 38, 44, 45, 47, 55,
    65, 66, 67, 58, 59, 32, 11,
};

const int kTradingExceptionProtections[29] = {
    1,  2,  3,  4,  5,  12, 13, 14, 17, 20,
    22, 23, 24, 29, 34, 36, 44, 45, 31, 32,
    47, 65, 66, 67, 56, 57, 58, 59, 11,
};

bool is_trade_punish_type(CUser* user, uint32_t mask) {
    return user->isTradePunishType(static_cast<int>(mask)) != 0;
}

bool is_security_service_used(CUser* user) {
    return user->isSecurityServiceUsed() != 0;
}

}  // namespace

// IDA_EA: 0x8288994
// IDA_NAME: _ZN21CSecu_ProtectionField13CheckOppositeEP5CUser18SECURITY_PROTCTIONS1_
// DEMANGLED: CSecu_ProtectionField::CheckOpposite(CUser *,SECURITY_PROTCTION,CUser *)
int CSecu_ProtectionField::CheckOpposite(CUser* user,
                                         SECURITY_PROTCTION protection,
                                         CUser* opposite_user) {
    if (!user || !opposite_user) return 1;

    const int error = Check(user, protection);
    if (!error && opposite_user->isTradePunishType(0)) return 115;
    return error;
}

// IDA_EA: 0x8288074
// IDA_NAME: _Z23ExceptionProtectionTypei23SECURITY_PROTCTION_TYPES_S_S_S_S_S_S_S_
// DEMANGLED: ExceptionProtectionType(int,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE,SECURITY_PROTCTION_TYPE)
unsigned int ExceptionProtectionType(int value,
                                     int type_1,
                                     int type_2,
                                     int type_4,
                                     int type_8,
                                     int type_16,
                                     int type_32,
                                     int type_64,
                                     int type_128,
                                     int type_256)
    __asm__("_Z23ExceptionProtectionTypei23SECURITY_PROTCTION_TYPES_S_S_S_S_S_S_S_");
unsigned int ExceptionProtectionType(int value,
                                     int type_1,
                                     int type_2,
                                     int type_4,
                                     int type_8,
                                     int type_16,
                                     int type_32,
                                     int type_64,
                                     int type_128,
                                     int type_256) {
    if (!type_1) {
        value &= ~1u;
    }
    if (!type_2) {
        value &= ~2u;
    }
    if (!type_4) {
        value &= ~4u;
    }
    if (!type_8) {
        value &= ~8u;
    }
    if (!type_16) {
        value &= ~0x10u;
    }
    if (!type_32) {
        value &= ~0x20u;
    }
    if (!type_64) {
        value &= ~0x40u;
    }
    if (!type_128) {
        value &= ~0x80u;
    }
    if (!type_256) {
        return static_cast<unsigned int>(value) & 0xFFFFFEFFu;
    }
    return static_cast<unsigned int>(value);
}

// IDA_EA: 0x82AA6D4
// IDA_NAME: _ZN21CSecu_ProtectionFieldC2Ev
// DEMANGLED: CSecu_ProtectionField::CSecu_ProtectionField(void)
CSecu_ProtectionField::CSecu_ProtectionField() : protection_types() {
    init();
}

CSecu_ProtectionField::~CSecu_ProtectionField() {}

// IDA_EA: 0x828824E
// IDA_NAME: _ZN21CSecu_ProtectionField4initEv
// DEMANGLED: CSecu_ProtectionField::init(void)
bool CSecu_ProtectionField::init() {
    uint32_t values[kSecurityProtectionMax + 1];
    for (unsigned int index = 0; index <= kSecurityProtectionMax; ++index) {
        values[index] = 511u;
    }
    for (unsigned int index = 0;
         index < sizeof(kRestoreExceptionProtections) /
                     sizeof(kRestoreExceptionProtections[0]);
         ++index) {
        const int protection = kRestoreExceptionProtections[index];
        values[protection] = ExceptionProtectionType(values[protection],
                                                     0,
                                                     0,
                                                     0,
                                                     0,
                                                     16,
                                                     0,
                                                     0,
                                                     128,
                                                     0);
    }
    for (unsigned int index = 0; index <= kSecurityProtectionMax; ++index) {
        values[index] = 511u;
    }
    for (unsigned int index = 0;
         index < sizeof(kTradingExceptionProtections) /
                     sizeof(kTradingExceptionProtections[0]);
         ++index) {
        const int protection = kTradingExceptionProtections[index];
        values[protection] = ExceptionProtectionType(values[protection],
                                                     1,
                                                     2,
                                                     4,
                                                     8,
                                                     16,
                                                     32,
                                                     64,
                                                     0,
                                                     256);
    }

    protection_types.clear();
    for (unsigned int index = 0; index <= kSecurityProtectionMax; ++index) {
        protection_types.insert(
            std::make_pair(static_cast<int>(index), values[index]));
    }
    return false;
}

// IDA_EA: 0x82880DC
// IDA_NAME: _ZN21CSecu_ProtectionField17GetProtectionTypeE18SECURITY_PROTCTION
// DEMANGLED: CSecu_ProtectionField::GetProtectionType(SECURITY_PROTCTION)
int CSecu_ProtectionField::GetProtectionType(SECURITY_PROTCTION protection) {
    CSecu_ProtectionField* const self = this;
    std::map<int, uint32_t>::iterator it =
        self->protection_types.find(protection);
    if (it != self->protection_types.end()) {
        return static_cast<int>(it->second);
    }
    return 0;
}

// IDA_EA: 0x82884D6
// IDA_NAME: _ZN21CSecu_ProtectionField10GetItemPtrEP5CUsercs
// DEMANGLED: CSecu_ProtectionField::GetItemPtr(CUser *,char,short)
CItem* CSecu_ProtectionField::GetItemPtr(CUser* user,
                                         char inventory_kind,
                                         short slot) {
    Inven_Item item;
    if (inventory_kind == 2) {
        item = user->getCurCharacCargoR()->get_cargo_slot(slot);
    } else if (inventory_kind == 0 || inventory_kind == 3 ||
               inventory_kind == 7) {
        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(inventory_kind));
        item = user->getCurCharacInvenR()->GetInvenSlot(inventory_type, slot);
    }

    return G_CDataManager()->find_item(item.item_id_);
}

// IDA_EA: 0x8288786
// IDA_NAME: _ZN21CSecu_ProtectionField9CheckItemEP5CUser18SECURITY_PROTCTIONPK5CItem
// DEMANGLED: CSecu_ProtectionField::CheckItem(CUser *,SECURITY_PROTCTION,CItem const *)
int CSecu_ProtectionField::CheckItem(CUser* user,
                                      SECURITY_PROTCTION protection,
                                      const CItem* item) {
    if (!item || !user) {
        return 1;
    }

    const int item_index = item->get_index();
    if (item_index == 8048 || item_index == 8049 ||
        item_index == 10000915 || item_index == 690000097) {
        return Check(user, protection);
    }
    return 0;
}

// IDA_EA: 0x82886AE
// IDA_NAME: _ZN21CSecu_ProtectionField15GetInvenItemPtrEP5CUsercs
// DEMANGLED: CSecu_ProtectionField::GetInvenItemPtr(CUser *,char,short)
Inven_Item* CSecu_ProtectionField::GetInvenItemPtr(CUser* user,
                                                    char inventory_kind,
                                                    short slot) {
    if (!user) {
        return 0;
    }
    if (inventory_kind == 2) {
        return user->getCurCharacCargoR()->GetCargoRef(slot);
    }
    if (inventory_kind != 0 && inventory_kind != 3 && inventory_kind != 7) {
        return 0;
    }
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(inventory_kind));
    return user->getCurCharacInvenR()->GetInvenRef(inventory_type, slot);
}

// IDA_EA: 0x82887D0
// IDA_NAME: _ZN21CSecu_ProtectionField15CheckItemRarityEP5CUser18SECURITY_PROTCTIONPK5CItemPK10Inven_Item
// DEMANGLED: CSecu_ProtectionField::CheckItemRarity(CUser *,SECURITY_PROTCTION,CItem const *,Inven_Item const *)
int CSecu_ProtectionField::CheckItemRarity(
    CUser* user,
    SECURITY_PROTCTION protection,
    const CItem* item,
    const Inven_Item* inventory_item) {
    if (!item || !user) {
        return 1;
    }

    int rarity = 0;
    if (item->get_rarity() > 1) {
        rarity = item->get_rarity();
    }
    const int protection_type = this->GetProtectionType(protection);
    if (!protection_type) {
        return 0;
    }
    if (!inventory_item) {
        return 1;
    }

    // The call at vtable +0x4c is the original protection-specific rarity
    // predicate.  It shares the physical slot currently named
    // is_random_option by a different call site, so retain the exact ABI
    // through a local typed view rather than widening CItemVTable globally.
    typedef int (*ProtectionRarityFn)(const CItem*, int);
    const ProtectionRarityFn protection_rarity =
        reinterpret_cast<ProtectionRarityFn>(item->vtable_->is_random_option);
    const bool requires_full_check =
        rarity > 2 ||
        (item->get_rarity() == 2 &&
         protection_rarity(item, static_cast<int>(protection)) != 1) ||
        inventory_item->GetUpgrade() > 10;

    const int account_error = CheckAccountType(
        user, static_cast<unsigned long>(protection_type));
    if (account_error) {
        return account_error;
    }
    if (!requires_full_check) {
        return 0;
    }

    if (static_cast<int>(protection) == 35 ||
        static_cast<int>(protection) == 41) {
        if (static_cast<signed char>(protection_type) < 0 &&
            is_trade_punish_type(user, 128u)) {
            return 229;
        }
    }
    if (static_cast<int>(protection) == 35) {
        const unsigned int flags = static_cast<unsigned char>(protection_type);
        if (((flags & 1u) && is_trade_punish_type(user, 1u)) ||
            ((flags & 2u) && is_trade_punish_type(user, 2u)) ||
            ((flags & 4u) && is_trade_punish_type(user, 4u)) ||
            ((flags & 8u) && is_trade_punish_type(user, 8u)) ||
            ((flags & 0x40u) && is_trade_punish_type(user, 64u))) {
            return 114;
        }
        if ((flags & 0x20u) && is_trade_punish_type(user, 32u)) {
            return 228;
        }
    }
    return 0;
}

// IDA_EA: 0x8288AC8
// IDA_NAME: _ZN21CSecu_ProtectionField16CheckAccountTypeEP5CUserm
// DEMANGLED: CSecu_ProtectionField::CheckAccountType(CUser *,unsigned long)
int CSecu_ProtectionField::CheckAccountType(CUser* user,
                                            unsigned long protection_type) {
    if (!user) {
        return 1;
    }
    WongWork::CSecurityCard* const security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        if (static_cast<unsigned int>(security_card->getFailCnt()) > 2u) {
            return 122;
        }
        if (!security_card->isCertified()) {
            user->sendBackSecurityAuthRequest();
            return 123;
        }
    } else {
        Sanicova::CPad* const pad = user->getPad();
        if (pad->isActivate()) {
            if (pad->getFailCnt() > 2) {
                return 136;
            }
            if (!pad->isCertified()) {
                user->sendBackSecurityAuthRequest();
                return 137;
            }
        }
    }
    return (protection_type & 0x10u) != 0u &&
                   user->isTradePunishType(16u)
               ? 114
               : 0;
}

// IDA_EA: 0x82881A2
// IDA_NAME: _ZN21CSecu_ProtectionField14GetCashShopErrEi
// DEMANGLED: CSecu_ProtectionField::GetCashShopErr(int)
int CSecu_ProtectionField::GetCashShopErr(int error) {
    if (error == 123) {
        return 132;
    }
    if (error > 123) {
        if (error == 207) {
            return 138;
        }
        if (error > 207) {
            if (error == 228) {
                return 137;
            }
            if (error == 229) {
                return 136;
            }
        } else {
            if (error == 136) {
                return 133;
            }
            if (error == 137) {
                return 134;
            }
        }
    } else {
        if (error == 115) {
            return 130;
        }
        if (error > 115) {
            if (error == 120) {
                return 139;
            }
            if (error == 122) {
                return 131;
            }
        } else if (error == 114) {
            return 135;
        }
    }
    return 1;
}

// IDA_EA: 0x8288140
// IDA_NAME: _ZN21CSecu_ProtectionField14GetOppositeErrEi
// DEMANGLED: CSecu_ProtectionField::GetOppositeErr(int)
int CSecu_ProtectionField::GetOppositeErr(int error) {
    if (error == 136) {
        return 236;
    }
    if (error > 136) {
        if (error == 137) {
            return 138;
        }
        if (static_cast<unsigned int>(error - 228) <= 1u) {
            return 115;
        }
        return 1;
    }
    switch (error) {
    case 122:
        return 238;
    case 123:
        return 124;
    case 114:
        return 115;
    default:
        return 1;
    }
}

// IDA_EA: 0x8288BF2
// IDA_NAME: _ZN21CSecu_ProtectionField16CheckRestoreTypeEP5CUserm
// DEMANGLED: CSecu_ProtectionField::CheckRestoreType(CUser *,unsigned long)
int CSecu_ProtectionField::CheckRestoreType(
    CUser* user, unsigned long protection_type) {
    if (!user) {
        return 1;
    }
    if (static_cast<signed char>(protection_type) < 0 &&
        is_trade_punish_type(user, 128u)) {
        return 229;
    }
    return 0;
}

// IDA_EA: 0x8288C46
// IDA_NAME: _ZN21CSecu_ProtectionField16CheckTradingTypeEP5CUserm18SECURITY_PROTCTION
// DEMANGLED: CSecu_ProtectionField::CheckTradingType(CUser *,unsigned long,SECURITY_PROTCTION)
int CSecu_ProtectionField::CheckTradingType(
    CUser* user,
    unsigned long protection_type,
    SECURITY_PROTCTION protection) {
    (void)protection;
    if (!user) {
        return 1;
    }
    const unsigned int flags =
        static_cast<unsigned int>(static_cast<unsigned char>(protection_type));
    if ((flags & 1u) != 0 && is_trade_punish_type(user, 1u)) {
        return 114;
    }
    if ((flags & 2u) != 0 && is_trade_punish_type(user, 2u)) {
        return 114;
    }
    if ((flags & 4u) != 0 && is_trade_punish_type(user, 4u)) {
        return 114;
    }
    if ((flags & 8u) != 0 && is_trade_punish_type(user, 8u)) {
        return 114;
    }
    if ((flags & 0x20u) != 0 && is_trade_punish_type(user, 32u)) {
        return 228;
    }
    if ((flags & 0x40u) != 0 && is_trade_punish_type(user, 64u)) {
        return 114;
    }
    return 0;
}

// IDA_EA: 0x8288DBC
// IDA_NAME: _ZN21CSecu_ProtectionField12CheckEtcTypeEP5CUser18SECURITY_PROTCTION
// DEMANGLED: CSecu_ProtectionField::CheckEtcType(CUser *,SECURITY_PROTCTION)
int CSecu_ProtectionField::CheckEtcType(
    CUser* user, SECURITY_PROTCTION protection) {
    if (!user) {
        return 1;
    }
    if (user->isRestingUser()) {
        return 207;
    }
    if ((protection == 8 || protection == 9) &&
        is_security_service_used(user) != true) {
        return 233;
    }
    return 0;
}

// IDA_EA: 0x8288A02
// IDA_NAME: _ZN21CSecu_ProtectionField5CheckEP5CUser18SECURITY_PROTCTION
// DEMANGLED: CSecu_ProtectionField::Check(CUser *,SECURITY_PROTCTION)
int CSecu_ProtectionField::Check(CUser* user,
                                 SECURITY_PROTCTION protection) {
    if (!user) {
        return 1;
    }
    const int protection_value = static_cast<int>(protection);
    const int protection_type =
        this->GetProtectionType(protection);
    if (!protection_type) {
        return 0;
    }
    const int restore_result =
        this->CheckRestoreType(user, static_cast<unsigned long>(protection_type));
    if (restore_result) {
        return restore_result;
    }
    const int trading_result =
        this->CheckTradingType(
            user,
            static_cast<unsigned long>(protection_type),
            protection);
    if (trading_result) {
        return trading_result;
    }
    const int etc_result =
        this->CheckEtcType(user, protection);
    if (etc_result) {
        return etc_result;
    }
    return 0;
}
