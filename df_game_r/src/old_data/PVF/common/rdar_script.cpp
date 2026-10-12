#include "old_data/PVF/common/rdar_script.h"

#include "old_data/PVF/common/pvf_pack.h"
#include "old_data/PVF/common/script_file_list.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <algorithm>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include <istream>
#include <map>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif


std::map<std::string, int> g_mapKeyNameIndexTable;

extern "C" RDARScriptStringManager g_scriptStringManager_;
RDARScriptStringManager g_scriptStringManager_;

static char g_szScriptFileBaseDirectory[260] = {};
static std::vector<std::string> g_szScriptFilePriorityBaseDirectory;
static PackSet g_script_package;
static bool s_bUseScriptPack = false;
static bool g_isAllowScriptDuplication = false;
static void (*g_fpScriptLoadingErrorFunction)(const char*) = 0;

static int ss_strcpy(char* dest, size_t count, const char* src) {
    if (dest == 0 || count == 0) {
        return 0;
    }
    if (src == 0) {
        dest[0] = 0;
        return 0;
    }
    strncpy(dest, src, count - 1);
    dest[count - 1] = 0;
    return 1;
}

static int ss_strcat(char* dest, size_t count, const char* src) {
    if (dest == 0 || count == 0 || src == 0) {
        return 0;
    }
    const size_t used = strlen(dest);
    if (used >= count) {
        return 0;
    }
    strncat(dest, src, count - used - 1);
    dest[count - 1] = 0;
    return 1;
}

// IDA_EA: 0x8AA6551
// IDA_NAME: _ZL15parseStringLinePcPS_S0_
// DEMANGLED: parseStringLine(char *,char **,char **)
static __attribute__((noinline, used))
int parseStringLine(char* line, char** key, char** value) {
    if (key != 0) {
        *key = 0;
    }
    if (value != 0) {
        *value = 0;
    }
    if (line == 0) {
        return -1;
    }

    char* cursor = line;
    while (*cursor != 0) {
        if (*cursor == '\r' || *cursor == '\n') {
            return 0;
        }
        if (*cursor != ' ' && *cursor != '\t') {
            break;
        }
        ++cursor;
    }
    if (*cursor == 0 || (cursor[0] == '/' && cursor[1] == '/')) {
        return 0;
    }

    char* key_start = cursor;
    char* separator = strchr(cursor, '>');
    if (separator == 0) {
        return -1;
    }
    *separator = 0;
    char* value_start = separator + 1;
    if (value_start - key_start == 1) {
        return -1;
    }
    if (value_start - key_start - 1 > 40) {
        return -2;
    }

    for (cursor = value_start; *cursor != 0; ++cursor) {
        if (*cursor == '\r' || *cursor == '\n') {
            *cursor = 0;
            break;
        }
        if (*cursor == '\\' && cursor[1] == 'n') {
            *cursor = '\n';
            for (char* shift = cursor + 1; *shift != 0; ++shift) {
                *shift = shift[1];
            }
        }
    }

    if (key != 0) {
        *key = key_start;
    }
    if (value != 0) {
        *value = value_start;
    }
    return 0;
}

// IDA_EA: 0x8AA4E98
// IDA_NAME: _ZN23RDARScriptStringManagerC2Ev
// DEMANGLED: RDARScriptStringManager::RDARScriptStringManager(void)
RDARScriptStringManager::RDARScriptStringManager()
    : string_file_list_(),
      string_tables_(),
      last_found_string_(),
      last_found_wstring_() {}

// IDA_EA: 0x8AA4F04
// IDA_NAME: _ZN23RDARScriptStringManagerD2Ev
// DEMANGLED: RDARScriptStringManager::~RDARScriptStringManager()
RDARScriptStringManager::~RDARScriptStringManager() {
    release();
}

// IDA_EA: 0x8AA5014
// IDA_NAME: _ZN23RDARScriptStringManager7releaseEv
// DEMANGLED: RDARScriptStringManager::release(void)
int RDARScriptStringManager::release() {
    for (std::vector<StringDictionary*>::iterator it = string_tables_.begin();
         it != string_tables_.end();
         ++it) {
        delete *it;
    }
    string_tables_.clear();
    return 0;
}

// IDA_EA: 0x8AA50B4
// IDA_NAME: _ZN23RDARScriptStringManager17getStringFileListEv
// DEMANGLED: RDARScriptStringManager::getStringFileList(void)
STScriptFileList* RDARScriptStringManager::getStringFileList() {
    return &string_file_list_;
}

// IDA_EA: 0x8AA50BC
// IDA_NAME: _ZN23RDARScriptStringManager15writeStringFileEi
// DEMANGLED: RDARScriptStringManager::writeStringFile(int)
int RDARScriptStringManager::writeStringFile(int file_index) {
    const char* file_name = string_file_list_.GetFileName(file_index);
    char filename[512];
    snprintf(filename, sizeof(filename), "Script/%s",
             file_name != 0 ? file_name : "");

    FILE* stream = fopen(filename, "w");
    if (stream == 0) {
        return 0;
    }

    if (file_index >= 0 &&
        static_cast<size_t>(file_index) < string_tables_.size() &&
        string_tables_[file_index] != 0) {
        const StringDictionary& dictionary = *string_tables_[file_index];
        for (StringDictionary::const_iterator it = dictionary.begin();
             it != dictionary.end();
             ++it) {
            std::string value = it->second;
            std::string::size_type pos = 0;
            while ((pos = value.find('\n', pos)) != std::string::npos) {
                value.replace(pos, 1, "\\n");
                pos += 2;
            }
            fprintf(stream, "%s>%s\n", it->first.c_str(), value.c_str());
        }
    }

    fclose(stream);
    return 1;
}

// IDA_EA: 0x8AA529E
// IDA_NAME: _ZN23RDARScriptStringManager14loadStringFileEPSiPKciPSt6vectorI8ErrorLogSaIS4_EE
// DEMANGLED: RDARScriptStringManager::loadStringFile(std::istream *,char const*,int,std::vector<ErrorLog,std::allocator<ErrorLog>> *)
int RDARScriptStringManager::loadStringFile(std::istream* stream,
                                            const char* filename,
                                            int file_index,
                                            std::vector<ErrorLog>* errors) {
    (void)errors;
    if (stream == 0 || filename == 0 || stream->bad()) {
        return 0;
    }

    StringDictionary* dictionary = new StringDictionary();
    bool duplicate = false;
    char line[10536];
    while (stream->getline(line, sizeof(line))) {
        char* key = 0;
        char* value = 0;
        const int parsed = parseStringLine(line, &key, &value);
        if (parsed == -2 || parsed == -1) {
            delete dictionary;
            return 0;
        }
        if (key != 0 && value != 0) {
            std::pair<StringDictionary::iterator, bool> inserted =
                dictionary->insert(std::make_pair(
                    std::string(key), std::string(value)));
            if (!inserted.second) {
                duplicate = true;
            }
        }
    }
    if (stream->bad() || (stream->fail() && !stream->eof())) {
        delete dictionary;
        return 0;
    }

    if (duplicate) {
        delete dictionary;
        return 0;
    }

    while (static_cast<int>(string_tables_.size()) <= file_index) {
        string_tables_.push_back(0);
    }
    delete string_tables_[file_index];
    string_tables_[file_index] = dictionary;
    return 1;
}

// IDA_EA: 0x8AA57FE
// IDA_NAME: _ZNK23RDARScriptStringManager10findStringEiPKcPb
// DEMANGLED: RDARScriptStringManager::findString(int,char const*,bool *)const
const char* RDARScriptStringManager::findString(int file_index,
                                                const char* key,
                                                bool* found) const {
    if (found != 0) {
        *found = false;
    }
    if (key == 0) {
        return "RDARScriptStringManager::ERROR::input NULL string";
    }
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0) {
        return "RDARScriptStringManager::ERROR::requested fileIndex isn't in string table";
    }
    StringDictionary::const_iterator it =
        string_tables_[file_index]->find(key);
    if (it == string_tables_[file_index]->end()) {
        last_found_string_ = key;
        return last_found_string_.c_str();
    }
    if (found != 0) {
        *found = true;
    }
    last_found_string_ = it->second;
    return last_found_string_.c_str();
}

// IDA_EA: 0x8AA594C
// IDA_NAME: _ZNK23RDARScriptStringManager10findStringEiPKwPb
// DEMANGLED: RDARScriptStringManager::findString(int,wchar_t const*,bool *)const
const char* RDARScriptStringManager::findString(int file_index,
                                                const wchar_t* key,
                                                bool* found) const {
    if (found != 0) {
        *found = false;
    }
    if (key == 0) {
        return "RDARScriptStringManager::ERROR::input NULL string";
    }

    return findString(file_index, toTString(key), found);
}

