#include "old_game/appearance/pet/creature.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

namespace user_creature {


namespace {

void FreeStream(Stream* stream) {
    switch (stream->pool_kind()) {
        case 0:
            GlobalData::stream_pool()->Free(stream);
            break;
        case 1:
            GlobalData::small_stream_pool()->Free(stream);
            break;
        case 2:
            GlobalData::big_stream_pool()->Free(stream);
            break;
    }
}

void CopyInvenItem(SIG_CREATURE_ITEM_COMPATIBLE_DATA& destination,
                   const Inven_Item* item) {
    destination.item_flag = item->seal_or_state_;
    destination.item_type = item->item_category_;
    destination.item_id = static_cast<uint32_t>(item->item_id_);
    destination.item_attr = item->item_attr_;
    destination.add_info = item->add_info_;
    destination.endurance = item->avatar_select_;
    destination.expire_date = item->extend_info_;
    destination.amplify_option = item->amplify_option_;
    destination.identifier = item->globally_unique_id_;
}

void PrepareCreatureItem(SIG_CREATURE_ITEM* payload) {
    std::memset(payload, 0, sizeof(*payload));
}

}  // namespace

// IDA_EA: 0x833F7A8
// IDA_NAME: _ZN13user_creature12CDBMsgSender4InitE19ENUM_INTERNALPACKETP5CUser
// DEMANGLED: user_creature::CDBMsgSender::Init(ENUM_INTERNALPACKET,CUser *)
int CDBMsgSender::Init(ENUM_INTERNALPACKET signal, CUser* user) {
    stream = signal == static_cast<ENUM_INTERNALPACKET>(137)
        ? GlobalData::big_stream_pool()->Acquire("Creature.cpp", 167)
        : GlobalData::stream_pool()->Acquire("Creature.cpp", 169);
    if (!stream) {
        return 0;
    }
    this->user = user;
    CStreamGuard guard(stream, false);
    *guard << signal;
    *guard << user->GetUID();
    return 1;
}

// IDA_EA: 0x833F8AE
// IDA_NAME: _ZN13user_creature12CDBMsgSenderC2E19ENUM_INTERNALPACKETP5CUser
// DEMANGLED: user_creature::CDBMsgSender::CDBMsgSender(ENUM_INTERNALPACKET,CUser *)
CDBMsgSender::CDBMsgSender(ENUM_INTERNALPACKET signal, CUser* user) {
    stream = 0;
    Init(signal, user);
}

// IDA_EA: 0x833F8D8
// IDA_NAME: _ZN13user_creature12CDBMsgSenderD2Ev
// DEMANGLED: user_creature::CDBMsgSender::~CDBMsgSender(void)
CDBMsgSender::~CDBMsgSender() {
    if (stream) {
        FreeStream(stream);
        stream = 0;
    }
}

// IDA_EA: 0x833F950
// IDA_NAME: _ZN13user_creature12CDBMsgSender4SendEv
// DEMANGLED: user_creature::CDBMsgSender::Send(void)
int CDBMsgSender::Send() {
    if (!stream) {
        return 0;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, stream);
    stream = 0;
    return 1;
}

// IDA_EA: 0x833F994
// IDA_NAME: _ZN13user_creature12CDBMsgSender6CancelEv
// DEMANGLED: user_creature::CDBMsgSender::Cancel(void)
CDBMsgSender* CDBMsgSender::Cancel() {
    if (stream) {
        FreeStream(stream);
        stream = 0;
    }
    return this;
}

// IDA_EA: 0x83425F0
// IDA_NAME: _ZN13user_creature12CDBMsgSender11GetInBufferI22SIG_SWAP_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_SWAP_CREATURE_ITEM * user_creature::CDBMsgSender::GetInBuffer<SIG_SWAP_CREATURE_ITEM>(void)
template <>
SIG_SWAP_CREATURE_ITEM*
CDBMsgSender::GetInBuffer<SIG_SWAP_CREATURE_ITEM>() {
    if (!stream) {
        return 0;
    }
    CStreamGuard guard(stream, false);
    return guard.GetInBuffer<SIG_SWAP_CREATURE_ITEM>();
}

// IDA_EA: 0x834266A
// IDA_NAME: _ZN13user_creature12CDBMsgSender11GetInBufferI17SIG_CREATURE_ITEMEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM * user_creature::CDBMsgSender::GetInBuffer<SIG_CREATURE_ITEM>(void)
template <>
SIG_CREATURE_ITEM* CDBMsgSender::GetInBuffer<SIG_CREATURE_ITEM>() {
    if (!stream) {
        return 0;
    }
    CStreamGuard guard(stream, false);
    return guard.GetInBuffer<SIG_CREATURE_ITEM>();
}

// IDA_EA: 0x8342B94
// IDA_NAME: _ZN13user_creature12CDBMsgSender11GetInBufferI22SIG_CREATURE_ITEM_LISTEEPT_v
// DEMANGLED: SIG_CREATURE_ITEM_LIST * user_creature::CDBMsgSender::GetInBuffer<SIG_CREATURE_ITEM_LIST>(void)
template <>
SIG_CREATURE_ITEM_LIST*
CDBMsgSender::GetInBuffer<SIG_CREATURE_ITEM_LIST>() {
    if (!stream) {
        return 0;
    }
    CStreamGuard guard(stream, false);
    return guard.GetInBuffer<SIG_CREATURE_ITEM_LIST>();
}

// IDA_EA: 0x833FA0C
// IDA_NAME: _ZN13user_creature12CDBMsgSender16SwapCreatureItemEP5CUserP10Inven_ItemiS4_i
// DEMANGLED: user_creature::CDBMsgSender::SwapCreatureItem(CUser *,Inven_Item *,int,Inven_Item *,int)
int CDBMsgSender::SwapCreatureItem(CUser* user,
                                   Inven_Item* first,
                                   int second_slot,
                                   Inven_Item* second,
                                   int first_slot) {
    (void)user;
    if (!stream) {
        return 0;
    }
    SIG_SWAP_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_SWAP_CREATURE_ITEM>();
    std::memset(payload, 0, sizeof(*payload));
    payload->second_slot = static_cast<unsigned int>(second_slot);
    payload->first_ui_id = static_cast<unsigned int>(
        first->get_add_info());
    payload->first_empty = first->item_id_ == 0 ? 1 : 0;
    payload->first_slot = static_cast<unsigned int>(first_slot);
    payload->second_ui_id = static_cast<unsigned int>(
        second->get_add_info());
    payload->second_empty = second->item_id_ == 0 ? 1 : 0;
    return 1;
}

// IDA_EA: 0x833FAAA
// IDA_NAME: _ZN13user_creature12CDBMsgSender18InsertCreatureItemEP5CUserP10Inven_Itemiiiii
// DEMANGLED: user_creature::CDBMsgSender::InsertCreatureItem(CUser *,Inven_Item *,int,int,int,int,int)
int CDBMsgSender::InsertCreatureItem(CUser* user,
                                     Inven_Item* item,
                                     int slot,
                                     int creature_flag,
                                     int item_count,
                                     int duration_days,
                                     int delete_tick) {
    if (!stream) {
        return 0;
    }

    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    if (!payload) {
        return 0;
    }
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.slot = slot;
    payload->detail.charac_no =
        static_cast<unsigned int>(user->getCurCharacNo());
    payload->detail.experience = 100;
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.expiration_value = duration_days;
    payload->detail.delete_tick = delete_tick;
    payload->current_creature_flag = static_cast<unsigned int>(item_count);
    payload->detail.cera_order_no =
        static_cast<unsigned int>(user->get_cera_item_order_no());

    const int item_id = item->item_id_;
    CItem* const template_item = G_CDataManager()->find_item(item_id);
    if (!template_item || template_item->is_stackable() != 0) {
        return 0;
    }
    if (reinterpret_cast<const CEquipItem*>(template_item)->get_sub_type() !=
        0) {
        payload->detail.creature_type = 0;
    } else {
        payload->detail.creature_type = 1;
        payload->update_kind = 6;
    }
    return 1;
}

// IDA_EA: 0x833FBDE
// IDA_NAME: _ZN13user_creature12CDBMsgSender18RenameCreatureItemEiPKc
// DEMANGLED: user_creature::CDBMsgSender::RenameCreatureItem(int,char const*)
int CDBMsgSender::RenameCreatureItem(int ui_id, const char* name) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    payload->update_mask |= 1u;
    payload->detail.ui_id = ui_id;
    std::strncpy(payload->detail.name, name, 12);
    return 1;
}

