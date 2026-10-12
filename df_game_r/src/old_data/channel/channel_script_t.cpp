#include "old_data/channel/channel_script_t.h"

#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <cstdio>
#include <string>
#include <utility>


typedef char channel_script_t_Size_Must_Be_0x30[
    (sizeof(channel_script_t) == 0x30) ? 1 : -1];

namespace {

// The 0828 text writer preserves an empty trailing field on every channel
// row and on two dungeon-index lines.  The normal lexer exposes `` as an
// empty DNF_TOK_STRING instead of whitespace.
bool IsEmptyFieldSeparatorToken() {
    if (g_dnfLex.getLastToken() != DNF_TOK_STRING) {
        return false;
    }
    const char* const text = g_dnfLex.getString();
    return text != 0 && text[0] == '\0';
}

}  // namespace

// IDA_EA: 0x837DC56
// IDA_NAME: _ZN16channel_script_tC2Ev
// DEMANGLED: channel_script_t::channel_script_t(void)
channel_script_t::channel_script_t() : dungeon_infos(), channel_infos() {}

// IDA_EA: 0x837DC9C
// IDA_NAME: _ZN16channel_script_tD2Ev
// DEMANGLED: channel_script_t::~channel_script_t()
channel_script_t::~channel_script_t() {}

// IDA_EA: 0x88DCC28
// IDA_NAME: _ZNK16channel_script_t19GetPvPChannelNumberEhj
// DEMANGLED: channel_script_t::GetPvPChannelNumber(unsigned char,unsigned int)const
int channel_script_t::GetPvPChannelNumber(uint8_t server_group,
                                          uint32_t channel_no) const {
    ServerChannelInfoMap::const_iterator server =
        channel_infos.find(static_cast<int>(server_group));
    if (server == channel_infos.end()) {
        return -1;
    }

    int pvp_channel_number = -1;
    for (ChannelInfoMap::const_iterator channel = server->second.begin();
         channel != server->second.end();
         ++channel) {
        const unsigned int channel_type =
            static_cast<unsigned int>(channel->second.channel_type);
        if (channel_type <= 0x10u &&
            ((1u << channel_type) & 0x1E700u) != 0) {
            ++pvp_channel_number;
        }
        if (static_cast<uint32_t>(channel->first) == channel_no) {
            return pvp_channel_number;
        }
    }
    return -1;
}

// IDA_EA: 0x88DCD38
// IDA_NAME: _ZNK16channel_script_t14getChannelInfoEhj
// DEMANGLED: channel_script_t::getChannelInfo(unsigned char,unsigned int)const
const channel_info_t* channel_script_t::getChannelInfo(
    uint8_t server_group,
    uint32_t channel_no) const {
    ServerChannelInfoMap::const_iterator server =
        channel_infos.find(static_cast<int>(server_group));
    if (server == channel_infos.end()) {
        return 0;
    }

    ChannelInfoMap::const_iterator channel =
        server->second.find(static_cast<int>(channel_no));
    if (channel == server->second.end()) {
        return 0;
    }
    return &channel->second;
}

// IDA_EA: 0x88DCE3C
// IDA_NAME: _ZNK16channel_script_t12getBonusRateEhjj
// DEMANGLED: channel_script_t::getBonusRate(unsigned char,unsigned int,unsigned int)const
long double channel_script_t::getBonusRate(uint8_t server_group,
                                           uint32_t channel_no,
                                           uint32_t dungeon_index) const {
    const channel_info_t* channel = getChannelInfo(server_group, channel_no);
    if (channel == 0 || channel->dungeon_indices == 0) {
        return 0.0L;
    }

    for (std::vector<int>::const_iterator it = channel->dungeon_indices->begin();
         it != channel->dungeon_indices->end();
         ++it) {
        if (static_cast<uint32_t>(*it) == dungeon_index) {
            return static_cast<long double>(channel->dungeon_bonus_rate) /
                   100.0L;
        }
    }
    return 0.0L;
}

// IDA_EA: 0x88DCEE2
// IDA_NAME: _ZNK16channel_script_t23getBonusRateEventServerEhj
// DEMANGLED: channel_script_t::getBonusRateEventServer(unsigned char,unsigned int)const
long double channel_script_t::getBonusRateEventServer(
    uint8_t server_group,
    uint32_t channel_no) const {
    if (getChannelInfo(server_group, channel_no) != 0) {
        return 0.02L;
    }
    return 0.0L;
}

