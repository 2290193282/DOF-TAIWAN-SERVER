#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_MACHINE_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISJOINT_MACHINE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/stRepairDisjointMachineParam.
struct stRepairDisjointMachineParam : public ParamBase {
    // IDA_EA: 0x82312CA
    // IDA_NAME: _ZN28stRepairDisjointMachineParamC2Ev
    stRepairDisjointMachineParam();

    // IDA_EA: 0x82475B8 / 0x82475E8
    // IDA_NAME: _ZN28stRepairDisjointMachineParamD2Ev / D0Ev
    virtual ~stRepairDisjointMachineParam();

    int error_code;  // +0x04
};

// IDA unit: 99_misc_global/stUpgradeDisjointMachineParam.
struct stUpgradeDisjointMachineParam : public ParamBase {
    // IDA_EA: 0x82312F0
    // IDA_NAME: _ZN29stUpgradeDisjointMachineParamC2Ev
    stUpgradeDisjointMachineParam();

    // IDA_EA: 0x824756A / 0x824759A
    // IDA_NAME: _ZN29stUpgradeDisjointMachineParamD2Ev / D0Ev
    virtual ~stUpgradeDisjointMachineParam();

    int error_code;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stRepairDisjointMachineParam) == 0x08u,
              "CMD240 parameter must retain its i386 ABI size");
static_assert(sizeof(stUpgradeDisjointMachineParam) == 0x08u,
              "CMD244 parameter must retain its i386 ABI size");
#endif

// The state-3 generic bodies are emitted once by A.  This is deliberately an
// extern template declaration: C supplies only the two concrete derived
// vtables and must not emit a competing primary-template COMDAT provider.
extern template class IPacketDispatcher<MSG_BASE,
                                        stRepairDisjointMachineParam,
                                        static_cast<ch_state>(3)>;
extern template class IPacketDispatcher<MSG_BASE,
                                        stUpgradeDisjointMachineParam,
                                        static_cast<ch_state>(3)>;

// IDA unit: 13_database_dispatch/Dispatcher_RepairDisjointMachine.
class Dispatcher_RepairDisjointMachine
    : public IPacketDispatcher<MSG_BASE,
                               stRepairDisjointMachineParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596B7C
    // IDA_NAME: _ZN32Dispatcher_RepairDisjointMachineC2Ev
    Dispatcher_RepairDisjointMachine();

    // IDA_EA: 0x8245A30 / 0x8245A60
    // IDA_NAME: _ZN32Dispatcher_RepairDisjointMachineD2Ev / D0Ev
    virtual ~Dispatcher_RepairDisjointMachine();

    // IDA_EA: 0x81CFDD2 / 0x81CFE3C / 0x81CFEF2
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_UpgradeDisjointMachine.
class Dispatcher_UpgradeDisjointMachine
    : public IPacketDispatcher<MSG_BASE,
                               stUpgradeDisjointMachineParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596BEC
    // IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachineC2Ev
    Dispatcher_UpgradeDisjointMachine();

    // IDA_EA: 0x82459E2 / 0x8245A12
    // IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachineD2Ev / D0Ev
    virtual ~Dispatcher_UpgradeDisjointMachine();

    // IDA_EA: 0x81CFF2A / 0x81CFF94 / 0x81D004A
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RepairDisjointMachine) == 0x04u,
              "CMD240 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_UpgradeDisjointMachine) == 0x04u,
              "CMD244 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_MACHINE_H_
