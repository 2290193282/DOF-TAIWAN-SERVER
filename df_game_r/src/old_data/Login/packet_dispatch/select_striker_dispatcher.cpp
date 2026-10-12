#include "old_data/Login/packet_dispatch/select_striker_dispatcher.h"

#include <climits>
#include <cstring>

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/striker_script.h"
#include "old_data/db/character/tag_charac_info_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const char kSelectStrikerReadSignature[] =
    "virtual int Dispatcher_SelectStriker::read(PacketBuf&, MSG_BASE&)";
const char kSelectStrikerProcessSignature[] =
    "virtual int Dispatcher_SelectStriker::process(CUser*, MSG_BASE&, ParamBase&)";
const unsigned int kTagCharacterSkillCount = 204u;

void MakeTagSkillList(CUser* user, PacketGuard& packet) {
    const _Charac_info* const tag_character = user->getTagCharacR();
    const ENUM_SKILL_TREE_KIND skill_tree =
        (tag_character->fields_.skill_tree_index_ == static_cast<int8_t>(-1) ||
         tag_character->fields_.skill_tree_index_ == 0)
            ? ENUM_SKILL_TREE_0
            : ENUM_SKILL_TREE_1;
    const SkillSlot* const skill_slot = user->getTagCharacSkillR();
    const _Mastered_skill* const skills =
        skill_slot->get_skillslot_buf(skill_tree);

    int count_index = packet.get_index();
    int count = 0;
    packet.put_byte(0);
    for (unsigned int index = 0; index < kTagCharacterSkillCount; ++index) {
        if (!skills[index].skill_index) {
            continue;
        }
        packet.put_byte(index);
        packet.put_byte(skills[index].skill_index);
        packet.put_byte(skills[index].skill_level);
        ++count;
    }
    packet.put_byte(count_index, count);
}

}  // namespace

// The old CMD488 template is its sole specialization/vtable owner.  The
// 606-slot PacketDispatcher root registration intentionally remains absent.
// IDA_EA: 0x824A442
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824A59E
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824A5A8
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825B44C
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B47C
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D5C4
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x85983AE
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_STRIKER15stSelectStrikerL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SELECT_STRIKER,stSelectStriker,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SELECT_STRIKER,
                                 stSelectStriker,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8595900
// IDA_NAME: _ZN24Dispatcher_SelectStrikerC2Ev
// DEMANGLED: Dispatcher_SelectStriker::Dispatcher_SelectStriker(void)
Dispatcher_SelectStriker::Dispatcher_SelectStriker()
    : IPacketDispatcher<MSG_SELECT_STRIKER,
                        stSelectStriker,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8243EC4
// IDA_NAME: _ZN24Dispatcher_SelectStrikerD2Ev
// DEMANGLED: Dispatcher_SelectStriker::~Dispatcher_SelectStriker()
// IDA_EA: 0x8243EF4
// IDA_NAME: _ZN24Dispatcher_SelectStrikerD0Ev
// DEMANGLED: Dispatcher_SelectStriker::~Dispatcher_SelectStriker()
Dispatcher_SelectStriker::~Dispatcher_SelectStriker() {}

// IDA_EA: 0x81E3D32
// IDA_NAME: _ZN24Dispatcher_SelectStriker4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SelectStriker::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SelectStriker::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SELECT_STRIKER& request = static_cast<MSG_SELECT_STRIKER&>(message);
    uint8_t charac_index = 0;
    if (packet.get_byte(charac_index) != 1) {
        return LineFunc(23425, kSelectStrikerReadSignature, 0, 0);
    }
    request.charac_index = static_cast<int8_t>(charac_index);
    if (packet.get_byte(request.striker_skill_index) == 1) {
        return 0;
    }
    return LineFunc(23428, kSelectStrikerReadSignature, 0, 0);
}

// IDA_EA: 0x81E4066
// IDA_NAME: _ZN24Dispatcher_SelectStriker11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_SelectStriker::check_error(CUser *,MSG_BASE &)
int Dispatcher_SelectStriker::check_error(CUser* user, MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return INT_MAX;
    }

    const MSG_SELECT_STRIKER& request =
        static_cast<const MSG_SELECT_STRIKER&>(message);
    const int current_index = user->get_charac_idx(user->get_charac_no(-1));
    if (current_index == -1) {
        return 57;
    }
    if (request.charac_index == current_index) {
        user->setCurCharacTagCharacNo(0);
        user->setCurCharacStrikerSkillIndex(0);
        user->character_view().enableSaveCharacView();
        user->setTagCharac(0);
        return INT_MAX;
    }
    if (request.charac_index < 0 ||
        static_cast<std::vector<_Charac_info>::size_type>(request.charac_index) >=
            user->characters_.size()) {
        return 57;
    }

    const _Charac_info& tag_candidate = user->characters_[
        static_cast<std::vector<_Charac_info>::size_type>(request.charac_index)];
    if (tag_candidate.fields_.level_ <
        G_CDataManager()->get_striker_skill_minimum_level()) {
        return 14;
    }
    SkillSlot* const candidate_skill_slot =
        const_cast<SkillSlot*>(&tag_candidate.fields_.skill_slot_);
    if (candidate_skill_slot->IsLoadStrikerSkill() == 1) {
        const int16_t job = static_cast<int8_t>(tag_candidate.fields_.job_);
        const int16_t grow_type = static_cast<int8_t>(
            static_cast<uint8_t>(tag_candidate.fields_.grow_type_.growth_type) << 4u) >> 4;
        if (G_CDataManager()->get_striker_script()->checkSkillIndex(
                job, grow_type, request.striker_skill_index) == 1) {
            return 0;
        }
        return 21;
    }
    return 19;
}