// IDA_EA: 0x88DC008
// IDA_NAME: _Z19importChannelScriptP16channel_script_t
// DEMANGLED: importChannelScript(channel_script_t *)
int importChannelScript(channel_script_t* channel_script) {
    if (loadRDARScriptFile("", "Etc/channel_info.etc")) {
        return applyChannelScript(channel_script);
    }

    LogManager::logFormat(0,
                          "../RDARScriptChannel.cpp",
                          "importChannelScript",
                          0x38,
                          "importChannelScript - can't find channel_info.etc");
    return 0;
}

// IDA_EA: 0x88DC069
// IDA_NAME: _Z18applyChannelScriptP16channel_script_t
// DEMANGLED: applyChannelScript(channel_script_t *)
int applyChannelScript(channel_script_t* channel_script) {
    std::string type;
    while (ScanType(type, true) == 1) {
        if (type == "[dungeon]") {
            std::string classification;
            channel_dungeon_info_t dungeon_info;

            if (ScanStr(&classification) != 1) {
                return 1;
            }
            if (ScanStr(&dungeon_info.dungeon_group_name) != 1) {
                return 1;
            }

            bool success = false;
            bool allow_empty_dungeon_separator = false;
            while (1) {
                const int dungeon_index = ScanInt(&success);
                if (!success) {
                    if (allow_empty_dungeon_separator &&
                        IsEmptyFieldSeparatorToken()) {
                        allow_empty_dungeon_separator = false;
                        continue;
                    }
                    break;
                }
                allow_empty_dungeon_separator = true;
                LogManager::logFormat(0,
                                      "../RDARScriptChannel.cpp",
                                      "applyChannelScript",
                                      0x65,
                                      "[DUNGEON LIST] %s\t%d",
                                      classification.c_str(),
                                      dungeon_index);
                dungeon_info.dungeon_indices.push_back(dungeon_index);
            }

            channel_script->dungeon_infos[classification] = dungeon_info;
        } else if (type == "[server]") {
            std::map<int, channel_info_t> channel_info_map;
            bool success = false;
            const int server_group = ScanInt(&success);
            if (!success) {
                return 1;
            }

            // The legacy 0627 writer ends a channel row after the tenth grid
            // percentage.  The authoritative 0828 writer appends one empty
            // string token (``) to the same row.  ScanInt consumes a token even
            // when it cannot convert it, so treating that optional token as the
            // end of the whole [server] block drops every following channel.
            // Accept exactly one empty separator after a complete row while
            // preserving the original TYPE-token termination path.
            bool allow_empty_row_separator = false;
            while (1) {
                channel_info_t channel_info;
                const int channel_no = ScanInt(&success);
                if (!success) {
                    if (allow_empty_row_separator &&
                        IsEmptyFieldSeparatorToken()) {
                        allow_empty_row_separator = false;
                        continue;
                    }
                    break;
                }
                allow_empty_row_separator = false;

                if (channel_info_map.find(channel_no) !=
                    channel_info_map.end()) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptChannel.cpp",
                        "applyChannelScript",
                        0x85,
                        "importChannelScript - invalid channel number. %d "
                        "already exists",
                        channel_no);
                    return 0;
                }

                if (ScanStr(&channel_info.channel_name) != 1) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptChannel.cpp",
                        "applyChannelScript",
                        0x8C,
                        "importChannelScript - invalid channel name "
                        "(requires STR)");
                    return 0;
                }

                channel_info.channel_type = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptChannel.cpp",
                        "applyChannelScript",
                        0x9C,
                        "importChannelScript - invalid channel type "
                        "(requires INT)");
                    return 0;
                }

                if (ScanStr(&channel_info.dungeon_group_name) != 1) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptChannel.cpp",
                        "applyChannelScript",
                        0xA3,
                        "importChannelScript - invalid dungeon "
                        "classification (requires STR)");
                    return 0;
                }

                LogManager::logFormat(0,
                                      "../RDARScriptChannel.cpp",
                                      "applyChannelScript",
                                      0xAB,
                                      "[CHANNEL LIST] %d\t%s\t%d",
                                      channel_no,
                                      channel_info.dungeon_group_name.c_str(),
                                      channel_info.channel_type);

                if (channel_info.dungeon_group_name != "[none]") {
                    channel_script_t::DungeonInfoMap::iterator dungeon =
                        channel_script->dungeon_infos.find(
                            channel_info.dungeon_group_name);
                    if (dungeon == channel_script->dungeon_infos.end()) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptChannel.cpp",
                            "applyChannelScript",
                            0xB6,
                            "importChannelScript - invalid dungeon "
                            "classification. %s does not exist",
                            channel_info.dungeon_group_name.c_str());
                        return 0;
                    }
                    channel_info.dungeon_indices =
                        &dungeon->second.dungeon_indices;
                    channel_info.display_name =
                        dungeon->second.dungeon_group_name;
                }

                const int dungeon_bonus_rate = ScanInt(&success);
                channel_info.dungeon_bonus_rate =
                    static_cast<unsigned char>(dungeon_bonus_rate);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptChannel.cpp",
                        "applyChannelScript",
                        0xC3,
                        "importChannelScript - invalid dungeon bonus rate. "
                        "(required INT)");
                    return 0;
                }

                for (int index = 0; index <= 9; ++index) {
                    channel_info.level_grid_percent[index] =
                        ScanFloat(&success);
                    if (!success) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptChannel.cpp",
                            "applyChannelScript",
                            0xCC,
                            "importChannelScript - invalid level "
                            "GridPercent(%d). (required Float)",
                            index);
                        return 0;
                    }
                }

                channel_info_map[channel_no] = channel_info;
                allow_empty_row_separator = true;
            }

            channel_script->channel_infos.insert(
                std::make_pair(server_group, channel_info_map));
        } else {
            LogManager::logFormat(
                0,
                "../RDARScriptChannel.cpp",
                "applyChannelScript",
                0xEF,
                "importChannelScript - invalid type. %s is not acceptable",
                type.c_str());
            return 1;
        }
    }

    return 1;
}

