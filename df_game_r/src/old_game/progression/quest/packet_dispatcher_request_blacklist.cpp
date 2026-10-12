#include "old_game/progression/quest/packet_dispatcher_request_blacklist.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x822BBB2
// IDA_NAME: _ZN24Packet_Request_BlackListC2Ev
// DEMANGLED: Packet_Request_BlackList::Packet_Request_BlackList(void)
Packet_Request_BlackList::Packet_Request_BlackList()
    : header(0x5e0u, 0x12u), account_id(0), uid(0) {}

// IDA_EA: 0x81CE03C
// IDA_NAME: _ZN27Dispatcher_RequestBlackList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestBlackList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestBlackList::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81CE046
// IDA_NAME: _ZN27Dispatcher_RequestBlackList11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RequestBlackList::check_error(CUser *,MSG_BASE &)
int Dispatcher_RequestBlackList::check_error(CUser* user, MSG_BASE&) {
    return user->get_state() > 2 ? 0 : -1;
}

// IDA_EA: 0x81CE070
// IDA_NAME: _ZN27Dispatcher_RequestBlackList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestBlackList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestBlackList::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    stRequestBlackListParam& result =
        reinterpret_cast<stRequestBlackListParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(
            10007,
            "virtual int Dispatcher_RequestBlackList::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result,
            user->get_acc_id());
    }

    Packet_Request_BlackList request;
    request.account_id = user->get_acc_id();
    request.uid = user->GetUID();
    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&request), request.header.size);
    return 0;
}

// IDA_EA: 0x8245DD8
// IDA_NAME: _ZN27Dispatcher_RequestBlackListD2Ev
// IDA_EA: 0x8245E08
// IDA_NAME: _ZN27Dispatcher_RequestBlackListD0Ev
// DEMANGLED: Dispatcher_RequestBlackList::~Dispatcher_RequestBlackList()
Dispatcher_RequestBlackList::~Dispatcher_RequestBlackList() {}

// IDA_EA: 0x85960C4
// IDA_NAME: _ZN27Dispatcher_RequestBlackListC2Ev
// DEMANGLED: Dispatcher_RequestBlackList::Dispatcher_RequestBlackList(void)
Dispatcher_RequestBlackList::Dispatcher_RequestBlackList()
    : IPacketDispatcher<MSG_BASE,
                        stRequestBlackListParam,
                        static_cast<ch_state>(3)>() {}
