#ifndef OLDSRV_GLOBAL_ITEM_OPTION_H_
#define OLDSRV_GLOBAL_ITEM_OPTION_H_

#include <stddef.h>
#include <stdint.h>


// IDA unit: 99_misc_global/stAmplifyOption_t.
struct __attribute__((packed)) stAmplifyOption_t {
    stAmplifyOption_t* reset();
    int getAbilityType() const;
    int getAbilityValue() const;
    char isIdentified() const;
    bool hasAbility() const;
    int identify();
    stAmplifyOption_t* _setAbility(uint8_t ability_type,
                                   uint16_t ability_value,
                                   bool identified);
    int GetLock() const;
    stAmplifyOption_t* SetLock(uint8_t lock);
    uint16_t* getValues(uint8_t& ability_type,
                        uint16_t& ability_value) const;
    stAmplifyOption_t* assign(uint8_t ability_type, uint16_t ability_value);

    // IDA +0x00/+0x01/+0x03 respectively.  The high bit of the first
    // byte is the unidentified marker; it is deliberately part of the
    // original byte rather than a C++ bit-field.
    uint8_t ability_type_and_identification_;
    uint16_t ability_value_;
    uint8_t lock_;
};

static_assert(sizeof(stAmplifyOption_t) == 4u,
              "stAmplifyOption_t IDA ABI size must stay 4");
static_assert(offsetof(stAmplifyOption_t, ability_type_and_identification_) == 0x00u,
              "stAmplifyOption_t ability byte must retain IDA +0x00");
static_assert(offsetof(stAmplifyOption_t, ability_value_) == 0x01u,
              "stAmplifyOption_t ability value must retain IDA +0x01");
static_assert(offsetof(stAmplifyOption_t, lock_) == 0x03u,
              "stAmplifyOption_t lock must retain IDA +0x03");

// IDA unit: 99_misc_global/itemGloballyUniqueIdentifier_t.
struct __attribute__((packed)) itemGloballyUniqueIdentifier_t {
    bool reset();
    itemGloballyUniqueIdentifier_t* SetServerId(int server_id);
    char* SetDatetime(int datetime);
    char* SetSequenceNo(uint16_t sequence_no);

    int32_t server_id_;
    int32_t datetime_;
    uint16_t sequence_no_;
};

static_assert(sizeof(itemGloballyUniqueIdentifier_t) == 10u,
              "itemGloballyUniqueIdentifier_t IDA ABI size must stay 10");
static_assert(offsetof(itemGloballyUniqueIdentifier_t, server_id_) == 0x00u,
              "itemGloballyUniqueIdentifier_t server must retain IDA +0x00");
static_assert(offsetof(itemGloballyUniqueIdentifier_t, datetime_) == 0x04u,
              "itemGloballyUniqueIdentifier_t datetime must retain IDA +0x04");
static_assert(offsetof(itemGloballyUniqueIdentifier_t, sequence_no_) == 0x08u,
              "itemGloballyUniqueIdentifier_t sequence must retain IDA +0x08");

// IDA unit: 99_misc_global/CItemGloballyUniqueIdentifierGenerator.
class CItemGloballyUniqueIdentifierGenerator {
public:
    CItemGloballyUniqueIdentifierGenerator();
    itemGloballyUniqueIdentifier_t* generate(
        itemGloballyUniqueIdentifier_t* identifier,
        int server_id);
    int getSeq();

private:
    uint16_t sequence_;
};

static_assert(sizeof(CItemGloballyUniqueIdentifierGenerator) == 0x02u,
              "CItemGloballyUniqueIdentifierGenerator IDA ABI size must stay 2");

extern CItemGloballyUniqueIdentifierGenerator
    g_ItemGloballyUniqueIdentifierGenerator;

