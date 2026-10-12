#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"

#include <algorithm>
#include <cstring>
#include <limits.h>
#include <map>
#include <time.h>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/system_time.h"
#include "common/manager/user_history_log_abi.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/npc/npc_common_data.h"
#include "old_data/PVF/npc/npc_script.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace {

const int kMainInventory = 1;
const int kDbSignalUpdateNpcRelationship = 367;
const int kMaximumRelationships = 50;
const int kNpcInteractionHalfWidth = 80;
const int kNpcInteractionHalfHeight = 150;
const uint8_t kErrorInvalidState = 1;
const uint8_t kErrorInventoryFull = 4;
const uint8_t kErrorInvalidItem = 7;
const uint8_t kErrorItemChanged = 17;
const uint8_t kErrorUnavailable = 22;
const uint8_t kErrorItemLocked = 95;
const eItemDelReason kNpcGiftDeleteReason = static_cast<eItemDelReason>(3);
const eItemAddReason kNpcGiftAddReason = static_cast<eItemAddReason>(8);
const eItemAddReason kNpcGiftStagingAddReason =
    static_cast<eItemAddReason>(7);

const char kReadSignature[] =
    "virtual int Dispatcher_GiveGiftToNPC::read(PacketBuf&, MSG_BASE&)";

struct NpcRelationshipSavePayload {
    int32_t charac_no;
    int32_t count;
    stNPCRelationship_t relationships[kMaximumRelationships];
};

static_assert(sizeof(stNPCRelationship_t) == 12u,
              "signal367 relationship record must remain 12 bytes");
static_assert(sizeof(NpcRelationshipSavePayload) == 608u,
              "signal367 payload must remain exactly 608 bytes");

struct UserNpcRelationshipState {
    UserNpcRelationshipState()
        : charac_no(0),
          loaded(false),
          dirty(false),
          generation(0),
          queued_generation(0),
          relationships() {}

    int32_t charac_no;
    bool loaded;
    bool dirty;
    uint32_t generation;
    uint32_t queued_generation;
    std::vector<stNPCRelationship_t> relationships;
};

typedef std::map<CUser*, UserNpcRelationshipState> UserStateMap;

UserStateMap& GetUserStates() {
    static UserStateMap states;
    return states;
}

class NpcCommonCache {
public:
    NpcCommonCache() : attempted_(false), valid_(false), data_() {
        data_.clear();
    }

    const stNPCCommonData_t* get() {
        if (!attempted_) {
            attempted_ = true;
            data_.clear();
            valid_ = loadNPCCommonData(data_, "common.dat") == 0 &&
                     data_.max_favor_value > 0 &&
                     data_.favor_level_values[1] > 0 &&
                     data_.favor_level_values[1] <=
                         data_.favor_level_values[2] &&
                     data_.favor_level_values[2] <=
                         data_.favor_level_values[3] &&
                     data_.favor_level_values[3] <= data_.max_favor_value;
        }
        return valid_ ? &data_ : 0;
    }

private:
    bool attempted_;
    bool valid_;
    stNPCCommonData_t data_;
};

NpcCommonCache& GetNpcCommonCache() {
    static NpcCommonCache cache;
    return cache;
}

uint32_t CurrentSecond() {
    return static_cast<uint32_t>(GlobalData::system_time()
                                     ? GlobalData::system_time()->getCurSec()
                                     : time(0));
}

uint32_t ResetDayKey(uint32_t second) {
    CEnvironment* const environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    time_t adjusted = static_cast<time_t>(second);
    adjusted -= static_cast<time_t>(reset_hour) * 3600;
    struct tm local_value;
    memset(&local_value, 0, sizeof(local_value));
    if (!localtime_r(&adjusted, &local_value)) {
        return second / 86400u;
    }
    return static_cast<uint32_t>((local_value.tm_year + 1900) * 400 +
                                 local_value.tm_yday);
}

int FavorLevel(const stNPCCommonData_t& common, uint16_t value) {
    if (value >= common.max_favor_value) {
        return 4;
    }
    for (int level = 1; level <= 3; ++level) {
        if (common.favor_level_values[level] >= value) {
            return level;
        }
    }
    return 4;
}

uint16_t ClampFavor(const stNPCCommonData_t& common, int value) {
    if (value <= 0) {
        return 0;
    }
    if (value >= static_cast<int>(common.max_favor_value)) {
        return common.max_favor_value;
    }
    return static_cast<uint16_t>(value);
}

