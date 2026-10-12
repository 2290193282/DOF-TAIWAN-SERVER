#include "old_game/player/core/premium_runtime.h"

#include <time.h>

#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "common/config.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"


namespace WongWork {

// IDA_EA: 0x86AF8B4
// IDA_NAME: _ZN8WongWork14CHandlePremium23handleNotifyPremiumInfoEP5CUserii
// DEMANGLED: WongWork::CHandlePremium::handleNotifyPremiumInfo(CUser *,int,int)
void CHandlePremium::handleNotifyPremiumInfo(CUser* user,
                                             int premium_type,
                                             int value) {
    PacketGuard packet;
    packet.put_header(0, 66);
    packet.put_short(2);
    packet.put_byte(premium_type);
    packet.put_int(value);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x86AFA7C
// IDA_NAME: _ZN8WongWork14CHandlePremium20handlePremiumTimeoutEP5CUser
void CHandlePremium::handlePremiumTimeout(CUser* user) {
    if (!user) return;
    CUserPremium* const premium = user->GetPremiumInfoW();
    const SUserPremiumInfo expired = premium->CheckPremiumTimeout();
    const int premium_type = expired.premium_type;
    if (premium_type == 12) {
        premium->setPremiumState(static_cast<ENUM_PREMIUM_TYPE>(12),
                                 static_cast<ENUM_PREMIUM_STATE>(3));
        handleNotifyPremiumInfo(user, 12, 0);
        return;
    }
    if (premium_type <= 0 ||
        premium_type >= oldsrv::config::kPremiumDefinitionTypeCount) return;

    // The original timeout owner removes the expired record through the same
    // recalculate=true mutation entrypoint used by live premium updates.
    // That entrypoint now also reconciles contract-derived awakening levels.
    handleSetUserPremium(user,
                         static_cast<ENUM_PREMIUM_TYPE>(premium_type),
                         0, 0, 0, true);
    if (user->getCurCharacR()) {
        user->ProcPremiumFatigue();
        user->SendFatigue();
        const SPremiumInfo& definition = g_SPremiumInfo[premium_type];
        if (definition.advantage_bucket == 1) {
            if (definition.over_equipable_count > 0) {
                user->SetNeedCheckOverEquipItem();
            }
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_NOTIPACKET>(2), 1);
        }
        if (premium_type == 83 || premium_type == 87) {
            user->ReCalcChattingEmoticon();
            user->SendChattingEmoticon();
        } else if (premium_type == 33) {
            const int restrict_code =
                G_CDataManager()->get_item_list()->GetRestrictCode(197);
            user->RemoveRestrictedGoods(restrict_code);
            user->SetOneDayLetheFlag(
                CPremiumLetheManager::NOT_USE_LETHE_STATE);
            user->SetIsEndLetheAtLogin();
        }
    }
    handleNotifyPremiumInfo(user, premium_type, 0);
}

// IDA_EA: 0x86B0594
// IDA_NAME: _ZN8WongWork14CHandlePremium30handleRemoveOneDayLethePremiumEP5CUser
// DEMANGLED: WongWork::CHandlePremium::handleRemoveOneDayLethePremium(CUser *)
int CHandlePremium::handleRemoveOneDayLethePremium(CUser* user) {
    if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
        handleSetUserPremium(user, static_cast<ENUM_PREMIUM_TYPE>(33),
                             0, 0, 0, true);
        const int restrict_code =
            G_CDataManager()->get_item_list()->GetRestrictCode(197);
        user->RemoveRestrictedGoods(restrict_code);
        handleNotifyPremiumInfo(user, 33, 0);
    }

    user->SetOneDayLetheFlag(CPremiumLetheManager::NOT_USE_LETHE_STATE);
    cMyTrace trace(
        "static void WongWork::CHandlePremium::handleRemoveOneDayLethePremium(CUser*)",
        948, 0);
    return trace("ONE_DAY_LETHE : PREMIUM_END, char(%s)",
                 user->getCurCharacName());
}

