#include "old_game/pvp/battlefield/dispatcher_fair_pvp_score.h"

#include <stdint.h>

#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x8597672
// IDA_NAME: _ZN23Dispatcher_FairPvPScoreC2Ev
// DEMANGLED: Dispatcher_FairPvPScore::Dispatcher_FairPvPScore(void)
Dispatcher_FairPvPScore::Dispatcher_FairPvPScore() {}

// IDA_EA: 0x8240186
// IDA_NAME: _ZN23Dispatcher_FairPvPScoreD2Ev
// DEMANGLED: Dispatcher_FairPvPScore::~Dispatcher_FairPvPScore()
// IDA_EA: 0x82401B6
// IDA_NAME: _ZN23Dispatcher_FairPvPScoreD0Ev
// DEMANGLED: Dispatcher_FairPvPScore::~Dispatcher_FairPvPScore()
Dispatcher_FairPvPScore::~Dispatcher_FairPvPScore() {}

// IDA_EA: 0x822793C
// IDA_NAME: _ZN23Dispatcher_FairPvPScore12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_FairPvPScore::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_FairPvPScore::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }

    uint16_t target_unique_id = 0;
    if (packet.get_short(target_unique_id) != 1) {
        return LineFunc(
            58425,
            "virtual int Dispatcher_FairPvPScore::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint8_t detail_mode = 0;
    if (packet.get_byte(detail_mode) != 1) {
        return LineFunc(
            58428,
            "virtual int Dispatcher_FairPvPScore::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const uint16_t own_unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    if (own_unique_id == target_unique_id) {
        PacketGuard out_packet;
        out_packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                              ENUM_CMDPACKET_FAIR_PVP_SCORE);
        out_packet.put_byte(1);
        out_packet.put_short(target_unique_id);
        fair_pvp::CFairPvPScore* score =
            static_cast<fair_pvp::CFairPvPScore*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
        score->MakePacketSocre(out_packet, detail_mode);
        out_packet.finalize(true);
        user->Send(out_packet);
        return 0;
    }

    CUser* const target = G_GameWorld()->find_from_world(target_unique_id);
    if (!target) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_FAIR_PVP_SCORE, 3);
        return 0;
    }

    PacketGuard out_packet;
    if (detail_mode == 1) {
        const int slot = user->getCurCharacInvenR()->check_item_exist(3312);
        if (slot == -1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_FAIR_PVP_SCORE, 17);
            return 0;
        }

        const Inven_Item item =
            user->getCurCharacInvenR()->GetInvenSlot(1, slot);
        if (item.get_add_info() <= 0 ||
            user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                1,
                static_cast<eItemDelReason>(3),
                true) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_FAIR_PVP_SCORE, 17);
            return 0;
        }

        out_packet.clear();
        out_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 14);
        out_packet.put_byte(0);
        out_packet.put_short(1);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), slot, out_packet);
        out_packet.finalize(true);
        user->Send(out_packet);
    }

    out_packet.clear();
    out_packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                          ENUM_CMDPACKET_FAIR_PVP_SCORE);
    out_packet.put_byte(1);
    out_packet.put_short(target_unique_id);
    fair_pvp::CFairPvPScore* const score =
        static_cast<fair_pvp::CFairPvPScore*>(
            target->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
    score->MakePacketSocre(out_packet, detail_mode);
    out_packet.finalize(true);
    user->Send(out_packet);
    return 0;
}

// IDA_EA: 0x8234316
// IDA_NAME: _ZN23Dispatcher_FairPvPScore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_FairPvPScore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_FairPvPScore::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234320
// IDA_NAME: _ZN23Dispatcher_FairPvPScore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_FairPvPScore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_FairPvPScore::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
