#include "infra/network/packet/packet_buf.h"

#include "infra/log/format/trace.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/security/crypto/cipher.h"
#include "infra/security/policy/packet_integrity.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/item_growth_state.h"
#include "old_game/world/region/game_world.h"

#include <cstring>
#include <zlib.h>

// IDA_EA: 0x80CB89E
// IDA_NAME: _ZN9PacketBuf8put_byteEi
// DEMANGLED: PacketBuf::put_byte(int)
int PacketBuf::put_byte(int value) {
    return put_byte(index_, value);
}

// IDA_EA: 0x80CB8C2
// IDA_NAME: _ZN9PacketBuf7put_intEi
// DEMANGLED: PacketBuf::put_int(int)
int PacketBuf::put_int(int value) {
    return put_int(index_, value);
}

// IDA_EA: 0x80D9E80
// IDA_NAME: _ZN9PacketBuf9put_shortEi
// DEMANGLED: PacketBuf::put_short(int)
int PacketBuf::put_short(int value) {
    return put_short(index_, value);
}

// IDA_EA: 0x8110B1C
// IDA_NAME: _ZN9PacketBuf9get_indexEv
// DEMANGLED: PacketBuf::get_index(void)
int PacketBuf::get_index() {
    return index_;
}

// IDA_EA: 0x811DEDC
// IDA_NAME: _ZN9PacketBuf10put_binaryEPci
// DEMANGLED: PacketBuf::put_binary(char*,int)
int PacketBuf::put_binary(char* data, int size) {
    return put_binary(index_, data, size);
}

// IDA_EA: 0x81252BA
// IDA_NAME: _ZN9PacketBuf9set_indexEi
// DEMANGLED: PacketBuf::set_index(int)
PacketBuf* PacketBuf::set_index(int index) {
    index_ = index;
    return this;
}

// IDA_EA: 0x815093E
// IDA_NAME: _ZN9PacketBuf10put_binaryEPKci
// DEMANGLED: PacketBuf::put_binary(char const*,int)
int PacketBuf::put_binary(const char* data, int size) {
    return put_binary(index_, data, size);
}

// IDA_EA: 0x81B73B8
// IDA_NAME: _ZN9PacketBuf7put_strEPci
// DEMANGLED: PacketBuf::put_str(char*,int)
int PacketBuf::put_str(char* data, int size) {
    return put_str(index_, data, size);
}

// IDA_EA: 0x822B684
// IDA_NAME: _ZN9PacketBuf7put_strEPKci
// DEMANGLED: PacketBuf::put_str(char const*,int)
int PacketBuf::put_str(const char* data, int size) {
    return put_str(index_, data, size);
}

// IDA_EA: 0x822B6B0
// IDA_NAME: _ZN9PacketBuf12put_item_idxEm
// DEMANGLED: PacketBuf::put_item_idx(unsigned long)
int PacketBuf::put_item_idx(unsigned long item_idx) {
    return put_int(index_, static_cast<int>(item_idx));
}

// IDA_EA: 0x822B6D4
// IDA_NAME: _ZN9PacketBuf2atEi
// DEMANGLED: PacketBuf::at(int)
int PacketBuf::at(int index) {
    return static_cast<unsigned char>(packet_[index]);
}

// IDA_EA: 0x822B6E8
// IDA_NAME: _ZN9PacketBuf12get_item_idxERm
// DEMANGLED: PacketBuf::get_item_idx(unsigned long&)
int PacketBuf::get_item_idx(unsigned long& item_idx) {
    unsigned int value = 0;
    const int ok = get_int(value);
    item_idx = value;
    return ok;
}

// IDA_EA: 0x822B702
// IDA_NAME: _ZN9PacketBuf10get_packetEi
// DEMANGLED: PacketBuf::get_packet(int)
char* PacketBuf::get_packet(int index) {
    if (index > PACKET_MAX_SIZE - 1) {
        cMyTrace trace("get_packet", 0x170, 5);
        trace("[%s][%d]", "get_packet", 0x170);
    }
    return packet_ + index;
}

