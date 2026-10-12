#ifndef OLDSRV_COMMON_CONFIG_H_
#define OLDSRV_COMMON_CONFIG_H_

namespace oldsrv {
namespace config {

enum CharacterJobId {
    kCharacterJobSwordman = 0,
    kCharacterJobFighter = 1,
    kCharacterJobGunner = 2,
    kCharacterJobMage = 3,
    kCharacterJobPriest = 4,
    kCharacterJobAtGunner = 5,
    kCharacterJobThief = 6,
    kCharacterJobAtFighter = 7,
    kCharacterJobAtMage = 8,
    kCharacterJobDemonicSwordman = 9,
    kCharacterJobCreatorMage = 10,
    kCharacterJobAtSwordman = 11,
    kCharacterJobCount = 12,
    kCharacterJobCommon = kCharacterJobCount,
    kCharacterJobInvalid = kCharacterJobCommon,
    kCharacterJobWithCommonCount = kCharacterJobCount + 1
};

enum CharacterLevelConfig {
    kCharacterLevelMinimum = 1,
    kCharacterLevelMaximum = 85,
    kCharacterLevelBeforeMaximum = kCharacterLevelMaximum - 1,
    kCharacterLevelCount =
        kCharacterLevelMaximum - kCharacterLevelMinimum + 1,
    kCharacterLevelTableCount = kCharacterLevelMaximum + 1
};

enum CharacterLevelSubsystemConfig {
    kCharacterLevelRevision = 70,
    kTrainingQuestLevelMaximum = 70,
    kTrainingQuestLevelCount =
        kTrainingQuestLevelMaximum - kCharacterLevelMinimum + 1,
    kLevelRewardTableLevelCapacity = 70,
    kFatigueBatterySpecialCharacterLevel = 70,
    kFatigueBatteryMoneyActivationLevelDefault = 70,
    kChannelLevelStatisticMaximum = 70,
    kChannelLevelStatisticCount =
        kChannelLevelStatisticMaximum - kCharacterLevelMinimum + 1
};

enum CharacterJobSubsystemConfig {
    kLevelRewardTableJobCount = 8,
    kLevelRewardTableJobIndexCount = kLevelRewardTableJobCount + 1
};

// premiumlist_new.etc uses definition ids through 109.  This is the PVF
// definition-table extent; it is deliberately separate from the legacy
// per-user premium-state layout embedded in CUser.
enum PremiumDefinitionConfig {
    kPremiumDefinitionTypeMaximum = 109,
    kPremiumDefinitionTypeCount = kPremiumDefinitionTypeMaximum + 1
};

// The target PVF contains a native [weapon avatar] equipment category. The
// client consumes it at slot 10, so the following equipment categories use
// their native shifted values without a compatibility mapping layer.
enum EquipmentTypeConfig {
    kEquipmentTypeHatAvatar = 0,
    kEquipmentTypeHairAvatar = 1,
    kEquipmentTypeFaceAvatar = 2,
    kEquipmentTypeCoatAvatar = 3,
    kEquipmentTypePantsAvatar = 4,
    kEquipmentTypeShoesAvatar = 5,
    kEquipmentTypeBreastAvatar = 6,
    kEquipmentTypeWaistAvatar = 7,
    kEquipmentTypeSkinAvatar = 8,
    kEquipmentTypeAuroraAvatar = 9,
    kEquipmentTypeWeaponAvatar = 10,
    kEquipmentTypeWeapon = 11,
    kEquipmentTypeTitle = 12,
    kEquipmentTypeCoat = 13,
    kEquipmentTypeShoulder = 14,
    kEquipmentTypePants = 15,
    kEquipmentTypeShoes = 16,
    kEquipmentTypeWaist = 17,
    kEquipmentTypeAmulet = 18,
    kEquipmentTypeWrist = 19,
    kEquipmentTypeRing = 20,
    kEquipmentTypeSupport = 21,
    kEquipmentTypeMagicStone = 22,
    kEquipmentTypeCreature = 23,
    kEquipmentTypeArtifactRed = 24,
    kEquipmentTypeArtifactBlue = 25,
    kEquipmentTypeArtifactGreen = 26,
    kEquipmentTypeCount = 27,
    kEquipmentTypeInvalid = kEquipmentTypeCount,

    kAvatarEquipmentTypeFirst = kEquipmentTypeHatAvatar,
    kAvatarEquipmentTypeLast = kEquipmentTypeWeaponAvatar,
    kAvatarEquipmentTypeCount =
        kAvatarEquipmentTypeLast - kAvatarEquipmentTypeFirst + 1,
    kAvatarInventorySlotBase = kAvatarEquipmentTypeCount,

    kOrdinaryEquipmentTypeFirst = kEquipmentTypeWeapon,
    kOrdinaryEquipmentTypeLast = kEquipmentTypeMagicStone,
    kOrdinaryEquipmentTypeCount =
        kOrdinaryEquipmentTypeLast - kOrdinaryEquipmentTypeFirst + 1,
    kOrdinaryEquipmentTypeMask =
        ((1u << kOrdinaryEquipmentTypeCount) - 1u)
        << kOrdinaryEquipmentTypeFirst,

