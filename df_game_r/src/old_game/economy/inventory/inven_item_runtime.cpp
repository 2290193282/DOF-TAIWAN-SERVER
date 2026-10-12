#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x80CB7D8
// IDA_NAME: _ZN10Inven_Item5resetEv
// DEMANGLED: Inven_Item::reset(void)
Inven_Item* Inven_Item::reset() {
    seal_or_state_ = 0;
    item_category_ = 0;
    item_attr_ = 0;
    item_id_ = 0;
    add_info_ = 0;
    avatar_select_ = 0;
    extend_info_ = 0;
    amplify_option_.reset();
    chronicle_random_option_.reset();
    upgrade_separate_.reset();
    reserved_capacity_.reset();
    // IDA deliberately does not clear globally_unique_id_ at +0x15.
    return this;
}

// IDA_EA: 0x80CB854
// IDA_NAME: _ZN10Inven_ItemC2Ev
// DEMANGLED: Inven_Item::Inven_Item(void)
Inven_Item::Inven_Item() {
    // C++ constructs UpgradeSeparateInfo (+0x33) and ReservedCapacity (+0x34)
    // in declaration order before this body, exactly matching 0x80CB854.
    // All preceding subobjects are trivial and have no original C2 call.
    reset();
}

// IDA_EA: 0x80CB884
// IDA_NAME: _ZN10Inven_Item12set_add_infoEi
// DEMANGLED: Inven_Item::set_add_info(int)
Inven_Item* Inven_Item::set_add_info(int add_info) {
    add_info_ = add_info;
    return this;
}

// IDA_EA: 0x80CB892
// IDA_NAME: _ZN10Inven_Item13ResetItemAttrEv
// DEMANGLED: Inven_Item::ResetItemAttr(void)
Inven_Item* Inven_Item::ResetItemAttr() {
    item_attr_ = 0;
    return this;
}

// IDA_EA: 0x80F0FFC
// IDA_NAME: _ZN10Inven_Item10SetUpgradeEh
// DEMANGLED: Inven_Item::SetUpgrade(unsigned char)
Inven_Item* Inven_Item::SetUpgrade(uint8_t upgrade) {
    item_attr_ = static_cast<uint8_t>((upgrade & 0x1fu) |
                                              (item_attr_ & 0xe0u));
    return this;
}

// IDA_EA: 0x80F506C
// IDA_NAME: _ZNK10Inven_Item10GetUpgradeEv
// DEMANGLED: Inven_Item::GetUpgrade(void)const
int Inven_Item::GetUpgrade() const {
    return item_attr_ & 0x1fu;
}

// IDA_EA: 0x80F783A
// IDA_NAME: _ZNK10Inven_Item12get_add_infoEv
// DEMANGLED: Inven_Item::get_add_info(void)const
int Inven_Item::get_add_info() const {
    return add_info_;
}

// IDA_EA: 0x811ED66
// IDA_NAME: _ZNK10Inven_Item7isEmptyEv
// DEMANGLED: Inven_Item::isEmpty(void)const
bool Inven_Item::isEmpty() const {
    return item_id_ == 0;
}

// IDA_EA: 0x81203F6
// IDA_NAME: _ZN10Inven_Item18SetTradeLimitCountEh
// DEMANGLED: Inven_Item::SetTradeLimitCount(unsigned char)
Inven_Item* Inven_Item::SetTradeLimitCount(uint8_t count) {
    item_attr_ = static_cast<uint8_t>((32u * (count & 7u)) |
                                              (item_attr_ & 0x1fu));
    return this;
}

// IDA_EA: 0x8120422
// IDA_NAME: _ZNK10Inven_Item18GetTradeLimitCountEv
// DEMANGLED: Inven_Item::GetTradeLimitCount(void)const
char Inven_Item::GetTradeLimitCount() const {
    return static_cast<char>(item_attr_ >> 5);
}

// IDA_EA: 0x814A62E
// IDA_NAME: _ZN10Inven_Item7setCopyERKS_
// DEMANGLED: Inven_Item::setCopy(Inven_Item const&)
Inven_Item* Inven_Item::setCopy(const Inven_Item& rhs) {
    seal_or_state_ = rhs.seal_or_state_;
    item_category_ = rhs.item_category_;
    item_id_ = rhs.item_id_;
    add_info_ = rhs.add_info_;
    avatar_select_ = rhs.avatar_select_;
    item_attr_ = rhs.item_attr_;
    extend_info_ = rhs.extend_info_;
    amplify_option_ = rhs.amplify_option_;
    chronicle_random_option_ = rhs.chronicle_random_option_;
    upgrade_separate_ = rhs.upgrade_separate_;
    reserved_capacity_ = rhs.reserved_capacity_;
    // 0x814A62E intentionally omits the 10-byte GUID at +0x15.
    return this;
}

// IDA_EA: 0x81507DC
// IDA_NAME: _ZNK10Inven_Item11GetItemAttrEv
// DEMANGLED: Inven_Item::GetItemAttr(void)const
int Inven_Item::GetItemAttr() const {
    return item_attr_;
}

// IDA_EA: 0x81507E8
// IDA_NAME: _ZN10Inven_Item11SetItemAttrEh
// DEMANGLED: Inven_Item::SetItemAttr(unsigned char)
Inven_Item* Inven_Item::SetItemAttr(uint8_t attr) {
    item_attr_ = attr;
    return this;
}

// IDA_EA: 0x8150800
// IDA_NAME: _ZNK10Inven_Item16isAvatarItemTypeEv
// DEMANGLED: Inven_Item::isAvatarItemType(void)const
bool Inven_Item::isAvatarItemType() const {
    return item_category_ == 8;
}

