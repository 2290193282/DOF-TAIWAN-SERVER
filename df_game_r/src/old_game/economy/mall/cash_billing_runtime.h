#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_BILLING_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_BILLING_RUNTIME_H_

#include <stdint.h>

#include <netinet/in.h>
#include <sys/time.h>

namespace nexon {
namespace cash {

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// The four fields are proven by every serializer/deserializer and by the
// old i386 constructor: begin/write/read cursors followed by capacity.
class ByteBuffer {
public:
    // IDA_EA: 0x81AE9E2.
    ByteBuffer(char* buffer, unsigned int capacity);

    // IDA_EA: 0x81ACBA0.
    ByteBuffer& operator<<(const char* value);
    // IDA_EA: 0x81ACD1E.
    ByteBuffer& operator<<(const wchar_t* value);
    // IDA_EA: 0x81ACEFA.
    ByteBuffer& operator<<(unsigned short value);
    // IDA_EA: 0x81AE614.
    ByteBuffer& operator<<(unsigned char value);
    // IDA_EA: 0x81AE65E.
    ByteBuffer& operator<<(short value);
    // IDA_EA: 0x81AE6C4.
    ByteBuffer& operator<<(int value);
    // IDA_EA: 0x81AE74C.
    ByteBuffer& operator<<(unsigned int value);
    // IDA_EA: 0x81AE7D4.
    ByteBuffer& operator<<(unsigned long long value);
    // IDA_EA: 0x81AE8BC.
    ByteBuffer& operator>>(unsigned char& value);
    // IDA_EA: 0x81AE8E4.
    ByteBuffer& operator>>(int& value);
    // IDA_EA: 0x81AE94E.
    ByteBuffer& operator>>(unsigned int& value);

    // IDA_EA: 0x81AE9B8.
    ByteBuffer* wrap(char* buffer, unsigned int capacity);
    // IDA_EA: 0x81ACF60.
    int length() const;

private:
    char* begin_;
    char* write_;
    char* read_;
    unsigned int capacity_;
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// Both sockaddr_in objects are independently evidenced: accept(2) writes the
// first at +4, while recvfrom(2) writes the second at +0x18.  The host-order
// port at +0x14 is deliberately distinct from either wire-order endpoint.
class UDPSocket {
public:
    // IDA_EA: 0x81AED2C.
    UDPSocket();
    // IDA_EA: 0x81AED44.
    ~UDPSocket();

    // IDA_EA: 0x81AED58.
    int open();
    // IDA_EA: 0x81AEDC6.
    int bind(const char* address, uint16_t host_port);
    // IDA_EA: 0x81AEE82.
    int bind(uint16_t host_port, bool non_blocking);
    // IDA_EA: 0x81AEFA2.
    bool setOptNonBlock();
    // IDA_EA: 0x81AF000.
    int send(char* buffer,
             int length,
             uint16_t destination_port,
             const char* destination_address);
    // IDA_EA: 0x81AF0DE.
    int recv(char* buffer, int length);
    // IDA_EA: 0x81AF17A.
    int getHandle();
    // IDA_EA: 0x81AF184.
    UDPSocket* close();
    // IDA_EA: 0x81AF1B8.
    bool setOptResizeSendBuf(int size);
    // IDA_EA: 0x81AF210.
    bool setOptResizeRecvBuf(int size);
    // IDA_EA: 0x81AF268.
    bool pollReadEvent() const;
    // IDA_EA: 0x81AF334.
    bool pollWriteEvent() const;
    // IDA_EA: 0x81AF400.
    bool pollErrorEvent() const;

private:
    int fd_;
    sockaddr_in bound_endpoint_;
    uint16_t bound_port_host_order_;
    sockaddr_in last_sender_endpoint_;
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// The accepted endpoint is retained at +4; the separately cached peer IPv4
// and peer port are at +0x14/+0x18.  No padding member is declared: i386
// alignment supplies the two bytes before the failure counter at +0x1c.
class TCPSocket {
public:
    // IDA_EA: 0x81AF4CC.
    TCPSocket();
    // IDA_EA: 0x81AF50E.
    TCPSocket(const char* address, uint16_t host_port);
    // IDA_EA: 0x81AF51E.
    ~TCPSocket();

