#include "old_data/Login/packet_dispatch/mouse_register_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/hades_public_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/auth_security/db_mouse_register.h"
#include "old_data/db/auth_security/db_passpad_update_fail_cnt.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/security/crypto/cipher.h"
#include "infra/security/crypto/ctea_abi.h"

// This inclusion remains translation-unit local.  The explicit instantiation
// below is the sole owner of the old state-2 MSG_TEMP_STRUCT/TempParam vtable.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_MouseRegister::dispatch_sig(CUser*, PacketBuf&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_MouseRegister::process(CUser*, MSG_BASE&, ParamBase&)";

void NotifyMouseReply(CUser* user, unsigned char result,
                      unsigned char error, int fail_count) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY);
    response.put_byte(result);
    response.put_byte(error);
    response.put_byte(fail_count);
    response.finalize(true);
    user->Send(response);
}

void SendMouseRegisterRequest(CUser* user, unsigned char request_kind,
                              const char* previous_password,
                              const char* requested_password,
                              bool copy_pad_password) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("PacketDispatcher_Secu.cpp",
                                            copy_pad_password ? 838 : 698),
        true);
    *guard << 163;
    *guard << user->GetUID();

    SIG_MOUSE_REGISTER* const request = guard.GetInBuffer<SIG_MOUSE_REGISTER>();
    // Raw CMD116 writes these fields individually.  Do not clear the payload:
    // its producer/consumer contract owns the untouched bytes.
    request->account_id = static_cast<unsigned int>(user->get_acc_id());
    request->request_kind = request_kind;
    std::strncpy(request->previous_password, previous_password,
                 sizeof(request->previous_password));
    std::strncpy(request->requested_password, requested_password,
                 sizeof(request->requested_password));
    std::strncpy(request->ip, user->getWebAddress(), sizeof(request->ip));

    if (request_kind == 3u) {
        CUser_history_log(user)->SecuService(
            static_cast<ENUM_SECU_SERVICE_TYPE>(0),
            static_cast<ENUM_SECU_SERVICE_ACTION>(4));
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
    } else {
        if (request_kind == 1u) {
            CUser_history_log(user)->SecuService(
                static_cast<ENUM_SECU_SERVICE_TYPE>(0),
                static_cast<ENUM_SECU_SERVICE_ACTION>(2));
        } else if (request_kind == 2u) {
            CUser_history_log(user)->SecuService(
                static_cast<ENUM_SECU_SERVICE_TYPE>(0),
                static_cast<ENUM_SECU_SERVICE_ACTION>(3));
        }
        if (request_kind == 1u || request_kind == 2u) {
            user->getHades()->ChangeSecurity(
                static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
        }
    }

    if (copy_pad_password) {
        std::memcpy(request->encrypted_password, user->getPad()->getPassword(),
                    sizeof(request->encrypted_password));
        request->advice_change_password =
            static_cast<uint8_t>(user->getPad()->getAdvideChangePwd());
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

}  // namespace

// IDA_EA: 0x8265276
// IDA_NAME: _ZN15MSG_TEMP_STRUCTC2Ev
// DEMANGLED: MSG_TEMP_STRUCT::MSG_TEMP_STRUCT(void)
MSG_TEMP_STRUCT::MSG_TEMP_STRUCT() : MSG_BASE() {}

// IDA_EA: 0x826533C
// IDA_NAME: _ZN9TempParamC2Ev
// DEMANGLED: TempParam::TempParam(void)
TempParam::TempParam() : ParamBase(), result(0) {}

// IDA_EA: 0x8265A6E
// IDA_NAME: _ZN9TempParamD2Ev
// DEMANGLED: TempParam::~TempParam()
// IDA_EA: 0x8265A9E
// IDA_NAME: _ZN9TempParamD0Ev
// DEMANGLED: TempParam::~TempParam()
TempParam::~TempParam() {}

// IDA_EA: 0x82669B4
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8266B10
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x826700E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x826703E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x8267142
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x8267148
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x859864E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_TEMP_STRUCT9TempParamL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_TEMP_STRUCT,TempParam,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_TEMP_STRUCT, TempParam,
                                static_cast<ch_state>(2)>;

// IDA_EA: 0x8596054
// IDA_NAME: _ZN24Dispatcher_MouseRegisterC2Ev
// DEMANGLED: Dispatcher_MouseRegister::Dispatcher_MouseRegister(void)
Dispatcher_MouseRegister::Dispatcher_MouseRegister() {}

// IDA_EA: 0x82657B0
// IDA_NAME: _ZN24Dispatcher_MouseRegisterD2Ev
// DEMANGLED: Dispatcher_MouseRegister::~Dispatcher_MouseRegister()
// IDA_EA: 0x82657E0
// IDA_NAME: _ZN24Dispatcher_MouseRegisterD0Ev
// DEMANGLED: Dispatcher_MouseRegister::~Dispatcher_MouseRegister()
Dispatcher_MouseRegister::~Dispatcher_MouseRegister() {}

// IDA_EA: 0x8263210
// IDA_NAME: _ZN24Dispatcher_MouseRegister4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MouseRegister::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MouseRegister::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8263224
// IDA_NAME: _ZN24Dispatcher_MouseRegister7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MouseRegister::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MouseRegister::process(CUser* user, MSG_BASE& message,
                                      ParamBase& param) {
    TempParam& result = static_cast<TempParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result >= 0) {
        return 0;
    }
    return LineFunc(376, kProcessSignature, result.result, 0);
}

