#include "old_game/progression/skill/item_making_skill.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/item_making_skill_script.h"
#include "old_data/db/skill/item_making_skill_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/mt_rand.h"

#include <cstring>
#include <ctime>


// IDA_EA: 0x8545108
// IDA_NAME: _ZN16CItemMakingSkillC2Ev
// DEMANGLED: CItemMakingSkill::CItemMakingSkill(void)
CItemMakingSkill::CItemMakingSkill() {
    const unsigned long seed_value =
        static_cast<unsigned long>(std::time(0));
    random_ = new CMTRand(seed_value);
}

// IDA_EA: 0x8545192
// IDA_NAME: _ZN16CItemMakingSkillD2Ev
// DEMANGLED: CItemMakingSkill::~CItemMakingSkill()
// IDA_EA: 0x85451E4
// IDA_NAME: _ZN16CItemMakingSkillD0Ev
// DEMANGLED: CItemMakingSkill::~CItemMakingSkill()
CItemMakingSkill::~CItemMakingSkill() {
    if (random_) {
        delete random_;
    }
    random_ = 0;
}

// IDA_EA: 0x8545202
// IDA_NAME: _ZN16CItemMakingSkill8loadDataEP5CUserPc
// DEMANGLED: CItemMakingSkill::loadData(CUser *,char *)
bool CItemMakingSkill::loadData(CUser* user, char* data) {
    (void)user;
    _reset();
    const SIG_ITEM_MAKING_SKILL* payload =
        reinterpret_cast<const SIG_ITEM_MAKING_SKILL*>(data);
    const int16_t loaded[11] = {
        payload->weapon,
        payload->cloth,
        payload->leather,
        payload->light_armor,
        payload->heavy_armor,
        payload->plate,
        payload->amulet,
        payload->wrist,
        payload->ring,
        payload->support,
        payload->magic_stone,
    };
    for (int index = 0; index <= 10; ++index) {
        item_making_skill_[index] =
            loaded[index] <= 999 ? loaded[index] : 1000;
    }
    return 1;
}

// IDA_EA: 0x8545276
// IDA_NAME: _ZN16CItemMakingSkill9_saveDataEP5CUser
// DEMANGLED: CItemMakingSkill::_saveData(CUser *)
int CItemMakingSkill::_saveData(CUser* user) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("ItemMakingSkill.cpp", 51);
    CStreamGuard guard(stream, true);
    guard << 622;
    guard << user->GetUID();
    SIG_ITEM_MAKING_SKILL* payload =
        guard.GetInBuffer<SIG_ITEM_MAKING_SKILL>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    payload->weapon = item_making_skill_[0];
    payload->cloth = item_making_skill_[1];
    payload->leather = item_making_skill_[2];
    payload->light_armor = item_making_skill_[3];
    payload->heavy_armor = item_making_skill_[4];
    payload->plate = item_making_skill_[5];
    payload->amulet = item_making_skill_[6];
    payload->wrist = item_making_skill_[7];
    payload->ring = item_making_skill_[8];
    payload->support = item_making_skill_[9];
    payload->magic_stone = item_making_skill_[10];
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x85453BA
// IDA_NAME: _ZN16CItemMakingSkill6_resetEv
// DEMANGLED: CItemMakingSkill::_reset(void)
int CItemMakingSkill::_reset() {
    return reinterpret_cast<int>(
        std::memset(item_making_skill_, 0, sizeof(item_making_skill_)));
}

