#include "old_game/pvp/season/pvp_rating_abi.h"

#include <cmath>

// IDA_EA: 0x844DCA1
// IDA_NAME: _ZN10CEloRating4calcERfS0_22ENUM_ELO_RATING_SYSTEMi
// DEMANGLED: CEloRating::calc(float &,float &,ENUM_ELO_RATING_SYSTEM,int)
float& CEloRating::calc(float& rating,
                        float& opposing_rating,
                        ENUM_ELO_RATING_SYSTEM rating_system,
                        int k_factor) {
    const float old_rating = rating;
    const float old_opposing_rating = opposing_rating;
    const float expected_score = static_cast<float>(
        1.0 / (std::pow(10.0, (opposing_rating - rating) / 400.0) + 1.0));
    const float opposing_expected_score = static_cast<float>(
        1.0 / (std::pow(10.0, (old_rating - old_opposing_rating) / 400.0) +
               1.0));

    float score = 0.0f;
    switch (static_cast<int>(rating_system)) {
    case 1:
        score = 0.5f;
        break;
    case 2:
        score = 1.0f;
        break;
    case 0:
    default:
        break;
    }

    const float opposing_score = 1.0f - score;
    rating = static_cast<long double>(k_factor) * (score - expected_score) +
             old_rating;
    opposing_rating = static_cast<long double>(k_factor) *
                          (opposing_score - opposing_expected_score) +
                      old_opposing_rating;
    return opposing_rating;
}
