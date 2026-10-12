#include "old_data/Login/packet_dispatch/guild_board_dispatchers.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kOpenGuildBoardReadSignature[] =
    "virtual int Dispatcher_Open_Guild_Board::read(PacketBuf&, MSG_BASE&)";
const char kWriteGuildBoardReadSignature[] =
    "virtual int Dispatcher_Write_On_The_GuildBoard::read(PacketBuf&, MSG_BASE&)";
const char kDeleteGuildBoardReadSignature[] =
    "virtual int Dispatcher_Delete_GuildBoard_Text::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x822B83C
// IDA_NAME: _ZN18STGuildBoardDBInfoC2Ev
// DEMANGLED: STGuildBoardDBInfo::STGuildBoardDBInfo(void)
STGuildBoardDBInfo::STGuildBoardDBInfo()
    : author() {
    std::memset(text, 0, sizeof(text));
    created_time = 0;
    board_index = 0;
    author_charac_no = 0;
}

// This is the sole old state-3 template/vtable family for CMD341--343.
// Its root table registration remains deferred with PacketDispatcher itself.
// IDA_EA: 0x8252666
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x82527D0
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x82527DA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825286E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825C77E / 0x825C7AE
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EED0Ev
// IDA_EA: 0x85989B2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_GUILD_BOARD17stGuildBoardParamL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_GUILD_BOARD,
                                 stGuildBoardParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x85971D4
// IDA_NAME: _ZN27Dispatcher_Open_Guild_BoardC2Ev
// DEMANGLED: Dispatcher_Open_Guild_Board::Dispatcher_Open_Guild_Board(void)
Dispatcher_Open_Guild_Board::Dispatcher_Open_Guild_Board()
    : IPacketDispatcher<MSG_GUILD_BOARD,
                        stGuildBoardParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245502
// IDA_NAME: _ZN27Dispatcher_Open_Guild_BoardD2Ev
// DEMANGLED: Dispatcher_Open_Guild_Board::~Dispatcher_Open_Guild_Board()
// IDA_EA: 0x8245532
// IDA_NAME: _ZN27Dispatcher_Open_Guild_BoardD0Ev
// DEMANGLED: Dispatcher_Open_Guild_Board::~Dispatcher_Open_Guild_Board()
Dispatcher_Open_Guild_Board::~Dispatcher_Open_Guild_Board() {}

// IDA_EA: 0x81D307E
// IDA_NAME: _ZN27Dispatcher_Open_Guild_Board4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Open_Guild_Board::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Open_Guild_Board::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GUILD_BOARD& request = static_cast<MSG_GUILD_BOARD&>(message);
    if (packet.get_byte(request.request_kind) == 1) {
        return 0;
    }
    return LineFunc(12458, kOpenGuildBoardReadSignature, 0, 0);
}

// IDA_EA: 0x81D30D0
// IDA_NAME: _ZN27Dispatcher_Open_Guild_Board7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Open_Guild_Board::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Open_Guild_Board::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    const MSG_GUILD_BOARD& request = static_cast<const MSG_GUILD_BOARD&>(message);
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendGuildBoardOpen(user->getCurCharacNo(),
                              user->get_charac_guildkey(),
                              static_cast<char>(request.request_kind));
    return 0;
}

// IDA_EA: 0x85971F0
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoardC2Ev
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::Dispatcher_Write_On_The_GuildBoard(void)
Dispatcher_Write_On_The_GuildBoard::Dispatcher_Write_On_The_GuildBoard()
    : IPacketDispatcher<MSG_GUILD_BOARD,
                        stGuildBoardParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82454B4
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoardD2Ev
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::~Dispatcher_Write_On_The_GuildBoard()
// IDA_EA: 0x82454E4
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoardD0Ev
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::~Dispatcher_Write_On_The_GuildBoard()
Dispatcher_Write_On_The_GuildBoard::~Dispatcher_Write_On_The_GuildBoard() {}

