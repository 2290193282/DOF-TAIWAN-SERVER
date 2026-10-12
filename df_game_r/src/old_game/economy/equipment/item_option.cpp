#include "old_game/economy/equipment/item_option.h"

#include "common/manager/dnff_lib.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

namespace {

void StoreU32LittleEndian(uint8_t* destination, uint32_t value) {
    destination[0] = static_cast<uint8_t>(value);
    destination[1] = static_cast<uint8_t>(value >> 8);
    destination[2] = static_cast<uint8_t>(value >> 16);
    destination[3] = static_cast<uint8_t>(value >> 24);
}

void StoreU16LittleEndian(uint8_t* destination, uint16_t value) {
    destination[0] = static_cast<uint8_t>(value);
    destination[1] = static_cast<uint8_t>(value >> 8);
}

}  // namespace

// IDA_EA: 0x80CB2B2
// IDA_NAME: _ZN17RandomOptionField5resetEv
// DEMANGLED: RandomOptionField::reset(void)
RandomOptionField* RandomOptionField::reset() {
    option_index_ = 0;
    first_value_ = 0;
    second_value_ = 0;
    return this;
}

// IDA_EA: 0x811ED22
// IDA_NAME: _ZNK17RandomOptionField5emptyEv
// DEMANGLED: RandomOptionField::empty(void)const
bool RandomOptionField::empty() const {
    return option_index_ == 0;
}

// IDA_EA: 0x80CB2CC
// IDA_NAME: _ZN16RandomOptionSeed5resetEv
// DEMANGLED: RandomOptionSeed::reset(void)
RandomOptionSeed* RandomOptionSeed::reset() {
    seed_ = 0;
    return this;
}

// IDA_EA: 0x80CB2D8
// IDA_NAME: _ZN12RandomOption5resetEv
// DEMANGLED: RandomOption::reset(void)
RandomOptionSeed* RandomOption::reset() {
    option_fields_[0].reset();
    option_fields_[1].reset();
    option_fields_[2].reset();
    initial_seed_.reset();
    changed_option_.reset();
    return changed_option_seed_.reset();
}

// IDA_EA: 0x822ABD0
// IDA_NAME: _ZN12RandomOption5emptyEv
// DEMANGLED: RandomOption::empty(void)
bool RandomOption::empty() {
    return option_fields_[0].empty();
}

// IDA_EA: 0x85F4002
// IDA_NAME: _ZN12RandomOption13change_optionE25ENUM_RANDOM_OPTION_NUMBER
bool RandomOption::change_option(ENUM_RANDOM_OPTION_NUMBER option_number) {
    return !changed_option_.empty() &&
           (changed_option_seed_.seed_ & 3u) ==
               static_cast<uint8_t>(option_number);
}

// IDA_EA: 0x85F4040
// IDA_NAME: _ZN12RandomOption16get_option_indexE25ENUM_RANDOM_OPTION_NUMBER
int RandomOption::get_option_index(
    ENUM_RANDOM_OPTION_NUMBER option_number) {
    if (change_option(option_number)) {
        return changed_option_.option_index_;
    }
    return option_fields_[static_cast<int>(option_number)].option_index_;
}

// IDA_EA: 0x85F4078
// IDA_NAME: _ZN12RandomOption15set_first_valueE25ENUM_RANDOM_OPTION_NUMBERh
int RandomOption::set_first_value(ENUM_RANDOM_OPTION_NUMBER option_number,
                                  uint8_t value) {
    if (change_option(option_number)) {
        changed_option_.first_value_ = value;
    }
    option_fields_[static_cast<int>(option_number)].first_value_ = value;
    return value;
}

// IDA_EA: 0x85F40BE
// IDA_NAME: _ZN12RandomOption16set_second_valueE25ENUM_RANDOM_OPTION_NUMBERh
int RandomOption::set_second_value(ENUM_RANDOM_OPTION_NUMBER option_number,
                                   uint8_t value) {
    if (change_option(option_number)) {
        changed_option_.second_value_ = value;
    }
    option_fields_[static_cast<int>(option_number)].second_value_ = value;
    return value;
}

