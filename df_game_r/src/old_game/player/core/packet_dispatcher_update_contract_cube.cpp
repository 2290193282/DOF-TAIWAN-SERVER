#include "old_game/player/core/packet_dispatcher_update_contract_cube.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_UpdateContractOfCubeInfo::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x826044E
// IDA_NAME: _ZN32MSG_UPDATE_CONTRACT_OF_CUBE_INFOC2Ev
// DEMANGLED: MSG_UPDATE_CONTRACT_OF_CUBE_INFO::MSG_UPDATE_CONTRACT_OF_CUBE_INFO(void)
MSG_UPDATE_CONTRACT_OF_CUBE_INFO::MSG_UPDATE_CONTRACT_OF_CUBE_INFO()
    : MSG_BASE(), enabled(0), premium_type(0) {}

// IDA_EA: 0x8260546
// IDA_NAME: _ZN26stUpdateContractOfCubeInfoC2Ev
// DEMANGLED: stUpdateContractOfCubeInfo::stUpdateContractOfCubeInfo(void)
stUpdateContractOfCubeInfo::stUpdateContractOfCubeInfo()
    : ParamBase(), error_code(0), enabled(0), premium_type(0) {}

// IDA_EA: 0x8260C62
// IDA_NAME: _ZN26stUpdateContractOfCubeInfoD2Ev
// IDA_EA: 0x8260C92
// IDA_NAME: _ZN26stUpdateContractOfCubeInfoD0Ev
// DEMANGLED: stUpdateContractOfCubeInfo::~stUpdateContractOfCubeInfo()
stUpdateContractOfCubeInfo::~stUpdateContractOfCubeInfo() {}

// IDA_EA: 0x825F868
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpdateContractOfCubeInfo::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    MSG_UPDATE_CONTRACT_OF_CUBE_INFO& request =
        static_cast<MSG_UPDATE_CONTRACT_OF_CUBE_INFO&>(message);
    if (packet.get_byte(request.enabled) != 1) {
        return LineFunc(2398, kReadSignature, 0, 0);
    }
    if (packet.get_byte(request.premium_type) != 1) {
        return LineFunc(2401, kReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x825F900
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpdateContractOfCubeInfo::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    const MSG_UPDATE_CONTRACT_OF_CUBE_INFO& request =
        static_cast<const MSG_UPDATE_CONTRACT_OF_CUBE_INFO&>(message);
    stUpdateContractOfCubeInfo& result =
        static_cast<stUpdateContractOfCubeInfo&>(param);
    result.enabled = request.enabled;
    result.premium_type = request.premium_type;
    result.error_code = check_error(user, message);
    if (result.error_code == 0) {
        user->setConctractOfCubePremiumInfo(result.enabled != 0,
                                             result.premium_type);
        user->history_log().SelectCubeForPremiumService(result.premium_type);
    }
    return 0;
}

// IDA_EA: 0x825F9AE
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::send(CUser *,ParamBase &)
void Dispatcher_UpdateContractOfCubeInfo::send(CUser* user, ParamBase& param) {
    const stUpdateContractOfCubeInfo& result =
        static_cast<const stUpdateContractOfCubeInfo&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_UPDATE_CONTRACT_OF_CUBE_INFO);
    packet.put_byte(result.error_code == 0);
    packet.put_byte(result.enabled);
    packet.put_byte(result.premium_type);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x825FA8E
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::check_error(CUser *,MSG_BASE &)
int Dispatcher_UpdateContractOfCubeInfo::check_error(CUser* user, MSG_BASE&) {
    return user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(92)) ? 0 : 8;
}

// IDA_EA: 0x82609A4
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfoD2Ev
// IDA_EA: 0x82609D4
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfoD0Ev
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::~Dispatcher_UpdateContractOfCubeInfo()
Dispatcher_UpdateContractOfCubeInfo::~Dispatcher_UpdateContractOfCubeInfo() {}

// IDA_EA: 0x8597EFA
// IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfoC2Ev
// DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::Dispatcher_UpdateContractOfCubeInfo(void)
Dispatcher_UpdateContractOfCubeInfo::Dispatcher_UpdateContractOfCubeInfo()
    : IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,
                        stUpdateContractOfCubeInfo,
                        static_cast<ch_state>(0)>() {}
