#include "old_game/economy/inventory/packet_dispatcher_delete_item.h"

#include <stdint.h>

#include "common/config.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

enum {
    kCmdDeleteItem = 18,
    kMaxDeleteItems = 100,
    kInventoryProtection = 29,
    kEquipmentDeleteProtection = 70,
    kDeleteItemChecksumHack = 603,
    kDeathTowerSkillMaterialItem = 6515,
    kHeroMissionCubeItem = 3037,
};

const char kReadSignature[] =
    "virtual int DisPatcher_DeleteItem::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int DisPatcher_DeleteItem::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8595150
// IDA_NAME: _ZN21DisPatcher_DeleteItemC2Ev
// DEMANGLED: DisPatcher_DeleteItem::DisPatcher_DeleteItem(void)
DisPatcher_DeleteItem::DisPatcher_DeleteItem()
    : IPacketDispatcher<MSG_DELETE_ITEM,
                        stDeleteItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82321F4
// IDA_NAME: _ZN21DisPatcher_DeleteItemD2Ev
// DEMANGLED: DisPatcher_DeleteItem::~DisPatcher_DeleteItem()
// IDA_EA: 0x8232224
// IDA_NAME: _ZN21DisPatcher_DeleteItemD0Ev
// DEMANGLED: DisPatcher_DeleteItem::~DisPatcher_DeleteItem()
DisPatcher_DeleteItem::~DisPatcher_DeleteItem() {}

// IDA_EA: 0x81C17C0
// IDA_NAME: _ZN21DisPatcher_DeleteItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DeleteItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DeleteItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DELETE_ITEM& input = static_cast<MSG_DELETE_ITEM&>(message);
    char item_space = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(2790, kReadSignature, 0, 0);
    }
    input.item_space = item_space;

    char item_count = 0;
    if (packet.get_byte(item_count) != 1) {
        return LineFunc(2791, kReadSignature, 0, 0);
    }
    input.item_count = item_count;
    if (input.item_count > kMaxDeleteItems) {
        return LineFunc(2795, kReadSignature, 0, 0);
    }
    for (int index = 0; input.item_count > index; ++index) {
        int16_t delete_type = 0;
        if (packet.get_short(delete_type) != 1) {
            return LineFunc(2800, kReadSignature, 0, 0);
        }
        input.delete_type[index] = delete_type;

        int16_t slot = 0;
        if (packet.get_short(slot) != 1) {
            return LineFunc(2801, kReadSignature, 0, 0);
        }
        input.slots[index] = slot;

        uint32_t item_id = 0;
        if (packet.get_int(item_id) != 1) {
            return LineFunc(2802, kReadSignature, 0, 0);
        }
        input.item_ids[index] = item_id;

        int32_t count = 0;
        if (packet.get_int(count) != 1) {
            return LineFunc(2803, kReadSignature, 0, 0);
        }
        input.counts[index] = count;
        if (input.counts[index] <= 0) {
            return LineFunc(2805, kReadSignature, 0, 0);
        }
    }
    int32_t inventory_checksum = 0;
    if (packet.get_int(inventory_checksum) == 1) {
        input.inventory_checksum = inventory_checksum;
        return 0;
    }
    return LineFunc(2808, kReadSignature, 0, 0);
}

// IDA_EA: 0x81C1A6C
// IDA_NAME: _ZN21DisPatcher_DeleteItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_DeleteItem::check_error(CUser *,MSG_BASE &)
int DisPatcher_DeleteItem::check_error(CUser* user, MSG_BASE& message) {
    MSG_DELETE_ITEM& input = static_cast<MSG_DELETE_ITEM&>(message);
    if (user->getCurCharacInvenCheckSum() != input.inventory_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(kDeleteItemChecksumHack),
            1u,
            0u,
            0u);
    }
    return 0;
}

// IDA_EA: 0x81C1AD6
// IDA_NAME: _ZN21DisPatcher_DeleteItem13exercise_modeEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DeleteItem::exercise_mode(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DeleteItem::exercise_mode(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& result) {
    MSG_DELETE_ITEM& input = static_cast<MSG_DELETE_ITEM&>(message);
    stDeleteItemParam& param = static_cast<stDeleteItemParam&>(result);

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return 0;
    }

    WongWork::CDeathTower* death_tower = 0;
    if (user->get_state() == 10) {
        death_tower = user->getDeathTower();
    }
    bool unlimited_stackable_items = false;
    if (death_tower) {
        CDungeon* const dungeon = death_tower->getCDungeonMGr()->getDungeon();
        unlimited_stackable_items = dungeon->limitOfStackableItemInTower() != 1;
        if (dungeon->limitOfStackableItemInTower()) {
            for (int index = 0; input.item_count > index; ++index) {
                CItem* const item = G_CDataManager()->find_item(
                    input.item_ids[index]);
                if (item && item->get_index() == kDeathTowerSkillMaterialItem) {
                    death_tower->map_monster_kill_checker_.setUseSkillMaterial(
                        static_cast<uint32_t>(
                            GlobalData::s_systemTime_.getCurTickCount()));
                }
            }
        }
    }

    if (party->party_member_coin_limit_ != 1 ||
        (user->get_state() != 5 && !unlimited_stackable_items)) {
        return 0;
    }

    param.item_count_ = 0;
    param.item_space_ = input.item_space;
    for (int index = 0; input.item_count > index; ++index) {
        param.slots_[index] = input.slots[index];
        param.counts_[index] = input.counts[index];
        param.delete_type_[index] = input.delete_type[index];
    }
    return 1;
}

