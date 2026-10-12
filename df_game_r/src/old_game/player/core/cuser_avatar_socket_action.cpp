#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x86810A8
// IDA_NAME: _ZN5CUser26isEnableAvatarSocketActionEv
// DEMANGLED: CUser::isEnableAvatarSocketAction(void)
int CUser::isEnableAvatarSocketAction() {
    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        this, static_cast<SECURITY_PROTCTION>(17));
    if (security_result) {
        return security_result;
    }

    private_store::CPrivateStoreMgr* const private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (private_store_manager->IsBusyPrivateStore(this)) {
        return 19;
    }

    if (IsCurCharacGhost()) {
        return 19;
    }

    if (CheckInTrade()) {
        return 19;
    }

    if (GetAssaultPlace()) {
        return 19;
    }

    if (G_GameWorld()->IsPVPChannel()) {
        return 19;
    }

    return 0;
}

// IDA_EA: 0x8681160
// IDA_NAME: _ZN5CUser21isEnableGiveGiftToNPCEv
// DEMANGLED: CUser::isEnableGiveGiftToNPC(void)
int CUser::isEnableGiveGiftToNPC() {
    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        this, static_cast<SECURITY_PROTCTION>(21));
    if (security_result) {
        return security_result;
    }

    private_store::CPrivateStoreMgr* const private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (private_store_manager->IsBusyPrivateStore(this)) {
        return 19;
    }

    if (IsCurCharacGhost()) {
        return 19;
    }

    if (CheckInTrade()) {
        return 19;
    }

    if (GetAssaultPlace()) {
        return 19;
    }

    if (G_GameWorld()->IsPVPChannel()) {
        return 19;
    }

    return 0;
}
