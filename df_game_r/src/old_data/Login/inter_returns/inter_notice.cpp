#include "old_data/Login/inter_returns/inter_notice.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x84C5212
// IDA_NAME: _ZN12Inter_Notice12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Notice::dispatch_sig(CUser *,char *,int)
int Inter_Notice::dispatch_sig(CUser* /*user*/,
                               char* payload,
                               int /*payload_size*/) {
    SIG_INTER_NOTICE& notice = *reinterpret_cast<SIG_INTER_NOTICE*>(payload);
    G_GameWorld()->send_chat_msg(0,
                                 0,
                                 0,
                                 0,
                                 notice.message,
                                 notice.message_length,
                                 0,
                                 0,
                                 static_cast<ENUM_NOTIPACKET>(12));
    return 0;
}

// IDA_EA: 0x84E9F32
// IDA_NAME: _ZN12Inter_NoticeC2Ev
// DEMANGLED: Inter_Notice::Inter_Notice(void)
Inter_Notice::Inter_Notice() : ISigDispatcher() {}

// IDA_EA: 0x84E9F4E / 0x84E9F7E
// IDA_NAME: _ZN12Inter_NoticeD2Ev / D0Ev
// DEMANGLED: Inter_Notice::~Inter_Notice()
Inter_Notice::~Inter_Notice() {}
