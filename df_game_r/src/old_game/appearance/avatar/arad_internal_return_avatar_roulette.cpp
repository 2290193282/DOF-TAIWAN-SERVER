#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include "old_data/PVF/data/datamanager/avatar_roulette_server.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/appearance/avatar/arad_avatar_packet_dispatchers.h"
#include "old_game/economy/inventory/avatar_coin.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <ctime>

namespace {

// The original routine uses a 20-byte local buffer for this exact null-
// terminated identifier: M + four two-digit values + a ten-digit character
// number.  It is passed unchanged to CInventory::AddAvatarItem.
void MakeAvatarRouletteIpg(CUser* user, char (&ipg_no)[20]) {
    const std::time_t now = static_cast<std::time_t>(
        GlobalData::system_time()->getCurSec());
    const std::tm* const local = std::localtime(&now);
    std::snprintf(ipg_no,
                  sizeof(ipg_no),
                  "M%02d%02d%02d%02d%10d",
                  G_CEnvironment()->get_channel_no(),
                  (local->tm_year + 1900) % 2000,
                  local->tm_mon + 1,
                  local->tm_mday,
                  user->getCurCharacNo());
}

}  // namespace

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A3FC
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarRoulette::Arad_INTER_kAvatarRoulette(void)
Arad_INTER_kAvatarRoulette::Arad_INTER_kAvatarRoulette()
    : Arad_IInterDispatcher(static_cast<ENUM_INTERNALPACKET_JPN>(44)) {}

// IDA_EA: 0x819A420
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteD2Ev
// IDA_EA: 0x819A450
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarRoulette::~Arad_INTER_kAvatarRoulette()
Arad_INTER_kAvatarRoulette::~Arad_INTER_kAvatarRoulette() {}

// IDA_EA: 0x8198A9A
// IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRoulette12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kAvatarRoulette::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kAvatarRoulette::dispatch_sig(CUser* user,
                                               char* payload,
                                               int length) {
    (void)length;  // The old routine only tests the payload pointer.
    if (user == 0) {
        return 968;
    }
    if (payload == 0) {
        return 974;
    }
    if (user->getCurCharacR() == 0) {
        return 980;
    }

    const arad::SigAvatarRoulette& request =
        *reinterpret_cast<const arad::SigAvatarRoulette*>(payload);
    char ipg_no[20] = {};
    MakeAvatarRouletteIpg(user, ipg_no);

    AvatarRouletteServer::PickedAvatarInfo picked;
    if (GlobalData::s_pAvatarRoulette->Pickup(
            picked,
            static_cast<unsigned char>(request.roulette_type),
            static_cast<unsigned char>(request.select_no)) != 1) {
        // This path replies with USE_AVATAR_POTTERY rather than
        // USE_AVATAR_ROULETTE.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_POTTERY, 1);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    AvatarCoin* const avatar_coin = &inventory->avatar_coin_;
    if (!avatar_coin->IsUsable()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 22);
        return 0;
    }

    const char avatar_select = picked.picked_group > 1u ? 3 : -1;
    // The old branch checks value_0a == 3 before passing zero in either
    // branch.  Keep the data dependence explicit without inventing a period
    // semantic for value_0a.
    const int usable_period = picked.value_0a == 3u ? 0 : 0;
    const int slot = inventory->AddAvatarItem(
        static_cast<int>(picked.item_no),
        usable_period,
        false,
        static_cast<char>(picked.avatar_option),
        avatar_select,
        ipg_no,
        static_cast<eAvatarItemAddReason>(21),
        false,
        static_cast<uint16_t>(picked.value_09));
    if (slot == -1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 4);
        return 0;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             slot);
    if (!avatar_coin->Use()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_AVATAR_ROULETTE, 22);
        return 0;
    }

    AvatarCoin::HistoryLog::SubLog(user, 1);
    AvatarCoin::SendSyncPacket(user);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_AVATAR_ROULETTE);
    response.put_byte(1);
    response.put_short(slot);
    response.put_byte(picked.avatar_option);
    response.put_short(picked.value_09);
    response.finalize(true);
    user->Send(response);
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
