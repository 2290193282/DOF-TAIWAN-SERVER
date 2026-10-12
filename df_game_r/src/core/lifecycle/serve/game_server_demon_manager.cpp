#include "core/lifecycle/serve/game_server_demon_manager.h"

#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>


// IDA_EA: 0x80F6330
// IDA_NAME: _ZN22GameServerDemonManagerC2Ev
// DEMANGLED: GameServerDemonManager::GameServerDemonManager(void)
GameServerDemonManager::GameServerDemonManager()
    : prohibit_connect_(false),
      notice_length_(0),
      notice_message_() {
    std::memset(notice_message_, 0, sizeof(notice_message_));
}

// IDA_EA: 0x80F621C
// IDA_NAME: _ZN22GameServerDemonManagerD2Ev
// DEMANGLED: GameServerDemonManager::~GameServerDemonManager()
GameServerDemonManager::~GameServerDemonManager() {}

// IDA_EA: 0x80F6222
// IDA_NAME: _ZN22GameServerDemonManager19SetUserProhibitFlagEb
// DEMANGLED: GameServerDemonManager::SetUserProhibitFlag(bool)
GameServerDemonManager* GameServerDemonManager::SetUserProhibitFlag(
    bool prohibit) {
    prohibit_connect_ = prohibit;
    return this;
}

// IDA_EA: 0x80F623A
// IDA_NAME: _ZN22GameServerDemonManager26Notice10MinShutDownMessageEv
// DEMANGLED: GameServerDemonManager::Notice10MinShutDownMessage(void)
int GameServerDemonManager::Notice10MinShutDownMessage() {
    TimerQueue* queue = G_TimerQueue();
    return queue == 0 ? 0 : queue->InsertTimer(2, 0, 162, 60, 0, 0);
}

// IDA_EA: 0x80F6280
// IDA_NAME: _ZN22GameServerDemonManager16SetNoticeMessageEhPc
// DEMANGLED: GameServerDemonManager::SetNoticeMessage(unsigned char,char *)
void* GameServerDemonManager::SetNoticeMessage(uint8_t length, char* source) {
    notice_length_ = length;
    std::memset(notice_message_, 0, sizeof(notice_message_));
    return std::memcpy(notice_message_, source, notice_length_);
}

// IDA_EA: 0x84ED404
// IDA_NAME: _ZN22GameServerDemonManager22IsProhibitConnectStateEv
// DEMANGLED: GameServerDemonManager::IsProhibitConnectState(void)
int GameServerDemonManager::IsProhibitConnectState() {
    return prohibit_connect_ ? 1 : 0;
}

// IDA_EA: 0x863BFF6
// IDA_NAME: _ZN22GameServerDemonManager16GetNoticeMessageEv
// DEMANGLED: GameServerDemonManager::GetNoticeMessage(void)
char* GameServerDemonManager::GetNoticeMessage() {
    return notice_message_;
}

// IDA_EA: 0x80F61B8
// IDA_NAME: _Z30GameserverDemonManagerInstancev
// DEMANGLED: GameserverDemonManagerInstance(void)
GameServerDemonManager* GameserverDemonManagerInstance() {
    static GameServerDemonManager instance;
    return &instance;
}
