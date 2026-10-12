#ifndef OLDSRV_PROTOCOL_PACKET_BUF_H_
#define OLDSRV_PROTOCOL_PACKET_BUF_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

struct Inven_Item;

class PacketBuf {
public:
    enum PACKETBUF_ERROR_TYPE {
        PACKETBUF_ERROR_NONE = 0,
        PACKETBUF_ERROR_OVERFLOW = 1,
    };

    enum {
        PACKET_INDEX_BEGIN = static_cast<int>(sizeof(CS_Header)),
        PACKET_HEADER_SIZE = static_cast<int>(sizeof(SC_Header)),
        PACKET_BUFFER_LIMIT = 94900,
        PACKET_MAX_SIZE = 95000,
        PACKET_STORAGE_SIZE = 95000,
    };

    PacketBuf();
    ~PacketBuf();

    int put_byte(int value);
    int put_int(int value);
    int put_short(int value);
    int get_index();
    int put_binary(char* data, int size);
    PacketBuf* set_index(int index);
    int put_binary(const char* data, int size);
    int put_str(char* data, int size);
    int put_str(const char* data, int size);
    int put_item_idx(unsigned long item_idx);
    int at(int index);
    int get_item_idx(unsigned long& item_idx);
    char* get_packet(int index);
    bool is_finallized() const;
    int clear();
    PacketBuf* clear_ptr();
    int put_header(int packet_class, int packet_id);
    int put_byte(int& index, int value);
    int put_short(int& index, int value);
    int put_int(int& index, int value);
    int put_str(int& index, char* data, int size);
    int put_str(int& index, const char* data, int size);
    int put_binary(int& index, char* data, int size);
    int put_binary(int& index, const char* data, int size);
    int is_ptr_end(int index, int size);
    bool is_len_end(int size);
    int get_byte(char& value);
    int get_byte(unsigned char& value);
    int get_short(short& value);
    int get_short(unsigned short& value);
    int get_int(int& value);
    int get_int(unsigned int& value);
    int get_int(unsigned long& value);
    int get_str(char* data, int capacity, int size);
    int get_binary(char* data, int size);
    PacketBuf* set_packet(const char* packet, int size);
    int bind_packet(char* packet, int size);
    PacketBuf* finalize(bool encrypt);
    void encrypt_packet();
    bool isEncryptRequired() const;
    char* get_buf_ptr(int index);
    int get_len() const;
    // C2S readers bind a complete frame without finalizing it.  Diagnostics
    // must inspect that bound extent without calling get_len(), whose retail
    // contract is intentionally limited to finalized S2C packets.
    int get_bound_length() const { return length_; }
    int compress_packet();
    int put_packet(const Inven_Item& item);
    void copy(const PacketBuf& rhs);
    unsigned char* encFinalize();
    bool isCompressRequired(char* packet_id) const;
    PacketBuf* setLastError(PACKETBUF_ERROR_TYPE error);
    int getLastError();

private:
    int byte_encoding_;
    int index_;
    int length_;
    bool finalized_;
    unsigned char align_after_finalized_[3];
    int last_error_;
    char* packet_;
    char storage_[PACKET_STORAGE_SIZE];
    bool encrypt_required_;
};

static_assert(PacketBuf::PACKET_INDEX_BEGIN == static_cast<int>(sizeof(CS_Header)),
              "PacketBuf C2S read cursor must match CS_Header");
static_assert(PacketBuf::PACKET_HEADER_SIZE == static_cast<int>(sizeof(SC_Header)),
              "PacketBuf S2C write cursor must match SC_Header");

#if defined(__i386__)
static_assert(sizeof(PacketBuf) == 0x17334,
              "PacketBuf must retain the old 32-bit df_game_r layout");
#endif


#endif  // OLDSRV_PROTOCOL_PACKET_BUF_H_
