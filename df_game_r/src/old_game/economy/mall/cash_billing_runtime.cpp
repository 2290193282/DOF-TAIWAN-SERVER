#include "old_game/economy/mall/cash_billing_runtime.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <limits>
#include <new>
#include <stdexcept>

namespace nexon {
namespace cash {

// IDA_EA: 0x81ACBA0
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEPKc
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(char const*)
ByteBuffer& ByteBuffer::operator<<(const char* value) {
    const unsigned int value_length =
        static_cast<unsigned int>(std::strlen(value));
    if (static_cast<unsigned int>(
            std::numeric_limits<unsigned short>::max()) < value_length) {
        throw std::invalid_argument(
            "string length too long: must shorter than max(unsigned short)");
    }

    if (value_length + length() <= capacity_) {
        *this << static_cast<unsigned short>(value_length);
        std::strncpy(write_, value, value_length);
        write_ += std::strlen(value);
    }
    return *this;
}

// IDA_EA: 0x81ACD1E
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEPKw
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(wchar_t const*)
ByteBuffer& ByteBuffer::operator<<(const wchar_t* value) {
    const unsigned int byte_length =
        static_cast<unsigned int>(2 * std::wcslen(value));
    if (static_cast<unsigned int>(
            std::numeric_limits<unsigned short>::max()) < byte_length) {
        throw std::invalid_argument(
            "string length too long: must shorter than max(unsigned short)");
    }

    if (byte_length + length() <= capacity_) {
        *this << static_cast<unsigned short>(byte_length);
        std::wcsncpy(reinterpret_cast<wchar_t*>(write_),
                     value,
                     std::wcslen(value));
        write_ += byte_length;
    }
    return *this;
}

// IDA_EA: 0x81ACEFA
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEt
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(unsigned short)
ByteBuffer& ByteBuffer::operator<<(unsigned short value) {
    if (static_cast<unsigned int>(length() + 2) <= capacity_) {
        write_[0] = static_cast<char>((value >> 8) & 0xff);
        write_[1] = static_cast<char>(value & 0xff);
        write_ += 2;
    }
    return *this;
}

// IDA_EA: 0x81ACF60
// IDA_NAME: _ZNK5nexon4cash10ByteBuffer6lengthEv
// DEMANGLED: nexon::cash::ByteBuffer::length(void)const
int ByteBuffer::length() const {
    return static_cast<int>(write_ - begin_);
}

// IDA_EA: 0x81AE52E
// IDA_NAME: _ZN5nexon4cash12NCashMessageC2Ev
// DEMANGLED: nexon::cash::NCashMessage::NCashMessage(void)
NCashMessage::NCashMessage()
    : message_marker_(85),
      body_length_(0) {}

// IDA_EA: 0x81AE542
// IDA_NAME: _ZN5nexon4cash12NCashMessage4dataEv
// DEMANGLED: nexon::cash::NCashMessage::data(void)
char* NCashMessage::data() {
    return reinterpret_cast<char*>(&wire_);
}

// IDA_EA: 0x81AE54E
// IDA_NAME: _ZNK5nexon4cash12NCashMessage6lengthEv
// DEMANGLED: nexon::cash::NCashMessage::length(void)const
int NCashMessage::length() const {
    return body_length_ + 3;
}

// IDA_EA: 0x81AE560
// IDA_NAME: _ZN5nexon4cash12NCashMessage4bodyEv
// DEMANGLED: nexon::cash::NCashMessage::body(void)
char* NCashMessage::body() {
    return reinterpret_cast<char*>(wire_.payload);
}

// IDA_EA: 0x81AE56C
// IDA_NAME: _ZN5nexon4cash12NCashMessage11body_lengthEv
// DEMANGLED: nexon::cash::NCashMessage::body_length(void)
int NCashMessage::body_length() {
    return body_length_;
}

// IDA_EA: 0x81AE578
// IDA_NAME: _ZN5nexon4cash12NCashMessage13encode_headerEv
// DEMANGLED: nexon::cash::NCashMessage::encode_header(void)
int NCashMessage::encode_header() {
    wire_.wire_magic = 85;
    const uint16_t network_body_length = htons(body_length_);
    std::memcpy(data() + 1,
                &network_body_length,
                sizeof(network_body_length));
    return 1;
}

// IDA_EA: 0x81AE5AE
// IDA_NAME: _ZN5nexon4cash12NCashMessage13encode_headerEj
// DEMANGLED: nexon::cash::NCashMessage::encode_header(unsigned int)
int NCashMessage::encode_header(unsigned int body_length) {
    body_length_ = static_cast<uint16_t>(body_length);
    return encode_header();
}

// IDA_EA: 0x81AE5CE
// IDA_NAME: _ZN5nexon4cash12NCashMessage13decode_headerEv
// DEMANGLED: nexon::cash::NCashMessage::decode_header(void)
int NCashMessage::decode_header() {
    uint16_t network_body_length = 0;
    std::memcpy(&network_body_length,
                data() + 1,
                sizeof(network_body_length));
    body_length_ = ntohs(network_body_length);
    if (body_length_ <= 0x200u) {
        return 1;
    }
    body_length_ = 0;
    return 0;
}

// IDA_EA: 0x81AE614
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEh
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(unsigned char)
ByteBuffer& ByteBuffer::operator<<(unsigned char value) {
    if (static_cast<unsigned int>(length() + 1) < capacity_) {
        *write_ = static_cast<char>(value);
        ++write_;
    }
    return *this;
}

// IDA_EA: 0x81AE65E
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEs
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(short)
ByteBuffer& ByteBuffer::operator<<(short value) {
    if (static_cast<unsigned int>(length() + 2) <= capacity_) {
        const unsigned short encoded = static_cast<unsigned short>(value);
        write_[0] = static_cast<char>((encoded >> 8) & 0xff);
        write_[1] = static_cast<char>(encoded & 0xff);
        write_ += 2;
    }
    return *this;
}

// IDA_EA: 0x81AE6C4
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEi
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(int)
ByteBuffer& ByteBuffer::operator<<(int value) {
    if (static_cast<unsigned int>(length() + 4) <= capacity_) {
        const unsigned int encoded = static_cast<unsigned int>(value);
        write_[0] = static_cast<char>((encoded >> 24) & 0xff);
        write_[1] = static_cast<char>((encoded >> 16) & 0xff);
        write_[2] = static_cast<char>((encoded >> 8) & 0xff);
        write_[3] = static_cast<char>(encoded & 0xff);
        write_ += 4;
    }
    return *this;
}

// IDA_EA: 0x81AE74C
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEj
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(unsigned int)
ByteBuffer& ByteBuffer::operator<<(unsigned int value) {
    if (static_cast<unsigned int>(length() + 4) <= capacity_) {
        write_[0] = static_cast<char>((value >> 24) & 0xff);
        write_[1] = static_cast<char>((value >> 16) & 0xff);
        write_[2] = static_cast<char>((value >> 8) & 0xff);
        write_[3] = static_cast<char>(value & 0xff);
        write_ += 4;
    }
    return *this;
}

// IDA_EA: 0x81AE7D4
// IDA_NAME: _ZN5nexon4cash10ByteBufferlsEy
// DEMANGLED: nexon::cash::ByteBuffer::operator<<(unsigned long long)
ByteBuffer& ByteBuffer::operator<<(unsigned long long value) {
    if (static_cast<unsigned int>(length() + 8) <= capacity_) {
        write_[0] = static_cast<char>((value >> 56) & 0xff);
        write_[1] = static_cast<char>((value >> 48) & 0xff);
        write_[2] = static_cast<char>((value >> 40) & 0xff);
        write_[3] = static_cast<char>((value >> 32) & 0xff);
        write_[4] = static_cast<char>((value >> 24) & 0xff);
        write_[5] = static_cast<char>((value >> 16) & 0xff);
        write_[6] = static_cast<char>((value >> 8) & 0xff);
        write_[7] = static_cast<char>(value & 0xff);
        write_ += 8;
    }
    return *this;
}

// IDA_EA: 0x81AE8BC
// IDA_NAME: _ZN5nexon4cash10ByteBufferrsERh
// DEMANGLED: nexon::cash::ByteBuffer::operator>>(unsigned char &)
ByteBuffer& ByteBuffer::operator>>(unsigned char& value) {
    value = static_cast<unsigned char>(*read_);
    ++read_;
    return *this;
}

// IDA_EA: 0x81AE8E4
// IDA_NAME: _ZN5nexon4cash10ByteBufferrsERi
// DEMANGLED: nexon::cash::ByteBuffer::operator>>(int &)
ByteBuffer& ByteBuffer::operator>>(int& value) {
    value =
        (static_cast<unsigned char>(read_[0]) << 24) |
        (static_cast<unsigned char>(read_[1]) << 16) |
        (static_cast<unsigned char>(read_[2]) << 8) |
        static_cast<unsigned char>(read_[3]);
    read_ += 4;
    return *this;
}

// IDA_EA: 0x81AE94E
// IDA_NAME: _ZN5nexon4cash10ByteBufferrsERj
// DEMANGLED: nexon::cash::ByteBuffer::operator>>(unsigned int &)
ByteBuffer& ByteBuffer::operator>>(unsigned int& value) {
    value =
        (static_cast<unsigned char>(read_[0]) << 24) |
        (static_cast<unsigned char>(read_[1]) << 16) |
        (static_cast<unsigned char>(read_[2]) << 8) |
        static_cast<unsigned char>(read_[3]);
    read_ += 4;
    return *this;
}

// IDA_EA: 0x81AE9B8
// IDA_NAME: _ZN5nexon4cash10ByteBuffer4wrapEPcj
// DEMANGLED: nexon::cash::ByteBuffer::wrap(char *,unsigned int)
ByteBuffer* ByteBuffer::wrap(char* buffer, unsigned int capacity) {
    new (this) ByteBuffer(buffer, capacity);
    return this;
}

// IDA_EA: 0x81AE9E2
// IDA_NAME: _ZN5nexon4cash10ByteBufferC2EPcj
// DEMANGLED: nexon::cash::ByteBuffer::ByteBuffer(char *,unsigned int)
ByteBuffer::ByteBuffer(char* buffer, unsigned int capacity)
    : begin_(buffer),
      write_(buffer),
      read_(buffer),
      capacity_(capacity) {}

// IDA_EA: 0x81AED2C
// IDA_NAME: _ZN5nexon4cash9UDPSocketC2Ev
// DEMANGLED: nexon::cash::UDPSocket::UDPSocket(void)
UDPSocket::UDPSocket()
    : fd_(-1),
      bound_port_host_order_(0) {}

// IDA_EA: 0x81AED44
// IDA_NAME: _ZN5nexon4cash9UDPSocketD2Ev
// DEMANGLED: nexon::cash::UDPSocket::~UDPSocket()
UDPSocket::~UDPSocket() {
    close();
}

// IDA_EA: 0x81AED58
// IDA_NAME: _ZN5nexon4cash9UDPSocket4openEv
// DEMANGLED: nexon::cash::UDPSocket::open(void)
int UDPSocket::open() {
    if (fd_ != -1) {
        return 0;
    }

    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ != -1) {
        return 1;
    }

