#include "old_game/progression/skill/packet_dispatcher_refund_skill.h"

#include "old_data/PVF/data/data_manager.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const int kNormalRefundItemId = 2660294;
const int kPvpRefundItemId = 2660295;
const int kRefundDeleteReason = 3;

const char kRefundSkillReadSignature[] =
    "virtual int Dispatcher_RefundSkill::read(PacketBuf&, MSG_BASE&)";
const char kRefundSkillProcessSignature[] =
    "virtual int Dispatcher_RefundSkill::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// This TU is the sole owner of CMD427's concrete state-0 template/vtable.
// IDA_EA: 0x8244A0A / 0x8244A3A / 0x859598C.
template class IPacketDispatcher<MSG_REFUND_SKILL,
                                 stRefundSkill,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859598C
// IDA_NAME: _ZN22Dispatcher_RefundSkillC2Ev
// DEMANGLED: Dispatcher_RefundSkill::Dispatcher_RefundSkill(void)
Dispatcher_RefundSkill::Dispatcher_RefundSkill()
    : IPacketDispatcher<MSG_REFUND_SKILL,
                        stRefundSkill,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244A0A
// IDA_NAME: _ZN22Dispatcher_RefundSkillD2Ev
// DEMANGLED: Dispatcher_RefundSkill::~Dispatcher_RefundSkill()
// IDA_EA: 0x8244A3A
// IDA_NAME: _ZN22Dispatcher_RefundSkillD0Ev
// DEMANGLED: Dispatcher_RefundSkill::~Dispatcher_RefundSkill()
Dispatcher_RefundSkill::~Dispatcher_RefundSkill() {}

// IDA_EA: 0x81DA6FE
// IDA_NAME: _ZN22Dispatcher_RefundSkill4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RefundSkill::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RefundSkill::read(PacketBuf& packet, MSG_BASE& base) {
    MSG_REFUND_SKILL& message = static_cast<MSG_REFUND_SKILL&>(base);
    if (packet.get_int(message.inventory_item_id) != 1)
        return LineFunc(18661, kRefundSkillReadSignature, 0, 0);
    if (packet.get_byte(message.skill_tree_index) != 1)
        return LineFunc(18663, kRefundSkillReadSignature, 0, 0);
    if (packet.get_byte(message.skill_index) != 1)
        return LineFunc(18665, kRefundSkillReadSignature, 0, 0);
    if (packet.get_byte(message.refund_count) != 1)
        return LineFunc(18667, kRefundSkillReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81DAD1C
// IDA_NAME: _ZN22Dispatcher_RefundSkill11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RefundSkill::check_error(CUser *,MSG_BASE &)
int Dispatcher_RefundSkill::check_error(
    CUser* user, const MSG_REFUND_SKILL& message) const {
    if (!user || !user->getCurCharacR()) return -1;
    if (user->get_state() != 3) return 7;

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        if (message.inventory_item_id != kPvpRefundItemId) return 22;
    } else if (message.inventory_item_id != kNormalRefundItemId) {
        return 22;
    }

    const int current_tree = user->GetCurCharacSkillTreeIndex();
    if (current_tree != -1 && current_tree != 0) {
        if (message.skill_tree_index != 1) return 19;
    } else if (message.skill_tree_index == 1) {
        return 19;
    }
    return message.refund_count ? 0 : 19;
}

