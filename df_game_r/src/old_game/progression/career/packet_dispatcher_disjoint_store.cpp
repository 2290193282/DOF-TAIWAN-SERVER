#include "old_game/progression/career/packet_dispatcher_disjoint_store.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/progression/career/expert_job_disjointer.h"
#include "old_game/progression/career/packet_dispatcher_expert_job_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x823127E
// IDA_NAME: _ZN26stCreateDisjointStoreParamC2Ev
// DEMANGLED: stCreateDisjointStoreParam::stCreateDisjointStoreParam(void)
stCreateDisjointStoreParam::stCreateDisjointStoreParam()
    : ParamBase(), error_code(0) {}

// IDA_EA: 0x8247654
// IDA_NAME: _ZN26stCreateDisjointStoreParamD2Ev
// DEMANGLED: stCreateDisjointStoreParam::~stCreateDisjointStoreParam()
// IDA_EA: 0x8247684
// IDA_NAME: _ZN26stCreateDisjointStoreParamD0Ev
// DEMANGLED: stCreateDisjointStoreParam::~stCreateDisjointStoreParam()
stCreateDisjointStoreParam::~stCreateDisjointStoreParam() {}

// IDA_EA: 0x8231316
// IDA_NAME: _ZN25stEnterDisjointStoreParamC2Ev
// DEMANGLED: stEnterDisjointStoreParam::stEnterDisjointStoreParam(void)
stEnterDisjointStoreParam::stEnterDisjointStoreParam()
    : ParamBase(), error_code(0) {}

// IDA_EA: 0x824751C
// IDA_NAME: _ZN25stEnterDisjointStoreParamD2Ev
// DEMANGLED: stEnterDisjointStoreParam::~stEnterDisjointStoreParam()
// IDA_EA: 0x824754C
// IDA_NAME: _ZN25stEnterDisjointStoreParamD0Ev
// DEMANGLED: stEnterDisjointStoreParam::~stEnterDisjointStoreParam()
stEnterDisjointStoreParam::~stEnterDisjointStoreParam() {}

// IDA_EA: 0x8596B44
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStoreC2Ev
// DEMANGLED: Dispatcher_CreateDisjointStore::Dispatcher_CreateDisjointStore(void)
Dispatcher_CreateDisjointStore::Dispatcher_CreateDisjointStore()
    : IPacketDispatcher<MSG_CREATE_DISJOINT_STORE,
                        stCreateDisjointStoreParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245ACC
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStoreD2Ev
// DEMANGLED: Dispatcher_CreateDisjointStore::~Dispatcher_CreateDisjointStore()
// IDA_EA: 0x8245AFC
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStoreD0Ev
// DEMANGLED: Dispatcher_CreateDisjointStore::~Dispatcher_CreateDisjointStore()
Dispatcher_CreateDisjointStore::~Dispatcher_CreateDisjointStore() {}

// IDA_EA: 0x81CF750
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CreateDisjointStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CreateDisjointStore::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CREATE_DISJOINT_STORE& request =
        static_cast<MSG_CREATE_DISJOINT_STORE&>(message);
    if (packet.get_int(request.title_length) != 1) {
        return LineFunc(10614,
                        "virtual int Dispatcher_CreateDisjointStore::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_str(request.title, 256, request.title_length) != 1) {
        return LineFunc(10617,
                        "virtual int Dispatcher_CreateDisjointStore::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(request.cost) != 1) {
        return LineFunc(10620,
                        "virtual int Dispatcher_CreateDisjointStore::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_short(request.pos_x) != 1) {
        return LineFunc(10623,
                        "virtual int Dispatcher_CreateDisjointStore::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_short(request.pos_y) != 1) {
        return LineFunc(10626,
                        "virtual int Dispatcher_CreateDisjointStore::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    return 0;
}

// IDA_EA: 0x81CF8D4
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStore11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreateDisjointStore::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CreateDisjointStore::check_error(CUser* user,
                                                 MSG_BASE&,
                                                 ParamBase&) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    if (user->GetCurCharacExpertJobType() != 3) {
        return -2;
    }
    if (user->get_state() != 3 || user->GetCurCharacExpertJob() ||
        user->GetParty()) {
        return 19;
    }
    return 0;
}

// IDA_EA: 0x81CF986
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreateDisjointStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CreateDisjointStore::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    MSG_CREATE_DISJOINT_STORE& request =
        static_cast<MSG_CREATE_DISJOINT_STORE&>(message);
    stCreateDisjointStoreParam& result =
        static_cast<stCreateDisjointStoreParam&>(param);
    result.error_code = check_error(user, message, param);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(
            10692,
            "virtual int Dispatcher_CreateDisjointStore::process(CUser*, MSG_BASE&, ParamBase&)",
            result.error_code,
            user->get_acc_id());
    }

    expert_job::CExpertJob* const job =
        GlobalData::s_ExpertJobMgr->AcquireExpertJob(
            static_cast<ENUM_EXPERT_JOB_TYPE>(3));
    if (!job) {
        result.error_code = 21;
        return 0;
    }

    user->SetCurCharacExpertJob(job);
    const int error = static_cast<expert_job::CDisjointer*>(job)
                          ->OnCreateDisjointStore(user,
                                                   request.title,
                                                   request.cost,
                                                   request.pos_x,
                                                   request.pos_y);
    if (error) {
        user->SetCurCharacExpertJob(0);
        GlobalData::s_ExpertJobMgr->
            FreeExpertJob(job);
        result.error_code = error;
    }
    return 0;
}

// IDA_EA: 0x81CFAF0
// IDA_NAME: _ZN30Dispatcher_CreateDisjointStore4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_CreateDisjointStore::send(CUser *,ParamBase &)
void Dispatcher_CreateDisjointStore::send(CUser* user, ParamBase& param) {
    const stCreateDisjointStoreParam& result =
        static_cast<const stCreateDisjointStoreParam&>(param);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_DISJOINT_STORE,
                                 static_cast<unsigned char>(result.error_code));
    }
}

