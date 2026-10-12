#include "common/manager/user.h"
#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/creature/datamanager/creature_script.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <new>
#include <cstring>
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8336E66
// IDA_NAME: _ZN13user_creature8CStomach14IncrementValueEf
// DEMANGLED: user_creature::CStomach::IncrementValue(float)
long double user_creature::CStomach::IncrementValue(float value) {
    value_ += value;
    if (value_ > 100.0f) {
        value_ = 100.0f;
    }
    return value_;
}

// IDA_EA: 0x8336DEE
// IDA_NAME: _ZN13user_creature8CStomach13IsSaveStomachENS_14CREATURE_STATEE
// DEMANGLED: user_creature::CStomach::IsSaveStomach(user_creature::CREATURE_STATE)
int user_creature::CStomach::IsSaveStomach(CREATURE_STATE state) {
    if (state == CREATURE_STATE_ABI_1) {
        return value_ > 0.0f;
    }
    if (state >= CREATURE_STATE_ABI_1 && state <= CREATURE_STATE_ABI_3) {
        return value_ != 100.0f;
    }
    return 1;
}

// IDA_EA: 0x834032A
// IDA_NAME: _ZN13user_creature17CPacketRespondentC2EP5CUserii
// DEMANGLED: user_creature::CPacketRespondent::CPacketRespondent(CUser *,int,int)
user_creature::CPacketRespondent::CPacketRespondent(CUser* user,
                                                     int packet_class,
                                                     int packet_id)
    : PacketGuard(),
      user_(user),
      packet_class_(packet_class),
      packet_id_(packet_id),
      reserved_zero_(0),
      send_on_destroy_(false) {}

// IDA_EA: 0x834036A
// IDA_NAME: _ZN13user_creature17CPacketRespondentD2Ev
// DEMANGLED: user_creature::CPacketRespondent::~CPacketRespondent(void)
user_creature::CPacketRespondent::~CPacketRespondent() {
    if (send_on_destroy_) {
        if (user_) {
            user_->Send(*this);
        }
    }
}

// IDA_EA: 0x8340760
// IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketGainExpCreatureEiib
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketGainExpCreature(int,int,bool)
int user_creature::CPacketRespondent::MakeNotipacketGainExpCreature(
    int exp,
    int level,
    bool grow_type) {
    clear();
    put_header(0, 102);
    put_byte(level);
    if (grow_type) {
        put_byte(1);
        put_int(static_cast<int>(static_cast<int16_t>(exp >> 16)));
        put_byte(10);
        put_byte(static_cast<unsigned char>(exp));
    } else {
        put_byte(0);
        put_int(exp);
    }
    return finalize(true);
}

// IDA_EA: 0x83404B8
// IDA_NAME: _ZN13user_creature17CPacketRespondent20SendAccordingToPlaceEv
// DEMANGLED: user_creature::CPacketRespondent::SendAccordingToPlace(void)
int user_creature::CPacketRespondent::SendAccordingToPlace() {
    send_on_destroy_ = false;
    if (user_->get_state() == 5) {
        return user_->Send(*this);
    }
    return Broadcast(user_->getCurCharacVill(), user_->get_area(false));
}

// IDA_EA: 0x8340842
// IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketEvoluteCreatureEi
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketEvoluteCreature(int)
int user_creature::CPacketRespondent::MakeNotipacketEvoluteCreature(
    int creature_id) {
    clear();
    put_header(0, 106);
    put_byte(creature_id);
    put_short(user_->get_unique_id());
    return finalize(true);
}

// IDA_EA: 0x833EFB0
// IDA_NAME: _ZN13user_creature13CCreatureItem9SetItemIdEi
// DEMANGLED: user_creature::CCreatureItem::SetItemId(int)
user_creature::CCreatureItem* user_creature::CCreatureItem::SetItemId(
    int item_id) {
    item_id_ = item_id;
    return this;
}

