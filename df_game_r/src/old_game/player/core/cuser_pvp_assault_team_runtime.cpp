#include "old_game/player/core/cuser.h"

#include <cstring>

#include "old_game/dungeon/instance/packet_dispatcher_req_peer.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x868056A
// IDA_NAME: _ZN5CUser18GetUserListByPartyERSt6vectorIPS_SaIS1_EE
// DEMANGLED: CUser::GetUserListByParty(std::vector<CUser*>&)
CParty* CUser::GetUserListByParty(std::vector<CUser*>& users) {
    users.push_back(this);

    CParty* const party = GetParty();
    if (!party) return 0;

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party->get_user(seat);
        if (member && member != this) users.push_back(member);
    }
    return party;
}

// IDA_EA: 0x86863B6
// IDA_NAME: _ZN15cUserHistoryLog8StartPvPE23ENUM_PVP_HISTORY_REASONPKcS2_S2_S2_S2_S2_S2_S2_
// DEMANGLED: cUserHistoryLog::StartPvP(ENUM_PVP_HISTORY_REASON,char const*,...)
int cUserHistoryLog::StartPvP(ENUM_PVP_HISTORY_REASON reason,
                              const char* team0_name0,
                              const char* team0_name1,
                              const char* team0_name2,
                              const char* team0_name3,
                              const char* team1_name0,
                              const char* team1_name1,
                              const char* team1_name2,
                              const char* team1_name3) {
    owner_->LogHistory("PVP,%d,\"%s\",\"%s\",\"%s\",\"%s\"",
                       static_cast<int>(reason),
                       team0_name0,
                       team0_name1,
                       team0_name2,
                       team0_name3);
    return owner_->LogHistory("PVP,%d,\"%s\",\"%s\",\"%s\",\"%s\"",
                              static_cast<int>(reason),
                              team1_name0,
                              team1_name1,
                              team1_name2,
                              team1_name3);
}

// IDA_EA: 0x868DCBC
// IDA_NAME: _ZN5CUser14alonePlay_ModeERK15MSG_REQ_TO_PEER
// DEMANGLED: CUser::alonePlay_Mode(MSG_REQ_TO_PEER const&)
bool CUser::alonePlay_Mode(const MSG_REQ_TO_PEER& request) {
    return request.request_type <= 3 &&
           GetGameOptionRef()->get_etc_option(27) == 1;
}

// IDA_EA: 0x868DD44
// IDA_NAME: _ZN5CUser17get_SoloPlay_modeEv
// DEMANGLED: CUser::get_SoloPlay_mode(void)
int CUser::get_SoloPlay_mode() {
    return static_cast<int>(static_cast<int16_t>(
        GetGameOptionRef()->get_etc_option(27)));
}

// IDA_EA: 0x868DD76
// IDA_NAME: _ZN5CUser20get_SoloPlay_msgTypeERK15MSG_REQ_TO_PEER
// DEMANGLED: CUser::get_SoloPlay_msgType(MSG_REQ_TO_PEER const&)
int CUser::get_SoloPlay_msgType(const MSG_REQ_TO_PEER& request) {
    switch (request.request_type) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
        return 2;
    case 3:
        return 3;
    default:
        return 4;
    }
}

// IDA_EA: 0x868E0EE
// IDA_NAME: _ZN5CUser29Send_SoloPlay_Reject_NotiPackEPKc26ENUM_SOLO_PLAY_REJECT_MODE25ENUM_SOLO_PLAY_REJECT_MSG
// DEMANGLED: CUser::Send_SoloPlay_Reject_NotiPack(char const*,ENUM_SOLO_PLAY_REJECT_MODE,ENUM_SOLO_PLAY_REJECT_MSG)
void CUser::Send_SoloPlay_Reject_NotiPack(const char* character_name,
                                          int reject_mode,
                                          int message_type) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 381);
    if (character_name) {
        const int length = static_cast<int>(std::strlen(character_name));
        packet.put_int(length);
        packet.put_str(character_name, length);
    } else {
        packet.put_int(0);
    }
    packet.put_int(reject_mode);
    packet.put_int(message_type);
    packet.finalize(true);
    Send(packet);
}
