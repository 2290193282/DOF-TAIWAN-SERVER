#include "old_game/player/core/cuser.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_data/Login/packet_dispatch/security_card_dispatchers.h"
#include "old_data/Login/security/security_card.h"
#include "old_data/db/account/db_force_save_simple_sso.h"
#include "old_data/db/taiwan/taiwan_restrict_dispatchers.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/match/integrated_matching_runtime.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"


// IDA_EA: 0x86487EA
// IDA_NAME: _ZN5CUser17prepareDisconnectEv
// DEMANGLED: CUser::prepareDisconnect(void)
void CUser::prepareDisconnect() {
    if (get_state() <= 1) {
        return;
    }

    integrated_matching::CancelForDisconnect(this);
    channel_zombie_matching_producer::ResetUser(this);

    if (isHumanCertified() != 1) {
        setHumanCertified(true);
        getHackAnalyzer()->addServerHackCnt(
            this, static_cast<WongWork::ENUM_HACKTYPE>(0x3B9u), 1u, 0u, 0u);
        ++human_certify_error_count_;
        DB_ForceSaveSimpleSSO::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            static_cast<int>(human_certify_error_count_));
    }

    if (getSecurityCard()->getRequestState() == 1 ||
        getPad()->getRequestState() == 1) {
        Dispatcher_SecurityCardAuthCancel::process(this);
    }

    const int process_sequence = G_CEnvironment()->getProcessSequence();
    AntibotCheckerDeleteClient(GlobalData::antibot_checker(),
                               process_sequence,
                               0,
                               static_cast<unsigned int>(get_acc_id()));

    DBUpdateExitTime::makeRequest(
        static_cast<uint32_t>(get_acc_id()),
        static_cast<int>(static_cast<unsigned char>(get_unique_id())),
        getGarenaAuthData()->getTimeStamp());
}

// IDA_EA: 0x86804CE
// IDA_NAME: _ZN5CUser12OnDisconnectEv
// DEMANGLED: CUser::OnDisconnect(void)
char CUser::OnDisconnect() {
    upgrade_room::OnUserDisconnect(this);
    golden_commerce::OnUserDisconnect(this);
    dual_raid::OnUserDisconnect(this);

    if (get_state() == 7) {
        PvP_Room* const room =
            G_CGameManager()->GetPvp(GetPvpIndex(), this, 0);
        if (room) {
            room->Disconnect(this);
        }
    }

    if (GetAssaultPlace() != 0) {
        return static_cast<char>(
            pvp_assault::GetInstanceAssaultMgr()->OnLeaveAssaultPlace(this,
                                                                       false));
    }
    return 0;
}