void MarkDirty(UserNpcRelationshipState& state) {
    state.dirty = true;
    ++state.generation;
    if (state.generation == 0) {
        state.generation = 1;
        state.queued_generation = 0;
    }
}

void ResetDailyRecord(stNPCRelationship_t& relationship,
                      uint32_t now,
                      UserNpcRelationshipState& state) {
    if (relationship.update_time == 0 ||
        ResetDayKey(relationship.update_time) != ResetDayKey(now)) {
        relationship.mood = 0;  // persistent per-user gifts used today
        relationship.update_time = now;
        MarkDirty(state);
    }
}

bool IsCurrentNearbyNpc(CUser* user, uint16_t npc_id) {
    GameWorld* const world = G_GameWorld();
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR() || user->getMoveSpace() == 1 || !world ||
        world->IsPVPChannel()) {
        return false;
    }

    const int village_index =
        static_cast<signed char>(user->getCurCharacVill());
    const int area_index = user->get_area(false);
    if (!world->check_valid_area(village_index, area_index)) {
        return false;
    }
    Village* const village = world->getUserVillage(user);
    Area* const area = village ? village->getArea(area_index) : 0;
    if (!area || area->npc_ids.size() != area->npc_positions.size()) {
        return false;
    }

    const int user_x = user->get_posX();
    const int user_y = user->get_posY();
    for (size_t index = 0; index < area->npc_ids.size(); ++index) {
        if (area->npc_ids[index] != static_cast<int>(npc_id)) {
            continue;
        }
        const int npc_x = area->npc_positions[index].first;
        const int npc_y = area->npc_positions[index].second;
        // These are the server's native NPC exclusion dimensions used by
        // Is_restrictive_commercial_transaction_zone.  Strict inequalities
        // reject boundary/out-of-range forged NPC ids.
        if (npc_x - kNpcInteractionHalfWidth < user_x &&
            npc_x + kNpcInteractionHalfWidth > user_x &&
            npc_y - kNpcInteractionHalfHeight < user_y &&
            npc_y + kNpcInteractionHalfHeight > user_y) {
            return true;
        }
    }
    return false;
}

UserNpcRelationshipState* FindLoadedState(CUser* user) {
    if (!user) {
        return 0;
    }
    UserStateMap& states = GetUserStates();
    UserStateMap::iterator found = states.find(user);
    if (found == states.end() || !found->second.loaded ||
        found->second.charac_no != user->get_charac_no(-1)) {
        return 0;
    }
    return &found->second;
}

stNPCRelationship_t* FindRelationship(UserNpcRelationshipState& state,
                                      uint16_t npc_id) {
    for (std::vector<stNPCRelationship_t>::iterator it =
             state.relationships.begin();
         it != state.relationships.end(); ++it) {
        if (it->npc_id == npc_id) {
            return &*it;
        }
    }
    return 0;
}

stNPCRelationship_t* FindOrPrepareRelationship(
    UserNpcRelationshipState& state,
    const CNPCScript& script,
    const stNPCCommonData_t& common,
    uint16_t npc_id,
    uint32_t now,
    stNPCRelationship_t& pending,
    bool& must_commit_pending) {
    must_commit_pending = false;
    stNPCRelationship_t* relationship = FindRelationship(state, npc_id);
    if (relationship) {
        ResetDailyRecord(*relationship, now, state);
        return relationship;
    }
    if (state.relationships.size() >= kMaximumRelationships) {
        return 0;
    }

    pending.npc_id = npc_id;
    pending.relationship = ClampFavor(
        common, script.script.favorable_relation.default_favor);
    pending.mood = 0;
    pending.reward_state = 0;
    pending.update_time = now;
    must_commit_pending = true;
    return &pending;
}

int CountMaximumFavorNpc(const UserNpcRelationshipState& state,
                         const stNPCCommonData_t& common,
                         uint16_t except_npc_id) {
    int count = 0;
    for (std::vector<stNPCRelationship_t>::const_iterator it =
             state.relationships.begin();
         it != state.relationships.end(); ++it) {
        if (it->npc_id != except_npc_id &&
            FavorLevel(common, it->relationship) == 4) {
            ++count;
        }
    }
    return count;
}

