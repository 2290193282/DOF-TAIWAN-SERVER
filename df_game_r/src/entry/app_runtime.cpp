#include "entry/app_entry.h"

#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "common/manager/global_data.h"
#include "infra/log/format/clog.h"
#include "old_data/channel/arad_server_state.h"
#include "old_game/dungeon/instance/timer_dungeon_inout_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "common/manager/mt_rand.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/security/crypto/cipher.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <vector>


// IDA data refs in SelectKeyString @0x82A002A:
// g_stStackableScriptFileList @0x94FDC00,
// g_stEquipmentScriptFileList @0x94FAC00,
// g_stMonsterScriptFileList @0x94FF260,
// g_questScriptFileList @0x9500480,
// g_stPassiveObjectScriptFileList @0x94FF3A0.
STScriptFileList g_stStackableScriptFileList;
STScriptFileList g_stEquipmentScriptFileList;
STScriptFileList g_stMonsterScriptFileList;
STScriptFileList g_questScriptFileList;
STScriptFileList g_stPassiveObjectScriptFileList;

namespace {

const char kNoname[] = "???";
const int kCipherKeyGroupCount = 5;
const size_t kInitCipherStackBufferSize = 2048;

CMTRand& app_rand() {
    static CMTRand rand;
    return rand;
}

int get_rand_int(int limit) {
    if (limit <= 0) {
        return 0;
    }
    return static_cast<int>(app_rand().randInt(static_cast<unsigned int>(limit - 1)));
}

typedef std::map<int, std::string> CipherKeyStringMap;

CipherKeyStringMap& cipher_key_string_map(int group) {
    switch (group) {
        case 0:
            return g_stStackableScriptFileList;
        case 1:
            return g_stEquipmentScriptFileList;
        case 2:
            return g_stMonsterScriptFileList;
        case 3:
            return g_questScriptFileList;
        case 4:
            return g_stPassiveObjectScriptFileList;
    }
    return g_stStackableScriptFileList;
}

int cipher_key_string_step_limit(int group) {
    switch (group) {
        case 0:
            return 2000;
        case 1:
            return 5000;
        case 2:
            return 500;
        case 3:
        case 4:
            return 1000;
    }
    return 0;
}

void reset_cipher_key_records() {
    std::fill(dword_940BE84,
              dword_940BE84 + GlobalData::kSecuCipherKeyStringCount * 4,
              -1);
}

void release_cipher_keystring_manager() {
    for (int index = 0; index < GlobalData::kSecuCipherKeyStringCount; ++index) {
        delete GlobalData::s_secu_cipher_keystring_manager_[index];
        GlobalData::s_secu_cipher_keystring_manager_[index] = 0;
    }
}

void log_cipher_keystring_missing(const char* function_name) {
    if (CLog::instance() != 0) {
        CLog::instance()->logError("%s: old script-list key maps are not restored",
                                   function_name);
    } else {
        std::fprintf(stderr, "%s: old script-list key maps are not restored\n",
                     function_name);
    }
}

std::string& inter_message_name(unsigned int index) {
    static std::vector<std::string> names;
    if (names.empty()) {
        names.resize(0x354);
        for (unsigned int i = 0; i < names.size(); ++i) {
            std::ostringstream text;
            text << "ENUM_INTERNALPACKET_" << i;
            names[i] = text.str();
        }
        names[0] = "E_DISPATCH_GAME_PACKET";
    }
    return names[index];
}

class StringResource {
public:
    StringResource() : strings_() {
        strings_[0] = "";
    }

    bool get(int id, std::string* out) const {
        std::map<int, std::string>::const_iterator it = strings_.find(id);
        if (it == strings_.end()) {
            return false;
        }
        if (out != 0) {
            *out = it->second;
        }
        return true;
    }

private:
    std::map<int, std::string> strings_;
};

}  // namespace

const char* GetNotifyPacketName(int packet_id);
const char* GetCommandPacketName(int packet_id);

// IDA_EA: 0x82A002A
// IDA_NAME: _Z15SelectKeyStringPiS_
// DEMANGLED: SelectKeyString(int *,int *)
const char* SelectKeyString(int* key_group, int* key_index) {
    int selected_group = get_rand_int(5);
    CipherKeyStringMap& selected_map = cipher_key_string_map(selected_group);
    if (key_group != 0) {
        *key_group = selected_group;
    }
    if (key_index != 0) {
        *key_index = -1;
    }

    if (selected_map.empty()) {
        log_cipher_keystring_missing("SelectKeyString");
        return 0;
    }

    CipherKeyStringMap::const_iterator it = selected_map.begin();
    const CipherKeyStringMap::const_iterator end = selected_map.end();
    const int step_count =
        get_rand_int(cipher_key_string_step_limit(selected_group));
    int steps = 0;
    while (steps++ < step_count) {
        ++it;
        if (it == end) {
            --it;
            break;
        }
    }

    if (key_index != 0) {
        *key_index = it->first;
    }
    return it->second.c_str();
}