// IDA_EA: 0x833FC5A
// IDA_NAME: _ZN13user_creature12CDBMsgSender15UseFeedCreatureEii
// DEMANGLED: user_creature::CDBMsgSender::UseFeedCreature(int,int)
int CDBMsgSender::UseFeedCreature(int ui_id, int stomach) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    payload->update_mask |= 2u;
    payload->detail.ui_id = ui_id;
    // IDA 0x833FC5A writes s[17] (SIG_CREATURE_ITEM +0x44), which is
    // detail.stomach.  DB_UpdateCreatureItem maps update-mask bit 0x2 to
    // creature_items.stomach.
    payload->detail.stomach = stomach;
    return 1;
}

// IDA_EA: 0x833FCC2
// IDA_NAME: _ZN13user_creature12CDBMsgSender13HatchCreatureEiii
// DEMANGLED: user_creature::CDBMsgSender::HatchCreature(int,int,int)
int CDBMsgSender::HatchCreature(int ui_id, int creature_type, int item_id) {
    if (!stream) {
        return 0;
    }

    SIG_CREATURE_ITEM* const payload = GetInBuffer<SIG_CREATURE_ITEM>();
    std::memset(payload, 0, sizeof(*payload));
    payload->update_mask |= 0x244u;
    payload->detail.ui_id = ui_id;
    payload->detail.creature_type = static_cast<uint8_t>(creature_type);
    payload->inven_item_copy.item_id = static_cast<uint32_t>(item_id);

    CItem* const item = G_CDataManager()->find_item(item_id);
    payload->detail.expiration_value = item->getUsablePeriod();
    payload->detail.delete_tick = item->getExpirationDate();
    return 1;
}

