#include "common/manager/global_effect_manager_abi.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>

CGlobalEffectManager* GlobalData::s_globalEffectManager_ = 0;

// IDA_EA: 0x84B800A
// IDA_NAME: _ZN13CGlobalEffectC2EjfPKcl
// DEMANGLED: CGlobalEffect::CGlobalEffect(unsigned int,float,char const*,long)
CGlobalEffect::CGlobalEffect(unsigned int type,
                             float value,
                             const char* name,
                             long timeout)
    : type_(type),
      value_(value),
      timeout_(timeout) {
    std::strncpy(name_, name, sizeof(name_));
}

// IDA_EA: 0x82A7214
// IDA_NAME: _ZN13CGlobalEffectD2Ev
// DEMANGLED: CGlobalEffect::~CGlobalEffect()
CGlobalEffect::~CGlobalEffect() {}

// IDA_EA: 0x84B7FDA
// IDA_NAME: _ZNK13CGlobalEffect7getTypeEv
// DEMANGLED: CGlobalEffect::getType(void)const
int CGlobalEffect::getType() const {
    return static_cast<int>(type_);
}

// IDA_EA: 0x84B7880
// IDA_NAME: _ZNK13CGlobalEffect14makeNotifyInfoER11PacketGuard
// DEMANGLED: CGlobalEffect::makeNotifyInfo(PacketGuard &)const
int CGlobalEffect::makeNotifyInfo(PacketGuard& packet) const {
    packet.put_int(static_cast<int>(type_));
    packet.put_int(static_cast<int>(value_ * 10000.0f));
    const int name_length = static_cast<int>(std::strlen(name_));
    packet.put_int(name_length);
    packet.put_str(name_, name_length);
    return packet.put_int(timeout_);
}

// IDA_EA: 0x84B7FE4
// IDA_NAME: _ZNK13CGlobalEffect8getValueEv
// DEMANGLED: CGlobalEffect::getValue(void)const
float CGlobalEffect::getValue() const {
    return value_;
}

// IDA_EA: 0x84B7FF8
// IDA_NAME: _ZNK13CGlobalEffect9isTimeoutEl
// DEMANGLED: CGlobalEffect::isTimeout(long)const
bool CGlobalEffect::isTimeout(long current_seconds) const {
    return timeout_ <= current_seconds;
}

// IDA_EA: 0x82A721A
// IDA_NAME: _ZN20CGlobalEffectManagerC2Ev
// DEMANGLED: CGlobalEffectManager::CGlobalEffectManager(void)
CGlobalEffectManager::CGlobalEffectManager()
    : active_effects_(),
      reserved_effects_() {}

// IDA_EA: 0x8234FCA
// IDA_NAME: _ZNK20CGlobalEffectManager12getEffectNumEv
// DEMANGLED: CGlobalEffectManager::getEffectNum(void)const
int CGlobalEffectManager::getEffectNum() const {
    const int active_count = static_cast<int>(active_effects_.size());
    return active_count + static_cast<int>(reserved_effects_.size());
}

// IDA_EA: 0x84B7932
// IDA_NAME: _ZN20CGlobalEffectManager15registNewEffectEjfPKcll
// DEMANGLED: CGlobalEffectManager::registNewEffect(unsigned int,float,char const*,long,long)
int CGlobalEffectManager::registNewEffect(unsigned int effect_type,
                                          float effect_value,
                                          const char* character_name,
                                          long begin_delay_seconds,
                                          long duration_seconds) {
    const int affected = isAffectedEffect(effect_type);
    if (affected) {
        return affected;
    }

    const long current_seconds = GlobalData::s_systemTime_.getCurSec();
    if (begin_delay_seconds != 0) {
        reserved_effects_.push_back(CGlobalEffect(
            effect_type,
            effect_value,
            character_name,
            current_seconds + begin_delay_seconds + duration_seconds));
        TimerGlobalEffectItemBegin::registTimer(
            effect_type, static_cast<unsigned int>(begin_delay_seconds));
        return TimerGlobalEffectItemTimeout::registTimer(
            effect_type,
            static_cast<unsigned int>(begin_delay_seconds + duration_seconds));
    }

    active_effects_.push_back(CGlobalEffect(effect_type,
                                             effect_value,
                                             character_name,
                                             current_seconds + duration_seconds));
    notifyGlobalEffectInfo();
    return TimerGlobalEffectItemTimeout::registTimer(
        effect_type, static_cast<unsigned int>(duration_seconds));
}

