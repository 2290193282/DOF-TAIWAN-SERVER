#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/os_api.h"

namespace {

int DeleteRequiredItem(CUser* user,
                       int item_id,
                       int count,
                       bool use_vilmark_lookup) {
    CInventory* const inventory_read = user->getCurCharacInvenR();
    const int slot = use_vilmark_lookup
                         ? inventory_read->check_vilmark_dungeon_item_exist(item_id)
                         : inventory_read->check_item_exist(item_id);
    if (slot == -1 || inventory_read->GetInvenSlot(1, slot).get_add_info() <
                          count) {
        return 17;
    }
    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            slot,
            count,
            static_cast<eItemDelReason>(14),
            true) != 1) {
        return 17;
    }

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), slot, packet);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

void SendBloodDungeonAdmissionMessage(CUser* user, int admission_fee) {
    char text[256] = {};
    const char* const format =
        g_scriptStringManager_.findString(4, "game_server_msg_110", 0);
    OS_API::snprintf(text, sizeof(text), format ? format : "", admission_fee);

    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(static_cast<int>(std::strlen(text)));
    packet.put_str(text, std::strlen(text));
    packet.finalize(true);
    user->Send(packet);
}

int FinishSuccessfulDungeonStart(const CDungeon* dungeon,
                                 CUser* user,
                                 int dimension_mode) {
    if (dimension_mode != 1 &&
        dungeon_runtime::IsLimitedInCountDungeon(dungeon->get_index()) &&
        !dungeon_runtime::TryConsumeInCount(user, dungeon->get_index())) {
        return 173;
    }
    return 0;
}

}  // namespace

