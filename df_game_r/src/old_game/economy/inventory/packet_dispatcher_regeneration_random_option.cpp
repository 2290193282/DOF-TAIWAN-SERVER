#include "old_game/economy/inventory/packet_dispatcher_regeneration_random_option.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistc_proxy_abi.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispather_RegenerationRandomOption::read(PacketBuf&, MSG_BASE&)";
const char kCheckStateSignature[] =
    "virtual int Dispather_RegenerationRandomOption::check_state(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x82316D0
// IDA_NAME: _ZN16stRegenROIResultC2Ev
// DEMANGLED: stRegenROIResult::stRegenROIResult(void)
stRegenROIResult::stRegenROIResult()
    : ParamBase(), result_code(0), item_slot(0) {}

// IDA_EA: 0x8246E1A / 0x8246E4A
// IDA_NAME: _ZN16stRegenROIResultD2Ev / D0Ev
// DEMANGLED: stRegenROIResult::~stRegenROIResult()
stRegenROIResult::~stRegenROIResult() {}

// IDA_EA: 0x81D69D2
// IDA_NAME: _ZN34Dispather_RegenerationRandomOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispather_RegenerationRandomOption::read(PacketBuf &,MSG_BASE &)
int Dispather_RegenerationRandomOption::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    MSG_REGENERATION_RANDOM_OPTION& request =
        static_cast<MSG_REGENERATION_RANDOM_OPTION&>(message);
    if (packet.get_short(request.first_item_slot) != 1) {
        return LineFunc(16595, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.second_item_slot) != 1) {
        return LineFunc(16598, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.select_part) != 1) {
        return LineFunc(16602, kReadSignature, 0, 0);
    }
    if (packet.get_short(request.regeneration_mode) != 1) {
        return LineFunc(16605, kReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81D6AF4
// IDA_NAME: _ZN34Dispather_RegenerationRandomOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispather_RegenerationRandomOption::process(CUser *,MSG_BASE &,ParamBase &)
int Dispather_RegenerationRandomOption::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    const MSG_REGENERATION_RANDOM_OPTION& request =
        static_cast<const MSG_REGENERATION_RANDOM_OPTION&>(message);
    stRegenROIResult& result = static_cast<stRegenROIResult&>(param);
    result.result_code = 0;

    Inven_Item input_items[2];
    CItem* item_prototypes[2] = {0, 0};
    const int slots[2] = {request.first_item_slot, request.second_item_slot};
    uint8_t preserve_seal_or_state = 1;

    for (int index = 0; index <= 1; ++index) {
        input_items[index] = user->getCurCharacInvenR()->GetInvenSlot(1,
                                                                        slots[index]);
        if (input_items[index].isEmpty() ||
            !input_items[index].isEquipableItemType() ||
            input_items[index].chronicle_random_option_.random_option_.empty()) {
            result.result_code = 19;
            return 0;
        }

        item_prototypes[index] =
            G_CDataManager()->find_item(input_items[index].item_id_);
        if (item_prototypes[index] == 0 ||
            item_prototypes[index]->vtable_->is_random_option(
                item_prototypes[index]) != 1) {
            result.result_code = 19;
            return 0;
        }
        if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                user, RestrictCategory::AuthMobile, 16) != 0) {
            result.result_code = 209;
            return 0;
        }
        const int security_error = GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(27));
        if (security_error != 0) {
            result.result_code = security_error;
            return 0;
        }
        if (user->CheckItemLock(1, slots[index])) {
            result.result_code = 213;
            return 0;
        }
        if (input_items[index].seal_or_state_ != 1) {
            preserve_seal_or_state = 0;
        }
    }

    const uint8_t reseal_count = static_cast<uint8_t>(
        (static_cast<unsigned int>(input_items[0].GetReSealCount()) +
         static_cast<unsigned int>(input_items[1].GetReSealCount()) + 1u) /
        2u);
    int cost = 0;
    Inven_Item generated_item;
    const int regeneration_error =
        G_CDataManager()->get_regeneration_roi()->Regenerate(
                         item_prototypes[0]->getUsableLevel(),
                         item_prototypes[1]->getUsableLevel(),
                         request.select_part,
                         request.regeneration_mode,
                         cost,
                         generated_item,
                         user);
    if (regeneration_error != 0) {
        result.result_code = regeneration_error;
        return 0;
    }

    const char* const date = GlobalData::s_systemTime_.getCurDate();
    const int level_section = user->get_charac_10_level_section();
    statistc_proxy::add(user, "log_random_option", "regen_cnt", 1,
                         "%d,'%s'", level_section, date);
    if (cost > 0) {
        statistc_proxy::add(user, "log_random_option", "regen_gold", cost,
                             "%d,'%s'", level_section,
                             GlobalData::s_systemTime_.getCurDate());
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (inventory->use_money(cost, static_cast<eMoneySubReason>(47), true) !=
        1) {
        result.result_code = 10;
        return 0;
    }

    // The original publishes slot 0 before deleting either material item.
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);
    for (int index = 0; index <= 1; ++index) {
        if (inventory->delete_item(static_cast<INVEN_TYPE>(1), slots[index],
                                   1, static_cast<eItemDelReason>(46), true) !=
            1) {
            result.result_code = 19;
            return 0;
        }
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), slots[0]);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), slots[1]);

    generated_item.seal_or_state_ = preserve_seal_or_state;
    generated_item.SetReSealCount(reseal_count);
    const int inserted_slot = inventory->insertItemIntoInventory(
        generated_item, static_cast<eItemAddReason>(54), true, true);
    if (inserted_slot >= 0) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 inserted_slot);
        result.item_slot = static_cast<uint16_t>(inserted_slot);
    }
    // The legacy service writes this history record even when the final insert
    // did not obtain a slot; do not convert that branch into a rollback.
    user->history_log().RandomOption(
        generated_item, static_cast<randomOptionReason::T>(1));
    return 0;
}