// IDA_EA: 0x833769C
// IDA_NAME: _ZN13user_creature9CCreature13GetCreatureIdEi
// DEMANGLED: user_creature::CCreature::GetCreatureId(int)
int user_creature::CCreature::GetCreatureId(int item_id) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || item->is_stackable() != 0) {
        return -1;
    }
    const int creature_id =
        reinterpret_cast<const CEquipItem*>(item)->get_creature_species();
    creature_id_ = creature_id;
    return creature_id;
}

// IDA_EA: 0x83375AE
// IDA_NAME: _ZN13user_creature9CCreature9SetItemIdEi
// DEMANGLED: user_creature::CCreature::SetItemId(int)
user_creature::CCreature* user_creature::CCreature::SetItemId(int item_id) {
    CCreatureItem::SetItemId(item_id);
    CCreatureScript* const script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(GetCreatureId(
            GetItemId()));
    creature_script_ = script;
    if (!script) {
        return this;
    }
    const unsigned char grow_type = script->IsItemGrowCreature() ? 0u : 1u;
    item_grow_state_ = grow_type;
    if (grow_type != 1u && experience_ == 0) {
        experience_ = 5;
        item_grow_balance_ = 5u;
    }
    return this;
}

// IDA_EA: 0x8337D58
// IDA_NAME: _ZN13user_creature9CCreature19IsAbleSelectEvoluteEP5CUser
// DEMANGLED: user_creature::CCreature::IsAbleSelectEvolute(CUser *)
int user_creature::CCreature::IsAbleSelectEvolute(CUser* user) {
    CCreatureScript* const script = creature_script_;
    if (!script || script->GetTypeSelectEvolutionLevel() == -1 ||
        script->GetTypeSelectEvolutionLevel() > level_) {
        return 0;
    }
    if (!script->IsExistEvolutionQuest()) {
        return 1;
    }
    if (user) {
        PacketGuard packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&packet));
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x8337E48
// IDA_NAME: _ZN13user_creature9CCreature13SelectEvoluteEP5CUser
// DEMANGLED: user_creature::CCreature::SelectEvolute(CUser *)
int user_creature::CCreature::SelectEvolute(CUser* user) {
    CCreatureScript* const script = creature_script_;
    if (!script) {
        return 0;
    }
    const int creature_id = item_grow_balance_ <= 4u
        ? script->GET_BTypeCreature() : script->GET_ATypeCreature();
    if (creature_id == -1) {
        return 0;
    }
    CCreatureScript* const target_script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(creature_id);
    if (!target_script) {
        return 0;
    }
    SetItemId(target_script->GetItemId());
    CInventory* const inventory = reinterpret_cast<CInventory*>(
        user->getCurCharacInvenW());
    Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeCreature);
    if (!equipped) {
        return 0;
    }
    equipped->item_id_ = GetItemId();
    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    const int write_result = sender.EvoluteCreature(
        GetUid(), GetItemId());
    if (write_result == 1) {
        sender.Send();
        CPacketRespondent respondent(user, 0, 0);
        respondent.MakeNotipacketEvoluteCreature(creature_id);
        respondent.SendAccordingToPlace();
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(3),
                                 oldsrv::config::kEquipmentTypeCreature);
    }
    return write_result == 1 ? 1 : 0;
}

// IDA_EA: 0x83380FC
// IDA_NAME: _ZNK13user_creature9CCreature13IsAbleEvoluteEP5CUser
// DEMANGLED: user_creature::CCreature::IsAbleEvolute(CUser *)const
int user_creature::CCreature::IsAbleEvolute(CUser* user) const {
    CCreatureScript* const script = creature_script_;
    if (!script || script->GetEvolutionLevel() == 0 ||
        script->GetEvolutionLevel() > level_) {
        return 0;
    }
    if (!script->IsExistEvolutionQuest()) {
        return 1;
    }
    if (user) {
        PacketGuard packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&packet));
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x8336F6E
// IDA_NAME: _ZN13user_creature9CArtifact13EquipArtifactEPK10Inven_Item
// DEMANGLED: user_creature::CArtifact::EquipArtifact(Inven_Item const*)
int user_creature::CArtifact::EquipArtifact(const Inven_Item* item) {
    const int item_id = item->item_id_;
    CItem* const artifact = G_CDataManager()->find_item(item_id);
    if (!artifact || artifact->is_stackable() != 0) {
        return 0;
    }
    item_pointer_ = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(artifact));
    return 1;
}

