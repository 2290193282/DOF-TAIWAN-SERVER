#include "old_game/appearance/avatar/packet_dispatcher_disjoint_avatar.h"

#include <cstring>

#include "common/config.h"
#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/server_utility.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "old_data/db/avatar_aura/db_update_avatar_history.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_DisJointAvatar::dispatch_sig(CUser*, PacketBuf&)";
const char kEmptyIpgAgencyNo[] = "";

typedef int (*AvatarFunctionSlot)(const CItem*, int);

void ThrowAvatarError(int error) {
    // The direct old body raises the named fixed-width game error.  The
    // enclosing packet-dispatch boundary owns its error conversion.
    throw static_cast<ENUM_ERROR>(error);
}

uint16_t ReadEmblemType(const stAvatarEmblemInfo_t& emblem, int index) {
    return emblem.sockets[index].socket_type;
}

int GetItemType(const CItem* item) {
    return item->vtable_->get_item_type(item);
}

bool IsAvatarDisjointEnabled(const CItem* item) {
    const AvatarFunctionSlot enable =
        reinterpret_cast<AvatarFunctionSlot>(item->vtable_->slots_6_through_9[3]);
    return enable && enable(item, 2) == 1;
}

}  // namespace

// IDA_EA: 0x8217C06
// IDA_NAME: _ZN25Dispatcher_DisJointAvatar12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DisJointAvatar::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DisJointAvatar::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    const int socket_error = user->isEnableAvatarSocketAction();
    if (socket_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISJOINT_AVATAR,
                                 static_cast<unsigned char>(socket_error));
        return 0;
    }
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 20)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISJOINT_AVATAR, 209);
        return 0;
    }

    uint16_t avatar_slot = 0;
    unsigned long avatar_item_id = 0;
    if (packet.get_short(avatar_slot) != 1) {
        return LineFunc(51879, kDispatchSignature, 0, 0);
    }
    if (packet.get_item_idx(avatar_item_id) != 1) {
        return LineFunc(51880, kDispatchSignature, 0, 0);
    }
    if (user->CheckItemLock(2, static_cast<int>(avatar_slot))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DISJOINT_AVATAR, 213);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const Inven_Item avatar = inventory->GetInvenSlot(2, avatar_slot);
    if (static_cast<unsigned long>(static_cast<uint32_t>(avatar.item_id_)) !=
        avatar_item_id) {
        ThrowAvatarError(17);
    }

    CItem* const avatar_item = G_CDataManager()->find_item(avatar.item_id_);
    if (!avatar_item) {
        ThrowAvatarError(17);
    }
    if (avatar_item->get_grade() == 0 || avatar_item->get_grade() > 3) {
        ThrowAvatarError(19);
    }
    if (!IsAvatarDisjointEnabled(avatar_item)) {
        ThrowAvatarError(23);
    }

    const int avatar_key = avatar.get_add_info();
    const WongWork::CAvatarItemMgr* const avatar_manager =
        inventory->GetAvatarItemMgrR();
    if (avatar_manager->GetExpireDate(avatar_key) != 0) {
        ThrowAvatarError(23);
    }
    if (avatar.upgrade_separate_.IsTradeRestriction()) {
        ThrowAvatarError(23);
    }

    const stAvatarEmblemInfo_t* const emblem =
        avatar_manager->getJewelSocketData(avatar_key);
    int jewel_type_index = _selectJewelTypeIndex(emblem);
    if (GetItemType(avatar_item) == 9) {
        jewel_type_index = -1;
    }

    std::vector<std::pair<int, int> > result_items;
    const int result =
        _getResultItems(user, avatar_item, jewel_type_index, result_items);
    if (result != 0) {
        ThrowAvatarError(result);
    }
    if (inventory->check_empty_count(static_cast<Inven_Item::ITEM_TYPE>(9),
                                     static_cast<int>(result_items.size())) !=
        1) {
        ThrowAvatarError(4);
    }

    if (inventory->delete_item(static_cast<INVEN_TYPE>(2),
                               static_cast<int>(avatar_slot),
                               1,
                               static_cast<eItemDelReason>(6),
                               true) != 1) {
        ThrowAvatarError(22);
    }

    char ipg_agency_no[0x18] = {};
    const char* const source_ipg_agency_no =
        avatar_manager->GetIPGAgencyNo(avatar_key);
    if (source_ipg_agency_no) {
        std::strncpy(ipg_agency_no,
                     source_ipg_agency_no,
                     sizeof(ipg_agency_no) - 1);
    }
    if (inventory->GetAvatarItemMgrW()->UnRegistItem(avatar_key) != 1) {
        ThrowAvatarError(22);
    }
    inventory->SendAvatarEvent(54,
                               avatar_key,
                               5,
                               static_cast<int>(avatar_slot) +
                                   oldsrv::config::kAvatarInventorySlotBase,
                               0);
    DB_UpdateAvatarHistory::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()),
        avatar_key,
        ipg_agency_no);

    std::vector<int> inserted_slots;
    const int inserted = inventory->insert_event_items(
        result_items,
        inserted_slots,
        static_cast<eMoneyAddReason>(0),
        static_cast<eItemAddReason>(10),
        false,
        kEmptyIpgAgencyNo,
        kEmptyIpgAgencyNo);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_DISJOINT_AVATAR);
    response.put_byte(1);
    response.put_short(avatar_slot);
    response.put_short(inserted);
    for (std::vector<int>::const_iterator value = inserted_slots.begin();
         value != inserted_slots.end();) {
        response.put_short(*value++);
        response.put_item_idx(static_cast<unsigned long>(*value++));
        response.put_int(*value++);
    }
    response.finalize(true);
    user->Send(response);

    SendDisjointAvatarInfo(avatar_item);
    SendCreateEmblemInfo(result_items);
    return 0;
}

