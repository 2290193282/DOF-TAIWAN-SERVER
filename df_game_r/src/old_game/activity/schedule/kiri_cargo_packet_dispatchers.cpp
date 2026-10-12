#include "old_game/activity/schedule/kiri_cargo_packet_dispatchers.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <limits>
#include <stdint.h>
#include <string>
#include <utility>

namespace {

const char kKiriCargoPvf[] = "kiricargo.evt";
const uint8_t kPurchaseNotAllowed = 20;
const uint8_t kInsufficientGold = 10;
const uint8_t kBonusNotReady = 21;
const uint8_t kBonusInventoryFailure = 22;
const int kActiveState = 1;
const int kSecondsPerDay = 86400;

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct KiriCargoConfig {
    KiriCargoConfig()
        : purchase_gold(0), expiration_days(0), reward_item_id(0) {}

    int purchase_gold;
    int expiration_days;
    int reward_item_id;
};

class KiriCargoConfigCache {
public:
    KiriCargoConfigCache() : attempted_(false), valid_(false), config_() {}

    const KiriCargoConfig* get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile("Etc/", kKiriCargoPvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        bool has_purchase_gold = false;
        bool has_expiration_days = false;
        bool has_reward_item = false;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            bool success = false;
            if (tag == "[purchase gold]") {
                config_.purchase_gold = ScanInt(&success);
                has_purchase_gold = success;
            } else if (tag == "[day for expiration]") {
                config_.expiration_days = ScanInt(&success);
                has_expiration_days = success;
            } else if (tag == "[reward item]") {
                config_.reward_item_id = ScanInt(&success);
                has_reward_item = success;
            }
            if (!success && (tag == "[purchase gold]" ||
                             tag == "[day for expiration]" ||
                             tag == "[reward item]")) {
                return false;
            }
        }

        if (!has_purchase_gold || !has_expiration_days || !has_reward_item ||
            config_.purchase_gold <= 0 || config_.expiration_days <= 0 ||
            config_.expiration_days >
                std::numeric_limits<int>::max() / kSecondsPerDay ||
            config_.reward_item_id <= 0) {
            return false;
        }

        CDataManager* const data_manager = G_CDataManager();
        CItem* const reward = data_manager
                                  ? data_manager->find_item(
                                        config_.reward_item_id)
                                  : 0;
        return reward && reward->vtable_;
    }

    bool attempted_;
    bool valid_;
    KiriCargoConfig config_;
};

KiriCargoConfigCache& GetConfigCache() {
    static KiriCargoConfigCache cache;
    return cache;
}

EventClassify::CEventScriptMng* GetEventScriptManager() {
    CDataManager* const data_manager = G_CDataManager();
    return data_manager ? data_manager->get_event_script_mng() : 0;
}

bool IsReadyUser(CUser* user) {
    return user && user->get_state() == 3 && user->get_acc_id() > 0 &&
           user->getCurCharacR() && user->getCurCharacInvenR() &&
           user->getCurCharacInvenW() && !user->CheckInTrade() &&
           GetEventScriptManager();
}

uint32_t CurrentSeconds() {
    const long current = GlobalData::s_systemTime_.getCurSec();
    return current > 0 ? static_cast<uint32_t>(current) : 0u;
}

uint32_t ReadExpiry(CUser* user) {
    std::pair<int, int> history;
    if (!user || !user->get_ingame_event_history(
                     KiriCargo::kInGameEventHistoryType, history) ||
        history.second != kActiveState || history.first <= 0) {
        return 0u;
    }
    return static_cast<uint32_t>(history.first);
}

void SaveState(CUser* user, uint32_t expiry, bool active) {
    user->set_ingame_event_history(
        KiriCargo::kInGameEventHistoryType,
        active ? static_cast<int>(expiry) : 0,
        active ? kActiveState : 0);
    EventClassify::CEventScriptMng* const manager = GetEventScriptManager();
    // NO_XA_BOUNDARY: this is the existing asynchronous MyISAM update/insert
    // owner.  It exposes no DB acknowledgement to the packet dispatcher.
    manager->process_update_ingame_history(user);
    user->reset_ingame_event_history_update();
}

uint32_t RemainingSeconds(CUser* user) {
    const uint32_t expiry = ReadExpiry(user);
    if (expiry == 0u) {
        // CLIENT_IDA_EVIDENCE: NOTI602 sub_621910 treats 0xFFFFFFFF as the
        // not-purchased state and does not enter the countdown state machine.
        return std::numeric_limits<uint32_t>::max();
    }
    const uint32_t now = CurrentSeconds();
    return expiry > now ? expiry - now : 0u;
}

