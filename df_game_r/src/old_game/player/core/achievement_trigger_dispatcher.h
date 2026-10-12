#ifndef OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_TRIGGER_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_TRIGGER_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#pragma pack(push, 1)
struct MSG_ACHIEVEMENT_TRIGGER : MSG_BASE {
    uint32_t achievement_id;
    uint16_t first;
    uint16_t second;
    uint16_t third;
};
#pragma pack(pop)

static_assert(sizeof(MSG_ACHIEVEMENT_TRIGGER) == 0x17u,
              "CMD417 message must retain its 23-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_ACHIEVEMENT_TRIGGER, achievement_id) == 0x0du &&
                  offsetof(MSG_ACHIEVEMENT_TRIGGER, first) == 0x11u &&
                  offsetof(MSG_ACHIEVEMENT_TRIGGER, second) == 0x13u &&
                  offsetof(MSG_ACHIEVEMENT_TRIGGER, third) == 0x15u,
              "CMD417 message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class Dispatcher_AchievementTrigger
    : public IPacketDispatcher<MSG_ACHIEVEMENT_TRIGGER,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859786A
    // IDA_NAME: _ZN29Dispatcher_AchievementTriggerC2Ev
    // DEMANGLED: Dispatcher_AchievementTrigger::Dispatcher_AchievementTrigger(void)
    Dispatcher_AchievementTrigger();

    // IDA_EA: 0x8244BDE
    // IDA_NAME: _ZN29Dispatcher_AchievementTriggerD2Ev
    // DEMANGLED: Dispatcher_AchievementTrigger::~Dispatcher_AchievementTrigger()
    // IDA_EA: 0x8244C0E
    // IDA_NAME: _ZN29Dispatcher_AchievementTriggerD0Ev
    // DEMANGLED: Dispatcher_AchievementTrigger::~Dispatcher_AchievementTrigger()
    virtual ~Dispatcher_AchievementTrigger();

    // IDA_EA: 0x81D7DFC
    // IDA_NAME: _ZN29Dispatcher_AchievementTrigger4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_AchievementTrigger::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D7F1E
    // IDA_NAME: _ZN29Dispatcher_AchievementTrigger7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_AchievementTrigger::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AchievementTrigger) == 0x04u,
              "CMD417 dispatcher must retain its four-byte i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_ACHIEVEMENT_TRIGGER_DISPATCHER_H_
