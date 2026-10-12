#include "old_data/Login/inter_returns/inter_tcp_server_connect_return.h"

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"

// IDA_EA: 0x84C8DCA
// IDA_NAME: _ZN22Inter_TcpServerConnect12dispatch_sigEP5CUserPci
int Inter_TcpServerConnect::dispatch_sig(CUser*, char* payload, int) {
    if (payload[10] == static_cast<char>(0xcb)) {
        CServerProxyMgr<CGuildServerProxy>* const manager =
            GlobalData::s_guild_proxy_mgr;
        int iterator = 0;
        for (int server_group = manager->GetStartIndex();
             iterator < manager->GetEndIndex();
             server_group = manager->GetNextIndex(iterator)) {
            manager->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(server_group))->InitTcp();
        }
    } else if (payload[10] == static_cast<char>(0xc9)) {
        CServerProxyMgr<CMonitorServerProxy>* const manager =
            GlobalData::s_monitor_proxy_mgr;
        int iterator = 0;
        for (int server_group = manager->GetStartIndex();
             iterator < manager->GetEndIndex();
             server_group = manager->GetNextIndex(iterator)) {
            manager->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(server_group))->InitTcp();
        }
    }
    return 0;
}

// IDA_EA: 0x84ED4B4
// IDA_NAME: _ZN22Inter_TcpServerConnectC2Ev
// DEMANGLED: Inter_TcpServerConnect::Inter_TcpServerConnect(void)
Inter_TcpServerConnect::Inter_TcpServerConnect() : ISigDispatcher() {}

// IDA_EA: 0x84FA1E8
// IDA_NAME: _ZN22Inter_TcpServerConnectD2Ev
// DEMANGLED: Inter_TcpServerConnect::~Inter_TcpServerConnect()
// IDA_EA: 0x84FA218
// IDA_NAME: _ZN22Inter_TcpServerConnectD0Ev
// DEMANGLED: Inter_TcpServerConnect::~Inter_TcpServerConnect()
Inter_TcpServerConnect::~Inter_TcpServerConnect() {}
