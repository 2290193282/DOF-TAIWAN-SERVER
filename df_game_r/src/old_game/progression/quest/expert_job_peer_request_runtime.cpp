#include "old_game/progression/quest/quest_runtime.h"

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace expert_job {

// IDA_EA: 0x849E55E
// IDA_NAME: _ZN10expert_job13CExpertJobMgr22IsAvailableRequestPeerEP5CUserS2_
// DEMANGLED: expert_job::CExpertJobMgr::IsAvailableRequestPeer(CUser *,CUser *)
int CExpertJobMgr::IsAvailableRequestPeer(CUser* requester, CUser* target) {
    if (!requester->getCurCharacR() || !target->getCurCharacR()) return 3;
    if (requester->GetCurCharacExpertJob()) return 19;
    if (target->GetCurCharacExpertJob()) return 19;
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(target))
        return 61;
    if (target->IsCurCharacGhost()) return 19;
    return 0;
}

// IDA_EA: 0x849E256
// IDA_NAME: _ZN10expert_job13CExpertJobMgr13OnRequestPeerEP5CUserS2_22ENUM_PEER_REQUEST_TYPER11PacketGuard
// DEMANGLED: expert_job::CExpertJobMgr::OnRequestPeer(CUser *,CUser *,ENUM_PEER_REQUEST_TYPE,PacketGuard &)
int CExpertJobMgr::OnRequestPeer(CUser* requester,
                                 CUser* target,
                                 ENUM_PEER_REQUEST_TYPE request_type,
                                 PacketGuard& packet) {
    const int error = IsAvailableRequestPeer(requester, target);
    if (error) {
        return requester->SendCmdPeerErrorPacket(
            ENUM_CMDPACKET_REQUEST_PEER,
            request_type,
            static_cast<unsigned char>(error),
            packet);
    }

    packet.clear();
    packet.put_header(0, 7);
    packet.put_short(requester->get_unique_id());
    packet.put_byte(static_cast<int>(request_type));
    packet.put_int(target->get_unique_id());
    packet.finalize(true);
    return target->Send(packet);
}

}  // namespace expert_job