// IDA_EA: 0x81E3DCA
// IDA_NAME: _ZN24Dispatcher_SelectStriker7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SelectStriker::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SelectStriker::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    stSelectStriker& result = static_cast<stSelectStriker&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(23443, kSelectStrikerProcessSignature, result.result, 0);
    }

    const MSG_SELECT_STRIKER& request =
        static_cast<const MSG_SELECT_STRIKER&>(message);
    _Charac_info& tag_candidate = user->characters_[
        static_cast<std::vector<_Charac_info>::size_type>(request.charac_index)];
    if (tag_candidate.fields_.tag_charac_info_loaded_) {
        user->setCurCharacTagCharacNo(tag_candidate.fields_.character_no_);
        user->setCurCharacStrikerSkillIndex(request.striker_skill_index);
        user->character_view().enableSaveCharacView();
        user->setTagCharac(&tag_candidate);
        user->getTagCharacInvenR()->set_parent(user);
        user->getTagCharacSkillR()->set_parent(user);
        user->SendTagCharacInfo();
        return 0;
    }

    CStreamGuard guard(GlobalData::stream_pool()->Acquire(
                           "PacketDispatcher_Impl_1.cpp", 23462),
                       true);
    *guard << 766;
    *guard << user->GetUID();
    SIG_REQUEST_TAG_CHARAC_INFO* const payload =
        guard.GetInBuffer<SIG_REQUEST_TAG_CHARAC_INFO>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = tag_candidate.fields_.character_no_;
    payload->striker_skill_index = request.striker_skill_index;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81E42EE
// IDA_NAME: _ZN24Dispatcher_SelectStriker4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SelectStriker::send(CUser *,ParamBase &)
void Dispatcher_SelectStriker::send(CUser* user, ParamBase& param) {
    const int result = static_cast<const stSelectStriker&>(param).result;
    if (result == INT_MAX) {
        return;
    }
    if (result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SELECT_STRIKER,
                                 static_cast<unsigned char>(result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_SELECT_STRIKER);
}

// IDA_EA: 0x86903F8
// IDA_NAME: _ZN5CUser17SendTagCharacInfoEv
// DEMANGLED: CUser::SendTagCharacInfo(void)
void CUser::SendTagCharacInfo() {
    _Charac_info* const tag_character = getTagCharacW();
    if (!tag_character) {
        return;
    }
    if (tag_character->fields_.tag_charac_info_loaded_ != 1u) {
        tag_character->fields_.tag_charac_info_loaded_ = 1u;
    }
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 414);
    packet.put_short(1);
    MakeTagCharacInfo(packet);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8690776
// IDA_NAME: _ZN5CUser17MakeTagCharacInfoER11PacketGuard
// DEMANGLED: CUser::MakeTagCharacInfo(PacketGuard &)
int CUser::MakeTagCharacInfo(PacketGuard& packet) {
    if (!getCurCharacR() || !getTagCharacR()) {
        return 0;
    }

    const _Charac_info* const tag_character = getTagCharacR();
    packet.put_short(get_unique_id());
    const int name_length = static_cast<int>(
        std::strlen(tag_character->fields_.character_name_));
    packet.put_int(name_length);
    packet.put_str(tag_character->fields_.character_name_, name_length);
    packet.put_byte(tag_character->fields_.level_);
    packet.put_byte(static_cast<int8_t>(tag_character->fields_.job_));
    packet.put_byte(static_cast<int8_t>(tag_character->fields_.grow_type_.growth_type));
    packet.put_byte(getCurCharacStrikerSkillIndex());

    if (IsHavePremiumAdvantage()) {
        _Additioal_info additional_info = *getTagCharacAddInfoR();
        additional_info += GetPremiumInfo()->GetAdditionalInfo();
        packet.put_int(82);
        packet.put_binary(reinterpret_cast<const char*>(&additional_info), 82);
    } else {
        packet.put_int(82);
        packet.put_binary(reinterpret_cast<const char*>(getTagCharacAddInfoR()),
                          82);
    }

    getTagCharacInvenR()->MakeEquipList(
        &packet,
        true,
        static_cast<ENUM_USERINFO>(1),
        static_cast<ENUM_EQUIPSLOT>(0));
    MakeTagSkillList(this, packet);

    CQuestShop* const quest_shop = static_cast<CQuestShop*>(
        GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    quest_shop->makePacket_BuyingStatusList(packet, true);
    return 1;
}
