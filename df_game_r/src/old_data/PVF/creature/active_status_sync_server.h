#ifndef DATAMANAGER_ACTIVE_STATUS_SYNC_SERVER_H_
#define DATAMANAGER_ACTIVE_STATUS_SYNC_SERVER_H_

// Canonical B-owned PVF creature-status declarations.

#include <stddef.h>

#include <map>
#include <string>
#include <vector>

// The original enum has no recovered enumerator labels, but its global name
// and i386 int ABI are used by the active-status SDC calculation entrypoints.
enum ENUM_ACTIVESTATUS : int {};

struct IRDActiveObject;

// IDA_EA: 0x808D0A6 / 0x808D18C and the CMD184 payload reader at 0x81EA16E.
// This 0x35-byte packed record is copied before the server recomputes status
// damage and duration.  Fields without a recovered semantic label stay raw.
#pragma pack(push, 1)
struct STSDCActiveStatusSyncDataSend {
    uint8_t raw_00;                         // +0x00
    int8_t active_status_type;              // +0x01
    uint8_t active_status_level;            // +0x02
    int32_t damage_value;                   // +0x03
    int32_t duration;                       // +0x07
    uint8_t source_kind;                    // +0x0b
    uint8_t is_user_to_user;                // +0x0c
    uint32_t raw_0d;                        // +0x0d
    uint32_t raw_11;                        // +0x11
    uint32_t raw_15;                        // +0x15
    uint32_t raw_19;                        // +0x19
    int32_t source_level;                   // +0x1d
    uint32_t raw_21;                        // +0x21
    int32_t monster_unique_id;              // +0x25
    uint32_t object_type_mask;              // +0x29; _SDC bitmask value
    int32_t raw_2d;                         // +0x2d
    int32_t raw_31;                         // +0x31
};

#pragma pack(pop)

float GetUniformVelocityFloatSDC(float first,
                                 float last,
                                 int position,
                                 int maximum_position);

// IDA_EA: 0x8096A2C, 0x8096AB4, 0x8096B22.
struct CreatureCommonScript {
    CreatureCommonScript();
    ~CreatureCommonScript();
    CreatureCommonScript* clear();

    float attack_constant;
    int response_delay;
    int evolution_fade_in_delay;
    int evolution_fade_out_delay;
    int evolution_delay;
    int dying_delay;
    std::vector<int> damage;
    std::vector<int> mana;
    int second_per_fill_point_spend;
    int second_per_fill_point_charge;
    int max_fill_point;
    int fill_point_to_eat_feed;
    int ambient_in_room_list[2];
    int ambient_in_main_game[2];
    int ambient_in_pvp[2];
    int ambient_in_room_list_time_term;
    int ambient_in_room_list_force_delay;
    int ambient_in_main_game_time_term;
    int ambient_in_main_game_force_delay;
    int ambient_in_pvp_time_term;
    int ambient_in_pvp_force_delay;
};

// IDA_EA: 0x8234870, 0x8234876.
class SDCActiveStatusSyncCommon {
public:
    SDCActiveStatusSyncCommon();
    ~SDCActiveStatusSyncCommon();

    int getActiveStatusDurationClient(int active_status,
                                      int level,
                                      IRDActiveObject* active_object);
    int getActiveStatusDurationServer(int active_status,
                                      int level,
                                      int object_type_mask,
                                      int raw_2d,
                                      int raw_31);
    float getActiveStatusPower1(int active_status, int level);
    float getActiveStatusPower2(int active_status, int level);
    float getActiveStatusEtcParameter(int active_status,
                                      int level,
                                      int parameter_index);
    int makeEtcParameters(ENUM_ACTIVESTATUS active_status,
                          int level,
                          void* active_status_data,
                          std::vector<int>* parameters,
                          int* duration);
    int makeEtcParameters(ENUM_ACTIVESTATUS active_status,
                          int level,
                          std::vector<int>& parameters);
    int getDuration(ENUM_ACTIVESTATUS active_status,
                    int level,
                    void* active_status_data);
    int getPoisonDamage(STSDCActiveStatusSyncDataSend* data,
                        void* unused,
                        int elapsed);
    int getDiseaseDamage(STSDCActiveStatusSyncDataSend* data, int elapsed);
    int getBurnDamage(bool& valid,
                      STSDCActiveStatusSyncDataSend* data,
                      void* unused,
                      int elapsed);
    int getBleedingDamageClient(STSDCActiveStatusSyncDataSend* data,
                                bool unused,
                                void* unused_data);
    int getBleedingDamageServer(STSDCActiveStatusSyncDataSend* data);
    float getUserToUserDamageRate(bool death_tower);
    void getActiveStatusDamageDurationForSDC(
        char active_status,
        int level,
        int* duration,
        int* damage,
        STSDCActiveStatusSyncDataSend* data,
        void* unused);
};

