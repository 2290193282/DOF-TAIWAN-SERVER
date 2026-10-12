#include "old_data/Login/security/inter_account_security_return_dispatchers.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/pvp_party/db_save_user_handicap.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/codehack_checksum.h"

namespace {

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D1AB6
// IDA_NAME: _ZN23Inter_SaveClientHackLog12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SaveClientHackLog::dispatch_sig(CUser *,char *,int)
int Inter_SaveClientHackLog::dispatch_sig(CUser*, char* payload, int) {
    CUser* const user = G_CGameManager()->getUser(
        G_GameWorld()->find_session(ReadU32(payload, 0)), 0);
    if (!user || ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                     RestrictCategory::Security, 1) != 1) {
        return 0;
    }

    const uint16_t hack_type = ReadU16(payload, 8);
    int reduction = 0;
    if (hack_type == 503) {
        if (ReadU32(payload, 14) > 50u) reduction += 40;
    } else if (hack_type > 503) {
        if (hack_type == 806) {
            if (ReadU32(payload, 14) > 25u) reduction += 30;
        } else if (hack_type == 954 && ReadU32(payload, 14) > 1200u) {
            reduction += 20;
        }
    } else if (hack_type == 403 && ReadU32(payload, 14) > 250u) {
        reduction += 20;
    }

    int next_rate = 100 - std::min(100, reduction);
    next_rate = std::min(next_rate, user->getStdDropRate());
    if (user->getStdDropRate() != next_rate) {
        DB_SaveUserHandicap::makeRequest(user->get_acc_id(), 40001, 101,
                                         next_rate, 3, 0);
        user->setStdDropRate(next_rate);
    }
    return 0;
}

// IDA_EA: 0x84ED86C / 0x84F9654 / 0x84F9684
Inter_SaveClientHackLog::Inter_SaveClientHackLog() : ISigDispatcher() {}
Inter_SaveClientHackLog::~Inter_SaveClientHackLog() {}

// IDA_EA: 0x84D0648
// IDA_NAME: _ZN23Inter_CodeChecksumRange12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CodeChecksumRange::dispatch_sig(CUser *,char *,int)
int Inter_CodeChecksumRange::dispatch_sig(CUser*, char* payload, int) {
    CodeHackCheckRangeEx range;
    CodeHackChecksumEx checksum;
    const char* const checksum_source = range.setData(payload);
    checksum.setData(checksum_source);

    CGameManager::UserMap& users = G_CGameManager()->users_by_account_;
    for (CGameManager::UserMap::iterator it = users.begin();
         it != users.end(); ++it) {
        CUser* const target = it->second;
        if (target->checkCodeChecksumRange(checksum, range) != 1 &&
            target->get_state() > 1) {
            // The original reads this counter before reporting; preserve that
            // call even though its returned value is not used in this path.
            static_cast<void>(target->GetChecksumAnswerCount());
            target->getHackAnalyzer()->addServerHackCnt(
                target, static_cast<WongWork::ENUM_HACKTYPE>(0x3B7u), 1, 0,
                0);
        }
    }
    return 0;
}

// IDA_EA: 0x84ED6E4
// IDA_NAME: _ZN23Inter_CodeChecksumRangeC2Ev
// DEMANGLED: Inter_CodeChecksumRange::Inter_CodeChecksumRange(void)
Inter_CodeChecksumRange::Inter_CodeChecksumRange() : ISigDispatcher() {}

// IDA_EA: 0x84F9A4A
// IDA_NAME: _ZN23Inter_CodeChecksumRangeD2Ev
// DEMANGLED: Inter_CodeChecksumRange::~Inter_CodeChecksumRange()
// IDA_EA: 0x84F9A7A
// IDA_NAME: _ZN23Inter_CodeChecksumRangeD0Ev
// DEMANGLED: Inter_CodeChecksumRange::~Inter_CodeChecksumRange()
Inter_CodeChecksumRange::~Inter_CodeChecksumRange() {}

// IDA_EA: 0x84D07D2
// IDA_NAME: _ZN19Inter_MouseRegister12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MouseRegister::dispatch_sig(CUser *,char *,int)
int Inter_MouseRegister::dispatch_sig(CUser* user, char* payload, int) {
    const uint32_t error = ReadU32(payload, 22);
    if (error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER,
                                 static_cast<unsigned char>(error));
        return 0;
    }

    Sanicova::CPad* const pad = user->getPad();
    const uint32_t request_type = ReadU32(payload, 4);
    if (request_type == 1 || request_type == 2) {
        pad->setActivate(true);
        pad->setCertified(true);
        pad->setPassword(payload + 35);
        pad->setCancelCnt(static_cast<int>(ReadU32(payload, 27)));
        pad->setFailCnt(static_cast<int>(ReadU32(payload, 31)));
    } else if (request_type == 3) {
        pad->setActivate(false);
        pad->setCertified(false);
        pad->setPassword(payload + 35);
    }

    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_MOUSEREGISTER);
    reply.put_byte(1);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED71C / 0x84F99FC / 0x84F9A2C
Inter_MouseRegister::Inter_MouseRegister() : ISigDispatcher() {}
Inter_MouseRegister::~Inter_MouseRegister() {}

// IDA_EA: 0x84D271E
// IDA_NAME: _ZN24Inter_ChagneUserHandicap12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ChagneUserHandicap::dispatch_sig(CUser *,char *,int)
int Inter_ChagneUserHandicap::dispatch_sig(CUser*, char* payload, int) {
    CUser* const target = G_CGameManager()->GetUserByAccId(ReadU32(payload, 10));
    if (!target || target->get_state() <= 1) {
        return 0;
    }
    if (ReadU32(payload, 14) == 1) {
        target->setStdDropRate(static_cast<int>(ReadU32(payload, 18)));
    }
    return 0;
}

// IDA_EA: 0x84ED8DC / 0x84F951C / 0x84F954C
Inter_ChagneUserHandicap::Inter_ChagneUserHandicap() : ISigDispatcher() {}
Inter_ChagneUserHandicap::~Inter_ChagneUserHandicap() {}
