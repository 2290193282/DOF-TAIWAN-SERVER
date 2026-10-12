#ifndef OLDSRV_RUNTIME_NETWORK_ENDPOINT_IMPL_H_
#define OLDSRV_RUNTIME_NETWORK_ENDPOINT_IMPL_H_

#include "infra/network/transport/network_endpoint.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


template <int RecvSize, int SendSize>
void CNetwork<RecvSize, SendSize>::initialize_state() {
    socket_ = -1;
    reserved_04_ = 0;
    reserved_08_ = 0;
    reserved_0c_ = 0;
    reserved_10_ = 0;
    reserved_14_ = 0;
    memset(pad_15_17_, 0, sizeof(pad_15_17_));
    remain_recv_len_ = 0;
    memset(pad_1c_28_, 0, sizeof(pad_1c_28_));
    recv_flag_ = 0;
    memset(pad_2a_2b_, 0, sizeof(pad_2a_2b_));
    recv_cursor_ = recv_buffer_;
    remain_send_len_ = 0;
    send_cursor_ = send_buffer_;
    nat_type_ = 0;
    memset(nat_pad_, 0, sizeof(nat_pad_));
    ip_ = 0;
    memset(str_ip_, 0, sizeof(str_ip_));
    port_ = 0;
    memset(port_pad_, 0, sizeof(port_pad_));
    mtu_ = 0;
    inner_ip_ = 0;
    memset(mac_addr_, 0, sizeof(mac_addr_));
    memset(mac_pad_, 0, sizeof(mac_pad_));
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::lock_send() {
    return send_mutex_.lock();
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::unlock_send() {
    return send_mutex_.unlock();
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::receive_packet() {
    if (socket_ < 0) {
        return 0;
    }

    errno = 0;
    size_t nbytes = static_cast<size_t>(RecvSize - remain_recv_len_);
    if (!nbytes) {
        recv_cursor_ = recv_buffer_;
        remain_recv_len_ = 0;
        nbytes = RecvSize;
        recv_flag_ = 0;
    }

    ssize_t rc = read(socket_, recv_cursor_, nbytes);
    if (rc >= 0) {
        return rc ? static_cast<int>(rc) : -1;
    }
    if (NetworkErrnoIsNonBlocking()) {
        return 0;
    }
    NetworkLogError("recv_packet", "RECV ERROR DISCONNECT NOW", socket_);
    return -1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::flush_send_packet() {
    if (!remain_send_len_) {
        return 0;
    }

    lock_send();
    ssize_t written = write(socket_, send_buffer_, remain_send_len_);
    if (written > 0) {
        if (remain_send_len_ <= written) {
            if (remain_send_len_ < written) {
                fprintf(stderr,
                        "[Network.h:send_packet] offset error[Remain_Data: "
                        "%d Send:%d]\n",
                        remain_send_len_, static_cast<int>(written));
                unlock_send();
                return -1;
            }
            send_cursor_ = send_buffer_;
            remain_send_len_ = 0;
        } else {
            send_cursor_ = send_buffer_ + written;
            remain_send_len_ -= static_cast<int>(written);
            if (remain_send_len_ > SendSize) {
                fprintf(stderr,
                        "[Network.h:send_packet] m_remain_sendlen < "
                        "MAX_SEND_BUF : m_remain_sendlen:%d]\n",
                        remain_send_len_);
                unlock_send();
                return -1;
            }
            memmove(send_buffer_, send_cursor_, remain_send_len_);
            send_cursor_ = send_buffer_ + remain_send_len_;
        }
        unlock_send();
        return static_cast<int>(written);
    }

    if (NetworkErrnoIsNonBlocking()) {
        unlock_send();
        return 0;
    }

    NetworkLogError("send_packet", "SEND ERROR DISCONNECT NOW", socket_);
    unlock_send();
    return -1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::append_send_packet(char* src, int bytes) {
    if (socket_ < 0) {
        return -1;
    }
    if (bytes <= 0) {
        fprintf(stderr,
                "[Network.h:send_packet] !!!Send Packet[(%d,%d) Size(%d) "
                "Error\n",
                src ? src[0] : 0, src ? src[1] : 0, bytes);
        return -1;
    }

    errno = 0;
    lock_send();
    remain_send_len_ += bytes;
    if (remain_send_len_ > SendSize) {
        remain_send_len_ -= bytes;
        fprintf(stderr,
                "[Network.h:send_packet] !!!Send Packet Overflow P_TYPE[%d] "
                "Size:Remain[%d] Last[%d]\n",
                src ? src[1] : 0, remain_send_len_, bytes);
        unlock_send();
        return -1;
    }

    if (send_cursor_ >= send_buffer_ && send_cursor_ < send_buffer_ + SendSize) {
        memcpy(send_cursor_, src, bytes);
        send_cursor_ += bytes;
        unlock_send();
        return flush_send_packet();
    }

    remain_send_len_ -= bytes;
    fprintf(stderr,
            "[Network.h:send_packet] !!!Send Packet Buffer error P_TYPE[%d] "
            "Size:Remain[%d] Last[%d]\n",
            src ? src[1] : 0, remain_send_len_, bytes);
    unlock_send();
    return -1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::parse_server_packet(int received_bytes) {
    size_t parsing_len = static_cast<size_t>(received_bytes + remain_recv_len_);
    const size_t header_size = 10;

    if (parsing_len <= header_size - 1) {
        remain_recv_len_ += received_bytes;
        recv_cursor_ += received_bytes;
        fprintf(stderr,
                "[Network.h:Parsing_ServerPacket] (offset:%p - buf:%p) = "
                "remainlen:%d, Recv Size[%d]\n",
                recv_cursor_, recv_buffer_, remain_recv_len_, received_bytes);
        return 1;
    }

    do {
        if (remain_recv_len_) {
            recv_cursor_ -= remain_recv_len_;
        }

        const uint16_t packet_size = NetworkReadU16(recv_cursor_ + 2);
        if (packet_size <= header_size - 1 || packet_size > 0x1800) {
            fprintf(stderr,
                    "[Network.h:Parsing_ServerPacket] Recv Size[%d], Parsing "
                    "Packet Size[%u], recvbuf_offset[%p] is Too Large\n",
                    received_bytes, packet_size, recv_cursor_);
            recv_cursor_ = recv_buffer_;
            remain_recv_len_ = 0;
            return 0;
        }

        if (parsing_len < packet_size) {
            break;
        }

        NetworkPacketDispatch dispatch = GetServerPacketDispatch();
        if (dispatch) {
            dispatch(recv_cursor_, packet_size);
        }
        parsing_len -= packet_size;
        recv_cursor_ += packet_size;
        remain_recv_len_ = 0;

        if (!parsing_len) {
            recv_cursor_ = recv_buffer_;
            break;
        }
    } while (parsing_len > header_size - 1);

    if (parsing_len > 0) {
        if (parsing_len > 0x186A0) {
            return 0;
        }
        memmove(recv_buffer_, recv_cursor_, parsing_len);
        recv_cursor_ = recv_buffer_ + parsing_len;
        remain_recv_len_ = static_cast<int>(parsing_len);
    }
    return 1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::parse_auction_packet(int received_bytes) {
    size_t parsing_len = static_cast<size_t>(received_bytes + remain_recv_len_);
    const size_t header_size = 18;

    if (parsing_len <= header_size - 1) {
        remain_recv_len_ += received_bytes;
        recv_cursor_ += received_bytes;
        return 1;
    }

    while (true) {
        if (remain_recv_len_) {
            recv_cursor_ -= remain_recv_len_;
        }

        const uint8_t category = static_cast<uint8_t>(recv_cursor_[0]);
        const uint8_t packet_id = static_cast<uint8_t>(recv_cursor_[1]);
        const uint32_t packet_size = NetworkReadU32(recv_cursor_ + 2);
        if (packet_size <= header_size - 1 || packet_size > 0x5000) {
            fprintf(stderr,
                    "[Network.h:Parsing_AuctionServerPacket] Recv Size[%d], "
                    "Parsing Packet Size[%u] PacketHeader[%x] PacketType[%d] "
                    "is Too Large At Auction Packet\n",
                    received_bytes, packet_size, category, packet_id);
            recv_cursor_ = recv_buffer_;
            remain_recv_len_ = 0;
            return 0;
        }

        if (parsing_len < packet_size) {
            break;
        }

        NetworkPacketDispatch dispatch = GetAuctionPacketDispatch();
        if (dispatch) {
            dispatch(recv_cursor_, static_cast<int>(packet_size));
        }
        parsing_len -= packet_size;
        recv_cursor_ += packet_size;
        remain_recv_len_ = 0;

        if (!parsing_len) {
            recv_cursor_ = recv_buffer_;
            break;
        }
        if (parsing_len <= header_size - 1) {
            fprintf(stderr,
                    "[Network.h:Parsing_AuctionServerPacket] need more data "
                    "(parsinglength < HEADER_SIZE): body=%d !!\n",
                    static_cast<int>(parsing_len));
            break;
        }
    }

    if (parsing_len > 0) {
        if (parsing_len > 0x186A0) {
            return 0;
        }
        memmove(recv_buffer_, recv_cursor_, parsing_len);
        recv_cursor_ = recv_buffer_ + parsing_len;
        remain_recv_len_ = static_cast<int>(parsing_len);
    }
    return 1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::close_socket() {
    reserved_04_ = 0;
    reserved_08_ = 0;
    if (socket_ >= 0) {
        shutdown(socket_, SHUT_RD);
        close(socket_);
    }
    socket_ = -1;
    nat_type_ = 0;
    ip_ = 0;
    port_ = 0;
    inner_ip_ = 0;
    return 1;
}

template <int RecvSize, int SendSize>
int CNetwork<RecvSize, SendSize>::assign_socket(int fd, bool block) {
    socket_ = fd;
    if (!block) {
        NetworkSetNonBlock(socket_);
    }
    memset(str_ip_, 0, sizeof(str_ip_));
    return 1;
}

template <int RecvSize, int SendSize>
void* CNetwork<RecvSize, SendSize>::reset_network_buffers() {
    remain_recv_len_ = 0;
    recv_cursor_ = recv_buffer_;
    memset(recv_buffer_, 0, sizeof(recv_buffer_));
    remain_send_len_ = 0;
    send_cursor_ = send_buffer_;
    return memset(send_buffer_, 0, sizeof(send_buffer_));
}

template <int RecvSize, int SendSize>
char* CNetwork<RecvSize, SendSize>::peer_ip_string(char* out, int bytes) {
    static char empty[] = "";
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (out) {
        memset(out, 0, bytes);
    }
    if (!out || bytes <= 15) {
        return empty;
    }
    if (getpeername(socket_, reinterpret_cast<struct sockaddr*>(&addr), &len)) {
        return empty;
    }

    const char* src = inet_ntoa(addr.sin_addr);
    strncpy(out, src, 0x10u);
    return out;
}

template <int RecvSize, int SendSize>
char* CNetwork<RecvSize, SendSize>::set_nat_with_mtu(char nat_type,
                                                     uint32_t inner_ip,
                                                     uint32_t ip,
                                                     uint16_t port,
                                                     uint32_t mtu) {
    nat_type_ = static_cast<uint8_t>(nat_type);
    inner_ip_ = inner_ip;
    ip_ = ip;
    port_ = port;
    mtu_ = mtu;
    struct in_addr addr;
    addr.s_addr = ip_;
    strncpy(str_ip_, inet_ntoa(addr), 0x10u);
    return str_ip_;
}

template <int RecvSize, int SendSize>
char* CNetwork<RecvSize, SendSize>::set_nat_with_mac(char nat_type,
                                                     uint32_t inner_ip,
                                                     uint32_t ip,
                                                     uint16_t port,
                                                     const char* mac_addr) {
    if (mac_addr) {
        strncpy(mac_addr_, mac_addr, 0x41u);
    } else {
        memset(mac_addr_, 0, sizeof(mac_addr_));
    }
    nat_type_ = static_cast<uint8_t>(nat_type);
    inner_ip_ = inner_ip;
    ip_ = ip;
    port_ = port;
    struct in_addr addr;
    addr.s_addr = ip_;
    strncpy(str_ip_, inet_ntoa(addr), 0x10u);
    return str_ip_;
}


#endif  // OLDSRV_RUNTIME_NETWORK_ENDPOINT_IMPL_H_