// IDA_EA: 0x848F390
// IDA_NAME: _ZNK9PacketBuf13is_finallizedEv
// DEMANGLED: PacketBuf::is_finallized(void)const
bool PacketBuf::is_finallized() const {
    return finalized_;
}

// IDA_EA: 0x858C8C0
// IDA_NAME: _ZN9PacketBufC2Ev
// DEMANGLED: PacketBuf::PacketBuf(void)
PacketBuf::PacketBuf() : byte_encoding_(0) {
    clear();
}

// IDA_EA: 0x858C8DC
// IDA_NAME: _ZN9PacketBufD2Ev
// DEMANGLED: PacketBuf::~PacketBuf()
PacketBuf::~PacketBuf() {}

// IDA_EA: 0x858C8E2
// IDA_NAME: _ZN9PacketBuf5clearEv
// DEMANGLED: PacketBuf::clear(void)
int PacketBuf::clear() {
    index_ = 0;
    length_ = 0;
    finalized_ = false;
    packet_ = storage_;
    setLastError(PACKETBUF_ERROR_NONE);
    encrypt_required_ = true;
    return static_cast<int>(reinterpret_cast<intptr_t>(clear_ptr()));
}

// IDA_EA: 0x858C93A
// IDA_NAME: _ZN9PacketBuf9clear_ptrEv
// DEMANGLED: PacketBuf::clear_ptr(void)
PacketBuf* PacketBuf::clear_ptr() {
    index_ = PACKET_INDEX_BEGIN;
    return this;
}

// IDA_EA: 0x858C94A
// IDA_NAME: _ZN9PacketBuf10put_headerEii
// DEMANGLED: PacketBuf::put_header(int,int)
int PacketBuf::put_header(int packet_class, int packet_id) {
    SC_Header* const sc_header = reinterpret_cast<SC_Header*>(packet_);
    sc_header->cmd = static_cast<uint8_t>(packet_class);
    sc_header->type = static_cast<uint16_t>(packet_id);
    index_ = PACKET_HEADER_SIZE;
    return 1;
}

// IDA_EA: 0x858C978
// IDA_NAME: _ZN9PacketBuf8put_byteERii
// DEMANGLED: PacketBuf::put_byte(int&,int)
int PacketBuf::put_byte(int& index, int value) {
    if (is_ptr_end(index, 1)) {
        return 0;
    }
    packet_[index++] = static_cast<char>(value);
    return 1;
}

// IDA_EA: 0x858C9C8
// IDA_NAME: _ZN9PacketBuf9put_shortERii
// DEMANGLED: PacketBuf::put_short(int&,int)
int PacketBuf::put_short(int& index, int value) {
    if (is_ptr_end(index, 2)) {
        return 0;
    }
    if (byte_encoding_ == 0) {
        packet_[index++] = static_cast<char>(value & 0xFF);
        packet_[index++] = static_cast<char>((value >> 8) & 0xFF);
        return 1;
    }
    if (byte_encoding_ == 1) {
        packet_[index++] = static_cast<char>((value >> 8) & 0xFF);
        packet_[index++] = static_cast<char>(value & 0xFF);
        return 1;
    }
    LogManager::logFormat(1, "packet_buf.cpp",
                          "bool PacketBuf::put_short(int&, int)", 0xA6,
                          "m_byte_encoding error %d", byte_encoding_);
    return 0;
}

// IDA_EA: 0x858CAD4
// IDA_NAME: _ZN9PacketBuf7put_intERii
// DEMANGLED: PacketBuf::put_int(int&,int)
int PacketBuf::put_int(int& index, int value) {
    if (is_ptr_end(index, 4)) {
        return 0;
    }
    if (byte_encoding_ == 0) {
        packet_[index++] = static_cast<char>(value & 0xFF);
        packet_[index++] = static_cast<char>((value >> 8) & 0xFF);
        packet_[index++] = static_cast<char>((value >> 16) & 0xFF);
        packet_[index++] = static_cast<char>((value >> 24) & 0xFF);
        return 1;
    }
    if (byte_encoding_ == 1) {
        packet_[index++] = static_cast<char>((value >> 24) & 0xFF);
        packet_[index++] = static_cast<char>((value >> 16) & 0xFF);
        packet_[index++] = static_cast<char>((value >> 8) & 0xFF);
        packet_[index++] = static_cast<char>(value & 0xFF);
        return 1;
    }
    LogManager::logFormat(1, "packet_buf.cpp",
                          "bool PacketBuf::put_int(int&, int)", 0xC4,
                          "m_byte_encoding error %d", byte_encoding_);
    return 0;
}

