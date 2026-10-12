#include "old_data/channel/arad_server_state.h"

#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>
#include <map>

namespace {

typedef char SigServerState_Size_Must_Be_0x195[
    (sizeof(arad::SigServerState) == 0x195) ? 1 : -1];
typedef char SigServerStateTagData_Size_Must_Be_0x14[
    (sizeof(arad::SigServerState::tagData) == 0x14) ? 1 : -1];

}  // namespace

namespace arad {

// IDA_EA: 0x818670C
// IDA_NAME: _ZN4arad14SigServerState7tagDataC2Ev
// DEMANGLED: arad::SigServerState::tagData::tagData(void)
SigServerState::tagData::tagData() : message() {}

// IDA_EA: 0x8186722
// IDA_NAME: _ZN4arad14SigServerStateC2Ev
// DEMANGLED: arad::SigServerState::SigServerState(void)
SigServerState::SigServerState() : count(0), data() {
    init();
}

// IDA_EA: 0x818676E
// IDA_NAME: _ZN4arad14SigServerState4initEv
// DEMANGLED: arad::SigServerState::init(void)
void* SigServerState::init() {
    active = false;
    count = 0;
    return std::memset(data, 0, sizeof(data));
}

// IDA_EA: 0x81A9D22
// IDA_NAME: _ZN4arad14SigServerState3setEbiiRK22AradServerStateMessage
// DEMANGLED: arad::SigServerState::set(bool,int,int,AradServerStateMessage const&)
SigServerState* SigServerState::set(bool is_active,
                                    int notify_category,
                                    int effect,
                                    const AradServerStateMessage& message) {
    active = is_active;
    data[count].notify_category = notify_category;
    data[count].effect = effect;
    std::memcpy(&data[count].message, &message, sizeof(message));
    ++count;
    return this;
}

}  // namespace arad

namespace ARAD {

// IDA_EA: 0x81A922C
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManagerC2Ev
// DEMANGLED: ARAD::Arad_ServerStateManager::Arad_ServerStateManager(void)
Arad_ServerStateManager::Arad_ServerStateManager()
    : active_(false), mutex_(), states_() {}

// IDA_EA: 0x81A927E
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManagerD2Ev
// DEMANGLED: ARAD::Arad_ServerStateManager::~Arad_ServerStateManager()
Arad_ServerStateManager::~Arad_ServerStateManager() {}

// IDA_EA: 0x81A92CA
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager11insertStateEPN4arad14SigServerStateE
// DEMANGLED: ARAD::Arad_ServerStateManager::insertState(arad::SigServerState *)
int Arad_ServerStateManager::insertState(arad::SigServerState* state) {
    MutexGuard guard(&mutex_);
    for (int index = 0; state->count > index; ++index) {
        insertStateNoSync(state->data[index].effect, state->data[index].message);
    }
    return 0;
}

// IDA_EA: 0x81A936E
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager11insertStateE23ENUM_GLOBAL_EFFECT_ARADRK22AradServerStateMessage
// DEMANGLED: ARAD::Arad_ServerStateManager::insertState(ENUM_GLOBAL_EFFECT_ARAD,AradServerStateMessage const&)
int Arad_ServerStateManager::insertState(
    int effect,
    const AradServerStateMessage& message) {
    MutexGuard guard(&mutex_);
    return insertStateNoSync(effect, message);
}

// IDA_EA: 0x81A93D4
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager11deleteStateE23ENUM_GLOBAL_EFFECT_ARAD
// DEMANGLED: ARAD::Arad_ServerStateManager::deleteState(ENUM_GLOBAL_EFFECT_ARAD)
int Arad_ServerStateManager::deleteState(int effect) {
    MutexGuard guard(&mutex_);
    AradServerStateMessage message;
    if (findState(effect, message)) {
        message.active = 0;
        sendEffectPacket(effect, message, 0);
        saveServerState(1, effect, message, false);
        cMyTrace trace(
            "void ARAD::Arad_ServerStateManager::deleteState(ENUM_GLOBAL_EFFECT_ARAD)",
            56,
            0);
        trace("[SERVER_STAT] delete server stat. (category:%d, code:%d, end:%u)",
              1,
              effect,
              message.end_sec);
        states_.erase(effect);
    }
    return 0;
}

// IDA_EA: 0x81A9500
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager9sendStateEP5CUser
// DEMANGLED: ARAD::Arad_ServerStateManager::sendState(CUser *)
int Arad_ServerStateManager::sendState(CUser* user) {
    MutexGuard guard(&mutex_);
    for (std::map<int, AradServerStateMessage>::const_iterator it =
             states_.begin();
         it != states_.end();
         ++it) {
        sendEffectPacket(it->first, it->second, user);
    }
    return 0;
}

// IDA_EA: 0x81A95DC
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager13checkContinusEv
// DEMANGLED: ARAD::Arad_ServerStateManager::checkContinus(void)
int Arad_ServerStateManager::checkContinus() {
    MutexGuard guard(&mutex_);
    const unsigned int cur_sec = static_cast<unsigned int>(
        GlobalData::system_time()->getCurSec());
    for (std::map<int, AradServerStateMessage>::iterator it = states_.begin();
         it != states_.end();) {
        if (it->second.end_sec <= cur_sec && it->second.active == 1) {
            it->second.active = 0;
            sendEffectPacket(it->first, it->second, 0);
            saveServerState(1, it->first, it->second, false);
            cMyTrace trace(
                "void ARAD::Arad_ServerStateManager::checkContinus()", 86, 0);
            trace("[SERVER_STAT] delete server stat. (category:%d, code:%d, end:%u)",
                  1,
                  it->first,
                  it->second.end_sec);
            states_.erase(it++);
        } else {
            ++it;
        }
    }
    attachTimer();
    return 0;
}

// IDA_EA: 0x81A97F8
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager15loadServerStateEv
// DEMANGLED: ARAD::Arad_ServerStateManager::loadServerState(void)
void Arad_ServerStateManager::loadServerState() {
    arad::SigServerState state;
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localjapan/Arad_ServerStateManager.cpp", 106);
    CStreamGuard guard(stream, true);
    DISPATCHER::make_internal_stream_jpn(
        guard, kInternalPacketLoadServerState, -1);
    guard->put_binary(&state, sizeof(state));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x81A98D4
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager11attachTimerEv
// DEMANGLED: ARAD::Arad_ServerStateManager::attachTimer(void)
int Arad_ServerStateManager::attachTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 172, 120, 0, 0);
}