    std::printf("Could not create a UDP socket : %d\n", errno);
    return 0;
}

// IDA_EA: 0x81AEDC6
// IDA_NAME: _ZN5nexon4cash9UDPSocket4bindEPKct
// DEMANGLED: nexon::cash::UDPSocket::bind(char const*,unsigned short)
int UDPSocket::bind(const char* address, uint16_t host_port) {
    bound_port_host_order_ = host_port;
    std::memset(&bound_endpoint_, 0, sizeof(bound_endpoint_));
    bound_endpoint_.sin_family = AF_INET;
    bound_endpoint_.sin_addr.s_addr = ::inet_addr(address);
    bound_endpoint_.sin_port = htons(bound_port_host_order_);
    if (::bind(fd_,
               reinterpret_cast<const sockaddr*>(&bound_endpoint_),
               sizeof(bound_endpoint_)) == 0) {
        return 1;
    }

    std::printf("Udp Port binding fail, %s , %d\n", address, host_port);
    return 0;
}

// IDA_EA: 0x81AEE82
// IDA_NAME: _ZN5nexon4cash9UDPSocket4bindEtb
// DEMANGLED: nexon::cash::UDPSocket::bind(unsigned short,bool)
int UDPSocket::bind(uint16_t host_port, bool non_blocking) {
    bound_port_host_order_ = host_port;
    std::memset(&bound_endpoint_, 0, sizeof(bound_endpoint_));
    bound_endpoint_.sin_family = AF_INET;
    bound_endpoint_.sin_addr.s_addr = htonl(0);
    bound_endpoint_.sin_port = htons(bound_port_host_order_);

    if (::bind(fd_,
               reinterpret_cast<const sockaddr*>(&bound_endpoint_),
               sizeof(bound_endpoint_)) != 0) {
        const int bind_errno = errno;
        if (bind_errno == EADDRINUSE) {
            std::printf("Port %d for receiving UDP is in use\n", host_port);
        } else if (bind_errno == EADDRNOTAVAIL) {
            std::puts("Cannot assign requested address");
        } else if (bind_errno != 0) {
            std::printf("Could not bind UDP receive port. Error= %d , strerror = %s\n",
                        bind_errno,
                        std::strerror(bind_errno));
        }
        return 0;
    }

    if (non_blocking) {
        setOptNonBlock();
    }
    return 1;
}

// IDA_EA: 0x81AEFA2
// IDA_NAME: _ZN5nexon4cash9UDPSocket14setOptNonBlockEv
// DEMANGLED: nexon::cash::UDPSocket::setOptNonBlock(void)
bool UDPSocket::setOptNonBlock() {
    const int flags = ::fcntl(fd_, F_GETFL, 0) | O_NONBLOCK;
    return ::fcntl(fd_, F_SETFL, flags) >= 0;
}

// IDA_EA: 0x81AF000
// IDA_NAME: _ZN5nexon4cash9UDPSocket4sendEPcitPKc
// DEMANGLED: nexon::cash::UDPSocket::send(char *,int,unsigned short,char const*)
int UDPSocket::send(char* buffer,
                    int length,
                    uint16_t destination_port,
                    const char* destination_address) {
    if (!buffer || length <= 0) {
        return -1;
    }
    if (!destination_address) {
        return 0;
    }

    sockaddr_in destination;
    std::memset(&destination, 0, sizeof(destination));
    destination.sin_family = AF_INET;
    destination.sin_port = htons(destination_port);
    destination.sin_addr.s_addr = ::inet_addr(destination_address);

    const int sent = static_cast<int>(
        ::sendto(fd_,
                 buffer,
                 static_cast<size_t>(length),
                 0,
                 reinterpret_cast<const sockaddr*>(&destination),
                 sizeof(destination)));
    if (sent < 0 && (errno == EAGAIN || errno == EINTR)) {
        return 0;
    }
    return sent;
}

// IDA_EA: 0x81AF0DE
// IDA_NAME: _ZN5nexon4cash9UDPSocket4recvEPci
// DEMANGLED: nexon::cash::UDPSocket::recv(char *,int)
int UDPSocket::recv(char* buffer, int length) {
    if (!buffer || length <= 0) {
        return -1;
    }

    socklen_t sender_length = sizeof(last_sender_endpoint_);
    const int received = static_cast<int>(
        ::recvfrom(fd_,
                   buffer,
                   static_cast<size_t>(length),
                   0,
                   reinterpret_cast<sockaddr*>(&last_sender_endpoint_),
                   &sender_length));
    if (received >= 0) {
        return received;
    }
    if (errno == EAGAIN || errno == EINTR) {
        return 0;
    }
    return -1;
}

// IDA_EA: 0x81AF17A
// IDA_NAME: _ZN5nexon4cash9UDPSocket9getHandleEv
// DEMANGLED: nexon::cash::UDPSocket::getHandle(void)
int UDPSocket::getHandle() {
    return fd_;
}

// IDA_EA: 0x81AF184
// IDA_NAME: _ZN5nexon4cash9UDPSocket5closeEv
// DEMANGLED: nexon::cash::UDPSocket::close(void)
UDPSocket* UDPSocket::close() {
    const int previous_fd = fd_;
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
        bound_port_host_order_ = 0;
        return this;
    }