// IDA_EA: 0x858CC68
// IDA_NAME: _ZN9PacketBuf7put_strERiPci
// DEMANGLED: PacketBuf::put_str(int&,char*,int)
int PacketBuf::put_str(int& index, char* data, int size) {
    if (is_ptr_end(index, size)) {
        return 0;
    }
    for (int i = 0; i < size && data[i] != 0; ++i) {
        packet_[index++] = data[i];
    }
    return 1;
}

// IDA_EA: 0x858CCE8
// IDA_NAME: _ZN9PacketBuf7put_strERiPKci
// DEMANGLED: PacketBuf::put_str(int&,char const*,int)
int PacketBuf::put_str(int& index, const char* data, int size) {
    if (is_ptr_end(index, size)) {
        return 0;
    }
    for (int i = 0; i < size && data[i] != 0; ++i) {
        packet_[index++] = data[i];
    }
    return 1;
}

// IDA_EA: 0x858CD68
// IDA_NAME: _ZN9PacketBuf10put_binaryERiPci
// DEMANGLED: PacketBuf::put_binary(int&,char*,int)
int PacketBuf::put_binary(int& index, char* data, int size) {
    if (data == 0 || size == 0) {
        LogManager::logFormat(1, "packet_buf.cpp",
                              "bool PacketBuf::put_binary(int&, char*, int)",
                              0x123, "bin is null or size is zero");
        return 0;
    }
    if (is_ptr_end(index, size)) {
        return 0;
    }
    for (int i = 0; i < size; ++i) {
        packet_[index++] = data[i];
    }
    return 1;
}

// IDA_EA: 0x858CE16
// IDA_NAME: _ZN9PacketBuf10put_binaryERiPKci
// DEMANGLED: PacketBuf::put_binary(int&,char const*,int)
int PacketBuf::put_binary(int& index, const char* data, int size) {
    if (data == 0 || size == 0) {
        LogManager::logFormat(
            1, "packet_buf.cpp",
            "bool PacketBuf::put_binary(int&, const char*, int)", 0x138,
            "bin is null or size is zero");
        return 0;
    }
    if (is_ptr_end(index, size)) {
        return 0;
    }
    for (int i = 0; i < size; ++i) {
        packet_[index++] = data[i];
    }
    return 1;
}

// IDA_EA: 0x858CEC4
// IDA_NAME: _ZN9PacketBuf10is_ptr_endEii
// DEMANGLED: PacketBuf::is_ptr_end(int,int)
int PacketBuf::is_ptr_end(int index, int size) {
    if (index + size <= PACKET_BUFFER_LIMIT) {
        return 0;
    }
    setLastError(PACKETBUF_ERROR_OVERFLOW);
    return 1;
}

// IDA_EA: 0x858CEFC
// IDA_NAME: _ZN9PacketBuf10is_len_endEi
// DEMANGLED: PacketBuf::is_len_end(int)
bool PacketBuf::is_len_end(int size) {
    return size + index_ > length_;
}

// IDA_EA: 0x858CF22
// IDA_NAME: _ZN9PacketBuf8get_byteERc
// DEMANGLED: PacketBuf::get_byte(char&)
int PacketBuf::get_byte(char& value) {
    if (is_len_end(1)) {
        return 0;
    }
    value = packet_[index_++];
    return 1;
}

