#include "old_game/activity/schedule/packet_dispatcher_event_server_level_up.h"

#include <climits>
#include <stdint.h>

#include <vector>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const ENUM_CMDPACKET kCommand =
    ENUM_CMDPACKET_REQUEST_EVENT_SERVER_LEVEL_UP;
const unsigned char kClientMainInventorySelector = 0;
const INVEN_TYPE kServerMainInventory = static_cast<INVEN_TYPE>(1);
const StackableAction::Type kLevelUpTicketAction =
    static_cast<StackableAction::Type>(38);

enum EventServerLevelUpError {
    kErrorLevelCondition = 3,
    kErrorItem = 17,
    kErrorState = 23,
    kErrorServerType = 99,
};

void SendError(CUser* user, EventServerLevelUpError error) {
    user->SendCmdErrorPacket(kCommand, static_cast<unsigned char>(error));
}

bool ResolveLevelRange(const CStackableItem* ticket, int& level_range) {
    const int declared_range = ticket->booster_data_mode_;
    if (declared_range <= 0) {
        return false;
    }
    if (declared_range == 1) {
        level_range = 1;
        return true;
    }

    // PVF random level-up tickets declare their maximum through
    // [levelup range] and carry the actual 1..N weighted result in the
    // ordinary booster table.  CStackableItem owns that roll algorithm.
    std::vector<stStackableBoosterItemInfo_t> results;
    ticket->getBoosterItem(results);
    // Every 0828 level-up-ticket booster contains one draw.  Treat a
    // malformed/multi-draw table as unusable instead of silently discarding
    // extra results and granting a PVF-dependent amount.
    if (results.size() != 1u) {
        return false;
    }

    const int rolled_range = static_cast<int>(results.front().item_count);
    if (rolled_range <= 0 || rolled_range > declared_range) {
        return false;
    }
    level_range = rolled_range;
    return true;
}

bool ResolveTargetLevel(const CStackableItem* ticket,
                        int current_level,
                        int level_value,
                        int& target_level) {
    switch (ticket->levelup_mode()) {
        case -1: {
            // Omitted [levelup mode]: preserve the legacy additive ticket
            // contract and cap only at the server's character-level ceiling.
            const int remaining_levels =
                oldsrv::config::kCharacterLevelMaximum - current_level;
            target_level = current_level +
                (level_value < remaining_levels ? level_value
                                                : remaining_levels);
            return target_level > current_level;
        }
        case 1:
            // Both official 0828 mode-1 tickets declare [levelup range] 15
            // and say "level up to level 15".  Their booster result is the
            // absolute destination, not an amount to add to the current
            // level.
            if (level_value <= current_level ||
                level_value > oldsrv::config::kCharacterLevelMaximum) {
                return false;
            }
            target_level = level_value;
            return true;
        default:
            // No official 0828 evidence establishes another mode.  Reject
            // it rather than silently treating a new policy as additive.
            return false;
    }
}

void SendSuccess(CUser* user,
                 uint16_t slot,
                 uint8_t item_space,
                 int gained_exp,
                 int gained_sp,
                 int gained_sfp) {
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET, kCommand);
    response.put_byte(1);
    response.put_short(slot);
    response.put_byte(item_space);
    response.put_int(gained_exp);
    response.put_short(static_cast<uint16_t>(gained_sp));
    response.put_short(static_cast<uint16_t>(gained_sfp));
    response.finalize(true);
    user->Send(response);
}

}  // namespace

Dispatcher_RequestEventServerLevelUp::
    Dispatcher_RequestEventServerLevelUp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RequestEventServerLevelUp::~
    Dispatcher_RequestEventServerLevelUp() {}

int Dispatcher_RequestEventServerLevelUp::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    return dispatch_sig(user, packet);
}