    // IDA_EA: 0x81AF532.
    int open();
    // IDA_EA: 0x81AF5B4.
    int bind(uint16_t host_port, bool non_blocking);
    // IDA_EA: 0x81AF674.
    int listen(int backlog);
    // IDA_EA: 0x81AF6AE.
    int send(char* buffer, unsigned int length);
    // IDA_EA: 0x81AF76E.
    int recv(char* buffer, unsigned int length);
    // IDA_EA: 0x81AF7F4.
    int getHandle() const;
    // IDA_EA: 0x81AF7FE.
    int shutdown(int how);
    // IDA_EA: 0x81AF80C.
    TCPSocket* close();
    // IDA_EA: 0x81AF85E.
    bool setOptNonBlock();
    // IDA_EA: 0x81AF8BC.
    bool setOptReuseAdrs(bool value);
    // IDA_EA: 0x81AF926.
    bool setOptLinger(bool value);
    // IDA_EA: 0x81AF990.
    bool setOptNagle(bool value);
    // IDA_EA: 0x81AF9EA.
    int connect(const char* address, uint16_t host_port);
    // IDA_EA: 0x81AFA98.
    int connect_nonb(const char* address,
                     uint16_t host_port,
                     timeval timeout);
    // IDA_EA: 0x81AFCC8.
    bool pollReadEvent() const;
    // IDA_EA: 0x81AFD94.
    bool pollWriteEvent() const;
    // IDA_EA: 0x81AFE60.
    bool pollErrorEvent() const;
    // IDA_EA: 0x81AFF2C.
    int accept(TCPSocket& accepted_socket);
    // IDA_EA: 0x81AFFB8.
    char* getPeerAdrs();
    // IDA_EA: 0x81AFFC4.
    int getPeerPort();
    // IDA_EA: 0x81AFFD0.
    bool setOptResizeSendBuf(int size);
    // IDA_EA: 0x81B0036.
    bool setOptResizeRecvBuf(int size);
    // IDA_EA: 0x81B009C.
    static void SetRecvBufSize(int size);
    // IDA_EA: 0x81B00AA.
    static void SetSendBufSize(int size);

private:
    int fd_;
    sockaddr_in accepted_peer_endpoint_;
    in_addr peer_ipv4_;
    uint16_t peer_port_network_order_;
    int transient_send_failures_;

    static int msRecvBufSize;
    static int msSendBufSize;
};

// The old receiver sends/receives this exact three-byte header before a body
// of at most 0x200 bytes.  It is a protocol record, not opaque storage.
#pragma pack(push, 1)
struct NCashWireFrame {
    uint8_t wire_magic;
    uint16_t body_length_network_order;
    uint8_t payload[0x200];
};
#pragma pack(pop)

static_assert(sizeof(NCashWireFrame) == 0x203u,
              "NCash wire frame must retain its old three-byte header");

// The in-object marker at +0 and the uint16 body length at +2 are distinct
// from the on-wire frame beginning at +4.  The compiler supplies the one-byte
// natural alignment gap; no synthetic padding is introduced.
class NCashMessage {
public:
    // IDA_EA: 0x81AE52E.
    NCashMessage();

    // IDA_EA: 0x81AE542.
    char* data();
    // IDA_EA: 0x81AE54E.
    int length() const;
    // IDA_EA: 0x81AE560.
    char* body();
    // IDA_EA: 0x81AE56C.
    int body_length();
    // IDA_EA: 0x81AE578.
    int encode_header();
    // IDA_EA: 0x81AE5AE.
    int encode_header(unsigned int body_length);
    // IDA_EA: 0x81AE5CE.
    int decode_header();

private:
    uint8_t message_marker_;
    uint16_t body_length_;
    NCashWireFrame wire_;
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(ByteBuffer) == 0x10u,
              "ByteBuffer old i386 ABI must stay 0x10");
#endif
static_assert(sizeof(NCashMessage) == 0x208u,
              "NCashMessage old ABI must stay 0x208");
#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(UDPSocket) == 0x28u,
              "UDPSocket old i386 ABI must stay 0x28");
static_assert(sizeof(TCPSocket) == 0x20u,
              "TCPSocket old i386 ABI must stay 0x20");
#endif

}  // namespace cash
}  // namespace nexon

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_BILLING_RUNTIME_H_
