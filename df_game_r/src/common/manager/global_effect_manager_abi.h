#ifndef OLDSRV_GLOBAL_GLOBAL_EFFECT_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_GLOBAL_EFFECT_MANAGER_ABI_H_

#include <stdint.h>

#include <vector>

#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/concurrent/timer/timer_sig.h"

class PacketGuard;

// IDA unit: 99_misc_global/stGlobalEffect_t.  The vending-machine output
// path consumes this exact three-word value from CStackableItem +0x280.
struct stGlobalEffect_t {
    uint32_t effect_type;
    float effect_value;
    uint32_t duration_seconds;
};

static_assert(sizeof(stGlobalEffect_t) == 0x0cu,
              "stGlobalEffect_t IDA ABI size must stay 0x0c");

// IDA unit: 99_misc_global/CGlobalEffect.
// IDA_EA: CGlobalEffect dtor/getType/ctor @0x82A7214/@0x84B7FDA/@0x84B800A.
class CGlobalEffect {
public:
    CGlobalEffect(unsigned int type, float value, const char* name,
                  long timeout);
    ~CGlobalEffect();

    int getType() const;

    // IDA_EA: 0x84B7880
    // IDA_NAME: _ZNK13CGlobalEffect14makeNotifyInfoER11PacketGuard
    // DEMANGLED: CGlobalEffect::makeNotifyInfo(PacketGuard &)const
    int makeNotifyInfo(PacketGuard& packet) const;

    // IDA_EA: 0x84B7FE4
    // IDA_NAME: _ZNK13CGlobalEffect8getValueEv
    // DEMANGLED: CGlobalEffect::getValue(void)const
    float getValue() const;

    // IDA_EA: 0x84B7FF8
    // IDA_NAME: _ZNK13CGlobalEffect9isTimeoutEl
    // DEMANGLED: CGlobalEffect::isTimeout(long)const
    bool isTimeout(long current_seconds) const;

    uint32_t type_;
    float value_;
    char name_[0x1e];
    uint8_t reserved_26[2];
    int32_t timeout_;
};

static_assert(sizeof(CGlobalEffect) == 0x2Cu,
              "CGlobalEffect IDA ABI size must stay 0x2C");

// IDA unit: 99_misc_global/CGlobalEffectManager.
// IDA_EA: ctor @0x82A721A, isAffectedEffect @0x84B7D2C.
// The old type lives in the global namespace because C/B raw old-symbol
// references use _ZN20CGlobalEffectManager... and GlobalData::s_*.
class CGlobalEffectManager {
public:
    CGlobalEffectManager();

    // IDA_EA: 0x8234FCA
    // IDA_NAME: _ZNK20CGlobalEffectManager12getEffectNumEv
    // DEMANGLED: CGlobalEffectManager::getEffectNum(void)const
    int getEffectNum() const;

    // IDA_EA: 0x84B7932
    // IDA_NAME: _ZN20CGlobalEffectManager15registNewEffectEjfPKcll
    // DEMANGLED: CGlobalEffectManager::registNewEffect(unsigned int,float,char const*,long,long)
    int registNewEffect(unsigned int effect_type,
                        float effect_value,
                        const char* character_name,
                        long begin_delay_seconds,
                        long duration_seconds);

    // IDA_EA: 0x84B7A94
    // IDA_NAME: _ZN20CGlobalEffectManager12checkTimeoutEjj
    // DEMANGLED: CGlobalEffectManager::checkTimeout(unsigned int,unsigned int)
    void checkTimeout(unsigned int effect_type, unsigned int current_seconds);

    // IDA_EA: 0x84B7C50
    // IDA_NAME: _ZN20CGlobalEffectManager19beginReservedEffectEj
    // DEMANGLED: CGlobalEffectManager::beginReservedEffect(unsigned int)
    // The lone old caller discards EAX.  The emitted body ends in a void
    // notifier and therefore has no reliable value on the non-empty path.
    void beginReservedEffect(unsigned int effect_type);

    // IDA_EA: 0x84B7D2C
    // IDA_NAME: _ZNK20CGlobalEffectManager16isAffectedEffectEj
    // DEMANGLED: CGlobalEffectManager::isAffectedEffect(unsigned int)const
    int isAffectedEffect(unsigned int type) const;

    // IDA_EA: 0x84B7DAA
    // IDA_NAME: _ZNK20CGlobalEffectManager14getEffectValueEj
    // DEMANGLED: CGlobalEffectManager::getEffectValue(unsigned int)const
    float getEffectValue(unsigned int effect_type) const;

