#include "old_game/world/region/game_world.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

namespace {

// CUserModifier only has its original vtable ABI recovered at this point.
// 0x86C9308 invokes slot zero with exactly (modifier, user); the decompiler's
// apparent third local is not passed by the assembly call sequence.
typedef void (*CUserModifierInvoke)(CUserModifier* modifier, CUser* user);

}  // namespace

// IDA_EA: 0x86C9308
// IDA_NAME: _ZN9GameWorld11modify_userEP13CUserModifier
// DEMANGLED: GameWorld::modify_user(CUserModifier *)
bool GameWorld::modify_user(CUserModifier* modifier) {
    CUserModifierInvoke* const vtable =
        *reinterpret_cast<CUserModifierInvoke**>(modifier);
    CUserModifierInvoke const invoke = vtable[0];

    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        invoke(modifier, it->second);
    }
    return false;
}

// IDA_EA: 0x86CAB92
// IDA_NAME: _ZN9GameWorld31check_peerTypemsg_sameWorldUserEP5CUserS1_cPci
// DEMANGLED: GameWorld::check_peerTypemsg_sameWorldUser(CUser *,CUser *,char,char *,int)
void GameWorld::check_peerTypemsg_sameWorldUser(CUser* from_user,
                                                CUser* to_user,
                                                char chat_type,
                                                char* message,
                                                int message_len) {
    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        from_user->GetServerGroup() != to_user->GetServerGroup()) {
        send_peerTypemsg_sameWorldUser(to_user, from_user, chat_type,
                                       21, 21, message, message_len);
        return;
    }

    if (to_user->isBlackUser(from_user->get_charac_no(-1))) {
        send_peerTypemsg_sameWorldUser(to_user, from_user, chat_type,
                                       77, 254, message, message_len);
        return;
    }

    if (from_user->isBlackUser(to_user->get_charac_no(-1))) {
        send_peerTypemsg_sameWorldUser(to_user, from_user, chat_type,
                                       77, 254, message, message_len);
        return;
    }

    if (GlobalData::s_pGMAccounts_->isGM(
            static_cast<uint32_t>(to_user->get_acc_id()))) {
        send_peerTypemsg_sameWorldUser(to_user, from_user, chat_type,
                                       90, 32, message, message_len);
        return;
    }

    send_peerTypemsg_sameWorldUser(from_user, to_user, chat_type,
                                   0, 1, message, message_len);
}

// IDA_EA: 0x86CAFAA
// IDA_NAME: _ZN9GameWorld13arrange_usersEv
// DEMANGLED: GameWorld::arrange_users(void)
void GameWorld::arrange_users() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user) {
            cMyTrace trace("void GameWorld::arrange_users()", 4262, 0);
            trace("GameWorld::arrange_users ACCID: %s",
                  NumberToString(static_cast<uint32_t>(user->get_acc_id()),
                                 0));
            user->DisConnSig(static_cast<DISCONN_SIG>(7), true, 0);
        }
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        int iteration = 0;
        CServerProxyMgr<CStatisticServerProxy>* const manager =
            GlobalData::s_statistic_proxy_mgr;
        for (int server_group = manager->GetStartIndex();
             iteration < manager->GetEndIndex();
             server_group = manager->GetNextIndex(iteration)) {
            DeleteChannel(G_CEnvironment()->get_gc_no_hardcode(server_group),
                          static_cast<ENUM_SERVER_GROUP>(server_group));
        }
    } else {
        DeleteChannel(G_CEnvironment()->get_gc_no(), SERVER_GROUP_DEFAULT);
    }

    G_CEnvironment()->set_suspend_state(true);
}
