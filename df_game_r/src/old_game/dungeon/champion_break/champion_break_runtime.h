#ifndef OLDSRV_OLD_GAME_DUNGEON_CHAMPION_BREAK_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_CHAMPION_BREAK_RUNTIME_H_

class CParty;
class CUser;
class PacketGuard;
struct itemGloballyUniqueIdentifier_t;

namespace champion_break {

// Official PVF owners:
//   etc/dungeonetc/championbreaksystem.etc
//   etc/variousbufflist.etc (Champion_Break, indexes 1..5)
// The feature is optional so a stock 0627 PVF still starts normally.
bool LoadConfig();
bool IsDungeonEnabled(int dungeon_index);

// CLIENT_IDA_EVIDENCE: NOTI28/sub_582250 reads this byte immediately before
// the dungeon-mission u32 descriptor and routes it to sub_4689E0(bool).
void WriteDungeonInfoEnableFlag(PacketGuard& packet, int dungeon_index);

// A committed dungeon start owns a fresh counter generation.  Champion
// monster kinds 1/2 increment it; reaching the configured threshold emits
// NOTI603 exactly once while the later clear result retains the success latch.
void BeginDungeonRun(CParty* party, int dungeon_index);
void OnMonsterKilled(CParty* party, int monster_kind);

// S2C70 always contains eight u16 rights.  Returns true only for the special
// ChampionBreak stage (all eight values non-zero -> client result state 9).
// The ordinary stage has four canonical rights followed by four zeroes.
bool WriteCardSelectRights(CParty* party, PacketGuard& packet);

// ChampionBreak reuses card_type 1 in the ordinary two-byte click grammar.
// card_type 0 remains CParty's normal upper row.  Both rows complete in the
// same state-9 round; type 1 records a free owner in selected_cards_[1] so the
// native dungeon_clear path grants the original gold-card contents.
bool IsCombinedCardStage(CParty* party);
bool TryHandleCardSelect(CUser* user, CParty* party, int card_index);
void SendCombinedCardInfo(CParty* party);
bool FinishCombinedCardSelectIfReady(CParty* party);
bool HandleCardSelectTimeout(CParty* party);

// The selected one-match buff becomes active only on the next eligible normal
// dungeon.  Completion/give-up consumes the active generation but preserves a
// newly selected pending buff.
void FinishDungeonRun(CParty* party);
void AbortDungeonRun(CParty* party);
void ResetSession(CUser* user);

int GetHunterExpBonusRate(CUser* user);
int GetClearExpBonusRate(CUser* user);
int GetMagicSealDropBonusRate(CUser* user);
int GetMagicSealMinimumUpgrade(CUser* user);
int GetMagicSealMaximumUpgrade(CUser* user);
int GetUniqueDropBonusRate(CUser* user);

// Server-side, non-persistent view of variousbufflist effect 5.  The backing
// Inven_Item is never changed, so database/inventory-save serializers always
// see the real reinforcement value.  Runtime equipment packet/stat consumers
// may request the effective slot-11 value during the one-match run.
int GetProjectedEquipmentUpgrade(CUser* user,
                                 int equipment_slot,
                                 int item_id,
                                 const itemGloballyUniqueIdentifier_t& guid,
                                 int stored_upgrade);
void RecordHunterExpBonus(CUser* user, int bonus_exp);
int ConsumeHunterExpBonus(CUser* user);

}  // namespace champion_break

#endif  // OLDSRV_OLD_GAME_DUNGEON_CHAMPION_BREAK_RUNTIME_H_
