#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPDATE_CONTRACT_CUBE_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPDATE_CONTRACT_CUBE_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/MSG_UPDATE_CONTRACT_OF_CUBE_INFO.
#pragma pack(push, 1)
struct MSG_UPDATE_CONTRACT_OF_CUBE_INFO : public MSG_BASE {
    // IDA_EA: 0x826044E
    // IDA_NAME: _ZN32MSG_UPDATE_CONTRACT_OF_CUBE_INFOC2Ev
    // DEMANGLED: MSG_UPDATE_CONTRACT_OF_CUBE_INFO::MSG_UPDATE_CONTRACT_OF_CUBE_INFO(void)
    MSG_UPDATE_CONTRACT_OF_CUBE_INFO();

    uint8_t enabled;       // +0x0d
    uint8_t premium_type;  // +0x0e
};
#pragma pack(pop)

static_assert(sizeof(MSG_UPDATE_CONTRACT_OF_CUBE_INFO) == 0x0fu,
              "MSG_UPDATE_CONTRACT_OF_CUBE_INFO must retain its IDA ABI size");
static_assert(offsetof(MSG_UPDATE_CONTRACT_OF_CUBE_INFO, enabled) == 0x0du,
              "MSG_UPDATE_CONTRACT_OF_CUBE_INFO enabled offset changed");
static_assert(offsetof(MSG_UPDATE_CONTRACT_OF_CUBE_INFO, premium_type) == 0x0eu,
              "MSG_UPDATE_CONTRACT_OF_CUBE_INFO premium type offset changed");

// IDA unit: 99_misc_global/stUpdateContractOfCubeInfo.
struct stUpdateContractOfCubeInfo : public ParamBase {
    // IDA_EA: 0x8260546
    // IDA_NAME: _ZN26stUpdateContractOfCubeInfoC2Ev
    // DEMANGLED: stUpdateContractOfCubeInfo::stUpdateContractOfCubeInfo(void)
    stUpdateContractOfCubeInfo();

    // IDA_EA: 0x8260C62 / 0x8260C92
    // IDA_NAME: _ZN26stUpdateContractOfCubeInfoD2Ev / D0Ev
    // DEMANGLED: stUpdateContractOfCubeInfo::~stUpdateContractOfCubeInfo()
    virtual ~stUpdateContractOfCubeInfo();

    int32_t error_code;    // +0x04
    uint8_t enabled;       // +0x08
    uint8_t premium_type;  // +0x09
};

#if defined(__i386__)
static_assert(sizeof(stUpdateContractOfCubeInfo) == 0x0cu,
              "stUpdateContractOfCubeInfo must retain its i386 ABI size");
#endif

// IDA unit: 13_database_dispatch/Dispatcher_UpdateContractOfCubeInfo.
struct Dispatcher_UpdateContractOfCubeInfo
    : public IPacketDispatcher<MSG_UPDATE_CONTRACT_OF_CUBE_INFO,
                               stUpdateContractOfCubeInfo,
                               static_cast<ch_state>(0)> {
    // IDA_EA: 0x8597EFA
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfoC2Ev
    // DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::Dispatcher_UpdateContractOfCubeInfo(void)
    Dispatcher_UpdateContractOfCubeInfo();

    // IDA_EA: 0x82609A4 / 0x82609D4
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfoD2Ev / D0Ev
    // DEMANGLED: Dispatcher_UpdateContractOfCubeInfo::~Dispatcher_UpdateContractOfCubeInfo()
    virtual ~Dispatcher_UpdateContractOfCubeInfo();

    // IDA_EA: 0x825F868
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x825F900
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x825F9AE
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x825FA8E
    // IDA_NAME: _ZN35Dispatcher_UpdateContractOfCubeInfo11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpdateContractOfCubeInfo) == 0x04u,
              "Dispatcher_UpdateContractOfCubeInfo must retain its i386 ABI size");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_UPDATE_CONTRACT_CUBE_H_