// IDA_EA: 0x81D314A
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoard4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Write_On_The_GuildBoard::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    int text_length = 0;
    MSG_GUILD_BOARD& request = static_cast<MSG_GUILD_BOARD&>(message);
    if (packet.get_int(text_length) != 1) {
        return LineFunc(12485, kWriteGuildBoardReadSignature, 0, 0);
    }
    if (packet.get_str(request.board.text,
                       static_cast<int>(sizeof(request.board.text)),
                       text_length) == 1) {
        return 0;
    }
    return LineFunc(12488, kWriteGuildBoardReadSignature, 0, 0);
}

// IDA_EA: 0x81D31EE
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoard7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Write_On_The_GuildBoard::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    MSG_GUILD_BOARD& request = static_cast<MSG_GUILD_BOARD&>(message);
    stGuildBoardParam& result = static_cast<stGuildBoardParam&>(param);
    if (!request.board.text[0]) {
        result.result = 1;
        return 0;
    }
    if (G_CDataManager()->hasPreventString(request.board.text) ||
        G_CDataManager()->hasSpecialCharacterExceptSpace(request.board.text)) {
        result.result = 1;
        return 0;
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendGuildBoardWrite(request.board.text,
                               user->get_acc_id(),
                               user->getCurCharacNo(),
                               user->get_charac_job(),
                               user->getCurCharacGrowType(),
                               user->get_charac_guildkey());
    return 0;
}

// IDA_EA: 0x81D3320
// IDA_NAME: _ZN34Dispatcher_Write_On_The_GuildBoard4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Write_On_The_GuildBoard::send(CUser *,ParamBase &)
void Dispatcher_Write_On_The_GuildBoard::send(CUser* user, ParamBase& param) {
    const stGuildBoardParam& result = static_cast<const stGuildBoardParam&>(param);
    user->SendCmdErrorPacket(
        ENUM_CMDPACKET_WRITE_ON_THE_GUILDBOARD,
        static_cast<unsigned char>(result.result));
}

// IDA_EA: 0x859720C
// IDA_NAME: _ZN33Dispatcher_Delete_GuildBoard_TextC2Ev
// DEMANGLED: Dispatcher_Delete_GuildBoard_Text::Dispatcher_Delete_GuildBoard_Text(void)
Dispatcher_Delete_GuildBoard_Text::Dispatcher_Delete_GuildBoard_Text()
    : IPacketDispatcher<MSG_GUILD_BOARD,
                        stGuildBoardParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245466
// IDA_NAME: _ZN33Dispatcher_Delete_GuildBoard_TextD2Ev
// DEMANGLED: Dispatcher_Delete_GuildBoard_Text::~Dispatcher_Delete_GuildBoard_Text()
// IDA_EA: 0x8245496
// IDA_NAME: _ZN33Dispatcher_Delete_GuildBoard_TextD0Ev
// DEMANGLED: Dispatcher_Delete_GuildBoard_Text::~Dispatcher_Delete_GuildBoard_Text()
Dispatcher_Delete_GuildBoard_Text::~Dispatcher_Delete_GuildBoard_Text() {}

// IDA_EA: 0x81D334E
// IDA_NAME: _ZN33Dispatcher_Delete_GuildBoard_Text4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Delete_GuildBoard_Text::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Delete_GuildBoard_Text::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    MSG_GUILD_BOARD& request = static_cast<MSG_GUILD_BOARD&>(message);
    if (packet.get_int(request.board.board_index) == 1) {
        return 0;
    }
    return LineFunc(12545, kDeleteGuildBoardReadSignature, 0, 0);
}

// IDA_EA: 0x81D33A0
// IDA_NAME: _ZN33Dispatcher_Delete_GuildBoard_Text7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Delete_GuildBoard_Text::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Delete_GuildBoard_Text::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)param;
    const MSG_GUILD_BOARD& request = static_cast<const MSG_GUILD_BOARD&>(message);
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendGuildBoardDelete(request.board.board_index,
                                user->getCurCharacNo(),
                                user->get_charac_guildkey());
    return 0;
}

