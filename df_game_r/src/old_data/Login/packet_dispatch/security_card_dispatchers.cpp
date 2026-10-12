#include "old_data/Login/packet_dispatch/security_card_dispatchers.h"

#include <stdint.h>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/security/security_card.h"
#include "old_data/Login/security/security_card_center.h"
#include "old_data/db/auth_security/db_passpad_update_cancel_cnt.h"
#include "old_data/db/auth_security/db_security_card_activate.h"
#include "old_data/db/auth_security/db_security_card_add_cert_cnt.h"
#include "old_data/db/auth_security/db_security_card_disuse.h"
#include "old_data/db/auth_security/db_security_card_issue.h"
#include "old_data/db/auth_security/db_security_card_retransfer.h"
#include "old_data/db/auth_security/db_security_card_update_cancel_cnt.h"
#include "old_data/db/auth_security/db_security_card_update_fail_cnt.h"
#include "old_data/Login/packet_dispatch/mouse_register_dispatcher.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/crypto/csha1_abi.h"
#include "infra/security/crypto/cipher.h"

// The two state-2 security dispatcher triplets below each have exactly one
// source/vtable owner.  Do not move this include into the public header.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kSecurityCardIssueDispatchSignature[] =
    "virtual int Dispatcher_SecurityCardIssue::dispatch_sig(CUser*, PacketBuf&)";
const char kSecurityCardRetransferDispatchSignature[] =
    "virtual int Dispatcher_SecurityCardRetransfer::dispatch_sig(CUser*, PacketBuf&)";

int ReadPhoneAndHash(PacketBuf& packet, uint32_t& phone_length,
                     char phone[12], char phone_hash[41], int line_int,
                     int line_string, int line_phone,
                     const char* signature) {
    if (packet.get_int(phone_length) != 1) {
        return LineFunc(line_int, signature, 0, 0);
    }
    if (packet.get_str(phone, 12, static_cast<int>(phone_length)) != 1) {
        return LineFunc(line_string, signature, 0, 0);
    }
    if (GlobalData::s_securityCardCenter->checkPhoneNumber(phone) != 1) {
        return LineFunc(line_phone, signature, 0, 0);
    }

    CSHA1 phone_sha1;
    phone_sha1.Update(reinterpret_cast<const uint8_t*>(phone), phone_length);
    phone_sha1.Final();
    phone_sha1.GetHexHash(phone_hash);
    return 0;
}

void GenerateSecurityCardRequestData(char card[39], char server_key[33],
                                     char cert_key[6]) {
    GlobalData::s_securityCardCenter->generateCardData(card);
    GlobalData::s_securityCardCenter->generateServerKey(server_key);
    GlobalData::s_securityCardCenter->generateCertKey(cert_key);
}

}  // namespace

// IDA_EA: 0x8596444
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssueC2Ev
// DEMANGLED: Dispatcher_SecurityCardIssue::Dispatcher_SecurityCardIssue(void)
Dispatcher_SecurityCardIssue::Dispatcher_SecurityCardIssue() {}

// IDA_EA: 0x82422BC
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssueD2Ev
// DEMANGLED: Dispatcher_SecurityCardIssue::~Dispatcher_SecurityCardIssue()
// IDA_EA: 0x82422EC
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssueD0Ev
// DEMANGLED: Dispatcher_SecurityCardIssue::~Dispatcher_SecurityCardIssue()
Dispatcher_SecurityCardIssue::~Dispatcher_SecurityCardIssue() {}

// IDA_EA: 0x8209C8C
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssue12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardIssue::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardIssue::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return LineFunc(42390, kSecurityCardIssueDispatchSignature, 0, 0);
    }

    uint32_t phone_length = 0;
    char phone[12] = {};
    char phone_hash[41] = {};
    const int input_result = ReadPhoneAndHash(
        packet, phone_length, phone, phone_hash, 42399, 42400, 42403,
        kSecurityCardIssueDispatchSignature);
    if (input_result != 0) {
        return input_result;
    }

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        security_card = user->getSecurityCard();
        if (!security_card->isCertified()) {
            return LineFunc(42419, kSecurityCardIssueDispatchSignature, 0, 0);
        }
        security_card = user->getSecurityCard();
        if (security_card->getFailCnt() > 2) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 122);
            return 0;
        }
    }

    Sanicova::CPad* pad = user->getPad();
    if (pad->isActivate()) {
        pad = user->getPad();
        if (pad->getFailCnt() > 2) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 136);
            return 0;
        }
        pad = user->getPad();
        if (!pad->isCertified()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 134);
            return 0;
        }
    }

    char card[39] = {};
    char server_key[33] = {};
    char cert_key[6] = {};
    GenerateSecurityCardRequestData(card, server_key, cert_key);
    DB_SecurityCardIssue::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()),
        cert_key, server_key, phone, phone_hash, card);
    return 0;
}