// IDA_EA: 0x8150812
// IDA_NAME: _ZNK10Inven_Item19isEquipableItemTypeEv
// DEMANGLED: Inven_Item::isEquipableItemType(void)const
bool Inven_Item::isEquipableItemType() const {
    return item_category_ == 1 || item_category_ == 5 ||
           item_category_ == 6 || item_category_ == 8;
}

// IDA_EA: 0x822B456
// IDA_NAME: _ZNK10Inven_Item14GetReSealCountEv
// DEMANGLED: Inven_Item::GetReSealCount(void)const
char Inven_Item::GetReSealCount() const {
    return static_cast<char>(item_attr_ >> 5);
}

// IDA_EA: 0x822B466
// IDA_NAME: _ZN10Inven_Item14SetReSealCountEh
// DEMANGLED: Inven_Item::SetReSealCount(unsigned char)
Inven_Item* Inven_Item::SetReSealCount(uint8_t count) {
    item_attr_ = static_cast<uint8_t>((32u * (count & 7u)) |
                                              (item_attr_ & 0x1fu));
    return this;
}

// IDA_EA: 0x8335B76
// IDA_NAME: _ZNK10Inven_Item18IsCreatureItemTypeEv
// DEMANGLED: Inven_Item::IsCreatureItemType(void)const
bool Inven_Item::IsCreatureItemType() const {
    return item_category_ == 5 || item_category_ == 7 ||
           item_category_ == 6;
}

// IDA_EA: 0x8348170
// IDA_NAME: _ZN10Inven_Item16GetInventoryTypeEv
// DEMANGLED: Inven_Item::GetInventoryType(void)
int Inven_Item::GetInventoryType() {
    const unsigned int item_type = item_category_;
    if (item_type > 0x0au) return 1;
    const int mask = 1 << item_type;
    if ((mask & 0x61f) != 0) return 1;
    if ((mask & 0x100) != 0) return 2;
    if ((mask & 0x0e0) == 0) return 1;
    return 3;
}

// IDA_EA: 0x83481E2
// IDA_NAME: _ZNK10Inven_Item12GetItemSpaceEv
// DEMANGLED: Inven_Item::GetItemSpace(void)const
int Inven_Item::GetItemSpace() const {
    const unsigned int item_type = item_category_;
    if (item_type > 0x0au) return 0;
    const int mask = 1 << item_type;
    if ((mask & 0x61f) != 0) return 0;
    if ((mask & 0x100) != 0) return 1;
    if ((mask & 0x0e0) == 0) return 0;
    return 7;
}

// IDA_EA: 0x850D102
// IDA_NAME: _ZNK10Inven_Item14getPackedCode1Ev
// DEMANGLED: Inven_Item::getPackedCode1(void)const
unsigned int Inven_Item::getPackedCode1() const {
    return (static_cast<uint32_t>(add_info_) & 0xfff00000u) |
           (16u * static_cast<uint32_t>(item_id_)) |
           (2u * (item_category_ & 7u)) |
           (seal_or_state_ != 0);
}

// IDA_EA: 0x850D14E
// IDA_NAME: _ZNK10Inven_Item6getKeyEv
// DEMANGLED: Inven_Item::getKey(void)const
int Inven_Item::getKey() const {
    return item_id_;
}

// IDA_EA: 0x854B4BE
// IDA_NAME: _ZN10Inven_Item10IncUpgradeEv
// DEMANGLED: Inven_Item::IncUpgrade(void)
int Inven_Item::IncUpgrade() {
    const int result = ((item_attr_ & 0x1f) + 1) & 0x1f |
                       (item_attr_ & 0xe0);
    item_attr_ = static_cast<uint8_t>(result);
    return result;
}

// IDA_EA: 0x833F38E
// IDA_NAME: _ZN21Inven_Item_Compatible7setCopyERK10Inven_Item
// DEMANGLED: Inven_Item_Compatible::setCopy(Inven_Item const&)
Inven_Item_Compatible*
Inven_Item_Compatible::setCopy(const Inven_Item& item) {
    seal_or_state_ = item.seal_or_state_;
    item_category_ = item.item_category_;
    item_id_ = item.item_id_;
    add_info_ = item.add_info_;
    avatar_select_ = item.avatar_select_;
    item_attr_ = static_cast<uint8_t>(item.GetItemAttr());
    extend_info_ = item.extend_info_;
    globally_unique_id_ = item.globally_unique_id_;
    // 0x833F38E copies the ten-byte GUID before the four-byte amplify
    // subobject.  Preserve that observable member-copy order.
    amplify_option_ = item.amplify_option_;
    return this;
}

// IDA_EA: 0x833F418
// IDA_NAME: _ZN21Inven_Item_Compatible13makeInvenItemER10Inven_Item
// DEMANGLED: Inven_Item_Compatible::makeInvenItem(Inven_Item &)
Inven_Item* Inven_Item_Compatible::makeInvenItem(Inven_Item& item) {
    item.seal_or_state_ = seal_or_state_;
    item.item_category_ = item_category_;
    item.item_id_ = item_id_;
    item.add_info_ = add_info_;
    item.avatar_select_ = avatar_select_;
    item.SetItemAttr(item_attr_);
    item.extend_info_ = extend_info_;
    item.globally_unique_id_ = globally_unique_id_;
    // 0x833F418 restores GUID before the amplify subobject.
    item.amplify_option_ = amplify_option_;
    return &item;
}
