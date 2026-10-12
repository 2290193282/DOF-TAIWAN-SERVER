#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/etc/delete_invalid_item_data.h"
#include "old_game/economy/inventory/item_timeout_runtime.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/title_book_runtime.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>


namespace {

bool IsInvalidItemForDeletion(int item_id) {
    return G_CDataManager()->get_delete_invalid_item_script()
               ->IsInvalidItem2Delete(item_id) != 0;
}

int AmountToDelete(const Inven_Item& item, int matched_amount) {
    return matched_amount ? item.get_add_info() - matched_amount
                          : item.get_add_info();
}

Inven_Item& RedeemItem(
    Redeem_Item::Inven_Item_Expand_RedeemInfo& redeem_info) {
    return *reinterpret_cast<Inven_Item*>(redeem_info.item_storage);
}

}  // namespace

// IDA_EA: 0x867CF0C
// IDA_NAME: _ZN5CUser14_isMatchedItemEiRKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::_isMatchedItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
int CUser::_isMatchedItem(
    int item_id,
    const std::vector<std::pair<int, int> >& matched_items) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             matched_items.begin();
         it != matched_items.end();
         ++it) {
        if (it->first == item_id) {
            return it->second;
        }
    }
    return -1;
}

// IDA_EA: 0x867D0BE
// IDA_NAME: _ZN5CUser24_deleteSpecificItemInvenE10INVEN_TYPERKSt6vectorISt4pairIiiESaIS3_EERS5_ii
// DEMANGLED: CUser::_deleteSpecificItemInven(INVEN_TYPE,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&,int,int)
void CUser::_deleteSpecificItemInven(
    INVEN_TYPE inventory_type,
    const std::vector<std::pair<int, int> >& matched_items,
    std::vector<std::pair<int, int> >& changed_slots,
    int begin_slot,
    int end_slot) {
    CInventory* const inventory = getCurCharacInvenR();
    std::vector<std::pair<int, int> > invalid_items;

    for (int slot = begin_slot; slot <= end_slot; ++slot) {
        const Inven_Item item = inventory->GetInvenSlot(
            static_cast<int>(inventory_type), slot);
        if (item.isEmpty()) {
            continue;
        }

        bool remove_item = false;
        int matched_amount = -1;
        if (_checkTimeoutItem(item)) {
            remove_item = true;
            matched_amount = 0;
        } else {
            matched_amount = _isMatchedItem(item.getKey(),
                                            matched_items);
            if (matched_amount == -1) {
                if (IsInvalidItemForDeletion(item.getKey())) {
                    invalid_items.push_back(
                        std::make_pair(item.getKey(),
                                       item.get_add_info()));
                    remove_item = true;
                }
            } else if (item.get_add_info() > matched_amount) {
                remove_item = true;
            }
        }

        if (!remove_item) {
            continue;
        }

        enableSaveInven();
        inventory->delete_item(
            inventory_type,
            slot,
            AmountToDelete(item, matched_amount),
            static_cast<eItemDelReason>(19),
            true);
        changed_slots.push_back(std::make_pair(
            static_cast<ENUM_ITEMSPACE>(inventory_type ==
                                                static_cast<INVEN_TYPE>(0)
                                            ? 3
                                            : 0),
            slot));
    }

    RewardItem2DeleteInvalidItem("Inven", invalid_items);
}

