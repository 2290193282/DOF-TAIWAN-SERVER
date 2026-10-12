#include "infra/network/packet/interface_packet_buf.h"


// IDA_EA: 0x80CB8E6
// IDA_NAME: _ZN18InterfacePacketBuf5clearEv
// DEMANGLED: InterfacePacketBuf::clear(void)
int InterfacePacketBuf::clear() {
    return packet_->clear();
}

// IDA_EA: 0x80CB8FC
// IDA_NAME: _ZN18InterfacePacketBuf10put_headerEii
// DEMANGLED: InterfacePacketBuf::put_header(int,int)
int InterfacePacketBuf::put_header(int packet_class, int packet_id) {
    return packet_->put_header(packet_class, packet_id);
}

// IDA_EA: 0x80CB920
// IDA_NAME: _ZN18InterfacePacketBuf8put_byteEi
// DEMANGLED: InterfacePacketBuf::put_byte(int)
int InterfacePacketBuf::put_byte(int value) {
    return packet_->put_byte(value);
}

// IDA_EA: 0x80CB93C
// IDA_NAME: _ZN18InterfacePacketBuf7put_intEi
// DEMANGLED: InterfacePacketBuf::put_int(int)
int InterfacePacketBuf::put_int(int value) {
    return packet_->put_int(value);
}

// IDA_EA: 0x80CB958
// IDA_NAME: _ZN18InterfacePacketBuf8finalizeEb
// DEMANGLED: InterfacePacketBuf::finalize(bool)
int InterfacePacketBuf::finalize(bool encrypt) {
    return static_cast<int>(reinterpret_cast<intptr_t>(packet_->finalize(encrypt)));
}

// IDA_EA: 0x80D9EA4
// IDA_NAME: _ZN18InterfacePacketBuf9put_shortEi
// DEMANGLED: InterfacePacketBuf::put_short(int)
int InterfacePacketBuf::put_short(int value) {
    return packet_->put_short(value);
}

// IDA_EA: 0x8110B28
// IDA_NAME: _ZN18InterfacePacketBuf8put_byteERii
// DEMANGLED: InterfacePacketBuf::put_byte(int&,int)
int InterfacePacketBuf::put_byte(int& index, int value) {
    return packet_->put_byte(index, value);
}

// IDA_EA: 0x8110B4C
// IDA_NAME: _ZN18InterfacePacketBuf9get_indexEv
// DEMANGLED: InterfacePacketBuf::get_index(void)
int InterfacePacketBuf::get_index() const {
    return packet_->get_index();
}

// IDA_EA: 0x8116908
// IDA_NAME: _ZN18InterfacePacketBuf9put_shortERii
// DEMANGLED: InterfacePacketBuf::put_short(int&,int)
int InterfacePacketBuf::put_short(int& index, int value) {
    return packet_->put_short(index, value);
}

// IDA_EA: 0x811DF08
// IDA_NAME: _ZN18InterfacePacketBuf10put_binaryEPci
// DEMANGLED: InterfacePacketBuf::put_binary(char*,int)
int InterfacePacketBuf::put_binary(char* data, int size) {
    return packet_->put_binary(data, size);
}

// IDA_EA: 0x815096A
// IDA_NAME: _ZN18InterfacePacketBuf10put_binaryEPKci
// DEMANGLED: InterfacePacketBuf::put_binary(char const*,int)
int InterfacePacketBuf::put_binary(const char* data, int size) {
    return packet_->put_binary(data, size);
}

// IDA_EA: 0x815098E
// IDA_NAME: _ZN18InterfacePacketBuf10put_packetERK10Inven_Item
// DEMANGLED: InterfacePacketBuf::put_packet(Inven_Item const&)
int InterfacePacketBuf::put_packet(const Inven_Item& item) {
    return packet_->put_packet(item);
}

// IDA_EA: 0x81B73E4
// IDA_NAME: _ZN18InterfacePacketBuf7put_strEPci
// DEMANGLED: InterfacePacketBuf::put_str(char*,int)
int InterfacePacketBuf::put_str(char* data, int size) {
    return packet_->put_str(data, size);
}

// IDA_EA: 0x822B766
// IDA_NAME: _ZNK18InterfacePacketBuf3getEv
// DEMANGLED: InterfacePacketBuf::get(void)const
PacketBuf* InterfacePacketBuf::get() const {
    return packet_;
}

