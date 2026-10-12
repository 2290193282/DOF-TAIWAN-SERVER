#include "old_game/social/party/packet_dispatcher_call_party_member_realtime_info.h"

#include "common/manager/global_tools.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x81CE34C
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallPartyMemberRealtimeInfo::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81CE356
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallPartyMemberRealtimeInfo::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& param) {
    (void)param;
    const int result = check_error(user, message);
    if (result > 0) {
        return result;
    }
    if (result >= 0) {
        return 0;
    }
    return -1;
}

// IDA_EA: 0x81CE398
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::check_error(CUser *,MSG_BASE &)
int Dispatcher_CallPartyMemberRealtimeInfo::check_error(CUser* user,
                                                         MSG_BASE&) {
    if (!user) {
        return LineFunc(
            10113,
            "int Dispatcher_CallPartyMemberRealtimeInfo::check_error(CUser*, MSG_BASE&)",
            0,
            0);
    }
    return user->get_state() == 3 ? 0 : -1;
}

// IDA_EA: 0x81CE3EE
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfo4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::send(CUser *,ParamBase &)
void Dispatcher_CallPartyMemberRealtimeInfo::send(CUser* user, ParamBase&) {
    PacketGuard response;
    PacketGuard unused_guard;
    CParty* const party = user->GetParty();

    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_CALL_PARTY_MEMBER_REALTIME_INFO);
    if (party) {
        party->send_party_realtime_info();
        return;
    }

    response.put_byte(0);
    response.put_byte(19);
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x8245D3C
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfoD2Ev
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::~Dispatcher_CallPartyMemberRealtimeInfo()
// IDA_EA: 0x8245D6C
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfoD0Ev
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::~Dispatcher_CallPartyMemberRealtimeInfo()
Dispatcher_CallPartyMemberRealtimeInfo::~Dispatcher_CallPartyMemberRealtimeInfo() {}

// IDA_EA: 0x8596508
// IDA_NAME: _ZN38Dispatcher_CallPartyMemberRealtimeInfoC2Ev
// DEMANGLED: Dispatcher_CallPartyMemberRealtimeInfo::Dispatcher_CallPartyMemberRealtimeInfo(void)
Dispatcher_CallPartyMemberRealtimeInfo::Dispatcher_CallPartyMemberRealtimeInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>() {}