// IDA_EA: 0x8336FC8
// IDA_NAME: _ZN13user_creature9CArtifact19GetExperienceAmountEv
// DEMANGLED: user_creature::CArtifact::GetExperienceAmount(void)
long double user_creature::CArtifact::GetExperienceAmount() {
    CItem* const item = reinterpret_cast<CItem*>(item_pointer_);
    if (!item || item->is_stackable() != 0) {
        return 0.0;
    }
    return static_cast<long double>(
        reinterpret_cast<const CEquipItem*>(item)->get_experience_amount());
}

// IDA_EA: 0x8337012
// IDA_NAME: _ZN13user_creature9CArtifact23GetCreatureMinimumLevelEv
// DEMANGLED: user_creature::CArtifact::GetCreatureMinimumLevel(void)
int user_creature::CArtifact::GetCreatureMinimumLevel() {
    CItem* const item = reinterpret_cast<CItem*>(item_pointer_);
    if (!item || item->is_stackable() != 0) {
        return 0;
    }
    return reinterpret_cast<const CEquipItem*>(item)
        ->get_creature_minimum_level();
}

// IDA_EA: 0x833715A
// IDA_NAME: _ZN13user_creature9CCreature8SetStateEP5CUserNS_14CREATURE_STATEE
// DEMANGLED: user_creature::CCreature::SetState(CUser *,user_creature::CREATURE_STATE)
user_creature::CCreature* user_creature::CCreature::SetState(
    CUser* user,
    CREATURE_STATE state) {
    CCreature* const creature = this;
    const float stomach_value = static_cast<float>(
        creature->stomach_.GetStomachValue(
            creature->state_, creature->item_grow_state_ != 0));

    if (creature->state_ != state &&
        state == CREATURE_STATE_ABI_1 &&
        creature->item_grow_state_ != 0) {
        TimerQueue* timer_queue = G_TimerQueue();
        timer_queue->InsertTimer(
            0,
            user->GetUID(),
            92,
            static_cast<int>(stomach_value * 60.0f),
            static_cast<unsigned int>(
                static_cast<unsigned short>(user->get_unique_id())),
            user->getCurCharacNo());
    } else if (creature->state_ != state &&
               state == CREATURE_STATE_ABI_3 &&
               creature->item_grow_state_ != 0 &&
               creature->IsDieCreature()) {
        user_creature::CPacketRespondent respondent(user, 0, 0);
        respondent.MakeNotipacketRevivalCreature();
        respondent.SendAll();
        creature->stomach_.SetStomachValue(1.0f);
        reinterpret_cast<InterfacePacketBuf*>((&respondent))->clear();
        respondent.MakeNotipacketCreatureState(
            creature->GetUid(),
            1);
        respondent.Send();
    }

    creature->state_ = state;
    creature->state_dirty_ = 1;
    return creature;
}

// IDA_EA: 0x83373D4
// IDA_NAME: _ZN13user_creature9CCreature7UseFeedEP5CUserRi
// DEMANGLED: user_creature::CCreature::UseFeed(CUser *,int &)
int user_creature::CCreature::UseFeed(CUser* user, int& exp) {
    CCreature* const creature = this;
    if (creature->item_grow_state_ != 1u) {
        return 0;
    }

    user_creature::CPacketRespondent respondent(user, 0, 0);
    if (creature->IsDieCreature()) {
        respondent.MakeNotipacketRevivalCreature();
        respondent.SendAll();
    }

    exp = static_cast<int>(creature->stomach_.IncrementValue(30.0f));
    respondent.MakeNotipacketCreatureState(
        creature->GetUid(), exp);
    const int result = respondent.Send();
    return result;
}

