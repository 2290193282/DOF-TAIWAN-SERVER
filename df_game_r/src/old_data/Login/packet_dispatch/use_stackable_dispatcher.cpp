#include "old_data/Login/packet_dispatch/use_stackable_dispatcher.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kUseStackDispatchSignature[] =
    "virtual int DisPatcher_UseStack::dispatch_sig(CUser*, PacketBuf&)";

void SendUseStackError(CUser* user,
                       int error,
                       uint8_t item_space,
                       int argument_1,
                       int argument_2) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_STACKABLE);
    response.put_byte(0);
    response.put_byte(error);
    response.put_byte(item_space);
    response.put_int(argument_1);
    response.put_int(argument_2);
    response.finalize(true);
    user->Send(response);
}

bool IsFatigueRecoveryItem(unsigned int item_id, int& fatigue_cost) {
    switch (item_id) {
    case 7456u:
        fatigue_cost = 40;
        return true;
    case 7518u:
        fatigue_cost = 10;
        return true;
    case 7644u:
    case 7645u:
        fatigue_cost = 6;
        return true;
    case 8079u:
    case 2600026u:
    case 2600413u:
    case 690000094u:
        fatigue_cost = 30;
        return true;
    case 8221u:
        fatigue_cost = 20;
        return true;
    default:
        return false;
    }
}

bool IsVillageUseException(const CItem* item, unsigned int item_id) {
    const int item_type = item->vtable_->get_item_type(item);
    if (item_type == 3 || item_type == 22 || item_type == 24 ||
        item_type == 35) {
        return true;
    }
    return item_id == 10u || item_id == 24u || item_id == 51u ||
           (item_id >= 0x1e6du && item_id <= 0x1e73u) ||
           item_id == 2660234u;
}

bool IsTowerAllowedItem(unsigned int item_id) {
    return item_id == 10u || item_id == 24u || item_id == 51u ||
           item_id == 2660234u;
}

}  // namespace

// IDA_EA: 0x85955F0
// IDA_NAME: _ZN19DisPatcher_UseStackC2Ev
// DEMANGLED: DisPatcher_UseStack::DisPatcher_UseStack(void)
DisPatcher_UseStack::DisPatcher_UseStack()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82331C6
// IDA_NAME: _ZN19DisPatcher_UseStackD2Ev
// DEMANGLED: DisPatcher_UseStack::~DisPatcher_UseStack()
// IDA_EA: 0x82331F6
// IDA_NAME: _ZN19DisPatcher_UseStackD0Ev
// DEMANGLED: DisPatcher_UseStack::~DisPatcher_UseStack()
DisPatcher_UseStack::~DisPatcher_UseStack() {}

