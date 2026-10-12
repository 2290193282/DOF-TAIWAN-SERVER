#include "old_game/economy/inventory/inventory.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/Login/account_cargo/account_cargo_script.h"
#include "old_data/db/inventory/db_create_account_cargo.h"
#include "old_data/db/inventory/db_upgrade_account_cargo.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x828AD72
// IDA_NAME: _ZN13CAccountCargo18CreateAccountCargoEP5CUser
// DEMANGLED: CAccountCargo::CreateAccountCargo(CUser *)
int CAccountCargo::CreateAccountCargo(CUser* user) {
    if (user->IsExistAccountCargo()) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        20);
    }

    AccountCargoScript* const script = AccountCargoAt(G_CDataManager());
    if (script->required_level > user->get_charac_level()) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        14);
    }

    UpgradeInfo* const info = script->GetCreateInfo();
    if (!info) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        19);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 24)) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        209);
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(4));
    if (protection_error) {
        return user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
            static_cast<unsigned char>(protection_error));
    }

    Inven_Item required_item;
    const int item_slot = user->getCurCharacInvenR()->GetInvenData(
        script->required_item, required_item);
    if (info->required_item_count &&
        (item_slot == -1 ||
         info->required_item_count > required_item.get_add_info())) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_gold > 0 &&
        info->required_gold > user->getCurCharacMoney()) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_item_count &&
        user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), item_slot, info->required_item_count,
            static_cast<eItemDelReason>(36), true) != 1) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_gold > 0 &&
        user->getCurCharacInvenW()->use_money(
            info->required_gold, static_cast<eMoneySubReason>(41), true) !=
            1) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        22);
    }

    SendNotifyRecipe(user, item_slot, info->required_item_count != 0);
    DB_CreateAccountCargo::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(info->capacity));
    return 0;
}

// IDA_EA: 0x828B0B2
// IDA_NAME: _ZN13CAccountCargo19UpgradeAccountCargoEP5CUser
// DEMANGLED: CAccountCargo::UpgradeAccountCargo(CUser *)
int CAccountCargo::UpgradeAccountCargo(CUser* user) {
    if (!user->IsExistAccountCargo()) {
        // The original CMD307 path deliberately reports the older CMD306 id.
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO,
                                        21);
    }

    CAccountCargo* const account_cargo =
        static_cast<CAccountCargo*>(user->GetAccountCargo());
    AccountCargoScript* const script = AccountCargoAt(G_CDataManager());
    UpgradeInfo* const info =
        script->GetNextUpgradeInfo(account_cargo->GetCapacity());
    if (!info) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        19);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 25)) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        209);
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(5));
    if (protection_error) {
        return user->SendCmdErrorPacket(
            ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
            static_cast<unsigned char>(protection_error));
    }

    Inven_Item required_item;
    const int item_slot = user->getCurCharacInvenR()->GetInvenData(
        script->required_item, required_item);
    if (info->required_item_count &&
        (item_slot == -1 ||
         info->required_item_count > required_item.get_add_info())) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_gold > 0 &&
        info->required_gold > user->getCurCharacMoney()) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_item_count &&
        user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), item_slot, info->required_item_count,
            static_cast<eItemDelReason>(36), true) != 1) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        22);
    }

    if (info->required_gold > 0 &&
        user->getCurCharacInvenW()->use_money(
            info->required_gold, static_cast<eMoneySubReason>(41), true) !=
            1) {
        return user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO,
                                        22);
    }

    SendNotifyRecipe(user, item_slot, info->required_item_count != 0);
    DB_UpgradeAccountCargo::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(info->capacity));
    return 0;
}

// IDA_EA: 0x828B3D0
// IDA_NAME: _ZN13CAccountCargo16SendNotifyRecipeEP5CUserib
// DEMANGLED: CAccountCargo::SendNotifyRecipe(CUser *,int,bool)
void CAccountCargo::SendNotifyRecipe(CUser* user,
                                     int item_slot,
                                     bool deleted_required_item) {
    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    if (deleted_required_item) {
        packet.put_short(2);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), item_slot, packet);
    } else {
        packet.put_short(1);
    }
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), 0, packet);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x828A12A
// IDA_NAME: _ZN13CAccountCargo12DepositMoneyEj
// DEMANGLED: CAccountCargo::DepositMoney(unsigned int)
int CAccountCargo::DepositMoney(unsigned int money) {
    CUser* const user = user_;
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 26)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DEPOSIT_MONEY, 209);
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(2));
    if (protection_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_DEPOSIT_MONEY,
            static_cast<unsigned char>(protection_error));
        return 0;
    }

    if (static_cast<int>(money) > user->getCurCharacMoney() ||
        static_cast<int>(money) < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DEPOSIT_MONEY, 10);
        return 0;
    }

    if (!CheckMoneyLimit(money)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DEPOSIT_MONEY, 95);
        return 0;
    }

    if (user->getCurCharacInvenW()->use_money(
            static_cast<int>(money), static_cast<eMoneySubReason>(40), true) !=
        1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DEPOSIT_MONEY, 10);
        return 0;
    }

    AddMoney(money);
    SendNotifyMoney(ENUM_CMDPACKET_DEPOSIT_MONEY);
    altered_ = 1u;
    return 1;
}

// IDA_EA: 0x828A2F6
// IDA_NAME: _ZN13CAccountCargo13WithdrawMoneyEj
// DEMANGLED: CAccountCargo::WithdrawMoney(unsigned int)
int CAccountCargo::WithdrawMoney(unsigned int money) {
    CUser* const user = user_;
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 26)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WITHDRAW_MONEY, 209);
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(3));
    if (protection_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_WITHDRAW_MONEY,
            static_cast<unsigned char>(protection_error));
        return 0;
    }

    if (money_ < money || static_cast<int>(money) < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WITHDRAW_MONEY, 10);
        return 0;
    }

    if (!user->CheckMoney(static_cast<int>(money))) {
        // The old CMD309 branch reports its capacity failure against CMD308.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DEPOSIT_MONEY, 94);
        return 0;
    }

    SubMoney(money);
    if (!user->getCurCharacInvenW()->gain_money(
            static_cast<int>(money), static_cast<eMoneyAddReason>(27), true,
            0)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WITHDRAW_MONEY, 10);
        return 0;
    }

    SendNotifyMoney(ENUM_CMDPACKET_WITHDRAW_MONEY);
    altered_ = 1u;
    return 1;
}

// IDA_EA: 0x828A4CA
// IDA_NAME: _ZN13CAccountCargo15CheckMoneyLimitEj
// DEMANGLED: CAccountCargo::CheckMoneyLimit(unsigned int)
bool CAccountCargo::CheckMoneyLimit(unsigned int money) {
    UpgradeInfo* const info = AccountCargoAt(G_CDataManager())
                                  ->GetCurrUpgradeInfo(
                                      static_cast<int>(capacity_));
    return info && static_cast<uint32_t>(info->money_limit) >= money + money_;
}

// IDA_EA: 0x828A7DC
// IDA_NAME: _ZN13CAccountCargo15SendNotifyMoneyE14ENUM_CMDPACKET
// DEMANGLED: CAccountCargo::SendNotifyMoney(ENUM_CMDPACKET)
void CAccountCargo::SendNotifyMoney(ENUM_CMDPACKET command) {
    PacketGuard packet;
    packet.put_header(1, command);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(money_));
    packet.finalize(true);
    user_->Send(packet);
}