// IDA_EA: 0x8AA59F0
// IDA_NAME: _ZNK23RDARScriptStringManager12getTagByNameEPSt6vectorISbIwSt11char_traitsIwESaIwEESaIS4_EES7_iPKw
// DEMANGLED: RDARScriptStringManager::getTagByName(std::vector<std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>,std::allocator<std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>>> *,std::vector<std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>,std::allocator<std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>>> *,int,wchar_t const*)const
int RDARScriptStringManager::getTagByName(std::vector<std::wstring>* keys,
                                          std::vector<std::wstring>* values,
                                          int file_index,
                                          const wchar_t* needle) const {
    if (keys == 0 || values == 0 || needle == 0) {
        return 0;
    }
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0) {
        return 0;
    }

    const StringDictionary& dictionary = *string_tables_[file_index];
    for (StringDictionary::const_iterator it = dictionary.begin();
         it != dictionary.end();
         ++it) {
        std::string compact_value = it->second;
        compact_value.erase(std::remove(compact_value.begin(),
                                        compact_value.end(),
                                        ' '),
                            compact_value.end());

        const wchar_t* wide_value = toUnicode(compact_value);

        if (wcsstr(wide_value, needle) == 0) {
            continue;
        }

        std::wstring wide_key(toUnicode(it->first));
        keys->push_back(wide_key);

        std::wstring original_value(toUnicode(it->second));
        values->push_back(original_value);
    }
    return 1;
}

// IDA_EA: 0x8AA5CBA
// IDA_NAME: _ZNK23RDARScriptStringManager11getKeyIndexEiPKw
// DEMANGLED: RDARScriptStringManager::getKeyIndex(int,wchar_t const*)const
int RDARScriptStringManager::getKeyIndex(int file_index,
                                         const wchar_t* text) const {
    if (text == 0) {
        return -1;
    }
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0) {
        return -1;
    }

    const StringDictionary& dictionary = *string_tables_[file_index];
    int index = 0;
    for (StringDictionary::const_iterator it = dictionary.begin();
         it != dictionary.end();
         ++it, ++index) {
        const wchar_t* wide_value = toUnicode(it->second);

        if (wcscmp(wide_value, text) == 0 &&
            it->first.c_str() != 0) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x8AA5E40
// IDA_NAME: _ZNK23RDARScriptStringManager12getTagByNameEiPKwS1_b
// DEMANGLED: RDARScriptStringManager::getTagByName(int,wchar_t const*,wchar_t const*,bool)const
const wchar_t* RDARScriptStringManager::getTagByName(
    int file_index,
    const wchar_t* text,
    const wchar_t* alternate_text,
    bool add_if_missing) const {
    (void)add_if_missing;
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0 ||
        text == 0) {
        last_found_wstring_ = L"`";
        if (text != 0) {
            last_found_wstring_ += text;
        }
        last_found_wstring_ += L" is not found, ";
        const char* file_name = string_file_list_.GetFileName(file_index);
        if (file_name != 0) {
            last_found_wstring_ += toUnicode(file_name);
        }
        last_found_wstring_ += L"`";
        return last_found_wstring_.c_str();
    }

    StringDictionary& dictionary = *string_tables_[file_index];
    int key_index = getKeyIndex(file_index, text);
    if (key_index == -1 && alternate_text != 0 && *alternate_text != 0) {
        key_index = getKeyIndex(file_index, alternate_text);
    }

    std::string selected_key;
    if (key_index != -1) {
        int index = 0;
        for (StringDictionary::const_iterator it = dictionary.begin();
             it != dictionary.end();
             ++it, ++index) {
            if (index == key_index) {
                selected_key = it->first;
                break;
            }
        }
    } else {
        std::string base_key;
        if (alternate_text != 0 && *alternate_text != 0) {
            const int alternate_index = getKeyIndex(file_index,
                                                     alternate_text);
            int index = 0;
            for (StringDictionary::const_iterator it = dictionary.begin();
                 it != dictionary.end();
                 ++it, ++index) {
                if (index == alternate_index) {
                    base_key = it->first;
                    break;
                }
            }
        }

        const char* narrow_text = toTString(text);

        for (int index = 0; index <= 9999; ++index) {
            char candidate[256];
            snprintf(candidate,
                     sizeof(candidate),
                     "%s_%d",
                     base_key.c_str(),
                     index);
            if (dictionary.find(candidate) == dictionary.end()) {
                dictionary[candidate] = narrow_text;
                selected_key = candidate;
                break;
            }
        }
    }

    if (selected_key.empty()) {
        last_found_wstring_ = L"`";
        last_found_wstring_ += text;
        last_found_wstring_ += L" is not found, ";
        const char* file_name = string_file_list_.GetFileName(file_index);
        if (file_name != 0) {
            last_found_wstring_ += toUnicode(file_name);
        }
        last_found_wstring_ += L"`";
        return last_found_wstring_.c_str();
    }

    char tag[320];
    snprintf(tag, sizeof(tag), "<%d::%s>", file_index, selected_key.c_str());
    last_found_wstring_ = toUnicode(tag);
    return last_found_wstring_.c_str();
}

// IDA_EA: 0x8AA5D98
// IDA_NAME: _ZN23RDARScriptStringManager13setTableValueEiiPKc
// DEMANGLED: RDARScriptStringManager::setTableValue(int,int,char const*)
int RDARScriptStringManager::setTableValue(int file_index,
                                           int tag_index,
                                           const char* value) {
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0 ||
        tag_index < 0 ||
        value == 0) {
        return 0;
    }

    StringDictionary& dictionary = *string_tables_[file_index];
    StringDictionary::iterator it = dictionary.begin();
    for (int index = 0; it != dictionary.end() && index < tag_index; ++index) {
        ++it;
    }
    if (it == dictionary.end()) {
        return 0;
    }

    it->second = value;
    return 1;
}

// IDA_EA: 0x8AA5DF2
// IDA_NAME: _ZN23RDARScriptStringManager16getTagSizeInFileEi
// DEMANGLED: RDARScriptStringManager::getTagSizeInFile(int)
int RDARScriptStringManager::getTagSizeInFile(int file_index) {
    if (file_index < 0 ||
        static_cast<size_t>(file_index) >= string_tables_.size() ||
        string_tables_[file_index] == 0) {
        return -1;
    }
    return static_cast<int>(string_tables_[file_index]->size());
}

// IDA_EA: 0x8AA60B0
// IDA_NAME: _ZN23RDARScriptStringManager18changeSpacilTagStrERSs
// DEMANGLED: RDARScriptStringManager::changeSpacilTagStr(std::string &)
int RDARScriptStringManager::changeSpacilTagStr(std::string& text) {
    if (text.empty()) {
        return 0;
    }

    const char* begin_tag = "<URL LINK>";
    const char* end_tag = "</URL LINK>";
    const std::string::size_type begin = text.find(begin_tag);
    if (begin == std::string::npos) {
        return 0;
    }

    const std::string::size_type end = text.rfind(end_tag);
    if (end == std::string::npos || begin == end) {
        return 0;
    }

    const std::string prefix = text.substr(0, begin);
    const std::string middle =
        text.substr(begin + strlen(begin_tag),
                    end - begin - strlen(begin_tag));
    const std::string suffix = text.substr(end + strlen(end_tag));
    if (prefix.empty() || middle.empty()) {
        return 0;
    }

    std::string changed(prefix);
    changed += static_cast<char>(0x1C);
    changed += middle;
    changed += static_cast<char>(0x1D);
    if (!suffix.empty()) {
        changed += suffix;
    }
    changed += "\n";
    if (changed.empty()) {
        return 0;
    }

    text = changed;
    return 1;
}

// IDA_EA: 0x8AA6448
// IDA_NAME: _ZN23RDARScriptStringManager17findChangedStringEiPKcPb
// DEMANGLED: RDARScriptStringManager::findChangedString(int,char const*,bool *)
const char* RDARScriptStringManager::findChangedString(int file_index,
                                                       const char* key,
                                                       bool* found) {
    static std::string ret(findString(file_index, key, found));
    changeSpacilTagStr(ret);
    return ret.c_str();
}

