#include "common/manager/school_server_proxy_abi.h"

#include "infra/log/format/trace.h"


// IDA_EA: 0x8471E24
// IDA_NAME: _ZN18CSchoolServerProxyC2ESsi
// DEMANGLED: CSchoolServerProxy::CSchoolServerProxy(std::string,int)
CSchoolServerProxy::CSchoolServerProxy(std::string school_server_ip,
                                       int school_server_port)
    : school_server_ip_(school_server_ip),
      school_server_port_(school_server_port),
      udp_handler_() {}

// IDA_EA: 0x8471E7A
// IDA_NAME: _ZN18CSchoolServerProxy4InitEv
// DEMANGLED: CSchoolServerProxy::Init(void)
bool CSchoolServerProxy::Init() {
    if (udp_handler_.InitClientSocket() != -1) {
        return true;
    }

    LogManager::logFormat(1,
                          "DF_SchoolServerProxy.cpp",
                          "bool CSchoolServerProxy::Init()",
                          0x3C,
                          "[CSchoolServerProxy::Init] Can't init udp client socket\n");
    return false;
}

// IDA_EA: 0x8471ED2
// IDA_NAME: _ZN18CSchoolServerProxy10SendPacketEPci
// DEMANGLED: CSchoolServerProxy::SendPacket(char *,int)
int CSchoolServerProxy::SendPacket(char* buffer, int bytes) {
    return udp_handler_.SendToServer(
        buffer,
        static_cast<size_t>(bytes),
        static_cast<uint16_t>(school_server_port_),
        school_server_ip_.c_str());
}
