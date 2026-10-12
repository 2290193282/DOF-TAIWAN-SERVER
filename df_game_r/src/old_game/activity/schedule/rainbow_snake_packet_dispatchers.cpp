#include "old_game/activity/schedule/rainbow_snake_packet_dispatchers.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace {

const char kRainbowSnakePvf[] = "2013_rainbowsnakeevent.evt";
const uint8_t kOperationFailed = 1;
const int kMaximumStage = 8;
const int kMaximumPoints = 200;
const int kRandomUpperBound = 100;
const int kPartySlotCount = 4;

struct RainbowState {
    RainbowState() : stage(0), points(0) {}

    uint8_t stage;
    uint16_t points;
};

struct RainbowReward {
    RainbowReward() : lower_bound(0), item_id(0), count(0) {}

    int lower_bound;
    int item_id;
    int count;
};

struct RainbowRewardStep {
    RainbowRewardStep() : number(0), rewards() {}

    int number;
    std::vector<RainbowReward> rewards;
};

bool RewardLowerBoundLess(const RainbowReward& lhs,
                          const RainbowReward& rhs) {
    return lhs.lower_bound < rhs.lower_bound;
}

bool RewardStepLess(const RainbowRewardStep& lhs,
                    const RainbowRewardStep& rhs) {
    return lhs.number < rhs.number;
}

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

// This typed cache is intentionally independent of CDataManager, CUser, and
// the old event-manager layouts.  It adds no member to any restored ABI owner
// and loads the authoritative 0828 PVF only when CMD581 first needs a reward.
class RainbowRewardConfig {
public:
    RainbowRewardConfig() : loaded_(false), steps_() {}

    bool select(int step_number, RainbowReward& reward) {
        if (!load() || step_number < 1 || step_number > kMaximumStage) {
            return false;
        }

        const RainbowRewardStep& step =
            steps_[static_cast<size_t>(step_number - 1)];
        const int roll = step_number == kMaximumStage
                             ? 0
                             : static_cast<int>(
                                   get_rand_int(kRandomUpperBound) %
                                   kRandomUpperBound);

        const RainbowReward* selected = 0;
        for (std::vector<RainbowReward>::const_iterator it =
                 step.rewards.begin();
             it != step.rewards.end();
             ++it) {
            if (it->lower_bound <= roll) {
                selected = &*it;
            } else {
                break;
            }
        }
        if (!selected) {
            return false;
        }
        reward = *selected;
        return true;
    }

private:
    bool load() {
        if (loaded_) {
            return true;
        }
        if (!loadRDARScriptFile("Event/", kRainbowSnakePvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        std::vector<RainbowRewardStep> parsed_steps;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag != "[step]") {
                continue;
            }

            bool success = false;
            RainbowRewardStep step;
            step.number = ScanInt(&success);
            if (!success) {
                return false;
            }

            for (;;) {
                RainbowReward reward;
                reward.lower_bound = ScanInt(&success);
                if (!success) {
                    break;
                }
                reward.item_id = ScanInt(&success);
                if (!success) {
                    return false;
                }
                reward.count = ScanInt(&success);
                if (!success) {
                    return false;
                }
                step.rewards.push_back(reward);
            }
            parsed_steps.push_back(step);
        }

        std::sort(parsed_steps.begin(), parsed_steps.end(), RewardStepLess);
        if (parsed_steps.size() != static_cast<size_t>(kMaximumStage)) {
            return false;
        }

        for (int index = 0; index < kMaximumStage; ++index) {
            RainbowRewardStep& step = parsed_steps[static_cast<size_t>(index)];
            if (step.number != index + 1) {
                return false;
            }
            std::sort(step.rewards.begin(),
                      step.rewards.end(),
                      RewardLowerBoundLess);

            const size_t expected_count =
                step.number == kMaximumStage ? 1u : 9u;
            if (step.rewards.size() != expected_count ||
                step.rewards[0].lower_bound != 0) {
                return false;
            }
            for (size_t reward_index = 0;
                 reward_index < step.rewards.size();
                 ++reward_index) {
                const RainbowReward& reward = step.rewards[reward_index];
                if (reward.lower_bound < 0 || reward.lower_bound >= 100 ||
                    reward.item_id <= 0 || reward.count <= 0) {
                    return false;
                }
                if (reward_index > 0 &&
                    step.rewards[reward_index - 1].lower_bound >=
                        reward.lower_bound) {
                    return false;
                }
            }
        }

        steps_.swap(parsed_steps);
        loaded_ = true;
        return true;
    }