bool MakeStackableReward(int item_id, int count, Inven_Item& item) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const prototype = data_manager ? data_manager->find_item(item_id) : 0;
    if (!prototype || !prototype->vtable_ || prototype->is_stackable() != 1 ||
        item_id <= 0 || count <= 0) {
        return false;
    }
    item.reset();
    item.item_id_ = item_id;
    item.set_add_info(count);
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = item_id;
    item.set_add_info(count);
    return item.item_category_ != 1 && !item.IsCreatureItemType() &&
           item.item_category_ != 8;
}

bool PreflightPossibleRewards(
    const CNPCScript& script,
    int mood,
    int favor_level,
    int favor_change,
    const CInventory& source_deleted_preview,
    uint8_t& error_code) {
    error_code = 0;
    if (mood < 0 || mood > 2 || favor_level < 0 || favor_level > 4 ||
        favor_change < 0 || favor_change > 2) {
        error_code = kErrorInvalidState;
        return false;
    }
    const stFavorableRelationShip_t::stRewardItemInfo_t& reward =
        script.script.favorable_relation.reward_items[favor_level][mood];
    if (reward.total_rates[favor_change] <= 0) {
        return true;
    }
    const std::vector<stFavorableRelationShip_t::stRewardItemElement_t>&
        candidates = reward.reward_items[favor_change];
    if (candidates.empty()) {
        error_code = kErrorInvalidItem;
        return false;
    }

    int previous_rate = 0;
    for (std::vector<stFavorableRelationShip_t::stRewardItemElement_t>::
             const_iterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (it->accumulated_rate <= previous_rate ||
            it->accumulated_rate > reward.total_rates[favor_change]) {
            error_code = kErrorInvalidItem;
            return false;
        }
        previous_rate = it->accumulated_rate;
        if (it->item_index == 0 && it->item_count == 0) {
            continue;
        }
        Inven_Item possible;
        if (!MakeStackableReward(it->item_index, it->item_count, possible)) {
            error_code = kErrorInvalidItem;
            return false;
        }
        InventoryMemory candidate_memory;
        CInventory candidate;
        candidate.SetInventoryMemory(&candidate_memory);
        candidate.setCopy(source_deleted_preview);
        if (candidate.tryInsertItemIntoInventory(possible) < 0) {
            error_code = kErrorInventoryFull;
            return false;
        }
    }
    if (previous_rate != reward.total_rates[favor_change]) {
        error_code = kErrorInvalidItem;
        return false;
    }
    return true;
}

bool SelectReward(const CNPCScript& script,
                  int mood,
                  int favor_level,
                  int favor_change,
                  Inven_Item& reward_item) {
    const stFavorableRelationShip_t::stRewardItemInfo_t& reward =
        script.script.favorable_relation.reward_items[favor_level][mood];
    reward_item.reset();
    if (reward.total_rates[favor_change] <= 0) {
        return true;
    }
    reward_item = script.getGiftRewardItem(
        static_cast<ENUM_NPC_MOOD>(mood),
        static_cast<ENUM_NPC_FAVOR_CHANGE_CONST>(favor_change),
        static_cast<unsigned int>(favor_level));
    if (reward_item.item_id_ == 0 && reward_item.get_add_info() == 0) {
        // The restored RNG may hit its inclusive upper edge.  This is the
        // native no-reward outcome, not permission to reroll the gift.
        return true;
    }
    CItem* const prototype = G_CDataManager()->find_item(reward_item.item_id_);
    return prototype && prototype->is_stackable() == 1 &&
           reward_item.item_category_ != 1 &&
           !reward_item.IsCreatureItemType() &&
           reward_item.item_category_ != 8 &&
           reward_item.get_add_info() > 0;
}

