#include "common/manager/shared_server_message_inter_receiver.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/shared_server_message_abi.h"


// IDA_EA: 0x84DAEEE
// IDA_NAME: _ZN23Inter_LoadServerMessage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadServerMessage::dispatch_sig(CUser *,char *,int)
int Inter_LoadServerMessage::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)user;
    (void)payload_size;

    CSharedServerMessageManager* manager =
        G_CGameManager()->GetSharedServerMessageManager();
    if (manager) {
        manager->SetServerMessage(
            reinterpret_cast<SIG_EVENT_SERVER_MESSAGE*>(payload));
    }
    return 0;
}

// IDA_EA: 0x84EE0F4
// IDA_NAME: _ZN23Inter_LoadServerMessageC2Ev
// DEMANGLED: Inter_LoadServerMessage::Inter_LoadServerMessage(void)
Inter_LoadServerMessage::Inter_LoadServerMessage() {}

// IDA_EA: 0x84F7E42
// IDA_NAME: _ZN23Inter_LoadServerMessageD2Ev
// DEMANGLED: Inter_LoadServerMessage::~Inter_LoadServerMessage()
// IDA_EA: 0x84F7E72
// IDA_NAME: _ZN23Inter_LoadServerMessageD0Ev
// DEMANGLED: Inter_LoadServerMessage::~Inter_LoadServerMessage()
// The compiler-emitted deleting destructor invokes this D2 body then delete.
Inter_LoadServerMessage::~Inter_LoadServerMessage() {}