// IDA_EA: 0x81C1CF6
// IDA_NAME: _ZN21DisPatcher_DeleteItem16restricted_checkEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_DeleteItem::restricted_check(CUser *,MSG_BASE &)
int DisPatcher_DeleteItem::restricted_check(CUser* user, MSG_BASE&) {
    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(user, RestrictCategory::AuthMobile, 1)) {
        return 209;
    }
    return GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(kInventoryProtection));
}

// IDA_EA: 0x81C1D5C
// IDA_NAME: _ZN21DisPatcher_DeleteItem15condition_checkEP5CUserR8MSG_BASER10INVEN_TYPEi
// DEMANGLED: DisPatcher_DeleteItem::condition_check(CUser *,MSG_BASE &,INVEN_TYPE &,int)
int DisPatcher_DeleteItem::condition_check(CUser* user,
                                           MSG_BASE& message,
                                           INVEN_TYPE& inventory_type,
                                           int index) {
    MSG_DELETE_ITEM& input = static_cast<MSG_DELETE_ITEM&>(message);
    if (inventory_type == static_cast<INVEN_TYPE>(0) &&
        input.slots[index] <= oldsrv::config::kAvatarEquipmentTypeLast) {
        return 23;
    }
    const int actual_inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(input.item_space));
    return user->CheckItemLock(actual_inventory_type, input.slots[index]) ?
        213 : 0;
}