// IDA_EA: 0x83374D2
// IDA_NAME: _ZN13user_creature9CCreature11GetExpLevelEi
// DEMANGLED: user_creature::CCreature::GetExpLevel(int)
int user_creature::CCreature::GetExpLevel(int exp) {
    CCreatureScript* const script = creature_script_;
    int level = 0;
    if (item_grow_state_ == 1u) {
        level = G_CDataManager()->get_creature_exp_level(exp);
    } else {
        int item_count =
            script->GetGrowItemCountforLevelUp();
        if (item_count == 0) {
            item_count = 25;
        }
        level = exp / item_count + 1;
    }
    if (script && script->GetMaxLevel() < level) {
        return script->GetMaxLevel();
    }
    return level;
}

// IDA_EA: 0x83376F6
// IDA_NAME: _ZN13user_creature9CCreature19IsAvailableArtifactEi
// DEMANGLED: user_creature::CCreature::IsAvailableArtifact(int)
int user_creature::CCreature::IsAvailableArtifact(int artifact_slot) {
    CCreatureScript* const script = creature_script_;
    if (!script) {
        return 0;
    }
    return script->IsAvailableArtifactSlot(artifact_slot);
}

// IDA_EA: 0x8337724
// IDA_NAME: _ZN13user_creature9CCreature9GetAddExpEiP5CUser
// DEMANGLED: user_creature::CCreature::GetAddExp(int,CUser *)
long double user_creature::CCreature::GetAddExp(int artifact_slot,
                                                CUser* user) {
    if (IsAvailableArtifact(artifact_slot) == 0) {
        return 0.0;
    }

    CInventory* const inventory = reinterpret_cast<CInventory*>(
        user->getCurCharacInvenW());
    CArtifact* const artifact =
        inventory->GetCreatureMgrW()->GetArtifact(artifact_slot);
    if (!artifact) {
        return 0.0;
    }
    const int minimum_level = artifact->GetCreatureMinimumLevel();
    if (minimum_level == 0 || level_ >= minimum_level) {
        return artifact->GetExperienceAmount();
    }
    return 0.0;
}

// IDA_EA: 0x83377B6
// IDA_NAME: _ZN13user_creature9CCreature12CalculateExpEiRbP5CUser
// DEMANGLED: user_creature::CCreature::CalculateExp(int,bool &,CUser *)
user_creature::CCreature* user_creature::CCreature::CalculateExp(
    int item_id,
    bool& level_up,
    CUser* user) {
    const long double artifact_bonus =
        GetAddExp(oldsrv::config::kEquipmentTypeArtifactRed, user) +
        GetAddExp(oldsrv::config::kEquipmentTypeArtifactBlue, user) +
        GetAddExp(oldsrv::config::kEquipmentTypeArtifactGreen, user);
    const int bonus_exp = static_cast<int>(
        static_cast<long double>(item_id) * artifact_bonus / 100.0);
    experience_ += item_id + bonus_exp;
    const int exp_level = GetExpLevel(experience_);
    if (level_ < exp_level) {
        level_up = true;
    }
    level_ = exp_level;
    return this;
}

// IDA_EA: 0x833756E
// IDA_NAME: _ZN13user_creature9CCreature10isMaxLevelEv
// DEMANGLED: user_creature::CCreature::isMaxLevel(void)
int user_creature::CCreature::isMaxLevel() {
    CCreatureScript* const script = creature_script_;
    if (script && level_ >= script->GetMaxLevel()) {
        return 1;
    }
    return 0;
}

// IDA_EA: 0x83378AA
// IDA_NAME: _ZN13user_creature9CCreature12vaildItemExpEi
// DEMANGLED: user_creature::CCreature::vaildItemExp(int)
int user_creature::CCreature::vaildItemExp(int item_id) {
    if (state_ == CREATURE_STATE_ABI_2 || item_grow_state_ != 0u) {
        return 0;
    }
    CCreatureScript* const script = creature_script_;
    return script->validItemExp(item_id) != 0 ? 1 : 0;
}

