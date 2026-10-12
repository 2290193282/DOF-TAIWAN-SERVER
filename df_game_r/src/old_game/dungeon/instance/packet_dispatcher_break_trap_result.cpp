#include "old_game/dungeon/instance/packet_dispatcher_break_trap_result.h"

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/log/format/log_manager.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_game/activity/schedule/sugar_time_event.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/player/core/cuser.h"

namespace {

const unsigned char kGenericInvalidTrapContext = 19;

bool IsExactEmptyCmd736(PacketBuf& packet) {
    // The 0828 sender has no business payload: its declared semantic frame is
    // exactly the CS header.  Do not probe the bound buffer's physical tail
    // here; inbound decryption deliberately leaves cipher-block padding after
    // that semantic frame.
    if (packet.get_index() != static_cast<int>(sizeof(CS_Header))) {
        return false;
    }

    const CS_Header* const header =
        reinterpret_cast<const CS_Header*>(packet.get_packet(0));
    return header && header->cmd == ENUM_PACKETCLASS_CMDPACKET &&
           header->type == ENUM_CMDPACKET_BREAK_TRAP_RESULT &&
           header->length == sizeof(CS_Header);
}

void SendInvalidTrapContext(CUser* user) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BREAK_TRAP_RESULT,
                                 kGenericInvalidTrapContext);
    }
}

}  // namespace

Dispatcher_BreakTrapResult::Dispatcher_BreakTrapResult()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(5)>() {}

Dispatcher_BreakTrapResult::~Dispatcher_BreakTrapResult() {}

int Dispatcher_BreakTrapResult::check_state(CUser* user, PacketBuf& packet) {
    // dispatch_template invokes check_state before read().  Defer malformed
    // frames to read() so wire errors cannot be disguised as an ordinary
    // business failure; only an already-proven exact frame gets a state
    // failure envelope here.
    if (!IsExactEmptyCmd736(packet)) return 0;
    if (!user || user->get_state() != 5 || user->is_fighting() == 0) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "int Dispatcher_BreakTrapResult::check_state(CUser*, PacketBuf&)",
            0,
            "[TIME_SPIRAL][CMD736_REJECT] reason=state uid=%d state=%d fighting=%d cursor=%d",
            user ? user->GetUID() : -1,
            user ? user->get_state() : -1,
            user ? user->is_fighting() : -1,
            packet.get_index());
        SendInvalidTrapContext(user);
        return 1;
    }
    return 0;
}

int Dispatcher_BreakTrapResult::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    (void)packet;
    // The 140828 client sends CMD736 after resolving the room-local trap but
    // does not send the old DestroyPassiveObject callback first.  Use the
    // already validated empty command plus the authoritative current
    // TimeSpiral map as an idempotent late arm.  The helper still rejects
    // every non-trap map, party, dungeon and owner context.
    CParty* const party = user ? user->GetParty() : 0;
    if (party) {
        (void)time_spiral::ArmAfterTrapDestroyed(
            party, time_spiral::GetTrapPassiveObjectId());
    }
    time_spiral::BreakTrapSelection selection;
    if (time_spiral::SelectOrReplay(user, &selection)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "int Dispatcher_BreakTrapResult::dispatch_sig(CUser*, PacketBuf&)",
            0,
            "[TIME_SPIRAL][CMD736_REPLY] uid=%d target=%u,%u result=%u",
            user ? user->GetUID() : -1,
            static_cast<unsigned int>(selection.target_x),
            static_cast<unsigned int>(selection.target_y),
            static_cast<unsigned int>(selection.result));
        taiwan_reward_event_notifications::SendBreakTrapSuccess(
            user, selection.target_x, selection.target_y, selection.result);
        return 0;
    }

    // The 0828 failure envelope is proven, but its TimeSpiral-specific code
    // is not.  Use the server's existing generic invalid-context code without
    // claiming it as a recovered mechanism-specific value.
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "int Dispatcher_BreakTrapResult::dispatch_sig(CUser*, PacketBuf&)",
        0,
        "[TIME_SPIRAL][CMD736_REJECT] reason=runtime_context uid=%d",
        user ? user->GetUID() : -1);
    SendInvalidTrapContext(user);
    return 0;
}

int Dispatcher_BreakTrapResult::read(PacketBuf& packet, MSG_BASE& message) {
    (void)message;
    const bool exact = IsExactEmptyCmd736(packet);
    if (!exact) {
        const CS_Header* const header =
            packet.get_index() >= static_cast<int>(sizeof(CS_Header))
                ? reinterpret_cast<const CS_Header*>(packet.get_packet(0))
                : 0;
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "int Dispatcher_BreakTrapResult::read(PacketBuf&, MSG_BASE&)",
            0,
            "[TIME_SPIRAL][CMD736_REJECT] reason=frame cursor=%d cmd=%d type=%d length=%d",
            packet.get_index(),
            header ? static_cast<int>(header->cmd) : -1,
            header ? static_cast<int>(header->type) : -1,
            header ? static_cast<int>(header->length) : -1);
    }
    return exact ? 0 : 1;
}

int Dispatcher_BreakTrapResult::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
