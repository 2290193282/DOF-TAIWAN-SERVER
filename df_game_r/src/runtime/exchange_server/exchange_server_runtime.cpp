#include "runtime/exchange_server/exchange_server_runtime.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>

#include "common/manager/exchange_server_hit_rate_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_game/player/core/character_cache_data.h"
#include "old_game/player/core/exchange_server_character_cache.h"

namespace {

const int kExchangeQueue = 5;
const uint32_t kSessionNodeDummy = 36641u;
const ENUM_CMDPACKET kPacketServerInfo =
    ENUM_CMDPACKET_EXCHANGE_SERVER_INFO;
const ENUM_CMDPACKET kPacketServerInfoRet =
    ENUM_CMDPACKET_EXCHANGE_SERVER_INFO_RET;
const ENUM_CMDPACKET kPacketCharacInfo =
    ENUM_CMDPACKET_EXCHANGE_SERVER_CHARAC_INFO;
const ENUM_CMDPACKET kPacketCharacInfoRet =
    ENUM_CMDPACKET_EXCHANGE_SERVER_CHARAC_INFO_RET;

// CExchangeServerThread has a frozen 0xA8 i386 layout.  Keep lifecycle state
// out-of-object while retaining one process-wide singleton worker.
std::atomic<bool> g_exchange_stop_after_drain(false);
std::atomic<bool> g_exchange_thread_started(false);

bool IsCharacterSignal(int signal) {
    return (signal >= 7 && signal <= 12) || signal == 17 || signal == 25 ||
           signal == 32 || signal == 179 || signal == 642;
}

Stream* AcquireQueueStream(int bytes) {
    if (bytes <= 4095) {
        return GlobalData::small_stream_pool()->Acquire(
            "ExchangeServer.cpp", 378);
    }
    if (bytes <= 0x3FFF) {
        return GlobalData::stream_pool()->Acquire("ExchangeServer.cpp", 382);
    }
    return GlobalData::big_stream_pool()->Acquire("ExchangeServer.cpp", 386);
}

bool PutQueueMessage(MsgQueueMgr::QUEUE_IDX queue, int signal, int uid,
                     const void* payload, int bytes) {
    Stream* const stream = AcquireQueueStream(bytes + 8);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    guard << signal << uid;
    if (bytes > 0 && payload) {
        guard.put_binary(const_cast<void*>(payload), bytes);
    }
    return GlobalData::s_msgq_mgr->put(queue, guard) == 1;
}

uint32_t ReadUnalignedU32(const char* value) {
    uint32_t result = 0;
    memcpy(&result, value, sizeof(result));
    return result;
}

}  // namespace

