#include "old_game/progression/career/packet_dispatcher_disjoint_machine.h"

#include "common/manager/global_tools.h"
#include "old_game/progression/career/expert_job_disjointer.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x82312CA
// IDA_NAME: _ZN28stRepairDisjointMachineParamC2Ev
// DEMANGLED: stRepairDisjointMachineParam::stRepairDisjointMachineParam(void)
stRepairDisjointMachineParam::stRepairDisjointMachineParam()
    : ParamBase(), error_code(0) {}

// IDA_EA: 0x82475B8
// IDA_NAME: _ZN28stRepairDisjointMachineParamD2Ev
// DEMANGLED: stRepairDisjointMachineParam::~stRepairDisjointMachineParam()
// IDA_EA: 0x82475E8
// IDA_NAME: _ZN28stRepairDisjointMachineParamD0Ev
// DEMANGLED: stRepairDisjointMachineParam::~stRepairDisjointMachineParam()
stRepairDisjointMachineParam::~stRepairDisjointMachineParam() {}

// IDA_EA: 0x82312F0
// IDA_NAME: _ZN29stUpgradeDisjointMachineParamC2Ev
// DEMANGLED: stUpgradeDisjointMachineParam::stUpgradeDisjointMachineParam(void)
stUpgradeDisjointMachineParam::stUpgradeDisjointMachineParam()
    : ParamBase(), error_code(0) {}

// IDA_EA: 0x824756A
// IDA_NAME: _ZN29stUpgradeDisjointMachineParamD2Ev
// DEMANGLED: stUpgradeDisjointMachineParam::~stUpgradeDisjointMachineParam()
// IDA_EA: 0x824759A
// IDA_NAME: _ZN29stUpgradeDisjointMachineParamD0Ev
// DEMANGLED: stUpgradeDisjointMachineParam::~stUpgradeDisjointMachineParam()
stUpgradeDisjointMachineParam::~stUpgradeDisjointMachineParam() {}

// IDA_EA: 0x8596B7C
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachineC2Ev
// DEMANGLED: Dispatcher_RepairDisjointMachine::Dispatcher_RepairDisjointMachine(void)
Dispatcher_RepairDisjointMachine::Dispatcher_RepairDisjointMachine()
    : IPacketDispatcher<MSG_BASE,
                        stRepairDisjointMachineParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245A30
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachineD2Ev
// DEMANGLED: Dispatcher_RepairDisjointMachine::~Dispatcher_RepairDisjointMachine()
// IDA_EA: 0x8245A60
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachineD0Ev
// DEMANGLED: Dispatcher_RepairDisjointMachine::~Dispatcher_RepairDisjointMachine()
Dispatcher_RepairDisjointMachine::~Dispatcher_RepairDisjointMachine() {}

// IDA_EA: 0x81CFDD2
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachine4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RepairDisjointMachine::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RepairDisjointMachine::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81CFDDC
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachine11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RepairDisjointMachine::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RepairDisjointMachine::check_error(CUser* user,
                                                   MSG_BASE&,
                                                   ParamBase&) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    return user->GetCurCharacExpertJobType() == 3 ? 0 : -2;
}

// IDA_EA: 0x81CFE3C
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachine7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RepairDisjointMachine::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RepairDisjointMachine::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    stRepairDisjointMachineParam& result =
        static_cast<stRepairDisjointMachineParam&>(param);
    result.error_code = check_error(user, message, param);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(
            10836,
            "virtual int Dispatcher_RepairDisjointMachine::process(CUser*, MSG_BASE&, ParamBase&)",
            result.error_code,
            user->get_acc_id());
    }

    const int error = expert_job::CDisjointer::OnRepairDisjointMachine(user);
    if (error) {
        result.error_code = error;
    }
    return 0;
}

// IDA_EA: 0x81CFEF2
// IDA_NAME: _ZN32Dispatcher_RepairDisjointMachine4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RepairDisjointMachine::send(CUser *,ParamBase &)
void Dispatcher_RepairDisjointMachine::send(CUser* user, ParamBase& param) {
    const stRepairDisjointMachineParam& result =
        static_cast<const stRepairDisjointMachineParam&>(param);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REPAIR_DISJOINT_MACHINE,
                                 static_cast<unsigned char>(result.error_code));
    }
}

// IDA_EA: 0x8596BEC
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachineC2Ev
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::Dispatcher_UpgradeDisjointMachine(void)
Dispatcher_UpgradeDisjointMachine::Dispatcher_UpgradeDisjointMachine()
    : IPacketDispatcher<MSG_BASE,
                        stUpgradeDisjointMachineParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82459E2
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachineD2Ev
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::~Dispatcher_UpgradeDisjointMachine()
// IDA_EA: 0x8245A12
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachineD0Ev
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::~Dispatcher_UpgradeDisjointMachine()
Dispatcher_UpgradeDisjointMachine::~Dispatcher_UpgradeDisjointMachine() {}

// IDA_EA: 0x81CFF2A
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachine4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeDisjointMachine::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81CFF34
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachine11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeDisjointMachine::check_error(CUser* user,
                                                    MSG_BASE&,
                                                    ParamBase&) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    return user->GetCurCharacExpertJobType() == 3 ? 0 : -2;
}

// IDA_EA: 0x81CFF94
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachine7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeDisjointMachine::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    stUpgradeDisjointMachineParam& result =
        static_cast<stUpgradeDisjointMachineParam&>(param);
    result.error_code = check_error(user, message, param);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(
            10889,
            "virtual int Dispatcher_UpgradeDisjointMachine::process(CUser*, MSG_BASE&, ParamBase&)",
            result.error_code,
            user->get_acc_id());
    }

    const int error = expert_job::CDisjointer::OnUpgradeDisjointMachine(user);
    if (error) {
        result.error_code = error;
    }
    return 0;
}

// IDA_EA: 0x81D004A
// IDA_NAME: _ZN33Dispatcher_UpgradeDisjointMachine4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_UpgradeDisjointMachine::send(CUser *,ParamBase &)
void Dispatcher_UpgradeDisjointMachine::send(CUser* user, ParamBase& param) {
    const stUpgradeDisjointMachineParam& result =
        static_cast<const stUpgradeDisjointMachineParam&>(param);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_UPGRADE_DISJOINT_MACHINE,
                                 static_cast<unsigned char>(result.error_code));
    }
}
