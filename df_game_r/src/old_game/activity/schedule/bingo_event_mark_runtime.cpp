#include "old_game/activity/schedule/bingo_runtime.h"

#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x80C9FF6
// IDA_NAME: _ZN10BingoEvent12reqBingoMarkERN15StackableAction11input_paramE
// DEMANGLED: BingoEvent::reqBingoMark(StackableAction::input_param &)
int BingoEvent::reqBingoMark(StackableAction::input_param& param) {
    CUser* const user = param.user;
    CStackableItem* const item = param.stackable_item;
    if (!user || !item) {
        return 19;
    }

    BingoData* const data = user->getBingoData();
    if (data->line_count > 2) {
        return 7;
    }

    const std::vector<int>& action_values = item->action_values();
    if (action_values.empty()) {
        return 23;
    }
    const int center_value = action_values[0];
    if (center_value <= 0 || center_value > 50) {
        return 23;
    }

    const int random_value = bingo_->getBingoRandomVaule(center_value);
    if (random_value == 0) {
        return 19;
    }

    const unsigned int old_board = data->bingo_count;
    const int line_count =
        bingo_->calNewValue(data->bingo_count, random_value);
    if (data->bingo_count != old_board) {
        data->reward_state = 1;
        int added_lines = line_count - data->line_count;
        if (added_lines > 0) {
            if (added_lines > 3) {
                added_lines = 3;
            }
            data->line_count = static_cast<uint8_t>(line_count);
            for (int count = 0; count < added_lines; ++count) {
                sendOneMatchLineReward(*user);
            }
        }
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 451);
    packet.put_byte(random_value);
    packet.put_int(static_cast<int>(data->bingo_count));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}
