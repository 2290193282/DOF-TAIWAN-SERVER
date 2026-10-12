#include "old_data/Login/packet_dispatch/combo_skill_dispatcher.h"

#include <stdint.h>

#include <vector>

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kComboSkillReadSignature[] =
    "virtual int Dispatcher_ComboSkill::read(PacketBuf&, MSG_BASE&)";
const char kComboSkillProcessSignature[] =
    "virtual int Dispatcher_ComboSkill::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// The original specialization is the sole CMD509 template/vtable provider.
// The 606-entry PacketDispatcher registration remains intentionally absent.
// IDA_EA: 0x826153A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82616B8
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82616C2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8262434
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8262464
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8262624
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8599176
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_COMBO_SKILL12stComboSkillL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL,stComboSkill,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_COMBO_SKILL,
                                 stComboSkill,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597E52
// IDA_NAME: _ZN21Dispatcher_ComboSkillC2Ev
// DEMANGLED: Dispatcher_ComboSkill::Dispatcher_ComboSkill(void)
Dispatcher_ComboSkill::Dispatcher_ComboSkill()
    : IPacketDispatcher<MSG_COMBO_SKILL,
                        stComboSkill,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8260A40
// IDA_NAME: _ZN21Dispatcher_ComboSkillD2Ev
// DEMANGLED: Dispatcher_ComboSkill::~Dispatcher_ComboSkill()
// IDA_EA: 0x8260A70
// IDA_NAME: _ZN21Dispatcher_ComboSkillD0Ev
// DEMANGLED: Dispatcher_ComboSkill::~Dispatcher_ComboSkill()
Dispatcher_ComboSkill::~Dispatcher_ComboSkill() {}

// IDA_EA: 0x825EF78
// IDA_NAME: _ZN21Dispatcher_ComboSkill4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ComboSkill::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ComboSkill::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_COMBO_SKILL& request = static_cast<MSG_COMBO_SKILL&>(message);
    request.combo_skills.clear();

    if (packet.get_byte(request.skill_tree_index) != 1) {
        return LineFunc(769, kComboSkillReadSignature, 0, 0);
    }

    char combo_count = 0;
    if (packet.get_byte(combo_count) != 1) {
        return LineFunc(772, kComboSkillReadSignature, 0, 0);
    }

    for (int combo_number = 0;
         combo_number < combo_count && combo_number <= 5;
         ++combo_number) {
        int16_t combo_index = 0;
        if (packet.get_short(combo_index) != 1) {
            return LineFunc(780, kComboSkillReadSignature, 0, 0);
        }

        if (checkComboSkillIndex(combo_index) != 1) {
            continue;
        }

        char skill_count = 0;
        if (packet.get_byte(skill_count) != 1) {
            return LineFunc(786, kComboSkillReadSignature, 0, 0);
        }

        std::vector<int> skills;
        for (int skill_number = 0;
             skill_number < skill_count && skill_number <= 5;
             ++skill_number) {
            int16_t skill_index = 0;
            if (packet.get_short(skill_index) != 1) {
                return LineFunc(793, kComboSkillReadSignature, 0, 0);
            }
            skills.push_back(skill_index);
        }
        request.combo_skills.insert(std::make_pair(combo_index, skills));
    }
    return 0;
}

// IDA_EA: 0x825F292
// IDA_NAME: _ZN21Dispatcher_ComboSkill20checkComboSkillIndexEs
// DEMANGLED: Dispatcher_ComboSkill::checkComboSkillIndex(short)
int Dispatcher_ComboSkill::checkComboSkillIndex(short skill_index) const {
    return G_CDataManager()->is_combo_skill_index(skill_index) ? 1 : 0;
}

// IDA_EA: 0x825F2FC
// IDA_NAME: _ZN21Dispatcher_ComboSkill7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ComboSkill::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ComboSkill::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    stComboSkill& result = static_cast<stComboSkill&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(831, kComboSkillProcessSignature, result.result, 0);
    }

    MSG_COMBO_SKILL& request = static_cast<MSG_COMBO_SKILL&>(message);
    const ENUM_SKILL_TREE_KIND skill_tree =
        static_cast<ENUM_SKILL_TREE_KIND>(
            static_cast<int8_t>(request.skill_tree_index));
    SkillSlot* const skill_slot = user->getCurCharacSkillW();
    skill_slot->setComboSkillInfo(skill_tree, request.combo_skills);
    skill_slot->verifyComboSkillTree(skill_tree);
    // Rearranging a combo need not buy/move a learned skill, but UpdateData
    // queues SIG797 only inside isSaveSkill().  Mark this edit and snapshot
    // both combo trees now so a character reselect cannot reload old rows.
    user->enableSaveSkill();
    user->updateComboSkill();
    return 0;
}

// IDA_EA: 0x825F3CE
// IDA_NAME: _ZN21Dispatcher_ComboSkill11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ComboSkill::check_error(CUser *,MSG_BASE &)
int Dispatcher_ComboSkill::check_error(CUser* user, MSG_BASE& message) {
    if (!user || user->get_state() <= 2 ||
        user->get_charac_job() !=
            oldsrv::config::kCharacterJobDemonicSwordman) {
        return -1;
    }
    const MSG_COMBO_SKILL& request = static_cast<const MSG_COMBO_SKILL&>(message);
    return request.skill_tree_index < 2u ? 0 : 19;
}

// IDA_EA: 0x825F444
// IDA_NAME: _ZN21Dispatcher_ComboSkill4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ComboSkill::send(CUser *,ParamBase &)
void Dispatcher_ComboSkill::send(CUser* user, ParamBase& param) {
    const stComboSkill& result = static_cast<const stComboSkill&>(param);
    if (result.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMBO_SKILL_INFO,
                                 static_cast<unsigned char>(result.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_COMBO_SKILL_INFO);
}
