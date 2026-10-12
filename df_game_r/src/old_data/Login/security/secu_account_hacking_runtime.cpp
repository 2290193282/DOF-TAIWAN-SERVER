#include "old_game/player/core/cuser.h"

#include <cstring>

#include "old_data/db/character/db_select_charac_mid.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8278AEC
// IDA_NAME: _ZN19Secu_AccountHacking8isAvatarEj
// DEMANGLED: Secu_AccountHacking::isAvatar(unsigned int)
int Secu_AccountHacking::isAvatar(uint32_t item_id) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    return item && item->vtable_ && item->vtable_->is_avatar_item(item);
}

// IDA_EA: 0x8278B48
// IDA_NAME: _ZN19Secu_AccountHacking8sendMailEPKcP23MSG_CHECK_MAIL_RECEIVER
// DEMANGLED: Secu_AccountHacking::sendMail(char const*,MSG_CHECK_MAIL_RECEIVER *)
char Secu_AccountHacking::sendMail(const char* charac_name,
                                   MSG_CHECK_MAIL_RECEIVER* message) {
    // The entry test and each dispatch arm independently retrieve the current
    // inventory in the old body.  Retaining that call sequence also preserves
    // the original null-inventory return of one.
    if (!owner_->getCurCharacInvenR()) {
        return 1;
    }

    if (message->check_type == MSG_CHECK_MAIL_RECEIVER::kMailboxSend) {
        const MSG_MAILBOX_SEND& request = message->payload.mailbox_send;
        const uint32_t current_gold = static_cast<uint32_t>(
            owner_->getCurCharacInvenR()->get_money());

        sendGold(charac_name, 0, request.money, current_gold, 2, 0);
        const char is_avatar = static_cast<char>(isAvatar(request.item_id));
        if (!is_avatar) {
            return is_avatar;
        }
        return sendAvatar(charac_name, 0);
    }

    if (message->check_type != MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend) {
        // Type 1 has no account-hacking scan in the original function and
        // falls through with its check-type value in EAX.
        return static_cast<char>(message->check_type);
    }

    const MSG_MULTI_MAILBOX_SEND& request =
        message->payload.multi_mailbox_send;
    const uint32_t current_gold = static_cast<uint32_t>(
        owner_->getCurCharacInvenR()->get_money());
    sendGold(charac_name, 0, request.money, current_gold, 2, 0);

    for (int index = 0; index < request.item_count; ++index) {
        if (isAvatar(request.items[index].item_id)) {
            sendAvatar(charac_name, 0);
        }
    }

    // The old loop terminator leaves EAX as zero, regardless of sendGold or
    // sendAvatar results.
    return 0;
}

// IDA_EA: 0x82792A0
// IDA_NAME: _ZN19Secu_AccountHacking21isDetectionGoldPattenEv
// DEMANGLED: Secu_AccountHacking::isDetectionGoldPatten(void)
int Secu_AccountHacking::isDetectionGoldPatten() {
    return detection_flags_[0] || detection_flags_[1] ||
           detection_flags_[2] || detection_flags_[3];
}

// IDA_EA: 0x8278F16
// IDA_NAME: _ZN19Secu_AccountHacking10sendAvatarEPKcj
// DEMANGLED: Secu_AccountHacking::sendAvatar(char const*,unsigned int)
char Secu_AccountHacking::sendAvatar(const char* charac_name,
                                     uint32_t account_id) {
    char result = static_cast<char>(isSameAccountCharacter(charac_name));
    if (result) {
        return result;
    }
    result = static_cast<char>(owner_ &&
        static_cast<unsigned short>(owner_->getCurCharacUsedFatigue()) <= 100u);
    if (!result) {
        return result;
    }

    ++avatar_count_;
    if (isDetectionGoldPatten()) {
        setPatten(AccountHackingType::AVATAR_DETECTION,
                  charac_name,
                  account_id,
                  0);
    }
    result = static_cast<char>(
        avatar_count_ > 7u &&
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec()) -
            login_time_ <= 0x257u);
    if (result) {
        return setPatten(AccountHackingType::AVATAR_REPEAT,
                         charac_name,
                         account_id,
                         0);
    }
    return result;
}