// IDA_EA: 0x859FCFC
// IDA_NAME: _ZN6CParty22do_after_dungeon_startEPK8CDungeonP5CUser
// DEMANGLED: CParty::do_after_dungeon_start(CDungeon const*,CUser *)
int CParty::do_after_dungeon_start(const CDungeon* dungeon, CUser* user) {
    if (dungeon->_do_after_dungeon_start(user) != 1) {
        return 173;
    }
    user->setCurCharacLastPlayDungeonIndex(dungeon->get_index());

    const int dimension_mode = battle_field_.instance_dungeon_mode_;
    const uint8_t blood_type = dungeon->blood_dungeon_type_;
    if (blood_type == 1u) {
        if (dimension_mode == 1) {
            SendBloodDungeonAdmissionMessage(user, 0);
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
        const int admission_fee = get_blood_dungeon_admission_fee(user);
        user->getCurCharacInvenW()->use_money(
            admission_fee, static_cast<eMoneySubReason>(34), true);
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 0);
        SendBloodDungeonAdmissionMessage(user, admission_fee);

        CCharacBloodDungeon* const blood =
            static_cast<CCharacBloodDungeon*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
        ServerParameterScript* const parameters =
            G_CDataManager()->get_server_parameter_script();
        const int entry_count = blood->GetEnterCount();
        const int lottery_point = parameters->GetLottePoint(entry_count);
        const int price_average =
            parameters->GetPriceAverage(user->get_charac_level());
        blood->IncEnterCount();
        blood->IncPoint(lottery_point);
        blood->IncUseGold(admission_fee - price_average);
    }

    const int dimension_type = dungeon->get_dimension_possible();
    if (dimension_type > 0) {
        if (dimension_mode == 1) {
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
        const int dimension_index = dimension_type - 1;
        if (static_cast<signed char>(user->getCurCharacR()
                                         ->fields_
                                         .dimension_inout_[dimension_index]) <=
                0 &&
            user->isGMUser() != 1) {
            return 173;
        }
        user->subDemensionInoutValue(static_cast<char>(dimension_index));
        G_GameWorld()->send_party_info_to_all(this, 1);
    }
    if (blood_type == 2u && dimension_mode != 1) {
        user->subUltimateInoutValue(2);
    }
    if (dungeon->isTournamentDungeon() && dungeon->get_index() == 122 &&
        dimension_mode != 1) {
        const int remaining = user->getUltimateInoutValue(0);
        if (remaining <= 0) {
            if (user->isGMUser() != 1) {
                return 173;
            }
        } else {
            user->subUltimateInoutValue(0);
        }
    }

    if (time_spiral::IsDungeon(dungeon->get_index())) {
        if (dimension_mode == 1) {
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
        if (time_spiral::ShouldConsumeTicket(
                this, user, dungeon->get_index())) {
            const RequiredItem& required = dungeon->getRequiredItem();
            const int result = DeleteRequiredItem(
                user, required.item_id, required.item_count, false);
            if (result != 0) return result;
            time_spiral::MarkTicketConsumed(
                this, user, dungeon->get_index());
        } else if (!time_spiral::IsTicketConsumed(
                       this, user, dungeon->get_index())) {
            // A missing/mismatched run generation must not turn a checked
            // TimeSpiral ticket into a free entry.
            return 17;
        }
    }

    // IDA 0x85A02C5 reads CDungeon+0x89c (ancient_dungeon_) before
    // joining the tournament branch.  Event dungeons are handled by the
    // distinct +0x85c branch below.
    const bool legacy_required_item_owner =
        dungeon->ancient_dungeon_ || dungeon->isTournamentDungeon() ||
        dungeon->isSAODungeon();
    if (legacy_required_item_owner) {
        if (dimension_mode == 1) {
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
        const int item_id = dungeon->required_item_.item_id;
        if (item_id != -1 &&
            dungeon->required_item_.delete_after_start == 1u) {
            const int result = DeleteRequiredItem(
                user,
                item_id,
                dungeon->required_item_.item_count,
                dungeon->get_index() == 1500);
            if (result != 0) {
                return result;
            }
        }
    }

    // 140828 also assigns [required item] directly to ordinary, non-event
    // fixed-difficulty dungeons (notably Dragon Nest 3538/3539).  Admission
    // already validates every member's item, but the 0627 type-only owner
    // above never consumed it.  The per-seat start-map-finish flag calls this
    // transaction once for a successful entry; TimeSpiral and event dungeons
    // retain their dedicated run/event owners and are excluded from this
    // common required-item path.  Tower of Despair has already applied its
    // first-entry-free/repeat-entry ticket rule in the virtual hook above;
    // charging here again rejects the free entry or consumes a second ticket.
    if (!legacy_required_item_owner &&
        !dungeon->isTowerOfDespairDungeon() &&
        !time_spiral::IsDungeon(dungeon->get_index()) &&
        !dungeon->event_dungeon_ && dungeon->required_item_.item_id != -1 &&
        dungeon->required_item_.delete_after_start == 1u) {
        if (dimension_mode == 1) {
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
        const int result = DeleteRequiredItem(
            user,
            dungeon->required_item_.item_id,
            dungeon->required_item_.item_count,
            false);
        if (result != 0) {
            return result;
        }
    }

    if (!dungeon->event_dungeon_) {
        if (dungeon->village_attack_revenge_dungeon_) {
            const int revenge_count = user->GetCurRevengeDungeonCount();
            if (revenge_count == 0 ||
                dungeon->revenge_dungeon_entry_limit_ < revenge_count) {
                user->SetCurRevengeDungeonCount(0);
                return 17;
            }
            user->DecreaseRevengeDungeonTicket();
            user->VillageAttack_DBUpdate();
        }
        return FinishSuccessfulDungeonStart(dungeon, user, dimension_mode);
    }

    if (dungeon->IsOpenScheduleDungeon() != 1) {
        return 21;
    }
    const int event_item_id = dungeon->additional_required_item_.item_id;
    const int ordinary_item_id = dungeon->required_item_.item_id;
    if ((event_item_id == -1 && ordinary_item_id == -1) ||
        (event_item_id != -1 &&
         dungeon->additional_required_item_.delete_after_start != 1u) ||
        (ordinary_item_id != -1 &&
         dungeon->required_item_.delete_after_start != 1u)) {
        return FinishSuccessfulDungeonStart(dungeon, user, dimension_mode);
    }

    int result = 17;
    if (event_item_id != -1) {
        result = do_after_dungeon_start_checkndelete_item(
            dungeon->additional_required_item_, user);
        if (result == 0) {
            user->SetDeleteEventDungeonInvitation(true);
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
    }
    if (ordinary_item_id != -1) {
        result = do_after_dungeon_start_checkndelete_item(
            dungeon->required_item_, user);
        if (result == 0) {
            user->SetDeleteEventDungeonInvitation(true);
            return FinishSuccessfulDungeonStart(
                dungeon, user, dimension_mode);
        }
    }
    return result;
}

// IDA_EA: 0x85A0756
// IDA_NAME: _ZN6CParty40do_after_dungeon_start_checkndelete_itemERK12RequiredItemP5CUser
// DEMANGLED: CParty::do_after_dungeon_start_checkndelete_item(RequiredItem const&,CUser *)
int CParty::do_after_dungeon_start_checkndelete_item(const RequiredItem& item,
                                                      CUser* user) {
    return DeleteRequiredItem(user, item.item_id, item.item_count, false);
}