    // The old function's no-op branch returned the register holding fd_;
    // preserve that unusual observable result under its pointer ABI.
    return reinterpret_cast<UDPSocket*>(static_cast<intptr_t>(previous_fd));
}

// IDA_EA: 0x81AF1B8
// IDA_NAME: _ZN5nexon4cash9UDPSocket19setOptResizeSendBufEi
// DEMANGLED: nexon::cash::UDPSocket::setOptResizeSendBuf(int)
bool UDPSocket::setOptResizeSendBuf(int size) {
    return size > 0 &&
           ::setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) >= 0;
}

// IDA_EA: 0x81AF210
// IDA_NAME: _ZN5nexon4cash9UDPSocket19setOptResizeRecvBufEi
// DEMANGLED: nexon::cash::UDPSocket::setOptResizeRecvBuf(int)
bool UDPSocket::setOptResizeRecvBuf(int size) {
    return size > 0 &&
           ::setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) >= 0;
}

// IDA_EA: 0x81AF268
// IDA_NAME: _ZNK5nexon4cash9UDPSocket13pollReadEventEv
// DEMANGLED: nexon::cash::UDPSocket::pollReadEvent(void)const
bool UDPSocket::pollReadEvent() const {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd_, &read_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;
    return ::select(2, &read_fds, 0, 0, &timeout) != 0;
}