// IDA_EA: 0x8596C08
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStoreC2Ev
// DEMANGLED: Dispatcher_EnterDisjointStore::Dispatcher_EnterDisjointStore(void)
Dispatcher_EnterDisjointStore::Dispatcher_EnterDisjointStore()
    : IPacketDispatcher<MSG_ENTER_DISJOINT_STORE,
                        stEnterDisjointStoreParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245994
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStoreD2Ev
// DEMANGLED: Dispatcher_EnterDisjointStore::~Dispatcher_EnterDisjointStore()
// IDA_EA: 0x82459C4
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStoreD0Ev
// DEMANGLED: Dispatcher_EnterDisjointStore::~Dispatcher_EnterDisjointStore()
Dispatcher_EnterDisjointStore::~Dispatcher_EnterDisjointStore() {}

// IDA_EA: 0x81D0082
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_EnterDisjointStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EnterDisjointStore::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_ENTER_DISJOINT_STORE& request =
        static_cast<MSG_ENTER_DISJOINT_STORE&>(message);
    return packet.get_short(request.owner_unique_id) == 1
               ? 0
               : LineFunc(10920,
                          "virtual int Dispatcher_EnterDisjointStore::read(PacketBuf&, MSG_BASE&)",
                          0,
                          0);
}

// IDA_EA: 0x81D00D8
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStore11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EnterDisjointStore::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EnterDisjointStore::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase&) {
    const MSG_ENTER_DISJOINT_STORE& request =
        static_cast<const MSG_ENTER_DISJOINT_STORE&>(message);
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    CUser* const owner = G_GameWorld()->find_from_world(request.owner_unique_id);
    if (!owner) {
        return 21;
    }
    expert_job::CExpertJob* const job = owner->GetCurCharacExpertJob();
    if (owner->GetCurCharacExpertJobType() != 3 || !job ||
        job->GetType() != 3) {
        return 19;
    }
    return 0;
}

// IDA_EA: 0x81D01AE
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EnterDisjointStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EnterDisjointStore::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    const MSG_ENTER_DISJOINT_STORE& request =
        static_cast<const MSG_ENTER_DISJOINT_STORE&>(message);
    stEnterDisjointStoreParam& result =
        static_cast<stEnterDisjointStoreParam&>(param);
    result.error_code = check_error(user, message, param);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(
            10975,
            "virtual int Dispatcher_EnterDisjointStore::process(CUser*, MSG_BASE&, ParamBase&)",
            result.error_code,
            user->get_acc_id());
    }

    CUser* const owner = G_GameWorld()->find_from_world(request.owner_unique_id);
    static_cast<expert_job::CDisjointer*>(owner->GetCurCharacExpertJob())
        ->OnEnterDisjointStore(user);
    return 0;
}

// IDA_EA: 0x81D0286
// IDA_NAME: _ZN29Dispatcher_EnterDisjointStore4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_EnterDisjointStore::send(CUser *,ParamBase &)
void Dispatcher_EnterDisjointStore::send(CUser* user, ParamBase& param) {
    const stEnterDisjointStoreParam& result =
        static_cast<const stEnterDisjointStoreParam&>(param);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENTER_DISJOINT_STORE,
                                 static_cast<unsigned char>(result.error_code));
    }
}
