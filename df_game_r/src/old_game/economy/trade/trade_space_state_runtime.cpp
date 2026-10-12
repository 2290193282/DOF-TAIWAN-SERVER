#include "old_game/economy/mail/mail_trade_value_types.h"

#include "common/manager/game_manager.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

int SendChangeTradeState(PacketGuard& packet,
                         CUser* source,
                         CUser* target,
                         int state)
    __asm__("_Z20SendChangeTradeStateR11PacketGuardP5CUserS2_15ENUM_TRADESTATE");
// IDA_EA: 0x852F0A0
// IDA_NAME: _Z20SendChangeTradeStateR11PacketGuardP5CUserS2_15ENUM_TRADESTATE
// DEMANGLED: SendChangeTradeState(PacketGuard &,CUser *,CUser *,ENUM_TRADESTATE)
int SendChangeTradeState(PacketGuard& packet,
                         CUser* source,
                         CUser* target,
                         int state) {
    packet.clear();
    packet.put_header(0, 17);
    packet.put_short(source->get_unique_id());
    packet.put_byte(state);
    packet.finalize(true);
    source->Send(packet);
    return target->Send(packet);
}

// IDA_EA: 0x852F134
// IDA_NAME: _ZN11CTradeSpace18change_trade_stateEP5CUseri
// DEMANGLED: CTradeSpace::change_trade_state(CUser *,int)
int CTradeSpace::change_trade_state(CUser* owner, int state) {
    PacketGuard packet;
    CUser* opposite = 0;
    int opposite_index = 0;
    int owner_index = 0;
    if (traders_[0] == owner) {
        opposite = traders_[1];
        opposite_index = 1;
    } else {
        opposite = traders_[0];
        opposite_index = 0;
        owner_index = 1;
    }

    if (!opposite) {
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_SET_ITEMTRADE_STATE);
        packet.put_byte(0);
        packet.put_byte(1);
        packet.finalize(true);
        owner->Send(packet);
        return 1;
    }

    switch (state) {
    case 0:
        if (IsLocked()) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 7,
                                      packet);
            return 0;
        }
        trade_states_[owner_index] = 0;
        SendChangeTradeState(packet, owner, opposite, 0);
        return 0;

    case 1: {
        const int security_error =
            (GlobalData::s_pSecuProtectionField)
                ->Check(owner, static_cast<SECURITY_PROTCTION>(54));
        if (security_error) {
            owner->SendCmdErrorPacket(
                ENUM_CMDPACKET_SET_ITEMTRADE_STATE,
                static_cast<unsigned char>(security_error));
            opposite->SendCmdErrorPacket(
                ENUM_CMDPACKET_SET_ITEMTRADE_STATE,
                static_cast<unsigned char>((GlobalData::s_pSecuProtectionField)->GetOppositeErr(security_error)));
            if (security_error == 123 || security_error == 137) {
                packet.clear();
                packet.put_header(0, 176);
                packet.put_byte(1);
                packet.finalize(true);
                opposite->Send(packet);
            }
            return 0;
        }
        if (IsLocked()) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 7,
                                      packet);
            return 0;
        }

        int required_slots[11] = {};
        int empty_slots[11] = {};
        const CInventory* const recipient_inventory =
            static_cast<const CInventory*>(opposite->getCurCharacInvenR());
        int quick_empty_count = recipient_inventory->QuickEmptyCount();
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const TradeSlot& offered = trade_slots_[owner_index][slot_index];
            const int item_id = offered.item_.item_id_;
            if (!item_id) {
                continue;
            }
            const int item_type = offered.item_.item_category_;
            if (offered.item_.isEquipableItemType()) {
                ++required_slots[item_type];
            } else if (recipient_inventory->check_item_exist(item_id) < 0) {
                ++required_slots[item_type];
            }
        }
        for (int item_type = 1; item_type <= 10; ++item_type) {
            empty_slots[item_type] = recipient_inventory->InvenEmptycount(
                static_cast<Inven_Item::ITEM_TYPE>(item_type));
            if (required_slots[item_type] > empty_slots[item_type]) {
                quick_empty_count +=
                    empty_slots[item_type] - required_slots[item_type];
                if (quick_empty_count < 0) {
                    owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE,
                                              4, packet);
                    return 0;
                }
            }
        }
        trade_states_[owner_index] = 1;
        SendChangeTradeState(packet, owner, opposite, 1);
        if (trade_states_[opposite_index] == 1) {
            _SetLock(true);
        }
        return 0;
    }

    case 2: {
        if (cancel_trade()) {
            G_CGameManager()->PutTradeSpace(this);
        } else {
            cMyTrace trace("bool CTradeSpace::change_trade_state(CUser*, int)",
                           2523, 5);
            trace("change_trade_state, cancel_trade Failed 1!  %d %d",
                  opposite->getCurCharacNo(), owner->getCurCharacNo());
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 1,
                                      packet);
        }

        int remaining_count = 0;
        for (int trader_index = 0; trader_index <= 1; ++trader_index) {
            for (int slot_index = 3; slot_index <= 26; ++slot_index) {
                if (trade_slots_[trader_index][slot_index]
                        .item_.item_id_) {
                    ++remaining_count;
                }
            }
        }
        if (remaining_count && traders_[0] && traders_[1]) {
            cMyTrace trace("bool CTradeSpace::change_trade_state(CUser*, int)",
                           2539, 5);
            trace("TRADE CANCELED! BUT, ITEM REMAINED! %d Count , Ch1 %d Ch2 %d",
                  remaining_count, traders_[1]->get_charac_no(-1),
                  traders_[0]->get_charac_no(-1));
            for (int trader_index = 0; trader_index <= 1; ++trader_index) {
                CUser* const trader = traders_[trader_index];
                if (!trader) {
                    continue;
                }
                for (int slot_index = 3; slot_index <= 26; ++slot_index) {
                    const TradeSlot& offered =
                        trade_slots_[trader_index][slot_index];
                    if (!offered.item_.item_id_) {
                        continue;
                    }
                    cMyTrace item_trace(
                        "bool CTradeSpace::change_trade_state(CUser*, int)",
                        2552, 5);
                    item_trace("Remained Item %d : %d x %d",
                               trader->get_charac_no(-1),
                               offered.item_.item_id_,
                               offered.item_.get_add_info());
                }
            }
        }
        return 0;
    }

    case 3: {
        const unsigned int own_gold =
            static_cast<unsigned int>(trade_gold_[owner_index]);
        const unsigned int opposite_gold =
            static_cast<unsigned int>(trade_gold_[opposite_index]);
        if (owner && !owner->GetReliablePerson() &&
            !owner->IsAvailableCurCharacTradeGoldDaily(own_gold)) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 70,
                                      packet);
            return 0;
        }
        if (opposite && !opposite->GetReliablePerson() &&
            !opposite->IsAvailableCurCharacTradeGoldDaily(opposite_gold)) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 71,
                                      packet);
            return 0;
        }
        if (trade_states_[owner_index] != 1) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 7,
                                      packet);
            return 0;
        }

        trade_states_[owner_index] = 3;
        if (trade_states_[opposite_index] != 3) {
            return 0;
        }

        bool own_low_value_offer = false;
        bool opposite_low_value_offer = false;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const TradeSlot& offered = trade_slots_[opposite_index][slot_index];
            const int item_id = offered.item_.item_id_;
            if (!item_id) {
                continue;
            }
            CItem* const item = G_CDataManager()->find_item(item_id);
            if (!item) {
                continue;
            }
            typedef unsigned char (*IsAvatarItemFn)(CItem*);
            IsAvatarItemFn const is_avatar_item =
                reinterpret_cast<IsAvatarItemFn>(
                    (*reinterpret_cast<void***>(item))[4]);
            if (is_avatar_item(item) != 1 && item->get_rarity() <= 1) {
                own_low_value_offer = true;
                break;
            }
        }
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const TradeSlot& offered = trade_slots_[owner_index][slot_index];
            const int item_id = offered.item_.item_id_;
            if (!item_id) {
                continue;
            }
            CItem* const item = G_CDataManager()->find_item(item_id);
            if (!item) {
                continue;
            }
            typedef unsigned char (*IsAvatarItemFn)(CItem*);
            IsAvatarItemFn const is_avatar_item =
                reinterpret_cast<IsAvatarItemFn>(
                    (*reinterpret_cast<void***>(item))[4]);
            if (is_avatar_item(item) != 1 && item->get_rarity() <= 1) {
                opposite_low_value_offer = true;
                break;
            }
        }

        if (proceed_trade()) {
            G_CGameManager()->PutTradeSpace(this);
            if (owner) {
                owner->IncrementTradeGold(own_gold,
                                          own_low_value_offer ? 1 : 0);
            }
            if (opposite) {
                opposite->IncrementTradeGold(
                    opposite_gold, opposite_low_value_offer ? 1 : 0);
            }
        } else {
            cMyTrace trace("bool CTradeSpace::change_trade_state(CUser*, int)",
                           2797, 5);
            trace("change_trade_state, proceed_trade Failed2! %d %d",
                  opposite->getCurCharacNo(), owner->getCurCharacNo());
            trade_states_[owner_index] = 0;
            trade_states_[opposite_index] = 0;
            SendChangeTradeState(packet, owner, opposite, 0);
            SendChangeTradeState(packet, opposite, owner, 0);
        }

        const unsigned short owner_fatigue =
            static_cast<unsigned short>(owner->getCurCharacUsedFatigue());
        const unsigned short opposite_fatigue =
            static_cast<unsigned short>(opposite->getCurCharacUsedFatigue());
        const int random_ratio = static_cast<int>(get_rand_int(25));
        const int random_fatigue = static_cast<int>(get_rand_int(20));
        const int owner_money =
            static_cast<CInventory*>(owner->getCurCharacInvenR())->get_money();
        const long double owner_threshold =
            static_cast<long double>(random_ratio) * 0.01L + 0.7L;
        if (static_cast<int>(own_gold) >
                static_cast<long double>(static_cast<int>(own_gold) +
                                         owner_money) *
                    owner_threshold &&
            static_cast<int>(own_gold) > 9999) {
            const unsigned int opposite_account =
                static_cast<unsigned int>(opposite->get_acc_id());
            const unsigned int owner_etc =
                static_cast<unsigned int>(owner->SetETC(704, opposite_account));
            reinterpret_cast<WongWork::CHackAnalyzer*>(owner->getHackAnalyzer())
                ->addServerHackCnt(
                    owner, static_cast<WongWork::ENUM_HACKTYPE>(0x2c0), 1,
                    owner_etc, 0);
            if (owner_fatigue >= random_fatigue + 120) {
                const unsigned int fatigue_etc = static_cast<unsigned int>(
                    owner->SetETC(701, opposite_account));
                reinterpret_cast<WongWork::CHackAnalyzer*>(
                    owner->getHackAnalyzer())
                    ->addServerHackCnt(
                        owner,
                        static_cast<WongWork::ENUM_HACKTYPE>(0x2bd), 1,
                        fatigue_etc, 0);
            }
            const unsigned int reverse_etc = static_cast<unsigned int>(
                opposite->SetETC(705, owner->get_acc_id()));
            reinterpret_cast<WongWork::CHackAnalyzer*>(
                opposite->getHackAnalyzer())
                ->addServerHackCnt(
                    opposite,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x2c1), 1,
                    reverse_etc, 0);
        }

        const int opposite_money =
            static_cast<CInventory*>(opposite->getCurCharacInvenR())->get_money();
        if (static_cast<int>(opposite_gold) >
                static_cast<long double>(static_cast<int>(opposite_gold) +
                                         opposite_money) *
                    0.9L &&
            static_cast<int>(opposite_gold) > 9999) {
            const unsigned int owner_account =
                static_cast<unsigned int>(owner->get_acc_id());
            const unsigned int opposite_etc =
                static_cast<unsigned int>(opposite->SetETC(704, owner_account));
            reinterpret_cast<WongWork::CHackAnalyzer*>(
                opposite->getHackAnalyzer())
                ->addServerHackCnt(
                    opposite,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x2c0), 1,
                    opposite_etc, 0);
            if (opposite_fatigue > 0x77u) {
                const unsigned int fatigue_etc = static_cast<unsigned int>(
                    opposite->SetETC(701, owner_account));
                reinterpret_cast<WongWork::CHackAnalyzer*>(
                    opposite->getHackAnalyzer())
                    ->addServerHackCnt(
                        opposite,
                        static_cast<WongWork::ENUM_HACKTYPE>(0x2bd), 1,
                        fatigue_etc, 0);
            }
            const unsigned int reverse_etc = static_cast<unsigned int>(
                owner->SetETC(705, opposite->get_acc_id()));
            reinterpret_cast<WongWork::CHackAnalyzer*>(owner->getHackAnalyzer())
                ->addServerHackCnt(
                    owner, static_cast<WongWork::ENUM_HACKTYPE>(0x2c1), 1,
                    reverse_etc, 0);
        }
        return 0;
    }

    case 4:
        if (!IsLocked()) {
            owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 7,
                                      packet);
            return 0;
        }
        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        SendChangeTradeState(packet, owner, opposite, 0);
        SendChangeTradeState(packet, opposite, owner, 0);
        _SetLock(false);
        return 0;

    default:
        owner->SendCmdErrorPacket(ENUM_CMDPACKET_SET_ITEMTRADE_STATE, 1, packet);
        return 1;
    }
}