// IDA_EA: 0x84B7A94
// IDA_NAME: _ZN20CGlobalEffectManager12checkTimeoutEjj
// DEMANGLED: CGlobalEffectManager::checkTimeout(unsigned int,unsigned int)
void CGlobalEffectManager::checkTimeout(unsigned int effect_type,
                                        unsigned int current_seconds) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GLOBAL_EFFECT_INFO);
    packet.put_byte(0);
    int count_index = packet.get_index();
    int timeout_count = 0;
    packet.put_int(0);

    for (std::vector<CGlobalEffect>::iterator it = active_effects_.begin();
         it != active_effects_.end();) {
        if (it->getType() == static_cast<int>(effect_type) &&
            it->isTimeout(static_cast<long>(current_seconds))) {
            it->makeNotifyInfo(packet);
            ++timeout_count;
            it = active_effects_.erase(it);
        } else {
            ++it;
        }
    }

    packet.put_int(count_index, timeout_count);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x84B7C50
// IDA_NAME: _ZN20CGlobalEffectManager19beginReservedEffectEj
// DEMANGLED: CGlobalEffectManager::beginReservedEffect(unsigned int)
void CGlobalEffectManager::beginReservedEffect(unsigned int effect_type) {
    if (reserved_effects_.empty()) {
        return;
    }

    for (std::vector<CGlobalEffect>::iterator it = reserved_effects_.begin();
         it != reserved_effects_.end();) {
        if (it->getType() == static_cast<int>(effect_type)) {
            active_effects_.push_back(*it);
            it = reserved_effects_.erase(it);
        } else {
            ++it;
        }
    }
    notifyGlobalEffectInfo();
}