// IDA_EA: 0x81C1DDA
// IDA_NAME: _ZN21DisPatcher_DeleteItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DeleteItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DeleteItem::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& result) {
    MSG_DELETE_ITEM& input = static_cast<MSG_DELETE_ITEM&>(message);
    stDeleteItemParam& param = static_cast<stDeleteItemParam&>(result);

    param.error_ = check_error(user, message);
    if (param.error_ > 0) {
        return 0;
    }
    if (param.error_ < 0) {
        return LineFunc(2958, kProcessSignature, param.error_, user->get_acc_id());
    }
    if (exercise_mode(user, message, result)) {
        return 0;
    }

    INVEN_TYPE inventory_type = static_cast<INVEN_TYPE>(6);
    if (input.item_space == 1) {
        inventory_type = static_cast<INVEN_TYPE>(2);
    } else if (input.item_space == 3) {
        inventory_type = static_cast<INVEN_TYPE>(0);
    }
    if (inventory_type == static_cast<INVEN_TYPE>(2)) {
        param.error_ = 23;
        param.item_space_ = input.item_space;
        return 0;
    }

    bool requires_restriction_check = false;
    for (int index = 0; input.item_count > index; ++index) {
        if (input.delete_type[index] <= 1) {
            requires_restriction_check = true;
            break;
        }
    }
    if (requires_restriction_check) {
        param.error_ = restricted_check(user, message);
        if (param.error_ != 0) {
            param.item_space_ = input.item_space;
            return 0;
        }
    }

    for (int index = 0; input.item_count > index; ++index) {
        param.error_ = condition_check(user, message, inventory_type, index);
        if (param.error_ != 0) {
            param.item_space_ = input.item_space;
            return 0;
        }

        eItemDelReason delete_reason = static_cast<eItemDelReason>(3);
        if (input.delete_type[index] == 1) {
            delete_reason = static_cast<eItemDelReason>(
                user->get_state() == 3 ? 5 : 6);
        }

        const int actual_inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(input.item_space));
        Inven_Item source = user->getCurCharacInvenR()->GetInvenSlot(
            actual_inventory_type, input.slots[index]);
        if (static_cast<int>(delete_reason) == 3) {
            Global::CBossTowerFunc::UseItem(user, source);
        }
        if (source.item_id_ == kHeroMissionCubeItem) {
            ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
                user,
                HeroMissionCondition::MissionNo::USE_CUBE,
                static_cast<unsigned int>(input.counts[index]));
        }
        if (user->get_state() == 3 &&
            static_cast<int>(delete_reason) == 5 &&
            input.item_ids[index] == 0) {
            const int security_error = GlobalData::s_pSecuProtectionField->Check(
                user,
                static_cast<SECURITY_PROTCTION>(kEquipmentDeleteProtection));
            if (security_error != 0) {
                param.error_ = security_error;
                return 0;
            }
        }

        const int delete_error = user->delete_item(
            input.item_space,
            input.slots[index],
            input.item_ids[index],
            input.counts[index],
            delete_reason);
        if (delete_error > 0) {
            cMyTrace trace(kProcessSignature, 3114, 5);
            trace("delete_item failed in the middle of deleting items. %d/%d",
                  index,
                  input.item_count);
            if (index > 1) {
                PacketGuard partial;
                partial.put_header(1, kCmdDeleteItem);
                partial.put_byte(1);
                partial.put_byte(input.item_space);
                partial.put_byte(static_cast<unsigned char>(index - 1));
                for (int sent = 0; sent < index - 1; ++sent) {
                    partial.put_short(input.slots[sent]);
                    partial.put_int(input.counts[sent]);
                    partial.put_short(input.delete_type[sent]);
                }
                partial.finalize(1);
                user->Send(partial);
            }

            PacketGuard failure;
            failure.put_header(1, kCmdDeleteItem);
            failure.put_byte(0);
            failure.put_byte(static_cast<unsigned char>(delete_error));
            failure.put_byte(input.item_space);
            failure.finalize(1);
            user->Send(failure);
            return LineFunc(3144, kProcessSignature, 0, user->get_acc_id());
        }

        if (!source.isEmpty() && static_cast<int>(delete_reason) == 5) {
            CItem* const source_item = G_CDataManager()->find_item(source.item_id_);
            if (source_item) {
                if (source_item->is_stackable()) {
                    source.set_add_info(input.counts[index]);
                }
                user->Add_RedeemInfo(source, 0, false);
            }
        }

        CItem* const requested_item = G_CDataManager()->find_item(
            input.item_ids[index]);
        if (requested_item) {
            int sell_price = 0;
            G_Store()->GetSellItemPrice(source,
                                        requested_item,
                                        static_cast<short>(input.counts[index]),
                                        false,
                                        sell_price);
            if (user->get_state() != 8) {
                GetInstanceValueStatistic()->AddValueStatistic(
                    static_cast<VALUE_STATISTIC_FIELD>(22), user, sell_price);
            }
            if (user->get_state() == 5) {
                switch (input.delete_type[index]) {
                    case 2:
                        GetInstanceCubeStatistic()->collectCubeStatistics(
                            requested_item,
                            input.counts[index],
                            user,
                            static_cast<CUBE_STATISTIC_FIELD>(103));
                        break;
                    case 3:
                        GetInstanceCubeStatistic()->collectCubeStatistics(
                            requested_item,
                            input.counts[index],
                            user,
                            static_cast<CUBE_STATISTIC_FIELD>(104));
                        break;
                    case 4:
                        GetInstanceCubeStatistic()->collectCubeStatistics(
                            requested_item,
                            input.counts[index],
                            user,
                            static_cast<CUBE_STATISTIC_FIELD>(105));
                        break;
                    default:
                        break;
                }
            }
        }
    }

    param.item_space_ = input.item_space;
    param.item_count_ = input.item_count;
    for (int index = 0; input.item_count > index; ++index) {
        param.slots_[index] = input.slots[index];
        param.counts_[index] = input.counts[index];
        param.delete_type_[index] = input.delete_type[index];
    }
    return 0;
}

// IDA_EA: 0x81C2792
// IDA_NAME: _ZN21DisPatcher_DeleteItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_DeleteItem::send(CUser *,ParamBase &)
void DisPatcher_DeleteItem::send(CUser* user, ParamBase& result) {
    stDeleteItemParam& param = static_cast<stDeleteItemParam&>(result);
    if (param.error_ == 0x7fffffff) {
        return;
    }

    PacketGuard packet;
    if (param.error_ != 0) {
        packet.put_header(1, kCmdDeleteItem);
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(param.error_));
        packet.put_byte(param.item_space_);
        packet.finalize(1);
        user->Send(packet);
        return;
    }

    packet.put_header(1, kCmdDeleteItem);
    packet.put_byte(1);
    packet.put_byte(param.item_space_);
    packet.put_byte(param.item_count_);
    for (int index = 0; param.item_count_ > index; ++index) {
        packet.put_short(param.slots_[index]);
        packet.put_int(param.counts_[index]);
        packet.put_short(param.delete_type_[index]);
    }
    packet.finalize(1);
    user->Send(packet);
}