    kCharacterEquipmentTypeFirst = kEquipmentTypeHatAvatar,
    kCharacterEquipmentTypeLast = kEquipmentTypeMagicStone,
    kCharacterEquipmentTypeCount =
        kCharacterEquipmentTypeLast - kCharacterEquipmentTypeFirst + 1,

    kCreatureArtifactEquipmentTypeFirst = kEquipmentTypeCreature,
    kCreatureArtifactEquipmentTypeLast = kEquipmentTypeArtifactGreen,
    kCreatureArtifactEquipmentTypeCount =
        kCreatureArtifactEquipmentTypeLast -
        kCreatureArtifactEquipmentTypeFirst + 1,

    kCharacterViewEquipmentTypeFirst = kEquipmentTypeHatAvatar,
    kCharacterViewEquipmentTypeLast = kEquipmentTypeTitle,
    kCharacterViewEquipmentTypeCount =
        kCharacterViewEquipmentTypeLast -
        kCharacterViewEquipmentTypeFirst + 1
};

// The target client exposes five 56-slot main-inventory pages followed by
// the existing 63-slot jewel page.  Database persistence keeps the official
// split: the primary inventory blob owns slots 0..288 and the jewel blob owns
// slots 289..351.
enum InventoryLayoutConfig {
    kInventoryItemRecordSize = 0x3D,

    kMainInventoryQuickSlotFirst = 3,
    kMainInventoryQuickSlotLast = 8,
    kMainInventoryEquipmentSlotFirst = 9,
    kMainInventoryEquipmentSlotLast = 64,
    kMainInventoryConsumableSlotFirst = 65,
    kMainInventoryConsumableSlotLast = 120,
    kMainInventoryMaterialSlotFirst = 121,
    kMainInventoryMaterialSlotLast = 176,
    kMainInventoryQuestSlotFirst = 177,
    kMainInventoryQuestSlotLast = 232,
    kMainInventoryEtcSlotFirst = 233,
    kMainInventoryEtcSlotLast = 288,
    kMainInventoryJewelSlotFirst = 289,
    kMainInventoryJewelSlotLast = 351,

    kMainInventoryItemCount = 352,
    kMainInventoryLastSlot = kMainInventoryItemCount - 1,
    kMainInventoryPrimaryItemCount = 289,
    kMainInventoryExpandedItemCount =
        kMainInventoryItemCount - kMainInventoryPrimaryItemCount,

    kAvatarInventoryItemCount = 105,
    kAvatarInventoryLastSlot = kAvatarInventoryItemCount - 1,
    kCreatureInventoryItemCount = 242,
    kCreatureInventoryLastSlot = kCreatureInventoryItemCount - 1,
    // 0828 keeps creature/artifact equipment at physical equipment slots
    // 23..26, while the DB/login creature table exposes the same four
    // records at its canonical tail 238..241.  These are intentionally
    // different namespaces; persist through this projection instead of
    // carrying the 0627 physical-slot offset forward.
    kCreatureInventoryEquipmentFirst = 238,
    kCreatureInventoryEquipmentLast =
        kCreatureInventoryEquipmentFirst +
        kCreatureArtifactEquipmentTypeCount - 1,
    kCreatureInventoryEquippedCreatureSlot =
        kCreatureInventoryEquipmentFirst,
    kCreatureEquipmentPersistenceOffset =
        kCreatureInventoryEquipmentFirst -
        kCreatureArtifactEquipmentTypeFirst,
    kCargoInventoryItemCount = 152,
    kCargoInventoryLastSlot = kCargoInventoryItemCount - 1,
    kPersistedClearAvatarCount = 11,
    kAvatarDetailItemCount =
        kAvatarEquipmentTypeCount + kAvatarInventoryItemCount,

    kEquippedAvatarAndOrdinaryItemCount =
        kAvatarEquipmentTypeCount + kOrdinaryEquipmentTypeCount,
    kEquipmentTransferItemCount = kEquipmentTypeCount,

    kMainInventoryGrowthItemCount = 40,
    kMainInventoryGrowthByteCount =
        kMainInventoryGrowthItemCount * kInventoryItemRecordSize
};

static_assert(kCreatureInventoryEquipmentLast == kCreatureInventoryLastSlot,
              "0828 creature equipment tail must remain DB slots 238..241");
static_assert(kEquipmentTypeCreature + kCreatureEquipmentPersistenceOffset ==
                  kCreatureInventoryEquippedCreatureSlot,
              "0828 physical creature slot 23 must persist as DB slot 238");

// CMD197 and NOTI173 carry a fixed sequence of 16-bit game options for the
// selected client generation. Future upgrades change the count here; packet,
// DB compression and login-return sizes are derived from it.
enum GameOptionPayloadConfig {
    kGameEtcOptionCount = 38,
    kGameEtcOptionBytes = kGameEtcOptionCount * 2,
    kGameEtcOptionDefaultValue = 0x7fff,
    kMaximumCompressedGameEtcOptionBytes =
        kGameEtcOptionBytes + 64
};

enum GameOptionIndex {
    kGameEtcOptionTargetSelectionMode = 36,
    kGameEtcOptionReserved37 = 37
};

}  // namespace config
}  // namespace oldsrv

#endif  // OLDSRV_COMMON_CONFIG_H_
