#ifndef OLDSRV_RUNTIME_NETWORK_ENDPOINT_H_
#define OLDSRV_RUNTIME_NETWORK_ENDPOINT_H_

#include <stdint.h>
#include <stddef.h>

#include "infra/concurrent/thread/mutex.h"

class CMonitorServerProxy;
class CUser;
namespace exchange_server {
class CSession;
}

// Callback slot used by the restored CNetwork parsing functions. The original
// server calls CDispatchServerPacket directly; this indirection keeps the
// network layer buildable while the dispatcher class graph is restored.
typedef void (*NetworkPacketDispatch)(const char* packet, int size);

void SetServerPacketDispatch(NetworkPacketDispatch callback);
void SetAuctionPacketDispatch(NetworkPacketDispatch callback);
NetworkPacketDispatch GetServerPacketDispatch();
NetworkPacketDispatch GetAuctionPacketDispatch();

bool NetworkErrnoIsNonBlocking();
bool NetworkSetNonBlock(int fd);
int NetworkCreateConnectionSocket(int& fd, const char* ip, int port,
                                  bool nonblock);
bool NetworkCreateListenSocket(int& fd, int port, const char* ip);
void NetworkLogError(const char* func, const char* text, int fd);
uint16_t NetworkReadU16(const char* src);
uint32_t NetworkReadU32(const char* src);

// Old df_game_r CNetwork<RecvSize, SendSize> layout.
// The member ordering mirrors IDA offsets on the 32-bit CentOS build:
//   +0x00 socket, +0x18 remain recv length, +0x2c recv cursor,
//   +48 recv buffer, +(48+RecvSize) send remain,
//   +(52+RecvSize) send cursor, +(56+RecvSize) send buffer,
//   +(56+RecvSize+SendSize) NAT/session metadata, then Mutex.
template <int RecvSize, int SendSize>
class CNetwork {
public:
    CNetwork();
    ~CNetwork();

    static bool SetNonBlock(int fd);
    static int CreateConnectionSocket(int& fd, const char* ip, int port);
    static int CreateConnectionBlockSocket(int& fd, const char* ip, int port);
    static bool CreateListenSocket(int& fd, int port, const char* ip);

    int get_socket();
    int set_socket(int fd, bool block);
    int disconnect();

    int recv_packet();
    int send_packet();
    int send_packet(char* src, int bytes);
    int send_lock();
    int send_unlock();
    // The client 4096/450000 specialization is read directly by the old
    // NetworkThread (no standalone ELF provider exists for that template
    // instantiation).  Keep this accessor inline so its call site remains the
    // original +0x1030 remaining-send-length load; larger server links retain
    // their explicitly emitted specializations below.
    int GetRemainSendSize() const { return remain_send_len_; }

    int Parsing_ServerPacket(int received_bytes);
    int Parsing_AuctionServerPacket(int received_bytes);

    void* initNetworkBuffer();
    char* GetPeerIP2(char* out, int bytes);

    char* get_str_ip();
    int get_nat_type();
    char* set_nat(char nat_type, uint32_t inner_ip, uint32_t ip,
                  uint16_t port, uint32_t mtu);
    char* set_nat(char nat_type, uint32_t inner_ip, uint32_t ip,
                  uint16_t port, const char* mac_addr);
    const char* get_mac_addr() const;
    uint32_t get_inner_ip();
    uint32_t get_ip();
    uint16_t get_port();
    uint32_t get_mtu();

private:
    // The old monitor-proxy InitTcp body passes its embedded network object's
    // real socket member to CNetwork::CreateConnectionSocket.  Keep that
    // source-level friendship rather than recreating the operation through a
    // byte offset or a compatibility wrapper.
    friend class CMonitorServerProxy;
    // CUser::Parsing owns the game-frame state machine at 0x8647890 and is
    // the only other original owner which accesses these in-object buffers.
    friend class CUser;
    // The original Exchange CSession derives from CNetwork<95000,450000>
    // and parses its retained receive cursor in place.  Grant that concrete
    // owner access without exposing the transport buffers to unrelated code.
    friend class exchange_server::CSession;

    void initialize_state();
    int append_send_packet(char* src, int bytes);
    int flush_send_packet();
    int receive_packet();
    int parse_server_packet(int received_bytes);
    int parse_auction_packet(int received_bytes);
    int lock_send();
    int unlock_send();
    int close_socket();
    int assign_socket(int fd, bool block);
    void* reset_network_buffers();
    char* set_nat_with_mtu(char nat_type, uint32_t inner_ip, uint32_t ip,
                           uint16_t port, uint32_t mtu);
    char* set_nat_with_mac(char nat_type, uint32_t inner_ip, uint32_t ip,
                           uint16_t port, const char* mac_addr);
    char* peer_ip_string(char* out, int bytes);

    int socket_;             // +0x00 active TCP socket fd.
    int reserved_04_;        // +0x04 old connection state slot.
    int reserved_08_;        // +0x08 old connection state slot.
    int reserved_0c_;        // +0x0c unused in exported network functions.
    int reserved_10_;        // +0x10 per-connection state flag.
    uint8_t reserved_14_;    // +0x14 byte flag reset by constructor.
    uint8_t pad_15_17_[3];
    int remain_recv_len_;    // +0x18 bytes retained after partial parsing.
    uint8_t pad_1c_28_[13];
    uint8_t recv_flag_;      // +0x29 reset when receive buffer wraps.
    uint8_t pad_2a_2b_[2];
    char* recv_cursor_;      // +0x2c current receive cursor.
    char recv_buffer_[RecvSize];
    int remain_send_len_;    // +(48+RecvSize) pending send bytes.
    char* send_cursor_;      // +(52+RecvSize) append cursor.
    char send_buffer_[SendSize];
    uint8_t nat_type_;       // NAT/session type byte.
    uint8_t nat_pad_[3];
    uint32_t ip_;            // External IPv4 in network byte order.
    char str_ip_[16];        // inet_ntoa(ip_) cached string.
    uint16_t port_;          // External port in network byte order usage.
    uint8_t port_pad_[2];
    uint32_t mtu_;           // UDP/NAT MTU value.
    uint32_t inner_ip_;      // Internal IPv4 in network byte order.
    char mac_addr_[65];      // IDA copies 0x41 bytes here.
    uint8_t mac_pad_[3];
    Mutex send_mutex_;       // Send-buffer lock at NAT metadata + 104.
};

typedef CNetwork<100000, 100000> CNetwork100000_100000;
typedef CNetwork<4096, 450000> CNetwork4096_450000;
typedef CNetwork<6144, 100000> CNetwork6144_100000;
typedef CNetwork<95000, 450000> CNetwork95000_450000;

#if defined(__i386__)
static_assert(sizeof(Mutex) == 0x18,
              "old i386 Mutex must match pthread_mutex_t storage");
static_assert(sizeof(CNetwork4096_450000) == 0x6EE88,
              "CNetwork<4096,450000> must match old CUser embedded ABI");
#endif


#endif  // OLDSRV_RUNTIME_NETWORK_ENDPOINT_H_