// IDA_EA: 0x8233A42
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssue4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardIssue::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardIssue::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A4C
// IDA_NAME: _ZN28Dispatcher_SecurityCardIssue7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardIssue::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardIssue::process(CUser* user, MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85964EC
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransferC2Ev
// DEMANGLED: Dispatcher_SecurityCardRetransfer::Dispatcher_SecurityCardRetransfer(void)
Dispatcher_SecurityCardRetransfer::Dispatcher_SecurityCardRetransfer() {}

// IDA_EA: 0x8242136
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransferD2Ev
// DEMANGLED: Dispatcher_SecurityCardRetransfer::~Dispatcher_SecurityCardRetransfer()
// IDA_EA: 0x8242166
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransferD0Ev
// DEMANGLED: Dispatcher_SecurityCardRetransfer::~Dispatcher_SecurityCardRetransfer()
Dispatcher_SecurityCardRetransfer::~Dispatcher_SecurityCardRetransfer() {}

// IDA_EA: 0x820A9A0
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransfer12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardRetransfer::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardRetransfer::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() != 3) {
        return LineFunc(42668, kSecurityCardRetransferDispatchSignature, 0, 0);
    }

    uint32_t phone_length = 0;
    char phone[12] = {};
    char phone_hash[41] = {};
    const int input_result = ReadPhoneAndHash(
        packet, phone_length, phone, phone_hash, 42673, 42674, 42677,
        kSecurityCardRetransferDispatchSignature);
    if (input_result != 0) {
        return input_result;
    }

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (!security_card->isActivate()) {
        return 0;
    }
    security_card = user->getSecurityCard();
    if (security_card->getFailCnt() > 2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 122);
        return 0;
    }

    Sanicova::CPad* pad = user->getPad();
    if (pad->isActivate()) {
        pad = user->getPad();
        if (pad->getFailCnt() > 2) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 136);
            return 0;
        }
        pad = user->getPad();
        if (!pad->isCertified()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_ISSUE, 134);
            return 0;
        }
    }

    char card[39] = {};
    char server_key[33] = {};
    char cert_key[6] = {};
    GenerateSecurityCardRequestData(card, server_key, cert_key);
    DB_SecurityCardRetransfer::makeRequest(
        user->GetUID(), phone, phone_hash,
        static_cast<unsigned int>(user->get_acc_id()), cert_key, server_key,
        card);
    return 0;
}

// IDA_EA: 0x8233AA6
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransfer4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardRetransfer::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardRetransfer::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233AB0
// IDA_NAME: _ZN33Dispatcher_SecurityCardRetransfer7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardRetransfer::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardRetransfer::process(CUser* user, MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596460
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseC2Ev
// DEMANGLED: Dispatcher_SecurityCardDisuse::Dispatcher_SecurityCardDisuse(void)
Dispatcher_SecurityCardDisuse::Dispatcher_SecurityCardDisuse() {}

// IDA_EA: 0x824226E
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseD2Ev
// DEMANGLED: Dispatcher_SecurityCardDisuse::~Dispatcher_SecurityCardDisuse()
// IDA_EA: 0x824229E
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuseD0Ev
// DEMANGLED: Dispatcher_SecurityCardDisuse::~Dispatcher_SecurityCardDisuse()
Dispatcher_SecurityCardDisuse::~Dispatcher_SecurityCardDisuse() {}