// IDA_EA: 0x82632A0
// IDA_NAME: _ZN24Dispatcher_MouseRegister11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MouseRegister::check_error(CUser *,MSG_BASE &)
int Dispatcher_MouseRegister::check_error(CUser* user, MSG_BASE& message) {
    (void)user;
    (void)message;
    return 0;
}

// IDA_EA: 0x82632B4
// IDA_NAME: _ZN24Dispatcher_MouseRegister4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_MouseRegister::send(CUser *,ParamBase &)
void Dispatcher_MouseRegister::send(CUser* user, ParamBase& param) {
    (void)user;
    (void)static_cast<TempParam&>(param).result;
}

// IDA_EA: 0x82632CA
// IDA_NAME: _ZN24Dispatcher_MouseRegister19checkValiedPasswordEPKc
// DEMANGLED: Dispatcher_MouseRegister::checkValiedPassword(char const*)
int Dispatcher_MouseRegister::checkValiedPassword(const char* password) {
    const int length = static_cast<int>(std::strlen(password));
    if (length > 6) {
        return 0;
    }
    for (int index = 0; index < length; ++index) {
        const char value = password[index];
        if (value != 'a' && (value <= '/' || value > '9')) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x826333A
// IDA_NAME: _ZN24Dispatcher_MouseRegister15decryptPasswordEPKcPcj
// DEMANGLED: Dispatcher_MouseRegister::decryptPassword(char const*,char *,unsigned int)
int Dispatcher_MouseRegister::decryptPassword(const char* encrypted,
                                               char* password,
                                               uint32_t length) {
    const int result = GlobalData::s_pcryptTEA_MousePassword_->Decrypt(
        encrypted, password, length);
    if (result >= 0) {
        return 1;
    }
    LogManager::logFormat(1, "PacketDispatcher_Secu.cpp",
                          "static bool Dispatcher_MouseRegister::decryptPassword(const char*, char*, size_t)",
                          433, "[%s][%d][%s]", "PacketDispatcher_Secu.cpp",
                          433, "Decrypt Fail");
    return 0;
}

// IDA_EA: 0x82633C6
// IDA_NAME: _ZN24Dispatcher_MouseRegister15doPassPadReplayEP5CUserPccRbRh
// DEMANGLED: Dispatcher_MouseRegister::doPassPadReplay(CUser *,char *,char,bool &,unsigned char &)
void Dispatcher_MouseRegister::doPassPadReplay(CUser* user, char* answer,
                                                char modification_type,
                                                bool& success,
                                                unsigned char& error) {
    success = false;
    error = 0;
    for (int index = 0; index <= 3; ++index) {
        answer[index] = static_cast<char>(
            answer[index] - user->getPad()->GetShakeData(index));
    }

    if (user->getPad()->checkAnswer(answer)) {
        CUser_history_log(user)->SecuService(
            static_cast<ENUM_SECU_SERVICE_TYPE>(0),
            static_cast<ENUM_SECU_SERVICE_ACTION>(1));
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
        success = true;

        PacketGuard response;
        response.put_header(1, ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY);
        response.put_byte(2);
        response.put_byte(user->getPad()->getAdvideChangePwd());
        response.finalize(true);
        user->Send(response);

        if (user->getPad()->getFailCnt() > 0) {
            DB_PassPadUpdateFailCnt::makeRequest(
                static_cast<unsigned int>(user->get_acc_id()), 0u,
                user->getWebAddress());
        }
        user->getPad()->setCertified(true);
        user->getPad()->resetFailCnt();
        DB_GoblinPadUpdateReward::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()), modification_type,
            false, user->getWebAddress());
        return;
    }

    CUser_history_log(user)->SecuService(
        static_cast<ENUM_SECU_SERVICE_TYPE>(0),
        static_cast<ENUM_SECU_SERVICE_ACTION>(0));
    if (user->getPad()->incFailCnt() <= 2) {
        NotifyMouseReply(user, 0u, 135u, user->getPad()->getFailCnt());
        user->getPad()->getFailCnt();
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
    } else {
        error = 136u;
        user->getHades()->ChangeSecurity(
            static_cast<XNuclear::ENUM_HADES_SECURITY>(0), 0u);
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x342u), 1u, 0u,
            0u);
    }
    DB_PassPadUpdateFailCnt::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(user->getPad()->getFailCnt()),
        user->getWebAddress());
}

