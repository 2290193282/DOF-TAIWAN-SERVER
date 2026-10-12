#ifndef OLDSRV_OLD_GAME_GAME_MASTER_PACKET_DISPATCHER_DEBUG_COMMAND_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_PACKET_DISPATCHER_DEBUG_COMMAND_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD065 is a discriminated packet.  The original dispatcher names the tail
// as Param.Sub.* and every sub-command begins at the same byte after command.
// Keep that physical protocol union explicit instead of re-reading the frame
// through numeric byte offsets.
#pragma pack(push, 1)
struct DebugCommandAddAvatarParam {
    uint32_t item_id;
    int8_t first_flag;
    uint8_t period;
    uint16_t hidden_option;
};

struct DebugCommandSetEnchantParam {
    int32_t item_space;
    int32_t inventory_slot;
    int32_t extend_info;
};

struct DebugCommandCharacterNameParam {
    int32_t length;
    char text[30];
};

union DebugCommandParam {
    DebugCommandAddAvatarParam add_avatar;
    DebugCommandSetEnchantParam set_enchant;
    DebugCommandCharacterNameParam character_name;
    uint8_t pvp_rank;
    uint32_t black_count;
    uint32_t guild_cargo_capacity;
    uint8_t guild_power_war_point;
    uint32_t user_power_war_point;
};

struct MSG_DEBUG_COMMAND : MSG_BASE {
    uint8_t command;
    DebugCommandParam param;
};
#pragma pack(pop)

static_assert(offsetof(MSG_DEBUG_COMMAND, command) == 0x0du &&
                  offsetof(MSG_DEBUG_COMMAND, param) == 0x0eu,
              "MSG_DEBUG_COMMAND legacy C2S prefix changed");
static_assert(offsetof(MSG_DEBUG_COMMAND, param.add_avatar.first_flag) ==
                      0x12u &&
                  offsetof(MSG_DEBUG_COMMAND, param.add_avatar.period) ==
                      0x13u &&
                  offsetof(MSG_DEBUG_COMMAND, param.add_avatar.hidden_option) ==
                      0x14u,
              "MSG_DEBUG_COMMAND AddAvatar layout changed");
static_assert(offsetof(MSG_DEBUG_COMMAND, param.set_enchant.inventory_slot) ==
                      0x12u &&
                  offsetof(MSG_DEBUG_COMMAND, param.set_enchant.extend_info) ==
                      0x16u &&
                  offsetof(MSG_DEBUG_COMMAND, param.character_name.text) ==
                      0x12u,
              "MSG_DEBUG_COMMAND variant layout changed");

// IDA unit: 13_database_dispatch/DisPatcher_DebugCommand.  CMD065 is a
// base-only dispatcher: the original read/process template hooks are empty
// and the command switch is guarded by the binary's fixed disabled flag.
class DisPatcher_DebugCommand
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8233412 / 0x823342E / 0x823345E.
    DisPatcher_DebugCommand();
    virtual ~DisPatcher_DebugCommand();

    // IDA_EA: 0x81FC4DE / 0x82333FE / 0x8233408.
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x858E724 / 0x858E918 / 0x858E9C4 / 0x858EAB8 / 0x858EBCC.
    int _debugCommandOn(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandOff(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandClearSkill(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandSetSkill(CUser* user, int skill_index, int level);
    int _debugCommandMasterNewSkill(CUser* user, int skill_index, int level);

    // IDA_EA: 0x858EC9C / 0x858EE54 / 0x858EFDE / 0x858F3BA / 0x858F64C.
    int _debugCommandAddItem(CUser* user, unsigned long item_index, int count);
    int _debugCommandSetGrowType(CUser* user, int kind, int grow_type);
    int _debugCommandSetLevel(CUser* user, int level);
    int _debugCommandLevelUpDown(CUser* user, int delta);
    int _debugCommandGuildLevelUp(CUser* user);

    // IDA_EA: 0x858F7B6 / 0x858FABA / 0x858FB4E / 0x858FCC4 / 0x858FDC8.
    int _debugCommandAddAvatar(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandPVPRankSet(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandSetEnchant(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandSetBlackCount(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandCargoCapacity(CUser* user, MSG_DEBUG_COMMAND& message);

    // IDA_EA: 0x858FE62 / 0x858FEE8 / 0x858FFEE / 0x8590092 / 0x859019E.
    int _debugCommandPowerwarPointUp(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandUserPowerwarPointUp(CUser* user,
                                         MSG_DEBUG_COMMAND& message);
    int _debugCommandPowerUpOn(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandInitUserPowerwarPoint(CUser* user,
                                           MSG_DEBUG_COMMAND& message);
    int _debugCommandClearInven(CUser* user, MSG_DEBUG_COMMAND& message);

    // IDA_EA: 0x8590254 / 0x859040E / 0x859044E / 0x859048E / 0x8590506.
    int _debugCommandClearAvtInven(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandPowerWarStart(CUser* user);
    int _debugCommandPowerWarEnd(CUser* user);
    int _debugCommandUserRepel(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandRequestMid(CUser* user, MSG_DEBUG_COMMAND& message);

    // IDA_EA: 0x859058A / 0x85906AC / 0x85908BA.
    int _debugCommandInvisible(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandVisible(CUser* user, MSG_DEBUG_COMMAND& message);
    int _debugCommandSetUserDefine(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DebugCommand) == 0x04u,
              "CMD065 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_PACKET_DISPATCHER_DEBUG_COMMAND_H_