// IDA unit: 99_misc_global/CItemGloballyUniqueIdentifierPrint. The old C2
// clears exactly 0x15 bytes and operator() writes the ten-byte identifier as
// a 20-character lower-case hexadecimal C string.
class CItemGloballyUniqueIdentifierPrint {
public:
    // IDA_EA: 0x84EBE58
    // IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintC2Ev
    // DEMANGLED: CItemGloballyUniqueIdentifierPrint::CItemGloballyUniqueIdentifierPrint(void)
    CItemGloballyUniqueIdentifierPrint();
    // IDA_EA: 0x84EBE7C
    // IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintD2Ev
    // DEMANGLED: CItemGloballyUniqueIdentifierPrint::~CItemGloballyUniqueIdentifierPrint()
    ~CItemGloballyUniqueIdentifierPrint();
    // IDA_EA: 0x84EBE82
    // IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintclERK30itemGloballyUniqueIdentifier_t
    // DEMANGLED: CItemGloballyUniqueIdentifierPrint::operator()(itemGloballyUniqueIdentifier_t const&)
    char* operator()(const itemGloballyUniqueIdentifier_t& identifier);

private:
    char text_[0x15];
};

static_assert(sizeof(CItemGloballyUniqueIdentifierPrint) == 0x15u,
              "CItemGloballyUniqueIdentifierPrint IDA ABI size must stay 0x15");

// IDA unit: 99_misc_global/RandomOptionField.
struct RandomOptionField {
    RandomOptionField* reset();
    bool empty() const;

    // Packet writers at 0x811F1AE/0x858E3EC emit these three bytes in this
    // order.  `empty` tests option_index_ only.
    uint8_t option_index_;
    uint8_t first_value_;
    uint8_t second_value_;
};

static_assert(sizeof(RandomOptionField) == 3u,
              "RandomOptionField IDA ABI size must stay 3");
static_assert(offsetof(RandomOptionField, option_index_) == 0x00u,
              "RandomOptionField option index must retain IDA +0x00");
static_assert(offsetof(RandomOptionField, first_value_) == 0x01u,
              "RandomOptionField first value must retain IDA +0x01");
static_assert(offsetof(RandomOptionField, second_value_) == 0x02u,
              "RandomOptionField second value must retain IDA +0x02");

// IDA unit: 99_misc_global/RandomOptionSeed.
struct RandomOptionSeed {
    RandomOptionSeed* reset();

    uint8_t seed_;
};

static_assert(sizeof(RandomOptionSeed) == 1u,
              "RandomOptionSeed IDA ABI size must stay 1");
static_assert(offsetof(RandomOptionSeed, seed_) == 0x00u,
              "RandomOptionSeed value must retain IDA +0x00");

// IDA_EA: 0x85F4002.  The original ABI names this enum even though all
// recovered callers presently use only the three wire indices 0, 1 and 2.
// Keeping it as a distinct enum is required for the member-function names
// emitted by the old i386 C++ ABI.
enum ENUM_RANDOM_OPTION_NUMBER {
    ENUM_RANDOM_OPTION_NUMBER_0 = 0,
    ENUM_RANDOM_OPTION_NUMBER_1 = 1,
    ENUM_RANDOM_OPTION_NUMBER_2 = 2,
};

// IDA unit: 99_misc_global/RandomOption. Packet serialization specializations
// remain separate PacketBuf/PacketGuard restoration work.
struct __attribute__((packed)) RandomOption {
    RandomOptionSeed* reset();
    bool empty();

    // IDA_EA: 0x85F4002 / 0x85F4040 / 0x85F4078 / 0x85F40BE.
    // A pending changed-option overlays the selected normal option; these
    // accessors preserve that original persistence and packet semantics.
    bool change_option(ENUM_RANDOM_OPTION_NUMBER option_number);
    int get_option_index(ENUM_RANDOM_OPTION_NUMBER option_number);
    int set_first_value(ENUM_RANDOM_OPTION_NUMBER option_number,
                        uint8_t value);
    int set_second_value(ENUM_RANDOM_OPTION_NUMBER option_number,
                         uint8_t value);

    template <typename PacketType>
    void put_packet_random_option(PacketType& packet) const;

    RandomOptionField option_fields_[3]; // IDA +0x00, +0x03, +0x06
    RandomOptionSeed initial_seed_;      // IDA +0x09
    RandomOptionField changed_option_;   // IDA +0x0a
    RandomOptionSeed changed_option_seed_; // IDA +0x0d
};

static_assert(sizeof(RandomOption) == 14u,
              "RandomOption IDA ABI size must stay 14");
static_assert(offsetof(RandomOption, option_fields_) == 0x00u,
              "RandomOption option fields must retain IDA +0x00");
