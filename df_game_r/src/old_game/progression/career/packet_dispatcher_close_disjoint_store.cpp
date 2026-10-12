#include "old_game/progression/career/packet_dispatcher_close_disjoint_store.h"

#include "common/manager/global_tools.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x823133C
// IDA_NAME: _ZN25stCloseDisjointStoreParamC2Ev
// DEMANGLED: stCloseDisjointStoreParam::stCloseDisjointStoreParam(void)
stCloseDisjointStoreParam::stCloseDisjointStoreParam()
    : ParamBase(), error_code(0) {}

// IDA_EA: 0x82474CE
// IDA_NAME: _ZN25stCloseDisjointStoreParamD2Ev
// IDA_EA: 0x82474FE
// IDA_NAME: _ZN25stCloseDisjointStoreParamD0Ev
// DEMANGLED: stCloseDisjointStoreParam::~stCloseDisjointStoreParam()
stCloseDisjointStoreParam::~stCloseDisjointStoreParam() {}

// IDA_EA: 0x81D02BE
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CloseDisjointStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CloseDisjointStore::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81D02C8
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStore11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_CloseDisjointStore::check_error(CUser *,MSG_BASE &)
int Dispatcher_CloseDisjointStore::check_error(CUser* user, MSG_BASE&) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    if (user->GetCurCharacExpertJobType() != 3) {
        return -2;
    }

    expert_job::CExpertJob* const job = user->GetCurCharacExpertJob();
    if (!job || job->GetType() != 3) {
        return 19;
    }
    return 0;
}

// IDA_EA: 0x81D0366
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CloseDisjointStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CloseDisjointStore::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    stCloseDisjointStoreParam& result =
        static_cast<stCloseDisjointStoreParam&>(param);
    result.error_code = check_error(user, message);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(
            11035,
            "virtual int Dispatcher_CloseDisjointStore::process(CUser*, MSG_BASE&, ParamBase&)",
            result.error_code,
            user->get_acc_id());
    }

    // IDA 0x81D0408 invokes CExpertJob's +0x04 vtable slot: the typed
    // OnCloseMission virtual, with ENUM_CLOSE_MISSION_TYPE(0).
    user->GetCurCharacExpertJob()->OnCloseMission(
        user, static_cast<expert_job::ENUM_CLOSE_MISSION_TYPE>(0));
    return 0;
}

// IDA_EA: 0x81D0412
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStore4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_CloseDisjointStore::send(CUser *,ParamBase &)
void Dispatcher_CloseDisjointStore::send(CUser* user, ParamBase& param) {
    const stCloseDisjointStoreParam& result =
        static_cast<const stCloseDisjointStoreParam&>(param);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CLOSE_DISJOINT_STORE,
                                 static_cast<unsigned char>(result.error_code));
    }
}

// IDA_EA: 0x8245946
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStoreD2Ev
// IDA_EA: 0x8245976
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStoreD0Ev
// DEMANGLED: Dispatcher_CloseDisjointStore::~Dispatcher_CloseDisjointStore()
Dispatcher_CloseDisjointStore::~Dispatcher_CloseDisjointStore() {}

// IDA_EA: 0x8596C24
// IDA_NAME: _ZN29Dispatcher_CloseDisjointStoreC2Ev
// DEMANGLED: Dispatcher_CloseDisjointStore::Dispatcher_CloseDisjointStore(void)
Dispatcher_CloseDisjointStore::Dispatcher_CloseDisjointStore()
    : IPacketDispatcher<MSG_BASE,
                        stCloseDisjointStoreParam,
                        static_cast<ch_state>(3)>() {}