// IDA_EA: 0x820A0F6
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardDisuse::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardDisuse::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 3) {
        return LineFunc(
            42460,
            "virtual int Dispatcher_SecurityCardDisuse::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    WongWork::CSecurityCard* const security_card = user->getSecurityCard();
    if (!security_card->isActivate()) {
        return LineFunc(
            42467,
            "virtual int Dispatcher_SecurityCardDisuse::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (!security_card->isCertified()) {
        return LineFunc(
            42470,
            "virtual int Dispatcher_SecurityCardDisuse::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    if (security_card->getFailCnt() > 2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_DISUSE, 122);
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_DISUSE);
    response.put_byte(1);
    response.finalize(true);
    user->Send(response);

    DB_SecurityCardDisuse::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()));
    security_card->init();
    return 0;
}

// IDA_EA: 0x8233A56
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardDisuse::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardDisuse::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A60
// IDA_NAME: _ZN29Dispatcher_SecurityCardDisuse7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardDisuse::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardDisuse::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859647C
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqC2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthReq::Dispatcher_SecurityCardAuthReq(void)
Dispatcher_SecurityCardAuthReq::Dispatcher_SecurityCardAuthReq() {}

// IDA_EA: 0x8242220
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqD2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthReq::~Dispatcher_SecurityCardAuthReq()
// IDA_EA: 0x8242250
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReqD0Ev
// DEMANGLED: Dispatcher_SecurityCardAuthReq::~Dispatcher_SecurityCardAuthReq()
Dispatcher_SecurityCardAuthReq::~Dispatcher_SecurityCardAuthReq() {}

// IDA_EA: 0x820A2C4
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardAuthReq::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardAuthReq::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2) {
        return LineFunc(
            42493,
            "virtual int Dispatcher_SecurityCardAuthReq::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (!security_card->isActivate()) {
        Sanicova::CPad* const pad = user->getPad();
        if (!pad->isActivate()) {
            return LineFunc(
                42533,
                "virtual int Dispatcher_SecurityCardAuthReq::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
    }

    security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        security_card = user->getSecurityCard();
        if (security_card->getFailCnt() > 2) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_AUTH_REQ,
                                     122);
            return 0;
        }
    } else {
        Sanicova::CPad* const pad = user->getPad();
        if (pad->getFailCnt() > 2) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_AUTH_REQ,
                                     136);
            return 0;
        }
    }

    GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(0));
    return 0;
}

// IDA_EA: 0x8233A6A
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardAuthReq::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardAuthReq::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A74
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthReq7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardAuthReq::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardAuthReq::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85964B4
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyC2Ev
// DEMANGLED: Dispatcher_SecurityCardCertKey::Dispatcher_SecurityCardCertKey(void)
Dispatcher_SecurityCardCertKey::Dispatcher_SecurityCardCertKey() {}

// IDA_EA: 0x8242184
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyD2Ev
// DEMANGLED: Dispatcher_SecurityCardCertKey::~Dispatcher_SecurityCardCertKey()
// IDA_EA: 0x82421B4
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKeyD0Ev
// DEMANGLED: Dispatcher_SecurityCardCertKey::~Dispatcher_SecurityCardCertKey()
Dispatcher_SecurityCardCertKey::~Dispatcher_SecurityCardCertKey() {}

