#ifndef OLDSRV_RUNTIME_EXCHANGE_SERVER_EXCHANGE_SERVER_RUNTIME_H_
#define OLDSRV_RUNTIME_EXCHANGE_SERVER_EXCHANGE_SERVER_RUNTIME_H_

#include <map>
#include <queue>
#include <stdint.h>
#include <time.h>

#include "infra/concurrent/thread/thread_interface.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/transport/epoll_handler.h"
#include "infra/network/transport/network_endpoint.h"

template <typename T, int Count>
class StaticPool;
class Stream;

namespace exchange_server {

class CSession;

enum EXCHANGE_SERVER_TYPE {
    EXCHANGE_SERVER_MASTER = 0,
    EXCHANGE_SERVER_SLAVE = 1,
};

enum CONNECT_TYPE {
    CONNECT_TYPE_LOCAL = 0,
    CONNECT_TYPE_MASTER_TO_SLAVE = 1,
    CONNECT_TYPE_SLAVE_TO_MASTER = 2,
};

// IDA type 2874.  The five slots are evidenced by calls through the original
// vtable at +0, +4, +8, +12 and +16 in CSession/CExchangeServer.
class IExchangeServer {
public:
    // IDA_EA: 0x8493A88
    IExchangeServer();

    virtual int DispatchPacket(CSession* session, char* packet, int bytes) = 0;
    virtual bool UnregisterExchangeServer(int channel_no) = 0;
    virtual CSession* AllocSession() = 0;
    virtual void FreeSession(CSession* session) = 0;
    virtual EXCHANGE_SERVER_TYPE GetServerType() = 0;
};

// IDA type 2868.  CNetwork<95000,450000> is exactly 0x851A0 bytes on i386;
// the event mask and channel number are the two original trailing words.
class CSession : public CNetwork<95000, 450000> {
public:
    // IDA_EA: 0x848FFD0 / 0x8495662
    CSession();
    ~CSession();

    // IDA_EA: 0x848FFB0 / 0x848FFBE
    int GetChannelNo();
    CSession* SetChannelNo(int channel_no);

    // IDA_EA: 0x8490024..0x84908F2
    int GetSocket();
    CSession* SetTriggerSessionEventType(int event_type);
    CSession* AddTriggerSessionEventType(int event_type);
    int GetTriggerSessionEventType();
    bool Parsing(int received_bytes);
    bool OnRecv();
    int OnSend();
    bool OnError();
    CSession* OnAccept();
    bool OnConnect();
    bool Send(PacketGuard& packet);
    static CSession* Listen(int port, const char* ip);
    static CSession* Connect(const char* ip, int port);
    bool Disconnect();

    static IExchangeServer* exchange_server_;

private:
    int trigger_session_event_type_;  // +0x851A0
    int channel_no_;                  // +0x851A4
};

template <typename Session>
class CEpoll : public EpollHandler {
public:
    CEpoll();
    ~CEpoll();

    int WaitForEvent(int timeout);
    bool RegisterSession(Session* session, int event_type);
    bool UnregisterSession(Session* session);
};

template <typename Session, typename Packet,
          template <typename> class EventDriver>
class CSessionManager : public EventDriver<Session> {
public:
    CSessionManager();
    ~CSessionManager();

    int WaitForSessionEvent(int timeout);
    bool TrySend(Session* session, Packet& packet);
    bool TryConnect(const char* ip, int port);
    bool TryListen(int port, const char* ip);
    bool TryDisconnect(Session* session);
};

// IDA type 2869, size 0x10.
struct ExchangeServerInfo {
    ExchangeServerInfo();

    uint32_t ip;
    uint16_t port;
    CONNECT_TYPE connect_type;
    CSession* session;
};

}  // namespace exchange_server

// IDA_EA: 0x849505A..0x8495838.  This is the actual 300-node session pool,
// including the original queue-backed 0x30-byte owner and 0x851B0-byte node.
template <>
class StaticPool<exchange_server::CSession, 300> {
public:
    struct CNode : public exchange_server::CSession {
        CNode();
        ~CNode();

        uint32_t dummy;
        uint8_t reference_count;
    };

    StaticPool();
    ~StaticPool();
    exchange_server::CSession* Acquire();
    exchange_server::CSession* Free(exchange_server::CSession* session);

private:
    CNode* nodes_;
    std::queue<CNode*> free_nodes_;
    int pool_error_;
};

namespace exchange_server {

class CExchangeServer : public IExchangeServer {
public:
    // IDA_EA: 0x8493AE6 / 0x8493B82
    CExchangeServer();
    ~CExchangeServer();

    // IExchangeServer slots.
    virtual int DispatchPacket(CSession* session, char* packet, int bytes);
    virtual bool UnregisterExchangeServer(int channel_no);
    virtual CSession* AllocSession();
    virtual void FreeSession(CSession* session);
    virtual EXCHANGE_SERVER_TYPE GetServerType();