void CommitGiftAudit(CUser* user,
                     CInventory* inventory,
                     const CNPCRelationEventManager* relation_event_manager,
                     uint16_t npc_id,
                     const Inven_Item& source_snapshot,
                     int source_count,
                     int reward_slot,
                     const Inven_Item& reward_item) {
    const int source_after = source_snapshot.item_category_ == 1
                                 ? 0
                                 : source_snapshot.get_add_info() - source_count;
    Inven_Item source_log = source_snapshot;
    if (source_after > 0) {
        source_log.set_add_info(source_after);
    }
    cUserHistoryLog* const history = CUser_history_log(user);
    if (history) {
        history->ItemDel(static_cast<INVEN_TYPE>(kMainInventory),
                         source_after,
                         source_count,
                         source_log,
                         kNpcGiftDeleteReason);
    }

    const int npc_log = relation_event_manager
                            ? relation_event_manager->IsGiftNpcLogTarget(npc_id)
                            : 0;
    const int item_log = relation_event_manager
                             ? relation_event_manager->IsGiftItemLogTarget(
                                   static_cast<unsigned int>(
                                       source_snapshot.item_id_))
                             : 0;
    if (npc_log || item_log) {
        cMyTrace trace("Dispatcher_GiveGiftToNPC::CommitGiftAudit",
                       __LINE__,
                       0);
        trace("[NPC_RELATION_EVENT][GIFT_LOG] npc_selector=%d "
              "item_selector=%d charac=%d npc=%u source_item=%d "
              "source_count=%d reward_item=%d reward_count=%d",
              npc_log,
              item_log,
              user ? user->get_charac_no(-1) : 0,
              static_cast<unsigned int>(npc_id),
              source_snapshot.item_id_,
              source_count,
              reward_item.item_id_,
              reward_item.item_id_ > 0 ? reward_item.get_add_info() : 0);
    }

    if (reward_item.item_id_ == 0 || reward_slot < 0) {
        return;
    }
    const Inven_Item* const final_reward =
        inventory->GetInvenRef(kMainInventory, reward_slot);
    const int resulting_count =
        final_reward && final_reward->item_id_ == reward_item.item_id_
            ? final_reward->get_add_info()
            : reward_item.get_add_info();
    CDataManager* const data_manager = G_CDataManager();
    const int log_type = data_manager
                             ? data_manager->IsDBLogItem(
                                   static_cast<uint32_t>(reward_item.item_id_))
                             : -1;
    if (reward_item.item_category_ == 2 && log_type != -1) {
        (void)user->AddDBLogItem(
            static_cast<uint32_t>(reward_item.item_id_),
            static_cast<uint32_t>(reward_item.get_add_info()),
            static_cast<ENUM_DBLOG_ITEM_TYPE>(log_type));
    }
    if (history) {
        history->ItemAdd(static_cast<INVEN_TYPE>(kMainInventory),
                         resulting_count,
                         reward_item.get_add_info(),
                         reward_item,
                         kNpcGiftAddReason);
    }
}

bool QueueAbsoluteSnapshot(CUser* user,
                           UserNpcRelationshipState& state,
                           bool force) {
    if (!user || !state.loaded || !state.dirty ||
        (!force && state.queued_generation == state.generation)) {
        return true;
    }
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues || state.relationships.size() > kMaximumRelationships) {
        return false;
    }
    Stream* const stream = pool->Acquire("npc_gift_0828.cpp", __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateNpcRelationship;
    *guard << user->GetUID();

    NpcRelationshipSavePayload payload;
    memset(&payload, 0, sizeof(payload));
    payload.charac_no = state.charac_no;
    payload.count = static_cast<int32_t>(state.relationships.size());
    if (!state.relationships.empty()) {
        memcpy(payload.relationships,
               &state.relationships[0],
               state.relationships.size() * sizeof(stNPCRelationship_t));
    }
    guard->put_binary(&payload, sizeof(payload));
    if (queues->put(MsgQueueMgr::DB_Q, guard) != 1) {
        return false;
    }
    // This is only queue acceptance.  Keep dirty=true; the disconnect hook
    // forces the same absolute UPDATE again, making retries idempotent.
    state.queued_generation = state.generation;
    return true;
}

uint8_t ClampErrorCode(int error) {
    return error > 0 && error <= 255 ? static_cast<uint8_t>(error)
                                     : kErrorInvalidState;
}

}  // namespace

MSG_GIVE_GIFT_TO_NPC::MSG_GIVE_GIFT_TO_NPC()
    : MSG_BASE(), npc_id(0), item_space(0), item_slot(0), item_id(0),
      item_count(0) {}

stGiveGiftToNPC::stGiveGiftToNPC()
    : ParamBase(),
      error_code(0),
      special_event(0),
      npc_id(0),
      daily_gift_count(0),
      favor_level(0),
      item_space(0),
      item_slot(0),
      item_id(0),
      remaining_count(0),
      reward_gate(0),
      level_change(0),
      favor_change(0),
      gift_result(0),
      favor_value(0),
      reward_item_id(0),
      reward_item_count(0),
      reaction(0),
      reward_slot(-1) {}

stGiveGiftToNPC::~stGiveGiftToNPC() {}

template class IPacketDispatcher<MSG_GIVE_GIFT_TO_NPC,
                                 stGiveGiftToNPC,
                                 static_cast<ch_state>(3)>;