// IDA_EA: 0x821857C
// IDA_NAME: _ZNK25Dispatcher_DisJointAvatar18_getJewelTypeIndexEt
// DEMANGLED: Dispatcher_DisJointAvatar::_getJewelTypeIndex(unsigned short)const
int Dispatcher_DisJointAvatar::_getJewelTypeIndex(uint16_t emblem_type) const {
    return getAvatarEmblemTypeIdx(emblem_type);
}

// IDA_EA: 0x8218598
// IDA_NAME: _ZNK25Dispatcher_DisJointAvatar21_selectJewelTypeIndexEPK20stAvatarEmblemInfo_t
// DEMANGLED: Dispatcher_DisJointAvatar::_selectJewelTypeIndex(stAvatarEmblemInfo_t const*)const
int Dispatcher_DisJointAvatar::_selectJewelTypeIndex(
    const stAvatarEmblemInfo_t* emblem) const {
    int type_count[5] = {0, 0, 0, 0, 0};
    int cumulative[5] = {0, 0, 0, 0, 0};
    int total = 0;
    int cumulative_total = 0;

    for (int slot = 0; slot <= 4; ++slot) {
        const uint16_t type = ReadEmblemType(*emblem, slot);
        if (type != 0 && type != 0xffefu && type != 0xffffu) {
            const int index = _getJewelTypeIndex(type);
            if (index != -1) {
                ++type_count[index];
                ++total;
            }
        }
    }
    if (!total) {
        return -1;
    }
    for (int index = 0; index <= 4; ++index) {
        const int weight = 10000 / total * type_count[index];
        cumulative[index] = weight + cumulative_total;
        cumulative_total += weight;
    }
    const int random = get_rand_int(cumulative_total);
    for (int index = 0; index <= 4; ++index) {
        if (cumulative[index] > random) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x821873C
// IDA_NAME: _ZNK25Dispatcher_DisJointAvatar15_getResultItemsEP5CUserPK5CItemiRSt6vectorISt4pairIiiESaIS7_EE
// DEMANGLED: Dispatcher_DisJointAvatar::_getResultItems(CUser *,CItem const*,int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)const
int Dispatcher_DisJointAvatar::_getResultItems(
    CUser* user,
    const CItem* avatar_item,
    int jewel_type_index,
    std::vector<std::pair<int, int> >& items) const {
    (void)user;
    const DisJoint* const disjoint = G_CDataManager()->get_disjoint();
    int result_item_id = 0;
    const int grade = avatar_item->get_grade();

    if (jewel_type_index == -1) {
        result_item_id = disjoint->avatar_parts_disjoint_info[grade]
                                                        [GetItemType(avatar_item)];
    } else {
        int character_job = oldsrv::config::kCharacterJobInvalid;
        for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
            if (avatar_item->check_job_type(static_cast<char>(job))) {
                character_job = job;
            }
        }
        if (character_job == oldsrv::config::kCharacterJobInvalid) {
            return 22;
        }
        result_item_id = disjoint->avatar_disjoint_info[character_job][grade]
                                                        [jewel_type_index];
    }
    if (!result_item_id) {
        return 22;
    }

    CItem* const result_item = G_CDataManager()->find_item(result_item_id);
    if (!result_item || GetItemType(result_item) != 16) {
        return 22;
    }

    std::vector<stStackableBoosterItemInfo_t> booster_items;
    static_cast<const CStackableItem*>(result_item)->getBoosterItem(
        booster_items);
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator item =
             booster_items.begin();
         item != booster_items.end();
         ++item) {
        items.push_back(std::make_pair(item->item_index,
                                       static_cast<int>(item->item_count)));
    }
    return 0;
}