namespace exchange_server {

// IDA_EA: 0x8493A88
IExchangeServer::IExchangeServer() {}

IExchangeServer* CSession::exchange_server_ = 0;

// IDA_EA: 0x848FFD0
CSession::CSession()
    : CNetwork<95000, 450000>(),
      trigger_session_event_type_(0),
      channel_no_(0) {
    reserved_0c_ = 1;
}

// IDA_EA: 0x8495662
CSession::~CSession() {}

// IDA_EA: 0x848FFB0
int CSession::GetChannelNo() { return channel_no_; }

// IDA_EA: 0x848FFBE
CSession* CSession::SetChannelNo(int channel_no) {
    channel_no_ = channel_no;
    return this;
}

// IDA_EA: 0x8490024
int CSession::GetSocket() { return get_socket(); }

// IDA_EA: 0x8490038
CSession* CSession::SetTriggerSessionEventType(int event_type) {
    trigger_session_event_type_ = event_type;
    return this;
}

// IDA_EA: 0x849004A
CSession* CSession::AddTriggerSessionEventType(int event_type) {
    trigger_session_event_type_ |= event_type;
    return this;
}

// IDA_EA: 0x8490066
int CSession::GetTriggerSessionEventType() {
    return trigger_session_event_type_;
}

// IDA_EA: 0x8490074
bool CSession::Parsing(int received_bytes) {
    int parsing_length = remain_recv_len_ + received_bytes;
    char* cursor = remain_recv_len_ ? recv_cursor_ - remain_recv_len_
                                    : recv_cursor_;

    while (parsing_length >= static_cast<int>(sizeof(SC_Header))) {
        const uint32_t packet_size = ReadUnalignedU32(cursor + 3);
        if (packet_size <= 12 || packet_size > 0x17318u) {
            fprintf(stderr,
                    "[ExchangeServerNetwork] invalid packet size=%u fd=%d\n",
                    packet_size, GetSocket());
            return false;
        }
        if (parsing_length < static_cast<int>(packet_size)) {
            break;
        }
        if (exchange_server_ &&
            exchange_server_->DispatchPacket(this, cursor, packet_size) != 0) {
            fprintf(stderr,
                    "[ExchangeServerNetwork] DispatchPacket failed fd=%d\n",
                    GetSocket());
        }
        cursor += packet_size;
        parsing_length -= packet_size;
        remain_recv_len_ = 0;
    }

    if (parsing_length > 0) {
        if (parsing_length > 95000) {
            return false;
        }
        memmove(recv_buffer_, cursor, parsing_length);
        remain_recv_len_ = parsing_length;
        recv_cursor_ = recv_buffer_ + parsing_length;
    } else {
        remain_recv_len_ = 0;
        recv_cursor_ = recv_buffer_;
    }
    return true;
}

// IDA_EA: 0x849030E
bool CSession::OnRecv() {
    const int received = recv_packet();
    if (received <= 0) {
        return received >= 0;
    }
    return Parsing(received);
}

// IDA_EA: 0x8490364
int CSession::OnSend() {
    return send_packet() < 0 ? -1 : GetRemainSendSize();
}

// IDA_EA: 0x84903EE
bool CSession::OnError() { return false; }

// IDA_EA: 0x849045E
CSession* CSession::OnAccept() {
    sockaddr_in address;
    socklen_t address_length = sizeof(address);
    const int accepted = accept(GetSocket(),
                                reinterpret_cast<sockaddr*>(&address),
                                &address_length);
    if (accepted < 0) {
        return 0;
    }
    CSession* const session = exchange_server_ ? exchange_server_->AllocSession()
                                               : 0;
    if (!session) {
        close(accepted);
        return 0;
    }
    session->set_socket(accepted, false);
    return session;
}

// IDA_EA: 0x84905D2
bool CSession::OnConnect() {
    if (!exchange_server_ ||
        exchange_server_->GetServerType() != EXCHANGE_SERVER_SLAVE) {
        return true;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, kPacketServerInfo);
    packet.put_int(GetServerChannelNo());
    packet.put_int(static_cast<int>(inet_addr(GetServerIP())));
    packet.put_short(GetServerPORT());
    packet.finalize(false);
    return Send(packet);
}

// IDA_EA: 0x84906D0
bool CSession::Send(PacketGuard& packet) {
    if (!packet.is_finallized()) {
        return false;
    }
    return send_packet(packet.get_packet(0), packet.get_len()) >= 0;
}

// IDA_EA: 0x8490767
CSession* CSession::Listen(int port, const char* ip) {
    int fd = -1;
    if (!CreateListenSocket(fd, port, ip)) {
        return 0;
    }
    CSession* const session = exchange_server_ ? exchange_server_->AllocSession()
                                               : 0;
    if (!session) {
        close(fd);
        return 0;
    }
    session->set_socket(fd, true);
    return session;
}

// IDA_EA: 0x8490829
CSession* CSession::Connect(const char* ip, int port) {
    int fd = -1;
    if (CreateConnectionSocket(fd, ip, port) != 0) {
        return 0;
    }
    CSession* const session = exchange_server_ ? exchange_server_->AllocSession()
                                               : 0;
    if (!session) {
        close(fd);
        return 0;
    }
    session->set_socket(fd, true);
    return session;
}

// IDA_EA: 0x84908F2
bool CSession::Disconnect() {
    if (exchange_server_) {
        exchange_server_->UnregisterExchangeServer(GetChannelNo());
    }
    disconnect();
    if (exchange_server_) {
        exchange_server_->FreeSession(this);
    }
    return true;
}

template <typename Session>
CEpoll<Session>::CEpoll() : EpollHandler() {}

template <typename Session>
CEpoll<Session>::~CEpoll() {}

// IDA_EA: 0x8495F0A
template <typename Session>
int CEpoll<Session>::WaitForEvent(int timeout) {
    epoll_event* const events = GetEpollEvents();
    const int count = epoll_wait(GetEpollFD(), events, 600, timeout);
    if (count < 0) {
        return errno == EINTR ? 1 : 0;
    }
    for (int index = 0; index < count; ++index) {
        Session* const session = static_cast<Session*>(events[index].data.ptr);
        bool keep = true;
        if (events[index].events & EPOLLIN) {
            if (session->GetTriggerSessionEventType() & 8) {
                Session* const accepted = session->OnAccept();
                if (accepted) {
                    RegisterSession(accepted, 7);
                }
            } else if (session->GetTriggerSessionEventType() & 1) {
                keep = session->OnRecv();
            }
        }
        if (keep && (events[index].events & EPOLLOUT)) {
            if (session->GetTriggerSessionEventType() & 16) {
                keep = session->OnConnect();
                if (keep) {
                    session->SetTriggerSessionEventType(7);
                }
            } else if ((session->GetTriggerSessionEventType() & 2) &&
                       session->OnSend() < 0) {
                keep = false;
            }
        }
        if (events[index].events & (EPOLLERR | EPOLLHUP)) {
            keep = session->OnError();
        }
        if (!keep) {
            UnregisterSession(session);
            session->Disconnect();
        }
    }
    return 1;
}

// IDA_EA: 0x84964CA
template <typename Session>
bool CEpoll<Session>::RegisterSession(Session* session, int event_type) {
    session->SetTriggerSessionEventType(event_type);
    return SetEpoll(session, session->GetSocket(), false) == 0;
}

// IDA_EA: 0x8496D5A
template <typename Session>
bool CEpoll<Session>::UnregisterSession(Session* session) {
    return ResetEpoll(session->GetSocket()) == 0;
}

template class CEpoll<CSession>;

template <typename Session, typename Packet,
          template <typename> class EventDriver>
CSessionManager<Session, Packet, EventDriver>::CSessionManager()
    : EventDriver<Session>() {}

template <typename Session, typename Packet,
          template <typename> class EventDriver>
CSessionManager<Session, Packet, EventDriver>::~CSessionManager() {}

// IDA_EA: 0x84952E0
template <typename Session, typename Packet,
          template <typename> class EventDriver>
int CSessionManager<Session, Packet, EventDriver>::WaitForSessionEvent(
    int timeout) {
    return this->WaitForEvent(timeout);
}

// IDA_EA: 0x84953B2
template <typename Session, typename Packet,
          template <typename> class EventDriver>
bool CSessionManager<Session, Packet, EventDriver>::TrySend(
    Session* session, Packet& packet) {
    if (session && session->Send(packet)) {
        session->AddTriggerSessionEventType(2);
        return true;
    }
    if (session) {
        TryDisconnect(session);
    }
    return false;
}

// IDA_EA: 0x8495506
template <typename Session, typename Packet,
          template <typename> class EventDriver>
bool CSessionManager<Session, Packet, EventDriver>::TryConnect(
    const char* ip, int port) {
    Session* const session = Session::Connect(ip, port);
    if (!session) {
        return false;
    }
    if (!this->RegisterSession(session, 20)) {
        session->Disconnect();
        return false;
    }
    session->AddTriggerSessionEventType(16);
    return true;
}

// IDA_EA: 0x8495570
template <typename Session, typename Packet,
          template <typename> class EventDriver>
bool CSessionManager<Session, Packet, EventDriver>::TryListen(
    int port, const char* ip) {
    Session* const session = Session::Listen(port, ip);
    return session && this->RegisterSession(session, 8);
}

// IDA_EA: 0x8496236
template <typename Session, typename Packet,
          template <typename> class EventDriver>
bool CSessionManager<Session, Packet, EventDriver>::TryDisconnect(
    Session* session) {
    this->UnregisterSession(session);
    return session->Disconnect();
}

template class CSessionManager<CSession, PacketGuard, CEpoll>;

// IDA_EA: 0x8491004
ExchangeServerInfo::ExchangeServerInfo()
    : ip(0), port(0), connect_type(CONNECT_TYPE_LOCAL), session(0) {}

}  // namespace exchange_server

