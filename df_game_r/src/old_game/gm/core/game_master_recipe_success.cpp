#include "old_game/gm/core/game_master_recipe_success.h"

#include "old_game/player/core/cuser.h"

namespace game_master {

CRecipeSuccess::CRecipeSuccess() : CCommand() {}

void CRecipeSuccess::parse() {
    // 0x84B399A leaves probability_mode_ unchanged when no parser exists.
    if (!parser_) {
        return;
    }

    const std::string& mode = parser_->token_vector()[1];
    if (mode == "success") {
        probability_mode_ = 1;
    } else if (mode == "fail") {
        probability_mode_ = 2;
    } else {
        probability_mode_ = 0;
    }
}

void CRecipeSuccess::execute() {
    // The original has no extra success mapping or response packet: it hands
    // the exact three-state value to the already-restored CUser transaction.
    GetUser()->recipeForceProbability(probability_mode_);
}

}  // namespace game_master