// IDA_EA: 0x81AF334
// IDA_NAME: _ZNK5nexon4cash9UDPSocket14pollWriteEventEv
// DEMANGLED: nexon::cash::UDPSocket::pollWriteEvent(void)const
bool UDPSocket::pollWriteEvent() const {
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(fd_, &write_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return ::select(2, 0, &write_fds, 0, &timeout) != 0;
}

// IDA_EA: 0x81AF400
// IDA_NAME: _ZNK5nexon4cash9UDPSocket14pollErrorEventEv
// DEMANGLED: nexon::cash::UDPSocket::pollErrorEvent(void)const
bool UDPSocket::pollErrorEvent() const {
    fd_set except_fds;
    FD_ZERO(&except_fds);
    FD_SET(fd_, &except_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return ::select(2, 0, 0, &except_fds, &timeout) != 0;
}

int TCPSocket::msRecvBufSize = 0;
int TCPSocket::msSendBufSize = 0;

// IDA_EA: 0x81AF4CC
// IDA_NAME: _ZN5nexon4cash9TCPSocketC2Ev
// DEMANGLED: nexon::cash::TCPSocket::TCPSocket(void)
TCPSocket::TCPSocket() {
    fd_ = -1;
    transient_send_failures_ = 0;
    std::memset(&peer_ipv4_, 0, sizeof(peer_ipv4_));
    peer_port_network_order_ = 0;
}

// IDA_EA: 0x81AF50E
// IDA_NAME: _ZN5nexon4cash9TCPSocketC2EPKct
// DEMANGLED: nexon::cash::TCPSocket::TCPSocket(char const*,unsigned short)
TCPSocket::TCPSocket(const char*, uint16_t) {}

// IDA_EA: 0x81AF51E
// IDA_NAME: _ZN5nexon4cash9TCPSocketD2Ev
// DEMANGLED: nexon::cash::TCPSocket::~TCPSocket()
TCPSocket::~TCPSocket() {
    close();
}

// IDA_EA: 0x81AF532
// IDA_NAME: _ZN5nexon4cash9TCPSocket4openEv
// DEMANGLED: nexon::cash::TCPSocket::open(void)
int TCPSocket::open() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_ == -1) {
        std::printf("Could not create a TDP socket : %d\n", errno);
        return 0;
    }

    setOptLinger(false);
    setOptNagle(false);
    return 1;
}

