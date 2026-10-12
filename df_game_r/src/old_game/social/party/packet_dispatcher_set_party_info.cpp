#include "old_game/social/party/packet_dispatcher_set_party_info.h"

#include <string>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/slang_filter.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x81C4396
// IDA_NAME: _ZN23DisPatcher_SetPartyInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPartyInfo::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPartyInfo::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_PARTY_SET_INFO& message =
        static_cast<MSG_PARTY_SET_INFO&>(base_message);
    if (packet.get_byte(message.title_index) != 1) {
        return LineFunc(4313,
                        "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (!message.title_index) {
        if (packet.get_int(message.title_length) != 1) {
            return LineFunc(4317,
                            "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                            0, 0);
        }
        if (packet.get_str(message.title, 32, message.title_length) != 1) {
            return LineFunc(4320,
                            "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                            0, 0);
        }
    }
    if (packet.get_byte(message.user_max) != 1) {
        return LineFunc(4322,
                        "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(message.dungeon_index) != 1) {
        return LineFunc(4325,
                        "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(message.dungeon_difficulty) == 1) {
        return 0;
    }
    return LineFunc(4326,
                    "virtual int DisPatcher_SetPartyInfo::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C4562
// IDA_NAME: _ZN23DisPatcher_SetPartyInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPartyInfo::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPartyInfo::process(CUser* user,
                                      MSG_BASE& base_message,
                                      ParamBase& base_param) {
    stSetPartyInfo& param = static_cast<stSetPartyInfo&>(base_param);
    param.raw_04 = check_error(user, base_message);
    if (param.raw_04 > 0) {
        return 0;
    }
    if (param.raw_04 < 0) {
        return LineFunc(4342,
                        "virtual int DisPatcher_SetPartyInfo::process(CUser*, MSG_BASE&, ParamBase&)",
                        param.raw_04, user->get_acc_id());
    }

    const MSG_PARTY_SET_INFO& message =
        static_cast<const MSG_PARTY_SET_INFO&>(base_message);
    CParty* party = user->GetParty();
    if (party) {
        if (party->getManager() != user) {
            param.raw_04 = 8;
            return 0;
        }
        party->SetPartyInfoUI(message);
        G_GameWorld()->send_party_info_to_all(party, 1);
        return 0;
    }

    party = G_CGameManager()->GetParty();
    if (!party) {
        param.raw_04 = 4;
        return 0;
    }
    party->create_party(user);
    party->SetPartyInfoUI(message);
    G_GameWorld()->send_party_info_to_all(party, 0);
    return 0;
}

// IDA_EA: 0x81C46C0
// IDA_NAME: _ZN23DisPatcher_SetPartyInfo4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SetPartyInfo::send(CUser *,ParamBase &)
void DisPatcher_SetPartyInfo::send(CUser* user, ParamBase& base_param) {
    const int result = static_cast<stSetPartyInfo&>(base_param).raw_04;
    if (result != 0x7fffffff && result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_PARTY_INFO,
                                 static_cast<unsigned char>(result));
    }
}

// IDA_EA: 0x81C4708
// IDA_NAME: _ZN23DisPatcher_SetPartyInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_SetPartyInfo::check_error(CUser *,MSG_BASE &)
int DisPatcher_SetPartyInfo::check_error(CUser*, MSG_BASE& base_message) {
    const MSG_PARTY_SET_INFO& message =
        static_cast<const MSG_PARTY_SET_INFO&>(base_message);
    const std::string title(message.title);
    return CSyncSlangFilter::GetInstance()->HasSlang(title) ? 159 : 0;
}

// IDA_EA: 0x8246306
// IDA_NAME: _ZN23DisPatcher_SetPartyInfoD2Ev
// DEMANGLED: DisPatcher_SetPartyInfo::~DisPatcher_SetPartyInfo()
// IDA_EA: 0x8246336
// IDA_NAME: _ZN23DisPatcher_SetPartyInfoD0Ev
// DEMANGLED: DisPatcher_SetPartyInfo::~DisPatcher_SetPartyInfo()
DisPatcher_SetPartyInfo::~DisPatcher_SetPartyInfo() {}

// IDA_EA: 0x85959FC
// IDA_NAME: _ZN23DisPatcher_SetPartyInfoC2Ev
// DEMANGLED: DisPatcher_SetPartyInfo::DisPatcher_SetPartyInfo(void)
DisPatcher_SetPartyInfo::DisPatcher_SetPartyInfo()
    : IPacketDispatcher<MSG_PARTY_SET_INFO,
                        stSetPartyInfo,
                        static_cast<ch_state>(3)>() {}