// IDA_EA: 0x811F1AE
// IDA_NAME: _ZNK12RandomOption24put_packet_random_optionI11PacketGuardEEvRT_
// DEMANGLED: void RandomOption::put_packet_random_option<PacketGuard>(PacketGuard &)const
template <>
void RandomOption::put_packet_random_option<PacketGuard>(
    PacketGuard& packet) const {
    int count_index = packet.get_index();
    packet.put_byte(0);

    int count = 0;
    for (int index = 0; index <= 2; ++index) {
        const RandomOptionField& field = option_fields_[index];
        if (field.empty()) {
            continue;
        }
        packet.put_byte(field.option_index_);
        packet.put_byte(field.first_value_);
        packet.put_byte(field.second_value_);
        ++count;
    }
    packet.put_byte(count_index, count);
    if (count == 0) {
        return;
    }
    packet.put_byte(initial_seed_.seed_);
    if (changed_option_.empty()) {
        packet.put_byte(-1);
        return;
    }
    packet.put_byte(changed_option_seed_.seed_ & 3u);
    packet.put_byte(changed_option_seed_.seed_);
    packet.put_byte(changed_option_.option_index_);
    packet.put_byte(changed_option_.first_value_);
    packet.put_byte(changed_option_.second_value_);
}

// IDA_EA: 0x858E3EC
// IDA_NAME: _ZNK12RandomOption24put_packet_random_optionI9PacketBufEEvRT_
// DEMANGLED: void RandomOption::put_packet_random_option<PacketBuf>(PacketBuf &)const
template <>
void RandomOption::put_packet_random_option<PacketBuf>(PacketBuf& packet) const {
    int count_index = packet.get_index();
    packet.put_byte(0);

    int count = 0;
    for (int index = 0; index <= 2; ++index) {
        const RandomOptionField& field = option_fields_[index];
        if (field.empty()) {
            continue;
        }
        packet.put_byte(field.option_index_);
        packet.put_byte(field.first_value_);
        packet.put_byte(field.second_value_);
        ++count;
    }
    packet.put_byte(count_index, count);
    if (count == 0) {
        return;
    }
    packet.put_byte(initial_seed_.seed_);
    if (changed_option_.empty()) {
        packet.put_byte(-1);
        return;
    }
    packet.put_byte(changed_option_seed_.seed_ & 3u);
    packet.put_byte(changed_option_seed_.seed_);
    packet.put_byte(changed_option_.option_index_);
    packet.put_byte(changed_option_.first_value_);
    packet.put_byte(changed_option_.second_value_);
}

// IDA_EA: 0x80CB332
// IDA_NAME: _ZN19UpgradeSeparateInfoC2Ev
// DEMANGLED: UpgradeSeparateInfo::UpgradeSeparateInfo(void)
UpgradeSeparateInfo::UpgradeSeparateInfo() {
    reset();
}

// IDA_EA: 0x80CB346
// IDA_NAME: _ZN19UpgradeSeparateInfo5resetEv
// DEMANGLED: UpgradeSeparateInfo::reset(void)
UpgradeSeparateInfo* UpgradeSeparateInfo::reset() {
    bits_ &= 0xE0u;
    bits_ &= static_cast<uint8_t>(~0x20u);
    bits_ &= 0x3Fu;
    return this;
}

// IDA_EA: 0x80CB36C
// IDA_NAME: _ZN16ReservedCapacityC2Ev
// DEMANGLED: ReservedCapacity::ReservedCapacity(void)
ReservedCapacity::ReservedCapacity() {
    reset();
}

// IDA_EA: 0x80CB380
// IDA_NAME: _ZN16ReservedCapacity5resetEv
// DEMANGLED: ReservedCapacity::reset(void)
ReservedCapacity* ReservedCapacity::reset() {
    value_0_ = 0;
    value_1_ = 0;
    value_2_ = 0;
    return this;
}

// IDA_EA: 0x80CB75C
// IDA_NAME: _ZN17stAmplifyOption_t5resetEv
// DEMANGLED: stAmplifyOption_t::reset(void)
stAmplifyOption_t* stAmplifyOption_t::reset() {
    ability_type_and_identification_ = 0;
    ability_value_ = 0;
    lock_ = 0;
    return this;
}

// IDA_EA: 0x80CB79E
// IDA_NAME: _ZN25ChronicleRandomOptionInfo5resetEv
// DEMANGLED: ChronicleRandomOptionInfo::reset(void)
RandomOptionSeed* ChronicleRandomOptionInfo::reset() {
    enchanted_skill_options_[0].reset();
    enchanted_skill_options_[1].reset();
    value_ = 0;
    return random_option_.reset();
}

// IDA_EA: 0x8150732
// IDA_NAME: _ZNK17stAmplifyOption_t14getAbilityTypeEv
// DEMANGLED: stAmplifyOption_t::getAbilityType(void)const
int stAmplifyOption_t::getAbilityType() const {
    if (!hasAbility()) {
        return 0;
    }
    if (isIdentified() == 1) {
        return ability_type_and_identification_;
    }
    return -128;
}