// IDA_EA: 0x81AF5B4
// IDA_NAME: _ZN5nexon4cash9TCPSocket4bindEtb
// DEMANGLED: nexon::cash::TCPSocket::bind(unsigned short,bool)
int TCPSocket::bind(uint16_t host_port, bool non_blocking) {
    setOptReuseAdrs(true);

    sockaddr_in endpoint;
    std::memset(&endpoint, 0, sizeof(endpoint));
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(host_port);
    endpoint.sin_addr.s_addr = htonl(0);

    if (::bind(fd_,
               reinterpret_cast<const sockaddr*>(&endpoint),
               sizeof(endpoint)) < 0) {
        close();
        return 0;
    }

    if (non_blocking) {
        setOptNonBlock();
    }
    std::printf("succeeded in binding TCP socket port #%d\n", host_port);
    return 1;
}

// IDA_EA: 0x81AF674
// IDA_NAME: _ZN5nexon4cash9TCPSocket6listenEi
// DEMANGLED: nexon::cash::TCPSocket::listen(int)
int TCPSocket::listen(int backlog) {
    if (::listen(fd_, backlog) >= 0) {
        return 1;
    }
    close();
    return 0;
}

// IDA_EA: 0x81AF6AE
// IDA_NAME: _ZN5nexon4cash9TCPSocket4sendEPcj
// DEMANGLED: nexon::cash::TCPSocket::send(char *,unsigned int)
int TCPSocket::send(char* buffer, unsigned int length) {
    if (!buffer || !length) {
        return -1;
    }

    const ssize_t sent = ::write(fd_, buffer, length);
    const int write_errno = errno;
    if (sent > 0) {
        transient_send_failures_ = 0;
        return static_cast<int>(sent);
    }
    if (write_errno == EAGAIN || write_errno == EINTR) {
        if (++transient_send_failures_ <= 100) {
            return 0;
        }
        transient_send_failures_ = 0;
        return -100;
    }
    if (write_errno != 0) {
        transient_send_failures_ = 0;
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81AF76E
// IDA_NAME: _ZN5nexon4cash9TCPSocket4recvEPcj
// DEMANGLED: nexon::cash::TCPSocket::recv(char *,unsigned int)
int TCPSocket::recv(char* buffer, unsigned int length) {
    if (!buffer || !length) {
        return -1;
    }

    const ssize_t received = ::read(fd_, buffer, length);
    if (received >= 0) {
        if (received == 0) {
            return -1;
        }
    } else if (errno == EAGAIN || errno == EINTR || errno == 0) {
        return 0;
    }
    return static_cast<int>(received);
}

// IDA_EA: 0x81AF7F4
// IDA_NAME: _ZNK5nexon4cash9TCPSocket9getHandleEv
// DEMANGLED: nexon::cash::TCPSocket::getHandle(void)const
int TCPSocket::getHandle() const {
    return fd_;
}

// IDA_EA: 0x81AF7FE
// IDA_NAME: _ZN5nexon4cash9TCPSocket8shutdownEi
// DEMANGLED: nexon::cash::TCPSocket::shutdown(int)
int TCPSocket::shutdown(int how) {
    static_cast<void>(how);
    return fd_;
}

// IDA_EA: 0x81AF80C
// IDA_NAME: _ZN5nexon4cash9TCPSocket5closeEv
// DEMANGLED: nexon::cash::TCPSocket::close(void)
TCPSocket* TCPSocket::close() {
    const int previous_fd = fd_;
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
        std::memset(&peer_ipv4_, 0, sizeof(peer_ipv4_));
        peer_port_network_order_ = 0;
        return this;
    }

    // As in UDPSocket::close, the old no-op branch exposed the fd register.
    return reinterpret_cast<TCPSocket*>(static_cast<intptr_t>(previous_fd));
}

// IDA_EA: 0x81AF85E
// IDA_NAME: _ZN5nexon4cash9TCPSocket14setOptNonBlockEv
// DEMANGLED: nexon::cash::TCPSocket::setOptNonBlock(void)
bool TCPSocket::setOptNonBlock() {
    const int flags = ::fcntl(fd_, F_GETFL, 0) | O_NONBLOCK;
    return ::fcntl(fd_, F_SETFL, flags) >= 0;
}

// IDA_EA: 0x81AF8BC
// IDA_NAME: _ZN5nexon4cash9TCPSocket15setOptReuseAdrsEb
// DEMANGLED: nexon::cash::TCPSocket::setOptReuseAdrs(bool)
bool TCPSocket::setOptReuseAdrs(bool value) {
    const int option = value;
    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &option,
                        sizeof(option)) >= 0;
}

