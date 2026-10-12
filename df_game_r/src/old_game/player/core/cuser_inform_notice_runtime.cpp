#include "old_game/player/core/cuser.h"

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>
#include <vector>

// IDA_EA: 0x868FA8A
// IDA_NAME: _ZN5CUser24getInformNoticeFlagIndexERSt6vectorIhSaIhEE
// DEMANGLED: CUser::getInformNoticeFlagIndex(std::vector<unsigned char,std::allocator<unsigned char>> &)
bool CUser::getInformNoticeFlagIndex(std::vector<uint8_t>& indexes) {
    indexes.clear();
    for (int byte_index = 0; byte_index <= 15; ++byte_index) {
        const uint8_t flags =
            static_cast<uint8_t>(getInformNoticeFlag(byte_index));
        for (int bit = 0; bit <= 7; ++bit) {
            if (((flags >> bit) & 1u) != 0) {
                indexes.push_back(
                    static_cast<uint8_t>(8 * byte_index + bit));
            }
        }
    }
    return false;
}

// IDA_EA: 0x868FB22
// IDA_NAME: _ZN5CUser20sendInformNoticeFlagEv
// DEMANGLED: CUser::sendInformNoticeFlag(void)
int CUser::sendInformNoticeFlag() {
    std::vector<uint8_t> indexes;
    getInformNoticeFlagIndex(indexes);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INFORM_NOTICE);
    packet.put_byte(static_cast<int>(indexes.size()));
    for (std::vector<uint8_t>::size_type index = 0; index < indexes.size();
         ++index) {
        packet.put_byte(indexes[index]);
    }
    packet.finalize(true);
    Send(packet);
    return 0;
}
