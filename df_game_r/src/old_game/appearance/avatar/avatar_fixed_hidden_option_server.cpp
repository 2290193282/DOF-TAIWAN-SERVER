#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/avatar_item_access.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <algorithm>
#include <cstdio>

// IDA_EA: 0x817EC67
// IDA_NAME: _ZL5Test1P29AvatarFixedHiddenOptionServerii
// DEMANGLED: Test1(AvatarFixedHiddenOptionServer *,int,int)
static int Test1(AvatarFixedHiddenOptionServer* server,
                 int input,
                 int expected) {
    const int skill_index = server->GetSkillIndex(
        static_cast<ENUM_RARITY>(3), input);
    if (expected == skill_index) {
        return std::printf("input==%d, output==%d/%d\n",
                           input,
                           skill_index,
                           expected);
    }
    return std::printf("input==%d, output!=%d/%d\n",
                       input,
                       skill_index,
                       expected);
}

namespace ARAD {

// IDA_EA: 0x817EF68
// IDA_NAME: _ZN4ARAD9SingletonI29AvatarFixedHiddenOptionServerEC2Ev
// DEMANGLED: ARAD::Singleton<AvatarFixedHiddenOptionServer>::Singleton(void)
template <>
Singleton<AvatarFixedHiddenOptionServer>::Singleton() {}

// IDA_EA: 0x817F0A4
// IDA_NAME: _ZN4ARAD9SingletonI29AvatarFixedHiddenOptionServerE3GetEv
// DEMANGLED: ARAD::Singleton<AvatarFixedHiddenOptionServer>::Get(void)
template <>
AvatarFixedHiddenOptionServer*
Singleton<AvatarFixedHiddenOptionServer>::Get() {
    static AvatarFixedHiddenOptionServer object;
    return &object;
}

}  // namespace ARAD

// IDA_EA: 0x817E02C
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServerC2Ev
// DEMANGLED: AvatarFixedHiddenOptionServer::AvatarFixedHiddenOptionServer(void)
AvatarFixedHiddenOptionServer::AvatarFixedHiddenOptionServer() {}

// IDA_EA: 0x817E0E6
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServerD2Ev
// DEMANGLED: AvatarFixedHiddenOptionServer::~AvatarFixedHiddenOptionServer()
AvatarFixedHiddenOptionServer::~AvatarFixedHiddenOptionServer() {}

// IDA_EA: 0x817E154
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer4LoadEv
// DEMANGLED: AvatarFixedHiddenOptionServer::Load(void)
bool AvatarFixedHiddenOptionServer::Load() {
    cMyTrace trace("virtual bool AvatarFixedHiddenOptionServer::Load()",
                   26,
                   9,
                   true,
                   false);
    trace("\t- Loading Avatar Hidden Option - ");
    return script_.loadScript() == 0 && Init() == 1;
}

// IDA_EA: 0x817E1E2
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer7DestroyEv
// DEMANGLED: AvatarFixedHiddenOptionServer::Destroy(void)
int AvatarFixedHiddenOptionServer::Destroy() {
    return 1;
}

// IDA_EA: 0x817E1EC
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer13GetSkillIndexE11ENUM_RARITY
// DEMANGLED: AvatarFixedHiddenOptionServer::GetSkillIndex(ENUM_RARITY)
int AvatarFixedHiddenOptionServer::GetSkillIndex(ENUM_RARITY rarity) {
    const int random_input = GetRandomInput(rarity);
    if (random_input == -1) {
        return -1;
    }

    return GetSkillIndex(
        rarity, static_cast<int>(get_rand_int(random_input)));
}

// IDA_EA: 0x817E23E
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer13GetSkillIndexE11ENUM_RARITYi
// DEMANGLED: AvatarFixedHiddenOptionServer::GetSkillIndex(ENUM_RARITY,int)
int AvatarFixedHiddenOptionServer::GetSkillIndex(ENUM_RARITY rarity,
                                                  int input) {
    const std::vector<int>& limits = skill_indices_[static_cast<int>(rarity)];
    if (limits.back() <= input || input < 0) {
        return -1;
    }

    return static_cast<int>(
               std::upper_bound(limits.begin(), limits.end(), input) -
               limits.begin()) +
           1;
}

// IDA_EA: 0x817E334
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer14GetRandomInputE11ENUM_RARITY
// DEMANGLED: AvatarFixedHiddenOptionServer::GetRandomInput(ENUM_RARITY)
int AvatarFixedHiddenOptionServer::GetRandomInput(ENUM_RARITY rarity) {
    const int rarity_index = static_cast<int>(rarity);
    if (rarity_index > 5) {
        return -1;
    }

    const std::vector<int>& limits = skill_indices_[rarity_index];
    if (!limits.empty()) {
        return limits.back();
    }

    return -1;
}