// IDA_EA: 0x85453E0
// IDA_NAME: _ZN16CItemMakingSkill24GainItemMakingSkillPointEPK14CStackableItemP5CUser
// DEMANGLED: CItemMakingSkill::GainItemMakingSkillPoint(CStackableItem const*,CUser *)
void CItemMakingSkill::GainItemMakingSkillPoint(
    const CStackableItem* item,
    CUser* user) {
    const ENUM_ITEM_MAKING_TYPE type =
        static_cast<ENUM_ITEM_MAKING_TYPE>(item->GetUpSkillType());
    const int type_index = static_cast<int>(type);
    const int need_level = item->GetNeedLevel();
    stItemMakingSkill* script = G_CDataManager()->GetItemMakingSkill();
    const int skill_level = GetItemMakingSkillLevel(item, user);
    int gain = static_cast<int>(random_->randInt() %
                                script->itemmaking_exp_max) +
               script->itemmaking_exp_min;
    int reason = 0;

    if (skill_level - need_level >= script->itemmaking_level_limit) {
        reason = 13;
        gain = 0;
    }

    if (script->exp_limits.empty()) {
        return;
    }

    if (user->get_charac_level() ==
        oldsrv::config::kCharacterLevelMaximum) {
        if (item_making_skill_[type_index] >=
            script->exp_limits.back().first) {
            reason = 22;
            gain = 0;
        }
    } else {
        for (unsigned int index = 0;
             index < script->exp_limits.size();
             ++index) {
            const std::pair<int, int>& limit = script->exp_limits[index];
            if (user->get_charac_level() < limit.second &&
                item_making_skill_[type_index] >= limit.first) {
                reason = 22;
                gain = 0;
                break;
            }
        }
    }

    if (item_making_skill_[type_index] > 999) {
        reason = 95;
        gain = 0;
    }

    const int16_t before = item_making_skill_[type_index];
    UpSkillLevel(type, gain, user);
    gain = item_making_skill_[type_index] - before;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GAIN_ITEM_MAKING_SKILL_POINT);
    packet.put_byte(reason);
    packet.put_byte(type_index);
    packet.put_short(gain);
    packet.finalize(true);
    user->Send(packet);
    CUser_history_log(user)->ItemMakingSkillAdd(
        type, reason, gain, item_making_skill_[type_index]);
}

// IDA_EA: 0x854571C
// IDA_NAME: _ZN16CItemMakingSkill12UpSkillLevelE21ENUM_ITEM_MAKING_TYPEiP5CUser
// DEMANGLED: CItemMakingSkill::UpSkillLevel(ENUM_ITEM_MAKING_TYPE,int,CUser *)
void CItemMakingSkill::UpSkillLevel(ENUM_ITEM_MAKING_TYPE type,
                                    int point,
                                    CUser* user) {
    stItemMakingSkill* script = G_CDataManager()->GetItemMakingSkill();
    const int type_index = static_cast<int>(type);
    if (type_index > 10 || script->exp_limits.empty()) {
        return;
    }
    if (point <= 0 || script->itemmaking_exp_max < point ||
        script->exp_limits.back().first < point) {
        return;
    }

    item_making_skill_[type_index] = static_cast<int16_t>(
        item_making_skill_[type_index] + point);
    if (user->get_charac_level() ==
        oldsrv::config::kCharacterLevelMaximum) {
        if (item_making_skill_[type_index] >
            script->exp_limits.back().first) {
            cMyTrace trace(__PRETTY_FUNCTION__, 181, 0);
            trace("CHARACTER_LEVEL_CURRENT_MAX :item_making_skill_[%d] = %d",
                  type_index,
                  item_making_skill_[type_index]);
            item_making_skill_[type_index] = static_cast<int16_t>(
                script->exp_limits.back().first);
        }
    } else {
        for (unsigned int index = 0;
             index < script->exp_limits.size();
             ++index) {
            const std::pair<int, int>& limit = script->exp_limits[index];
            if (user->get_charac_level() < limit.second &&
                item_making_skill_[type_index] > limit.first) {
                cMyTrace trace(__PRETTY_FUNCTION__, 197, 0);
                trace("NO CHARACTER_LEVEL_CURRENT_MAX : item_making_skill_[%d] = %d",
                      type_index,
                      item_making_skill_[type_index]);
                item_making_skill_[type_index] =
                    static_cast<int16_t>(limit.first);
            }
        }
    }

    if (item_making_skill_[type_index] > 1000) {
        cMyTrace trace(__PRETTY_FUNCTION__, 206, 0);
        trace("ITEM_MAKING_SKILL_LEVEL_LIMIT : item_making_skill_[%d] = %d",
              type_index,
              item_making_skill_[type_index]);
        item_making_skill_[type_index] = 1000;
    }
    alter();
}

// IDA_EA: 0x8545A62
// IDA_NAME: _ZNK16CItemMakingSkill24Send_ItemMakingSkillListEP5CUser
// DEMANGLED: CItemMakingSkill::Send_ItemMakingSkillList(CUser *)const
int CItemMakingSkill::Send_ItemMakingSkillList(CUser* user) const {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_TOTAL_ITEM_MAKING_SKILL_POINT);
    packet.put_byte(11);
    for (int index = 0; index <= 10; ++index) {
        packet.put_byte(index);
        packet.put_short(item_making_skill_[index]);
    }
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x8545B42
// IDA_NAME: _ZNK16CItemMakingSkill7getDataEPc
// DEMANGLED: CItemMakingSkill::getData(char *)const
void CItemMakingSkill::getData(char* data) const {
    SIG_ITEM_MAKING_SKILL* payload =
        reinterpret_cast<SIG_ITEM_MAKING_SKILL*>(data);
    payload->weapon = item_making_skill_[0];
    payload->cloth = item_making_skill_[1];
    payload->leather = item_making_skill_[2];
    payload->light_armor = item_making_skill_[3];
    payload->heavy_armor = item_making_skill_[4];
    payload->plate = item_making_skill_[5];
    payload->amulet = item_making_skill_[6];
    payload->wrist = item_making_skill_[7];
    payload->ring = item_making_skill_[8];
    payload->support = item_making_skill_[9];
    payload->magic_stone = item_making_skill_[10];
}

