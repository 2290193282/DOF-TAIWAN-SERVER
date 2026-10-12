#include "old_data/Login/packet_dispatch/blacklist_register_delete_dispatchers.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kRegisterReadSignature[] =
    "virtual int Dispatcher_RegisterToBlackList::read(PacketBuf&, MSG_BASE&)";
const char kDeleteReadSignature[] =
    "virtual int Dispatcher_DeleteToBlackList::read(PacketBuf&, MSG_BASE&)";

// The old binary compares CP949 bytes, not Unicode code points, when its
// environment's server group is 3.  Escape the exact two NUL-terminated
// literals recovered at 0x8BC17D6 and 0x8BC17DD so source encoding cannot
// alter the protocol branch.
const char kBlackListDeniedName0[] = "\xBB\xFE\xBF\xC0\xC0\xAF";
const char kBlackListDeniedName1[] = "\xBB\xFE\xC0\xAF\xB8\xAE";

template <typename PacketType>
void SendBlackListPacket(CUser* user, PacketType& request) {
    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup());
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(server_group)->SendPacket(
        reinterpret_cast<char*>(&request), 0x2c);
    GlobalData::s_guild_proxy_mgr->GetServerProxy(server_group)->SendPacket(
        reinterpret_cast<char*>(&request), 0x2c);
}

}  // namespace

// This concrete template family has one old provider.  CMD121/122 root slot
// registration is intentionally deferred until the complete 606-entry owner
// exists; emitting it here would activate only a partial dispatch table.
// IDA_EA: 0x8255EA0
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8255FF0
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8255FFA
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825608E
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x825D006
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D036
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8598686
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_REGISTER_TO_BLACKLIST26stRegisterToBlackListParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,stRegisterToBlackListParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,
                                 stRegisterToBlackListParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8255CAC
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8255DFC
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8255E06
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8255E9A
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x825CFB8
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825CFE8
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x85986A2
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DELETE_TO_BLACKLIST24stDeleteToBlackListParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,stDeleteToBlackListParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,
                                 stDeleteToBlackListParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x822BB2E
// IDA_NAME: _ZN28Packet_Register_To_BlackListC2Ev
// DEMANGLED: Packet_Register_To_BlackList::Packet_Register_To_BlackList(void)
Packet_Register_To_BlackList::Packet_Register_To_BlackList()
    : header(0x5dcu, 0x2cu) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x822BB70
// IDA_NAME: _ZN26Packet_Delete_To_BlackListC2Ev
// DEMANGLED: Packet_Delete_To_BlackList::Packet_Delete_To_BlackList(void)
Packet_Delete_To_BlackList::Packet_Delete_To_BlackList()
    : header(0x5ddu, 0x2cu) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x859608C
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackListC2Ev
// DEMANGLED: Dispatcher_RegisterToBlackList::Dispatcher_RegisterToBlackList(void)
Dispatcher_RegisterToBlackList::Dispatcher_RegisterToBlackList()
    : IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,
                        stRegisterToBlackListParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245E74
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackListD2Ev
// DEMANGLED: Dispatcher_RegisterToBlackList::~Dispatcher_RegisterToBlackList()
// IDA_EA: 0x8245EA4
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackListD0Ev
// DEMANGLED: Dispatcher_RegisterToBlackList::~Dispatcher_RegisterToBlackList()
Dispatcher_RegisterToBlackList::~Dispatcher_RegisterToBlackList() {}

// IDA_EA: 0x81CDBC6
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RegisterToBlackList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RegisterToBlackList::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_REGISTER_TO_BLACKLIST& request =
        static_cast<MSG_REGISTER_TO_BLACKLIST&>(message);
    if (packet.get_int(request.target_name_length) != 1)
        return LineFunc(9853, kRegisterReadSignature, 0, 0);
    if (packet.get_str(request.target_name,
                       static_cast<int>(sizeof(request.target_name)),
                       request.target_name_length) == 1)
        return 0;
    return LineFunc(9856, kRegisterReadSignature, 0, 0);
}

// IDA_EA: 0x81CDC70
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackList11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RegisterToBlackList::check_error(CUser *,MSG_BASE &)
int Dispatcher_RegisterToBlackList::check_error(CUser* user, MSG_BASE&) {
    return user->get_state() > 2 ? 0 : -1;
}