bool GiveReward(CUser* user, int reward_item_id) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const reward = data_manager
                              ? data_manager->find_item(reward_item_id)
                              : 0;
    if (!reward || !reward->vtable_) {
        return false;
    }

    ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
    const int slot = user->AddItem(reward_item_id,
                                   1,
                                   static_cast<eItemAddReason>(6),
                                   item_space,
                                   0);
    if (slot < 0) {
        return false;
    }

    // Once AddItem has committed the item to the live inventory there is no
    // proven exact-slot rollback API.  Publish NOTI14 and let the caller clear
    // the cargo state even if the notification itself cannot be delivered;
    // exposing a retry after insertion would deterministically duplicate it.
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1), item_space, slot);
    return true;
}

void SendError(CUser* user, ENUM_CMDPACKET command, uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(command, error);
    }
}

}  // namespace

namespace KiriCargo {

void SendInfo(CUser* user) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_KIRI_CARGO_INFO);
    packet.put_int(static_cast<int>(RemainingSeconds(user)));
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace KiriCargo

Dispatcher_KiriCargoPurchase::Dispatcher_KiriCargoPurchase()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_KiriCargoPurchase::~Dispatcher_KiriCargoPurchase() {}

int Dispatcher_KiriCargoPurchase::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_64FEC0 finalizes CMD542 immediately after
    // BeginCmd, so the native request has no semantic body.  The decrypted
    // transport block may still expose alignment bytes; the 0627 dispatcher
    // architecture leaves them unread.
    const KiriCargoConfig* const config = GetConfigCache().get();
    if (!IsReadyUser(user) || !config || ReadExpiry(user) != 0u) {
        SendError(user,
                  ENUM_CMDPACKET_KIRI_CARGO_PURCHASE,
                  kPurchaseNotAllowed);
        return 0;
    }

    const uint32_t now = CurrentSeconds();
    const uint32_t duration = static_cast<uint32_t>(
        config->expiration_days * kSecondsPerDay);
    if (now == 0u ||
        now > static_cast<uint32_t>(std::numeric_limits<int>::max()) -
                  duration) {
        SendError(user,
                  ENUM_CMDPACKET_KIRI_CARGO_PURCHASE,
                  kPurchaseNotAllowed);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (user->getCurCharacInvenR()->get_money() < config->purchase_gold ||
        inventory->use_money(config->purchase_gold,
                             static_cast<eMoneySubReason>(0),
                             true) != 1) {
        SendError(user,
                  ENUM_CMDPACKET_KIRI_CARGO_PURCHASE,
                  kInsufficientGold);
        return 0;
    }

    SaveState(user, now + duration, true);

    // Stackable inventory slot zero is the canonical money pseudo-slot.
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             0);
    user->SendCmdOkPacket(ENUM_CMDPACKET_KIRI_CARGO_PURCHASE);
    KiriCargo::SendInfo(user);
    return 0;
}

int Dispatcher_KiriCargoPurchase::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_KiriCargoPurchase::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_KiriCargoGetBonus::Dispatcher_KiriCargoGetBonus()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_KiriCargoGetBonus::~Dispatcher_KiriCargoGetBonus() {}

int Dispatcher_KiriCargoGetBonus::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_650010 likewise emits no semantic CMD543 body.
    const KiriCargoConfig* const config = GetConfigCache().get();
    const uint32_t expiry = ReadExpiry(user);
    const uint32_t now = CurrentSeconds();
    if (!IsReadyUser(user) || !config || expiry == 0u || now == 0u ||
        now < expiry) {
        SendError(user,
                  ENUM_CMDPACKET_KIRI_CARGO_GET_BONUS,
                  kBonusNotReady);
        return 0;
    }

    if (!GiveReward(user, config->reward_item_id)) {
        SendError(user,
                  ENUM_CMDPACKET_KIRI_CARGO_GET_BONUS,
                  kBonusInventoryFailure);
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: CMD543 success sub_64FE40 resets the client to the
    // same -1/no-cargo state used by NOTI602.  Clearing the account record
    // makes another 30-day purchase cycle possible, matching phase zero.
    SaveState(user, 0u, false);
    user->SendCmdOkPacket(ENUM_CMDPACKET_KIRI_CARGO_GET_BONUS);
    KiriCargo::SendInfo(user);
    return 0;
}

int Dispatcher_KiriCargoGetBonus::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_KiriCargoGetBonus::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