// IDA_EA: 0x867D3EE
// IDA_NAME: _ZN5CUser24_deleteSpecificItemCargoERKSt6vectorISt4pairIiiESaIS2_EERS4_
// DEMANGLED: CUser::_deleteSpecificItemCargo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
void CUser::_deleteSpecificItemCargo(
    const std::vector<std::pair<int, int> >& matched_items,
    std::vector<std::pair<int, int> >& changed_slots) {
    CCargo* const cargo = getCurCharacCargoW();
    std::vector<std::pair<int, int> > invalid_items;

    for (int slot = 0; slot <= 151; ++slot) {
        const Inven_Item item = cargo->get_cargo_slot(slot);
        if (item.isEmpty()) {
            continue;
        }

        bool remove_item = false;
        int matched_amount = -1;
        if (_checkTimeoutItem(item)) {
            remove_item = true;
            matched_amount = 0;
        } else {
            matched_amount = _isMatchedItem(item.getKey(),
                                            matched_items);
            if (matched_amount == -1) {
                if (IsInvalidItemForDeletion(item.getKey())) {
                    invalid_items.push_back(
                        std::make_pair(item.getKey(),
                                       item.get_add_info()));
                    remove_item = true;
                }
            } else if (item.get_add_info() > matched_amount) {
                remove_item = true;
            }
        }

        if (!remove_item) {
            continue;
        }

        cargo->delete_item(slot,
                           AmountToDelete(item, matched_amount),
                           static_cast<eItemDelReason>(19));
        changed_slots.push_back(
            std::make_pair(static_cast<ENUM_ITEMSPACE>(2), slot));
    }

    RewardItem2DeleteInvalidItem("Cargo", invalid_items);
}

// IDA_EA: 0x867D69C
// IDA_NAME: _ZN5CUser25_deleteSpecificRedeemItemERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::_deleteSpecificRedeemItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
int CUser::_deleteSpecificRedeemItem(
    const std::vector<std::pair<int, int> >& matched_items) {
    charac_expand::CData* const data = GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(6));
    if (!data) {
        return 0;
    }

    Redeem_Item::CRedeemItem* const redeem_items =
        static_cast<Redeem_Item::CRedeemItem*>(data);
    Redeem_Item::CRedeemItem::RedeemInfoList::iterator it =
        redeem_items->redeem_items_.begin();
    unsigned int index = 0;
    while (it != redeem_items->redeem_items_.end() && index <= 20u) {
        Inven_Item& item = RedeemItem(*it);
        if (!item.isEmpty() &&
            (_checkTimeoutItem(item) ||
             _isMatchedItem(item.getKey(), matched_items) != -1)) {
            it = redeem_items->redeem_items_.erase(it);
            redeem_items->alter();
        } else {
            ++it;
        }
        ++index;
    }
    // The old body preserves the initial GetCharacExpandData result in EAX
    // through the iterator loop; deleteSpecificItem returns its low byte.
    return static_cast<int>(reinterpret_cast<uintptr_t>(data));
}

// IDA_EA: 0x867D8A4
// IDA_NAME: _ZN5CUser31_deleteSpecificItemAccountCargoERKSt6vectorISt4pairIiiESaIS2_EERS4_
// DEMANGLED: CUser::_deleteSpecificItemAccountCargo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
void CUser::_deleteSpecificItemAccountCargo(
    const std::vector<std::pair<int, int> >& matched_items,
    std::vector<std::pair<int, int> >& changed_slots) {
    if (!IsExistAccountCargo()) {
        return;
    }

    CAccountCargo* const account_cargo = GetAccountCargo();
    std::vector<std::pair<int, int> > invalid_items;
    for (int slot = 0; slot < account_cargo->GetCapacity(); ++slot) {
        const Inven_Item item = account_cargo->GetSlot(slot);
        if (item.isEmpty()) {
            continue;
        }

        bool remove_item = false;
        int matched_amount = -1;
        if (_checkTimeoutItem(item)) {
            remove_item = true;
            matched_amount = 0;
        } else {
            matched_amount = _isMatchedItem(item.getKey(),
                                            matched_items);
            if (matched_amount == -1) {
                if (IsInvalidItemForDeletion(item.getKey())) {
                    invalid_items.push_back(
                        std::make_pair(item.getKey(),
                                       item.get_add_info()));
                    remove_item = true;
                }
            } else if (item.get_add_info() > matched_amount) {
                remove_item = true;
            }
        }

        if (remove_item) {
            account_cargo->DeleteItem(slot, AmountToDelete(item, matched_amount));
            changed_slots.push_back(
                std::make_pair(static_cast<ENUM_ITEMSPACE>(12), slot));
        }
    }

    RewardItem2DeleteInvalidItem("AccountCargo", invalid_items);
}