// IDA_EA: 0x81CDC9A
// IDA_NAME: _ZN30Dispatcher_RegisterToBlackList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RegisterToBlackList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RegisterToBlackList::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    stRegisterToBlackListParam& result =
        static_cast<stRegisterToBlackListParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0)
        return 0;
    if (result.result < 0) {
        return LineFunc(9887,
                        "virtual int Dispatcher_RegisterToBlackList::process(CUser*, MSG_BASE&, ParamBase&)",
                        result.result,
                        user->get_acc_id());
    }

    const MSG_REGISTER_TO_BLACKLIST& request =
        static_cast<const MSG_REGISTER_TO_BLACKLIST&>(message);
    Packet_Register_To_BlackList outgoing;
    outgoing.account_id = user->get_acc_id();
    std::strncpy(outgoing.target_name,
                 request.target_name,
                 request.target_name_length);
    if (G_CEnvironment()->get_server_group() == 3 &&
        (std::strncmp(outgoing.target_name, kBlackListDeniedName0, 0x1du) == 0 ||
         std::strncmp(outgoing.target_name, kBlackListDeniedName1, 0x1du) == 0)) {
        result.result = 0x7fffffff;
        return 0;
    }

    SendBlackListPacket(user, outgoing);
    return 0;
}

// IDA_EA: 0x85960A8
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackListC2Ev
// DEMANGLED: Dispatcher_DeleteToBlackList::Dispatcher_DeleteToBlackList(void)
Dispatcher_DeleteToBlackList::Dispatcher_DeleteToBlackList()
    : IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,
                        stDeleteToBlackListParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245E26
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackListD2Ev
// DEMANGLED: Dispatcher_DeleteToBlackList::~Dispatcher_DeleteToBlackList()
// IDA_EA: 0x8245E56
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackListD0Ev
// DEMANGLED: Dispatcher_DeleteToBlackList::~Dispatcher_DeleteToBlackList()
Dispatcher_DeleteToBlackList::~Dispatcher_DeleteToBlackList() {}

// IDA_EA: 0x81CDE36
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DeleteToBlackList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DeleteToBlackList::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DELETE_TO_BLACKLIST& request =
        static_cast<MSG_DELETE_TO_BLACKLIST&>(message);
    if (packet.get_int(request.target_name_length) != 1)
        return LineFunc(9934, kDeleteReadSignature, 0, 0);
    if (packet.get_str(request.target_name,
                       static_cast<int>(sizeof(request.target_name)),
                       request.target_name_length) == 1)
        return 0;
    return LineFunc(9937, kDeleteReadSignature, 0, 0);
}

// IDA_EA: 0x81CDEE0
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackList11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_DeleteToBlackList::check_error(CUser *,MSG_BASE &)
int Dispatcher_DeleteToBlackList::check_error(CUser* user, MSG_BASE&) {
    return user->get_state() > 2 ? 0 : -1;
}

// IDA_EA: 0x81CDF0A
// IDA_NAME: _ZN28Dispatcher_DeleteToBlackList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DeleteToBlackList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DeleteToBlackList::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    stDeleteToBlackListParam& result =
        static_cast<stDeleteToBlackListParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0)
        return 0;
    if (result.result < 0) {
        return LineFunc(9962,
                        "virtual int Dispatcher_DeleteToBlackList::process(CUser*, MSG_BASE&, ParamBase&)",
                        result.result,
                        user->get_acc_id());
    }

    const MSG_DELETE_TO_BLACKLIST& request =
        static_cast<const MSG_DELETE_TO_BLACKLIST&>(message);
    Packet_Delete_To_BlackList outgoing;
    outgoing.account_id = user->get_acc_id();
    std::strncpy(outgoing.target_name,
                 request.target_name,
                 request.target_name_length);
    SendBlackListPacket(user, outgoing);
    return 0;
}