static_assert(offsetof(RandomOption, initial_seed_) == 0x09u,
              "RandomOption initial seed must retain IDA +0x09");
static_assert(offsetof(RandomOption, changed_option_) == 0x0au,
              "RandomOption changed option must retain IDA +0x0a");
static_assert(offsetof(RandomOption, changed_option_seed_) == 0x0du,
              "RandomOption changed seed must retain IDA +0x0d");

// IDA unit: 08_quest_skill_growth/STEnchantedSkillOption. A declares the
// one-byte public value layout so the 99-owned Chronicle reset can keep the
// original external call boundary; the function body remains owned by 08.
struct STEnchantedSkillOption {
    STEnchantedSkillOption* reset();

    // reset @0x80CB778 operates on the exact original flag byte.
    uint8_t flags_;
};

static_assert(sizeof(STEnchantedSkillOption) == 1u,
              "STEnchantedSkillOption public ABI size must stay 1");
static_assert(offsetof(STEnchantedSkillOption, flags_) == 0x00u,
              "STEnchantedSkillOption flags must retain IDA +0x00");

// IDA unit: 99_misc_global/ChronicleRandomOptionInfo.
struct __attribute__((packed)) ChronicleRandomOptionInfo {
    RandomOptionSeed* reset();

    // ChronicleRandomOptionInfo::reset @0x80CB79E dispatches
    // RandomOption::reset on this exact +0x06 subobject.
    const RandomOption& random_option() const {
        return random_option_;
    }

    STEnchantedSkillOption enchanted_skill_options_[2]; // +0x00/+0x01
    uint32_t value_;
    RandomOption random_option_;
};

static_assert(sizeof(ChronicleRandomOptionInfo) == 20u,
              "ChronicleRandomOptionInfo IDA ABI size must stay 20");
static_assert(offsetof(ChronicleRandomOptionInfo, enchanted_skill_options_) == 0x00u,
              "ChronicleRandomOptionInfo enchant options must retain IDA +0x00");
static_assert(offsetof(ChronicleRandomOptionInfo, value_) == 0x02u,
              "ChronicleRandomOptionInfo dword must retain IDA +0x02");
static_assert(offsetof(ChronicleRandomOptionInfo, random_option_) == 0x06u,
              "ChronicleRandomOptionInfo RandomOption must retain IDA +0x06");

// IDA unit: 99_misc_global/UpgradeSeparateInfo.
struct UpgradeSeparateInfo {
    UpgradeSeparateInfo();

    UpgradeSeparateInfo* reset();
    bool IsTradeRestriction() const;
    int IncUpgradeSeparate();
    int GetUpgradeSeparate() const;
    UpgradeSeparateInfo* SetUpgradeSeparate(uint8_t upgrade);
    UpgradeSeparateInfo* SetUpgradeSeparateAndTradeRestriction(uint8_t value);

    // Bits 0..4 are separate upgrade, bit 5 is trade restriction; the
    // remaining high bits are retained by the original mutators.
    uint8_t bits_;
};

static_assert(sizeof(UpgradeSeparateInfo) == 1u,
              "UpgradeSeparateInfo IDA ABI size must stay 1");
static_assert(offsetof(UpgradeSeparateInfo, bits_) == 0x00u,
              "UpgradeSeparateInfo bits must retain IDA +0x00");

// IDA unit: 99_misc_global/ReservedCapacity.
struct __attribute__((packed)) ReservedCapacity {
    ReservedCapacity();

    ReservedCapacity* reset();

    uint32_t value_0_;
    uint32_t value_1_;
    uint8_t value_2_;
};

static_assert(sizeof(ReservedCapacity) == 9u,
              "ReservedCapacity IDA ABI size must stay 9");
static_assert(offsetof(ReservedCapacity, value_0_) == 0x00u,
              "ReservedCapacity first dword must retain IDA +0x00");
static_assert(offsetof(ReservedCapacity, value_1_) == 0x04u,
              "ReservedCapacity second dword must retain IDA +0x04");
static_assert(offsetof(ReservedCapacity, value_2_) == 0x08u,
              "ReservedCapacity byte must retain IDA +0x08");


#endif  // OLDSRV_GLOBAL_ITEM_OPTION_H_
