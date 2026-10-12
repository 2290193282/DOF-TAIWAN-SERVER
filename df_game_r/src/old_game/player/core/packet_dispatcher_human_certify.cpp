#include "old_game/player/core/packet_dispatcher_human_certify.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8596230
// IDA_NAME: _ZN23Dispatcher_HumanCertifyC2Ev
// DEMANGLED: Dispatcher_HumanCertify::Dispatcher_HumanCertify(void)
Dispatcher_HumanCertify::Dispatcher_HumanCertify()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242886
// IDA_NAME: _ZN23Dispatcher_HumanCertifyD2Ev
// DEMANGLED: Dispatcher_HumanCertify::~Dispatcher_HumanCertify()
// IDA_EA: 0x82428B6
// IDA_NAME: _ZN23Dispatcher_HumanCertifyD0Ev
// DEMANGLED: Dispatcher_HumanCertify::~Dispatcher_HumanCertify()
Dispatcher_HumanCertify::~Dispatcher_HumanCertify() {}

// IDA_EA: 0x82064D6
// IDA_NAME: _ZN23Dispatcher_HumanCertify12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_HumanCertify::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_HumanCertify::dispatch_sig(CUser* user, PacketBuf& packet) {
    static const char kDispatchSignature[] =
        "virtual int Dispatcher_HumanCertify::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2) {
        return LineFunc(40967, kDispatchSignature, 0, 0);
    }

    // 140828 native request: { u16 verification_data, u8 answer_mode=2 }.
    // The pre-140828 owner consumed an answer-size and a fixed ten-byte answer
    // buffer before these fields; accepting that prefix shifts the native
    // verification token and permits malformed trailing data.
    int answer_size = 0;
    unsigned short answer[5] = {0, 0, 0, 0, 0};
    unsigned short verification_data = 0;
    unsigned char answer_mode = 0;
    if (packet.get_short(verification_data) != 1 ||
        packet.get_byte(answer_mode) != 1 || answer_mode != 2u) {
        return LineFunc(40995, kDispatchSignature, 0, 0);
    }
    if (user->getMCAPManagerW().getPoint() > 0x251bu &&
        get_rand_int(100) <= 10) {
        user->getHistoryLogW().RequestCleanPad(
            user->getMCAPManagerW().getPoint(), 2);
        user->DisConnSig(static_cast<DISCONN_SIG>(53), true, 0);
    }

    bool clear_human_data = true;
    if (answer_mode == 2u) {
        user->setHumanCertified(true);
        user->SendMail(false);
        user->resetCleanpadFailCnt();
        user->resetHumanCertifyData();
    } else if (user->checkHumanCertify(answer_size, answer)) {
        if (answer_mode == 1u) {
            user->SendMail(true);
        }
        user->resetCleanpadFailCnt();
        user->getHistoryLogW().RequestCleanPad(
            user->getMCAPManagerW().getPoint(), 1);
        if (GlobalData::s_systemTime_.getCurTickCount() -
                user->getCleanPadRequestTick() <=
            0x7cfu) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x341u),
                1u,
                0u,
                0u);
        }
    } else {
        user->getHistoryLogW().RequestCleanPad(
            user->getMCAPManagerW().getPoint(), 2);
        clear_human_data = false;
        if (user->isCleanPadVerifyLimit()) {
            if (answer_mode == 1u) {
                user->SendMail(false);
                user->resetHumanCertifyData();
                user->resetCleanpadFailCnt();
            } else {
                user->DisConnSig(static_cast<DISCONN_SIG>(31), true, 0);
            }
        } else if (answer_mode == 1u) {
            if (user->reqSendMailCertify() != 1) {
                user->SendMail(true);
            }
        } else {
            user->setHumanCertified(true);
            user->reqHumanCertify4ClearMap(false);
        }
    }

    if (user->VerifyCleanPadVeriData(verification_data)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x337u),
            1u,
            0u,
            0u);
    }
    if (clear_human_data) {
        user->resetHumanCertifyData();
    }
    return 0;
}

// IDA_EA: 0x82338C6
// IDA_NAME: _ZN23Dispatcher_HumanCertify4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_HumanCertify::read(PacketBuf &,MSG_BASE &)
int Dispatcher_HumanCertify::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82338D0
// IDA_NAME: _ZN23Dispatcher_HumanCertify7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_HumanCertify::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_HumanCertify::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
