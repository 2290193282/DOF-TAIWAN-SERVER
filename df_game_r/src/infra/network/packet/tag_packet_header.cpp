#include "infra/network/packet/PacketHeader.h"

// IDA_EA: 0x8106BBC
// IDA_NAME: _ZN15tagPacketHeader11setPacketIDEi
// DEMANGLED: tagPacketHeader::setPacketID(int)
tagPacketHeader* tagPacketHeader::setPacketID(int packet_id_arg) {
    packet_id = static_cast<uint8_t>(packet_id_arg);
    return this;
}

// IDA_EA: 0x8106BCC
// IDA_NAME: _ZN15tagPacketHeader7getSizeEv
// DEMANGLED: tagPacketHeader::getSize(void)
int tagPacketHeader::getSize() {
    return static_cast<int>(size);
}

// IDA_EA: 0x8106BD8
// IDA_NAME: _ZN15tagPacketHeader7setSizeEi
// DEMANGLED: tagPacketHeader::setSize(int)
tagPacketHeader* tagPacketHeader::setSize(int packet_size) {
    size = static_cast<uint32_t>(packet_size);
    return this;
}

// IDA_EA: 0x8106BE6
// IDA_NAME: _ZN15tagPacketHeader11setCategoryEi
// DEMANGLED: tagPacketHeader::setCategory(int)
tagPacketHeader* tagPacketHeader::setCategory(int category_arg) {
    category = static_cast<uint8_t>(category_arg);
    return this;
}

// IDA_EA: 0x8106BF6
// IDA_NAME: _ZN15tagPacketHeaderC2Ev
// DEMANGLED: tagPacketHeader::tagPacketHeader(void)
tagPacketHeader::tagPacketHeader() {}
