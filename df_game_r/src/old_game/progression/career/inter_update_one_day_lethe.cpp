#include "old_data/db/taiwan/inter_slots_331_353.h"

#include "infra/log/format/trace.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/packet_guard.h"


namespace {

bool IsSecondSkillTree(uint8_t skill_tree_kind) {
    return skill_tree_kind != 0xffu && skill_tree_kind != 0u;
}

ENUM_SKILL_TREE_KIND ReplySkillTree(uint8_t skill_tree_kind) {
    return IsSecondSkillTree(skill_tree_kind) ? ENUM_SKILL_TREE_1
                                               : ENUM_SKILL_TREE_0;
}

}  // namespace


// IDA_EA: 0x84D894C
// IDA_NAME: _ZN23Inter_UpdateOneDayLethe12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdateOneDayLethe::dispatch_sig(CUser *,char *,int)
int Inter_UpdateOneDayLethe::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;

    SIG_INTER_PREMIUM_LETHE_FLAG* const reply =
        reinterpret_cast<SIG_INTER_PREMIUM_LETHE_FLAG*>(payload);
    SIG_LOAD_PREMIUM_LETHE* const backup_skill =
        reinterpret_cast<SIG_LOAD_PREMIUM_LETHE*>(reply->skill_slot);
    const bool second_skill_tree = IsSecondSkillTree(reply->skill_tree_kind);
    const ENUM_SKILL_TREE_KIND skill_tree =
        ReplySkillTree(reply->skill_tree_kind);

    if (reply->state == CPremiumLetheManager::USE_LETHE_STATE) {
        if (reply->success) {
            cMyTrace trace(
                "virtual int Inter_UpdateOneDayLethe::dispatch_sig(CUser*, char*, int)",
                16271, 0);
            trace("ONE_DAY_LETHE : INTER_USE_LETHE, char(%s)",
                  user->getCurCharacName());

            if (second_skill_tree) {
                user->SetOneDayLetheFlag2ND(
                    CPremiumLetheManager::USE_LETHE_STATE);
            } else {
                user->SetOneDayLetheFlag(
                    CPremiumLetheManager::USE_LETHE_STATE);
            }
            user->SaveBackupSkillInfo(backup_skill, skill_tree);

            PacketGuard packet;
            if (user->GetCurCharacSkillTreeIndex() == -1) {
                if (!reply->skill_tree_kind) {
                    packet.put_header(0, 180);
                    packet.put_byte(2);
                    packet.put_byte(1);
                    packet.finalize(true);
                    user->Send(packet);
                }
            } else {
                const uint8_t command_tree = second_skill_tree ? 2 : 1;
                packet.put_header(0, 180);
                packet.put_byte(2);
                packet.put_byte(command_tree);
                packet.finalize(true);
                user->Send(packet);
            }
        }
    } else if (reply->state == CPremiumLetheManager::SKILL_CONFIRM_STATE) {
        if (reply->success) {
            cMyTrace trace(
                "virtual int Inter_UpdateOneDayLethe::dispatch_sig(CUser*, char*, int)",
                16317, 0);
            trace("ONE_DAY_LETHE : INTER_CONFIRM_SKILL_STATE, char(%s)",
                  user->getCurCharacName());
            WongWork::CHandlePremium::handleRemoveOneDayLethePremium(user);

            if (second_skill_tree) {
                user->SetOneDayLetheFlag2ND(
                    CPremiumLetheManager::NOT_USE_LETHE_STATE);
            } else {
                user->SetOneDayLetheFlag(
                    CPremiumLetheManager::NOT_USE_LETHE_STATE);
            }
            reply->lethe_flag = CPremiumLetheManager::SKILL_CONFIRM_STATE;
            user->SaveBackupSkillInfo(backup_skill, skill_tree);
        }
    } else if (reply->state == CPremiumLetheManager::INIT_LETHE_SKILL) {
        if (second_skill_tree) {
            user->SetOneDayLetheFlag2ND(CPremiumLetheManager::USE_LETHE_STATE);
        } else {
            user->SetOneDayLetheFlag(CPremiumLetheManager::USE_LETHE_STATE);
        }
        reply->lethe_flag = CPremiumLetheManager::USE_LETHE_STATE;
        user->SaveBackupSkillInfo(backup_skill, skill_tree);
    } else if (reply->state == CPremiumLetheManager::NOT_USE_LETHE_STATE) {
        if (second_skill_tree) {
            user->SetOneDayLetheFlag2ND(
                CPremiumLetheManager::NOT_USE_LETHE_STATE);
        } else {
            user->SetOneDayLetheFlag(
                CPremiumLetheManager::NOT_USE_LETHE_STATE);
        }
        reply->lethe_flag = CPremiumLetheManager::NOT_USE_LETHE_STATE;
        user->SaveBackupSkillInfo(backup_skill, skill_tree);
    }
    return 0;
}


// IDA_EA: 0x84EDFF8
// IDA_NAME: _ZN23Inter_UpdateOneDayLetheC2Ev
// DEMANGLED: Inter_UpdateOneDayLethe::Inter_UpdateOneDayLethe(void)
Inter_UpdateOneDayLethe::Inter_UpdateOneDayLethe() {}

// IDA_EA: 0x84F82D4 (D2), 0x84F8304 (D0)
Inter_UpdateOneDayLethe::~Inter_UpdateOneDayLethe() {}
