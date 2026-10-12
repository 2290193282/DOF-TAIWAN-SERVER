#include "old_game/progression/quest/quest_runtime.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace expert_job {

// IDA_EA: 0x849E362
// IDA_NAME: _ZN10expert_job13CExpertJobMgr14OnResponsePeerEP5CUserS2_S2_S2_i22ENUM_PEER_REQUEST_TYPER11PacketGuard
// DEMANGLED: expert_job::CExpertJobMgr::OnResponsePeer(CUser *,CUser *,CUser *,CUser *,int,ENUM_PEER_REQUEST_TYPE,PacketGuard &)
int CExpertJobMgr::OnResponsePeer(CUser* requester,
                                  CUser* target,
                                  CUser* mission_user,
                                  CUser* peer_user,
                                  int payload,
                                  ENUM_PEER_REQUEST_TYPE request_type,
                                  PacketGuard& packet) {
    const int error = IsAvailableRequestPeer(requester, target);
    if (error) {
        return requester->SendCmdPeerErrorPacket(
            ENUM_CMDPACKET_RESPONSE_PEER,
            request_type,
            static_cast<unsigned char>(error),
            packet);
    }

    CExpertJob* const expert_job = AcquireExpertJob(
        static_cast<ENUM_EXPERT_JOB_TYPE>(1));
    if (!expert_job) {
        return requester->SendCmdPeerErrorPacket(
            ENUM_CMDPACKET_RESPONSE_PEER, request_type, 22, packet);
    }

    // Expert-job type 1 is CEnchanter.  The recovered vtable call starts the
    // session before the peer-request discriminator is recorded.
    expert_job->OnStartMission(mission_user, peer_user);
    static_cast<CEnchanter*>(expert_job)->SetPeerRequestType(request_type);

    packet.put_header(0, ENUM_NOTIPACKET_RESPONSE_PEER);
    packet.put_short(requester->get_unique_id());
    packet.put_byte(static_cast<int>(request_type));
    packet.put_int(payload);
    packet.finalize(true);
    target->Send(packet);

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_RESPONSE_PEER);
    packet.put_byte(1);
    packet.put_short(target->get_unique_id());
    packet.put_byte(static_cast<int>(request_type));
    packet.finalize(true);
    return requester->Send(packet);
}

}  // namespace expert_job