// IDA_EA: 0x8496660
StaticPool<exchange_server::CSession, 300>::CNode::CNode()
    : exchange_server::CSession(),
      dummy(kSessionNodeDummy),
      reference_count(0) {}

StaticPool<exchange_server::CSession, 300>::CNode::~CNode() {}

// IDA_EA: 0x849568A
StaticPool<exchange_server::CSession, 300>::StaticPool()
    : nodes_(new CNode[300]), free_nodes_(), pool_error_(0) {
    for (int index = 0; index < 300; ++index) {
        free_nodes_.push(&nodes_[index]);
    }
}

// IDA_EA: 0x8495838
StaticPool<exchange_server::CSession, 300>::~StaticPool() { delete[] nodes_; }

// IDA_EA: 0x849505A
exchange_server::CSession*
StaticPool<exchange_server::CSession, 300>::Acquire() {
    if (free_nodes_.empty()) {
        return 0;
    }
    CNode* const node = free_nodes_.front();
    free_nodes_.pop();
    if (node->dummy != kSessionNodeDummy) {
        pool_error_ = 3;
    }
    if (node->reference_count != 0) {
        pool_error_ = 4;
    }
    ++node->reference_count;
    return node;
}

// IDA_EA: 0x8495188
exchange_server::CSession*
StaticPool<exchange_server::CSession, 300>::Free(
    exchange_server::CSession* session) {
    if (!session) {
        return 0;
    }
    CNode* const node = static_cast<CNode*>(session);
    free_nodes_.push(node);
    if (node->dummy != kSessionNodeDummy) {
        pool_error_ = 5;
    }
    if (node->reference_count != 1) {
        pool_error_ = 6;
    }
    --node->reference_count;
    return node;
}

