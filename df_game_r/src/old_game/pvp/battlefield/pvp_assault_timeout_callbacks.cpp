#include "old_game/pvp/battlefield/pvp_assault.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_guard.h"

enum ENUM_ERROR : int {};

namespace pvp_assault {

// IDA_EA: 0x82EF320
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr21OnTimeoutCompleteLoadEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutCompleteLoad(unsigned int,int)
int CAssaultMgr::OnTimeoutCompleteLoad(unsigned int assault_place_id,
                                       int authen_value) {
    _GetAssaultPlace(assault_place_id, authen_value)->TimeoutCompleteLoad();
    return 1;
}

// IDA_EA: 0x82EF86E
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr33OnTimeoutCompleteLoadAfterAssaultEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutCompleteLoadAfterAssault(
//     unsigned int,int)
int CAssaultMgr::OnTimeoutCompleteLoadAfterAssault(unsigned int party_index,
                                                    int timer_key) {
    try {
        CParty* const party = G_CGameManager()->GetParty(
            static_cast<int>(party_index));
        if (!party) {
            throw static_cast<ENUM_ERROR>(1);
        }
        if (!party->check_timer_key(static_cast<TIMER_MESSAGE>(42),
                                    timer_key)) {
            throw static_cast<ENUM_ERROR>(23);
        }
        if (party->TimeoutCompleteAfterAssault()) {
            PacketGuard packet;
            packet.clear();
            packet.put_header(1, ENUM_CMDPACKET_LOAD_COMPLETE_AFTER_ASSAULT);
            packet.put_byte(1);
            packet.finalize(true);
            party->send_to_party(packet);
        }
    } catch (ENUM_ERROR) {
    }
    return 1;
}

// IDA_EA: 0x82EFA52
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr19OnTimeoutConnectP2PEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutConnectP2P(unsigned int,int)
int CAssaultMgr::OnTimeoutConnectP2P(unsigned int assault_place_id,
                                     int authen_value) {
    _GetAssaultPlace(assault_place_id, authen_value)->TimeoutConnectP2P();
    return 1;
}

// IDA_EA: 0x82EFAA8
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr19OnTimeoutEndAssaultEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutEndAssault(unsigned int,int)
int CAssaultMgr::OnTimeoutEndAssault(unsigned int assault_place_id,
                                     int authen_value) {
    _GetAssaultPlace(assault_place_id, authen_value)->TimeoutEndAssault();
    return 1;
}

// IDA_EA: 0x82EFAFE
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr21OnTimeoutCloseAssaultEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutCloseAssault(unsigned int,int)
int CAssaultMgr::OnTimeoutCloseAssault(unsigned int assault_place_id,
                                       int authen_value) {
    _GetAssaultPlace(assault_place_id, authen_value)->TimeoutCloseAssault();
    return 1;
}

// IDA_EA: 0x82EFA0C
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr31OnTimeoutConnectP2PAfterAssaultEji
// DEMANGLED: pvp_assault::CAssaultMgr::OnTimeoutConnectP2PAfterAssault(unsigned int,int)
int CAssaultMgr::OnTimeoutConnectP2PAfterAssault(unsigned int, int) {
    CParty* const party = G_CGameManager()->GetParty();
    if (party && party->TimeoutCompleteAfterAssault()) {
        party->ClearBadP2PUser();
    }
    return 1;
}

}  // namespace pvp_assault