// IDA_EA: 0x858CF70
// IDA_NAME: _ZN9PacketBuf8get_byteERh
// DEMANGLED: PacketBuf::get_byte(unsigned char&)
int PacketBuf::get_byte(unsigned char& value) {
    if (is_len_end(1)) {
        return 0;
    }
    value = static_cast<unsigned char>(packet_[index_++]);
    return 1;
}

// IDA_EA: 0x858CFC0
// IDA_NAME: _ZN9PacketBuf9get_shortERs
// DEMANGLED: PacketBuf::get_short(short&)
int PacketBuf::get_short(short& value) {
    if (is_len_end(2)) {
        return 0;
    }
    const int first = static_cast<unsigned char>(packet_[index_++]);
    const int second = static_cast<unsigned char>(packet_[index_++]);
    if (byte_encoding_ == 1) {
        value = static_cast<short>((first << 8) + second);
        return 1;
    }
    if (byte_encoding_ == 0) {
        value = static_cast<short>((second << 8) + first);
        return 1;
    }
    LogManager::logFormat(1, "packet_buf.cpp",
                          "bool PacketBuf::get_short(short int&)", 0x191,
                          "m_byte_encoding error %d", byte_encoding_);
    return 0;
}

// IDA_EA: 0x858D0B0
// IDA_NAME: _ZN9PacketBuf9get_shortERt
// DEMANGLED: PacketBuf::get_short(unsigned short&)
int PacketBuf::get_short(unsigned short& value) {
    short temp = 0;
    if (get_short(temp) != 1) {
        return 0;
    }
    value = static_cast<unsigned short>(temp);
    return 1;
}

// IDA_EA: 0x858D0F0
// IDA_NAME: _ZN9PacketBuf7get_intERi
// DEMANGLED: PacketBuf::get_int(int&)
int PacketBuf::get_int(int& value) {
    if (is_len_end(4)) {
        return 0;
    }
    const int b0 = static_cast<unsigned char>(packet_[index_++]);
    const int b1 = static_cast<unsigned char>(packet_[index_++]);
    const int b2 = static_cast<unsigned char>(packet_[index_++]);
    const int b3 = static_cast<unsigned char>(packet_[index_++]);
    if (byte_encoding_ == 1) {
        value = b3 + (b1 << 16) + (b0 << 24) + (b2 << 8);
        return 1;
    }
    if (byte_encoding_ == 0) {
        value = b0 + (b2 << 16) + (b3 << 24) + (b1 << 8);
        return 1;
    }
    LogManager::logFormat(1, "packet_buf.cpp",
                          "bool PacketBuf::get_int(int&)", 0x1B7,
                          "m_byte_encoding error %d", byte_encoding_);
    return 0;
}

// IDA_EA: 0x858D240
// IDA_NAME: _ZN9PacketBuf7get_intERj
// DEMANGLED: PacketBuf::get_int(unsigned int&)
int PacketBuf::get_int(unsigned int& value) {
    int temp = 0;
    if (get_int(temp) != 1) {
        return 0;
    }
    value = static_cast<unsigned int>(temp);
    return 1;
}

// IDA_EA: 0x858D27E
// IDA_NAME: _ZN9PacketBuf7get_intERm
// DEMANGLED: PacketBuf::get_int(unsigned long&)
int PacketBuf::get_int(unsigned long& value) {
    int temp = 0;
    if (get_int(temp) != 1) {
        return 0;
    }
    value = static_cast<unsigned long>(temp);
    return 1;
}

// IDA_EA: 0x858D2BC
// IDA_NAME: _ZN9PacketBuf7get_strEPcii
// DEMANGLED: PacketBuf::get_str(char*,int,int)
int PacketBuf::get_str(char* data, int capacity, int size) {
    if (data == 0) {
        LogManager::logFormat(
            1, "packet_buf.cpp", "bool PacketBuf::get_str(char*, int, int)",
            0x1F8, "o_buf is null");
        return 0;
    }
    if (capacity < 0 || size < 0 || size >= capacity) {
        return 0;
    }
    if (is_len_end(size)) {
        return 0;
    }
    if (size == 0) {
        return 1;
    }
    int i = 0;
    for (; i < size; ++i) {
        const char value = packet_[index_++];
        if (value == 0) {
            break;
        }
        data[i] = value;
    }
    data[i] = 0;
    return 1;
}

