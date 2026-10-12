#include "common/manager/money_log_abi.h"

#include "infra/security/policy/secu_gold_control_abi.h"
#include "common/manager/statistics_runtime.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

#include <stdint.h>


namespace {

char* int_result(int value) {
    return reinterpret_cast<char*>(static_cast<intptr_t>(value));
}

char* add_circulation(unsigned int field, CUser* user, int money) {
    return int_result(
        GetInstanceCirculationStatistic()->AddCirculationStatistic(
            static_cast<CIRCULATION_STATISTIC_FIELD>(field),
            user,
            static_cast<unsigned int>(money)));
}

char* add_value(unsigned int field, CUser* user, int money) {
    return int_result(GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(field),
        user,
        static_cast<unsigned int>(money)));
}

}  // namespace

// IDA_EA: 0x84FEBD9
// IDA_NAME: _Z20write_log_gain_money15eMoneyAddReasoniP5CUser
// DEMANGLED: write_log_gain_money(eMoneyAddReason,int,CUser*)
void write_log_gain_money(int reason, int money, CUser* user) {
    if (money > 0) {
        if (user) {
            reinterpret_cast<Secu_GoldControl*>((reinterpret_cast<Secu_GoldControl*>(user->GetGoldControl())))->AddGold(
                static_cast<unsigned int>(money),
                static_cast<eMoneyAddReason>(reason));
        }

        switch (static_cast<unsigned int>(reason)) {
        case 0u:
            add_circulation(2u, user, money);
            user->AddMoneyPlus(money);
            break;
        case 3u:
            add_circulation(3u, user, money);
            user->AddMoneyPlus(money);
            break;
        case 4u:
            add_circulation(0u, user, money);
            user->AddMoneyPlus(money);
            break;
        case 10u:
            add_circulation(7u, user, money);
            break;
        case 15u:
            add_circulation(6u, user, money);
            break;
        case 26u:
            add_circulation(14u, user, money);
            break;
        default:
            break;
        }
    } else if (money < 0) {
        LogManager::logFormat(
            1,
            "inventory.cpp",
            "void write_log_gain_money(eMoneyAddReason, int, CUser*)",
            0xD71,
            "write_log_gain_money() reason(%d), gold(%d), gold(%d)",
            reason,
            money,
            user ? user->get_charac_level() : 0);
    }
}

// IDA_EA: 0x84FED9B
// IDA_NAME: _Z19write_log_use_money15eMoneySubReasoniP5CUser
// DEMANGLED: write_log_use_money(eMoneySubReason,int,CUser*)
char* write_log_use_money(int reason, int money, CUser* user) {
    char* result = int_result(user ? user->get_charac_level() : 0);
    if (money > 0) {
        if (user) {
            void* gold_control = user->GetGoldControl();
            reinterpret_cast<Secu_GoldControl*>((reinterpret_cast<Secu_GoldControl*>(gold_control)))->SubGold(
                static_cast<unsigned int>(money),
                static_cast<eMoneySubReason>(reason));
        }

        result = int_result(reason);
        switch (static_cast<unsigned int>(reason)) {
        case 0u:
            add_circulation(18u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 1u:
            add_circulation(20u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 4u:
            add_circulation(26u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 6u:
            result = add_circulation(24u, user, money);
            break;
        case 10u:
            add_circulation(19u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 11u:
            result = add_circulation(30u, user, money);
            break;
        case 12u:
            result = add_circulation(33u, user, money);
            break;
        case 13u:
            add_circulation(25u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 15u:
            result = add_circulation(35u, user, money);
            break;
        case 16u:
            result = add_circulation(27u, user, money);
            break;
        case 19u:
            result = add_circulation(31u, user, money);
            break;
        case 20u:
            add_circulation(22u, user, money);
            add_value(24u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 21u:
            result = add_circulation(36u, user, money);
            break;
        case 22u:
            result = add_circulation(32u, user, money);
            break;
        case 33u:
            result = add_circulation(39u, user, money);
            break;
        case 34u:
            result = add_circulation(37u, user, money);
            break;
        case 36u:
            result = add_circulation(40u, user, money);
            break;
        case 42u:
            result = add_circulation(41u, user, money);
            break;
        case 43u:
            add_circulation(43u, user, money);
            result = add_value(29u, user, money);
            break;
        case 44u:
            result = add_circulation(44u, user, money);
            break;
        case 45u:
            result = add_circulation(45u, user, money);
            break;
        case 46u:
            add_circulation(46u, user, money);
            result = reinterpret_cast<char*>(user->AddMoneyMinus(money));
            break;
        case 47u:
            result = add_circulation(47u, user, money);
            break;
        default:
            return result;
        }
    } else if (money < 0) {
        LogManager::logFormat(
            1,
            "inventory.cpp",
            "void write_log_use_money(eMoneySubReason, int, CUser*)",
            0xDE9,
            "write_log_use_money() reason(%d), gold(%d), gold(%d)",
            reason,
            money,
            result);
    }
    return result;
}
