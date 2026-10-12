#ifndef OLDSRV_PROTOCOL_INTERFACE_PACKET_BUF_H_
#define OLDSRV_PROTOCOL_INTERFACE_PACKET_BUF_H_

#include "infra/network/packet/packet_buf.h"


class InterfacePacketBuf {
public:
    InterfacePacketBuf();
    ~InterfacePacketBuf();

    int clear();
    int put_header(int packet_class, int packet_id);
    int put_byte(int value);
    int put_int(int value);
    int finalize(bool encrypt);
    int put_short(int value);
    int put_byte(int& index, int value);
    int get_index() const;
    int put_short(int& index, int value);
    int put_binary(char* data, int size);
    int put_binary(const char* data, int size);
    int put_packet(const Inven_Item& item);
    int put_str(char* data, int size);
    PacketBuf* get() const;
    int put_str(const char* data, int size);
    int put_item_idx(unsigned long item_idx);
    PacketBuf* set_index(int index);
    int bind_packet(char* packet, int size);
    int get_short(short& value);
    int get_int(int& value);
    int get_binary(char* data, int size);
    char* get_packet(int index);
    int get_len() const;
    bool is_finallized() const;
    int put_int(int& index, int value);
    InterfacePacketBuf* AcquirePacketBuf(PacketBuf* packet);
    int getLastError() const;

protected:
    PacketBuf* packet_;
};


#endif  // OLDSRV_PROTOCOL_INTERFACE_PACKET_BUF_H_
