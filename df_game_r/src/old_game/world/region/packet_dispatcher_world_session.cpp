#include "old_game/world/region/packet_dispatcher_world_session.h"

#include <cstring>
#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/shared_server_message_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/packet_dispatcher_world_session_abi.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8596C40
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUserC2Ev
// DEMANGLED: Dispatcher_ReportAbuseUser::Dispatcher_ReportAbuseUser(void)
Dispatcher_ReportAbuseUser::Dispatcher_ReportAbuseUser() {}

// IDA_EA: 0x8241110
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUserD2Ev
// DEMANGLED: Dispatcher_ReportAbuseUser::~Dispatcher_ReportAbuseUser()
// IDA_EA: 0x8241140
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUserD0Ev
// DEMANGLED: Dispatcher_ReportAbuseUser::~Dispatcher_ReportAbuseUser()
Dispatcher_ReportAbuseUser::~Dispatcher_ReportAbuseUser() {}

// IDA_EA: 0x821EF48
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUser12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ReportAbuseUser::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ReportAbuseUser::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            55201,
            "virtual int Dispatcher_ReportAbuseUser::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (pvp_assault::GetInstanceAssaultMgr()->OnAbuseUser(user) == 1) {
        return 0;
    }
    return LineFunc(
        55217,
        "virtual int Dispatcher_ReportAbuseUser::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233EF2
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUser4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ReportAbuseUser::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ReportAbuseUser::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233EFC
// IDA_NAME: _ZN26Dispatcher_ReportAbuseUser7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ReportAbuseUser::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ReportAbuseUser::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859624C
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlagC2Ev
// DEMANGLED: Dispatcher_ChangeTutorialFlag::Dispatcher_ChangeTutorialFlag(void)
Dispatcher_ChangeTutorialFlag::Dispatcher_ChangeTutorialFlag() {}

// IDA_EA: 0x8242838
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlagD2Ev
// DEMANGLED: Dispatcher_ChangeTutorialFlag::~Dispatcher_ChangeTutorialFlag()
// IDA_EA: 0x8242868
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlagD0Ev
// DEMANGLED: Dispatcher_ChangeTutorialFlag::~Dispatcher_ChangeTutorialFlag()
Dispatcher_ChangeTutorialFlag::~Dispatcher_ChangeTutorialFlag() {}

// IDA_EA: 0x820697C
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlag12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(
            41237,
            "virtual int Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint32_t flag = 0;
    if (packet.get_int(flag) != 1) {
        return LineFunc(
            41240,
            "virtual int Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    uint8_t reward = 0;
    if (packet.get_byte(reward) != 1) {
        return LineFunc(
            41243,
            "virtual int Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (flag > 77u) {
        return LineFunc(
            41248,
            "virtual int Dispatcher_ChangeTutorialFlag::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->getCurCharacR()) {
        return 0;
    }

    if (flag != 77u) {
        if (user->setCurCharacTutorialFlag(flag) != 1) {
            return 0;
        }
        if (reward != 0) {
            user->RewardTutorial(flag);
        }
        if (flag == 31u) {
            CParty* const party = user->GetParty();
            if (party) {
                party->giveup_game(user, true, false, false);
            }
        }
        return 0;
    }

    for (uint32_t current = 0; current <= 77u; ++current) {
        user->setCurCharacTutorialFlag(current);
    }
    return 0;
}

// IDA_EA: 0x82338DA
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlag4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeTutorialFlag::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeTutorialFlag::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82338E4
// IDA_NAME: _ZN29Dispatcher_ChangeTutorialFlag7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeTutorialFlag::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeTutorialFlag::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595484
// IDA_NAME: _ZN22DisPatcher_IncreStatusC2Ev
// DEMANGLED: DisPatcher_IncreStatus::DisPatcher_IncreStatus(void)
DisPatcher_IncreStatus::DisPatcher_IncreStatus() {}

// IDA_EA: 0x8232CCC
// IDA_NAME: _ZN22DisPatcher_IncreStatusD2Ev
// DEMANGLED: DisPatcher_IncreStatus::~DisPatcher_IncreStatus()
// IDA_EA: 0x8232CFC
// IDA_NAME: _ZN22DisPatcher_IncreStatusD0Ev
// DEMANGLED: DisPatcher_IncreStatus::~DisPatcher_IncreStatus()
DisPatcher_IncreStatus::~DisPatcher_IncreStatus() {}

// IDA_EA: 0x81EDE34
// IDA_NAME: _ZN22DisPatcher_IncreStatus12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_IncreStatus::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_IncreStatus::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    short slot = 0;
    if (packet.get_short(slot) != 1) {
        return LineFunc(
            28924,
            "virtual int DisPatcher_IncreStatus::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CItem* const item = GlobalData::s_pSecuProtectionField->GetItemPtr(
        user, 0, slot);
    if (!item) {
        return LineFunc(
            28929,
            "virtual int DisPatcher_IncreStatus::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int check_result = GlobalData::s_pSecuProtectionField->CheckItem(
        user, static_cast<SECURITY_PROTCTION>(42), item);
    if (check_result != 0) {
        PacketGuard error_packet;
        static_cast<void>(user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INCREASE_STATUS,
            static_cast<unsigned char>(check_result),
            error_packet));
        return 0;
    }

    if (slot <= 152) {
        user->increase_status(slot);
        return 0;
    }

    user->UseQuestItem(static_cast<int>(static_cast<unsigned short>(slot)));
    cMyTrace trace(
        "virtual int DisPatcher_IncreStatus::dispatch_sig(CUser*, PacketBuf&)",
        28943,
        5);
    trace("__ USE QUEST ITEM __ %d", static_cast<int>(slot));
    return 0;
}

// IDA_EA: 0x8232CB8
// IDA_NAME: _ZN22DisPatcher_IncreStatus4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_IncreStatus::read(PacketBuf &,MSG_BASE &)
int DisPatcher_IncreStatus::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232CC2
// IDA_NAME: _ZN22DisPatcher_IncreStatus7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_IncreStatus::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_IncreStatus::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596A48
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessageC2Ev
// DEMANGLED: Dispatcher_UpdateServerMessage::Dispatcher_UpdateServerMessage(void)
Dispatcher_UpdateServerMessage::Dispatcher_UpdateServerMessage() {}

// IDA_EA: 0x824141C
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessageD2Ev
// DEMANGLED: Dispatcher_UpdateServerMessage::~Dispatcher_UpdateServerMessage()
// IDA_EA: 0x824144C
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessageD0Ev
// DEMANGLED: Dispatcher_UpdateServerMessage::~Dispatcher_UpdateServerMessage()
Dispatcher_UpdateServerMessage::~Dispatcher_UpdateServerMessage() {}

// IDA_EA: 0x821D9EE
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessage12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpdateServerMessage::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpdateServerMessage::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        const char* const account_id = NumberToString(
            static_cast<unsigned int>(user->get_acc_id()), 0);
        const int state = user->get_state();
        cMyTrace trace(
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            53702,
            0);
        trace("ServerMessage : Dispatcher_UpdateServerMessage => User State : %d, m_id : %s",
              state,
              account_id);
        return LineFunc(
            53703,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SERVER_MESSAGE_SEND);

    uint8_t message_index = 0;
    if (static_cast<unsigned char>(packet.get_byte(message_index)) != 1) {
        return LineFunc(
            53712,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    int message_length = 0;
    if (static_cast<unsigned char>(packet.get_int(message_length)) != 1) {
        return LineFunc(
            53717,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (static_cast<unsigned int>(message_length) >= 50u) {
        response.put_byte(0);
        response.put_byte(1);
        response.finalize(true);
        user->Send(response);
        return 0;
    }

    char message[50];
    if (static_cast<unsigned char>(
            packet.get_str(message, sizeof(message), message_length)) != 1) {
        return LineFunc(
            53732,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (static_cast<unsigned char>(G_CDataManager()->hasPreventString(message)) !=
        0) {
        response.put_byte(0);
        response.put_byte(23);
        response.finalize(true);
        user->Send(response);
        return 0;
    }

    int character_name_length = 0;
    if (static_cast<unsigned char>(packet.get_int(character_name_length)) != 1) {
        return LineFunc(
            53748,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char character_name[30] = {};
    if (static_cast<unsigned char>(packet.get_str(character_name,
                                                   sizeof(character_name),
                                                   character_name_length)) != 1) {
        return LineFunc(
            53753,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (std::strcmp(character_name, user->getCurCharacName()) != 0) {
        return LineFunc(
            53757,
            "virtual int Dispatcher_UpdateServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int cur_second = GlobalData::s_systemTime_.getCurSec();
    CSharedServerMessageManager* const manager =
        G_CGameManager()->GetSharedServerMessageManager();
    if (manager) {
        int result = 0;
        if (static_cast<unsigned char>(
                manager->CheckUserCondition(user, message_index, result)) != 0) {
            if (static_cast<unsigned char>(
                    manager->IsChangableMessage(message_index, cur_second)) != 0) {
                manager->UpdateServerMessage(user,
                                             message_index,
                                             message,
                                             character_name,
                                             cur_second);
            } else {
                response.put_byte(0);
            }
            response.put_byte(1);
        } else {
            response.put_byte(0);
            response.put_byte(result ? result : 3);
        }
    }
    response.put_byte(message_index);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233E16
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessage4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpdateServerMessage::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpdateServerMessage::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E20
// IDA_NAME: _ZN30Dispatcher_UpdateServerMessage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpdateServerMessage::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpdateServerMessage::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596A64
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessageC2Ev
// DEMANGLED: Dispatcher_CheckEnableServerMessage::Dispatcher_CheckEnableServerMessage(void)
Dispatcher_CheckEnableServerMessage::Dispatcher_CheckEnableServerMessage() {}

// IDA_EA: 0x82413CE
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessageD2Ev
// DEMANGLED: Dispatcher_CheckEnableServerMessage::~Dispatcher_CheckEnableServerMessage()
// IDA_EA: 0x82413FE
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessageD0Ev
// DEMANGLED: Dispatcher_CheckEnableServerMessage::~Dispatcher_CheckEnableServerMessage()
Dispatcher_CheckEnableServerMessage::~Dispatcher_CheckEnableServerMessage() {}

// IDA_EA: 0x821DF6A
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessage12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CheckEnableServerMessage::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CheckEnableServerMessage::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    if (user->get_state() != 3) {
        const char* const account_id = NumberToString(
            static_cast<unsigned int>(user->get_acc_id()), 0);
        const int state = user->get_state();
        cMyTrace trace(
            "virtual int Dispatcher_CheckEnableServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            53814,
            0);
        trace("ServerMessage : Dispatcher_CheckEnableServerMessage => User State : %d, m_id : %s",
              state,
              account_id);
        return LineFunc(
            53815,
            "virtual int Dispatcher_CheckEnableServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint8_t message_index = 0;
    if (static_cast<unsigned char>(packet.get_byte(message_index)) != 1) {
        return LineFunc(
            53821,
            "virtual int Dispatcher_CheckEnableServerMessage::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int cur_second = GlobalData::s_systemTime_.getCurSec();
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_SERVER_MESSAGE_CHECK);
    CSharedServerMessageManager* const manager =
        G_CGameManager()->GetSharedServerMessageManager();
    if (manager) {
        int result = 0;
        if (static_cast<unsigned char>(
                manager->CheckUserCondition(user, message_index, result)) != 0) {
            if (static_cast<unsigned char>(
                    manager->IsChangableMessage(message_index, cur_second)) == 0) {
                response.put_byte(0);
            }
            response.put_byte(1);
        } else {
            response.put_byte(0);
            response.put_byte(result ? result : 3);
        }
    }
    response.put_byte(message_index);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233E2A
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessage4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CheckEnableServerMessage::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CheckEnableServerMessage::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233E34
// IDA_NAME: _ZN35Dispatcher_CheckEnableServerMessage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CheckEnableServerMessage::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CheckEnableServerMessage::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859730E
// IDA_NAME: _ZN31Dispatcher_OperateRidableObjectC2Ev
// DEMANGLED: Dispatcher_OperateRidableObject::Dispatcher_OperateRidableObject(void)
Dispatcher_OperateRidableObject::Dispatcher_OperateRidableObject()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82403F6
// IDA_NAME: _ZN31Dispatcher_OperateRidableObjectD2Ev
// DEMANGLED: Dispatcher_OperateRidableObject::~Dispatcher_OperateRidableObject()
// IDA_EA: 0x8240426
// IDA_NAME: _ZN31Dispatcher_OperateRidableObjectD0Ev
// DEMANGLED: Dispatcher_OperateRidableObject::~Dispatcher_OperateRidableObject()
Dispatcher_OperateRidableObject::~Dispatcher_OperateRidableObject() {}

// IDA_EA: 0x8226DA0
// IDA_NAME: _ZN31Dispatcher_OperateRidableObject12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_OperateRidableObject::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_OperateRidableObject::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    if (user->get_state() != 5) {
        return 0;
    }

    char requested_state = 0;
    int object_id = 0;
    if (packet.get_byte(requested_state) != 1) {
        return LineFunc(
            58020,
            "virtual int Dispatcher_OperateRidableObject::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_int(object_id) != 1) {
        return LineFunc(
            58023,
            "virtual int Dispatcher_OperateRidableObject::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }

    const int state_value = static_cast<int>(requested_state);
    if (requested_state == 2) {
        user->history_log().StartRidable(
            static_cast<cUserHistoryLog::eRidableType>(0), object_id);
    } else if (state_value == 1 || state_value == 3) {
        user->history_log().EndRidable(
            static_cast<cUserHistoryLog::eRidableType>(0), object_id);
    }

    if (party->SetRidableObjectState(
            object_id,
            static_cast<eRidableObjectState>(state_value)) == 1) {
        user->SendCmdOkPacket(ENUM_CMDPACKET_OPERATE_RIDABLE_OBJECT);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_OPERATE_RIDABLE_OBJECT, 19);
    }
    return 0;
}

// IDA_EA: 0x823424E
// IDA_NAME: _ZN31Dispatcher_OperateRidableObject4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_OperateRidableObject::read(PacketBuf &,MSG_BASE &)
int Dispatcher_OperateRidableObject::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234258
// IDA_NAME: _ZN31Dispatcher_OperateRidableObject7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_OperateRidableObject::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_OperateRidableObject::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85961F8
// IDA_NAME: _ZN20Dispatcher_DieMobAllC2Ev
// DEMANGLED: Dispatcher_DieMobAll::Dispatcher_DieMobAll(void)
Dispatcher_DieMobAll::Dispatcher_DieMobAll()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824385E
// IDA_NAME: _ZN20Dispatcher_DieMobAllD2Ev
// DEMANGLED: Dispatcher_DieMobAll::~Dispatcher_DieMobAll()
// IDA_EA: 0x824388E
// IDA_NAME: _ZN20Dispatcher_DieMobAllD0Ev
// DEMANGLED: Dispatcher_DieMobAll::~Dispatcher_DieMobAll()
Dispatcher_DieMobAll::~Dispatcher_DieMobAll() {}

// IDA_EA: 0x81ECCE6
// IDA_NAME: _ZN20Dispatcher_DieMobAll12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DieMobAll::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DieMobAll::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 5) {
        return 0;
    }

    const int party_index =
        static_cast<int>(static_cast<short>(user->GetPartyIndex()));
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party) {
        return LineFunc(
            28406,
            "virtual int Dispatcher_DieMobAll::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    if (static_cast<unsigned char>(
            party->battle_field().IsClearDungeon()) == 1u) {
        return 0;
    }

    CDungeon* const party_dungeon = party->battle_field().GetDungeon();
    const unsigned int dungeon_exception =
        (party_dungeon &&
         user->GetParty()->battle_field().GetDungeon()->get_index() == 11006)
            ? 2u
            : 0u;
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(0xCCu),
        1u,
        dungeon_exception,
        0u);
    return 0;
}

// IDA_EA: 0x8232B56
// IDA_NAME: _ZN20Dispatcher_DieMobAll4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DieMobAll::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DieMobAll::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232B60
// IDA_NAME: _ZN20Dispatcher_DieMobAll7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DieMobAll::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DieMobAll::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