    bool loaded_;
    std::vector<RainbowRewardStep> steps_;
};

RainbowRewardConfig& GetRainbowRewardConfig() {
    static RainbowRewardConfig config;
    return config;
}

EventClassify::CEventScriptMng* GetEventScriptManager() {
    CDataManager* const data_manager = G_CDataManager();
    return data_manager ? data_manager->get_event_script_mng() : 0;
}

bool IsReadyUser(CUser* user) {
    // RECONSTRUCTED_ENABLE_RULE: the target 0828 PVF contains the Rainbow
    // reward table but the restored server has no reusable official schedule
    // or event-window owner for this family. Keep type 15 permanently enabled
    // for character-ready sessions instead of claiming an official date gate.
    return user && user->getCurCharacR() && user->get_acc_id() > 0 &&
           GetEventScriptManager();
}

int ClampInt(int value, int minimum, int maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

RainbowState ReadState(CUser* user) {
    RainbowState state;
    std::pair<int, int> history;
    if (user && user->get_ingame_event_history(
                    RainbowSnake::kInGameEventHistoryType, history)) {
        state.stage = static_cast<uint8_t>(
            ClampInt(history.first, 0, kMaximumStage));
        state.points = static_cast<uint16_t>(
            ClampInt(history.second, 0, kMaximumPoints));
    }
    return state;
}

void SaveState(CUser* user, const RainbowState& state) {
    user->set_ingame_event_history(RainbowSnake::kInGameEventHistoryType,
                                   static_cast<int>(state.stage),
                                   static_cast<int>(state.points));
    EventClassify::CEventScriptMng* const manager = GetEventScriptManager();
    // NO_XA_BOUNDARY: this legacy owner only queues an asynchronous MyISAM
    // update/insert and exposes no DB acknowledgement or claim journal to the
    // packet dispatcher. Resetting dirty state matches the existing owners;
    // it must not be described as crash-atomic persistence.
    manager->process_update_ingame_history(user);
    user->reset_ingame_event_history_update();
}

void AddUniqueRecipient(std::vector<CUser*>& recipients, CUser* user) {
    if (!IsReadyUser(user) || user->get_state() <= 2) {
        return;
    }
    if (std::find(recipients.begin(), recipients.end(), user) ==
        recipients.end()) {
        recipients.push_back(user);
    }
}

// RECONSTRUCTED_SERVER_RULE: the 0828 sender pairs CMD579 with P2P type 89,
// proving a shared party gauge but not the old server's recipient loop.  The
// formal owner therefore applies the delta to the requester's current ordinary
// CParty members that are still UID-valid, online, and attached to that same
// party.  A requester without such a party remains the sole recipient.
void CollectPointRecipients(CUser* requester,
                            std::vector<CUser*>& recipients) {
    recipients.clear();
    if (!requester) {
        return;
    }

    CParty* const party = requester->GetParty();
    if (party) {
        for (int seat = 0; seat < kPartySlotCount; ++seat) {
            CUser* const member = party->get_user(seat);
            if (member && party->checkValidUser(seat) == 1 &&
                member->GetParty() == party) {
                AddUniqueRecipient(recipients, member);
            }
        }
    }
    AddUniqueRecipient(recipients, requester);
}

void SendSuccess(CUser* user, ENUM_CMDPACKET command) {
    PacketGuard packet;
    packet.put_header(1, command);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
}

void SendPointState(CUser* user, const RainbowState& state) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: CMD580 handler sub_554FD0 consumes reward stage
    // as u8 and point gauge as u16 only after the framework success byte.
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT);
    packet.put_byte(1);
    packet.put_byte(state.stage);
    packet.put_short(state.points);
    packet.finalize(true);
    user->Send(packet);
}

void SendError(CUser* user, ENUM_CMDPACKET command) {
    if (user) {
        user->SendCmdErrorPacket(command, kOperationFailed);
    }
}