Dispatcher_GiveGiftToNPC::Dispatcher_GiveGiftToNPC() {}
Dispatcher_GiveGiftToNPC::~Dispatcher_GiveGiftToNPC() {}

int Dispatcher_GiveGiftToNPC::read(PacketBuf& packet,
                                   MSG_BASE& base_message) {
    MSG_GIVE_GIFT_TO_NPC& message =
        static_cast<MSG_GIVE_GIFT_TO_NPC&>(base_message);
    if (packet.get_short(message.npc_id) != 1) {
        return LineFunc(22201, kReadSignature, 0, 0);
    }
    if (packet.get_byte(message.item_space) != 1) {
        return LineFunc(22202, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.item_slot) != 1) {
        return LineFunc(22203, kReadSignature, 0, 0);
    }
    if (packet.get_int(message.item_id) != 1) {
        return LineFunc(22204, kReadSignature, 0, 0);
    }
    if (packet.get_int(message.item_count) != 1) {
        return LineFunc(22205, kReadSignature, 0, 0);
    }
    return 0;
}

int Dispatcher_GiveGiftToNPC::process(CUser* user,
                                      MSG_BASE& base_message,
                                      ParamBase& base_param) {
    const MSG_GIVE_GIFT_TO_NPC& message =
        static_cast<const MSG_GIVE_GIFT_TO_NPC&>(base_message);
    stGiveGiftToNPC& result =
        static_cast<stGiveGiftToNPC&>(base_param);

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        !user->getCurCharacInvenR()) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    const int state_error = user->isEnableGiveGiftToNPC();
    if (state_error != 0) {
        result.error_code = ClampErrorCode(state_error);
        return 0;
    }
    UserNpcRelationshipState* const state = FindLoadedState(user);
    const stNPCCommonData_t* const common = GetNpcCommonCache().get();
    CDataManager* const data_manager = G_CDataManager();
    CNPCRelationEventManager* const relation_event_manager =
        data_manager ? data_manager->get_npc_relation_event_manager() : 0;
    CNPCScript* const script =
        data_manager ? data_manager->find_npc(message.npc_id) : 0;
    CNPCDynamicInfoManager* const dynamic_manager =
        data_manager ? data_manager->get_npc_dynamic_info_manager() : 0;
    CNPCDynamicInfo* const dynamic =
        dynamic_manager ? dynamic_manager->getNPCInfo(message.npc_id) : 0;
    if (!state || !common || !script || !script->isFavorableNPC() ||
        !script->script.favorable_relation.enabled || !dynamic ||
        dynamic->script != script || !IsCurrentNearbyNpc(user, message.npc_id)) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    // The 0828 gift UI supplies ordinary character inventory items.  Other
    // ENUM_ITEMSPACE values include avatar/cargo/equipment owners with
    // different instance and notification contracts and fail closed here.
    if (message.item_space != 0 || message.item_count == 0 ||
        message.item_count > INT_MAX) {
        result.error_code = kErrorItemChanged;
        return 0;
    }
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory ||
        inventory->CheckValidSlot(kMainInventory, message.item_slot) != 1) {
        result.error_code = kErrorItemChanged;
        return 0;
    }
    const Inven_Item* const source =
        inventory->GetInvenRef(kMainInventory, message.item_slot);
    CItem* const source_prototype =
        data_manager->find_item(static_cast<int>(message.item_id));
    if (!source || source->item_id_ != static_cast<int>(message.item_id) ||
        !source_prototype || !source_prototype->vtable_) {
        result.error_code = kErrorItemChanged;
        return 0;
    }
    if (source_prototype->IsImpossibleGift() != 0) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }
    if (relation_event_manager &&
        relation_event_manager->IsDislikeItem(
            message.npc_id, static_cast<unsigned int>(message.item_id))) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }
    if (user->CheckItemLock(kMainInventory, message.item_slot)) {
        result.error_code = kErrorItemLocked;
        return 0;
    }
    const int requested_count = static_cast<int>(message.item_count);
    if ((source->item_category_ == 1 && requested_count != 1) ||
        (source->item_category_ != 1 &&
         (source->get_add_info() <= 0 ||
          requested_count > source->get_add_info()))) {
        result.error_code = kErrorItemChanged;
        return 0;
    }

    const uint32_t now = CurrentSecond();
    stNPCRelationship_t pending_relationship;
    bool must_commit_pending = false;
    stNPCRelationship_t* relationship = FindOrPrepareRelationship(
        *state,
        *script,
        *common,
        message.npc_id,
        now,
        pending_relationship,
        must_commit_pending);
    if (!relationship) {
        result.error_code = kErrorUnavailable;
        return 0;
    }
    const uint16_t maximum_daily =
        script->script.favorable_relation.max_gift_per_day;

    result.npc_id = message.npc_id;
    result.item_space = message.item_space;
    result.item_slot = message.item_slot;
    result.item_id = message.item_id;
    result.remaining_count = source->item_category_ == 1
                                 ? 0u
                                 : static_cast<uint32_t>(
                                       source->get_add_info() - requested_count);
    result.favor_value = relationship->relationship;
    result.favor_level = static_cast<uint8_t>(
        FavorLevel(*common, relationship->relationship));
    result.daily_gift_count = static_cast<uint8_t>(
        std::min<int>(relationship->mood, 0xff));

    // gift_result==2 is an explicit success exception in sub_567600: it
    // chooses animation 4 and deliberately skips client-side source removal.
    if (maximum_daily != 0 && relationship->mood >= maximum_daily) {
        result.gift_result = 2;
        result.remaining_count = source->item_category_ == 1
                                     ? 1u
                                     : static_cast<uint32_t>(
                                           source->get_add_info());
        (void)npc_gift::Flush(user);
        return 0;
    }
    if (relationship->mood == 0xffffu) {
        result.error_code = kErrorUnavailable;
        return 0;
    }

    const int mood = dynamic->getMood();
    if (mood < 0 || mood > 2) {
        result.error_code = kErrorInvalidState;
        return 0;
    }
    const bool key_item =
        dynamic->isKeyItem(message.item_id, message.item_count) == 1;
    const int old_level = FavorLevel(*common, relationship->relationship);
    uint16_t new_favor = relationship->relationship;
    int new_level = old_level;
    int favor_change = 0;
    int level_change = 0;
    ENUM_NPC_FAVOR_GIFT_RESULT_CONST gift_result =
        static_cast<ENUM_NPC_FAVOR_GIFT_RESULT_CONST>(0);

    if (!key_item) {
        const int16_t favor_delta = static_cast<int16_t>(
            dynamic->giveGiftItem(static_cast<unsigned int>(mood),
                                  message.item_id,
                                  message.item_count,
                                  source_prototype,
                                  gift_result));
        new_favor = ClampFavor(
            *common,
            static_cast<int>(relationship->relationship) + favor_delta);
        if (FavorLevel(*common, new_favor) == 4 && old_level != 4 &&
            common->max_favor_level_npc != 0 &&
            CountMaximumFavorNpc(*state, *common, message.npc_id) >=
                common->max_favor_level_npc) {
            new_favor = common->favor_level_values[3];
        }
        new_level = FavorLevel(*common, new_favor);
        favor_change = new_favor > relationship->relationship
                           ? 1
                           : (new_favor < relationship->relationship ? 2 : 0);
        level_change = new_level > old_level
                           ? 1
                           : (new_level < old_level ? 2 : 0);
    }

    // First mutate only an ownerless shadow.  This proves source deletion and
    // every possible random reward insertion before RNG selection or any live
    // inventory/relationship/mood mutation.
    InventoryMemory preview_memory;
    CInventory preview;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenRefR());
    if (preview.delete_item(static_cast<INVEN_TYPE>(kMainInventory),
                            message.item_slot,
                            requested_count,
                            kNpcGiftDeleteReason,
                            false) != 1) {
        result.error_code = kErrorItemChanged;
        return 0;
    }

    uint8_t reward_error = 0;
    if (!key_item && static_cast<int>(gift_result) == 0 &&
        !PreflightPossibleRewards(*script,
                                  mood,
                                  new_level,
                                  favor_change,
                                  preview,
                                  reward_error)) {
        result.error_code = reward_error;
        return 0;
    }
    Inven_Item reward_item;
    reward_item.reset();
    if (!key_item && static_cast<int>(gift_result) == 0 &&
        !SelectReward(*script,
                      mood,
                      new_level,
                      favor_change,
                      reward_item)) {
        result.error_code = kErrorInvalidItem;
        return 0;
    }

    const Inven_Item source_snapshot = *source;
    if (inventory->delete_item(static_cast<INVEN_TYPE>(kMainInventory),
                               message.item_slot,
                               requested_count,
                               kNpcGiftDeleteReason,
                               false) != 1) {
        result.error_code = kErrorItemChanged;
        return 0;
    }
    int reward_slot = -1;
    if (reward_item.item_id_ != 0) {
        reward_slot = inventory->insertItemIntoInventory(
            reward_item, kNpcGiftStagingAddReason, false, true);
        if (reward_slot < 0) {
            if (inventory->update_item(static_cast<INVEN_TYPE>(kMainInventory),
                                       message.item_slot,
                                       source_snapshot) != 1) {
                cMyTrace trace("Dispatcher_GiveGiftToNPC::process rollback",
                               __LINE__,
                               5);
                trace("CMD222 source rollback failed charac(%d) slot(%u) item(%u)",
                      user->get_charac_no(-1),
                      static_cast<unsigned int>(message.item_slot),
                      static_cast<unsigned int>(message.item_id));
            }
            result.error_code = kErrorInventoryFull;
            return 0;
        }
    }

    // The inventory transaction is now committed.  Only now advance the
    // per-character relation and, for an exact key item, the global NPC mood.
    if (must_commit_pending) {
        state->relationships.push_back(pending_relationship);
        relationship = &state->relationships.back();
    }
    ++relationship->mood;
    relationship->update_time = now;
    if (key_item) {
        result.special_event = 1;
        result.reaction = static_cast<uint8_t>(dynamic->changeMood(true));
    } else {
        relationship->relationship = new_favor;
    }
    MarkDirty(*state);

    CommitGiftAudit(user,
                    inventory,
                    relation_event_manager,
                    message.npc_id,
                    source_snapshot,
                    requested_count,
                    reward_slot,
                    reward_item);
    result.reward_slot = reward_slot;
    result.reward_item_id = reward_item.item_id_ > 0
                                ? static_cast<uint32_t>(reward_item.item_id_)
                                : 0u;
    result.reward_item_count = reward_item.item_id_ > 0
                                   ? static_cast<uint32_t>(
                                         reward_item.get_add_info())
                                   : 0u;
    result.daily_gift_count = static_cast<uint8_t>(
        std::min<int>(relationship->mood, 0xff));
    result.favor_value = relationship->relationship;
    result.favor_level = static_cast<uint8_t>(
        FavorLevel(*common, relationship->relationship));
    result.level_change = static_cast<uint8_t>(level_change);
    result.favor_change = static_cast<uint8_t>(favor_change);
    result.gift_result = static_cast<uint8_t>(gift_result);
    result.reward_gate = 0;  // tail is always present; zero/zero means no item
    result.error_code = 0;

    // Queue the absolute snapshot immediately.  Failure leaves it dirty for
    // CUser::_updateNPCRelationship and the forced disconnect retry.
    (void)npc_gift::Flush(user);
    return 0;
}