    // IDA_EA: 0x849103C..0x8493856
    void SetServerType(EXCHANGE_SERVER_TYPE type);
    void Reset();
    bool GetNetworkEvent();
    int GetExchangeServerCount(CONNECT_TYPE type);
    void SetMaster();
    void SetSlave();
    bool TrySend(int channel_no, PacketGuard& packet);
    bool AttachSessionInExchangeServer(int channel_no, CSession* session);
    bool RegisterExchangeServer(int channel_no,
                                ExchangeServerInfo& info);
    bool SendRequestCharacInfo(int signal, int uid, int dest_channel,
                               int charac_no, void* payload, int bytes);
    bool SendResponseCharacInfo(int signal, int uid, void* payload,
                                int bytes, CSession* session, int result);
    bool SendResponseCharacInfo(int signal, int uid, void* payload,
                                int bytes);
    bool SendResponseCharacInfo_forchina(int signal, int uid,
                                         void* payload, int bytes);
    bool ReturnCacheCharacter(int signal, int uid, int dest_channel,
                              int charac_no, bool erase_after_read,
                              CSession* response_session);
    bool ForwardRequestCharac_forchina(int signal, int uid,
                                       void* payload, int bytes);
    bool ForwardRequestCharac(int signal, int uid, void* payload,
                              int bytes);
    int OnRecvPacketCharacInfo(CSession* session, PacketGuard& packet);
    int OnRecvPacketCharacInfoRet(CSession* session, PacketGuard& packet);
    int OnRecvPacketServerInfo(CSession* session, PacketGuard& packet);
    int OnRecvPacketServerInfoRet(CSession* session, PacketGuard& packet);
    int OnRecvMessageServerInfo(Stream* message, int signal, int uid);
    int OnRecvMessageCharacInfo(Stream* message, int signal, int uid);
    bool DispatchMessage(Stream* message);
    Stream* GetMessage();
    bool FreeMessage(Stream* message, int signal);
    bool SendExchangeServerInfo();
    bool PreparePassiveSocket();

private:
    uint32_t local_ip_;  // +0x04
    uint16_t local_port_;  // +0x08
    CSessionManager<CSession, PacketGuard, CEpoll> session_manager_;  // +0x0c
    bool master_;  // +0x38
    bool received_server_info_;  // +0x39
    time_t last_server_info_send_;  // +0x3c
    ExchangeServerInfo local_info_;  // +0x40
    std::map<int, ExchangeServerInfo> exchange_servers_;  // +0x50
    EXCHANGE_SERVER_TYPE server_type_;  // +0x68
    StaticPool<CSession, 300> session_pool_;  // +0x6c
};

class CExchangeServerThread : public ThreadInterface {
public:
    // IDA_EA: 0x8493E36 / 0x8493DB6 / 0x8493E18
    CExchangeServerThread();
    virtual ~CExchangeServerThread();
    virtual void dispatch(void* user_data);

private:
    CExchangeServer exchange_server_;  // +0x0c
};

// IDA_EA: 0x848F27B..0x8493E8D.
int GetServerChannelNo();
char* GetServerIP();
int GetServerPORT();
int BeginExchangeServerThread();
int getExchangeServerThreadID();
CExchangeServerThread* GetInstanceExchangeServerThread();

// Called after network/dispatch ingress has joined.  It drains queue 5, then
// joins the sole consumer before DB_Q is allowed to stop.
bool StopExchangeServerThreadAfterDrain();

}  // namespace exchange_server

#if defined(__i386__)
static_assert(sizeof(CNetwork<95000, 450000>) == 0x851A0u,
              "CNetwork<95000,450000> old ABI size must stay 0x851A0");
static_assert(sizeof(exchange_server::CSession) == 0x851A8u,
              "exchange CSession old ABI size must stay 0x851A8");
static_assert(sizeof(StaticPool<exchange_server::CSession, 300>::CNode) ==
                  0x851B0u,
              "exchange session pool node old ABI size must stay 0x851B0");
static_assert(sizeof(StaticPool<exchange_server::CSession, 300>) == 0x30u,
              "exchange session pool old ABI size must stay 0x30");
static_assert(sizeof(exchange_server::ExchangeServerInfo) == 0x10u,
              "ExchangeServerInfo old ABI size must stay 0x10");
static_assert(sizeof(exchange_server::CExchangeServer) == 0x9Cu,
              "CExchangeServer old ABI size must stay 0x9C");
static_assert(sizeof(exchange_server::CExchangeServerThread) == 0xA8u,
              "CExchangeServerThread old ABI size must stay 0xA8");
#endif

#endif  // OLDSRV_RUNTIME_EXCHANGE_SERVER_EXCHANGE_SERVER_RUNTIME_H_