// IDA_EA: 0x817E392
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer17IsVaildSkillIndexE11ENUM_RARITYi
// DEMANGLED: AvatarFixedHiddenOptionServer::IsVaildSkillIndex(ENUM_RARITY,int)
bool AvatarFixedHiddenOptionServer::IsVaildSkillIndex(ENUM_RARITY rarity,
                                                       int skill_index) {
    const int rarity_index = static_cast<int>(rarity);
    if (rarity_index > 5) {
        return false;
    }

    const std::vector<STAvatarHiddenAbilityVariation>& abilities =
        script_.ability_lists[rarity_index];
    return !abilities.empty() &&
           skill_index < static_cast<int>(abilities.size());
}

// IDA_EA: 0x817E3E8
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer4InitEv
// DEMANGLED: AvatarFixedHiddenOptionServer::Init(void)
int AvatarFixedHiddenOptionServer::Init() {
    for (int rarity_index = 0; rarity_index <= 5; ++rarity_index) {
        const std::vector<STAvatarHiddenAbilityVariation>& abilities =
            script_.ability_lists[rarity_index];
        std::vector<int>& limits = skill_indices_[rarity_index];

        // The old body deliberately does not clear this vector before a
        // subsequent Load(); reserve(size - 1) and append are retained.
        limits.reserve(abilities.size() - 1u);

        int total_variation = 0;
        for (size_t ability_index = 1; ability_index < abilities.size();
             ++ability_index) {
            total_variation += abilities[ability_index].variation;
            limits.push_back(total_variation);
        }
    }

    return 1;
}

// IDA_EA: 0x817ECD8
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer8UnitTestEPS_
// DEMANGLED: AvatarFixedHiddenOptionServer::UnitTest(AvatarFixedHiddenOptionServer *)
int AvatarFixedHiddenOptionServer::UnitTest(
    AvatarFixedHiddenOptionServer* server) {
    if (!server) {
        // The original leaves EAX unchanged for a null diagnostic argument.
        // No in-tree caller supplies null, and a deterministic zero avoids
        // manufacturing an uninitialized C++ result in the rebuilt provider.
        return 0;
    }

    Test1(server, 0, 1);
    Test1(server, 149, 1);
    Test1(server, 150, 2);
    Test1(server, 9949, 62);
    Test1(server, 9950, 63);
    return Test1(server, 9999, 63);
}

// IDA_EA: 0x817E4C6
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer14CheckConditionEP5CUserPN4arad27SigAvatarHiddenOptionChangeE
// DEMANGLED: AvatarFixedHiddenOptionServer::CheckCondition(CUser *,arad::SigAvatarHiddenOptionChange *)
bool AvatarFixedHiddenOptionServer::CheckCondition(
    CUser* user, arad::SigAvatarHiddenOptionChange* signal) {
    CInventory* const inventory = user->getCurCharacInvenW();
    const int material_slot =
        static_cast<int>(static_cast<int16_t>(signal->material_slot));
    const Inven_Item material = inventory->GetInvenSlot(1, material_slot);

    if (material.isEmpty()) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp",
            "bool AvatarFixedHiddenOptionServer::CheckCondition(CUser*, arad::SigAvatarHiddenOptionChange*)",
            0x71,
            "AVATAR HIDDEN OPTION CHANGE : Material GetInvenSlot() failed. [slot:%d] [charac_no:%d]",
            material_slot,
            user->getCurCharacNo());
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                                 0x16u);
        return false;
    }

    const uint32_t material_item_id =
        avatar_inventory::ReadAvatarItemRecord(material).item_id.value;
    AvatarFixedHiddenOptionServer* const configured_server =
        ARAD::Singleton<AvatarFixedHiddenOptionServer>::Get();
    if (!configured_server->script_.isMaterialItemIndex(
            static_cast<int>(material_item_id))) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp",
            "bool AvatarFixedHiddenOptionServer::CheckCondition(CUser*, arad::SigAvatarHiddenOptionChange*)",
            0x78,
            "AVATAR HIDDEN OPTION CHANGE : isMaterial() failed [index:%d] [charac_no:%d]",
            static_cast<int>(material_item_id),
            user->getCurCharacNo());
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                                 0x11u);
        return false;
    }

    const int avatar_slot =
        static_cast<int>(static_cast<int16_t>(signal->avatar_slot));
    const Inven_Item avatar = inventory->GetInvenSlot(2, avatar_slot);
    if (avatar.isEmpty()) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp",
            "bool AvatarFixedHiddenOptionServer::CheckCondition(CUser*, arad::SigAvatarHiddenOptionChange*)",
            0x83,
            "AVATAR HIDDEN OPTION CHANGE : Avatar GetInvenSlot() failed. [slot:%d] [charac_no:%d]",
            avatar_slot,
            user->getCurCharacNo());
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                                 0x16u);
        return false;
    }

    const uint32_t item_id =
        avatar_inventory::ReadAvatarItemRecord(avatar).item_id.value;
    CItem* const item = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (item == 0) {
        return false;
    }

    const CItemVTable* const item_vtable =
        *reinterpret_cast<const CItemVTable* const*>(item);
    if (static_cast<unsigned char>(item_vtable->is_avatar_item(item)) != 1u) {
        return false;
    }

    if (static_cast<unsigned char>(item->IsHiddenOption()) == 1u) {
        return true;
    }

    LogManager::logFormat(
        1,
        "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp",
        "bool AvatarFixedHiddenOptionServer::CheckCondition(CUser*, arad::SigAvatarHiddenOptionChange*)",
        0x94,
        "AVATAR HIDDEN OPTION CHANGE : IsHiddenOption() failed. [index:%d] [charac_no:%d]",
        static_cast<int>(item_id),
        user->getCurCharacNo());
    user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                             0x11u);
    return false;
}