// IDA_EA: 0x8545B7E
// IDA_NAME: _ZN16CItemMakingSkill13GetPrevExpGapEi
// DEMANGLED: CItemMakingSkill::GetPrevExpGap(int)
int CItemMakingSkill::GetPrevExpGap(int level) {
    stItemMakingSkill* script = G_CDataManager()->GetItemMakingSkill();
    if (script->exp_limits.size() <=
            static_cast<unsigned int>(level - 1) ||
        level == 1) {
        return 0;
    }
    return script->exp_limits[level - 1].first -
           script->exp_limits[level - 2].first;
}

// IDA_EA: 0x8545C0C
// IDA_NAME: _ZNK16CItemMakingSkill13GetSkillLevelE21ENUM_ITEM_MAKING_TYPEP5CUser
// DEMANGLED: CItemMakingSkill::GetSkillLevel(ENUM_ITEM_MAKING_TYPE,CUser *)const
int CItemMakingSkill::GetSkillLevel(ENUM_ITEM_MAKING_TYPE type,
                                    CUser* user) const {
    stItemMakingSkill* script = G_CDataManager()->GetItemMakingSkill();
    const int type_index = static_cast<int>(type);
    int point_level = 0;
    for (; static_cast<unsigned int>(point_level) <
           script->exp_limits.size();
         ++point_level) {
        if (item_making_skill_[type_index] <
            script->exp_limits[point_level].first) {
            break;
        }
    }

    int character_level = 0;
    for (; static_cast<unsigned int>(character_level) <
           script->exp_limits.size();
         ++character_level) {
        if (user->get_charac_level() <
            script->exp_limits[character_level].second) {
            break;
        }
    }
    return point_level > character_level ? character_level : point_level;
}

// IDA_EA: 0x8545CF4
// IDA_NAME: _ZN16CItemMakingSkill23GetItemMakingSkillLevelEPK14CStackableItemP5CUser
// DEMANGLED: CItemMakingSkill::GetItemMakingSkillLevel(CStackableItem const*,CUser *)
int CItemMakingSkill::GetItemMakingSkillLevel(const CStackableItem* item,
                                              CUser* user) {
    return GetSkillLevel(
        static_cast<ENUM_ITEM_MAKING_TYPE>(item->GetUpSkillType()), user);
}

// IDA_EA: 0x8545D24
// IDA_NAME: _ZN16CItemMakingSkill23SetItemMakingSkillPointEiiP5CUser
// DEMANGLED: CItemMakingSkill::SetItemMakingSkillPoint(int,int,CUser *)
int CItemMakingSkill::SetItemMakingSkillPoint(int type,
                                              int point,
                                              CUser* user) {
    stItemMakingSkill* script = G_CDataManager()->GetItemMakingSkill();
    if (type > 10) {
        return static_cast<int>(reinterpret_cast<intptr_t>(script));
    }
    if (script->exp_limits.empty()) {
        return 1;
    }
    if (point <= 0 || point > 1000 ||
        script->exp_limits.back().first < point) {
        return 1;
    }

    item_making_skill_[type] = static_cast<int16_t>(point);
    if (user->get_charac_level() ==
        oldsrv::config::kCharacterLevelMaximum) {
        if (item_making_skill_[type] > script->exp_limits.back().first) {
            item_making_skill_[type] =
                static_cast<int16_t>(script->exp_limits.back().first);
        }
    } else {
        for (unsigned int index = 0;
             index < script->exp_limits.size();
             ++index) {
            const std::pair<int, int>& limit = script->exp_limits[index];
            if (user->get_charac_level() < limit.second &&
                item_making_skill_[type] > limit.first) {
                item_making_skill_[type] = static_cast<int16_t>(limit.first);
                break;
            }
        }
    }
    if (item_making_skill_[type] > 1000) {
        item_making_skill_[type] = 1000;
    }
    alter();
    return Send_ItemMakingSkillList(user);
}
