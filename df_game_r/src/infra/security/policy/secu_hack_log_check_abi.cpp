#include "old_game/player/core/cuser_lifecycle_support_types.h"

#include <stdint.h>

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x827ABEC
// IDA_NAME: _ZN17Secu_MonsterCheck10SetUserPtrEP5CUser
// DEMANGLED: Secu_MonsterCheck::SetUserPtr(CUser *)
Secu_MonsterCheck* Secu_MonsterCheck::SetUserPtr(CUser* user) {
    user_ = user;
    return this;
}

// IDA_EA: 0x827A7D0
// IDA_NAME: _ZN17Secu_HackLogCheck4InitEP5CUser
// DEMANGLED: Secu_HackLogCheck::Init(CUser *)
Secu_MonsterCheck* Secu_HackLogCheck::Init(CUser* user) {
    monster_check_.init_user_ = user;
    return monster_check_.SetUserPtr(user);
}

// IDA_EA: 0x827AC7A
// IDA_NAME: _ZN17Secu_MonsterCheck5ResetEv
// DEMANGLED: Secu_MonsterCheck::Reset(void)
Secu_MonsterCheck* Secu_MonsterCheck::Reset() {
    if (check_223_.violation_state_ == 1 && user_) {
        user_->getHackAnalyzer()->addServerHackCnt(
            user_, static_cast<WongWork::ENUM_HACKTYPE>(0xdf), 1u, 0u, 0u);
    }
    if (check_224_.violation_state_ == 1 && user_) {
        user_->getHackAnalyzer()->addServerHackCnt(
            user_, static_cast<WongWork::ENUM_HACKTYPE>(0xe0), 1u, 0u, 0u);
    }
    if (check_225_.violation_state_ == 1 && user_) {
        user_->getHackAnalyzer()->addServerHackCnt(
            user_, static_cast<WongWork::ENUM_HACKTYPE>(0xe1), 1u, 0u, 0u);
    }

    check_223_enabled_ = 0;
    check_224_enabled_ = 0;
    check_225_enabled_ = 0;
    return this;
}

// IDA_EA: 0x827A7F4
// IDA_NAME: _ZN17Secu_HackLogCheck7MoveMapEv
// DEMANGLED: Secu_HackLogCheck::MoveMap(void)
int Secu_HackLogCheck::MoveMap() {
    monster_enabled_ = 1;
    log_enabled_ = 1;
    audit_enabled_ = 1;
    return static_cast<int>(reinterpret_cast<intptr_t>(monster_check_.Reset()));
}

// IDA_EA: 0x827ABFA
// IDA_NAME: _ZN17Secu_MonsterCheck14SetMonsterInfoEPK15MSG_MONSTER_DIE
void Secu_MonsterCheck::SetMonsterInfo(const MSG_MONSTER_DIE* message) {
    if (!message) {
        return;
    }
    last_die_.monster_id_ = message->monster_id_;
    last_die_.monster_x_ = message->field_a24_;
    last_die_.monster_y_ = message->field_a26_;
    last_die_.tick_ = GlobalData::system_time()->getCurTickCount();
    init223(message);
    init224(message);
    init225(message);
}

// IDA_EA: 0x827B288
void Secu_MonsterCheck::init223(const MSG_MONSTER_DIE* message) {
    if (check_223_enabled_ == 1 || !message) {
        return;
    }
    check_223_.monster_id_ = message->monster_id_;
    check_223_.monster_x_ = message->field_a24_;
    check_223_.monster_y_ = message->field_a26_;
    check_223_.check_count_ = 0;
    check_223_.violation_state_ = 0;
    check_223_enabled_ = 1;
}

// IDA_EA: 0x827B2E4
void Secu_MonsterCheck::init224(const MSG_MONSTER_DIE* message) {
    if (check_224_enabled_ == 1 || !message) {
        return;
    }
    check_224_.monster_id_ = message->monster_id_;
    check_224_.tick_ = GlobalData::system_time()->getCurTickCount();
    check_224_.check_count_ = 0;
    check_224_.violation_state_ = 0;
    check_224_enabled_ = 1;
}

// IDA_EA: 0x827B334
void Secu_MonsterCheck::init225(const MSG_MONSTER_DIE* message) {
    if (check_225_enabled_ == 1 || !message) {
        return;
    }
    check_225_.monster_id_ = message->monster_id_;
    check_225_.tick_ = GlobalData::system_time()->getCurTickCount();
    check_225_.check_count_ = 0;
    check_225_.violation_state_ = 0;
    check_225_enabled_ = 1;
}