// IDA_EA: 0x817E8D6
// IDA_NAME: _ZN29AvatarFixedHiddenOptionServer18ChangeHiddenOptionEP5CUserPN4arad27SigAvatarHiddenOptionChangeE
// DEMANGLED: AvatarFixedHiddenOptionServer::ChangeHiddenOption(CUser *,arad::SigAvatarHiddenOptionChange *)
bool AvatarFixedHiddenOptionServer::ChangeHiddenOption(
    CUser* user, arad::SigAvatarHiddenOptionChange* signal) {
    const int material_slot =
        static_cast<int>(static_cast<int16_t>(signal->material_slot));
    CInventory* const inventory = user->getCurCharacInvenW();
    if (static_cast<unsigned char>(inventory->delete_item(
            static_cast<INVEN_TYPE>(1),
            material_slot,
            1,
            static_cast<eItemDelReason>(3),
            true)) != 1u) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp",
            "bool AvatarFixedHiddenOptionServer::ChangeHiddenOption(CUser*, arad::SigAvatarHiddenOptionChange*)",
            0xA1,
            "AVATAR HIDDEN OPTION CHANGE : delete_item() failed. [slot:%d] [charac_no:%d]",
            material_slot,
            user->getCurCharacNo());
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AVATAR_HIDDEN_OPTION_CHANGE,
                                 0x13u);
        return false;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             material_slot);
    cMyTrace delete_trace(
        "bool AvatarFixedHiddenOptionServer::ChangeHiddenOption(CUser*, arad::SigAvatarHiddenOptionChange*)",
        166,
        0);
    delete_trace(
        "AVATAR HIDDEN OPTION CHANGE : delete_item() success. [slot:%d] [charac_no:%d]",
        material_slot,
        user->getCurCharacNo());

    const int avatar_slot =
        static_cast<int>(static_cast<int16_t>(signal->avatar_slot));
    Inven_Item* const avatar = inventory->GetInvenRef(2, avatar_slot);
    if (avatar == 0) {
        return false;
    }

    const uint32_t item_id =
        avatar_inventory::ReadAvatarItemRecord(*avatar).item_id.value;
    CItem* const item = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (item == 0) {
        return false;
    }

    const CItemVTable* const item_vtable =
        *reinterpret_cast<const CItemVTable* const*>(item);
    if (static_cast<unsigned char>(item_vtable->is_avatar_item(item)) != 1u) {
        return false;
    }

    const uint16_t hidden_option = static_cast<uint16_t>(
        GetSkillIndex(static_cast<ENUM_RARITY>(item->get_grade())));
    avatar_inventory::SetAvatarHiddenOption(
        *avatar, avatar_inventory::HiddenOption{hidden_option});
    signal->hidden_option = hidden_option;

    arad::SigAradAvatarHiddenOptionSave save;
    save.account_id = static_cast<uint32_t>(user->get_acc_id());
    // getCurCharacNo @0x80CBC4E is the established named accessor for the
    // same first dword that the old body reads through getCurCharacR(); it
    // returns zero only when that pointer is null.
    save.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    save.item_id = item_id;
    save.slot = static_cast<int16_t>(static_cast<uint16_t>(
        static_cast<uint32_t>(signal->avatar_slot) + 10u));
    save.hidden_option = static_cast<int16_t>(hidden_option);

    cMyTrace success_trace(
        "bool AvatarFixedHiddenOptionServer::ChangeHiddenOption(CUser*, arad::SigAvatarHiddenOptionChange*)",
        194,
        0);
    success_trace(
        "AVATAR HIDDEN OPTION CHANGE : success [slot:%d] [item_id:%d] [hidden_option:%d] [charac_no:%d]",
        static_cast<int>(save.slot),
        static_cast<int>(item_id),
        static_cast<int>(hidden_option),
        user->getCurCharacNo());

    CStreamGuard stream_guard(
        GlobalData::s_stream_pool->Acquire(
            "localjapan/Arad_AvatarFixedHiddenOptionServer.cpp", 198),
        true);
    ARAD::DISPATCHER::make_internal_stream_jpn(
        stream_guard,
        ARAD::kInternalPacketAvatarHiddenOptionSave,
        -1);
    stream_guard->put_binary(&save, static_cast<int>(sizeof(save)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, stream_guard);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             avatar_slot);
    return true;
}
