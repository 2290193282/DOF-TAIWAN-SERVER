#ifndef OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_INITIALIZERS_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_INITIALIZERS_H_

namespace oldsrv {
namespace official_registry {

// 0627 official-server PVF registry ownership.  The 0828 client PVF still
// carries this complete resource, but its dungeon/world-map list rows were
// accidentally omitted.  Keep the two registry consumers on one identity.
enum {
    kSouthernDaleDungeonIndex = 100,
    kSouthernDaleWorldMapIndex = 14
};

inline const char* SouthernDaleDungeonScriptPath() {
    // Script.pvf stores this key in canonical ASCII-lowercase form.  Keep
    // internal references inside the DGN untouched; this is only the packed
    // registry/load key relative to Dungeon/.
    return "southerndale/grimseeker.dgn";
}

}  // namespace official_registry
}  // namespace oldsrv

// IDA_EA: 0x8A0CC3C
// IDA_NAME: _Z19InitCharacterScriptPKcS0_S0_S0_
// DEMANGLED: InitCharacterScript(char const*,char const*,char const*,char const*)
int InitCharacterScript(const char* src,
                        const char* list_file,
                        const char* exp_table,
                        const char* pvp_price_table);

// IDA_EA: 0x8A29EAD
// IDA_NAME: _Z23InitPassiveObjectScriptPKcS0_
// DEMANGLED: InitPassiveObjectScript(char const*,char const*)
int InitPassiveObjectScript(const char* src, const char* list_file);

// IDA_EA: 0x8991601
// IDA_NAME: _Z19InitEquipmentScriptPKcS0_S0_
// DEMANGLED: InitEquipmentScript(char const*,char const*,char const*)
int InitEquipmentScript(const char* src,
                        const char* list_file,
                        const char* price_table);

// IDA_EA: 0x89DCE98
// IDA_NAME: _Z13InitMapScriptPKcS0_
// DEMANGLED: InitMapScript(char const*,char const*)
int InitMapScript(const char* src, const char* list_file);

// IDA_EA: 0x88E5E3C
// IDA_NAME: _Z17InitDungeonScriptPcS_
// DEMANGLED: InitDungeonScript(char*,char*)
int InitDungeonScript(char* src, char* list_file);

// IDA_EA: 0x8A61642
// IDA_NAME: _Z15initQuestScriptPKcS0_
// DEMANGLED: initQuestScript(char const*,char const*)
int initQuestScript(const char* src, const char* list_file);

// IDA_EA: 0x8A97438
// IDA_NAME: _Z15InitSkillScriptPKcS0_
// DEMANGLED: InitSkillScript(char const*,char const*)
int InitSkillScript(const char* src, const char* list_file);

// IDA_EA: 0x88C98D4
// IDA_NAME: _Z21initAICharacterScriptPKcS0_
// DEMANGLED: initAICharacterScript(char const*,char const*)
int initAICharacterScript(const char* src, const char* filename);

// IDA_EA: 0x895AE5C
// IDA_NAME: _Z24initEventCharacterScriptPcS_
// DEMANGLED: initEventCharacterScript(char*,char*)
int initEventCharacterScript(char* src, char* filename);

// IDA_EA: 0x8A5AAD5
// IDA_NAME: _Z20initPvpMissionSystemPcS_
// DEMANGLED: initPvpMissionSystem(char*,char*)
int initPvpMissionSystem(char* src, char* filename);

// IDA_EA: 0x8A5CD6C
// IDA_NAME: _Z22initPvPSkillTreeScriptPcS_
// DEMANGLED: initPvPSkillTreeScript(char*,char*)
int initPvPSkillTreeScript(char* src, char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_COMMON_SCRIPT_INITIALIZERS_H_
