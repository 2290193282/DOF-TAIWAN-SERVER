#ifndef OLDSRV_INFRA_NETWORK_PROTOCOL_CLIENT_PROTOCOL_PROFILE_H_
#define OLDSRV_INFRA_NETWORK_PROTOCOL_CLIENT_PROTOCOL_PROFILE_H_

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"

namespace oldsrv {
namespace protocol {

// Central wire-policy boundary for the currently supported client family.
// Business dispatchers use semantic packet names; a future client extends
// this profile (or supplies another profile) instead of adding version names
// and wire exceptions to those business owners.
class ClientProtocolProfile {
public:
    enum {
        kWireCommandCount = ENUM_CMDPACKET_END,
        kWrappedCommandPacketClass = 2,
    };

    int wireCommandCount() const {
        return kWireCommandCount;
    }

    bool acceptsInboundCommandClass(ENUM_PACKETCLASS packet_class) const {
        return packet_class == ENUM_PACKETCLASS_CMDPACKET ||
               static_cast<unsigned int>(packet_class) ==
                   kWrappedCommandPacketClass;
    }

    bool isServerConsumedCommand(ENUM_PACKETCLASS packet_class,
                                 unsigned short packet_id) const {
        return acceptsInboundCommandClass(packet_class) &&
               packet_id == ENUM_CMDPACKET_SECURITY_STATUS;
    }
};

inline const ClientProtocolProfile& ActiveClientProtocolProfile() {
    static const ClientProtocolProfile profile;
    return profile;
}

}  // namespace protocol
}  // namespace oldsrv

#endif  // OLDSRV_INFRA_NETWORK_PROTOCOL_CLIENT_PROTOCOL_PROFILE_H_