// IDA_EA: 0x84DEE6A
// IDA_NAME: _ZN20Inter_GuildBoardData12dispatch_sigEP5CUserPci
int Inter_GuildBoardData::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;
    const GuildBoardReplyPrefix& reply =
        *reinterpret_cast<const GuildBoardReplyPrefix*>(payload);
    if (user->get_state() <= 2 ||
        user->get_charac_no(-1) != static_cast<int>(reply.charac_no)) {
        return 0;
    }

    PacketGuard packet;
    if (reply.request_type == 9002u) {
        packet.put_header(1, ENUM_CMDPACKET_OPEN_GUILD_BOARD);
    } else if (reply.request_type == 9006u) {
        packet.put_header(1, ENUM_CMDPACKET_WRITE_ON_THE_GUILDBOARD);
    }
    if (reply.error) {
        packet.put_byte(0);
        packet.put_short(reply.error);
    } else {
        packet.put_byte(1);
        packet.put_byte(reply.total_count);
        packet.put_byte(reply.entry_count);
        const GuildBoardReplyEntry* const entries =
            reinterpret_cast<const GuildBoardReplyEntry*>(payload + sizeof(reply));
        for (int index = 0; index < static_cast<int>(reply.entry_count); ++index) {
            const GuildBoardReplyEntry& entry = entries[index];
            packet.put_byte(entry.can_delete == 1);
            packet.put_byte(entry.author_job);
            packet.put_byte(user->get_charac_no(-1) ==
                            static_cast<int>(entry.author_charac_no));
            const int author_name_length = static_cast<int>(std::strlen(entry.author_name));
            packet.put_int(author_name_length);
            packet.put_str(entry.author_name, author_name_length);
            packet.put_int(entry.board_index);
            int elapsed = GlobalData::system_time()->getCurSec() - entry.created_time;
            if (elapsed < 0) {
                elapsed = 0;
            }
            packet.put_int(elapsed);
            const int text_length = static_cast<int>(std::strlen(entry.text));
            packet.put_int(text_length);
            packet.put_str(entry.text, text_length);
        }
        cMyTrace trace(
            "virtual int Inter_GuildBoardData::dispatch_sig(CUser*, char*, int)",
            19281,
            0);
        trace("GUILD_BOARD : CHARAC NO : %u, CODE : %u, TOTAL CNT : %u, CNT : %u : BOARD DATA SEND!!",
              user->get_charac_no(-1),
              reply.request_type,
              reply.total_count,
              reply.entry_count);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE4C8
// IDA_NAME: _ZN20Inter_GuildBoardDataC2Ev
Inter_GuildBoardData::Inter_GuildBoardData() {}

// IDA_EA: 0x84F751E / 0x84F754E
// IDA_NAME: _ZN20Inter_GuildBoardDataD2Ev / _ZN20Inter_GuildBoardDataD0Ev
Inter_GuildBoardData::~Inter_GuildBoardData() {}

// IDA_EA: 0x84DF2AC
// IDA_NAME: _ZN22Inter_GuildBoardDelete12dispatch_sigEP5CUserPci
int Inter_GuildBoardDelete::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)payload_size;
    const GuildBoardReplyPrefix& reply =
        *reinterpret_cast<const GuildBoardReplyPrefix*>(payload);
    if (user->get_state() <= 2 ||
        user->get_charac_no(-1) != static_cast<int>(reply.charac_no)) {
        return 0;
    }
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_DELETE_GUILDBOARD_TEXT);
    if (reply.error) {
        packet.put_byte(0);
        packet.put_short(reply.error);
    } else {
        packet.put_byte(1);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE4E4
// IDA_NAME: _ZN22Inter_GuildBoardDeleteC2Ev
Inter_GuildBoardDelete::Inter_GuildBoardDelete() {}

// IDA_EA: 0x84F74D0 / 0x84F7500
// IDA_NAME: _ZN22Inter_GuildBoardDeleteD2Ev / _ZN22Inter_GuildBoardDeleteD0Ev
Inter_GuildBoardDelete::~Inter_GuildBoardDelete() {}