namespace exchange_server {

// IDA_EA: 0x8493AE6
CExchangeServer::CExchangeServer()
    : IExchangeServer(),
      local_ip_(0),
      local_port_(0),
      session_manager_(),
      master_(false),
      received_server_info_(false),
      last_server_info_send_(0),
      local_info_(),
      exchange_servers_(),
      server_type_(EXCHANGE_SERVER_MASTER),
      session_pool_() {
    Reset();
}

// IDA_EA: 0x8493B82
CExchangeServer::~CExchangeServer() {}

// IDA_EA: 0x8491030
EXCHANGE_SERVER_TYPE CExchangeServer::GetServerType() { return server_type_; }

// IDA_EA: 0x849103C
void CExchangeServer::SetServerType(EXCHANGE_SERVER_TYPE type) {
    server_type_ = type;
}

// IDA_EA: 0x849104A
CSession* CExchangeServer::AllocSession() { return session_pool_.Acquire(); }

// IDA_EA: 0x8491060
void CExchangeServer::FreeSession(CSession* session) {
    session_pool_.Free(session);
}

// IDA_EA: 0x849107E
void CExchangeServer::Reset() {
    master_ = false;
    received_server_info_ = false;
    last_server_info_send_ = time(0);
}

// IDA_EA: 0x84910A6
bool CExchangeServer::GetNetworkEvent() {
    return session_manager_.WaitForSessionEvent(10) == 1;
}

// IDA_EA: 0x84910CA
int CExchangeServer::GetExchangeServerCount(CONNECT_TYPE type) {
    int count = 0;
    for (std::map<int, ExchangeServerInfo>::const_iterator it =
             exchange_servers_.begin();
         it != exchange_servers_.end(); ++it) {
        if (it->second.connect_type == type) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x849114C
void CExchangeServer::SetMaster() {
    SetServerType(EXCHANGE_SERVER_MASTER);
    master_ = true;
}

// IDA_EA: 0x84911A4
void CExchangeServer::SetSlave() {
    SetServerType(EXCHANGE_SERVER_SLAVE);
    master_ = false;
}

// IDA_EA: 0x84911FC
bool CExchangeServer::TrySend(int channel_no, PacketGuard& packet) {
    std::map<int, ExchangeServerInfo>::iterator found =
        exchange_servers_.find(channel_no);
    return found != exchange_servers_.end() && found->second.session &&
           session_manager_.TrySend(found->second.session, packet);
}

// IDA_EA: 0x8491296
bool CExchangeServer::AttachSessionInExchangeServer(int channel_no,
                                                    CSession* session) {
    std::map<int, ExchangeServerInfo>::iterator found =
        exchange_servers_.find(channel_no);
    if (found == exchange_servers_.end()) {
        return false;
    }
    session->SetChannelNo(channel_no);
    found->second.session = session;
    return true;
}

// IDA_EA: 0x849135C
bool CExchangeServer::RegisterExchangeServer(
    int channel_no, ExchangeServerInfo& info) {
    return exchange_servers_.insert(std::make_pair(channel_no, info)).second;
}

// IDA_EA: 0x849144C
bool CExchangeServer::UnregisterExchangeServer(int channel_no) {
    std::map<int, ExchangeServerInfo>::iterator found =
        exchange_servers_.find(channel_no);
    if (found == exchange_servers_.end()) {
        return false;
    }
    exchange_servers_.erase(found);
    if (GetExchangeServerCount(CONNECT_TYPE_MASTER_TO_SLAVE) == 0) {
        SetMaster();
    }
    return true;
}

// IDA_EA: 0x849152A
bool CExchangeServer::SendRequestCharacInfo(
    int signal, int uid, int dest_channel, int charac_no,
    void* payload, int bytes) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, kPacketCharacInfo);
    packet.put_int(signal);
    packet.put_int(uid);
    packet.put_int(dest_channel);
    packet.put_int(charac_no);
    packet.put_int(bytes);
    if (bytes > 0 && payload) {
        packet.put_binary(static_cast<char*>(payload), bytes);
    }
    packet.finalize(false);
    return TrySend(dest_channel, packet);
}

// IDA_EA: 0x8491644
bool CExchangeServer::SendResponseCharacInfo(
    int signal, int uid, void* payload, int bytes, CSession* session,
    int result) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, kPacketCharacInfoRet);
    packet.put_int(result);
    packet.put_int(signal);
    packet.put_int(uid);
    packet.put_int(bytes);
    if (bytes > 0 && payload) {
        packet.put_binary(static_cast<char*>(payload), bytes);
    }
    packet.finalize(false);
    return session_manager_.TrySend(session, packet);
}