// IDA_EA: 0x8150772
// IDA_NAME: _ZNK17stAmplifyOption_t15getAbilityValueEv
// DEMANGLED: stAmplifyOption_t::getAbilityValue(void)const
int stAmplifyOption_t::getAbilityValue() const {
    if (!hasAbility() || isIdentified() != 1) {
        return 0;
    }
    return ability_value_;
}

// IDA_EA: 0x81507BC
// IDA_NAME: _ZNK17stAmplifyOption_t12isIdentifiedEv
// DEMANGLED: stAmplifyOption_t::isIdentified(void)const
char stAmplifyOption_t::isIdentified() const {
    return static_cast<char>(
        static_cast<uint8_t>(~ability_type_and_identification_) >> 7);
}

// IDA_EA: 0x81507CC
// IDA_NAME: _ZNK17stAmplifyOption_t10hasAbilityEv
// DEMANGLED: stAmplifyOption_t::hasAbility(void)const
bool stAmplifyOption_t::hasAbility() const {
    return ability_type_and_identification_ != 0;
}

// IDA_EA: 0x8110B0A
// IDA_NAME: _ZNK19UpgradeSeparateInfo18IsTradeRestrictionEv
// DEMANGLED: UpgradeSeparateInfo::IsTradeRestriction(void)const
bool UpgradeSeparateInfo::IsTradeRestriction() const {
    return (bits_ & 0x20u) != 0;
}

// IDA_EA: 0x811ED32
// IDA_NAME: _ZN19UpgradeSeparateInfo18IncUpgradeSeparateEv
// DEMANGLED: UpgradeSeparateInfo::IncUpgradeSeparate(void)
int UpgradeSeparateInfo::IncUpgradeSeparate() {
    const int result = (((bits_ & 0x1Fu) + 1) & 0x1Fu) |
                       (bits_ & 0xE0u);
    bits_ = static_cast<uint8_t>(result);
    return result;
}

// IDA_EA: 0x811ED58
// IDA_NAME: _ZNK19UpgradeSeparateInfo18GetUpgradeSeparateEv
// DEMANGLED: UpgradeSeparateInfo::GetUpgradeSeparate(void)const
int UpgradeSeparateInfo::GetUpgradeSeparate() const {
    return bits_ & 0x1Fu;
}

// IDA_EA: 0x822B3D8
// IDA_NAME: _ZN17stAmplifyOption_t8identifyEv
// DEMANGLED: stAmplifyOption_t::identify(void)
int stAmplifyOption_t::identify() {
    if (isIdentified()) {
        return 0;
    }
    ability_type_and_identification_ &= static_cast<uint8_t>(~0x80u);
    return 1;
}

// IDA_EA: 0x822B40C
// IDA_NAME: _ZN17stAmplifyOption_t11_setAbilityEhtb
// DEMANGLED: stAmplifyOption_t::_setAbility(unsigned char,unsigned short,bool)
stAmplifyOption_t* stAmplifyOption_t::_setAbility(uint8_t ability_type,
                                                   uint16_t ability_value,
                                                   bool identified) {
    ability_type_and_identification_ = ability_type;
    ability_value_ = ability_value;
    stAmplifyOption_t* result = 0;
    if (!identified) {
        result = this;
        ability_type_and_identification_ |= 0x80u;
    }
    return result;
}

// IDA_EA: 0x828B5A8
// IDA_NAME: _ZNK17stAmplifyOption_t7GetLockEv
// DEMANGLED: stAmplifyOption_t::GetLock(void)const
int stAmplifyOption_t::GetLock() const {
    return lock_;
}

// IDA_EA: 0x844D478
// IDA_NAME: _ZN17stAmplifyOption_t7SetLockEh
// DEMANGLED: stAmplifyOption_t::SetLock(unsigned char)
stAmplifyOption_t* stAmplifyOption_t::SetLock(uint8_t lock) {
    lock_ = lock;
    return this;
}

// IDA_EA: 0x844D490
// IDA_NAME: _ZNK17stAmplifyOption_t9getValuesERhRt
// DEMANGLED: stAmplifyOption_t::getValues(unsigned char &,unsigned short &)const
uint16_t* stAmplifyOption_t::getValues(uint8_t& ability_type,
                                       uint16_t& ability_value) const {
    ability_type = ability_type_and_identification_;
    ability_value = ability_value_;
    return &ability_value;
}

// IDA_EA: 0x84B40E4
// IDA_NAME: _ZN17stAmplifyOption_t6assignEht
// DEMANGLED: stAmplifyOption_t::assign(unsigned char,unsigned short)
stAmplifyOption_t* stAmplifyOption_t::assign(uint8_t ability_type,
                                             uint16_t ability_value) {
    ability_type_and_identification_ = ability_type;
    ability_value_ = ability_value;
    return this;
}