// IDA_EA: 0x858D3B2
// IDA_NAME: _ZN9PacketBuf10get_binaryEPci
// DEMANGLED: PacketBuf::get_binary(char*,int)
int PacketBuf::get_binary(char* data, int size) {
    if (data == 0 || size == 0) {
        LogManager::logFormat(
            1, "packet_buf.cpp", "bool PacketBuf::get_binary(char*, int)",
            0x21E, "o_buf == NULL || size == 0");
        return 0;
    }
    if (is_len_end(size)) {
        return 0;
    }
    for (int i = 0; i < size; ++i) {
        data[i] = packet_[index_++];
    }
    return 1;
}

// IDA_EA: 0x858D462
// IDA_NAME: _ZN9PacketBuf10set_packetEPKci
// DEMANGLED: PacketBuf::set_packet(char const*,int)
PacketBuf* PacketBuf::set_packet(const char* packet, int size) {
    if (packet == 0 || size <= 0 || size > PACKET_MAX_SIZE) {
        return this;
    }
    clear();
    std::memcpy(storage_, packet, static_cast<size_t>(size));
    length_ = size;
    return this;
}

// IDA_EA: 0x858D4DE
// IDA_NAME: _ZN9PacketBuf11bind_packetEPci
// DEMANGLED: PacketBuf::bind_packet(char*,int)
int PacketBuf::bind_packet(char* packet, int size) {
    if (packet == 0 || size == 0) {
        LogManager::logFormat(1, "packet_buf.cpp",
                              "bool PacketBuf::bind_packet(char*, int)",
                              0x27C,
                              "packet is null or size is zero");
        return 0;
    }
    clear();
    packet_ = packet;
    length_ = size;
    return 1;
}

// IDA_EA: 0x858D548
// IDA_NAME: _ZN9PacketBuf8finalizeEb
// DEMANGLED: PacketBuf::finalize(bool)
PacketBuf* PacketBuf::finalize(bool encrypt) {
    encrypt_required_ = encrypt;
    _NS_PI_MakeHash_NOTI(this);
    length_ = index_;
    const uint32_t crc_before =
        Encryption::crc32N(packet_ + PACKET_HEADER_SIZE,
                           length_ - PACKET_HEADER_SIZE);
    SC_Header* const sc_header = reinterpret_cast<SC_Header*>(packet_);

    if (byte_encoding_ == 0) {
        sc_header->length = static_cast<uint32_t>(length_);
    } else if (byte_encoding_ == 1) {
        packet_[3] = static_cast<char>(length_ >> 24);
        packet_[4] = static_cast<char>(length_ >> 16);
        packet_[5] = static_cast<char>(length_ >> 8);
        packet_[6] = static_cast<char>(length_);
    } else {
        LogManager::logFormat(1, "packet_buf.cpp", "void PacketBuf::finalize(bool)",
                              0x2C4, "m_byte_encoding error %d",
                              byte_encoding_);
#if defined(__i386__)
        intptr_t raw_result;
        __asm__ __volatile__("" : "=a"(raw_result) : : "memory");
        return reinterpret_cast<PacketBuf*>(raw_result);
#else
#error "PacketBuf::finalize preserves the old df_game_r i386 EAX failure return"
#endif
    }

    if (isCompressRequired(packet_ + 1)) {
        const intptr_t raw_compress_result =
            static_cast<intptr_t>(compress_packet() ^ 1);
        if (static_cast<unsigned char>(raw_compress_result) != 0) {
            return reinterpret_cast<PacketBuf*>(raw_compress_result);
        }
    }

    const uint32_t crc_after =
        Encryption::crc32N(packet_ + PACKET_HEADER_SIZE,
                           length_ - PACKET_HEADER_SIZE);
    if (byte_encoding_ == 0) {
        sc_header->checksum1 = crc_before;
        sc_header->checksum2 = crc_after;
    } else if (byte_encoding_ == 1) {
        packet_[7] = static_cast<char>(crc_before >> 24);
        packet_[8] = static_cast<char>(crc_before >> 16);
        packet_[9] = static_cast<char>(crc_before >> 8);
        packet_[10] = static_cast<char>(crc_before);
        packet_[11] = 0;
        packet_[12] = 0;
        packet_[13] = 0;
        packet_[14] = 0;
    } else {
        LogManager::logFormat(1, "packet_buf.cpp", "void PacketBuf::finalize(bool)",
                              0x2EC, "m_byte_encoding error %d",
                              byte_encoding_);
#if defined(__i386__)
        intptr_t raw_result;
        __asm__ __volatile__("" : "=a"(raw_result) : : "memory");
        return reinterpret_cast<PacketBuf*>(raw_result);
#else
#error "PacketBuf::finalize preserves the old df_game_r i386 EAX failure return"
#endif
    }

    finalized_ = true;
    return this;
}