// IDA_EA: 0x81F63C0
// IDA_NAME: _ZN19DisPatcher_UseStack12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_UseStack::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_UseStack::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint16_t slot = 0;
    uint8_t item_space = 0;
    int argument_1 = 0;
    int argument_2 = 0;

    if (packet.get_short(slot) != 1) {
        return LineFunc(33557, kUseStackDispatchSignature, 0, 0);
    }
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(33560, kUseStackDispatchSignature, 0, 0);
    }
    if (packet.get_int(argument_1) != 1) {
        return LineFunc(33565, kUseStackDispatchSignature, 0, 0);
    }
    if (packet.get_int(argument_2) != 1) {
        return LineFunc(33566, kUseStackDispatchSignature, 0, 0);
    }

    const int state = user->get_state();
    if (state != 5 && state != 7 && state != 8 && state != 3 &&
        state != 6 && state != 10 && state != 12) {
        SendUseStackError(user, 19, item_space, argument_1, argument_2);
        return 0;
    }

    int checksum = 0;
    if (packet.get_int(checksum) != 1) {
        return LineFunc(33592, kUseStackDispatchSignature, 0, 0);
    }

    // ZombieMode.etc/dungeon 3101 owns its temporary passive-object items
    // through CMD517.  Ordinary inventory stackables (CMD44) never enter
    // that owner path, so reject them before checksum reporting, cooldown,
    // inventory mutation, or item effects can run.
    PvP_Room* const zombie_room = user->GetPVPRoom();
    if (zombie_room &&
        (zombie_mode::IsActiveRoom(zombie_room) ||
         zombie_mode::IsFinishedRoom(zombie_room))) {
        SendUseStackError(user, 19, item_space, argument_1, argument_2);
        return 0;
    }

    if (user->getCurCharacInvenCheckSum() != checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(603), 1, 0, 0);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, static_cast<RestrictCategory::Enum>(12), 1)) {
        SendUseStackError(user, 209, item_space, argument_1, argument_2);
        return 0;
    }

    CItem* const protected_item =
        GlobalData::s_pSecuProtectionField->GetItemPtr(
            user, static_cast<char>(item_space), static_cast<short>(slot));
    if (!protected_item) {
        return LineFunc(33620, kUseStackDispatchSignature, 0, 0);
    }
    const int security_error =
        GlobalData::s_pSecuProtectionField->CheckItem(
            user, static_cast<SECURITY_PROTCTION>(42), protected_item);
    if (security_error != 0) {
        SendUseStackError(user, security_error, item_space, argument_1,
                          argument_2);
        return 0;
    }
    if (protected_item->is_stackable() != 0 &&
        static_cast<CStackableItem*>(protected_item)->verifyUsablePlace(user) !=
            1) {
        SendUseStackError(user, 19, item_space, argument_1, argument_2);
        return 0;
    }

    const ENUM_ITEMSPACE inventory_space =
        static_cast<ENUM_ITEMSPACE>(item_space);
    const int inventory_type = GetInvenTypeFromItemSpace(inventory_space);
    const CInventory* const inventory = user->getCurCharacInvenR();

    if (!user->GetAssaultPlace() && (state == 3 || state == 6)) {
        const Inven_Item* const item = inventory->GetInvenRef(
            inventory_type, static_cast<int>(slot));
        if (!item) {
            return LineFunc(33719, kUseStackDispatchSignature, 0, 0);
        }
        const unsigned int item_id = static_cast<unsigned int>(item->item_id_);
        CItem* const prototype = G_CDataManager()->find_item(
            static_cast<int>(item_id));
        if (!prototype) {
            return 0;
        }
        if (!IsVillageUseException(prototype, item_id)) {
            SendUseStackError(user, 19, item_space, argument_1, argument_2);
            return 0;
        }

        int fatigue_cost = 0;
        if (IsFatigueRecoveryItem(item_id, fatigue_cost)) {
            if (state != 3) {
                SendUseStackError(user, 19, item_space, argument_1, argument_2);
                return 0;
            }
            if (user->getCurCharacFatigue() < fatigue_cost) {
                SendUseStackError(user, 95, item_space, argument_1, argument_2);
                return 0;
            }
        }
    }

    if (state == 10) {
        WongWork::CDeathTower* const death_tower = user->getDeathTower();
        if (!death_tower) {
            return LineFunc(33878, kUseStackDispatchSignature, 0, 0);
        }
        if (death_tower->handleUseStackable(user, inventory_space, slot) != 1) {
            return LineFunc(33880, kUseStackDispatchSignature, 0, 0);
        }
    }

    CParty* const party = user->GetParty();
    if (party) {
        WongWork::CDeathTower* const death_tower =
            state == 10 ? user->getDeathTower() : 0;
        if (death_tower) {
            const CDungeon* const dungeon =
                death_tower->getCDungeonMGr()->getDungeon();
            if (dungeon->limitOfStackableItemInTower() != 1 &&
                party->battle_field().instance_dungeon_mode() == 1) {
                SendUseStackError(user, 0, item_space, argument_1, argument_2);
                return 0;
            }
        } else {
            if (party->battle_field().instance_dungeon_mode() == 1) {
                SendUseStackError(user, 0, item_space, argument_1, argument_2);
                return 0;
            }

            const CDungeon* const dungeon = party->battle_field().GetDungeon();
            if (dungeon && dungeon->isTowerOfDespairDungeon() &&
                dungeon->limitOfStackableItemInTower() == 1) {
                const Inven_Item* const item = inventory->GetInvenRef(
                    inventory_type, static_cast<int>(slot));
                if (!item) {
                    return 0;
                }
                const unsigned int item_id =
                    static_cast<unsigned int>(item->item_id_);
                if (!IsTowerAllowedItem(item_id)) {
                    cMyTrace trace(kUseStackDispatchSignature, 33954, 5);
                    trace("TOD : charac_no(%u),dungeonIndex(%d),item_id(%lu)",
                          static_cast<unsigned int>(user->getCurCharacNo()),
                          party->battle_field().get_dungeon_index(), item_id);
                    return 0;
                }
            }

            if (dungeon && dungeon->isTournamentDungeon()) {
                const Inven_Item* const item = inventory->GetInvenRef(
                    inventory_type, static_cast<int>(slot));
                if (!item) {
                    return LineFunc(33969, kUseStackDispatchSignature, 0, 0);
                }
                CItem* const prototype = G_CDataManager()->find_item(
                    item->item_id_);
                if (!prototype || prototype->is_stackable() != 1) {
                    return 0;
                }
                const CStackableItem* const stackable =
                    static_cast<const CStackableItem*>(prototype);
                if (stackable->get_sub_type() == 2 ||
                    (stackable->has_summon_monster() &&
                     prototype->vtable_->get_item_type(prototype) != 18)) {
                    SendUseStackError(user, 255, item_space, argument_1,
                                      argument_2);
                    return 0;
                }
            }
        }
    }

    const Inven_Item* const item = inventory->GetInvenRef(
        inventory_type, static_cast<int>(slot));
    if (!item) {
        return 0;
    }
    if (item->item_id_ == 2600010) {
        const int now = GlobalData::s_systemTime_.getCurSec();
        const int chaos_kill_time = user->GetCurCharacChaosKillTime();
        int error = 0;
        if (now >= chaos_kill_time + 600) {
            if (party && party->CheckMemberLastTryAssaultTime() != 0) {
                error = -3;
            }
        } else {
            error = -6;
        }
        if (error != 0) {
            SendUseStackError(user, error, item_space, argument_1, argument_2);
            return 0;
        }
    }

    if (user->use_stackable(static_cast<short>(slot),
                            static_cast<int>(item_space), argument_1,
                            argument_2) == 1) {
        return 0;
    }
    return LineFunc(34056, kUseStackDispatchSignature, 0, 0);
}

// IDA_EA: 0x82331B2
// IDA_NAME: _ZN19DisPatcher_UseStack4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_UseStack::read(PacketBuf &,MSG_BASE &)
int DisPatcher_UseStack::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82331BC
// IDA_NAME: _ZN19DisPatcher_UseStack7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseStack::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_UseStack::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