// IDA_EA: 0x833FD74
// IDA_NAME: _ZN13user_creature12CDBMsgSender15EvoluteCreatureEii
// DEMANGLED: user_creature::CDBMsgSender::EvoluteCreature(int,int)
int CDBMsgSender::EvoluteCreature(int ui_id, int item_id) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    payload->update_mask |= 0x40u;
    payload->detail.ui_id = ui_id;
    payload->inven_item_copy.item_id = static_cast<uint32_t>(item_id);
    return 1;
}

// IDA_EA: 0x833FDDC
// IDA_NAME: _ZN13user_creature12CDBMsgSender20EventEvoluteCreatureEiii
// DEMANGLED: user_creature::CDBMsgSender::EventEvoluteCreature(int,int,int)
int CDBMsgSender::EventEvoluteCreature(int ui_id,
                                       int item_id,
                                       int experience) {
    if (!stream) {
        return 0;
    }

    SIG_CREATURE_ITEM* const payload = GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    payload->update_mask |= 0x248u;
    payload->detail.ui_id = ui_id;
    payload->inven_item_copy.item_id = static_cast<uint32_t>(item_id);
    payload->detail.experience = experience;

    CItem* const item = G_CDataManager()->find_item(item_id);
    payload->detail.expiration_value = item->getUsablePeriod();
    payload->detail.delete_tick = item->getExpirationDate();
    return 1;
}

// IDA_EA: 0x833FE8C
// IDA_NAME: _ZN13user_creature12CDBMsgSender8SendMailEP10Inven_Itemiiii
// DEMANGLED: user_creature::CDBMsgSender::SendMail(Inven_Item *,int,int,int,int)
int CDBMsgSender::SendMail(Inven_Item* item,
                           int charac_no,
                           int slot,
                           int creature_flag,
                           int value) {
    (void)slot;
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.ui_id = item->get_add_info();
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.charac_no = static_cast<unsigned int>(charac_no);
    payload->update_mask |= 0x100u;
    payload->detail.state = 1;
    payload->update_kind = 3;
    payload->current_creature_flag = static_cast<unsigned int>(value);
    return 1;
}