    // IDA_EA: 0x84B7E48
    // IDA_NAME: _ZNK20CGlobalEffectManager22notifyGlobalEffectInfoEv
    // DEMANGLED: CGlobalEffectManager::notifyGlobalEffectInfo(void)const
    void notifyGlobalEffectInfo() const;

    // IDA_EA: 0x84B7EC4
    // IDA_NAME: _ZNK20CGlobalEffectManager20makeGlobalEffectInfoER11PacketGuard
    // DEMANGLED: CGlobalEffectManager::makeGlobalEffectInfo(PacketGuard &)const
    int makeGlobalEffectInfo(PacketGuard& packet) const;

private:
    std::vector<CGlobalEffect> active_effects_;
    std::vector<CGlobalEffect> reserved_effects_;

    CGlobalEffectManager(const CGlobalEffectManager&);
    CGlobalEffectManager& operator=(const CGlobalEffectManager&);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CGlobalEffectManager) == 0x18u,
              "CGlobalEffectManager IDA ABI size must stay 0x18");
#endif

// IDA units: 99_misc_global/TimerGlobalEffectItemBegin and
// 99_misc_global/TimerGlobalEffectItemTimeout.  Both are TimerSig-only
// objects; their TimerDispatcher message-table registration is deliberately
// outside this ABI unit.
class TimerGlobalEffectItemBegin : public TimerSig {
public:
    // IDA_EA: 0x863CB7A
    // IDA_NAME: _ZN26TimerGlobalEffectItemBeginC2Ev
    // DEMANGLED: TimerGlobalEffectItemBegin::TimerGlobalEffectItemBegin(void)
    TimerGlobalEffectItemBegin();

    // IDA_EA: 0x863F46E
    // IDA_NAME: _ZN26TimerGlobalEffectItemBeginD2Ev
    // DEMANGLED: TimerGlobalEffectItemBegin::~TimerGlobalEffectItemBegin()
    // IDA_EA: 0x863F49E
    // IDA_NAME: _ZN26TimerGlobalEffectItemBeginD0Ev
    // DEMANGLED: TimerGlobalEffectItemBegin::~TimerGlobalEffectItemBegin()
    virtual ~TimerGlobalEffectItemBegin();

    // IDA_EA: 0x8637E56
    // IDA_NAME: _ZN26TimerGlobalEffectItemBegin12dispatch_sigEiij
    // DEMANGLED: TimerGlobalEffectItemBegin::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int effect_type);

    // IDA_EA: 0x8637E78
    // IDA_NAME: _ZN26TimerGlobalEffectItemBegin11registTimerEjj
    // DEMANGLED: TimerGlobalEffectItemBegin::registTimer(unsigned int,unsigned int)
    static int registTimer(unsigned int effect_type, unsigned int delay_seconds);
};

class TimerGlobalEffectItemTimeout : public TimerSig {
public:
    // IDA_EA: 0x863CB96
    // IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutC2Ev
    // DEMANGLED: TimerGlobalEffectItemTimeout::TimerGlobalEffectItemTimeout(void)
    TimerGlobalEffectItemTimeout();

    // IDA_EA: 0x863F420
    // IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutD2Ev
    // DEMANGLED: TimerGlobalEffectItemTimeout::~TimerGlobalEffectItemTimeout()
    // IDA_EA: 0x863F450
    // IDA_NAME: _ZN28TimerGlobalEffectItemTimeoutD0Ev
    // DEMANGLED: TimerGlobalEffectItemTimeout::~TimerGlobalEffectItemTimeout()
    virtual ~TimerGlobalEffectItemTimeout();

    // IDA_EA: 0x8637EC2
    // IDA_NAME: _ZN28TimerGlobalEffectItemTimeout12dispatch_sigEiij
    // DEMANGLED: TimerGlobalEffectItemTimeout::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int object_id, int param1,
                             unsigned int effect_type);

    // IDA_EA: 0x8637EF6
    // IDA_NAME: _ZN28TimerGlobalEffectItemTimeout11registTimerEjj
    // DEMANGLED: TimerGlobalEffectItemTimeout::registTimer(unsigned int,unsigned int)
    static int registTimer(unsigned int effect_type, unsigned int delay_seconds);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerGlobalEffectItemBegin) == 0x04u,
              "TimerGlobalEffectItemBegin must retain its TimerSig-only ABI");
static_assert(sizeof(TimerGlobalEffectItemTimeout) == 0x04u,
              "TimerGlobalEffectItemTimeout must retain its TimerSig-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_GLOBAL_EFFECT_MANAGER_ABI_H_