// IDA_EA: 0x81AF926
// IDA_NAME: _ZN5nexon4cash9TCPSocket12setOptLingerEb
// DEMANGLED: nexon::cash::TCPSocket::setOptLinger(bool)
bool TCPSocket::setOptLinger(bool value) {
    linger option;
    option.l_onoff = value;
    option.l_linger = 0;
    return ::setsockopt(fd_,
                        SOL_SOCKET,
                        SO_LINGER,
                        &option,
                        sizeof(option)) >= 0;
}

// IDA_EA: 0x81AF990
// IDA_NAME: _ZN5nexon4cash9TCPSocket11setOptNagleEb
// DEMANGLED: nexon::cash::TCPSocket::setOptNagle(bool)
bool TCPSocket::setOptNagle(bool value) {
    const int option = value;
    return ::setsockopt(fd_,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        &option,
                        sizeof(option)) == 0;
}

// IDA_EA: 0x81AF9EA
// IDA_NAME: _ZN5nexon4cash9TCPSocket7connectEPKct
// DEMANGLED: nexon::cash::TCPSocket::connect(char const*,unsigned short)
int TCPSocket::connect(const char* address, uint16_t host_port) {
    sockaddr_in endpoint;
    std::memset(&endpoint, 0, sizeof(endpoint));
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = ::inet_addr(address);
    endpoint.sin_port = htons(host_port);

    if (::connect(fd_,
                  reinterpret_cast<const sockaddr*>(&endpoint),
                  sizeof(endpoint)) < 0) {
        return 0;
    }

    peer_ipv4_ = endpoint.sin_addr;
    peer_port_network_order_ = endpoint.sin_port;
    return 1;
}