// IDA_EA: 0x849173E
bool CExchangeServer::SendResponseCharacInfo(
    int signal, int uid, void* payload, int bytes) {
    return SendResponseCharacInfo_forchina(signal, uid, payload, bytes);
}

// IDA_EA: 0x849176E
bool CExchangeServer::SendResponseCharacInfo_forchina(
    int signal, int uid, void* payload, int bytes) {
    return PutQueueMessage(MsgQueueMgr::DISPATCH_Q, signal, uid, payload,
                           bytes);
}

// IDA_EA: 0x84918D8
bool CExchangeServer::ReturnCacheCharacter(
    int signal, int uid, int dest_channel, int charac_no,
    bool erase_after_read, CSession* response_session) {
    if (GetServerChannelNo() != dest_channel) {
        return false;
    }

    CACHE_CHARACTER_TYPE* character = 0;
    CCacheCharacterMgr* const cache = GetInstanceCacheCharacterMgr();
    if (!cache->GetCacheCharacter(charac_no, &character, erase_after_read)) {
        return false;
    }

    int bytes = 0;
    bool loaded = false;
    uint8_t* const member = character->GetMemberPosition(
        static_cast<ENUM_INTERNALPACKET>(signal), &bytes, loaded);
    bool sent = false;
    if (member && loaded) {
        sent = response_session
                   ? SendResponseCharacInfo(signal, uid, member, bytes,
                                            response_session, 0)
                   : SendResponseCharacInfo(signal, uid, member, bytes);
    }
    if (erase_after_read) {
        cache->FreeCharacter(character);
    }
    return sent;
}

// IDA_EA: 0x8491AB6
bool CExchangeServer::ForwardRequestCharac_forchina(
    int signal, int uid, void* payload, int bytes) {
    if (signal == 32) {
        GetInstanceExchangeServerHitRate()->IncrementCacheLoadFailCount();
    }
    return PutQueueMessage(MsgQueueMgr::DB_Q, signal, uid, payload, bytes);
}

// IDA_EA: 0x8491C32
bool CExchangeServer::ForwardRequestCharac(
    int signal, int uid, void* payload, int bytes) {
    return ForwardRequestCharac_forchina(signal, uid, payload, bytes);
}