// IDA_EA: 0x84B7D2C
// IDA_NAME: _ZNK20CGlobalEffectManager16isAffectedEffectEj
// DEMANGLED: CGlobalEffectManager::isAffectedEffect(unsigned int)const
int CGlobalEffectManager::isAffectedEffect(unsigned int type) const {
    for (std::vector<CGlobalEffect>::const_iterator it =
             active_effects_.begin();
         it != active_effects_.end(); ++it) {
        if (it->getType() == static_cast<int>(type)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x84B7DAA
// IDA_NAME: _ZNK20CGlobalEffectManager14getEffectValueEj
// DEMANGLED: CGlobalEffectManager::getEffectValue(unsigned int)const
float CGlobalEffectManager::getEffectValue(unsigned int effect_type) const {
    float result = 0.0f;
    for (std::vector<CGlobalEffect>::const_iterator it =
             active_effects_.begin();
         it != active_effects_.end(); ++it) {
        if (it->getType() == static_cast<int>(effect_type)) {
            result += it->getValue();
        }
    }
    return result;
}

// IDA_EA: 0x84B7E48
// IDA_NAME: _ZNK20CGlobalEffectManager22notifyGlobalEffectInfoEv
// DEMANGLED: CGlobalEffectManager::notifyGlobalEffectInfo(void)const
void CGlobalEffectManager::notifyGlobalEffectInfo() const {
    PacketGuard packet;
    makeGlobalEffectInfo(packet);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x84B7EC4
// IDA_NAME: _ZNK20CGlobalEffectManager20makeGlobalEffectInfoER11PacketGuard
// DEMANGLED: CGlobalEffectManager::makeGlobalEffectInfo(PacketGuard &)const
int CGlobalEffectManager::makeGlobalEffectInfo(PacketGuard& packet) const {
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GLOBAL_EFFECT_INFO);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(active_effects_.size()));

    for (std::vector<CGlobalEffect>::const_iterator it =
             active_effects_.begin();
         it != active_effects_.end(); ++it) {
        it->makeNotifyInfo(packet);
    }
    return 0;
}

// IDA_EA: 0x863CB7A
// IDA_NAME: _ZN26TimerGlobalEffectItemBeginC2Ev
// DEMANGLED: TimerGlobalEffectItemBegin::TimerGlobalEffectItemBegin(void)
TimerGlobalEffectItemBegin::TimerGlobalEffectItemBegin()
    : TimerSig() {}

// IDA_EA: 0x863F46E
// IDA_NAME: _ZN26TimerGlobalEffectItemBeginD2Ev
// DEMANGLED: TimerGlobalEffectItemBegin::~TimerGlobalEffectItemBegin()
// IDA_EA: 0x863F49E
// IDA_NAME: _ZN26TimerGlobalEffectItemBeginD0Ev
// DEMANGLED: TimerGlobalEffectItemBegin::~TimerGlobalEffectItemBegin()
TimerGlobalEffectItemBegin::~TimerGlobalEffectItemBegin() {}

// IDA_EA: 0x8637E56
// IDA_NAME: _ZN26TimerGlobalEffectItemBegin12dispatch_sigEiij
// DEMANGLED: TimerGlobalEffectItemBegin::dispatch_sig(int,int,unsigned int)
int TimerGlobalEffectItemBegin::dispatch_sig(int object_id,
                                             int param1,
                                             unsigned int effect_type) {
    (void)object_id;
    (void)param1;
    GlobalData::s_globalEffectManager_->beginReservedEffect(effect_type);
    return 1;
}

// IDA_EA: 0x8637E78
// IDA_NAME: _ZN26TimerGlobalEffectItemBegin11registTimerEjj
// DEMANGLED: TimerGlobalEffectItemBegin::registTimer(unsigned int,unsigned int)
int TimerGlobalEffectItemBegin::registTimer(unsigned int effect_type,
                                            unsigned int delay_seconds) {
    return G_TimerQueue()->InsertTimer(2,
                                       0,
                                       119,
                                       static_cast<int>(delay_seconds),
                                       effect_type,
                                       0);
}

// IDA_EA: 0x863CB96
// IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutC2Ev
// DEMANGLED: TimerGlobalEffectItemTimeout::TimerGlobalEffectItemTimeout(void)
TimerGlobalEffectItemTimeout::TimerGlobalEffectItemTimeout()
    : TimerSig() {}

// IDA_EA: 0x863F420
// IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutD2Ev
// DEMANGLED: TimerGlobalEffectItemTimeout::~TimerGlobalEffectItemTimeout()
// IDA_EA: 0x863F450
// IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutD0Ev
// DEMANGLED: TimerGlobalEffectItemTimeout::~TimerGlobalEffectItemTimeout()
TimerGlobalEffectItemTimeout::~TimerGlobalEffectItemTimeout() {}

// IDA_EA: 0x8637EC2
// IDA_NAME: _ZN28TimerGlobalEffectItemTimeout12dispatch_sigEiij
// DEMANGLED: TimerGlobalEffectItemTimeout::dispatch_sig(int,int,unsigned int)
int TimerGlobalEffectItemTimeout::dispatch_sig(int object_id,
                                               int param1,
                                               unsigned int effect_type) {
    (void)object_id;
    (void)param1;
    const unsigned int current_seconds =
        static_cast<unsigned int>(GlobalData::s_systemTime_.getCurSec());
    GlobalData::s_globalEffectManager_->checkTimeout(effect_type,
                                                     current_seconds);
    return 1;
}

// IDA_EA: 0x8637EF6
// IDA_NAME: _ZN28TimerGlobalEffectItemTimeout11registTimerEjj
// DEMANGLED: TimerGlobalEffectItemTimeout::registTimer(unsigned int,unsigned int)
int TimerGlobalEffectItemTimeout::registTimer(unsigned int effect_type,
                                              unsigned int delay_seconds) {
    return G_TimerQueue()->InsertTimer(2,
                                       0,
                                       120,
                                       static_cast<int>(delay_seconds),
                                       effect_type,
                                       0);
}
