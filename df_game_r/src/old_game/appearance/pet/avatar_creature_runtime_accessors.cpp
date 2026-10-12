#include "common/manager/global_data.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "common/manager/system_time.h"

#include <map>

namespace {

const float kCreatureStomachMaxValue = 100.0f;

typedef std::map<int, WongWork::Avatar_Item*> AvatarItemMap;

}  // namespace

// IDA_EA: 0x822D20D
// IDA_NAME: _ZN8WongWork14CAvatarItemMgr9IsTempKeyEi
// DEMANGLED: WongWork::CAvatarItemMgr::IsTempKey(int)
unsigned int WongWork::CAvatarItemMgr::IsTempKey(int key) {
    return static_cast<unsigned int>(key) >> 31;
}

// IDA_EA: 0x82FA1EA
// IDA_NAME: _ZNK8WongWork11Avatar_Item13GetExpireDateEv
// DEMANGLED: WongWork::Avatar_Item::GetExpireDate(void)const
int WongWork::Avatar_Item::GetExpireDate() const {
    return expire_date_;
}

// IDA_EA: 0x82FA1F4
// IDA_NAME: _ZNK8WongWork11Avatar_Item13GetRemainDateEi
// DEMANGLED: WongWork::Avatar_Item::GetRemainDate(int)const
int WongWork::Avatar_Item::GetRemainDate(int tick) const {
    if (expire_date_ <= 0) {
        return expire_date_;
    }
    if (expire_date_ == tick) {
        return 1;
    }
    return expire_date_ - tick;
}

// IDA_EA: 0x82FA2EE
// IDA_NAME: _ZNK8WongWork11Avatar_Item18getJewelSocketDataEv
// DEMANGLED: WongWork::Avatar_Item::getJewelSocketData(void)const
const stAvatarEmblemInfo_t*
WongWork::Avatar_Item::getJewelSocketData() const {
    return &jewel_socket_data_;
}

// IDA_EA: 0x82FA336
// IDA_NAME: _ZNK8WongWork11Avatar_Item16getExpansionInfoEv
// DEMANGLED: WongWork::Avatar_Item::getExpansionInfo(void)const
const stAvatarExpansionInfo_t*
WongWork::Avatar_Item::getExpansionInfo() const {
    return &expansion_info_;
}