namespace {

void AddMonsterCheckHack(CUser* user, unsigned int type, unsigned int count) {
    if (user) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(type), count, 0u, 0u);
    }
}

}  // namespace

// IDA_EA: 0x827AD98
int Secu_MonsterCheck::Check223() {
    if (check_223_.monster_id_ == last_die_.monster_id_) {
        return 1;
    }
    if (check_223_enabled_ == 0) {
        return 1;
    }

    bool violation = false;
    if (check_223_.monster_x_ > last_die_.monster_x_) {
        violation = true;
    } else if (last_die_.monster_x_ - check_223_.monster_x_ <= 199u &&
               check_223_.monster_y_ > last_die_.monster_y_) {
        violation = true;
    } else if (last_die_.monster_y_ - check_223_.monster_y_ <= 149u) {
        violation = true;
    }
    if (!violation) {
        if (check_223_.violation_state_ == 1) {
            AddMonsterCheckHack(user_, 0xdfu, 1u);
        }
        check_223_.monster_id_ = last_die_.monster_id_;
        check_223_.monster_x_ = last_die_.monster_x_;
        check_223_.monster_y_ = last_die_.monster_y_;
        check_223_.violation_state_ = 0;
        return 1;
    }

    ++check_223_.check_count_;
    if (check_223_.check_count_ <= 4u) {
        return 1;
    }
    if (check_223_.check_count_ <= 10u) {
        check_223_.violation_state_ = 1;
        return 1;
    }
    check_223_.violation_state_ = 3;
    AddMonsterCheckHack(user_, 0xdfu, 3u);
    return 0;
}

// IDA_EA: 0x827AF78
int Secu_MonsterCheck::Check224() {
    if (check_224_.monster_id_ == last_die_.monster_id_) {
        return 1;
    }
    if (check_224_enabled_ == 0) {
        return 1;
    }

    if (last_die_.tick_ - check_224_.tick_ > 1000u) {
        if (check_224_.violation_state_ == 1) {
            AddMonsterCheckHack(user_, 0xe0u, 1u);
        }
        check_224_.check_count_ = 0;
        check_224_.violation_state_ = 0;
    } else {
        ++check_224_.check_count_;
        if (check_224_.check_count_ > 10u) {
            check_224_.violation_state_ = 3;
            AddMonsterCheckHack(user_, 0xe0u, 3u);
            return 0;
        }
        if (check_224_.check_count_ > 4u) {
            check_224_.violation_state_ = 1;
        }
    }
    check_224_.monster_id_ = last_die_.monster_id_;
    check_224_.tick_ = last_die_.tick_;
    return 1;
}

// IDA_EA: 0x827B100
int Secu_MonsterCheck::Check225() {
    if (check_225_.monster_id_ == last_die_.monster_id_) {
        return 1;
    }
    if (check_225_enabled_ == 0) {
        return 1;
    }

    if (last_die_.tick_ - check_225_.tick_ > 1999u) {
        if (check_225_.violation_state_ == 1) {
            AddMonsterCheckHack(user_, 0xe1u, 1u);
        }
        check_225_.check_count_ = 0;
        check_225_.violation_state_ = 0;
    } else {
        ++check_225_.check_count_;
        if (check_225_.check_count_ > 10u) {
            check_225_.violation_state_ = 3;
            AddMonsterCheckHack(user_, 0xe1u, 3u);
            return 0;
        }
        if (check_225_.check_count_ > 4u) {
            check_225_.violation_state_ = 1;
        }
    }
    check_225_.monster_id_ = last_die_.monster_id_;
    check_225_.tick_ = last_die_.tick_;
    return 1;
}

// IDA_EA: 0x827A81C
int Secu_HackLogCheck::DieMob(const MSG_MONSTER_DIE* message) {
    if (!message || message->monster_type_ == 0xffffu) {
        return 0;
    }
    monster_check_.SetMonsterInfo(message);
    if (monster_enabled_ != 0) {
        monster_enabled_ = static_cast<uint8_t>(monster_check_.Check223());
    }
    if (log_enabled_ != 0) {
        log_enabled_ = static_cast<uint8_t>(monster_check_.Check224());
    }
    if (audit_enabled_ != 0) {
        audit_enabled_ = static_cast<uint8_t>(monster_check_.Check225());
    }
    return audit_enabled_;
}