// IDA_EA: 0x822B770
// IDA_NAME: _ZN18InterfacePacketBuf7put_strEPKci
// DEMANGLED: InterfacePacketBuf::put_str(char const*,int)
int InterfacePacketBuf::put_str(const char* data, int size) {
    return packet_->put_str(data, size);
}

// IDA_EA: 0x822B794
// IDA_NAME: _ZN18InterfacePacketBuf12put_item_idxEm
// DEMANGLED: InterfacePacketBuf::put_item_idx(unsigned long)
int InterfacePacketBuf::put_item_idx(unsigned long item_idx) {
    return packet_->put_item_idx(item_idx);
}

// IDA_EA: 0x822B7B0
// IDA_NAME: _ZN18InterfacePacketBuf9set_indexEi
// DEMANGLED: InterfacePacketBuf::set_index(int)
PacketBuf* InterfacePacketBuf::set_index(int index) {
    return packet_->set_index(index);
}

// IDA_EA: 0x848F39C
// IDA_NAME: _ZN18InterfacePacketBuf11bind_packetEPci
// DEMANGLED: InterfacePacketBuf::bind_packet(char*,int)
int InterfacePacketBuf::bind_packet(char* packet, int size) {
    return packet_->bind_packet(packet, size);
}

// IDA_EA: 0x848F3C0
// IDA_NAME: _ZN18InterfacePacketBuf9get_shortERs
// DEMANGLED: InterfacePacketBuf::get_short(short&)
int InterfacePacketBuf::get_short(short& value) {
    return packet_->get_short(value);
}

// IDA_EA: 0x848F3DC
// IDA_NAME: _ZN18InterfacePacketBuf7get_intERi
// DEMANGLED: InterfacePacketBuf::get_int(int&)
int InterfacePacketBuf::get_int(int& value) {
    return packet_->get_int(value);
}

// IDA_EA: 0x848F3F8
// IDA_NAME: _ZN18InterfacePacketBuf10get_binaryEPci
// DEMANGLED: InterfacePacketBuf::get_binary(char*,int)
int InterfacePacketBuf::get_binary(char* data, int size) {
    return packet_->get_binary(data, size);
}

// IDA_EA: 0x848F41C
// IDA_NAME: _ZN18InterfacePacketBuf10get_packetEi
// DEMANGLED: InterfacePacketBuf::get_packet(int)
char* InterfacePacketBuf::get_packet(int index) {
    return packet_->get_packet(index);
}

// IDA_EA: 0x848F438
// IDA_NAME: _ZNK18InterfacePacketBuf7get_lenEv
// DEMANGLED: InterfacePacketBuf::get_len(void)const
int InterfacePacketBuf::get_len() const {
    return packet_->get_len();
}

// IDA_EA: 0x848F44E
// IDA_NAME: _ZNK18InterfacePacketBuf13is_finallizedEv
// DEMANGLED: InterfacePacketBuf::is_finallized(void)const
bool InterfacePacketBuf::is_finallized() const {
    return packet_->is_finallized();
}

// IDA_EA: 0x84B804A
// IDA_NAME: _ZN18InterfacePacketBuf7put_intERii
// DEMANGLED: InterfacePacketBuf::put_int(int&,int)
int InterfacePacketBuf::put_int(int& index, int value) {
    return packet_->put_int(index, value);
}

// IDA_EA: 0x858E294
// IDA_NAME: _ZN18InterfacePacketBufC2Ev
// DEMANGLED: InterfacePacketBuf::InterfacePacketBuf(void)
InterfacePacketBuf::InterfacePacketBuf() : packet_(0) {}

// IDA_EA: 0x858E29A
// IDA_NAME: _ZN18InterfacePacketBufD2Ev
// DEMANGLED: InterfacePacketBuf::~InterfacePacketBuf()
InterfacePacketBuf::~InterfacePacketBuf() {}

// IDA_EA: 0x858E2A0
// IDA_NAME: _ZN18InterfacePacketBuf16AcquirePacketBufEP9PacketBuf
// DEMANGLED: InterfacePacketBuf::AcquirePacketBuf(PacketBuf*)
InterfacePacketBuf* InterfacePacketBuf::AcquirePacketBuf(PacketBuf* packet) {
    packet_ = packet;
    return this;
}

// IDA_EA: 0x86D18EE
// IDA_NAME: _ZN18InterfacePacketBuf12getLastErrorEv
// DEMANGLED: InterfacePacketBuf::getLastError(void)
int InterfacePacketBuf::getLastError() const {
    return packet_->getLastError();
}