// IDA_EA: 0x81A991A
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager17insertStateNoSyncE23ENUM_GLOBAL_EFFECT_ARADRK22AradServerStateMessage
// DEMANGLED: ARAD::Arad_ServerStateManager::insertStateNoSync(ENUM_GLOBAL_EFFECT_ARAD,AradServerStateMessage const&)
int Arad_ServerStateManager::insertStateNoSync(
    int effect,
    const AradServerStateMessage& message) {
    AradServerStateMessage existing;
    const int result = findState(effect, existing);
    if (!result) {
        states_[effect] = message;
        saveServerState(1, effect, message, true);
        sendEffectPacket(effect, message, 0);
        cMyTrace trace(
            "void ARAD::Arad_ServerStateManager::insertStateNoSync(ENUM_GLOBAL_EFFECT_ARAD, const AradServerStateMessage&)",
            136,
            0);
        return trace(
            "[SERVER_STAT] apply server stat. (category:%d, code:%d, end:%u)",
            1,
            effect,
            message.end_sec);
    }
    return result;
}

// IDA_EA: 0x81A9A1C
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager16sendEffectPacketE23ENUM_GLOBAL_EFFECT_ARADRK22AradServerStateMessageP5CUser
// DEMANGLED: ARAD::Arad_ServerStateManager::sendEffectPacket(ENUM_GLOBAL_EFFECT_ARAD,AradServerStateMessage const&,CUser *)
void Arad_ServerStateManager::sendEffectPacket(
    int effect,
    const AradServerStateMessage& message,
    CUser* user) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_SERVER_STATE_MESSAGE);
    packet.put_int(1);
    packet.put_int(effect);
    char message_wire[sizeof(message)];
    std::memcpy(message_wire, &message, sizeof(message_wire));
    packet.put_binary(message_wire, sizeof(message_wire));
    packet.finalize(true);
    if (user) {
        user->Send(packet);
    } else {
        G_GameWorld()->send_all(packet);
    }
}

// IDA_EA: 0x81A9AFA
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager15saveServerStateE26ENUM_NOTIFY_EVENT_CATEGORY23ENUM_GLOBAL_EFFECT_ARADRK22AradServerStateMessageb
// DEMANGLED: ARAD::Arad_ServerStateManager::saveServerState(ENUM_NOTIFY_EVENT_CATEGORY,ENUM_GLOBAL_EFFECT_ARAD,AradServerStateMessage const&,bool)
void Arad_ServerStateManager::saveServerState(
    int notify_category,
    int effect,
    const AradServerStateMessage& message,
    bool is_active) {
    if (G_CEnvironment()->get_channel_no() == 10) {
        arad::SigServerState state;
        state.set(is_active, notify_category, effect, message);
        Stream* stream = GlobalData::stream_pool()->Acquire(
            "localjapan/Arad_ServerStateManager.cpp", 178);
        CStreamGuard guard(stream, true);
        DISPATCHER::make_internal_stream_jpn(
            guard, kInternalPacketSaveServerState, -1);
        guard->put_binary(&state, sizeof(state));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x81A9C2A
// IDA_NAME: _ZN4ARAD23Arad_ServerStateManager9findStateE23ENUM_GLOBAL_EFFECT_ARADR22AradServerStateMessage
// DEMANGLED: ARAD::Arad_ServerStateManager::findState(ENUM_GLOBAL_EFFECT_ARAD,AradServerStateMessage &)
int Arad_ServerStateManager::findState(int effect,
                                       AradServerStateMessage& message) {
    std::map<int, AradServerStateMessage>::const_iterator it =
        states_.find(effect);
    if (it != states_.end()) {
        std::memcpy(&message, &it->second, sizeof(message));
        return 1;
    }
    std::memset(&message, 0, sizeof(message));
    return 0;
}

}  // namespace ARAD
