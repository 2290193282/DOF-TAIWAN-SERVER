#include "old_game/economy/mail/mail_trade_runtime.h"

#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"

namespace private_store {

// IDA_EA: 0x85CB6C8
// IDA_NAME: _ZN13private_store16PrivateStoreZoneC2Ehh
// DEMANGLED: private_store::PrivateStoreZone::PrivateStoreZone(unsigned char,unsigned char)
PrivateStoreZone::PrivateStoreZone(uint8_t village_value, uint8_t area_value)
    : village(village_value),
      area(area_value) {}

// IDA_EA: 0x85CB6DE
// IDA_NAME: _ZNK13private_store16PrivateStoreZoneltERKS0_
// DEMANGLED: private_store::PrivateStoreZone::operator<(private_store::PrivateStoreZone const&)const
bool PrivateStoreZone::operator<(const PrivateStoreZone& rhs) const {
    return village < rhs.village || (village == rhs.village && area < rhs.area);
}

// IDA_EA: 0x85CB6F6
// IDA_NAME: _ZN13private_store15PrivateStorePosC2Ev
PrivateStorePos::PrivateStorePos()
    : zone(0, 0),
      x(0),
      y(0) {}

// IDA_EA: 0x85CB72C
// IDA_NAME: _ZN13private_store15PrivateStorePosC2Ehhss
PrivateStorePos::PrivateStorePos(uint8_t village_value,
                                 uint8_t area_value,
                                 int16_t x_value,
                                 int16_t y_value)
    : zone(village_value, area_value),
      x(x_value),
      y(y_value) {}

// IDA_EA: 0x85CB784
// IDA_NAME: _ZN13private_store15PrivateStorePos5clearEv
PrivateStorePos* PrivateStorePos::clear() {
    zone.village = 0;
    zone.area = 0;
    x = 0;
    y = 0;
    return this;
}

// IDA_EA: 0x85CB7A8
// IDA_NAME: _ZN13private_store13CErrorHandler3SetEP5CUserii
CErrorHandler* CErrorHandler::Set(CUser* user_value,
                                  int packet_class_value,
                                  int command_value) {
    user = user_value;
    packet_class = packet_class_value;
    command = command_value;
    return this;
}

// IDA_EA: 0x85C4D10
// IDA_NAME: _ZN13private_store13CPrivateStoreC2Ev
CPrivateStore::CPrivateStore()
    : ISeller(),
      IBuyer(),
      position(),
      buyers(),
      items(),
      subject(),
      world_position() {
    // The old C2 first completes all non-trivial member constructors, then
    // applies these scalar stores in this order.
    manager = 0;
    state = 0;
    items.reserve(14);
    PrivateStoreItem initial_item;
    items.resize(14, initial_item);
    subject.reserve(40);
    doll_created = false;
    doll_item_id = 0;
}

// IDA_EA: 0x85C4E70
// IDA_NAME: _ZN13private_store13CPrivateStore5ClearEv
CPrivateStore* CPrivateStore::Clear() {
    seller = 0;
    buyer = 0;
    state = 0;
    position.clear();
    buyers.clear();
    ResetItems();
    doll_created = false;
    doll_item_id = 0;
    return this;
}

// IDA_EA: 0x85C4ECC
// IDA_NAME: _ZN13private_store13CPrivateStoreD2Ev
CPrivateStore::~CPrivateStore() {}

// IDA_EA: 0x85C4C7C
// IDA_NAME: _ZN13private_store13CErrorHandler2DoEi
int CErrorHandler::Do(int error_code) {
    if (!user) return 0;

    CPacketBufPrivate packet;
    packet.MakeErrorReturnPacket(packet_class, command, error_code);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x82345D8
// IDA_NAME: _ZN13private_store13CPrivateStore8GetStateEv
int CPrivateStore::GetState() {
    return state;
}

// IDA_EA: 0x82345E4
// IDA_NAME: _ZN13private_store13CPrivateStore13isDollCreatedEv
int CPrivateStore::isDollCreated() {
    return doll_created;
}

// IDA_EA: 0x85CB87C / 0x85CB888
CUser* CPrivateStore::GetSeller() {
    return seller;
}

const std::string* CPrivateStore::GetSubject() const {
    return &subject;
}

// IDA_EA: 0x85CB894
// IDA_NAME: _ZN13private_store13CPrivateStore14GetSellerIndexEv
// DEMANGLED: private_store::CPrivateStore::GetSellerIndex(void)
uint16_t CPrivateStore::GetSellerIndex() const {
    return seller->get_unique_id();
}

// IDA_EA: 0x85CB8AA / 0x85CB8B8 / 0x85CB8C6
CPrivateStore* CPrivateStore::SetSeller(CUser* user_value) {
    seller = user_value;
    return this;
}

CPrivateStore* CPrivateStore::SetBuyer(CUser* user_value) {
    buyer = user_value;
    return this;
}

CPrivateStore* CPrivateStore::SetParent(CPrivateStoreMgr* manager_value) {
    manager = manager_value;
    return this;
}

// IDA_EA: 0x85CB8D4 / 0x85CB8E0 / 0x85CB8F8 / 0x85CB91C
PrivateStorePos* CPrivateStore::GetPos() {
    return &position;
}

CPrivateStore* CPrivateStore::SetDollCreated(bool value) {
    doll_created = value;
    return this;
}

unsigned long CPrivateStore::GetDollItemID() const {
    return doll_created ? doll_item_id : 0;
}

CPrivateStore* CPrivateStore::SetDollItemID(unsigned long value) {
    if (doll_created) {
        doll_item_id = static_cast<uint32_t>(value);
        return this;
    }
    return 0;
}

// IDA_EA: 0x85C4F52
// IDA_NAME: _ZN13private_store13CPrivateStore10ResetItemsEv
bool CPrivateStore::ResetItems() {
    for (std::vector<PrivateStoreItem>::iterator it = items.begin();
         it != items.end();
         ++it) {
        it->Reset();
    }
    return false;
}

// IDA_EA: 0x85CB7C8
// IDA_NAME: _ZN13private_store16CPrivateStorePosC2Ev
// DEMANGLED: private_store::CPrivateStorePos::CPrivateStorePos(void)
CPrivateStorePos::CPrivateStorePos()
    : village_object::IObject(),
      rectangle(),
      store(0) {}

// IDA_EA: 0x85CB7FC
// IDA_NAME: _ZN13private_store16CPrivateStorePosC2EPNS_13CPrivateStoreEii
// DEMANGLED: private_store::CPrivateStorePos::CPrivateStorePos(private_store::CPrivateStore *,int,int)
CPrivateStorePos::CPrivateStorePos(CPrivateStore* store_value,
                                   int center_x,
                                   int center_y)
    : village_object::IObject(),
      rectangle(),
      store(store_value) {
    rectangle.x = center_x - 80;
    rectangle.width = 160;
    rectangle.y = center_y - 50;
    rectangle.height = 100;
}

// IDA_EA: 0x85CB85C
// IDA_NAME: _ZNK13private_store16CPrivateStorePos7includeERKN14village_object7IObjectE
// DEMANGLED: private_store::CPrivateStorePos::include(village_object::IObject const&)const
bool CPrivateStorePos::include(const village_object::IObject& rhs) const {
    const CPrivateStorePos& rhs_position =
        reinterpret_cast<const CPrivateStorePos&>(rhs);
    return rectangle.include(rhs_position.rectangle);
}

// IDA_EA: 0x85CADF6
// IDA_NAME: _ZN13private_store16CPrivateStorePos15put_object_infoER11PacketGuard
// DEMANGLED: private_store::CPrivateStorePos::put_object_info(PacketGuard &)
bool CPrivateStorePos::put_object_info(PacketGuard& packet) {
    if (!store) {
        cMyTrace trace("virtual bool private_store::CPrivateStorePos::put_object_info(PacketGuard&)",
                       3339,
                       0);
        trace("NOTEXIST_ERROR : private_store(null)");
        return false;
    }

    const int current_state = store->GetState();
    if (current_state != 1 && current_state != 2) {
        cMyTrace trace("virtual bool private_store::CPrivateStorePos::put_object_info(PacketGuard&)",
                       3345,
                       0);
        trace("STATE_ERROR : private_store_state(%d)", current_state);
        return false;
    }

    CUser* const seller_user = store->GetSeller();
    if (!seller_user) {
        cMyTrace trace("virtual bool private_store::CPrivateStorePos::put_object_info(PacketGuard&)",
                       3351,
                       0);
        trace("NOTEXIST_ERROR : private_store(%d, %d) user(null)",
              store->GetPos()->zone.village,
              store->GetPos()->zone.area);
        return false;
    }

    const PrivateStorePos* const position = store->GetPos();
    if (!store->isDollCreated() &&
        (position->zone.village != seller_user->getCurCharacVill() ||
         position->zone.area != seller_user->get_area(false))) {
        cMyTrace trace("virtual bool private_store::CPrivateStorePos::put_object_info(PacketGuard&)",
                       3363,
                       0);
        trace("DIFFERENT_ERROR : private_store(%d,%d), user(%d,%d), uid(%d)",
              position->zone.village,
              position->zone.area,
              seller_user->getCurCharacVill(),
              seller_user->get_area(false),
              seller_user->get_unique_id());
        return false;
    }

    packet.put_byte(1);
    packet.put_short(store->GetSellerIndex());
    const std::string* const subject_value = store->GetSubject();
    packet.put_int(static_cast<int>(subject_value->length()));
    packet.put_str(subject_value->c_str(),
                   static_cast<int>(subject_value->length()));
    packet.put_byte(store->GetState() != 1);
    packet.put_byte(position->zone.village);
    packet.put_byte(position->zone.area);
    packet.put_short(position->x);
    packet.put_short(position->y);
    packet.put_byte(store->isDollCreated() ? 1 : 0);
    return true;
}

// IDA_EA: 0x85CCAE4
// IDA_NAME: _ZN13private_store16CPrivateStorePosaSERKS0_
// DEMANGLED: private_store::CPrivateStorePos::operator=(private_store::CPrivateStorePos const&)
CPrivateStorePos& CPrivateStorePos::operator=(const CPrivateStorePos& rhs) {
    village_object::IObject::operator=(rhs);
    rectangle = rhs.rectangle;
    store = rhs.store;
    return *this;
}

}  // namespace private_store