// IDA_EA: 0x8279182
// IDA_NAME: _ZN19Secu_AccountHacking22isSameAccountCharacterEPKc
// DEMANGLED: Secu_AccountHacking::isSameAccountCharacter(char const*)
int Secu_AccountHacking::isSameAccountCharacter(const char* charac_name) {
    if (!charac_name) return 0;
    for (std::vector<_Charac_info>::const_iterator it =
             owner_->characters_.begin();
         it != owner_->characters_.end(); ++it) {
        if (std::strncmp(it->fields_.character_name_,
                         charac_name,
                         sizeof(it->fields_.character_name_)) == 0) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x827912A
// IDA_NAME: _ZN19Secu_AccountHacking19getMatchingHacktypeEN18AccountHackingType1TE
// DEMANGLED: Secu_AccountHacking::getMatchingHacktype(AccountHackingType::T)
int Secu_AccountHacking::getMatchingHacktype(AccountHackingType::T type) {
    switch (type) {
    case AccountHackingType::GOLD_70_PERCENT:
        return 710;
    case AccountHackingType::GOLD_50_PERCENT:
        return 711;
    case AccountHackingType::GOLD_30_PERCENT:
        return 712;
    case AccountHackingType::GOLD_DETECTION:
        return 713;
    case AccountHackingType::AVATAR_DETECTION:
        return 714;
    case AccountHackingType::AVATAR_REPEAT:
        return 715;
    }
    return 4001;
}

// IDA_EA: 0x8279258
// IDA_NAME: _ZN19Secu_AccountHacking12isGoldPattenEN18AccountHackingType1TE
// DEMANGLED: Secu_AccountHacking::isGoldPatten(AccountHackingType::T)
int Secu_AccountHacking::isGoldPatten(AccountHackingType::T type) {
    return static_cast<unsigned int>(type) < 4u;
}

// IDA_EA: 0x8279282
// IDA_NAME: _ZN19Secu_AccountHacking14isAvatarPattenEN18AccountHackingType1TE
// DEMANGLED: Secu_AccountHacking::isAvatarPatten(AccountHackingType::T)
int Secu_AccountHacking::isAvatarPatten(AccountHackingType::T type) {
    return type == AccountHackingType::AVATAR_DETECTION ||
           type == AccountHackingType::AVATAR_REPEAT;
}

// IDA_EA: 0x827903C
// IDA_NAME: _ZN19Secu_AccountHacking9setPattenEN18AccountHackingType1TEPKcjj
// DEMANGLED: Secu_AccountHacking::setPatten(AccountHackingType::T,char const*,unsigned int,unsigned int)
char Secu_AccountHacking::setPatten(AccountHackingType::T type,
                                    const char* charac_name,
                                    uint32_t account_id,
                                    uint32_t gold) {
    char result = static_cast<char>(detection_flags_[static_cast<int>(type)]);
    if (result || !owner_) return result;

    detection_flags_[static_cast<int>(type)] = 1;
    const int hack_type = getMatchingHacktype(type);
    if (account_id) {
        return owner_->getHackAnalyzer()->addServerHackCnt(
            owner_, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1,
            account_id, gold);
    }

    DB_SelectCharacMid::makeRequest(owner_->GetUID(), 0, charac_name,
                                    static_cast<uint32_t>(hack_type), gold,
                                    0);
    return 0;
}

// IDA_EA: 0x8278CCE
// IDA_NAME: _ZN19Secu_AccountHacking8sendGoldEPKcjjjhPc
// DEMANGLED: Secu_AccountHacking::sendGold(char const*,unsigned int,unsigned int,unsigned int,unsigned char,char *)
char Secu_AccountHacking::sendGold(const char* charac_name,
                                   uint32_t account_id,
                                   uint32_t gold,
                                   uint32_t current_gold,
                                   uint8_t tracking_type,
                                   char* ip) {
    char result = static_cast<char>(isSameAccountCharacter(charac_name) ||
                                    gold <= 500u);
    if (result) return result;

    if (charac_name) {
        DB_SelectCharacMid::makeRequest(owner_->GetUID(), 0, charac_name,
                                        718u, gold, 0);
    } else {
        owner_->getHackAnalyzer()->addServerHackCnt(
            owner_, static_cast<WongWork::ENUM_HACKTYPE>(718), 1,
            account_id, gold);
        owner_->getHades()->GoldTracking(
            tracking_type, account_id, ip, gold,
            static_cast<uint32_t>(owner_->getCurCharacUsedFatigue()));
    }

    result = static_cast<char>(
        owner_ && owner_->getCurCharacUsedFatigue() <= 100);
    if (!result) return result;

    const int percent = static_cast<int>(
        static_cast<long double>(gold) /
        static_cast<long double>(current_gold) * 100.0L);
    result = setPatten(AccountHackingType::GOLD_DETECTION, charac_name,
                       account_id, gold);
    if (percent <= 69) {
        if (percent <= 49) {
            if (percent > 29) {
                return setPatten(AccountHackingType::GOLD_30_PERCENT,
                                 charac_name, account_id, gold);
            }
        } else {
            return setPatten(AccountHackingType::GOLD_50_PERCENT,
                             charac_name, account_id, gold);
        }
    } else {
        return setPatten(AccountHackingType::GOLD_70_PERCENT, charac_name,
                         account_id, gold);
    }
    return result;
}