void Dispatcher_GiveGiftToNPC::send(CUser* user, ParamBase& base_param) {
    if (!user) {
        return;
    }
    const stGiveGiftToNPC& result =
        static_cast<const stGiveGiftToNPC&>(base_param);
    if (result.error_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GIVE_GIFT_TO_NPC,
                                 result.error_code);
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_GIVE_GIFT_TO_NPC);
    packet.put_byte(1);
    packet.put_byte(result.special_event);
    packet.put_short(result.npc_id);
    packet.put_byte(result.daily_gift_count);
    packet.put_byte(result.favor_level);
    packet.put_byte(result.item_space);
    packet.put_short(result.item_slot);
    packet.put_int(static_cast<int>(result.item_id));
    packet.put_int(static_cast<int>(result.remaining_count));
    packet.put_byte(result.reward_gate);
    packet.put_byte(result.level_change);
    packet.put_byte(result.favor_change);
    packet.put_byte(result.gift_result);
    packet.put_short(result.favor_value);
    if (result.reward_gate == 0) {
        packet.put_int(static_cast<int>(result.reward_item_id));
        packet.put_int(static_cast<int>(result.reward_item_count));
    }
    if (result.special_event != 0) {
        packet.put_byte(result.reaction);
    }
    packet.finalize(true);
    user->Send(packet);

    // The client applies source removal while consuming the CMD reply.  Send
    // a merged/new reward slot only afterwards, otherwise a reward reusing
    // the freed source slot would be deleted by sub_567600.
    if (result.reward_slot >= 0) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 result.reward_slot);
    }
    if (result.special_event != 0) {
        CDataManager* const data_manager = G_CDataManager();
        CNPCDynamicInfoManager* const manager =
            data_manager ? data_manager->get_npc_dynamic_info_manager() : 0;
        CNPCDynamicInfo* const dynamic =
            manager ? manager->getNPCInfo(result.npc_id) : 0;
        GameWorld* const world = G_GameWorld();
        if (dynamic && world) {
            PacketGuard notification;
            notification.put_header(0, ENUM_NOTIPACKET_NPC_MOOD);
            notification.put_short(1);
            dynamic->makeNotiPacketCurMood(notification);
            notification.finalize(true);
            world->send_all(notification);
        }
    }
}

