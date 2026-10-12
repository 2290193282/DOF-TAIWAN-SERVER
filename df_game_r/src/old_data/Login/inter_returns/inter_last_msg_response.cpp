#include "old_data/Login/inter_returns/inter_last_msg_response.h"

#include "core/lifecycle/shutdown/shutdown_manager.h"

// IDA_EA: 0x84D0598
// IDA_NAME: _ZN21Inter_LastMsgResponse12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LastMsgResponse::dispatch_sig(CUser *,char *,int)
int Inter_LastMsgResponse::dispatch_sig(CUser* /*user*/,
                                        char* /*payload*/,
                                        int /*payload_size*/) {
    CShutdowManager* const shutdown_manager = GetInstanceShutdowManager();
    shutdown_manager->IncrementLastMsgResponseCount();

    if (shutdown_manager->GetLastMsgCount() <=
        shutdown_manager->GetLastMsgResponseCount()) {
        shutdown_manager->OnAllUserLogout();
    }
    return 0;
}

// IDA_EA: 0x84ED6C8
// IDA_NAME: _ZN21Inter_LastMsgResponseC2Ev
// DEMANGLED: Inter_LastMsgResponse::Inter_LastMsgResponse(void)
Inter_LastMsgResponse::Inter_LastMsgResponse() : ISigDispatcher() {}

// IDA_EA: 0x84F9A98 / 0x84F9AC8
// IDA_NAME: _ZN21Inter_LastMsgResponseD2Ev / D0Ev
// DEMANGLED: Inter_LastMsgResponse::~Inter_LastMsgResponse()
Inter_LastMsgResponse::~Inter_LastMsgResponse() {}