// IDA_EA: 0x84D0DE0
// IDA_NAME: _ZN37Inter_Monitor_RegisterBlackListResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Monitor_RegisterBlackListResult::dispatch_sig(CUser *,char *,int)
int Inter_Monitor_RegisterBlackListResult::dispatch_sig(CUser* user,
                                                        char* payload,
                                                        int payload_size) {
    (void)payload_size;
    if (!user->get_state())
        return 0;

    const Monitor_BlackList_Result& result =
        *reinterpret_cast<const Monitor_BlackList_Result*>(payload);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REGISITER_TO_BLACKLIST);
    switch (result.result) {
    case 1:
        if (user->registerToBlackList(result.charac_no) == 1) {
            packet.put_byte(1);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_Monitor_RegisterBlackListResult::dispatch_sig(CUser*, char*, int)",
                12176,
                "_BLACK_LIST_ :: false == pUser->registerToBlackList( recv->m_uCharacNo )");
            packet.put_byte(0);
            packet.put_int(74);
        }
        break;
    case 2:
        packet.put_byte(0);
        packet.put_int(74);
        break;
    case 3:
        packet.put_byte(0);
        packet.put_int(76);
        break;
    case 4:
        packet.put_byte(0);
        packet.put_int(78);
        break;
    case 5:
        packet.put_byte(0);
        packet.put_int(90);
        break;
    }
    const int target_name_size = static_cast<int>(std::strlen(result.target_name));
    packet.put_int(target_name_size);
    packet.put_str(result.target_name, target_name_size);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84ED754
// IDA_NAME: _ZN37Inter_Monitor_RegisterBlackListResultC2Ev
// DEMANGLED: Inter_Monitor_RegisterBlackListResult::Inter_Monitor_RegisterBlackListResult(void)
Inter_Monitor_RegisterBlackListResult::Inter_Monitor_RegisterBlackListResult() {}

// IDA_EA: 0x84F98C4
// IDA_NAME: _ZN37Inter_Monitor_RegisterBlackListResultD2Ev
// DEMANGLED: Inter_Monitor_RegisterBlackListResult::~Inter_Monitor_RegisterBlackListResult()
// IDA_EA: 0x84F98F4
// IDA_NAME: _ZN37Inter_Monitor_RegisterBlackListResultD0Ev
// DEMANGLED: Inter_Monitor_RegisterBlackListResult::~Inter_Monitor_RegisterBlackListResult()
Inter_Monitor_RegisterBlackListResult::~Inter_Monitor_RegisterBlackListResult() {}

// IDA_EA: 0x84D1034
// IDA_NAME: _ZN35Inter_Monitor_DeleteBlackListResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Monitor_DeleteBlackListResult::dispatch_sig(CUser *,char *,int)
int Inter_Monitor_DeleteBlackListResult::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int payload_size) {
    (void)payload_size;
    if (!user->get_state())
        return 0;

    const Monitor_BlackList_Result& result =
        *reinterpret_cast<const Monitor_BlackList_Result*>(payload);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_DELETE_TO_BLACKLIST);
    switch (result.result) {
    case 1:
        if (user->deleteToBlackList(result.charac_no) == 1) {
            packet.put_byte(1);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "virtual int Inter_Monitor_DeleteBlackListResult::dispatch_sig(CUser*, char*, int)",
                12247,
                "_BLACK_LIST_ :: false == pUser->deleteToBlackList( recv->m_uCharacNo )");
            packet.put_byte(0);
            packet.put_int(75);
        }
        break;
    case 2:
        packet.put_byte(0);
        packet.put_int(75);
        break;
    case 3:
        packet.put_byte(0);
        packet.put_int(76);
        break;
    }
    const int target_name_size = static_cast<int>(std::strlen(result.target_name));
    packet.put_int(target_name_size);
    packet.put_str(result.target_name, target_name_size);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84ED770
// IDA_NAME: _ZN35Inter_Monitor_DeleteBlackListResultC2Ev
// DEMANGLED: Inter_Monitor_DeleteBlackListResult::Inter_Monitor_DeleteBlackListResult(void)
Inter_Monitor_DeleteBlackListResult::Inter_Monitor_DeleteBlackListResult() {}

// IDA_EA: 0x84F9876
// IDA_NAME: _ZN35Inter_Monitor_DeleteBlackListResultD2Ev
// DEMANGLED: Inter_Monitor_DeleteBlackListResult::~Inter_Monitor_DeleteBlackListResult()
// IDA_EA: 0x84F98A6
// IDA_NAME: _ZN35Inter_Monitor_DeleteBlackListResultD0Ev
// DEMANGLED: Inter_Monitor_DeleteBlackListResult::~Inter_Monitor_DeleteBlackListResult()
Inter_Monitor_DeleteBlackListResult::~Inter_Monitor_DeleteBlackListResult() {}