// IDA_EA: 0x88DC81B
// IDA_NAME: _Z22traversalChannelScriptP16channel_script_t
// DEMANGLED: traversalChannelScript(channel_script_t *)
int traversalChannelScript(channel_script_t* channel_script) {
    char text[512];

    for (channel_script_t::DungeonInfoMap::iterator dungeon =
             channel_script->dungeon_infos.begin();
         dungeon != channel_script->dungeon_infos.end();
         ++dungeon) {
        std::snprintf(text,
                      sizeof(text),
                      "[DUNGEON] %s as %s : ",
                      dungeon->second.dungeon_group_name.c_str(),
                      dungeon->first.c_str());
        for (std::vector<int>::size_type index = 0;
             index < dungeon->second.dungeon_indices.size();
             ++index) {
            std::snprintf(text,
                          sizeof(text),
                          "%s%d ",
                          text,
                          dungeon->second.dungeon_indices[index]);
        }
        LogManager::logFormat(0,
                              "../RDARScriptChannel.cpp",
                              "traversalChannelScript",
                              0x107,
                              text);
    }

    for (channel_script_t::ServerChannelInfoMap::iterator server =
             channel_script->channel_infos.begin();
         server != channel_script->channel_infos.end();
         ++server) {
        LogManager::logFormat(0,
                              "../RDARScriptChannel.cpp",
                              "traversalChannelScript",
                              0x10C,
                              "[CHANNEL] Server %d",
                              server->first);

        for (channel_script_t::ChannelInfoMap::iterator channel =
                 server->second.begin();
             channel != server->second.end();
             ++channel) {
            std::snprintf(text,
                          sizeof(text),
                          "\t%d : %s type(%d) name(%s) tag(%s) ",
                          channel->first,
                          channel->second.channel_name.c_str(),
                          channel->second.channel_type,
                          channel->second.display_name.c_str(),
                          channel->second.dungeon_group_name.c_str());
            if (channel->second.dungeon_indices != 0) {
                for (std::vector<int>::size_type index = 0;
                     index < channel->second.dungeon_indices->size();
                     ++index) {
                    std::snprintf(
                        text,
                        sizeof(text),
                        "%s%d ",
                        text,
                        channel->second.dungeon_indices->at(index));
                }
            }
            LogManager::logFormat(0,
                                  "../RDARScriptChannel.cpp",
                                  "traversalChannelScript",
                                  0x11A,
                                  text);
        }
    }

    return 0;
}
