#include "old_game/player/core/cuser.h"

#include "old_data/db/auction_cashshop/db_get_premium_selected_cube_info.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/player/setting/character_option.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/world/region/village_attacked_runtime.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>

namespace {

enum {
    kPremiumSelectedCubeDbSignal = 818,
    kPremiumSelectedCubeType = 92,
    kAuraAvatarOptionPacket = 378,
    kCharacterOptionPacket = 390,
};

#pragma pack(push, 1)
struct RevengeDungeonStatusPacket {
    uint8_t reward_type;       // +0x00: 0 for reward kind 1, 1 for kind 2
    uint8_t is_open;           // +0x01
    uint32_t remaining_seconds; // +0x02 (unaligned in the original wire body)
    uint32_t reserved_06;      // +0x06
};
#pragma pack(pop)

static_assert(sizeof(RevengeDungeonStatusPacket) == 0x0au,
              "revenge-dungeon status notification must remain ten bytes");

}  // namespace

// IDA_EA: 0x86467EE
// IDA_NAME: _ZN5CUser34requestCubePremiumInfoIfContractedEv
// DEMANGLED: CUser::requestCubePremiumInfoIfContracted(void)
void CUser::requestCubePremiumInfoIfContracted() {
    if (!getCurCharacR()) {
        return;
    }
    if (!isAffectedPremium(
            static_cast<ENUM_PREMIUM_TYPE>(kPremiumSelectedCubeType))) {
        return;
    }

    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 1141);
    CStreamGuard guard(stream, true);
    guard << kPremiumSelectedCubeDbSignal;
    guard << GetUID();

    SIG_GET_PREMIUM_SELECTED_CUBE_INFO* const request =
        guard.GetInBuffer<SIG_GET_PREMIUM_SELECTED_CUBE_INFO>();
    request->charac_no = static_cast<uint32_t>(getCurCharacNo());
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x868DEA4
// IDA_NAME: _ZN5CUser23send_aura_avatar_optionEv
// DEMANGLED: CUser::send_aura_avatar_option(void)
void CUser::send_aura_avatar_option() {
    if (!getCurCharacR()) {
        return;
    }

    stAuraAvatarOption option;
    GetAuraAvatarOption(option);

    PacketGuard packet;
    packet.put_header(0, kAuraAvatarOptionPacket);
    packet.put_byte(3);
    for (int option_type = 0; option_type <= 2; ++option_type) {
        packet.put_byte(option_type);
        const int current_time = GlobalData::s_systemTime_.getCurSec();
        const int expiration_time = option.values[option_type];
        packet.put_int(expiration_time <= current_time
                           ? 0
                           : expiration_time - current_time);
    }
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x868F0E8
// IDA_NAME: _ZN5CUser16sendCharacOptionEv
// DEMANGLED: CUser::sendCharacOption(void)
void CUser::sendCharacOption() {
    CharacterOption option;
    if (GetCurCharacOption(option)) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, kCharacterOptionPacket);
    packet.put_int(sizeof(option.raw_00));
    packet.put_binary(reinterpret_cast<const char*>(option.raw_00),
                      sizeof(option.raw_00));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8680BD2
// IDA_NAME: _ZN5CUser22discard_mouse_registerEv
// DEMANGLED: CUser::discard_mouse_register(void)
void CUser::discard_mouse_register() {
    if (getPad()->getVersionInfo() != 1) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 171);
    packet.put_byte(1);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x868A99A
// IDA_NAME: _ZN5CUser18CheckMaxLuckyLevelEv
// DEMANGLED: CUser::CheckMaxLuckyLevel(void)
int CUser::CheckMaxLuckyLevel() {
    // The released body is an intentional leaf: it loads the constant 1 and
    // returns.  Keep the real ABI/provider rather than deleting the call site.
    return 1;
}

// IDA_EA: 0x8656ABC
// IDA_NAME: _ZN5CUser18SendRevengeDungeonEv
// DEMANGLED: CUser::SendRevengeDungeon(void)
void CUser::SendRevengeDungeon() {
    if (!getCurCharacR()) {
        return;
    }

    RevengeDungeonStatusPacket status = {};
    const int reward_type = GlobalData::s_villageMonsterMgr->GetRewardType();
    if (reward_type == 1) {
        status.reward_type = 0;
    } else if (reward_type == 2) {
        status.reward_type = 1;
    } else {
        return;
    }

    const uint32_t current_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    const uint32_t close_time = GlobalData::s_revengeDungeonMgr->GetCloseTime();
    status.is_open = 1;
    status.remaining_seconds =
        close_time < current_time ? 0 : close_time - current_time;

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_VILLAGE_ATTACKED_COMBINE);
    packet.put_binary(reinterpret_cast<const char*>(&status), sizeof(status));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x86568FC
// IDA_NAME: _ZN5CUser12SendOpenflagEv
// DEMANGLED: CUser::SendOpenflag(void)
void CUser::SendOpenflag() {
    if (!getCurCharacR()) {
        return;
    }

    unsigned int coin_no_per_day = 0;
    int missing_coin_count = 0;
    CCoinEventPerDay* const repeat_event =
        static_cast<CCoinEventPerDay*>(
            GlobalData::s_event_manager->GetRepeatEvent(4));
    if (repeat_event && repeat_event->IsEventing(0)) {
        coin_no_per_day = static_cast<unsigned int>(
            repeat_event->GetCoinNoPerDay(1));
    }
    if (IsHavePremiumAdvantage()) {
        coin_no_per_day += static_cast<unsigned int>(
            GetPremiumInfo()->GetAdvantageCoin());
    }

    CInventory* const inventory = getCurCharacInvenR();
    if (static_cast<unsigned int>(inventory->GetCoin()) >= coin_no_per_day ||
        static_cast<unsigned char>(GetOpencoin()) == 1u) {
        missing_coin_count = 0;
    } else {
        missing_coin_count = static_cast<int>(
            coin_no_per_day - static_cast<unsigned int>(inventory->GetCoin()));
    }

    PacketGuard packet;
    packet.put_header(0, 177);
    packet.put_byte(static_cast<char>(GetOpencoin()));
    packet.put_int(missing_coin_count);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8657F10
// IDA_NAME: _ZN5CUser11RecoverCoinEj
// DEMANGLED: CUser::RecoverCoin(unsigned int)
char CUser::RecoverCoin(unsigned int coin_no_per_day) {
    char result = getCurCharacR() == 0;
    if (result) {
        return result;
    }

    CCoinEventPerDay* const repeat_event =
        static_cast<CCoinEventPerDay*>(
            GlobalData::s_event_manager->GetRepeatEvent(4));
    if (repeat_event && repeat_event->IsEventing(0)) {
        coin_no_per_day = static_cast<unsigned int>(
            repeat_event->GetCoinNoPerDay(get_charac_level()));
        getCurCharacInvenR()->GetCoin();
    }
    if (IsHavePremiumAdvantage()) {
        coin_no_per_day += static_cast<unsigned int>(
            GetPremiumInfo()->GetAdvantageCoin());
        getCurCharacInvenR()->GetCoin();
    }

    if (static_cast<unsigned int>(getCurCharacInvenR()->GetCoin()) >=
        coin_no_per_day) {
        return static_cast<char>(SetOpencoin(1));
    }
    return static_cast<char>(SetOpencoin(0));
}
