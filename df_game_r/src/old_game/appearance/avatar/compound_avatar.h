#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_H_

#include <stddef.h>

#include <map>
#include <vector>

#include "common/config.h"
#include "old_game/appearance/avatar/compound_avatar_value_types.h"
#include "old_game/economy/inventory/inventory.h"

class CUser;
class CMTRand;
struct MSG_COMPOUND_AVATAR;
enum ENUM_ERROR : int;

namespace WongWork {

struct CCompoundAvatarAbiLayoutAssert;

// IDA unit: 06_userinfo_stat_inventory/WongWork_CCompoundAvatar.
//
// The object is an actual process-owned table set, constructed by
    // CDataManager::init before CompoundAvatar.etc is imported.  The twelve
    // 0828 per-job tables occupy +0x00..+0x8f and the random generator pointer
    // is at +0x90; there is no opaque tail or compatibility storage.
class CCompoundAvatar {
    friend struct CCompoundAvatarAbiLayoutAssert;
public:
    // IDA_EA: 0x8331D74
    // IDA_NAME: _ZN8WongWork15CCompoundAvatarC2Ev
    // DEMANGLED: WongWork::CCompoundAvatar::CCompoundAvatar(void)
    CCompoundAvatar();

    // IDA_EA: 0x8331EA2
    // IDA_NAME: _ZN8WongWork15CCompoundAvatarD2Ev
    // DEMANGLED: WongWork::CCompoundAvatar::~CCompoundAvatar()
    ~CCompoundAvatar();

    // IDA_EA: 0x8333EA6
    // IDA_NAME: _ZN8WongWork15CCompoundAvatar10LoadScriptEPKcS2_
    // DEMANGLED: WongWork::CCompoundAvatar::LoadScript(char const*,char const*)
    bool LoadScript(const char* directory, const char* filename);

    // The original transaction stays with this process-owned PVF table: it
    // validates both avatar records, consumes its exact material list, makes
    // the weighted result choice, mutates the avatar manager and emits the
    // CMD102 result/statistic packets.
    class tagCompoundInfo;
    tagCompoundInfo _MakeCompoundInfo(
        const CUser* user, const MSG_COMPOUND_AVATAR* request) const;
    int _CheckCondition(const CUser* user, const tagCompoundInfo& info) const;
    int _ProcCompoundCore(CUser* user,
                          tagCompoundInfo& info,
                          tagCompoundAvatarTable* table);
    void ProcCompound(CUser* user, const MSG_COMPOUND_AVATAR* request);
    int _SendError(CUser* user, ENUM_ERROR error);
    int _SendResult(CUser* user, const tagCompoundInfo& info);
    char* _generateIPGNo(const CUser* user, long timer, char* output) const;
    int _SendDisjointAvatarStatistic(tagCompoundInfo& info);
    bool ProcTest(int character_job,
                  int equipment_type,
                  int first,
                  int second,
                  std::map<int, int>& result);

    tagCompoundAvatarTable* table_for_job(int job) {
        return &tables_[job];
    }
    const tagCompoundAvatarTable* table_for_job(int job) const {
        return &tables_[job];
    }

private:
    tagCompoundAvatarTable
        tables_[oldsrv::config::kCharacterJobCount];  // 0828 +0x00 .. +0x8f
    CMTRand* random_;                                  // 0828 +0x90
};

// IDA_EA: 0x833424A / 0x8334344 / 0x8334388.
//
// This transaction value is stack-owned by ProcCompound.  Its three
// Inven_Item snapshots are real subobjects, and the vector at +0xdc carries
// the avatar mutations emitted after the commit.  Natural i386 alignment
// accounts for the observed gaps; no byte storage or fabricated tail is used.
class CCompoundAvatar::tagCompoundInfo {
public:
    // IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfoC2Ev
    tagCompoundInfo();
    // IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfoD2Ev
    ~tagCompoundInfo();
    // IDA_NAME: _ZN8WongWork15CCompoundAvatar15tagCompoundInfo5clearEv
    void clear();

    int character_job;                 // IDA +0x00
    int compound_grade;                 // IDA +0x04
    uint16_t first_avatar_slot;         // IDA +0x08
    Inven_Item first_avatar;            // IDA +0x0a
    uint16_t second_avatar_slot;        // IDA +0x48
    Inven_Item second_avatar;           // IDA +0x4a
    CItem* first_avatar_prototype;      // IDA +0x88
    CItem* second_avatar_prototype;     // IDA +0x8c
    uint16_t recipe_slot;               // IDA +0x90
    Inven_Item recipe;                  // IDA +0x92
    int requested_result_item;          // IDA +0xd0
    uint16_t requested_avatar_option;   // IDA +0xd4
    bool upper_compound;                // IDA +0xd6
    bool invalid_request;               // IDA +0xd7
    // clear() sets this to 0xffff; _ProcCompoundCore @ 0x8332EE8 replaces it
    // with AddAvatarItem's returned slot before _SendResult @ 0x8333A9C.
    uint16_t new_avatar_slot;           // IDA +0xd8
    std::vector<int> avatar_mutations;  // IDA +0xdc
    uint8_t compound_kind;              // IDA +0xe8
};

#if defined(__i386__)
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, character_job) ==
                  0x00u,
              "compound info job must stay at IDA +0x00");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, first_avatar) ==
                  0x0au,
              "compound info first avatar must stay at IDA +0x0a");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, second_avatar) ==
                  0x4au,
              "compound info second avatar must stay at IDA +0x4a");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo,
                       first_avatar_prototype) == 0x88u,
              "compound info first prototype must stay at IDA +0x88");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, recipe) == 0x92u,
              "compound info recipe must stay at IDA +0x92");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo,
                       requested_result_item) == 0xd0u,
              "compound info result id must stay at IDA +0xd0");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, new_avatar_slot) ==
                  0xd8u,
              "compound info created avatar slot must stay at IDA +0xd8");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo,
                       avatar_mutations) == 0xdcu,
              "compound info mutation vector must stay at IDA +0xdc");
static_assert(offsetof(CCompoundAvatar::tagCompoundInfo, compound_kind) ==
                  0xe8u,
              "compound info kind must stay at IDA +0xe8");
static_assert(sizeof(CCompoundAvatar::tagCompoundInfo) == 0xecu,
              "compound info IDA stack object must stay 0xec");
#endif

struct CCompoundAvatarAbiLayoutAssert {
#if defined(__i386__)
    static_assert(offsetof(CCompoundAvatar, tables_) == 0x00u,
                  "CCompoundAvatar table array must begin at IDA +0x00");
    static_assert(offsetof(CCompoundAvatar, random_) == 0x90u,
                  "CCompoundAvatar random generator must stay at 0828 +0x90");
    static_assert(sizeof(CCompoundAvatar) == 0x94u,
                  "CCompoundAvatar 0828 object size must stay 0x94");
#endif
};

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_COMPOUND_AVATAR_H_
