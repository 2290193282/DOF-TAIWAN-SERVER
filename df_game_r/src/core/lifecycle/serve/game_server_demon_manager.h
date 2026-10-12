#ifndef OLDSRV_PROCESS_GAME_SERVER_DEMON_MANAGER_H_
#define OLDSRV_PROCESS_GAME_SERVER_DEMON_MANAGER_H_

#include <stdint.h>


// IDA: GameServerDemonManager @0x80F621C..0x863BFF6.
// The original object is a compact bootstrap/runtime notice holder:
//   +0x00 prohibit-connect flag
//   +0x01 notice message length
//   +0x02 notice message buffer, 0xFF bytes
class GameServerDemonManager {
public:
    // IDA: GameServerDemonManager::GameServerDemonManager @0x80F6330.
    GameServerDemonManager();

    // IDA: GameServerDemonManager::~GameServerDemonManager @0x80F621C.
    ~GameServerDemonManager();

    // IDA: GameServerDemonManager::SetUserProhibitFlag @0x80F6222.
    GameServerDemonManager* SetUserProhibitFlag(bool prohibit);

    // IDA: GameServerDemonManager::Notice10MinShutDownMessage @0x80F623A.
    int Notice10MinShutDownMessage();

    // IDA: GameServerDemonManager::SetNoticeMessage @0x80F6280.
    void* SetNoticeMessage(uint8_t length, char* source);

    // IDA: GameServerDemonManager::IsProhibitConnectState @0x84ED404.
    int IsProhibitConnectState();

    // IDA: GameServerDemonManager::GetNoticeMessage @0x863BFF6.
    char* GetNoticeMessage();

private:
    bool prohibit_connect_;       // IDA byte at this+0.
    uint8_t notice_length_;       // IDA byte at this+1.
    char notice_message_[0xFF];   // IDA bytes at this+2.
};

// IDA_EA: 0x80F61B8
// IDA_NAME: _Z30GameserverDemonManagerInstancev
// The acceptor and emergency-shutdown paths share this function-local object;
// it must not be substituted with a per-thread flag.
GameServerDemonManager* GameserverDemonManagerInstance();


#endif  // OLDSRV_PROCESS_GAME_SERVER_DEMON_MANAGER_H_