// IDA_EA: 0x84E906A
// IDA_NAME: _ZN19UpgradeSeparateInfo18SetUpgradeSeparateEh
// DEMANGLED: UpgradeSeparateInfo::SetUpgradeSeparate(unsigned char)
UpgradeSeparateInfo* UpgradeSeparateInfo::SetUpgradeSeparate(uint8_t upgrade) {
    bits_ = static_cast<uint8_t>((upgrade & 0x1Fu) | (bits_ & 0xE0u));
    return this;
}

// IDA_EA: 0x85139E8
// IDA_NAME: _ZN30itemGloballyUniqueIdentifier_t5resetEv
// DEMANGLED: itemGloballyUniqueIdentifier_t::reset(void)
bool itemGloballyUniqueIdentifier_t::reset() {
    server_id_ = 0;
    datetime_ = 0;
    sequence_no_ = 0;
    // The original byte loop executes its comparison once after byte nine,
    // so the historical return value is false after a successful reset.
    return false;
}

// IDA_EA: 0x85574BC
// IDA_NAME: _ZN19UpgradeSeparateInfo37SetUpgradeSeparateAndTradeRestrictionEh
// DEMANGLED: UpgradeSeparateInfo::SetUpgradeSeparateAndTradeRestriction(unsigned char)
UpgradeSeparateInfo*
UpgradeSeparateInfo::SetUpgradeSeparateAndTradeRestriction(uint8_t value) {
    bits_ = value;
    return this;
}

// IDA_EA: 0x8894782
// IDA_NAME: _ZN30itemGloballyUniqueIdentifier_t11SetServerIdEi
// DEMANGLED: itemGloballyUniqueIdentifier_t::SetServerId(int)
itemGloballyUniqueIdentifier_t*
itemGloballyUniqueIdentifier_t::SetServerId(int server_id) {
    server_id_ = server_id;
    return this;
}

// IDA_EA: 0x8894798
// IDA_NAME: _ZN30itemGloballyUniqueIdentifier_t11SetDatetimeEi
// DEMANGLED: itemGloballyUniqueIdentifier_t::SetDatetime(int)
char* itemGloballyUniqueIdentifier_t::SetDatetime(int datetime) {
    datetime_ = datetime;
    return static_cast<char*>(static_cast<void*>(&datetime_));
}

// IDA_EA: 0x88947B2
// IDA_NAME: _ZN30itemGloballyUniqueIdentifier_t13SetSequenceNoEt
// DEMANGLED: itemGloballyUniqueIdentifier_t::SetSequenceNo(unsigned short)
char* itemGloballyUniqueIdentifier_t::SetSequenceNo(uint16_t sequence_no) {
    sequence_no_ = sequence_no;
    return static_cast<char*>(static_cast<void*>(&sequence_no_));
}

// IDA_EA: 0x84EBE58
// IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintC2Ev
// DEMANGLED: CItemGloballyUniqueIdentifierPrint::CItemGloballyUniqueIdentifierPrint(void)
CItemGloballyUniqueIdentifierPrint::CItemGloballyUniqueIdentifierPrint() {
    std::memset(text_, 0, sizeof(text_));
}

// IDA_EA: 0x84EBE7C
// IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintD2Ev
// DEMANGLED: CItemGloballyUniqueIdentifierPrint::~CItemGloballyUniqueIdentifierPrint()
CItemGloballyUniqueIdentifierPrint::~CItemGloballyUniqueIdentifierPrint() {
}

// IDA_EA: 0x84EBE82
// IDA_NAME: _ZN34CItemGloballyUniqueIdentifierPrintclERK30itemGloballyUniqueIdentifier_t
// DEMANGLED: CItemGloballyUniqueIdentifierPrint::operator()(itemGloballyUniqueIdentifier_t const&)
char* CItemGloballyUniqueIdentifierPrint::operator()(
    const itemGloballyUniqueIdentifier_t& identifier) {
    uint8_t identifier_bytes[10];
    StoreU32LittleEndian(identifier_bytes,
                          static_cast<uint32_t>(identifier.server_id_));
    StoreU32LittleEndian(identifier_bytes + 4,
                          static_cast<uint32_t>(identifier.datetime_));
    StoreU16LittleEndian(identifier_bytes + 8, identifier.sequence_no_);
    DNFFLib::Binary2Hex(identifier_bytes, 0xA, text_);
    return text_;
}