// IDA_EA: 0x86AFC32
// IDA_NAME: _ZN8WongWork14CHandlePremium19handlePremiumEffectEP5CUser17ENUM_PREMIUM_TYPE
// DEMANGLED: WongWork::CHandlePremium::handlePremiumEffect(CUser *,ENUM_PREMIUM_TYPE)
char CHandlePremium::handlePremiumEffect(CUser* user,
                                         ENUM_PREMIUM_TYPE premium_type) {
    CUserPremium* const premium = user->GetPremiumInfoW();
    const int current_second = GlobalData::s_systemTime_.getCurSec();
    const char result = static_cast<char>(user->IsPremiumUser());
    if (!result) {
        return result;
    }

    // Delayed premium starts enter through this effect hook rather than
    // handleSetUserPremium's recalculate branch.  Refresh the over-skill
    // projection before reconciling fixed-level awakening actives below.
    premium->ReCalcAdvantage();

    const int requested_type = static_cast<int>(premium_type);
    std::vector<SUserPremiumInfo> premium_list =
        premium->GetPremiumInfoList(requested_type);
    for (std::vector<SUserPremiumInfo>::const_iterator entry =
             premium_list.begin();
         entry != premium_list.end(); ++entry) {
        if (entry->start_time <= current_second) {
            if (user->getCurCharacLastPlayTick() <= entry->start_time) {
                const std::vector<std::pair<int, int> > rewards =
                    premium->GetAdvantageItem(entry->premium_type);
                for (std::vector<std::pair<int, int> >::const_iterator reward =
                         rewards.begin();
                     reward != rewards.end(); ++reward) {
                    const int item_id = reward->first;
                    const int item_count = reward->second;
                    Inven_Item item;
                    CItem* const prototype = G_CDataManager()->find_item(item_id);
                    prototype->vtable_->make_item(prototype, item);
                    item.item_id_ = item_id;
                    if (item.item_category_ == 1) {
                        item.set_add_info(user->GetSeedFromDate());
                    } else {
                        item.set_add_info(item_count);
                    }

                    const int inserted_slot =
                        user->getCurCharacInvenW()->insertItemIntoInventory(
                            item,
                            static_cast<eItemAddReason>(14),
                            true,
                            true);
                    if (inserted_slot < 0) {
                        CMailBoxHelper::ReqDBSendNewMail(
                            user,
                            item,
                            0,
                            static_cast<unsigned int>(user->getCurCharacNo()),
                            true,
                            0,
                            0);
                    } else {
                        user->SendUpdateItemList(
                            static_cast<CUser::eSendTarget>(1),
                            static_cast<ENUM_ITEMSPACE>(0),
                            inserted_slot);
                    }
                }

                if (requested_type != 9999) {
                    const int coin_bonus =
                        g_SPremiumInfo[entry->premium_type].coin_bonus;
                    const int coin_before =
                        user->getCurCharacInvenR()->GetCoin();
                    user->getCurCharacInvenW()->SetCoin(
                        static_cast<unsigned int>(coin_before + coin_bonus));
                    user->history_log().CoinAdd(
                        user->getCurCharacInvenR()->GetCoin(),
                        coin_bonus,
                        static_cast<eCoinAddReason>(4));
                }
            }

            if (g_SPremiumInfo[entry->premium_type].uses_time_window != 0 &&
                entry->end_time > current_second) {
                G_TimerQueue()->InsertTimer(
                    0,
                    user->GetUID(),
                    4,
                    entry->end_time - current_second,
                    static_cast<unsigned int>(entry->premium_type),
                    user->get_acc_id());
            }

            if (premium->IsNeedCheckEveryday()) {
                time_t timer = static_cast<time_t>(current_second);
                tm next_day;
                localtime_r(&timer, &next_day);
                ++next_day.tm_mday;
                next_day.tm_sec = 0;
                next_day.tm_min = 0;
                next_day.tm_hour = 0;
                const time_t next_day_second = mktime(&next_day);
                G_TimerQueue()->InsertTimer(
                    0,
                    user->GetUID(),
                    5,
                    static_cast<int>(next_day_second - timer),
                    static_cast<unsigned int>(entry->premium_type),
                    user->get_acc_id());
            }
        } else {
            G_TimerQueue()->InsertTimer(
                0,
                user->GetUID(),
                6,
                entry->start_time - current_second,
                static_cast<unsigned int>(entry->premium_type),
                user->get_acc_id());
        }
    }

    if (user->IsHavePremiumAdvantage() && requested_type != 9999) {
        premium->RecalcAdditionalInfo(user);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
    }
    user->ProcPremiumFatigue();
    user->SendFatigue();
    user->SendCoin();
    if (requested_type == 9999) {
        user->setCurCharacLastPlayTick(current_second);
    }
    (void)ReconcileAutomaticGiveSkillLevels(user, user->get_state() > 2);
    return result;
}

// IDA_EA: 0x86B02FE
// IDA_NAME: _ZN8WongWork14CHandlePremium19handleSetUserPCRoomEP5CUserll
// DEMANGLED: WongWork::CHandlePremium::handleSetUserPCRoom(CUser *,long,long)
void CHandlePremium::handleSetUserPCRoom(CUser* user,
                                         long start_time,
                                         long end_time) {
    const bool was_affected = user->isAffectedPremium(
        static_cast<ENUM_PREMIUM_TYPE>(12));
    handleSetUserPremium(user,
                         static_cast<ENUM_PREMIUM_TYPE>(12),
                         start_time,
                         end_time,
                         0,
                         true);

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    const long remaining_second = end_time - current_second;
    if (remaining_second <= 604799 && remaining_second > 600) {
        G_TimerQueue()->InsertTimer(0,
                                    user->GetUID(),
                                    7,
                                    static_cast<int>(remaining_second - 600),
                                    12,
                                    user->get_acc_id());
    }

    if (user->get_state() > 2) {
        handleNotifyPremiumInfo(user,
                                12,
                                static_cast<int>(remaining_second));
        if (was_affected) {
            if (remaining_second <= 604799 && remaining_second > 600) {
                G_TimerQueue()->InsertTimer(
                    0,
                    user->GetUID(),
                    4,
                    static_cast<int>(remaining_second),
                    12,
                    user->get_acc_id());
            }
        } else {
            handlePremiumEffect(user, static_cast<ENUM_PREMIUM_TYPE>(12));
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2),
                                 0);
        }
    }

    PacketGuard packet;
    packet.put_header(0, 426);
    packet.put_byte(user->HasCurCharacRentalItem(1) ? 1 : 0);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace WongWork
