#include "old_game/gm/core/game_master_set_multibox_fail_count.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <cstdlib>

namespace game_master {

CSetMultiboxFailCnt::CSetMultiboxFailCnt() : CCommand() {}

void CSetMultiboxFailCnt::parse() {
    // 0x84B34A4: reset first, trim parser +0x0c for its side effect, then
    // read token[1] without adding a substitute bounds check.  CMD179 has
    // already padded its vector to ten entries at 0x820BD1F..0x820BDAA.
    failure_count_ = 0;
    std::string& command_tail = parser_->command_tail();
    const std::string::size_type begin = command_tail.find_first_not_of(" ");
    if (begin != std::string::npos) {
        command_tail.erase(0, begin);
    }
    const std::string::size_type end = command_tail.find_last_not_of(" ");
    if (end != std::string::npos) {
        command_tail.erase(end + 1, std::string::npos);
    }
    failure_count_ = std::atoi(parser_->token_vector()[1].c_str());
}

void CSetMultiboxFailCnt::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    user->set_multiboxLotteryItemFailCnt(failure_count_);
    if (failure_count_ > 2) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 454);
        packet.finalize(1);
        user->Send(packet);
    }
}

}  // namespace game_master