// IDA_EA: 0x83378FC
// IDA_NAME: _ZN13user_creature9CCreature7GainExpEiP5CUser
// DEMANGLED: user_creature::CCreature::GainExp(int,CUser *)
int user_creature::CCreature::GainExp(int item_id, CUser* user) {
    CCreatureScript* const script = creature_script_;
    if (!script) {
        return 0;
    }
    const int evolution_level =
        script->GetTypeSelectEvolutionLevel();
    if (evolution_level != -1 &&
        level_ >= evolution_level) {
        return 0;
    }
    if (level_ >= script->GetMaxLevel()) {
        return 0;
    }
    if (item_grow_state_ == 1u) {
        return GainExpCreature(item_id, user);
    }
    return GainItemExpCreature(item_id, user);
}

// IDA_EA: 0x83379C0
// IDA_NAME: _ZN13user_creature9CCreature19GainItemExpCreatureEiP5CUser
// DEMANGLED: user_creature::CCreature::GainItemExpCreature(int,CUser *)
int user_creature::CCreature::GainItemExpCreature(int item_id, CUser* user) {
    if (item_grow_state_ != 0u) {
        return 0;
    }

    CCreatureScript* const script = creature_script_;
    if (script->validItem_AType(item_id) != 0) {
        unsigned char value = item_grow_balance_;
        ++value;
        if (value > 10u) {
            value = 10u;
        }
        item_grow_balance_ = value;
    }
    if (script->validItem_BType(item_id) != 0) {
        unsigned char value = item_grow_balance_;
        if (value > 0u) {
            --value;
        }
        item_grow_balance_ = value;
    }

    int packed_exp = experience_;
    packed_exp = (packed_exp & ~0xff) | item_grow_balance_;
    packed_exp += 0x00010000;
    experience_ = packed_exp;
    const int exp_level = GetExpLevel(
        static_cast<int>(static_cast<int16_t>(packed_exp >> 16)));
    bool level_up = false;
    if (level_ < exp_level) {
        level_up = true;
    }
    level_ = exp_level;

    if (level_ < script->GetMaxLevel() ||
        level_up) {
        user_creature::CPacketRespondent respondent(user, 0, 0);
        respondent.MakeNotipacketGainExpCreature(
            experience_,
            level_,
            IsGrowCreature() != 0);
    }
    if (level_up && IsAbleSelectEvolute(user) != 0 &&
        SelectEvolute(user) != 1) {
        return 0;
    }
    state_dirty_ = 1;
    return 1;
}

// IDA_EA: 0x8337BBC
// IDA_NAME: _ZN13user_creature9CCreature15GainExpCreatureEiP5CUser
// DEMANGLED: user_creature::CCreature::GainExpCreature(int,CUser *)
int user_creature::CCreature::GainExpCreature(int item_id, CUser* user) {
    if (item_grow_state_ != 1u) {
        return 0;
    }

    const float stomach_value = static_cast<float>(
        stomach_.GetStomachValue(state_, item_grow_state_ != 0u));
    if (static_cast<int>(stomach_value) <= 0) {
        return 0;
    }

    bool level_up = false;
    CalculateExp(item_id, level_up, user);
    CCreatureScript* const script = creature_script_;
    if (level_ < script->GetMaxLevel() ||
        level_up) {
        user_creature::CPacketRespondent respondent(user, 0, 0);
        respondent.MakeNotipacketGainExpCreature(
            experience_,
            level_,
            IsGrowCreature() != 0);
    }
    if (level_up && IsAbleEvolute(user) != 0 && Evolute(user) != 1) {
        return 0;
    }
    state_dirty_ = 1;
    return 1;
}

// IDA_EA: 0x833F0CA
// IDA_NAME: _ZN13user_creature9CCreature14IsGrowCreatureEv
// DEMANGLED: user_creature::CCreature::IsGrowCreature(void)
int user_creature::CCreature::IsGrowCreature() {
    return static_cast<int>(item_grow_state_ ^ 1u);
}