// IDA_EA: 0x81AFA98
// IDA_NAME: _ZN5nexon4cash9TCPSocket12connect_nonbEPKct7timeval
// DEMANGLED: nexon::cash::TCPSocket::connect_nonb(char const*,unsigned short,timeval)
int TCPSocket::connect_nonb(const char* address,
                            uint16_t host_port,
                            timeval timeout) {
    sockaddr_in endpoint;
    std::memset(&endpoint, 0, sizeof(endpoint));
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = ::inet_addr(address);
    endpoint.sin_port = htons(host_port);

    setOptNonBlock();
    int connect_result = ::connect(fd_,
                                   reinterpret_cast<const sockaddr*>(&endpoint),
                                   sizeof(endpoint));
    if (connect_result < 0 && errno != EINPROGRESS && errno != EINTR &&
        errno != EAGAIN && errno != EWOULDBLOCK) {
        return 0;
    }

    if (connect_result != 0) {
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(fd_, &write_fds);
        connect_result = ::select(fd_ + 1, 0, &write_fds, 0, &timeout);
        if (connect_result < 0 && errno != EINTR) {
            return 0;
        }
        if (connect_result <= 0) {
            return 0;
        }

        int socket_error = 0;
        socklen_t socket_error_length = sizeof(socket_error);
        if (::getsockopt(fd_,
                         SOL_SOCKET,
                         SO_ERROR,
                         &socket_error,
                         &socket_error_length) < 0) {
            return 0;
        }
        if (socket_error) {
            return 0;
        }
    }

    peer_ipv4_ = endpoint.sin_addr;
    peer_port_network_order_ = endpoint.sin_port;
    return 1;
}

// IDA_EA: 0x81AFCC8
// IDA_NAME: _ZNK5nexon4cash9TCPSocket13pollReadEventEv
// DEMANGLED: nexon::cash::TCPSocket::pollReadEvent(void)const
bool TCPSocket::pollReadEvent() const {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd_, &read_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return ::select(2, &read_fds, 0, 0, &timeout) != 0;
}