// IDA_EA: 0x88BB4B4
// IDA_NAME: _ZL15loadStringTablev
// DEMANGLED: loadStringTable(void)
static __attribute__((noinline, used)) int loadStringTable() {
    unsigned char* content = 0;
    uint32_t content_size = 0;
    if (g_script_package.get("stringtable.bin", &content, &content_size) !=
            PACK_OK ||
        content == 0) {
        delete[] content;
        return 0;
    }

    const bool imported =
        g_dnfLex.setStringTableFromBuffer(content,
                                          static_cast<int>(content_size));
    delete[] content;
    return imported ? 1 : 0;
}

// IDA_EA: 0x88C1558
// IDA_NAME: _ZL23importKeyNameIndexTablev
// DEMANGLED: importKeyNameIndexTable(void)
static __attribute__((noinline, used)) int importKeyNameIndexTable() {
    if (!loadRDARScriptFile("", "Skill/RDARKeyIndex.dat")) {
        return 0;
    }

    int index = 0;
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_DECIMAL) {
            index = g_dnfLex.getDecimal();
        } else if (token == DNF_TOK_CUSTOM_DATA) {
            g_mapKeyNameIndexTable[g_dnfLex.getCustomData()] = index;
        }
    }
    return 1;
}

// IDA_EA: 0x88BAF77
// IDA_NAME: _Z19LoadStreamFromPlainPKcS0_PN6DNFLex13stream_data_tE
// DEMANGLED: LoadStreamFromPlain(char const*,char const*,DNFLex::stream_data_t *)
bool LoadStreamFromPlain(const char* dir,
                         const char* filename,
                         DNFLex::stream_data_t* stream_data) {
    char full_path[260];
    char channel_path[260];
    memset(full_path, 0, sizeof(full_path));
    getScriptFileFullPath(full_path, sizeof(full_path), dir, filename);

    memset(channel_path, 0, sizeof(channel_path));
    getScriptFileFullPath(channel_path, sizeof(channel_path), dir,
                          "Etc/channel_info.etc");
    if (strcmp(full_path, channel_path) == 0) {
        memcpy(full_path, "./channel_info/channel_info.etc", 0x20u);
    }

    if (g_dnfLex.setFileStreamData(stream_data, full_path)) {
        return true;
    }
    if (g_fpScriptLoadingErrorFunction != 0) {
        g_fpScriptLoadingErrorFunction(full_path);
    }
    return false;
}

// IDA_EA: 0x88BB0FF
// IDA_NAME: _Z18LoadStreamFromPackPKcS0_PN6DNFLex13stream_data_tE
// DEMANGLED: LoadStreamFromPack(char const*,char const*,DNFLex::stream_data_t *)
bool LoadStreamFromPack(const char* dir,
                        const char* filename,
                        DNFLex::stream_data_t* stream_data) {
    if (dir == 0 || filename == 0 || stream_data == 0) {
        return false;
    }

    std::string name(dir);
    name += filename;
    stream_data->clear();
    if (name == "Etc/channel_info.etc") {
        if (g_dnfLex.setFileStreamData(stream_data,
                                       "./channel_info/channel_info.etc")) {
            return true;
        }
        if (g_fpScriptLoadingErrorFunction != 0) {
            g_fpScriptLoadingErrorFunction(name.c_str());
        }
        return false;
    }

    unsigned char* content = 0;
    uint32_t content_size = 0;
    bool found = g_script_package.get(name.c_str(), &content, &content_size) ==
                 PACK_OK;
    if (!found) {
        char server_token_path[1024];
        strncpy(server_token_path, name.c_str(), sizeof(server_token_path));
        server_token_path[sizeof(server_token_path) - 1] = 0;
        if (findFileWithServerToken(server_token_path)) {
            name = server_token_path;
            found = g_script_package.get(name.c_str(), &content,
                                         &content_size) == PACK_OK;
        }
    }

    if (found) {
        union {
            unsigned char* bytes;
            char* characters;
        } content_view;
        content_view.bytes = content;
        if (g_dnfLex.setStringStreamData(
                stream_data, content_view.characters, content_size)) {
            return true;
        }
        delete[] content;
    } else {
        delete[] content;
    }

    if (g_fpScriptLoadingErrorFunction != 0) {
        g_fpScriptLoadingErrorFunction(name.c_str());
    }
    return false;
}

// IDA_EA: 0x88BB367
// IDA_NAME: _Z10isDbcsLeadc
// DEMANGLED: isDbcsLead(char)
bool isDbcsLead(char ch) {
    return (static_cast<unsigned char>(ch) >> 7) != 0;
}

// IDA_EA: 0x88BB37C
// IDA_NAME: _Z11LexerOutputPKci
// DEMANGLED: LexerOutput(char const*,int)
void LexerOutput(const char* buffer, int size) {
    std::string text;
    if (buffer != 0 && size > 0) {
        text.assign(buffer, static_cast<size_t>(size));
    }
    LogManager::logFormat(0,
                          "../RDARScript.cpp",
                          "LexerOutput",
                          0x12F,
                          "%s - %s",
                          "LexerOutput",
                          toTString(text));
}

// IDA_EA: 0x88BB3C7
// IDA_NAME: _Z10LexerErrorPKc
// DEMANGLED: LexerError(char const*)
void LexerError(const char* message) {
    const std::string text(message != 0 ? message : "");
    LogManager::logFormat(0,
                          "../RDARScript.cpp",
                          "LexerError",
                          0x134,
                          "ERROR! %s",
                          toTString(text));
}

// IDA_EA: 0x88BB40A
// IDA_NAME: _Z24SetPriorityBaseDirectoryPKc
// DEMANGLED: SetPriorityBaseDirectory(char const*)
void SetPriorityBaseDirectory(const char* directory) {
    std::string value(directory != 0 ? directory : "");
    if (!value.empty()) {
        g_szScriptFilePriorityBaseDirectory.push_back(value);
    }
}