// IDA_EA: 0x84922EE
int CExchangeServer::OnRecvPacketCharacInfo(CSession* session,
                                            PacketGuard& packet) {
    int signal = 0;
    int uid = 0;
    int dest_channel = 0;
    int charac_no = 0;
    if (!packet.get_int(signal)) return 565;
    if (!packet.get_int(uid)) return 566;
    if (!packet.get_int(dest_channel)) return 567;
    if (!packet.get_int(charac_no)) return 568;

    CACHE_CHARACTER_TYPE request;
    int member_size = 0;
    bool loaded = false;
    uint8_t* const member = request.GetMemberPosition(
        static_cast<ENUM_INTERNALPACKET>(signal), &member_size, loaded);
    int payload_size = 0;
    if (member) {
        if (!packet.get_int(payload_size)) return 577;
        if (payload_size < 0 || payload_size > member_size ||
            !packet.get_binary(reinterpret_cast<char*>(member), payload_size)) {
            return 589;
        }
    }

    const bool erase_after_read = signal == 32;
    if (!ReturnCacheCharacter(signal, uid, dest_channel, charac_no,
                              erase_after_read, session)) {
        SendResponseCharacInfo(signal, uid, member, member_size, session, 21);
    }
    return 0;
}

// IDA_EA: 0x84925E8
int CExchangeServer::OnRecvPacketCharacInfoRet(CSession*,
                                               PacketGuard& packet) {
    int result = 0;
    int signal = 0;
    int uid = 0;
    if (!packet.get_int(result)) return 620;
    if (!packet.get_int(signal)) return 621;
    if (!packet.get_int(uid)) return 622;

    CACHE_CHARACTER_TYPE response;
    int member_size = 0;
    bool loaded = false;
    uint8_t* const member = response.GetMemberPosition(
        static_cast<ENUM_INTERNALPACKET>(signal), &member_size, loaded);
    int payload_size = 0;
    if (member) {
        if (!packet.get_int(payload_size)) return 634;
        if (payload_size < 0 || payload_size > member_size ||
            !packet.get_binary(reinterpret_cast<char*>(member), payload_size)) {
            return 642;
        }
    }

    if (result != 0 ||
        !SendResponseCharacInfo(signal, uid, member, member_size)) {
        ForwardRequestCharac(signal, uid, member, member_size);
    }
    return 0;
}

// IDA_EA: 0x8492854
int CExchangeServer::OnRecvPacketServerInfo(CSession* session,
                                             PacketGuard& packet) {
    int channel_no = 0;
    int ip = 0;
    short port = 0;
    if (!packet.get_int(channel_no)) return 669;
    if (!packet.get_int(ip)) return 670;
    if (!packet.get_short(port)) return 671;

    ExchangeServerInfo info;
    info.ip = static_cast<uint32_t>(ip);
    info.port = static_cast<uint16_t>(port);
    info.connect_type = CONNECT_TYPE_SLAVE_TO_MASTER;

    PacketGuard response;
    response.clear();
    response.put_header(1, kPacketServerInfoRet);
    response.put_int(GetServerChannelNo());
    if (GetServerType() == EXCHANGE_SERVER_MASTER) {
        response.put_int(exchange_servers_.size());
        for (std::map<int, ExchangeServerInfo>::const_iterator it =
                 exchange_servers_.begin();
             it != exchange_servers_.end(); ++it) {
            response.put_int(it->first);
            response.put_int(it->second.ip);
            response.put_short(it->second.port);
        }
    } else {
        response.put_int(0);
    }
    response.finalize(false);
    session_manager_.TrySend(session, response);

    RegisterExchangeServer(channel_no, info);
    AttachSessionInExchangeServer(channel_no, session);
    return 0;
}

// IDA_EA: 0x8492C5E
int CExchangeServer::OnRecvPacketServerInfoRet(CSession* session,
                                                PacketGuard& packet) {
    int peer_channel = 0;
    int server_count = 0;
    if (!packet.get_int(peer_channel)) return 729;
    if (!packet.get_int(server_count)) return 730;
    for (int index = 0; index < server_count; ++index) {
        int channel_no = 0;
        int ip = 0;
        short port = 0;
        if (!packet.get_int(channel_no)) return 736;
        if (!packet.get_int(ip)) return 737;
        if (!packet.get_short(port)) return 738;

        ExchangeServerInfo info;
        info.ip = static_cast<uint32_t>(ip);
        info.port = static_cast<uint16_t>(port);
        info.connect_type = CONNECT_TYPE_MASTER_TO_SLAVE;
        if ((info.ip != local_ip_ || info.port != local_port_) &&
            exchange_servers_.find(channel_no) == exchange_servers_.end()) {
            in_addr address;
            address.s_addr = info.ip;
            session_manager_.TryConnect(inet_ntoa(address), info.port);
        }
        RegisterExchangeServer(channel_no, info);
    }
    AttachSessionInExchangeServer(peer_channel, session);
    SetSlave();
    return 0;
}