// IDA_EA: 0x81DA820
// IDA_NAME: _ZN22Dispatcher_RefundSkill7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RefundSkill::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RefundSkill::process(CUser* user,
                                    MSG_BASE& base,
                                    ParamBase& raw) {
    const MSG_REFUND_SKILL& message =
        static_cast<const MSG_REFUND_SKILL&>(base);
    stRefundSkill& result = static_cast<stRefundSkill&>(raw);

    result.error_code = check_error(user, message);
    if (result.error_code > 0) return 0;
    if (result.error_code < 0) {
        return LineFunc(18682, kRefundSkillProcessSignature,
                        result.error_code, 0);
    }

    Inven_Item refund_item;
    const int item_slot = user->getCurCharacInvenRefR()->GetInvenData(
        static_cast<int>(message.inventory_item_id), refund_item);
    if (refund_item.item_id_ != static_cast<int>(message.inventory_item_id) ||
        refund_item.add_info_ < message.refund_count || item_slot < 0) {
        result.error_code = 22;
        return 0;
    }

    // The IDA body first deletes from an isolated CInventory copy.  That
    // preflight prevents a partial skill mutation if material removal cannot
    // succeed, then it invokes CUser::delete_item for the actual mutation.
    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenRefR());
    if (preview.delete_item(static_cast<INVEN_TYPE>(1), item_slot,
                            message.refund_count,
                            static_cast<eItemDelReason>(kRefundDeleteReason),
                            false) != 1) {
        result.error_code = 22;
        return 0;
    }

    result.skill_slot = user->getCurCharacSkillW()->refund_skill(
        message.skill_index, user->get_charac_level(), user->get_charac_job(),
        user->getCurCharFirstGrowType(), user->getCurCharSecondGrowType(),
        message.refund_count, result.error_code);
    if (result.skill_slot < 0 || result.skill_slot > 204) return 0;

    // The original handler records an actual-delete failure but continues its
    // completed skill-refund response path; retain that observable ordering.
    if (user->delete_item(0, item_slot, message.inventory_item_id,
                          message.refund_count,
                          static_cast<eItemDelReason>(kRefundDeleteReason))) {
        cMyTrace trace(kRefundSkillProcessSignature, 18738, 5);
        trace("delete_item failed - refund skill %d/%d",
              static_cast<int>(message.inventory_item_id),
              static_cast<int>(message.refund_count));
    }

    result.special_skill = 0;
    CSkill* const skill = G_CDataManager()->find_skill(
        user->get_charac_job(), result.skill_slot);
    if (skill && skill->IsSpecialSkill()) result.special_skill = 1;

    result.skill_tree_index = static_cast<int8_t>(
        user->GetCurCharacSkillTreeIndex());
    result.skill_index = message.skill_index;
    result.skill_level = static_cast<uint8_t>(
        user->getCurCharacSkillR()->get_skill_level(result.skill_slot));
    if (result.special_skill) {
        const ENUM_SKILL_TREE_KIND point_tree =
            user->GetCurCharacSkillTreeIndex() == -1 ||
                    user->GetCurCharacSkillTreeIndex() == 0
                ? ENUM_SKILL_TREE_SFP_0
                : ENUM_SKILL_TREE_SFP_1;
        result.remain_point = static_cast<uint16_t>(
            user->getCurCharacSkillR()->get_remain_sfp_at_index(point_tree));
    } else {
        result.remain_point = static_cast<uint16_t>(
            user->getCurCharacSkillR()->get_remain_sp_at_index(
                static_cast<ENUM_SKILL_TREE_KIND>(
                    static_cast<int>(result.skill_tree_index))));
    }
    result.inventory_item_id = static_cast<int>(message.inventory_item_id);
    result.refund_count = message.refund_count;
    return 0;
}

// IDA_EA: 0x81DAE0A
// IDA_NAME: _ZN22Dispatcher_RefundSkill4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RefundSkill::send(CUser *,ParamBase &)
void Dispatcher_RefundSkill::send(CUser* user, ParamBase& raw) {
    const stRefundSkill& result = static_cast<const stRefundSkill&>(raw);
    if (result.error_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REFUND_SKILL,
                                 static_cast<unsigned char>(result.error_code));
        return;
    }

    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_REFUND_SKILL);
    response.put_byte(1);
    response.put_byte(result.special_skill);
    response.put_byte(result.skill_tree_index);
    response.put_short(result.remain_point);
    response.put_byte(result.skill_slot);
    response.put_byte(result.skill_index);
    response.put_byte(result.skill_level);
    response.put_int(result.inventory_item_id);
    response.put_byte(result.refund_count);
    response.finalize(true);
    user->Send(response);
}