// IDA_EA: 0x867DB66
// IDA_NAME: _ZN5CUser28_deleteSpecificItemTitieBookERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::_deleteSpecificItemTitieBook(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
void CUser::_deleteSpecificItemTitieBook(
    const std::vector<std::pair<int, int> >& matched_items) {
    charac_expand::CData* const data = GetCharacExpandData(
        static_cast<ENUM_CHARAC_EXPAND_TYPE>(14));
    if (data) {
        static_cast<CTitleBook*>(data)->deleteTimeoutItem(matched_items);
    }
}

// IDA_EA: 0x867DBA0
// IDA_NAME: _ZN5CUser18deleteSpecificItemERKSt6vectorISt4pairIiiESaIS2_EERS4_
// DEMANGLED: CUser::deleteSpecificItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>>&)
char CUser::deleteSpecificItem(
    const std::vector<std::pair<int, int> >& matched_items,
    std::vector<std::pair<int, int> >& changed_slots) {
    if (!getCurCharacR()) {
        return 1;
    }

    private_store::CPrivateStore* const store =
        private_store::GetInstancePrivateStoreMgr()->FindStoreSeller(this);
    if (store) {
        store->RemovePrivateStore(false);
    }

    _deleteSpecificItemInven(
        static_cast<INVEN_TYPE>(0),
        matched_items,
        changed_slots,
        oldsrv::config::kOrdinaryEquipmentTypeFirst,
        oldsrv::config::kOrdinaryEquipmentTypeLast);
    _deleteSpecificItemInven(
        static_cast<INVEN_TYPE>(1),
        matched_items,
        changed_slots,
        oldsrv::config::kMainInventoryQuickSlotFirst,
        oldsrv::config::kMainInventoryLastSlot);
    _deleteSpecificItemCargo(matched_items, changed_slots);
    _deleteSpecificItemAccountCargo(matched_items, changed_slots);
    _deleteSpecificItemTitieBook(matched_items);

    WongWork::CMailBox* const mailbox = GetMailBox();
    if (mailbox) {
        std::vector<std::pair<int, int> > invalid_items;
        uint32_t mail_ids[21] = {};
        mailbox->setMailIterator();
        for (WongWork::CMailBox::CMail* mail = mailbox->getNextMail(); mail;
             mail = mailbox->getNextMail()) {
            const Inven_Item item = mail->GetPackageItem();
            bool remove_mail = _checkTimeoutItem(item) != 0;
            if (!remove_mail) {
                if (_isMatchedItem(item.getKey(), matched_items) !=
                    -1) {
                    remove_mail = true;
                } else if (IsInvalidItemForDeletion(item.getKey())) {
                    invalid_items.push_back(std::make_pair(
                        item.getKey(), item.get_add_info()));
                    remove_mail = true;
                }
            }

            if (remove_mail) {
                mail_ids[++mail_ids[0]] =
                    static_cast<uint32_t>(mail->getMailIdx());
                mailbox->RemoveMail(mail->getMailIdx());
            }
        }

        if (mail_ids[0]) {
            WongWork::CMailBoxHelper::ReqDBRemoveMail(this, mail_ids, false);
            PacketGuard packet;
            packet.put_header(0, 98);
            packet.put_int(mail_ids[0]);
            for (uint32_t index = 1; index <= mail_ids[0]; ++index) {
                packet.put_int(mail_ids[index]);
            }
            packet.finalize(true);
            Send(packet);
        }
        RewardItem2DeleteInvalidItem("Mail", invalid_items);
    }

    return static_cast<char>(_deleteSpecificRedeemItem(matched_items));
}