// IDA_EA: 0x81AFD94
// IDA_NAME: _ZNK5nexon4cash9TCPSocket14pollWriteEventEv
// DEMANGLED: nexon::cash::TCPSocket::pollWriteEvent(void)const
bool TCPSocket::pollWriteEvent() const {
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(fd_, &write_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return ::select(2, 0, &write_fds, 0, &timeout) != 0;
}

// IDA_EA: 0x81AFE60
// IDA_NAME: _ZNK5nexon4cash9TCPSocket14pollErrorEventEv
// DEMANGLED: nexon::cash::TCPSocket::pollErrorEvent(void)const
bool TCPSocket::pollErrorEvent() const {
    fd_set except_fds;
    FD_ZERO(&except_fds);
    FD_SET(fd_, &except_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return ::select(2, 0, 0, &except_fds, &timeout) != 0;
}

// IDA_EA: 0x81AFF2C
// IDA_NAME: _ZN5nexon4cash9TCPSocket6acceptERS1_
// DEMANGLED: nexon::cash::TCPSocket::accept(nexon::cash::TCPSocket&)
int TCPSocket::accept(TCPSocket& accepted_socket) {
    socklen_t endpoint_length = sizeof(accepted_socket.accepted_peer_endpoint_);
    accepted_socket.fd_ = ::accept(
        fd_,
        reinterpret_cast<sockaddr*>(&accepted_socket.accepted_peer_endpoint_),
        &endpoint_length);
    if (accepted_socket.fd_ < 0) {
        return 0;
    }

    accepted_socket.peer_ipv4_ = accepted_socket.accepted_peer_endpoint_.sin_addr;
    accepted_socket.peer_port_network_order_ =
        accepted_socket.accepted_peer_endpoint_.sin_port;
    accepted_socket.setOptNonBlock();
    return 1;
}

// IDA_EA: 0x81AFFB8
// IDA_NAME: _ZN5nexon4cash9TCPSocket11getPeerAdrsEv
// DEMANGLED: nexon::cash::TCPSocket::getPeerAdrs(void)
char* TCPSocket::getPeerAdrs() {
    return reinterpret_cast<char*>(&peer_ipv4_);
}

// IDA_EA: 0x81AFFC4
// IDA_NAME: _ZN5nexon4cash9TCPSocket11getPeerPortEv
// DEMANGLED: nexon::cash::TCPSocket::getPeerPort(void)
int TCPSocket::getPeerPort() {
    return peer_port_network_order_;
}

// IDA_EA: 0x81AFFD0
// IDA_NAME: _ZN5nexon4cash9TCPSocket19setOptResizeSendBufEi
// DEMANGLED: nexon::cash::TCPSocket::setOptResizeSendBuf(int)
bool TCPSocket::setOptResizeSendBuf(int size) {
    return size > 0 &&
           ::setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) >= 0;
}

// IDA_EA: 0x81B0036
// IDA_NAME: _ZN5nexon4cash9TCPSocket19setOptResizeRecvBufEi
// DEMANGLED: nexon::cash::TCPSocket::setOptResizeRecvBuf(int)
bool TCPSocket::setOptResizeRecvBuf(int size) {
    return size > 0 &&
           ::setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) >= 0;
}

// IDA_EA: 0x81B009C
// IDA_NAME: _ZN5nexon4cash9TCPSocket14SetRecvBufSizeEi
// DEMANGLED: nexon::cash::TCPSocket::SetRecvBufSize(int)
void TCPSocket::SetRecvBufSize(int size) {
    msRecvBufSize = size;
}

// IDA_EA: 0x81B00AA
// IDA_NAME: _ZN5nexon4cash9TCPSocket14SetSendBufSizeEi
// DEMANGLED: nexon::cash::TCPSocket::SetSendBufSize(int)
void TCPSocket::SetSendBufSize(int size) {
    msSendBufSize = size;
}

}  // namespace cash
}  // namespace nexon
