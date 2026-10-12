#include "old_game/activity/schedule/packet_dispatcher_blue_marble_confirm_info.h"

#include <stdint.h>
#include <stdio.h>

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_BlueMarbleConfirmInfo::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_BlueMarbleConfirmInfo::process(CUser*, MSG_BASE&, ParamBase&)";

void WriteConfirmLog(const char* format,
                     CUser* user,
                     int client_index,
                     int server_index,
                     int source_line) {
    char path[1024] = {};
    std::snprintf(path,
                  sizeof(path),
                  "./log/%s/BlueMarbleConfirmInfo",
                  G_CEnvironment()->get_file_name());
    CMyFileLog log(kProcessSignature, source_line);
    log(path,
        format,
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(user->get_charac_no(-1)),
        user->get_state(),
        user->getMoveSpace(),
        client_index,
        server_index);
}

}  // namespace

// IDA_EA: 0x82492D0
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BLUE_MARBLE_CONFIRM_INFO,
//     stBlueMarbleConfirmInfo,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824942C
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8249436
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B1DC / 0x825B20C
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EED2Ev / D0Ev
// IDA_EA: 0x825D594
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8599122
// IDA_NAME: _ZN17IPacketDispatcherI28MSG_BLUE_MARBLE_CONFIRM_INFO23stBlueMarbleConfirmInfoL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_BLUE_MARBLE_CONFIRM_INFO,
                                 stBlueMarbleConfirmInfo,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597DE2
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfoC2Ev
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::Dispatcher_BlueMarbleConfirmInfo(void)
Dispatcher_BlueMarbleConfirmInfo::Dispatcher_BlueMarbleConfirmInfo() {}

// IDA_EA: 0x8243C54
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfoD2Ev
// IDA_EA: 0x8243C84
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfoD0Ev
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::~Dispatcher_BlueMarbleConfirmInfo()
Dispatcher_BlueMarbleConfirmInfo::~Dispatcher_BlueMarbleConfirmInfo() {}

// IDA_EA: 0x81E6242
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BlueMarbleConfirmInfo::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    MSG_BLUE_MARBLE_CONFIRM_INFO& request =
        static_cast<MSG_BLUE_MARBLE_CONFIRM_INFO&>(message);
    if (packet.get_byte(request.user_absent) != 1) {
        return LineFunc(24845, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.blue_marble_index) == 1) {
        return 0;
    }
    return LineFunc(24848, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E68C8
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::check_error(CUser *,MSG_BASE &)
int Dispatcher_BlueMarbleConfirmInfo::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return -1;
    }
    return user->get_state() > 2 ? 0 : -1;
}

// IDA_EA: 0x81E62DA
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BlueMarbleConfirmInfo::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    MSG_BLUE_MARBLE_CONFIRM_INFO& request =
        static_cast<MSG_BLUE_MARBLE_CONFIRM_INFO&>(message);
    stBlueMarbleConfirmInfo& response =
        static_cast<stBlueMarbleConfirmInfo&>(param);
    response.dword_04 = static_cast<uint32_t>(check_error(user, message));
    if (static_cast<int32_t>(response.dword_04) > 0) {
        return 0;
    }
    if (static_cast<int32_t>(response.dword_04) < 0) {
        return LineFunc(24863,
                        kProcessSignature,
                        static_cast<int32_t>(response.dword_04),
                        0);
    }

    const int client_index = request.blue_marble_index;
    const int server_index = static_cast<int16_t>(user->getBlueMarbleIndex());
    WriteConfirmLog(
        "Client m_id(%u) charac_no(%u) userState(%d) userSpace(%d)"
        "client_blueMarble_Index(%d) server_bluemarble_index(%d)",
        user,
        client_index,
        server_index,
        24871);

    if (request.user_absent != 0) {
        WriteConfirmLog(
            "Not Exist User m_id(%u) charac_no(%u) userState(%d) userSpace(%d)"
            "client_blueMarble_Index(%d) server_bluemarble_index(%d)",
            user,
            client_index,
            server_index,
            24920);
        BlueMarble* const own_blue_marble = user->getBlueMarble();
        if (own_blue_marble) {
            own_blue_marble->checkBlueMarbleUserConfirm(user);
        }
        return 0;
    }

    if (static_cast<uint16_t>(request.blue_marble_index) ==
        static_cast<uint16_t>(user->getBlueMarbleIndex())) {
        BlueMarble* const own_blue_marble = user->getBlueMarble();
        if (own_blue_marble) {
            WriteConfirmLog(
                "OverLap User index same m_id(%u) charac_no(%u) userState(%d) userSpace(%d)"
                "client_blueMarble_Index(%d) server_bluemarble_index(%d)",
                user,
                client_index,
                server_index,
                24909);
            own_blue_marble->checkBlueMarbleUserConfirm(user);
        }
        return 0;
    }

    BlueMarble* const requested_blue_marble =
        G_CGameManager()->getBlueMarble(request.blue_marble_index);
    if (requested_blue_marble) {
        WriteConfirmLog(
            "OverLap User index different m_id(%u) charac_no(%u) userState(%d) userSpace(%d)"
            "client_blueMarble_Index(%d) server_bluemarble_index(%d)",
            user,
            client_index,
            server_index,
            24885);
        requested_blue_marble->checkBlueMarbleUserConfirm(user);
    }

    BlueMarble* const own_blue_marble = user->getBlueMarble();
    if (own_blue_marble) {
        WriteConfirmLog(
            "OverLap User index different m_id(%u) charac_no(%u) userState(%d) userSpace(%d)"
            "client_blueMarble_Index(%d) server_bluemarble_index(%d)",
            user,
            client_index,
            server_index,
            24896);
        own_blue_marble->checkBlueMarbleUserConfirm(user);
    }
    return 0;
}

// IDA_EA: 0x81E6904
// IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::send(CUser *,ParamBase &)
void Dispatcher_BlueMarbleConfirmInfo::send(CUser* user, ParamBase& param) {
    const stBlueMarbleConfirmInfo& response =
        static_cast<const stBlueMarbleConfirmInfo&>(param);
    if (response.dword_04 != 0u) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BLUE_MARBLE_CONFIRM_INFO,
            static_cast<unsigned char>(response.dword_04));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_BLUE_MARBLE_CONFIRM_INFO);
}
