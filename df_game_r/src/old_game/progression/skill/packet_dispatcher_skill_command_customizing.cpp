#include "old_game/progression/skill/packet_dispatcher_skill_command_customizing.h"

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_Skill_Command_Customizing::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_Skill_Command_Customizing::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x859719C
// IDA_NAME: _ZN36Dispatcher_Skill_Command_CustomizingC2Ev
// DEMANGLED: Dispatcher_Skill_Command_Customizing::Dispatcher_Skill_Command_Customizing(void)
Dispatcher_Skill_Command_Customizing::Dispatcher_Skill_Command_Customizing() {}

// IDA_EA: 0x824559E / 0x82455CE
// IDA_NAME: _ZN36Dispatcher_Skill_Command_CustomizingD2Ev / D0Ev
// DEMANGLED: Dispatcher_Skill_Command_Customizing::~Dispatcher_Skill_Command_Customizing()
Dispatcher_Skill_Command_Customizing::~Dispatcher_Skill_Command_Customizing() {}

// IDA_EA: 0x81D29AC
// IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Skill_Command_Customizing::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Skill_Command_Customizing::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    MSG_SKILL_COMMAND& request = static_cast<MSG_SKILL_COMMAND&>(message);
    if (packet.get_byte(request.request_count) != 1) {
        return LineFunc(12280, kReadSignature, 0, 0);
    }

    // IDA's loop intentionally has no count clamp here.  The later process
    // gate rejects 50 and above; retain the original wire-read ordering.
    for (int index = 0; index < static_cast<int>(request.request_count);
         ++index) {
        MSG_SKILL_COMMAND::CommandRequest& command = request.requests[index];
        if (packet.get_short(command.skill_index) != 1) {
            return LineFunc(12287, kReadSignature, 0, 0);
        }
        if (packet.get_byte(command.command_length) != 1) {
            return LineFunc(12290, kReadSignature, 0, 0);
        }
        for (int code_index = 0;
             code_index < static_cast<int>(command.command_length);
             ++code_index) {
            if (packet.get_byte(command.command[code_index]) != 1) {
                return LineFunc(12297, kReadSignature, 0, 0);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x81D2B42
// IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Skill_Command_Customizing::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Skill_Command_Customizing::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    MSG_SKILL_COMMAND& request = static_cast<MSG_SKILL_COMMAND&>(message);
    stSkillCommandParam& result = static_cast<stSkillCommandParam&>(param);
    if (request.request_count > 49u) {
        result.result = 252;
        return 0;
    }

    for (int index = 0; index < static_cast<int>(request.request_count);
         ++index) {
        const MSG_SKILL_COMMAND::CommandRequest& command = request.requests[index];
        STSkillCommandData command_data;
        command_data.word_00 = command.skill_index;
        command_data.byte_02 = command.command_length;

        CSkill* const skill = G_CDataManager()->find_skill(
            user->get_charac_job(), static_cast<int>(command_data.word_00));
        if (!skill) {
            result.result = 252;
            break;
        }
        if (skill->isCommandCustomizing() != 1) {
            result.result = 249;
            break;
        }
        if (command_data.byte_02 > 5u || command_data.byte_02 == 0u) {
            result.result = 251;
            break;
        }

        for (int code_index = 0;
             code_index < static_cast<int>(command_data.byte_02);
             ++code_index) {
            command_data.raw_03[code_index] = command.command[code_index];
            if (command_data.raw_03[code_index] > 8u) {
                result.result = 250;
                break;
            }
        }
        if (command_data.raw_03[command_data.byte_02 - 1u] < 4u ||
            command_data.raw_03[command_data.byte_02 - 1u] == 7u) {
            result.result = 250;
            break;
        }
        if (user->customizingSkillCommand(command_data) != 1) {
            result.result = 252;
            break;
        }
    }

    if (result.result != 0) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            kProcessSignature,
            12369,
            "SKILL_COMMAND_LOG : CUSTOMIZING ERROR!! ALL COMMAND DEFAULT!! m_id (%s)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
        user->defaultSkillCommand();
    }
    return 0;
}

// IDA_EA: 0x81D2DA4
// IDA_NAME: _ZN36Dispatcher_Skill_Command_Customizing4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Skill_Command_Customizing::send(CUser *,ParamBase &)
void Dispatcher_Skill_Command_Customizing::send(CUser* user, ParamBase& param) {
    const stSkillCommandParam& result =
        static_cast<const stSkillCommandParam&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SKILL_COMMAND_CUSTOMIZING);
    if (result.result != 0) {
        packet.put_byte(0);
        packet.put_short(result.result);
    } else {
        packet.put_byte(1);
    }
    packet.finalize(1);
    user->Send(packet);
}