bool GiveReward(CUser* user,
                int step_number,
                const RainbowReward& reward) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const prototype = data_manager
                                 ? data_manager->find_item(reward.item_id)
                                 : 0;
    if (!prototype || !prototype->vtable_) {
        return false;
    }

    // PVF step 8 is the fixed Rainbow creature.  Validate the live prototype
    // before insertion so a mismatched PVF/container cannot put it into an
    // ordinary itemspace and still advance the terminal stage.
    if (step_number == kMaximumStage &&
        (!prototype->vtable_->is_creature_item ||
         !prototype->vtable_->is_creature_item(prototype))) {
        return false;
    }

    ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
    const int slot = user->AddItem(reward.item_id,
                                   reward.count,
                                   static_cast<eItemAddReason>(6),
                                   item_space,
                                   0);
    if (slot < 0) {
        return false;
    }

    // CUser::SendUpdateItem owns NOTI14, including the 0828 creature tail for
    // itemspace 7. AddItem derives that space from the live prototype (with
    // the terminal creature validated above). Once insertion has committed
    // to the live inventory there is no proven rollback API for this exact
    // new slot. A notification lookup/send failure must therefore not turn
    // the claim into a retryable error and create a deterministic duplicate;
    // attempt NOTI14, then let the caller advance the in-memory event state.
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1), item_space, slot);
    return true;
}

}  // namespace

Dispatcher_AddRainbowPoint::Dispatcher_AddRainbowPoint()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_AddRainbowPoint::~Dispatcher_AddRainbowPoint() {}

int Dispatcher_AddRainbowPoint::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint16_t wire_delta = 0;
    if (packet.get_short(wire_delta) != 1) {
        SendError(user, ENUM_CMDPACKET_ADD_RAINBOW_POINT);
        return 0;
    }
    const int delta = (wire_delta & 0x8000u) != 0u
                          ? static_cast<int>(wire_delta) - 0x10000
                          : static_cast<int>(wire_delta);
    if (!IsReadyUser(user) ||
        (delta != -30 && delta != -1 && delta != 1 && delta != 30)) {
        SendError(user, ENUM_CMDPACKET_ADD_RAINBOW_POINT);
        return 0;
    }

    std::vector<CUser*> recipients;
    CollectPointRecipients(user, recipients);
    if (recipients.empty()) {
        SendError(user, ENUM_CMDPACKET_ADD_RAINBOW_POINT);
        return 0;
    }

    for (std::vector<CUser*>::iterator recipient = recipients.begin();
         recipient != recipients.end();
         ++recipient) {
        RainbowState state = ReadState(*recipient);
        state.points = static_cast<uint16_t>(
            ClampInt(static_cast<int>(state.points) + delta,
                     0,
                     kMaximumPoints));
        SaveState(*recipient, state);
    }

    SendSuccess(user, ENUM_CMDPACKET_ADD_RAINBOW_POINT);
    return 0;
}

int Dispatcher_AddRainbowPoint::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_AddRainbowPoint::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_RequestRainbowPoint::Dispatcher_RequestRainbowPoint()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RequestRainbowPoint::~Dispatcher_RequestRainbowPoint() {}

int Dispatcher_RequestRainbowPoint::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    (void)packet;
    if (!IsReadyUser(user)) {
        SendError(user, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT);
        return 0;
    }
    SendPointState(user, ReadState(user));
    return 0;
}

int Dispatcher_RequestRainbowPoint::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestRainbowPoint::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_RequestRainbowPointReward::Dispatcher_RequestRainbowPointReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RequestRainbowPointReward::~Dispatcher_RequestRainbowPointReward() {}

int Dispatcher_RequestRainbowPointReward::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    (void)packet;
    if (!IsReadyUser(user)) {
        SendError(user, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT_REWARD);
        return 0;
    }

    RainbowState state = ReadState(user);
    if (state.points < kMaximumPoints || state.stage >= kMaximumStage) {
        SendError(user, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT_REWARD);
        return 0;
    }

    const int reward_step = static_cast<int>(state.stage) + 1;
    RainbowReward reward;
    if (!GetRainbowRewardConfig().select(reward_step, reward) ||
        !GiveReward(user, reward_step, reward)) {
        SendError(user, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT_REWARD);
        return 0;
    }

    state.stage = static_cast<uint8_t>(reward_step);
    state.points = 0;
    SaveState(user, state);
    SendSuccess(user, ENUM_CMDPACKET_REQUEST_RAINBOW_POINT_REWARD);
    return 0;
}

int Dispatcher_RequestRainbowPointReward::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestRainbowPointReward::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