// IDA_EA: 0x833FF34
// IDA_NAME: _ZN13user_creature12CDBMsgSender8RecvMailEP10Inven_Itemiiii
// DEMANGLED: user_creature::CDBMsgSender::RecvMail(Inven_Item *,int,int,int,int)
int CDBMsgSender::RecvMail(Inven_Item* item,
                           int charac_no,
                           int slot,
                           int creature_flag,
                           int value) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.ui_id = item->get_add_info();
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.charac_no = static_cast<unsigned int>(charac_no);
    payload->update_mask |= 0x130u;
    payload->detail.state = 0;
    payload->detail.slot = slot;
    payload->update_kind = 4;
    payload->current_creature_flag = static_cast<unsigned int>(value);
    return 1;
}

// IDA_EA: 0x833FFE8
// IDA_NAME: _ZN13user_creature12CDBMsgSender11ChangeOwnerEP10Inven_Itemiiiiii
// DEMANGLED: user_creature::CDBMsgSender::ChangeOwner(Inven_Item *,int,int,int,int,int,int)
int CDBMsgSender::ChangeOwner(Inven_Item* item,
                              int charac_no,
                              int slot,
                              int creature_flag,
                              int value,
                              int old_charac_no,
                              int old_creature_flag) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.ui_id = item->get_add_info();
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.charac_no = static_cast<unsigned int>(charac_no);
    payload->update_mask |= 0x30u;
    payload->detail.slot = slot;
    payload->update_kind = 1;
    payload->current_creature_flag = static_cast<unsigned int>(value);
    payload->previous_charac_no = static_cast<unsigned int>(old_charac_no);
    payload->previous_creature_flag = static_cast<unsigned int>(old_creature_flag);
    return 1;
}

// IDA_EA: 0x83400A4
// IDA_NAME: _ZN13user_creature12CDBMsgSender12ChangeSlotNoEP10Inven_Itemiii
// DEMANGLED: user_creature::CDBMsgSender::ChangeSlotNo(Inven_Item *,int,int,int)
int CDBMsgSender::ChangeSlotNo(Inven_Item* item,
                               int charac_no,
                               int empty_slot,
                               int creature_flag) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.ui_id = item->get_add_info();
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.charac_no = static_cast<unsigned int>(charac_no);
    payload->update_mask |= 0x20u;
    payload->detail.slot = empty_slot;
    payload->update_kind = 2;
    return 1;
}

// IDA_EA: 0x8340144
// IDA_NAME: _ZN13user_creature12CDBMsgSender13AuctionRegistEP10Inven_Itemiiii
// DEMANGLED: user_creature::CDBMsgSender::AuctionRegist(Inven_Item *,int,int,int,int)
int CDBMsgSender::AuctionRegist(Inven_Item* item,
                                int charac_no,
                                int slot,
                                int creature_flag,
                                int value) {
    (void)slot;
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    CopyInvenItem(payload->inven_item_copy, item);
    payload->detail.ui_id = item->get_add_info();
    payload->detail.creature_flag = static_cast<uint8_t>(creature_flag);
    payload->detail.charac_no = static_cast<unsigned int>(charac_no);
    payload->update_mask |= 0x100u;
    payload->detail.state = 1;
    payload->update_kind = 5;
    payload->current_creature_flag = static_cast<unsigned int>(value);
    return 1;
}

// IDA_EA: 0x83401EC
// IDA_NAME: _ZN13user_creature12CDBMsgSender12UseKeepRightEii
// DEMANGLED: user_creature::CDBMsgSender::UseKeepRight(int,int)
int CDBMsgSender::UseKeepRight(int ui_id, int no_charge) {
    if (!stream) {
        return 0;
    }
    SIG_CREATURE_ITEM* const payload =
        GetInBuffer<SIG_CREATURE_ITEM>();
    PrepareCreatureItem(payload);
    payload->update_mask |= 0x80u;
    payload->detail.ui_id = ui_id;
    payload->detail.no_charge = static_cast<uint8_t>(no_charge);
    return 1;
}

}  // namespace user_creature