// IDA_EA: 0x82637FE
// IDA_NAME: _ZN24Dispatcher_MouseRegister12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_MouseRegister::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_MouseRegister::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 && user->get_state() != 2) {
        return LineFunc(636, kDispatchSignature, 0, 0);
    }

    unsigned char request_kind = 0;
    if (packet.get_byte(request_kind) != 1) {
        return LineFunc(642, kDispatchSignature, 0, 0);
    }
    if (user->getPad()->getFailCnt() > 2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER, 136);
        return 0;
    }
    if (user->getPad()->getCancelCnt() > 10) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER, 139);
        return 0;
    }

    char decoded_current[4096];
    char decoded_requested[4096];
    std::memset(decoded_current, 0, sizeof(decoded_current));
    std::memset(decoded_requested, 0, sizeof(decoded_requested));

    if (request_kind != 1u) {
        if (!user->getPad()->isActivate() || !user->getPad()->isCertified()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER, 1);
            return 0;
        }
    }

    if (request_kind == 3u) {
        SendMouseRegisterRequest(user, request_kind, decoded_current,
                                 decoded_requested, false);
        return 0;
    }

    unsigned char cipher_sequence = 14u;
    if (packet.get_byte(cipher_sequence) != 1) {
        return LineFunc(732, kDispatchSignature, 0, 0);
    }
    if (!cipher_sequence || cipher_sequence > 13u || cipher_sequence == 5u) {
        return LineFunc(739, kDispatchSignature, 0, 0);
    }

    // These two 32-byte buffers and the unchecked packet-provided length are
    // deliberate raw behavior at 0x82637FE.  Validation occurs in Cipher,
    // not before PacketBuf::get_binary or the duplicate copy.
    uint32_t encrypted_length = 0;
    int duplicate_length = 0;
    unsigned char encrypted[32];
    unsigned char encrypted_copy[32];
    if (packet.get_int(encrypted_length) != 1) {
        return LineFunc(744, kDispatchSignature, 0, 0);
    }
    if (packet.get_binary(reinterpret_cast<char*>(encrypted),
                          encrypted_length) != 1) {
        return LineFunc(746, kDispatchSignature, 0, 0);
    }
    duplicate_length = static_cast<int>(encrypted_length);
    std::memcpy(encrypted_copy, encrypted, encrypted_length);

    int decoded_current_length = 4096;
    int decoded_requested_length = 4096;
    Cipher* const cipher = GlobalData::s_secu_cipher_keystring_manager_[
        user->getPad()->GetSelectedCipherIdx()];
    if (cipher->Decrypt(cipher_sequence, encrypted, duplicate_length,
                        reinterpret_cast<unsigned char*>(decoded_current),
                        &decoded_current_length) != true) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER, 1);
        return 0;
    }
    if (cipher->Decrypt(cipher_sequence, encrypted_copy, duplicate_length,
                        reinterpret_cast<unsigned char*>(decoded_requested),
                        &decoded_requested_length) != true) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MOUSEREGISTER, 1);
        return 0;
    }

    for (int index = 0; index <= 3; ++index) {
        decoded_current[index] = static_cast<char>(
            decoded_current[index] - user->getPad()->GetShakeData(index));
        decoded_requested[index] = static_cast<char>(
            decoded_requested[index] - user->getPad()->GetShakeData(index));
    }
    if (checkValiedPassword(decoded_current) != 1 ||
        checkValiedPassword(decoded_requested) != 1) {
        return LineFunc(835, kDispatchSignature, 0, 0);
    }

    SendMouseRegisterRequest(user, request_kind, decoded_current,
                             decoded_requested, true);
    return 0;
}