// IDA_EA: 0x858D86A
// IDA_NAME: _ZN9PacketBuf14encrypt_packetEv
// DEMANGLED: PacketBuf::encrypt_packet(void)
void PacketBuf::encrypt_packet() {
    const SC_Header* const sc_header =
        reinterpret_cast<const SC_Header*>(packet_);
    const unsigned short packet_id = sc_header->type;
    if (sc_header->cmd == 0 && packet_id == 1) {
        uint32_t random_key = 0;
        Encryption::Encrypt(packet_ + PACKET_HEADER_SIZE,
                            length_ - PACKET_HEADER_SIZE, random_key);
        return;
    }

    unsigned char stack_buffer[PACKET_STORAGE_SIZE];
    unsigned char* output = stack_buffer;
    bool needs_delete = false;
    int adjusted_size = length_ - PACKET_HEADER_SIZE;
    G_Cipher()->AdjustDataSize(packet_id, &adjusted_size);
    if (adjusted_size > PACKET_STORAGE_SIZE) {
        LogManager::logFormat(
            1, "packet_buf.cpp", "void PacketBuf::encrypt_packet()", 0x302,
            "Buffer overflow[Cipher] : EncryptDataSize[%d]>[%d]",
            adjusted_size, PACKET_STORAGE_SIZE);
        output = new unsigned char[adjusted_size];
        needs_delete = true;
    }

    const int payload_size = length_ - PACKET_HEADER_SIZE;
    int output_capacity = PACKET_STORAGE_SIZE;
    G_Cipher()->Encrypt(packet_id,
                        reinterpret_cast<const unsigned char*>(
                            packet_ + PACKET_HEADER_SIZE),
                        payload_size, output, &output_capacity);
    length_ = PACKET_HEADER_SIZE + output_capacity;
    if (output_capacity > 0) {
        std::memcpy(packet_ + PACKET_HEADER_SIZE, output,
                    static_cast<size_t>(output_capacity));
    }
    if (needs_delete) {
        delete[] output;
    }
}

// IDA_EA: 0x858DA28
// IDA_NAME: _ZNK9PacketBuf17isEncryptRequiredEv
// DEMANGLED: PacketBuf::isEncryptRequired(void)const
bool PacketBuf::isEncryptRequired() const {
    return encrypt_required_;
}

// IDA_EA: 0x858DA38
// IDA_NAME: _ZN9PacketBuf11get_buf_ptrEi
// DEMANGLED: PacketBuf::get_buf_ptr(int)
char* PacketBuf::get_buf_ptr(int index) {
    return get_packet(index);
}

// IDA_EA: 0x858DA52
// IDA_NAME: _ZNK9PacketBuf7get_lenEv
// DEMANGLED: PacketBuf::get_len(void)const
int PacketBuf::get_len() const {
    if (finalized_) {
        return length_;
    }
    LogManager::logFormat(1, "packet_buf.cpp", "int PacketBuf::get_len() const",
                          0x338, "this packet is not finalized");
    return 0;
}