namespace npc_gift {

bool Load(CUser* user, const SIG_LOAD_ETC* payload) {
    const stNPCCommonData_t* const common = GetNpcCommonCache().get();
    CDataManager* const data_manager = G_CDataManager();
    if (!user || !payload || !common || !data_manager ||
        payload->charac_no == 0 ||
        payload->charac_no !=
            static_cast<uint32_t>(user->get_charac_no(-1)) ||
        payload->npc_relationship_count > kMaximumRelationships) {
        return false;
    }

    UserNpcRelationshipState loaded;
    loaded.charac_no = static_cast<int32_t>(payload->charac_no);
    loaded.loaded = true;
    // Reserve the DB ABI maximum while still in the login path.  A later
    // first gift can then commit its pending relationship record without an
    // allocation between the inventory commit and the relationship commit.
    loaded.relationships.reserve(kMaximumRelationships);
    for (uint32_t index = 0; index < payload->npc_relationship_count; ++index) {
        stNPCRelationship_t relationship = payload->npc_relationships[index];
        CNPCScript* const script = data_manager->find_npc(relationship.npc_id);
        if (relationship.npc_id == 0 || !script || !script->isFavorableNPC() ||
            !script->script.favorable_relation.enabled ||
            relationship.relationship > common->max_favor_value) {
            return false;
        }
        for (std::vector<stNPCRelationship_t>::const_iterator previous =
                 loaded.relationships.begin();
             previous != loaded.relationships.end(); ++previous) {
            if (previous->npc_id == relationship.npc_id) {
                return false;
            }
        }
        const uint16_t maximum_daily =
            script->script.favorable_relation.max_gift_per_day;
        if (maximum_daily != 0 && relationship.mood > maximum_daily) {
            relationship.mood = maximum_daily;
            MarkDirty(loaded);
        }
        ResetDailyRecord(relationship, CurrentSecond(), loaded);
        loaded.relationships.push_back(relationship);
    }

    UserStateMap& states = GetUserStates();
    UserStateMap::iterator previous = states.find(user);
    if (previous != states.end()) {
        (void)QueueAbsoluteSnapshot(user, previous->second, true);
        states.erase(previous);
    }
    states.insert(std::make_pair(user, loaded));
    return true;
}

bool ExportToCache(CUser* user, SIG_LOAD_ETC* payload) {
    UserNpcRelationshipState* const state = FindLoadedState(user);
    if (!user || !payload || !state || !state->loaded ||
        state->charac_no <= 0 ||
        state->charac_no != user->get_charac_no(-1) ||
        state->relationships.size() >
            static_cast<std::size_t>(kMaximumRelationships)) {
        return false;
    }

    payload->npc_relationship_count =
        static_cast<uint32_t>(state->relationships.size());
    std::memset(payload->npc_relationships,
                0,
                sizeof(payload->npc_relationships));
    if (!state->relationships.empty()) {
        std::memcpy(payload->npc_relationships,
                    &state->relationships[0],
                    state->relationships.size() *
                        sizeof(payload->npc_relationships[0]));
    }
    return true;
}

bool Flush(CUser* user) {
    UserNpcRelationshipState* const state = FindLoadedState(user);
    return state ? QueueAbsoluteSnapshot(user, *state, false) : false;
}

void ResetDaily(CUser* user) {
    UserNpcRelationshipState* const state = FindLoadedState(user);
    if (!state) {
        return;
    }
    const uint32_t now = CurrentSecond();
    for (std::vector<stNPCRelationship_t>::iterator it =
             state->relationships.begin();
         it != state->relationships.end(); ++it) {
        ResetDailyRecord(*it, now, *state);
    }
    (void)QueueAbsoluteSnapshot(user, *state, false);
}

void Erase(CUser* user) {
    UserStateMap& states = GetUserStates();
    UserStateMap::iterator found = states.find(user);
    if (found == states.end()) {
        return;
    }
    (void)QueueAbsoluteSnapshot(user, found->second, true);
    states.erase(found);
}

}  // namespace npc_gift_0828