// IDA_EA: 0x82A025A
// IDA_NAME: _Z19InitCipherKeyStringv
// DEMANGLED: InitCipherKeyString(void)
int InitCipherKeyString() {
    int start_sec = 0;
    if (GlobalData::system_time() != 0) {
        GlobalData::system_time()->update();
        start_sec = GlobalData::system_time()->getCurSec();
    }

    release_cipher_keystring_manager();
    reset_cipher_key_records();

    static int needed_key_length = 0;
    unsigned char key_buffer[kInitCipherStackBufferSize];

    for (int index = 0; index < GlobalData::kSecuCipherKeyStringCount; ++index) {
        Cipher* cipher = new Cipher();
        GlobalData::s_secu_cipher_keystring_manager_[index] = cipher;
        if (needed_key_length == 0) {
            needed_key_length = cipher->GetTotalKeyLength();
        }
        if (needed_key_length <= 0 ||
            needed_key_length > static_cast<int>(sizeof(key_buffer))) {
            release_cipher_keystring_manager();
            reset_cipher_key_records();
            return 0;
        }

        const char* selected_key_string =
            SelectKeyString(&dword_940BE84[4 * index], &dword_940BE88[4 * index]);
        if (selected_key_string == 0 || selected_key_string[0] == '\0') {
            log_cipher_keystring_missing("InitCipherKeyString");
            release_cipher_keystring_manager();
            reset_cipher_key_records();
            return 0;
        }

        std::memset(key_buffer, 0, sizeof(key_buffer));
        const size_t source_length = std::strlen(selected_key_string);
        if (source_length == 0) {
            release_cipher_keystring_manager();
            reset_cipher_key_records();
            return 0;
        }
        for (int offset = 0; offset < needed_key_length;
             offset += static_cast<int>(source_length)) {
            const size_t remaining =
                static_cast<size_t>(needed_key_length - offset);
            const size_t copy_length = std::min(source_length, remaining);
            std::memcpy(key_buffer + offset, selected_key_string, copy_length);
        }

        if (!cipher->Initialize(key_buffer, needed_key_length)) {
            release_cipher_keystring_manager();
            reset_cipher_key_records();
            return 0;
        }
    }

    int elapsed = 0;
    if (GlobalData::system_time() != 0) {
        GlobalData::system_time()->update();
        elapsed = GlobalData::system_time()->getCurSec() - start_sec;
    }
    if (CLog::instance() != 0) {
        CLog::instance()->logNotice("[!] Init CipherKeyString TotalTime : %ds",
                                    elapsed);
    } else {
        std::printf("[!] Init CipherKeyString TotalTime : %ds\n", elapsed);
    }
    return 1;
}

// IDA_EA: 0x82A0459
// IDA_NAME: _Z20RegistServerSchedulev
// DEMANGLED: RegistServerSchedule(void)
int RegistServerSchedule() {
    TimerQueue* queue = G_TimerQueue();
    if (queue == 0) {
        return 0;
    }
    // The original function registers many subsystem timers. Keep the common
    // system timers visible here; individual timer classes are restored in
    // their own modules and can replace this boundary as they land.
    queue->InsertTimer(2, 0, 97, 60, 0, 0);
    queue->InsertTimer(2, 0, 153, 60, 0, 0);
    queue->InsertTimer(2, 0, 161, 3, 0, 0);
    // IDA 0x82A0459 invokes this immediately after the generic schedule
    // entries and only for non-PvP channels.  RegistNextTimer consumes the
    // imported [dungeon inout time] map and owns subsequent transitions.
    if (!G_GameWorld()->IsPVPChannel()) {
        Timer_DungeonInoutOpenTime::RegistNextTimer();
    }
    // IDA_EA: 0x82A0459.  The startup scheduler first expires persisted
    // states (which installs timer 172), then queues their DB load.
    GlobalData::s_serverStateManager_->checkContinus();
    GlobalData::s_serverStateManager_->loadServerState();

    return 1;
}

// IDA_EA: 0x82A2112
// IDA_NAME: _Z13GetPacketName16ENUM_PACKETCLASSt
// DEMANGLED: GetPacketName(ENUM_PACKETCLASS,unsigned short)
const char* GetPacketName(int packet_class, unsigned short packet_id) {
    if (packet_class == ENUM_PACKETCLASS_NOTIPACKET) {
        return GetNotifyPacketName(packet_id);
    }
    if (packet_class == ENUM_PACKETCLASS_CMDPACKET) {
        return GetCommandPacketName(packet_id);
    }
    return "Undefined Packet Class";
}

// IDA_EA: 0x82A2150
// IDA_NAME: _Z19GetNotifyPacketName15ENUM_NOTIPACKET
// DEMANGLED: GetNotifyPacketName(ENUM_NOTIPACKET)
const char* GetNotifyPacketName(int packet_id) {
    if (packet_id < 0 || packet_id >= getNotiPacketNameCount()) {
        return kNoname;
    }
    return g_szNotiPacketName[packet_id];
}

// IDA_EA: 0x82A2191
// IDA_NAME: _Z20GetCommandPacketName14ENUM_CMDPACKET
// DEMANGLED: GetCommandPacketName(ENUM_CMDPACKET)
const char* GetCommandPacketName(int packet_id) {
    if (packet_id < 0 || packet_id >= getCmdPacketNameCount()) {
        return kNoname;
    }
    return g_szCmdPacketName[packet_id];
}

// IDA_EA: 0x82A21D2
// IDA_NAME: _Z19GetInterMessageName19ENUM_INTERNALPACKET
// DEMANGLED: GetInterMessageName(ENUM_INTERNALPACKET)
const char* GetInterMessageName(unsigned int packet_id) {
    if (packet_id >= 0x354) {
        return kNoname;
    }
    return inter_message_name(packet_id).c_str();
}

namespace app {

// IDA_EA: 0x8AD5439
// IDA_NAME: _ZN3app14stringResourceEv
// DEMANGLED: app::stringResource(void)
StringResource* stringResource() {
    static StringResource resource;
    return &resource;
}

// IDA_EA: 0x8AD54CC
// IDA_NAME: _ZN3app6getStrEi
// DEMANGLED: app::getStr(int)
std::string getStr(int id) {
    std::string text;
    if (stringResource()->get(id, &text)) {
        return text;
    }
    std::ostringstream fallback;
    fallback << "getStr(" << id << ") == none";
    return fallback.str();
}

}  // namespace app