// IDA_EA: 0x8493002
int CExchangeServer::DispatchPacket(CSession* session, char* packet,
                                    int bytes) {
    if (!packet || bytes < static_cast<int>(sizeof(SC_Header))) {
        return 789;
    }
    PacketGuard guard;
    if (!guard.bind_packet(packet, bytes)) {
        return 789;
    }
    guard.set_index(sizeof(SC_Header));
    if (static_cast<unsigned char>(packet[0]) != 1) {
        return 789;
    }
    const uint16_t packet_id =
        static_cast<uint16_t>(static_cast<unsigned char>(packet[1])) |
        static_cast<uint16_t>(static_cast<unsigned char>(packet[2]) << 8);
    switch (packet_id) {
        case kPacketServerInfo:
            return OnRecvPacketServerInfo(session, guard);
        case kPacketServerInfoRet:
            return OnRecvPacketServerInfoRet(session, guard);
        case kPacketCharacInfo:
            return OnRecvPacketCharacInfo(session, guard);
        case kPacketCharacInfoRet:
            return OnRecvPacketCharacInfoRet(session, guard);
        default:
            return 789;
    }
}

// IDA_EA: 0x84931DE
int CExchangeServer::OnRecvMessageServerInfo(Stream* message, int, int) {
    if (message->length() <
        static_cast<int>(sizeof(Packet_Exchange_Server_Info))) {
        return 0;
    }
    const Packet_Exchange_Server_Info* const info =
        reinterpret_cast<const Packet_Exchange_Server_Info*>(
            message->get_buffer());
    received_server_info_ = true;
    local_ip_ = info->exchange_server_ip;
    local_port_ = static_cast<uint16_t>(info->exchange_server_port);
    if (info->channel_no == GetServerChannelNo()) {
        master_ = true;
        SetServerType(EXCHANGE_SERVER_MASTER);
    } else {
        in_addr address;
        address.s_addr = info->exchange_server_ip;
        session_manager_.TryConnect(inet_ntoa(address),
                                    info->exchange_server_port);
        SetServerType(EXCHANGE_SERVER_SLAVE);
    }
    return 0;
}

// IDA_EA: 0x849341C
int CExchangeServer::OnRecvMessageCharacInfo(Stream* message, int signal,
                                             int uid) {
    int dest_channel = 0;
    int charac_no = 0;
    if (!(*message >> dest_channel) || !(*message >> charac_no)) {
        return 0;
    }
    const bool erase_after_read = signal == 32;
    if (ReturnCacheCharacter(signal, uid, dest_channel, charac_no,
                             erase_after_read, 0)) {
        return 0;
    }
    const int bytes = message->length();
    void* const payload = message->get_buffer();
    if (!SendRequestCharacInfo(signal, uid, dest_channel, charac_no,
                               payload, bytes)) {
        // The source belongs to queue 5 and is freed after DispatchMessage;
        // DB fallback therefore always receives a newly allocated stream.
        ForwardRequestCharac(signal, uid, payload, bytes);
    }
    return 0;
}

// IDA_EA: 0x8493530
bool CExchangeServer::DispatchMessage(Stream* message) {
    int signal = 0;
    int uid = 0;
    if (!(*message >> signal) || !(*message >> uid)) {
        return false;
    }
    if (IsCharacterSignal(signal)) {
        OnRecvMessageCharacInfo(message, signal, uid);
        return true;
    }
    if (signal == 184) {
        OnRecvMessageServerInfo(message, signal, uid);
        return true;
    }
    return false;
}

// IDA_EA: 0x849362A
Stream* CExchangeServer::GetMessage() {
    return GlobalData::s_msgq_mgr->get(
        static_cast<MsgQueueMgr::QUEUE_IDX>(kExchangeQueue));
}

// IDA_EA: 0x8493648
bool CExchangeServer::FreeMessage(Stream* message, int) {
    if (!message) {
        return false;
    }
    switch (message->pool_kind()) {
        case 1:
            return GlobalData::small_stream_pool()->Free(message) == 1;
        case 2:
            return GlobalData::big_stream_pool()->Free(message) == 1;
        case 0:
            return GlobalData::stream_pool()->Free(message) == 1;
        default:
            return true;
    }
}