// IDA_EA: 0x81D7278
// IDA_NAME: _ZN34Dispather_RegenerationRandomOption4sendEP5CUserR9ParamBase
// DEMANGLED: Dispather_RegenerationRandomOption::send(CUser *,ParamBase &)
void Dispather_RegenerationRandomOption::send(CUser* user, ParamBase& param) {
    const stRegenROIResult& result =
        static_cast<const stRegenROIResult&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REGENERATION_RANDOM_OPTION);
    if (result.result_code != 0) {
        packet.put_byte(0);
        packet.put_int(result.result_code);
    } else {
        packet.put_byte(1);
        packet.put_short(result.item_slot);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8232984
// IDA_NAME: _ZN34Dispather_RegenerationRandomOption11check_stateEP5CUserR9PacketBuf
// DEMANGLED: Dispather_RegenerationRandomOption::check_state(CUser *, PacketBuf &)
int Dispather_RegenerationRandomOption::check_state(CUser* user, PacketBuf&) {
    if (user->get_state() <= 2 || user->getCurCharacR() == 0) {
        return LineFunc(2208, kCheckStateSignature, 0, 0);
    }
    if (user->CheckInTrade()) {
        return LineFunc(2212, kCheckStateSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x8244D64 / 0x8244D94
// IDA_NAME: _ZN34Dispather_RegenerationRandomOptionD2Ev / D0Ev
// DEMANGLED: Dispather_RegenerationRandomOption::~Dispather_RegenerationRandomOption()
Dispather_RegenerationRandomOption::~Dispather_RegenerationRandomOption() {}

// IDA_EA: 0x85977C2
// IDA_NAME: _ZN34Dispather_RegenerationRandomOptionC2Ev
// DEMANGLED: Dispather_RegenerationRandomOption::Dispather_RegenerationRandomOption(void)
Dispather_RegenerationRandomOption::Dispather_RegenerationRandomOption()
    : IPacketDispatcher<MSG_REGENERATION_RANDOM_OPTION,
                        stRegenROIResult,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824FF82 / 0x82500D2 / 0x825D68A / 0x825D71C / 0x825C166 /
//         0x825C196 / 0x8598CA6
template class IPacketDispatcher<MSG_REGENERATION_RANDOM_OPTION,
                                 stRegenROIResult,
                                 static_cast<ch_state>(0)>;