int Dispatcher_RequestEventServerLevelUp::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    if (!user) {
        return 0;
    }

    uint16_t slot = 0;
    uint8_t item_space = 0;
    if (packet.get_short(slot) != 1 || packet.get_byte(item_space) != 1) {
        SendError(user, kErrorItem);
        return 0;
    }

    if (!user->getCurCharacR() || user->get_state() != 3 ||
        user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        SendError(user, kErrorState);
        return 0;
    }
    if (item_space != kClientMainInventorySelector ||
        slot > oldsrv::config::kMainInventoryLastSlot) {
        SendError(user, kErrorItem);
        return 0;
    }
    if (user->CheckItemLock(static_cast<int>(kServerMainInventory), slot)) {
        SendError(user, kErrorState);
        return 0;
    }

    const CInventory* const readable_inventory = user->getCurCharacInvenR();
    if (!readable_inventory) {
        SendError(user, kErrorState);
        return 0;
    }
    const Inven_Item source = readable_inventory->GetInvenSlot(
        static_cast<int>(kServerMainInventory), slot);
    if (source.isEmpty()) {
        SendError(user, kErrorItem);
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        SendError(user, kErrorItem);
        return 0;
    }
    CItem* const prototype = data_manager->find_item(source.getKey());
    if (!prototype || prototype->is_stackable() != 1 ||
        !user->verifyStackableItemProcess(kLevelUpTicketAction, prototype)) {
        SendError(user, kErrorItem);
        return 0;
    }
    CStackableItem* const ticket = static_cast<CStackableItem*>(prototype);

    const int current_level = user->get_charac_level();
    if (ticket->check_able_To_use(user->get_charac_job(), current_level) != 0 ||
        current_level > ticket->script_value_3cc_ ||
        current_level >= oldsrv::config::kCharacterLevelMaximum) {
        SendError(user, kErrorLevelCondition);
        return 0;
    }

    CEnvironment* const environment = G_CEnvironment();
    if (!environment) {
        SendError(user, kErrorServerType);
        return 0;
    }
    const int current_server_type = environment->get_server_type();
    const int required_server_type = ticket->script_value_3d0_;
    if (required_server_type != -1 &&
        required_server_type != current_server_type) {
        SendError(user, kErrorServerType);
        return 0;
    }
    // The canonical EXP owner intentionally suppresses gains above level 49
    // on event-server type 2.  Reject before deletion so CMD415 never reports
    // an EXP award that the owner discarded.
    if (current_server_type == 2 && current_level > 49) {
        SendError(user, kErrorLevelCondition);
        return 0;
    }

    int level_range = 0;
    if (!ResolveLevelRange(ticket, level_range)) {
        SendError(user, kErrorLevelCondition);
        return 0;
    }
    int target_level = 0;
    if (!ResolveTargetLevel(ticket, current_level, level_range, target_level)) {
        SendError(user, kErrorLevelCondition);
        return 0;
    }

    const int target_exp = data_manager->get_level_exp(target_level);
    const int bottom_exp = data_manager->get_level_exp(current_level);
    const int next_exp = data_manager->get_level_exp(current_level + 1);
    const int saved_exp = user->get_charac_exp();
    const int effective_exp =
        saved_exp < bottom_exp || saved_exp > next_exp ? bottom_exp : saved_exp;
    const int64_t gained_exp_64 =
        static_cast<int64_t>(target_exp) - static_cast<int64_t>(effective_exp);
    if (gained_exp_64 <= 0 || gained_exp_64 > INT_MAX) {
        SendError(user, kErrorLevelCondition);
        return 0;
    }
    const int gained_exp = static_cast<int>(gained_exp_64);

    // Once deletion succeeds, rewardExp is the canonical no-failure level,
    // SP/SFP, quest, guild, statistic and save-dirty transaction owner.
    // Obtain the writable inventory only at the mutation boundary so rejected
    // requests do not mark an otherwise unchanged inventory dirty.
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        SendError(user, kErrorState);
        return 0;
    }
    if (inventory->delete_item(kServerMainInventory,
                               slot,
                               1,
                               static_cast<eItemDelReason>(10),
                               true) != 1) {
        SendError(user, kErrorItem);
        return 0;
    }

    int gained_sp = 0;
    int gained_sfp = 0;
    int previous_sp = 0;
    int current_sp = 0;
    user->rewardExp(gained_exp,
                    gained_sp,
                    gained_sfp,
                    previous_sp,
                    current_sp,
                    static_cast<eExpAddReason>(0),
                    true);
    SendSuccess(user,
                slot,
                item_space,
                gained_exp,
                gained_sp,
                gained_sfp);
    return 0;
}

int Dispatcher_RequestEventServerLevelUp::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestEventServerLevelUp::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