// IDA_EA: 0x84937BA
bool CExchangeServer::SendExchangeServerInfo() {
    if (master_ || !received_server_info_) {
        const time_t now = time(0);
        if (now - last_server_info_send_ > 9) {
            CMonitorServerProxy* const monitor =
                GlobalData::s_monitor_proxy_mgr
                    ? GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                          SERVER_GROUP_DEFAULT)
                    : 0;
            if (monitor) {
                monitor->SendExchangeServerInfo(GetServerChannelNo(),
                                                static_cast<int>(local_ip_),
                                                local_port_);
            }
            last_server_info_send_ = now;
        }
    }
    return true;
}

// IDA_EA: 0x8493856
bool CExchangeServer::PreparePassiveSocket() {
    local_ip_ = inet_addr(GetServerIP());
    local_port_ = static_cast<uint16_t>(GetServerPORT());
    if (!session_manager_.TryListen(local_port_, GetServerIP())) {
        return false;
    }
    local_info_.ip = local_ip_;
    local_info_.port = local_port_;
    local_info_.connect_type = CONNECT_TYPE_LOCAL;
    RegisterExchangeServer(GetServerChannelNo(), local_info_);
    return true;
}

// IDA_EA: 0x8493E36
CExchangeServerThread::CExchangeServerThread()
    : ThreadInterface(), exchange_server_() {
    CSession::exchange_server_ = &exchange_server_;
}

// IDA_EA: 0x8493DB6 / 0x8493E18
CExchangeServerThread::~CExchangeServerThread() {}

// IDA_EA: 0x8493BFE
void CExchangeServerThread::dispatch(void*) {
    if (!exchange_server_.PreparePassiveSocket()) {
        fprintf(stderr, "Exchange Server starting error\n");
        std::exit(-1);
    }
    for (;;) {
        if (!g_exchange_stop_after_drain.load(std::memory_order_acquire)) {
            exchange_server_.SendExchangeServerInfo();
        }
        Stream* const message = exchange_server_.GetMessage();
        if (message) {
            int signal = 0;
            if (message->length() >= static_cast<int>(sizeof(signal))) {
                memcpy(&signal, message->get_buffer(), sizeof(signal));
            }
            exchange_server_.DispatchMessage(message);
            exchange_server_.FreeMessage(message, signal);
        }

        const bool stopping =
            g_exchange_stop_after_drain.load(std::memory_order_acquire);
        MsgQueueMgr* const queues = GlobalData::msgq_mgr();
        if (stopping &&
            (!queues || queues->empty(MsgQueueMgr::EXCHANGE_Q))) {
            break;
        }
        if (!stopping) {
            exchange_server_.GetNetworkEvent();
            usleep(10000);
        }
    }
}

// IDA_EA: 0x848F27B
int GetServerChannelNo() { return G_CEnvironment()->get_channel_no(); }

// IDA_EA: 0x848F28E
char* GetServerIP() {
    return const_cast<char*>(G_CEnvironment()->get_exchange_server_ip());
}

// IDA_EA: 0x848F2A3
int GetServerPORT() {
    return G_CEnvironment()->get_exchange_server_port();
}

// IDA_EA: 0x8493E8D
CExchangeServerThread* GetInstanceExchangeServerThread() {
    static CExchangeServerThread instance;
    return &instance;
}

// IDA_EA: 0x848F2B8
int BeginExchangeServerThread() {
    if (g_exchange_thread_started.load(std::memory_order_acquire)) {
        return 0;
    }
    g_exchange_stop_after_drain.store(false, std::memory_order_release);
    const int result = GetInstanceExchangeServerThread()->begin();
    if (result == 1) {
        g_exchange_thread_started.store(true, std::memory_order_release);
    }
    return result;
}

// IDA_EA: 0x848F2CD
int getExchangeServerThreadID() {
    return GetInstanceExchangeServerThread()->getThreadID();
}

bool StopExchangeServerThreadAfterDrain() {
    if (!g_exchange_thread_started.load(std::memory_order_acquire)) {
        return true;
    }
    g_exchange_stop_after_drain.store(true, std::memory_order_release);
    const bool joined = GetInstanceExchangeServerThread()->join();
    if (joined) {
        g_exchange_thread_started.store(false, std::memory_order_release);
    }
    return joined;
}

}  // namespace exchange_server