// IDA_EA: 0x88BB5D2
// IDA_NAME: _Z14InitRDARScriptPKcbS0_Pi
// DEMANGLED: InitRDARScript(char const*,bool,char const*,int *)
int InitRDARScript(const char* base_dir,
                   bool use_script_pack,
                   const char* script_pack,
                   int* pack_result) {
    DNFLexWrapper::TLoadStream load_stream =
        use_script_pack ? LoadStreamFromPack : LoadStreamFromPlain;
    g_dnfLex.SetHandler(isDbcsLead, LexerOutput, LexerError, load_stream);

    strncpy(g_szScriptFileBaseDirectory,
            base_dir != 0 ? base_dir : "",
            sizeof(g_szScriptFileBaseDirectory));
    g_szScriptFileBaseDirectory[sizeof(g_szScriptFileBaseDirectory) - 1] = 0;
    const size_t length = strlen(g_szScriptFileBaseDirectory);
    if (length <= 0) {
        return 0;
    }
    if (g_szScriptFileBaseDirectory[length - 1] != '\\' &&
        g_szScriptFileBaseDirectory[length - 1] != '/') {
        ss_strcat(g_szScriptFileBaseDirectory,
                  sizeof(g_szScriptFileBaseDirectory),
                  "\\");
    }

    std::string pack_name("Script.pvf");
    s_bUseScriptPack = use_script_pack;
    if (script_pack != 0 && *script_pack != 0) {
        s_bUseScriptPack = true;
        pack_name = script_pack;
    }

    if (s_bUseScriptPack) {
        g_script_package.close();
        const pack_ret_t result =
            g_script_package.open(pack_name.c_str(), 0, PACK_LOAD_ONLY_INDEX);
        if (pack_result != 0) {
            *pack_result = result;
        }
        if (result != PACK_OK) {
            return 0;
        }
        if (loadStringTable() != 1) {
            return 0;
        }
    }

    if (importKeyNameIndexTable() != 1) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x88BB7CF
// IDA_NAME: _Z13UseScriptPackv
// DEMANGLED: UseScriptPack(void)
int UseScriptPack() {
    return s_bUseScriptPack ? 1 : 0;
}

// IDA_EA: 0x88BB7DB
// IDA_NAME: _Z23findFileWithServerTokenPc
// DEMANGLED: findFileWithServerToken(char *)
int findFileWithServerToken(char* path) {
    if (path == 0) {
        return 0;
    }

    std::string original(path);
    std::string replacement;
    std::string::size_type last_of = original.find_last_of("/");
    if (last_of == std::string::npos) {
        last_of = original.find_last_of("\\");
    }
    if (last_of == std::string::npos) {
        return 0;
    }

    replacement.assign(original, 0, last_of + 1);
    replacement += "(R)";
    replacement.append(original, last_of + 1, std::string::npos);
    ss_strcpy(path, 0x104u, replacement.c_str());

    if (s_bUseScriptPack) {
        if (g_script_package.is_exist(path)) {
            return 1;
        }
    } else if (access(path, 0) == 0) {
        return 1;
    }

    ss_strcpy(path, 0x104u, original.c_str());
    return 0;
}

// IDA_EA: 0x88BBA4E
// IDA_NAME: _Z21getScriptFileFullPathPcjPKcS1_
// DEMANGLED: getScriptFileFullPath(char *,unsigned int,char const*,char const*)
char getScriptFileFullPath(char* dest,
                           unsigned int count,
                           const char* dir,
                           const char* filename) {
    if (dest == 0 || count == 0) {
        return 0;
    }

    if (!g_szScriptFilePriorityBaseDirectory.empty()) {
        for (int i = static_cast<int>(
                 g_szScriptFilePriorityBaseDirectory.size()) - 1;
             i >= 0;
             --i) {
            ss_strcpy(dest, count,
                      g_szScriptFilePriorityBaseDirectory[i].c_str());
            if (dir != 0) {
                ss_strcat(dest, count, dir);
            }
            if (filename != 0) {
                ss_strcat(dest, count, filename);
            }
            if (access(dest, 0) == 0) {
                return 1;
            }
            if (findFileWithServerToken(dest)) {
                return 1;
            }
        }
        *dest = 0;
    }

    ss_strcpy(dest, count, g_szScriptFileBaseDirectory);
    if (dir != 0) {
        ss_strcat(dest, count, dir);
    }
    if (filename != 0) {
        ss_strcat(dest, count, filename);
    }
    if (access(dest, 0) != 0) {
        return static_cast<char>(findFileWithServerToken(dest));
    }
    return 0;
}

// IDA_EA: 0x88BBBC5
// IDA_NAME: _Z27getScriptFileFullPathFormatPcjPKcz
// DEMANGLED: getScriptFileFullPathFormat(char *,unsigned int,char const*,...)
int getScriptFileFullPathFormat(char* dest,
                                unsigned int count,
                                const char* format,
                                ...) {
    char path[260];
    va_list args;
    va_start(args, format);
    vsnprintf(path, sizeof(path), format, args);
    va_end(args);
    ss_strcpy(dest, count, g_szScriptFileBaseDirectory);
    return ss_strcat(dest, count, path);
}

// IDA_EA: 0x88BBC80
// IDA_NAME: _Z16getDirectoryPathRSsPKcz
// DEMANGLED: getDirectoryPath(std::string &,char const*,...)
std::string* getDirectoryPath(std::string& output, const char* format, ...) {
    char path[512];
    va_list args;
    va_start(args, format);
    vsnprintf(path, sizeof(path), format, args);
    va_end(args);

    int index = static_cast<int>(strlen(path)) - 1;
    while (index >= 0 && path[index] != '\\' && path[index] != '/') {
        --index;
    }
    path[index + 1] = 0;
    output = path;
    return &output;
}

// IDA_EA: 0x88BC37B
// IDA_NAME: _Z7ScanIntPb
// DEMANGLED: ScanInt(bool *)
int ScanInt(bool* success) {
    int result = 0;
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_DECIMAL) {
        result = g_dnfLex.getDecimal();
        if (success != 0) {
            *success = true;
        }
    } else if (token == DNF_TOK_HEXADECIMAL) {
        result = g_dnfLex.getHexDecimal();
        if (success != 0) {
            *success = true;
        }
    } else if (token == DNF_TOK_FLOAT) {
        result = static_cast<int>(g_dnfLex.getFloat());
        if (success != 0) {
            *success = true;
        }
    } else if (success != 0) {
        *success = false;
    }
    return result;
}

// IDA_EA: 0x88BC424
// IDA_NAME: _Z7ScanIntPi
// DEMANGLED: ScanInt(int *)
int ScanInt(int* output) {
    if (output == 0) {
        return 0;
    }
    bool success = false;
    *output = ScanInt(&success);
    return success ? 1 : 0;
}

// IDA_EA: 0x88BC4E5
// IDA_NAME: _Z9ScanFloatPb
// DEMANGLED: ScanFloat(bool *)
float ScanFloat(bool* success) {
    float result = 0.0f;
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_DECIMAL) {
        result = static_cast<float>(g_dnfLex.getDecimal());
        if (success != 0) {
            *success = true;
        }
    } else if (token == DNF_TOK_HEXADECIMAL) {
        result = static_cast<float>(g_dnfLex.getHexDecimal());
        if (success != 0) {
            *success = true;
        }
    } else if (token == DNF_TOK_FLOAT) {
        result = g_dnfLex.getFloat();
        if (success != 0) {
            *success = true;
        }
    } else if (success != 0) {
        *success = false;
    }
    return result;
}

// IDA_EA: 0x88BC451
// IDA_NAME: _Z9ScanFloatiPb
// DEMANGLED: ScanFloat(int,bool *)
float ScanFloat(int precision, bool* success) {
    const float value = ScanFloat(success);
    const double scale = pow(10.0, static_cast<double>(precision) - 1.0);
    return static_cast<float>(
        static_cast<int>(static_cast<double>(value) * scale + 0.5) / scale);
}

// IDA_EA: 0x88BC58E
// IDA_NAME: _Z9ScanFloatPf
// DEMANGLED: ScanFloat(float *)
int ScanFloat(float* output) {
    if (output == 0) {
        return 0;
    }
    bool success = false;
    *output = ScanFloat(&success);
    return success ? 1 : 0;
}

// IDA_EA: 0x88BC5BB
// IDA_NAME: _Z12ScanFloatStrRfRSs
// DEMANGLED: ScanFloatStr(float &,std::string &)
int ScanFloatStr(float& value, std::string& text) {
    value = 0.0f;
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_DECIMAL) {
        value = static_cast<float>(g_dnfLex.getDecimal());
        return 0;
    }
    if (token == DNF_TOK_HEXADECIMAL) {
        value = static_cast<float>(g_dnfLex.getHexDecimal());
        return 0;
    }
    if (token == DNF_TOK_FLOAT) {
        value = g_dnfLex.getFloat();
        return 0;
    }
    if (token == DNF_TOK_STRING) {
        text = g_dnfLex.getString();
        return 1;
    }
    return -1;
}