// IDA_EA: 0x8218996
// IDA_NAME: _ZN25Dispatcher_DisJointAvatar22SendDisjointAvatarInfoEPK5CItem
// DEMANGLED: Dispatcher_DisJointAvatar::SendDisjointAvatarInfo(CItem const*)
int Dispatcher_DisJointAvatar::SendDisjointAvatarInfo(const CItem* avatar_item) {
    Packet_Avater_Disjoint_Statistic packet;
    packet.action_0a = 1;
    packet.first_grade_bucket_0e = avatar_item->get_rarity() == 2
                                        ? 2
                                        : avatar_item->get_grade() - 1;
    packet.first_item_type_12 = GetItemType(avatar_item);
    packet.first_succeeded_16 = 1;
    return GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x8218A36
// IDA_NAME: _ZN25Dispatcher_DisJointAvatar20SendCreateEmblemInfoERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: Dispatcher_DisJointAvatar::SendCreateEmblemInfo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
int Dispatcher_DisJointAvatar::SendCreateEmblemInfo(
    const std::vector<std::pair<int, int> >& items) {
    if (items.size() > 5u) {
        return 0;
    }

    Packet_Emblem_Create_Statistic packet;
    int emitted = 0;
    for (std::vector<std::pair<int, int> >::const_iterator item = items.begin();
         item != items.end();
         ++item) {
        CItem* const result_item = G_CDataManager()->find_item(item->first);
        if (!result_item) {
            return 0;
        }
        packet.emblem_grades_0e[emitted] = result_item->get_grade();
        packet.emblem_counts_22[emitted] = item->second;
        if (++emitted > 5) {
            break;
        }
    }
    if (emitted != static_cast<int>(items.size())) {
        return 0;
    }
    packet.emblem_count_0a = emitted;
    return GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
}

// IDA_EA: 0x8233D26
// IDA_NAME: _ZN25Dispatcher_DisJointAvatar4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DisJointAvatar::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DisJointAvatar::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D30
// IDA_NAME: _ZN25Dispatcher_DisJointAvatar7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DisJointAvatar::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DisJointAvatar::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8241860 / 0x8241890
// IDA_NAME: _ZN25Dispatcher_DisJointAvatarD2Ev / _ZN25Dispatcher_DisJointAvatarD0Ev
// DEMANGLED: Dispatcher_DisJointAvatar::~Dispatcher_DisJointAvatar()
Dispatcher_DisJointAvatar::~Dispatcher_DisJointAvatar() {}

// IDA_EA: 0x85968C0
// IDA_NAME: _ZN25Dispatcher_DisJointAvatarC2Ev
// DEMANGLED: Dispatcher_DisJointAvatar::Dispatcher_DisJointAvatar(void)
Dispatcher_DisJointAvatar::Dispatcher_DisJointAvatar()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