// IDA_EA: 0x82F9D34
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr9_FindItemEi
// DEMANGLED: WongWork::CAvatarItemMgr::_FindItem(int)const
WongWork::Avatar_Item*
WongWork::CAvatarItemMgr::_FindItem(int key) const {
    AvatarItemMap::const_iterator it = assigned_items_.find(key);
    if (it == assigned_items_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x82F9D98
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr19_FindWaitAssignItemEi
// DEMANGLED: WongWork::CAvatarItemMgr::_FindWaitAssignItem(int)const
WongWork::Avatar_Item*
WongWork::CAvatarItemMgr::_FindWaitAssignItem(int key) const {
    AvatarItemMap::const_iterator it = wait_assign_items_.find(key);
    if (it == wait_assign_items_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x82F9CAC
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr12_FindItemAllEi
// DEMANGLED: WongWork::CAvatarItemMgr::_FindItemAll(int)const
WongWork::Avatar_Item*
WongWork::CAvatarItemMgr::_FindItemAll(int key) const {
    if (IsTempKey(key)) {
        return _FindWaitAssignItem(key);
    }
    return _FindItem(key);
}

// IDA_EA: 0x82F981A
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetExpireDateEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetExpireDate(int)const
int WongWork::CAvatarItemMgr::GetExpireDate(int key) const {
    const WongWork::Avatar_Item* const item = _FindItemAll(key);
    return item ? item->GetExpireDate() : -1;
}

// IDA_EA: 0x82F9850
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetRemainDateEii
// DEMANGLED: WongWork::CAvatarItemMgr::GetRemainDate(int,int)const
int WongWork::CAvatarItemMgr::GetRemainDate(int key, int tick) const {
    const WongWork::Avatar_Item* const item = _FindItemAll(key);
    if (item) {
        return item->GetRemainDate(tick);
    }
    return -1;
}

// IDA_EA: 0x82F98F8
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr18getJewelSocketDataEi
// DEMANGLED: WongWork::CAvatarItemMgr::getJewelSocketData(int)const
const stAvatarEmblemInfo_t*
WongWork::CAvatarItemMgr::getJewelSocketData(int key) const {
    const WongWork::Avatar_Item* const item = _FindItemAll(key);
    if (item) {
        return item->getJewelSocketData();
    }
    return &default_emblem_;
}

// IDA_EA: 0x82F992E
// IDA_NAME: _ZNK8WongWork14CAvatarItemMgr16GetExpansionInfoEi
// DEMANGLED: WongWork::CAvatarItemMgr::GetExpansionInfo(int)const
const stAvatarExpansionInfo_t*
WongWork::CAvatarItemMgr::GetExpansionInfo(int key) const {
    const WongWork::Avatar_Item* const item = _FindItemAll(key);
    if (item) {
        return item->getExpansionInfo();
    }
    return 0;
}

// IDA_EA: 0x8339292
// IDA_NAME: _ZNK13user_creature12CCreatureMgr16FindCreatureItemEi
// DEMANGLED: user_creature::CCreatureMgr::FindCreatureItem(int)const
const user_creature::CCreatureItem*
user_creature::CCreatureMgr::FindCreatureItem(int key) const {
    CCreatureItemMap::const_iterator it = creature_items_.find(key);
    if (it == creature_items_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x833F00A
// IDA_NAME: _ZNK13user_creature13CCreatureItem13getExpireTimeEv
// DEMANGLED: user_creature::CCreatureItem::getExpireTime(void)const
int user_creature::CCreatureItem::getExpireTime() const {
    return expire_time_;
}

// IDA_EA: 0x833F016
// IDA_NAME: _ZNK13user_creature13CCreatureItem13GetRemainDateEi
// DEMANGLED: user_creature::CCreatureItem::GetRemainDate(int)const
int user_creature::CCreatureItem::GetRemainDate(int tick) const {
    const int expire_time = getExpireTime();
    if (expire_time <= 0) {
        return expire_time;
    }
    if (expire_time == tick) {
        return 1;
    }
    return expire_time - tick;
}

// IDA_EA: 0x833D758
// IDA_NAME: _ZNK13user_creature12CCreatureMgr13getExpireDateEi
// DEMANGLED: user_creature::CCreatureMgr::getExpireDate(int)const
int user_creature::CCreatureMgr::getExpireDate(int key) const {
    const user_creature::CCreatureItem* item =
        FindCreatureItem(key);
    if (item) {
        return item->getExpireTime();
    }
    return -1;
}

// IDA_EA: 0x833D78E
// IDA_NAME: _ZNK13user_creature12CCreatureMgr13GetRemainDateEii
// DEMANGLED: user_creature::CCreatureMgr::GetRemainDate(int,int)const
int user_creature::CCreatureMgr::GetRemainDate(int key, int tick) const {
    if (temporary_expire_time_ != -1) {
        return temporary_expire_time_;
    }

    const user_creature::CCreatureItem* item =
        FindCreatureItem(key);
    if (item) {
        return item->GetRemainDate(tick);
    }
    return -1;
}

// IDA_EA: 0x833F064
// IDA_NAME: _ZN13user_creature9CCreature8GetStateEv
// DEMANGLED: user_creature::CCreature::GetState(void)
int user_creature::CCreature::GetState() {
    return static_cast<int>(state_);
}

// IDA_EA: 0x8336CD8
// IDA_NAME: _ZN13user_creature8CStomachC2Ev
// DEMANGLED: user_creature::CStomach::CStomach(void)
user_creature::CStomach::CStomach()
    : last_tick_(GlobalData::system_time()->getCurSec()),
      value_(100.0f) {}

// IDA_EA: 0x8336CFC
// IDA_NAME: _ZN13user_creature8CStomach15GetStomachValueENS_14CREATURE_STATEEb
// DEMANGLED: user_creature::CStomach::GetStomachValue(user_creature::CREATURE_STATE,bool)
long double user_creature::CStomach::GetStomachValue(
    CREATURE_STATE state,
    bool grow_type) {
    if (grow_type == 1) {
        const int cur_sec = GlobalData::system_time()->getCurSec();
        const float elapsed = static_cast<float>(cur_sec - last_tick_);
        if (state == 1) {
            value_ += elapsed / -60.0f;
            if (value_ < 0.0f) {
                value_ = 0.0f;
            }
        } else if (state >= 1 && state <= 3) {
            value_ += elapsed / 360.0f;
            if (value_ > kCreatureStomachMaxValue) {
                value_ = kCreatureStomachMaxValue;
            }
        }
        last_tick_ = cur_sec;
        return value_;
    }
    return kCreatureStomachMaxValue;
}

// IDA_EA: 0x8336E4C
// IDA_NAME: _ZN13user_creature8CStomach6ResumeEv
// DEMANGLED: user_creature::CStomach::Resume(void)
int user_creature::CStomach::Resume() {
    last_tick_ = GlobalData::system_time()->getCurSec();
    return last_tick_;
}

// IDA_EA: 0x833EEF6
// IDA_NAME: _ZN13user_creature8CStomach15SetStomachValueEf
// DEMANGLED: user_creature::CStomach::SetStomachValue(float)
user_creature::CStomach*
user_creature::CStomach::SetStomachValue(float value) {
    value_ = value;
    return this;
}

// IDA_EA: 0x833EFBE
// IDA_NAME: _ZN13user_creature13CCreatureItem6GetUidEv
// DEMANGLED: user_creature::CCreatureItem::GetUid(void)
int user_creature::CCreatureItem::GetUid() {
    return uid_;
}

// IDA_EA: 0x83403B8
// IDA_NAME: _ZN13user_creature17CPacketRespondent8finalizeEb
// DEMANGLED: user_creature::CPacketRespondent::finalize(bool)
int user_creature::CPacketRespondent::finalize(bool encrypt) {
    send_on_destroy_ = true;
    return PacketGuard::finalize(encrypt);
}

// IDA_EA: 0x83403E0
// IDA_NAME: _ZN13user_creature17CPacketRespondent4SendEv
// DEMANGLED: user_creature::CPacketRespondent::Send(void)
int user_creature::CPacketRespondent::Send() {
    send_on_destroy_ = false;
    return user_->Send(*this);
}

// IDA_EA: 0x834048A
// IDA_NAME: _ZN13user_creature17CPacketRespondent7SendAllEv
// DEMANGLED: user_creature::CPacketRespondent::SendAll(void)
int user_creature::CPacketRespondent::SendAll() {
    send_on_destroy_ = false;
    G_GameWorld()->send_all(*this);
    return 1;
}

// IDA_EA: 0x83408B6
// IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketRevivalCreatureEv
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketRevivalCreature(void)
int user_creature::CPacketRespondent::MakeNotipacketRevivalCreature() {
    clear();
    put_header(0, 107);
    put_short(user_->get_unique_id());
    return finalize(true);
}

// IDA_EA: 0x8340918
// IDA_NAME: _ZN13user_creature17CPacketRespondent27MakeNotipacketCreatureStateEii
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketCreatureState(int,int)
int user_creature::CPacketRespondent::MakeNotipacketCreatureState(
    int creature_uid,
    int state) {
    clear();
    put_header(0, 103);
    put_int(creature_uid);
    put_int(state);
    return finalize(true);
}

// IDA_EA: 0x8338478
// IDA_NAME: _ZN13user_creature9CCreature13IsDieCreatureEv
// DEMANGLED: user_creature::CCreature::IsDieCreature(void)
bool user_creature::CCreature::IsDieCreature() {
    const long double stomach =
        stomach_.GetStomachValue(state_, item_grow_state_ != 0);
    return stomach < 1.0L;
}

// IDA_EA: 0x833F058
// IDA_NAME: _ZN13user_creature9CCreature7GetNameEv
// DEMANGLED: user_creature::CCreature::GetName(void)
char* user_creature::CCreature::GetName() {
    return name_;
}

// IDA_EA: 0x833F09A
// IDA_NAME: _ZN13user_creature9CCreature8GetLevelEv
// DEMANGLED: user_creature::CCreature::GetLevel(void)
int user_creature::CCreature::GetLevel() {
    return level_;
}

// IDA_EA: 0x833A638
// IDA_NAME: _ZNK13user_creature12CCreatureMgr17GetCreatureItemIdEv
// DEMANGLED: user_creature::CCreatureMgr::GetCreatureItemId(void)const
int user_creature::CCreatureMgr::GetCreatureItemId() const {
    CCreatureItem* const creature = active_creature_;
    return creature ? creature->GetItemId() : 0;
}

// IDA_EA: 0x833A660
// IDA_NAME: _ZNK13user_creature12CCreatureMgr13IsDieCreatureEv
// DEMANGLED: user_creature::CCreatureMgr::IsDieCreature(void)const
int user_creature::CCreatureMgr::IsDieCreature() const {
    CCreature* const creature = active_creature_;
    return creature ? creature->IsDieCreature() : 1;
}

// IDA_EA: 0x833A688
// IDA_NAME: _ZNK13user_creature12CCreatureMgr15GetCreatureNameEv
// DEMANGLED: user_creature::CCreatureMgr::GetCreatureName(void)const
const char* user_creature::CCreatureMgr::GetCreatureName() const {
    CCreature* const creature = active_creature_;
    return creature ? creature->GetName() : "";
}

// IDA_EA: 0x833A6B0
// IDA_NAME: _ZNK13user_creature12CCreatureMgr23GetEquipedCreatureLevelEv
// DEMANGLED: user_creature::CCreatureMgr::GetEquipedCreatureLevel(void)const
int user_creature::CCreatureMgr::GetEquipedCreatureLevel() const {
    CCreature* const creature = active_creature_;
    return creature ? creature->GetLevel() : 0;
}

// IDA_EA: 0x833C28A
// IDA_NAME: _ZN13user_creature12CCreatureMgr11TurnStomachEb
// DEMANGLED: user_creature::CCreatureMgr::TurnStomach(bool)
char user_creature::CCreatureMgr::TurnStomach(bool fighting) {
    user_creature::CCreature* creature = active_creature_;
    int result = reinterpret_cast<int>(creature);
    if (creature) {
        if (fighting) {
            result = creature->GetState() == 3;
            if (static_cast<unsigned char>(result)) {
                result = reinterpret_cast<int>(
                    creature->SetState(
                        owner_,
                        user_creature::CREATURE_STATE_ABI_1));
            }
        } else {
            result = creature->GetState() == 1;
            if (static_cast<unsigned char>(result)) {
                result = reinterpret_cast<int>(
                    creature->SetState(
                        owner_,
                        user_creature::CREATURE_STATE_ABI_3));
            }
        }
    }
    return static_cast<char>(result);
}
