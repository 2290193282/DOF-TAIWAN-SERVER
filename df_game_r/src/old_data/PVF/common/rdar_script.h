#ifndef OLDSRV_OLD_DATA_PVF_COMMON_RDAR_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_RDAR_SCRIPT_H_

#include <stdarg.h>

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/common/script_file_list.h"
#include "old_data/PVF/common/dnf_lex.h"


class STNameList;
class Comment;
struct ErrorLog;

// These are real named IDA parameter types.  Fixed underlying types allow the
// shared RDAR declarations to remain independent of the owning script layouts.
enum ENUM_DRAWLAYER : int;
enum ENUM_MAP_FOLLOW_TYPE : int;
enum ENUM_PASSAGETYPE : int;
enum ENUM_ELEMENT : int;

// IDA unit: 12_pvf_datamanager_scripts/RDARScriptStringManager.
class RDARScriptStringManager {
public:
    RDARScriptStringManager();
    ~RDARScriptStringManager();

    int release();
    STScriptFileList* getStringFileList();
    int writeStringFile(int file_index);
    int loadStringFile(std::istream* stream,
                       const char* filename,
                       int file_index,
                       std::vector<ErrorLog>* errors);
    const char* findString(int file_index,
                           const char* key,
                           bool* found) const;
    const char* findString(int file_index,
                           const wchar_t* key,
                           bool* found) const;
    int getTagByName(std::vector<std::wstring>* keys,
                     std::vector<std::wstring>* values,
                     int file_index,
                     const wchar_t* needle) const;
    int getKeyIndex(int file_index, const wchar_t* text) const;
    const wchar_t* getTagByName(int file_index,
                                const wchar_t* text,
                                const wchar_t* alternate_text,
                                bool add_if_missing) const;
    int setTableValue(int file_index, int tag_index, const char* value);
    int getTagSizeInFile(int file_index);
    int changeSpacilTagStr(std::string& text);
    const char* findChangedString(int file_index,
                                  const char* key,
                                  bool* found);

private:
    typedef std::map<std::string, std::string> StringDictionary;

    STScriptFileList string_file_list_;
    std::vector<StringDictionary*> string_tables_;
    mutable std::string last_found_string_;
    mutable std::wstring last_found_wstring_;

    RDARScriptStringManager(const RDARScriptStringManager&);
    RDARScriptStringManager& operator=(const RDARScriptStringManager&);
};

// IDA data: g_scriptStringManager_ @0x949B140.
extern "C" RDARScriptStringManager g_scriptStringManager_;

// IDA data: g_mapKeyNameIndexTable, populated by importKeyNameIndexTable
// @0x88C1558 and consumed by importCommand @0x88C167A.
extern std::map<std::string, int> g_mapKeyNameIndexTable;

// IDA_EA: RDAR stream/bootstrap functions @0x88BAF77..0x88BBC80.
bool LoadStreamFromPlain(const char* dir,
                         const char* filename,
                         DNFLex::stream_data_t* stream_data);
bool LoadStreamFromPack(const char* dir,
                        const char* filename,
                        DNFLex::stream_data_t* stream_data);
bool isDbcsLead(char ch);
void LexerOutput(const char* buffer, int size);
void LexerError(const char* message);
void SetPriorityBaseDirectory(const char* directory);
int InitRDARScript(const char* base_dir,
                   bool use_script_pack,
                   const char* script_pack,
                   int* pack_result);
int UseScriptPack();
int findFileWithServerToken(char* path);
char getScriptFileFullPath(char* dest,
                           unsigned int count,
                           const char* dir,
                           const char* filename);
int getScriptFileFullPathFormat(char* dest,
                                unsigned int count,
                                const char* format,
                                ...);
std::string* getDirectoryPath(std::string& output, const char* format, ...);
// IDA_EA: RDAR scanner functions @0x88BC37B..0x88BDEA9.
int ScanInt(bool* success);
int ScanInt(int* output);
float ScanFloat(int precision, bool* success);
float ScanFloat(bool* success);
int ScanFloat(float* output);
int ScanFloatStr(float& value, std::string& text);
int ScanStr(char* dest, int count);
int ScanStr(std::string* output);
int ScanType(std::string& output, bool log_error);
int ScanTypeOrStr(bool& is_type, std::string& output, bool log_error);
long double ScanAbsOrRel(bool& is_absolute,
                         std::string& output,
                         bool& success,
                         bool log_error);
int ScanTypeOrInt(bool& is_type,
                  std::string& output,
                  bool& success,
                  bool log_error);
int ScanStrOrInt(bool& is_string,
                 std::string& output,
                 bool& success,
                 bool log_error);
long double ScanTypeOrFloat(bool& is_type,
                            std::string& output,
                            bool& success,
                            bool log_error);
int ScanAnyData(int& type, std::string& text, int& value, bool log_error);
int ScanAll(float* value, std::string* text);
DNFLEX_TOKEN ScanAll(int* int_value, float* float_value, std::string* text);
// IDA_EA: RDAR utility functions @0x88C0144..0x88C9A98.
unsigned int getIndexOfStringFromStringVector(const char* text,
                                              std::vector<std::string>& table);
const char* getStringOfIndexFromStringVector(
    int index,
    std::vector<std::string>& table);
int OutputErrorMessage(const char* format);
std::map<std::string, int>* getItemGroupNameTable();
int getDrawLayer(const char* name);
char* getDrawLayerName(ENUM_DRAWLAYER layer);
char* getFollowTypeName(ENUM_MAP_FOLLOW_TYPE follow_type);
char* getPassageTypeName(ENUM_PASSAGETYPE passage_type);
int getItemRarity(const char* name);
int getElement(const char* name);
char* getElementName(ENUM_ELEMENT element);
int getTeam(const char* name);
int scanModuleType();
int getModuleType(const char* name);
int scanWorldType();
int getWorldType(const char* name);
int getCalculationType(const char* text);
int ConvertStringToTime(std::string text, long& value);
int ConvertHHMMSSStringToInt(std::string text,
                             unsigned char& hour,
                             unsigned char& minute,
                             unsigned char& second);
int ConvertHHMMStringToInt(std::string text,
                           unsigned char& hour,
                           unsigned char& minute);
int ConvertWeekdayStringToInt(std::string text, unsigned int& weekday);
int makeFullPath(std::string prefix, std::string& path);
int isKeyFile(const char* filename);
std::string getFileName(const std::string& path);
int loadRDARScriptListFile(STScriptFileList* list,
                           const char* dir,
                           const char* filename,
                           std::map<int, int>* index_table = 0);
// IDA_EA: saveRDARScriptListFile @0x88BBF52; loadRDARNameFile @0x88BC068; addRDARScriptListFile @0x88BC1EE.
int saveRDARScriptListFile(
    STScriptFileList* list,
    const char* dir,
    const char* filename,
    std::map<int, int>* index_table = 0,
    std::multimap<int, Comment*>* comments = 0);
int loadRDARNameFile(STNameList* list,
                     const char* dir,
                     const char* filename);
int addRDARScriptListFile(STScriptFileList* list,
                          const char* dir,
                          const char* filename);
// IDA_EA: InitRDARScriptStringManager @0x88BFF9C.
int InitRDARScriptStringManager(const char* filename,
                                const char* dir,
                                std::vector<ErrorLog>* errors);

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_RDAR_SCRIPT_H_
