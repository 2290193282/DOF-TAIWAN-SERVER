#ifndef OLDSRV_CHANNEL_ARAD_SERVER_STATE_H_
#define OLDSRV_CHANNEL_ARAD_SERVER_STATE_H_

#include "old_data/channel/arad_server_state_message.h"
#include "infra/concurrent/thread/mutex.h"

#include <map>

class CUser;

namespace arad {

#pragma pack(push, 1)

class SigServerState {
public:
    class tagData {
    public:
        tagData();

        int notify_category;
        int effect;
        AradServerStateMessage message;
    };

    SigServerState();
    void* init();
    SigServerState* set(bool active,
                        int notify_category,
                        int effect,
                        const AradServerStateMessage& message);

    bool active;
    int count;
    tagData data[20];
};

#pragma pack(pop)

}  // namespace arad

namespace ARAD {

class Arad_ServerStateManager {
public:
    Arad_ServerStateManager();
    ~Arad_ServerStateManager();

    int insertState(arad::SigServerState* state);
    int insertState(int effect, const AradServerStateMessage& message);
    int deleteState(int effect);
    int sendState(CUser* user);
    int checkContinus();
    void loadServerState();
    int attachTimer();
    int insertStateNoSync(int effect, const AradServerStateMessage& message);
    void sendEffectPacket(int effect,
                          const AradServerStateMessage& message,
                          CUser* user);
    void saveServerState(int notify_category,
                         int effect,
                         const AradServerStateMessage& message,
                         bool active);
    int findState(int effect, AradServerStateMessage& message);

private:
    bool active_;
    char reserved_01_[3];
    Mutex mutex_;
    std::map<int, AradServerStateMessage> states_;
};

}  // namespace ARAD

#endif  // OLDSRV_CHANNEL_ARAD_SERVER_STATE_H_