// IDA_EA: 0x858DAA2
// IDA_NAME: _ZN9PacketBuf15compress_packetEv
// DEMANGLED: PacketBuf::compress_packet(void)
int PacketBuf::compress_packet() {
    static Mutex compressed_buffer_mutex;
    static unsigned char compressed_buffer[PACKET_STORAGE_SIZE];
    const int payload_size = length_ - PACKET_HEADER_SIZE;
    if (payload_size <= 0) {
        LogManager::logFormat(
            1, "packet_buf.cpp", "bool PacketBuf::compress_packet()", 0x36E,
            "error; compress failed:unexpected message size");
        return 0;
    }

    uLongf out_len = static_cast<uLongf>(payload_size + 13);
    compressed_buffer_mutex.lock();
    const int rc = compress(
        compressed_buffer, &out_len,
        reinterpret_cast<const Bytef*>(packet_ + PACKET_HEADER_SIZE),
        static_cast<uLong>(payload_size));
    if (out_len <= PACKET_STORAGE_SIZE - PACKET_HEADER_SIZE) {
        std::memcpy(packet_ + PACKET_HEADER_SIZE, compressed_buffer,
                    static_cast<size_t>(out_len));
        compressed_buffer_mutex.unlock();
        length_ = static_cast<int>(out_len) + PACKET_HEADER_SIZE;
        SC_Header* const sc_header = reinterpret_cast<SC_Header*>(packet_);
        sc_header->length = static_cast<uint32_t>(length_);
        return 1;
    }
    LogManager::logFormat(1, "packet_buf.cpp",
                          "bool PacketBuf::compress_packet()", 0x380,
                          "error; compress failed:%d", rc);
    compressed_buffer_mutex.unlock();
    return 0;
}

// IDA_EA: 0x858DC2E
// IDA_NAME: _ZN9PacketBuf10put_packetERK10Inven_Item
// DEMANGLED: PacketBuf::put_packet(Inven_Item const&)
int PacketBuf::put_packet(const Inven_Item& item) {
    // CLIENT_IDA_EVIDENCE: CMD27 0x57D2E7 and NOTI40 0x5A140C consume this
    // value before the shared random-option parser.  It is the persisted
    // ChronicleRandomOptionInfo::value_ field at item offset +0x21.
    // 该字段属于共享 Inven_Item 序列化 owner，不能由调用方用 0 填充。
    put_int(static_cast<int>(item.chronicle_random_option_.value_));
    item.chronicle_random_option_.random_option()
        .put_packet_random_option(*this);

    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        !G_CDataManager()->GetUpgradeRevisionPvPChannel()) {
        put_byte(0);
    } else {
        put_byte(item.upgrade_separate_.GetUpgradeSeparate());
    }
    put_byte(item.upgrade_separate_.IsTradeRestriction());

    // CLIENT_IDA_EVIDENCE: BuildItemDetailFromPacket @ 0x896B46 stores the
    // u16 as per-item equipment-growth progress; NOTI477 @ 0x5A61D0 clamps
    // it to the current PVF growth-stage maximum.  The following u8 is the
    // current growth charge / maintained-fatigue value displayed at
    // 0x11BF29C.  The 0627 Inven_Item ABI remains unchanged: DB load owners
    // preload the stable ten-byte GUID sidecar, while this game-thread owner
    // performs a cache-only lookup.  A missing/zero GUID retains the safe
    // wire fallback; ReservedCapacity is deliberately not width-guessed.
    item_growth_state::State growth;
    item_growth_state::Lookup(item.globally_unique_id_, &growth);
    put_short(growth.progress);
    return put_byte(growth.fatigue_state);
}

// IDA_EA: 0x858DCD2
// IDA_NAME: _ZN9PacketBuf4copyERKS_
// DEMANGLED: PacketBuf::copy(PacketBuf const&)
void PacketBuf::copy(const PacketBuf& rhs) {
    index_ = rhs.index_;
    length_ = rhs.length_;
    finalized_ = rhs.finalized_;
    packet_ = storage_;
    std::memset(storage_, 0, sizeof(storage_));
    std::memcpy(storage_, rhs.storage_, static_cast<size_t>(length_));
}