// IDA_EA: 0x820A6BC
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardCertKey::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return LineFunc(
            42613,
            "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    unsigned int cert_key_length = 0;
    char cert_key[6] = {};
    if (packet.get_int(cert_key_length) != 1) {
        return LineFunc(
            42625,
            "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_str(cert_key, 6, static_cast<int>(cert_key_length)) != 1) {
        return LineFunc(
            42627,
            "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        security_card = user->getSecurityCard();
        if (security_card->isCertFlagOn()) {
            return LineFunc(
                42632,
                "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
    }

    security_card = user->getSecurityCard();
    if (security_card->isCertified()) {
        return LineFunc(
            42636,
            "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    for (unsigned int index = 0; index <= 4; ++index) {
        const int digit = static_cast<int>(
            static_cast<signed char>(cert_key[index])) - 0x30;
        if (static_cast<unsigned int>(digit) > 9u) {
            return LineFunc(
                42641,
                "virtual int Dispatcher_SecurityCardCertKey::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
    }

    security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        CUser_history_log(user)->SecuService(
            static_cast<ENUM_SECU_SERVICE_TYPE>(1),
            static_cast<ENUM_SECU_SERVICE_ACTION>(3));
        // Old-server callsite 0x820A906 passes enum 1 and value 0x1B.
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(1), 0x1Bu);
    } else {
        CUser_history_log(user)->SecuService(
            static_cast<ENUM_SECU_SERVICE_TYPE>(1),
            static_cast<ENUM_SECU_SERVICE_ACTION>(2));
        // Old-server callsite 0x820A92B passes enum 1 and value 0x1A.
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(1), 0x1Au);
    }

    security_card = user->getSecurityCard();
    const bool reissue_history = security_card->isActivate();
    DB_SecurityCardActivate::makeRequest(
        user->GetUID(), static_cast<unsigned int>(user->get_acc_id()),
        cert_key, reissue_history);
    return 0;
}

// IDA_EA: 0x8233A7E
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardCertKey::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardCertKey::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A88
// IDA_NAME: _ZN30Dispatcher_SecurityCardCertKey7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardCertKey::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardCertKey::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85964D0
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelC2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::Dispatcher_SecurityCardAuthCancel(void)
Dispatcher_SecurityCardAuthCancel::Dispatcher_SecurityCardAuthCancel() {}

// IDA_EA: 0x82421D2
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelD2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::~Dispatcher_SecurityCardAuthCancel()
// IDA_EA: 0x8242202
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancelD0Ev
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::~Dispatcher_SecurityCardAuthCancel()
Dispatcher_SecurityCardAuthCancel::~Dispatcher_SecurityCardAuthCancel() {}

// IDA_EA: 0x820A42C
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SecurityCardAuthCancel::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 1) {
        return LineFunc(
            42541,
            "virtual int Dispatcher_SecurityCardAuthCancel::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        security_card = user->getSecurityCard();
        if (security_card->getRequestState() != 1) {
            return 0;
        }
        if (!process(user)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_AUTH_CANCEL,
                                     125);
        }
    } else {
        Sanicova::CPad* pad = user->getPad();
        if (!pad->isActivate()) {
            return LineFunc(
                42576,
                "virtual int Dispatcher_SecurityCardAuthCancel::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
        pad = user->getPad();
        if (pad->getRequestState() != 1) {
            return 0;
        }
        if (!process(user)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SECURITY_CARD_AUTH_CANCEL,
                                     139);
        }
    }
    return 0;
}

// IDA_EA: 0x820A580
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel7processEP5CUser
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::process(CUser *)
int Dispatcher_SecurityCardAuthCancel::process(CUser* user) {
    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (security_card->isActivate()) {
        security_card = user->getSecurityCard();
        security_card->setRequestState(
            WongWork::CSecurityCard::REQUEST_STATE_NONE);
        security_card = user->getSecurityCard();
        security_card->incCancelCnt();
        security_card = user->getSecurityCard();
        const unsigned int cancel_count =
            static_cast<unsigned int>(security_card->getCancelCnt());
        DB_SecurityCardUpdateCancelCnt::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()), cancel_count);
        security_card = user->getSecurityCard();
        security_card->getCancelCnt();
    } else {
        Sanicova::CPad* pad = user->getPad();
        if (pad->isActivate()) {
            pad = user->getPad();
            pad->setRequestState(Sanicova::CPad::ENUM_REQUEST_STATE_NONE);
            pad = user->getPad();
            pad->incCancelCnt();
            char* const web_address = user->getWebAddress();
            pad = user->getPad();
            const unsigned int cancel_count =
                static_cast<unsigned int>(pad->getCancelCnt());
            DB_PassPadUpdateCancelCnt::makeRequest(
                static_cast<unsigned int>(user->get_acc_id()), cancel_count,
                web_address);
            pad = user->getPad();
            pad->getCancelCnt();
        }
    }
    return 1;
}

// IDA_EA: 0x8233A92
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardAuthCancel::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A9C
// IDA_NAME: _ZN33Dispatcher_SecurityCardAuthCancel7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardAuthCancel::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardAuthCancel::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8265362
// IDA_NAME: _ZN16SecuCardRpyParamC2Ev
// DEMANGLED: SecuCardRpyParam::SecuCardRpyParam(void)
SecuCardRpyParam::SecuCardRpyParam() : ParamBase(), success(0), error(0) {}

// IDA_EA: 0x8265A20
// IDA_NAME: _ZN16SecuCardRpyParamD2Ev
// DEMANGLED: SecuCardRpyParam::~SecuCardRpyParam()
// IDA_EA: 0x8265A50
// IDA_NAME: _ZN16SecuCardRpyParamD0Ev
// DEMANGLED: SecuCardRpyParam::~SecuCardRpyParam()
SecuCardRpyParam::~SecuCardRpyParam() {}

// IDA_EA: 0x8266BA4
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8266D00
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8266D0A
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x826705C
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x826708C
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x8267152
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x85986F6
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_SECU_RPY16SecuCardRpyParamL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECU_RPY,SecuCardRpyParam,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SECU_RPY, SecuCardRpyParam,
                                static_cast<ch_state>(2)>;

// IDA_EA: 0x8596498
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpyC2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::Dispatcher_SecurityCardAuthRpy(void)
Dispatcher_SecurityCardAuthRpy::Dispatcher_SecurityCardAuthRpy() {}

// IDA_EA: 0x82657FE
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpyD2Ev
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::~Dispatcher_SecurityCardAuthRpy()
// IDA_EA: 0x826582E
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpyD0Ev
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::~Dispatcher_SecurityCardAuthRpy()
Dispatcher_SecurityCardAuthRpy::~Dispatcher_SecurityCardAuthRpy() {}

// IDA_EA: 0x8262658
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityCardAuthRpy::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    MSG_SECU_RPY& input = static_cast<MSG_SECU_RPY&>(message);
    if (packet.get_byte(input.security_service) != 1) {
        return LineFunc(
            43,
            "virtual int Dispatcher_SecurityCardAuthRpy::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_byte(input.cipher_sequence) != 1) {
        return LineFunc(
            46,
            "virtual int Dispatcher_SecurityCardAuthRpy::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_int(input.encrypted_length) != 1) {
        return LineFunc(
            50,
            "virtual int Dispatcher_SecurityCardAuthRpy::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_binary(reinterpret_cast<char*>(input.encrypted),
                          input.encrypted_length) != 1) {
        return LineFunc(
            53,
            "virtual int Dispatcher_SecurityCardAuthRpy::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_byte(input.modification_type) == 1) {
        return 0;
    }
    return LineFunc(
        58,
        "virtual int Dispatcher_SecurityCardAuthRpy::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x82627CA
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityCardAuthRpy::process(CUser* user, MSG_BASE& message,
                                            ParamBase& param) {
    const int validation_result = check_error(user, message);
    if (validation_result > 0) {
        return -1;
    }

    MSG_SECU_RPY& input = static_cast<MSG_SECU_RPY&>(message);
    SecuCardRpyParam& result = static_cast<SecuCardRpyParam&>(param);
    result.success = 0;
    result.error = 0;

    uint8_t decrypted[4096];
    std::memset(decrypted, 0, sizeof(decrypted));
    int decrypted_length = static_cast<int>(sizeof(decrypted));

    WongWork::CSecurityCard* security_card = user->getSecurityCard();
    if (security_card->isActivate() && input.security_service == 0u) {
        // The card path intentionally ignores Cipher::Decrypt's return value.
        // The subsequent answer comparison receives the raw output buffer.
        Cipher* const cipher = GlobalData::s_secu_cipher_keystring_manager_[
            user->getPad()->GetSelectedCipherIdx()];
        cipher->Decrypt(input.cipher_sequence, input.encrypted,
                        static_cast<int>(input.encrypted_length), decrypted,
                        &decrypted_length);
    } else {
        if (!user->getPad()->isActivate() || input.security_service != 1u) {
            return LineFunc(
                118,
                "virtual int Dispatcher_SecurityCardAuthRpy::process(CUser*, MSG_BASE&, ParamBase&)",
                0, 0);
        }
        Cipher* const cipher = GlobalData::s_secu_cipher_keystring_manager_[
            user->getPad()->GetSelectedCipherIdx()];
        if (cipher->Decrypt(input.cipher_sequence, input.encrypted,
                            static_cast<int>(input.encrypted_length),
                            decrypted, &decrypted_length) != true) {
            result.error = 1;
            cMyTrace trace(
                "virtual int Dispatcher_SecurityCardAuthRpy::process(CUser*, MSG_BASE&, ParamBase&)",
                113, 0);
            trace("ERROR [TRACE AUTH RPY] : ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY,ENUM_ERROR_ETC");
            return 0;
        }
    }

    const uint8_t first_answer = decrypted[0];
    uint16_t second_answer = 0;
    std::memcpy(&second_answer, &decrypted[1], sizeof(second_answer));
    const uint8_t pad_answer[6] = {
        decrypted[0], decrypted[1], decrypted[2], decrypted[3], decrypted[4],
        decrypted[5]};

    security_card = user->getSecurityCard();
    if (!security_card->isActivate() || input.security_service != 0u) {
        if (!user->getPad()->isActivate() || input.security_service != 1u) {
            cMyTrace trace(
                "virtual int Dispatcher_SecurityCardAuthRpy::process(CUser*, MSG_BASE&, ParamBase&)",
                267, 0);
            trace("ERROR [TRACE AUTH RPY] : None Secu Service");
            return LineFunc(
                268,
                "virtual int Dispatcher_SecurityCardAuthRpy::process(CUser*, MSG_BASE&, ParamBase&)",
                0, 0);
        }

        user->getPad()->setRequestState(Sanicova::CPad::ENUM_REQUEST_STATE_NONE);
        if (user->getPad()->getFailCnt() > 2) {
            result.error = 136u;
            return 0;
        }

        uint8_t replay_answer[6];
        std::memcpy(replay_answer, pad_answer, sizeof(replay_answer));
        Dispatcher_MouseRegister::doPassPadReplay(
            user, reinterpret_cast<char*>(replay_answer),
            static_cast<char>(input.modification_type),
            result.success, result.error);
        if (!result.success) {
            return 0;
        }
        // Raw pad success sets the error byte to one, but send() prioritizes
        // success and emits only the trade-side notification in this case.
        result.error = 1u;
        result.success = 1u;
        return 0;
    }

    security_card->setRequestState(WongWork::CSecurityCard::REQUEST_STATE_NONE);
    if (static_cast<unsigned int>(security_card->getFailCnt()) > 2u) {
        result.error = 122u;
        return 0;
    }

    if (security_card->checkAnswer(first_answer,
                                   static_cast<unsigned char>(second_answer))) {
        CUser_history_log(user)->SecuService(
            static_cast<ENUM_SECU_SERVICE_TYPE>(1),
            static_cast<ENUM_SECU_SERVICE_ACTION>(1));
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);

        PacketGuard reply;
        reply.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY);
        reply.put_byte(1);
        reply.put_byte(security_card->getCertCnt() > 59 ? 1 : 0);
        reply.finalize(true);
        user->Send(reply);

        DB_SecurityCardUpdateFailCnt::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()), 0u);
        if (security_card->getCancelCnt()) {
            DB_SecurityCardUpdateCancelCnt::makeRequest(
                static_cast<unsigned int>(user->get_acc_id()), 0u);
        }
        DB_SecurityCardAddCertCnt::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()));
        security_card->setCertified(true);
        security_card->resetFailCnt();
        result.success = 1u;
        return 0;
    }

    CUser_history_log(user)->SecuService(
        static_cast<ENUM_SECU_SERVICE_TYPE>(1),
        static_cast<ENUM_SECU_SERVICE_ACTION>(0));
    if (security_card->incFailCnt() <= 2) {
        result.error = 121u;
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0),
            security_card->getFailCnt() == 1 ? 21u : 22u);
    } else {
        result.error = 122u;
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x343u), 1u, 0u,
            0u);
    }
    DB_SecurityCardUpdateFailCnt::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(security_card->getFailCnt()));
    return 0;
}