// IDA_EA: 0x88BC68B
// IDA_NAME: _Z7ScanStrPci
// DEMANGLED: ScanStr(char *,int)
int ScanStr(char* dest, int count) {
    if (dest == 0 || count <= 0) {
        return 0;
    }

    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_STRING) {
        const char* source = g_dnfLex.getString();
        if (strlen(source) <= static_cast<size_t>(count)) {
            strncpy(dest, source, static_cast<size_t>(count));
            return 1;
        }
    } else if (token == DNF_TOK_PAIR_INT) {
        const int file_index = g_dnfLex.getPairInt();
        if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
            const char* source = g_scriptStringManager_.findString(
                file_index, g_dnfLex.getPairStr(), 0);
            if (strlen(source) <= static_cast<size_t>(count)) {
                strncpy(dest, source, static_cast<size_t>(count));
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x88BC7A2
// IDA_NAME: _Z7ScanStrPSs
// DEMANGLED: ScanStr(std::string *)
int ScanStr(std::string* output) {
    if (output == 0) {
        return 0;
    }
    const DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_STRING) {
        *output = g_dnfLex.getString();
        return 1;
    } else if (token == DNF_TOK_PAIR_INT) {
        const int file_index = g_dnfLex.getPairInt();
        if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
            *output = g_scriptStringManager_.findString(
                file_index, g_dnfLex.getPairStr(), 0);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x88BC85F
// IDA_NAME: _Z8ScanTypeRSsb
// DEMANGLED: ScanType(std::string &,bool)
int ScanType(std::string& output, bool log_error) {
    int result = 1;
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_TYPE) {
            output = g_dnfLex.getType();
        } else {
            result = 0;
        }
        if (token == DNF_TOK_TYPE || token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanType",
                0x555,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanType",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88BC9BF
// IDA_NAME: _Z13ScanTypeOrStrRbRSsb
// DEMANGLED: ScanTypeOrStr(bool &,std::string &,bool)
int ScanTypeOrStr(bool& is_type, std::string& output, bool log_error) {
    int result = 1;
    is_type = true;
    while (1) {
        DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_STRING) {
            output = g_dnfLex.getString();
            result = 1;
            is_type = false;
        } else if (token == DNF_TOK_TYPE) {
            output = g_dnfLex.getType();
        } else if (token == DNF_TOK_PAIR_INT) {
            const int file_index = g_dnfLex.getPairInt();
            if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
                token = DNF_TOK_STRING;
                output = g_scriptStringManager_.findString(
                    file_index, g_dnfLex.getPairStr(), 0);
                result = 1;
                is_type = false;
            }
        } else {
            result = 0;
            is_type = false;
        }

        if (token == DNF_TOK_STRING ||
            token == DNF_TOK_TYPE ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanTypeOrStr",
                0x58E,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanTypeOrStr",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88BCBEF
// IDA_NAME: _Z12ScanAbsOrRelRbRSsS_b
// DEMANGLED: ScanAbsOrRel(bool &,std::string &,bool &,bool)
long double ScanAbsOrRel(bool& is_absolute,
                         std::string& output,
                         bool& success,
                         bool log_error) {
    float result = 0.0f;
    is_absolute = true;
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_DECIMAL) {
            result = static_cast<float>(g_dnfLex.getDecimal());
            is_absolute = true;
            success = true;
        } else if (token == DNF_TOK_HEXADECIMAL) {
            result = static_cast<float>(g_dnfLex.getHexDecimal());
            is_absolute = true;
            success = true;
        } else if (token == DNF_TOK_FLOAT) {
            result = g_dnfLex.getFloat();
            is_absolute = true;
            success = true;
        } else if (token == DNF_TOK_TYPE) {
            output = g_dnfLex.getType();
            const std::string::size_type open = output.find("[");
            if (open != std::string::npos) {
                output.erase(0, open + 1);
            }
            const std::string::size_type close = output.find("]");
            if (close != std::string::npos) {
                output.erase(close, output.size());
            }
            result = static_cast<float>(atof(output.c_str()));
            is_absolute = false;
            success = true;
        } else {
            is_absolute = true;
            success = false;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL ||
            token == DNF_TOK_FLOAT ||
            token == DNF_TOK_TYPE ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanAbsOrRel",
                0x5D8,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanAbsOrRel",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88BCEAF
// IDA_NAME: _Z13ScanTypeOrIntRbRSsS_b
// DEMANGLED: ScanTypeOrInt(bool &,std::string &,bool &,bool)
int ScanTypeOrInt(bool& is_type,
                  std::string& output,
                  bool& success,
                  bool log_error) {
    int result = 0;
    is_type = true;
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_DECIMAL) {
            result = g_dnfLex.getDecimal();
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_HEXADECIMAL) {
            result = g_dnfLex.getHexDecimal();
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_FLOAT) {
            result = static_cast<int>(g_dnfLex.getFloat());
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_TYPE) {
            output = g_dnfLex.getType();
            is_type = true;
            success = true;
        } else {
            is_type = false;
            success = false;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL ||
            token == DNF_TOK_FLOAT ||
            token == DNF_TOK_TYPE ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanTypeOrInt",
                0x613,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanTypeOrInt",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88BD0D5
// IDA_NAME: _Z12ScanStrOrIntRbRSsS_b
// DEMANGLED: ScanStrOrInt(bool &,std::string &,bool &,bool)
int ScanStrOrInt(bool& is_string,
                 std::string& output,
                 bool& success,
                 bool log_error) {
    int result = 0;
    is_string = true;
    while (1) {
        DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_DECIMAL) {
            result = g_dnfLex.getDecimal();
            is_string = false;
            success = true;
        } else if (token == DNF_TOK_HEXADECIMAL) {
            result = g_dnfLex.getHexDecimal();
            is_string = false;
            success = true;
        } else if (token == DNF_TOK_FLOAT) {
            result = static_cast<int>(g_dnfLex.getFloat());
            is_string = false;
            success = true;
        } else if (token == DNF_TOK_STRING) {
            output = g_dnfLex.getString();
            is_string = true;
            success = true;
        } else if (token == DNF_TOK_PAIR_INT) {
            const int file_index = g_dnfLex.getPairInt();
            if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
                output = g_scriptStringManager_.findString(
                    file_index, g_dnfLex.getPairStr(), 0);
            }
            is_string = true;
            success = true;
        } else {
            is_string = false;
            success = false;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL ||
            token == DNF_TOK_FLOAT ||
            token == DNF_TOK_STRING ||
            token == DNF_TOK_PAIR_INT ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanStrOrInt",
                0x65A,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanStrOrInt",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88C2E01
// IDA_NAME: _Z15ScanTypeOrFloatRbRSsS_b
// DEMANGLED: ScanTypeOrFloat(bool &,std::string &,bool &,bool)
long double ScanTypeOrFloat(bool& is_type,
                            std::string& output,
                            bool& success,
                            bool log_error) {
    float result = 0.0f;
    is_type = true;
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_DECIMAL) {
            result = static_cast<float>(g_dnfLex.getDecimal());
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_HEXADECIMAL) {
            result = static_cast<float>(g_dnfLex.getHexDecimal());
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_FLOAT) {
            result = g_dnfLex.getFloat();
            is_type = false;
            success = true;
        } else if (token == DNF_TOK_TYPE) {
            output = g_dnfLex.getType();
            is_type = true;
            success = true;
        } else {
            is_type = false;
            success = false;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL ||
            token == DNF_TOK_FLOAT ||
            token == DNF_TOK_TYPE ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanTypeOrFloat",
                0xC93,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanTypeOrFloat",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88BDB56
// IDA_NAME: _Z11ScanAnyDataRiRSsS_b
// DEMANGLED: ScanAnyData(int &,std::string &,int &,bool)
int ScanAnyData(int& type, std::string& text, int& value, bool log_error) {
    type = 3;
    int result = 0;
    while (1) {
        DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_DECIMAL) {
            value = g_dnfLex.getDecimal();
            type = 1;
            result = 1;
        } else if (token == DNF_TOK_HEXADECIMAL) {
            value = g_dnfLex.getHexDecimal();
            type = 1;
            result = 1;
        } else if (token == DNF_TOK_FLOAT) {
            value = static_cast<int>(g_dnfLex.getFloat());
            type = 1;
            result = 1;
        } else if (token == DNF_TOK_STRING) {
            text = g_dnfLex.getString();
            type = 0;
            result = 1;
        } else if (token == DNF_TOK_TYPE) {
            text = g_dnfLex.getType();
            type = 2;
            result = 1;
        } else if (token == DNF_TOK_PAIR_INT) {
            const int file_index = g_dnfLex.getPairInt();
            if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
                token = DNF_TOK_STRING;
                text = g_scriptStringManager_.findString(
                    file_index, g_dnfLex.getPairStr(), 0);
                type = 0;
                result = 1;
            }
        } else {
            result = 0;
            type = 3;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL ||
            token == DNF_TOK_FLOAT ||
            token == DNF_TOK_STRING ||
            token == DNF_TOK_TYPE ||
            token == DNF_TOK_EOF) {
            break;
        }
        if (log_error) {
            const std::string token_data = g_dnfLex.getLastTokenDataAsString();
            const std::string stream_name = g_dnfLex.getStreamName();
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "ScanAnyData",
                0x746,
                "ERROR! %s - TYPE mismatch. %s:%d, token = '%s'",
                "ScanAnyData",
                stream_name.c_str(),
                g_dnfLex.getLastLineNo(),
                token_data.c_str());
        }
    }
    return result;
}

// IDA_EA: 0x88C0144
// IDA_NAME: _Z32getIndexOfStringFromStringVectorPKcRSt6vectorISsSaISsEE
// DEMANGLED: getIndexOfStringFromStringVector(char const*,std::vector<std::string,std::allocator<std::string>> &)
unsigned int getIndexOfStringFromStringVector(const char* text,
                                              std::vector<std::string>& table) {
    for (unsigned int i = 0; i < table.size(); ++i) {
        if (table[i] == text) {
            return i;
        }
    }
    return static_cast<unsigned int>(-1);
}

// IDA_EA: 0x88C019D
// IDA_NAME: _Z32getStringOfIndexFromStringVectoriRSt6vectorISsSaISsEE
// DEMANGLED: getStringOfIndexFromStringVector(int,std::vector<std::string,std::allocator<std::string>> &)
const char* getStringOfIndexFromStringVector(
    int index,
    std::vector<std::string>& table) {
    if (index < 0 || static_cast<unsigned int>(index) >= table.size()) {
        return 0;
    }
    return table[static_cast<unsigned int>(index)].c_str();
}

// IDA_EA: 0x88C01E2
// IDA_NAME: _ZL17isExistScriptFilePKcS0_
// DEMANGLED: isExistScriptFile(char const*,char const*)
static int __attribute__((used, noinline)) isExistScriptFile(
    const char* dir, const char* filename) {
    char full_path[520];
    getScriptFileFullPath(full_path, 0x200u, dir, filename);
    return access(full_path, 0) == 0;
}

// IDA_EA: 0x88C023C
// IDA_NAME: _ZL16isExistScriptPVFPKcS0_
// DEMANGLED: isExistScriptPVF(char const*,char const*)
static int __attribute__((used, noinline)) isExistScriptPVF(
    const char* dir, const char* filename) {
    char path[512];
    snprintf(path,
             sizeof(path),
             "%s%s",
             dir != 0 ? dir : "",
             filename != 0 ? filename : "");

    std::string name;
    std::string directory;
    for (int i = static_cast<int>(strlen(path)); i >= 0; --i) {
        if (i == 0) {
            name = path;
            directory = "";
            break;
        }
        if (path[i] == '/' || path[i] == '\\') {
            name = &path[i + 1];
            path[i + 1] = 0;
            directory = path;
            break;
        }
    }

    directory += name;
    return g_script_package.is_exist(directory.c_str()) ? 1 : 0;
}

// IDA_EA: 0x88C03D6
// IDA_NAME: _Z18OutputErrorMessagePKc
// DEMANGLED: OutputErrorMessage(char const*)
int OutputErrorMessage(const char* format) {
    LogManager::logFormat(0,
                          "../RDARScript.cpp",
                          "OutputErrorMessage",
                          0xA26,
                          format);
    return 0;
}

// IDA_EA: 0x88C0409
// IDA_NAME: _Z21getItemGroupNameTablev
// DEMANGLED: getItemGroupNameTable(void)
std::map<std::string, int>* getItemGroupNameTable() {
    static std::map<std::string, int> mapItemGroupTable;
    return &mapItemGroupTable;
}

// IDA_EA: 0x88C0B9A
// IDA_NAME: _Z14scanModuleTypev
// DEMANGLED: scanModuleType(void)
int scanModuleType() {
    std::string text;
    ScanStr(&text);
    return getModuleType(text.c_str());
}

// IDA_EA: 0x88C0BFC
// IDA_NAME: _Z13getModuleTypePKc
// DEMANGLED: getModuleType(char const*)
int getModuleType(const char* name) {
    if (name != 0 && strcmp(name, "[pvp]") == 0) {
        return 4;
    }
    if (name != 0 && strcmp(name, "[fair pvp]") == 0) {
        return 5;
    }
    if (name != 0 && strcmp(name, "[dungeon]") == 0) {
        return 3;
    }
    if (name != 0 && strcmp(name, "[main game]") == 0) {
        return 3;
    }
    if (name != 0 && strcmp(name, "[dead tower]") == 0) {
        return 10;
    }
    if (name != 0 && strcmp(name, "[war room]") == 0) {
        return 7;
    }
    if (name != 0 && strcmp(name, "[room list]") == 0) {
        return 1;
    }
    if (name != 0 && strcmp(name, "[assault]") == 0) {
        return 9;
    }
    if (name != 0 && strcmp(name, "[pvp type]") == 0) {
        return 18;
    }
    if (name != 0 && strcmp(name, "[dungeon type]") == 0) {
        return 19;
    }
    if (name != 0 && strcmp(name, "[all]") == 0) {
        return 20;
    }
    if (name != 0 && strcmp(name, "[blood system]") == 0) {
        return 11;
    }
    if (name != 0 && strcmp(name, "[tournament dungeon]") == 0) {
        return 16;
    }
    if (name != 0 && strcmp(name, "[despair tower]") == 0) {
        return 12;
    }
    return 17;
}

// IDA_EA: 0x88C1430
// IDA_NAME: _Z13scanWorldTypev
// DEMANGLED: scanWorldType(void)
int scanWorldType() {
    std::string text;
    ScanStr(&text);
    return getWorldType(text.c_str());
}

// IDA_EA: 0x88C1492
// IDA_NAME: _Z12getWorldTypePKc
// DEMANGLED: getWorldType(char const*)
int getWorldType(const char* name) {
    if (name != 0 && strcmp(name, "[common]") == 0) {
        return 0;
    }
    if (name != 0 && strcmp(name, "[fair]") == 0) {
        return 1;
    }
    return 2;
}

// IDA_EA: 0x88BDE5A
// IDA_NAME: _Z7ScanAllPfPSs
// DEMANGLED: ScanAll(float *,std::string *)
int ScanAll(float* value, std::string* text) {
    int int_value = 0;
    int token = ScanAll(&int_value, value, text);
    if (token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL) {
        token = DNF_TOK_FLOAT;
        if (value != 0) {
            *value = static_cast<float>(int_value);
        }
    }
    return token;
}

// IDA_EA: 0x88BDEA9
// IDA_NAME: _Z7ScanAllPiPfPSs
// DEMANGLED: ScanAll(int *,float *,std::string *)
DNFLEX_TOKEN ScanAll(int* int_value,
                     float* float_value,
                     std::string* text) {
    DNFLEX_TOKEN token = g_dnfLex.getToken();
    if (token == DNF_TOK_DECIMAL) {
        if (int_value != 0) {
            *int_value = g_dnfLex.getDecimal();
        }
        return token;
    }
    if (token == DNF_TOK_HEXADECIMAL) {
        if (int_value != 0) {
            *int_value = g_dnfLex.getHexDecimal();
        }
        return token;
    }
    if (token == DNF_TOK_FLOAT) {
        if (float_value != 0) {
            *float_value = g_dnfLex.getFloat();
        }
        return token;
    }
    if (token == DNF_TOK_STRING) {
        if (text != 0) {
            *text = g_dnfLex.getString();
        }
        return token;
    }
    if (token == DNF_TOK_TYPE || token == DNF_TOK_CUSTOM_DATA) {
        if (text != 0) {
            *text = g_dnfLex.getType();
        }
        return token;
    }
    if (token == DNF_TOK_PAIR_INT) {
        const int file_index = g_dnfLex.getPairInt();
        if (g_dnfLex.getToken() == DNF_TOK_PAIR_STR) {
            token = DNF_TOK_STRING;
            if (text != 0) {
                *text = g_scriptStringManager_.findString(
                    file_index, g_dnfLex.getPairStr(), 0);
            }
        }
    }
    return token;
}

// IDA_EA: 0x88BE12D
// IDA_NAME: _Z12getDrawLayerPKc
// DEMANGLED: getDrawLayer(char const*)
int getDrawLayer(const char* name) {
    if (name != 0 && strcmp(name, "[close]") == 0) {
        return 0;
    }
    if (name != 0 && strcmp(name, "[normal]") == 0) {
        return 1;
    }
    if (name != 0 && strcmp(name, "[bottom]") == 0) {
        return 2;
    }
    if (name != 0 && strcmp(name, "[closeback]") == 0) {
        return 3;
    }
    if (name != 0 && strcmp(name, "[middleback]") == 0) {
        return 4;
    }
    if (name != 0 && strcmp(name, "[distantback]") == 0) {
        return 5;
    }
    if (name != 0 && strcmp(name, "[cover]") == 0) {
        return 6;
    }
    return 7;
}

// IDA_EA: 0x88BE212
// IDA_NAME: _Z16getDrawLayerName14ENUM_DRAWLAYER
// DEMANGLED: getDrawLayerName(ENUM_DRAWLAYER)
char* getDrawLayerName(ENUM_DRAWLAYER layer) {
    static char buffer[16];
    if (layer == 0) {
        strncpy(buffer, "[close]", sizeof(buffer));
    } else if (layer == 2) {
        strncpy(buffer, "[bottom]", sizeof(buffer));
    } else if (layer == 3) {
        strncpy(buffer, "[closeback]", sizeof(buffer));
    } else if (layer == 4) {
        strncpy(buffer, "[middleback]", sizeof(buffer));
    } else if (layer == 5) {
        strncpy(buffer, "[distantback]", sizeof(buffer));
    } else if (layer == 6) {
        strncpy(buffer, "[cover]", sizeof(buffer));
    } else {
        strncpy(buffer, "[normal]", sizeof(buffer));
    }
    buffer[sizeof(buffer) - 1] = 0;
    return buffer;
}

// IDA_EA: 0x88BE343
// IDA_NAME: _Z17getFollowTypeName20ENUM_MAP_FOLLOW_TYPE
// DEMANGLED: getFollowTypeName(ENUM_MAP_FOLLOW_TYPE)
char* getFollowTypeName(ENUM_MAP_FOLLOW_TYPE follow_type) {
    static char buffer[48];
    switch (follow_type) {
        case 1:
            strncpy(buffer, "[path gate]", sizeof(buffer));
            break;
        case 2:
            strncpy(buffer, "[parent]", sizeof(buffer));
            break;
        case 3:
            strncpy(buffer, "[from parent]", sizeof(buffer));
            break;
        case 4:
            strncpy(buffer, "[keep position]", sizeof(buffer));
            break;
        default:
            return 0;
    }
    buffer[sizeof(buffer) - 1] = 0;
    return buffer;
}

// IDA_EA: 0x88BE3E7
// IDA_NAME: _Z18getPassageTypeName16ENUM_PASSAGETYPE
// DEMANGLED: getPassageTypeName(ENUM_PASSAGETYPE)
char* getPassageTypeName(ENUM_PASSAGETYPE passage_type) {
    static char buffer[48];
    if (passage_type == 0) {
        strncpy(buffer, "[pass all]", sizeof(buffer));
    } else if (passage_type == 1) {
        strncpy(buffer, "[pass only float type]", sizeof(buffer));
    } else if (passage_type == 2) {
        strncpy(buffer, "[do not pass]", sizeof(buffer));
    } else if (passage_type == 3) {
        strncpy(buffer, "[pass only red team]", sizeof(buffer));
    } else if (passage_type == 4) {
        strncpy(buffer, "[pass only blue team]", sizeof(buffer));
    } else if (passage_type == 7) {
        strncpy(buffer, "[pass only character]", sizeof(buffer));
    } else {
        return 0;
    }
    buffer[sizeof(buffer) - 1] = 0;
    return buffer;
}

// IDA_EA: 0x88BFD97
// IDA_NAME: _Z13getItemRarityPKc
// DEMANGLED: getItemRarity(char const*)
int getItemRarity(const char* name) {
    if (name != 0 && strcmp(name, "common") == 0) {
        return 0;
    }
    if (name != 0 && strcmp(name, "uncommon") == 0) {
        return 1;
    }
    if (name != 0 && strcmp(name, "rare") == 0) {
        return 2;
    }
    if (name != 0 && strcmp(name, "unique") == 0) {
        return 3;
    }
    if (name != 0 && strcmp(name, "epic") == 0) {
        return 4;
    }
    if (name != 0 && strcmp(name, "chronicle") == 0) {
        return 5;
    }
    return 6;
}

// IDA_EA: 0x88BFE5B
// IDA_NAME: _Z10getElementPKc
// DEMANGLED: getElement(char const*)
int getElement(const char* name) {
    if (name != 0 && strcmp(name, "[fire element]") == 0) {
        return 0;
    }
    if (name != 0 && strcmp(name, "[water element]") == 0) {
        return 1;
    }
    if (name != 0 && strcmp(name, "[dark element]") == 0) {
        return 2;
    }
    if (name != 0 && strcmp(name, "[light element]") == 0) {
        return 3;
    }
    return 4;
}

// IDA_EA: 0x88BFEE0
// IDA_NAME: _Z14getElementName12ENUM_ELEMENT
// DEMANGLED: getElementName(ENUM_ELEMENT)
char* getElementName(ENUM_ELEMENT element) {
    static char buffer[16];
    if (element == 0) {
        strncpy(buffer, "[fire element]", sizeof(buffer));
    } else if (element == 1) {
        memcpy(buffer, "[water element]", sizeof(buffer));
    } else if (element == 2) {
        strncpy(buffer, "[dark element]", sizeof(buffer));
    } else if (element == 3) {
        memcpy(buffer, "[light element]", sizeof(buffer));
    } else {
        strncpy(buffer, "[no element]", sizeof(buffer));
    }
    buffer[sizeof(buffer) - 1] = 0;
    return buffer;
}

// IDA_EA: 0x88C2D7C
// IDA_NAME: _Z7getTeamPKc
// DEMANGLED: getTeam(char const*)
int getTeam(const char* name) {
    if (name != 0 && strcasecmp(name, "character") == 0) {
        return 0;
    }
    if (name != 0 && strcasecmp(name, "neutral") == 0) {
        return 200;
    }
    if (name != 0 && strcasecmp(name, "monster") == 0) {
        return 100;
    }
    if (name != 0 && strcasecmp(name, "all") == 0) {
        return 2;
    }
    return 100;
}

// IDA_EA: 0x88C227B
// IDA_NAME: _Z18getCalculationTypePKc
// DEMANGLED: getCalculationType(char const*)
int getCalculationType(const char* text) {
    std::string value(text != 0 ? text : "");
    if (value == "+") {
        return 0;
    }
    if (value == "%") {
        return 1;
    }
    return 2;
}

// IDA_EA: 0x88C2341
// IDA_NAME: _Z19ConvertStringToTimeSsRl
// DEMANGLED: ConvertStringToTime(std::string,long &)
int ConvertStringToTime(std::string text, long& value) {
    if (text.size() != 19) {
        return 0;
    }

    struct tm time_value;
    memset(&time_value, 0, sizeof(time_value));
    time_value.tm_year = atoi(text.substr(0, 4).c_str()) - 1900;
    time_value.tm_mon = atoi(text.substr(5, 2).c_str()) - 1;
    time_value.tm_mday = atoi(text.substr(8, 2).c_str());
    time_value.tm_hour = atoi(text.substr(11, 2).c_str());
    time_value.tm_min = atoi(text.substr(14, 2).c_str());
    time_value.tm_sec = atoi(text.substr(17, 2).c_str());
    value = static_cast<long>(mktime(&time_value));
    return 1;
}

// IDA_EA: 0x88C25DF
// IDA_NAME: _Z24ConvertHHMMSSStringToIntSsRhS_S_
// DEMANGLED: ConvertHHMMSSStringToInt(std::string,unsigned char &,unsigned char &,unsigned char &)
int ConvertHHMMSSStringToInt(std::string text,
                             unsigned char& hour,
                             unsigned char& minute,
                             unsigned char& second) {
    if (text.size() != 8) {
        return 0;
    }
    hour = static_cast<unsigned char>(atoi(text.substr(0, 2).c_str()));
    minute = static_cast<unsigned char>(atoi(text.substr(3, 2).c_str()));
    second = static_cast<unsigned char>(atoi(text.substr(6, 2).c_str()));
    return 1;
}

// IDA_EA: 0x88C2748
// IDA_NAME: _Z22ConvertHHMMStringToIntSsRhS_
// DEMANGLED: ConvertHHMMStringToInt(std::string,unsigned char &,unsigned char &)
int ConvertHHMMStringToInt(std::string text,
                           unsigned char& hour,
                           unsigned char& minute) {
    if (text.size() != 5) {
        return 0;
    }
    hour = static_cast<unsigned char>(atoi(text.substr(0, 2).c_str()));
    minute = static_cast<unsigned char>(atoi(text.substr(3, 2).c_str()));
    return 1;
}

// IDA_EA: 0x88C284A
// IDA_NAME: _Z25ConvertWeekdayStringToIntSsRj
// DEMANGLED: ConvertWeekdayStringToInt(std::string,unsigned int &)
int ConvertWeekdayStringToInt(std::string text, unsigned int& weekday) {
    if (text == "SUN") {
        weekday = 0;
    } else if (text == "MON") {
        weekday = 1;
    } else if (text == "TUE") {
        weekday = 2;
    } else if (text == "WED") {
        weekday = 3;
    } else if (text == "THU") {
        weekday = 4;
    } else if (text == "FRI") {
        weekday = 5;
    } else if (text == "SAT") {
        weekday = 6;
    } else if (text == "ALL") {
        weekday = 7;
    } else {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x88C9990
// IDA_NAME: _Z12makeFullPathSsRSs
// DEMANGLED: makeFullPath(std::string,std::string &)
int makeFullPath(std::string prefix, std::string& path) {
    if (path.find('/') != std::string::npos ||
        path.find('\\') != std::string::npos) {
        return 0;
    }
    path = prefix + path;
    return 1;
}

// IDA_EA: 0x88C9A4C
// IDA_NAME: _Z9isKeyFilePKc
// DEMANGLED: isKeyFile(char const*)
int isKeyFile(const char* filename) {
    if (filename == 0) {
        return 0;
    }
    const size_t length = strlen(filename);
    if (length < 4) {
        return 0;
    }
    return strcmp(&filename[length - 4], ".key") == 0;
}

// IDA_EA: 0x88C9A98
// IDA_NAME: _Z11getFileNameRKSs
// DEMANGLED: getFileName(std::string const&)
std::string getFileName(const std::string& path) {
    std::string name(path);
    std::string::size_type slash = name.find_last_of("/\\");
    if (slash != std::string::npos) {
        name.erase(0, slash + 1);
    }

    std::string::size_type dot = name.find_last_of('.');
    if (dot != std::string::npos) {
        name.erase(dot);
    }
    return name;
}

// IDA_EA: 0x88BBD77
// IDA_NAME: _Z22loadRDARScriptListFileP16STScriptFileListPKcS2_PSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: loadRDARScriptListFile(STScriptFileList *,char const*,char const*,std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> *)
int loadRDARScriptListFile(STScriptFileList* list,
                           const char* dir,
                           const char* filename,
                           std::map<int, int>* index_table) {
    (void)index_table;
    if (!loadRDARScriptFile(dir, filename)) {
        return 0;
    }
    STScriptFileList parsed;

    std::string file_name;
    while (1) {
        bool got_index = false;
        const int index = ScanInt(&got_index);
        if (!got_index) {
            if (g_dnfLex.getLastToken() == DNF_TOK_EOF) {
                break;
            }
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "loadRDARScriptListFile",
                0x31D,
                "SCRIPT FILE LIST LOAD FAIL! Malformed index "
                "[%s%s] line=%d token='%s'",
                dir != 0 ? dir : "",
                filename != 0 ? filename : "",
                g_dnfLex.getLastLineNo(),
                g_dnfLex.getLastTokenDataAsString().c_str());
            return 0;
        }
        if (ScanStr(&file_name) != 1) {
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "loadRDARScriptListFile",
                0x31D,
                "SCRIPT FILE LIST LOAD FAIL! Missing filename "
                "[%s%s][%d] line=%d token='%s'",
                dir != 0 ? dir : "",
                filename != 0 ? filename : "",
                index,
                g_dnfLex.getLastLineNo(),
                g_dnfLex.getLastTokenDataAsString().c_str());
            return 0;
        }

        STScriptFileList::iterator existing = parsed.find(index);
        const bool not_found = existing == parsed.end();
        if (!not_found && !g_isAllowScriptDuplication) {
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "loadRDARScriptListFile",
                0x323,
                "SCRIPT FILE LIST LOAD FAIL! Index Duplicated [%s%s][%d]",
                dir,
                filename,
                index);
            return 0;
        }

        const int sign = not_found ? 1 : -1;
        parsed.insert(std::make_pair(sign * index, file_name));
    }

    list->swap(parsed);
    return 1;
}

// IDA_EA: 0x88BBF52
// IDA_NAME: _Z22saveRDARScriptListFileP16STScriptFileListPKcS2_PSt3mapIiiSt4lessIiESaISt4pairIKiiEEEPSt8multimapIiP7CommentS5_SaIS6_IS7_SE_EEE
// DEMANGLED: saveRDARScriptListFile(STScriptFileList *,char const*,char const*,std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> *,std::multimap<int,Comment *,std::less<int>,std::allocator<std::pair<int const,Comment *>>> *)
int saveRDARScriptListFile(STScriptFileList* list,
                           const char* dir,
                           const char* filename,
                           std::map<int, int>* index_table,
                           std::multimap<int, Comment*>* comments) {
    (void)dir;
    (void)index_table;
    (void)comments;
    if (filename == 0) {
        return 0;
    }

    FILE* stream = fopen(filename, "wt");
    if (stream == 0) {
        return 0;
    }

    for (STScriptFileList::iterator it = list->begin();
         it != list->end();
         ++it) {
        fprintf(stream, "%d\t", it->first);
        fprintf(stream, "`%s`", it->second.c_str());
    }
    fclose(stream);
    return 1;
}

// IDA_EA: 0x88BC068
// IDA_NAME: _Z16loadRDARNameFileP10STNameListPKcS2_
// DEMANGLED: loadRDARNameFile(STNameList *,char const*,char const*)
int loadRDARNameFile(STNameList* list,
                     const char* dir,
                     const char* filename) {
    if (!loadRDARScriptFile(dir, filename)) {
        return 0;
    }
    list->Clear();

    std::string name;
    while (ScanStr(&name) == 1) {
        bool got_id = false;
        const int id = ScanInt(&got_id);
        if (!got_id) {
            break;
        }

        const std::pair<STNameList::iterator, bool> inserted =
            list->insert(std::make_pair(name, id));
        if (!inserted.second) {
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "loadRDARNameFile",
                0x3E8,
                "ITEM NAME FILE LIST LOAD FAIL! [%s%s][%s]",
                dir,
                filename,
                name.c_str());
        }
    }

    return list->begin() == list->end() ? 0 : 1;
}

// IDA_EA: 0x88BC1EE
// IDA_NAME: _Z21addRDARScriptListFileP16STScriptFileListPKcS2_
// DEMANGLED: addRDARScriptListFile(STScriptFileList *,char const*,char const*)
int addRDARScriptListFile(STScriptFileList* list,
                          const char* dir,
                          const char* filename) {
    if (!loadRDARScriptFile(dir, filename)) {
        return 0;
    }

    STScriptFileList staged(*list);
    std::string file_name;
    while (1) {
        bool got_index = false;
        const int index = ScanInt(&got_index);
        if (!got_index) {
            if (g_dnfLex.getLastToken() == DNF_TOK_EOF) {
                break;
            }
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "addRDARScriptListFile",
                0x40F,
                "SCRIPT FILE LIST ADD FAIL! Malformed index "
                "[%s%s] line=%d token='%s'",
                dir != 0 ? dir : "",
                filename != 0 ? filename : "",
                g_dnfLex.getLastLineNo(),
                g_dnfLex.getLastTokenDataAsString().c_str());
            return 0;
        }
        if (ScanStr(&file_name) != 1) {
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "addRDARScriptListFile",
                0x40F,
                "SCRIPT FILE LIST ADD FAIL! Missing filename "
                "[%s%s][%d] line=%d token='%s'",
                dir != 0 ? dir : "",
                filename != 0 ? filename : "",
                index,
                g_dnfLex.getLastLineNo(),
                g_dnfLex.getLastTokenDataAsString().c_str());
            return 0;
        }

        STScriptFileList::iterator existing = staged.find(index);
        if (existing == staged.end()) {
            staged.insert(std::make_pair(index, file_name));
        } else {
            LogManager::logFormat(
                0,
                "../RDARScript.cpp",
                "addRDARScriptListFile",
                0x418,
                "SCRIPT FILE LIST ADD FAIL! - DUPLICATED DATA [%s%s][%d]",
                dir,
                filename,
                index);
        }
    }

    list->swap(staged);
    return 1;
}

// IDA_EA: 0x88BFF9C
// IDA_NAME: _Z27InitRDARScriptStringManagerPKcS0_PSt6vectorI8ErrorLogSaIS2_EE
// DEMANGLED: InitRDARScriptStringManager(char const*,char const*,std::vector<ErrorLog,std::allocator<ErrorLog>> *)
int InitRDARScriptStringManager(const char* filename,
                                const char* dir,
                                std::vector<ErrorLog>* errors) {
    if (dir == 0) {
        dir = "";
    }
    if (filename == 0) {
        return 0;
    }

    STScriptFileList* string_file_list =
        g_scriptStringManager_.getStringFileList();
    g_scriptStringManager_.release();
    if (loadRDARScriptListFile(string_file_list, "", filename) != 1) {
        return 0;
    }

    DNFLexWrapper::TLoadStream load_stream =
        s_bUseScriptPack ? LoadStreamFromPack : LoadStreamFromPlain;
    bool success = true;
    const int count = static_cast<int>(string_file_list->size());
    for (int index = 0; index < count; ++index) {
        const char* file_name = string_file_list->GetFileName(index);
        DNFLex::stream_data_t stream_data;
        stream_data.clear();
        if (file_name != 0 &&
            load_stream(dir, file_name, &stream_data) &&
            stream_data.stream != 0) {
            success =
                (success &&
                 g_scriptStringManager_.loadStringFile(stream_data.stream,
                                                      file_name,
                                                      index,
                                                      errors) == 1);
            stream_data.destroy();
        } else {
            success = false;
            stream_data.destroy();
        }
    }
    return success ? 1 : 0;
}