// IDA_EA: 0x808E13C, 0x823487C, 0x8234890.
class SDCActiveStatusSyncServer : public SDCActiveStatusSyncCommon {
public:
    SDCActiveStatusSyncServer();
    ~SDCActiveStatusSyncServer();

    int loadActiveStatusScript();
};

enum ENUM_MODULE_TYPE {
    ENUM_MODULE_TYPE_NONE = 0
};

int getModuleTypeFromTag(std::string& tag);
float* addCreatureModuleRate(ENUM_MODULE_TYPE module_type, float rate);
int prepareServerDelayCheckDamage();
int importEtcParameterScriptForServer();
int importCreatureCommonScript(CreatureCommonScript* script,
                               const char* filename,
                               std::vector<int>* damage_vector);
int importActiveStatusTable(const char* filename);

extern int ACTIVESTATUS_MAXLEVEL;
extern int ACTIVESTATUS_DURATION[18];
extern float ACTIVESTATUS_POWER1[36];
extern float ACTIVESTATUS_POWER2[36];
extern float ACTIVESTATUS_ETC_PARAMETERS[144];
extern float ACTIVESTATUS_LEVEL_RATE_FOR_PROB;
extern float ACTIVESTATUS_LEVEL_RATE_FOR_POWER;
extern float ACTIVESTATUS_LEVEL_ABSOLUTE_RATE;
extern int LIGHTNING_TIME_PER_NUMBER;
extern float ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE;
extern float ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE_DEATH_TOWER;

// IDA_EA: 0x8095275..0x80966F1.  These are populated by
// importEtcParameterScriptForServer before CMD183 SDC damage validation runs.
extern float g_physicalDamageRateConst_;
extern float g_physicalReduceRateConst_;
extern float g_physicalRandomBaseConst_;
extern float g_physicalRandomRateConst_;
extern float g_magicalDamageRateConst_;
extern float g_magicalReduceRateConst_;
extern float g_magicalRandomBaseConst_;
extern float g_magicalRandomRateConst_;
extern float g_commonElementBaseConst_;

extern std::vector<int> g_creature_damage_vector_;
extern std::map<int, float> g_creatureModuleDamageRateMap;
extern std::map<int, float> g_creatureIndexDamageRateMap;
extern const char* const g_module_type_start_tags[17];
extern const char* const g_module_type_end_tags[17];
extern char g_creatureScriptBaseDirectory[256];

static_assert(sizeof(CreatureCommonScript) == 0x70u,
              "CreatureCommonScript 0828 layout size must stay 0x70");
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STSDCActiveStatusSyncDataSend) == 0x35u,
              "STSDCActiveStatusSyncDataSend IDA ABI size must stay 0x35");
static_assert(offsetof(STSDCActiveStatusSyncDataSend, damage_value) == 0x03u,
              "STSDCActiveStatusSyncDataSend damage offset");
static_assert(offsetof(STSDCActiveStatusSyncDataSend, monster_unique_id) ==
                  0x25u,
              "STSDCActiveStatusSyncDataSend monster ID offset");
static_assert(offsetof(STSDCActiveStatusSyncDataSend, object_type_mask) ==
                  0x29u,
              "STSDCActiveStatusSyncDataSend object-type offset");
#endif
static_assert(sizeof(SDCActiveStatusSyncCommon) == 1u,
              "SDCActiveStatusSyncCommon must remain an empty IDA base");
static_assert(sizeof(SDCActiveStatusSyncServer) == 1u,
              "SDCActiveStatusSyncServer must remain an empty IDA subtype");

#endif  // DATAMANAGER_ACTIVE_STATUS_SYNC_SERVER_H_