// IDA_EA: 0x8262F4E
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::send(CUser *,ParamBase &)
void Dispatcher_SecurityCardAuthRpy::send(CUser* user, ParamBase& param) {
    SecuCardRpyParam& result = static_cast<SecuCardRpyParam&>(param);
    if (result.success) {
        CTradeSpace* const trade_space =
            G_CGameManager()->GetTradeSpace(user->GetTradeSpace());
        if (trade_space && trade_space->is_empty() != 1) {
            PacketGuard trade_notification;
            trade_notification.clear();
            trade_notification.put_header(0, 176);
            trade_notification.put_byte(0);
            trade_notification.finalize(true);
            CUser* const opposite = trade_space->GetOppositeUser(user);
            if (opposite) {
                opposite->Send(trade_notification);
            }
        }
        return;
    }

    if (result.error) {
        PacketGuard reply;
        reply.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY);
        reply.put_byte(0);
        reply.put_byte(result.error);
        if (result.error == 135u) {
            reply.put_byte(user->getPad()->getFailCnt());
        }
        reply.finalize(true);
        user->Send(reply);
    }
}

// IDA_EA: 0x826314C
// IDA_NAME: _ZN30Dispatcher_SecurityCardAuthRpy11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityCardAuthRpy::check_error(CUser *,MSG_BASE &)
int Dispatcher_SecurityCardAuthRpy::check_error(CUser* user,
                                                MSG_BASE& message) {
    const MSG_SECU_RPY& input = static_cast<const MSG_SECU_RPY&>(message);
    if (input.cipher_sequence && input.cipher_sequence <= 13u &&
        input.cipher_sequence != 5u) {
        if (user->get_state() > 1) {
            return 0;
        }
        cMyTrace trace(
            "int Dispatcher_SecurityCardAuthRpy::check_error(CUser*, MSG_BASE&)",
            340, 0);
        trace("ERROR [TRACE AUTH RPY] : get_state()<LOGIN");
        return 341;
    }

    cMyTrace trace(
        "int Dispatcher_SecurityCardAuthRpy::check_error(CUser*, MSG_BASE&)",
        332, 0);
    trace("ERROR [TRACE AUTH RPY] : crypto_type2");
    return 333;
}