// IDA_EA: 0x858DF30
// IDA_NAME: _ZN9PacketBuf11encFinalizeEv
// DEMANGLED: PacketBuf::encFinalize(void)
unsigned char* PacketBuf::encFinalize() {
    const uint32_t crc_before =
        Encryption::crc32N(packet_ + PACKET_HEADER_SIZE,
                           length_ - PACKET_HEADER_SIZE);
    SC_Header* const sc_header = reinterpret_cast<SC_Header*>(packet_);
    if (byte_encoding_ == 0) {
        sc_header->length = static_cast<uint32_t>(length_);
    } else if (byte_encoding_ == 1) {
        packet_[3] = static_cast<char>(length_ >> 24);
        packet_[4] = static_cast<char>(length_ >> 16);
        packet_[5] = static_cast<char>(length_ >> 8);
        packet_[6] = static_cast<char>(length_);
    } else {
        LogManager::logFormat(1, "packet_buf.cpp", "void PacketBuf::encFinalize()",
                              0x419, "m_byte_encoding error %d",
                              byte_encoding_);
#if defined(__i386__)
        intptr_t raw_result;
        __asm__ __volatile__("" : "=a"(raw_result) : : "memory");
        return reinterpret_cast<unsigned char*>(raw_result);
#else
#error "PacketBuf::encFinalize preserves the old df_game_r i386 EAX failure return"
#endif
    }

    const uint32_t crc_after =
        Encryption::crc32N(packet_ + PACKET_HEADER_SIZE,
                           length_ - PACKET_HEADER_SIZE);
    if (byte_encoding_ == 0) {
        sc_header->checksum1 = crc_before;
        sc_header->checksum2 = crc_after;
        return reinterpret_cast<unsigned char*>(
            static_cast<uintptr_t>(crc_after >> 24));
    } else if (byte_encoding_ == 1) {
        packet_[7] = static_cast<char>(crc_before >> 24);
        packet_[8] = static_cast<char>(crc_before >> 16);
        packet_[9] = static_cast<char>(crc_before >> 8);
        packet_[10] = static_cast<char>(crc_before);
        packet_[11] = 0;
        packet_[12] = 0;
        packet_[13] = 0;
        packet_[14] = 0;
        return reinterpret_cast<unsigned char*>(packet_ + 14);
    }

    LogManager::logFormat(1, "packet_buf.cpp", "void PacketBuf::encFinalize()",
                          0x439, "m_byte_encoding error %d", byte_encoding_);
#if defined(__i386__)
    intptr_t raw_result;
    __asm__ __volatile__("" : "=a"(raw_result) : : "memory");
    return reinterpret_cast<unsigned char*>(raw_result);
#else
#error "PacketBuf::encFinalize preserves the old df_game_r i386 EAX failure return"
#endif
}

// IDA_EA: 0x858E248
// IDA_NAME: _ZNK9PacketBuf18isCompressRequiredEPc
// DEMANGLED: PacketBuf::isCompressRequired(char*)const
bool PacketBuf::isCompressRequired(char* packet_id) const {
    const unsigned short id = *reinterpret_cast<const unsigned short*>(packet_id);
    return id == 2 || id == 9 || id == 87;
}

// IDA_EA: 0x858E286
// IDA_NAME: _ZN9PacketBuf12setLastErrorENS_20PACKETBUF_ERROR_TYPEE
// DEMANGLED: PacketBuf::setLastError(PacketBuf::PACKETBUF_ERROR_TYPE)
PacketBuf* PacketBuf::setLastError(PACKETBUF_ERROR_TYPE error) {
    last_error_ = error;
    return this;
}

// IDA_EA: 0x86D18E2
// IDA_NAME: _ZN9PacketBuf12getLastErrorEv
// DEMANGLED: PacketBuf::getLastError(void)
int PacketBuf::getLastError() {
    return last_error_;
}
