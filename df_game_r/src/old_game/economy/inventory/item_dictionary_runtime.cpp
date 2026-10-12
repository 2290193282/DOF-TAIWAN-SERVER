#include "old_game/economy/inventory/item_dictionary_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <map>

namespace {

const unsigned int kDictionaryByteCount = 5000u;

}  // namespace

// IDA_EA: 0x811D5D4
// IDA_NAME: _ZN15CItemDictionaryC2Ev
// DEMANGLED: CItemDictionary::CItemDictionary(void)
CItemDictionary::CItemDictionary() {}

// IDA_EA: 0x811D5F0
// IDA_NAME: _ZN15CItemDictionaryD2Ev
// DEMANGLED: CItemDictionary::~CItemDictionary()
// IDA_EA: 0x811D620
// IDA_NAME: _ZN15CItemDictionaryD0Ev
// DEMANGLED: CItemDictionary::~CItemDictionary()
CItemDictionary::~CItemDictionary() {}

// IDA_EA: 0x811D63E
// IDA_NAME: _ZN15CItemDictionary9_saveDataEP5CUser
// DEMANGLED: CItemDictionary::_saveData(CUser *)
int CItemDictionary::_saveData(CUser* user) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "ItemDictionary.cpp", 24);
    CStreamGuard guard(stream, true);
    *guard << 0x2B2;
    *guard << user->GetUID();

    SIG_ITEM_DICTIONARY_FLAG* payload =
        guard->GetInBuffer<SIG_ITEM_DICTIONARY_FLAG>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    std::memcpy(payload->flags,
                check_flags_,
                sizeof(check_flags_));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x811D774
// IDA_NAME: _ZN15CItemDictionary6_resetEv
// DEMANGLED: CItemDictionary::_reset(void)
int CItemDictionary::_reset() {
    return reinterpret_cast<int>(
        std::memset(check_flags_, 0, sizeof(check_flags_)));
}

// IDA_EA: 0x811D79A
// IDA_NAME: _ZN15CItemDictionary8loadDataEP5CUserPc
// DEMANGLED: CItemDictionary::loadData(CUser *,char *)
bool CItemDictionary::loadData(CUser* user, char* data) {
    (void)user;
    _reset();
    const SIG_ITEM_DICTIONARY_FLAG* payload =
        reinterpret_cast<const SIG_ITEM_DICTIONARY_FLAG*>(data);
    std::memcpy(check_flags_,
                payload->flags,
                sizeof(check_flags_));
    return 1;
}

// IDA_EA: 0x811D7E0
// IDA_NAME: _ZNK15CItemDictionary7getDataEPc
// DEMANGLED: CItemDictionary::getData(char *)const
void CItemDictionary::getData(char* data) const {
    SIG_ITEM_DICTIONARY_FLAG* payload =
        reinterpret_cast<SIG_ITEM_DICTIONARY_FLAG*>(data);
    std::memcpy(payload->flags,
                check_flags_,
                sizeof(check_flags_));
}

// IDA_EA: 0x811D80E
// IDA_NAME: _ZN15CItemDictionary15setGainItemFlagEP5CUserj
// DEMANGLED: CItemDictionary::setGainItemFlag(CUser *,unsigned int)
void CItemDictionary::setGainItemFlag(CUser* user, unsigned int item_id) {
    if (!user) {
        return;
    }

    CItem* item = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!item || item->is_stackable()) {
        return;
    }

    const std::map<int, int>& table =
        G_CDataManager()->get_item_dictionary_item_to_flag_map();
    std::map<int, int>::const_iterator found =
        table.find(static_cast<int>(item_id));
    if (found != table.end()) {
        setItemCheckFlag(user,
                         item_id,
                         static_cast<unsigned int>(found->second));
    }
}

// IDA_EA: 0x811D8FC
// IDA_NAME: _ZN15CItemDictionary16setItemCheckFlagEP5CUserjj
// DEMANGLED: CItemDictionary::setItemCheckFlag(CUser *,unsigned int,unsigned int)
void CItemDictionary::setItemCheckFlag(CUser* user,
                                       unsigned int item_id,
                                       unsigned int item_flag) {
    if (!user) {
        return;
    }

    const unsigned int index = item_flag >> 3;
    const unsigned int bit = item_flag & 7u;
    if (index <= 0x1387u && bit <= 7u) {
        const unsigned int charac_no =
            static_cast<unsigned int>(user->get_charac_no(-1));
        if (getItemCheckFlag(charac_no, item_flag) != true) {
            check_flags_[index] += static_cast<uint8_t>(1u << bit);
            sendItemDictionaryInfo(user, item_id);
            alter();
        }
    } else {
        LogManager::logFormat(
            1,
            "ItemDictionary.cpp",
            "void CItemDictionary::setItemCheckFlag(CUser*, unsigned int, unsigned int)",
            0x62,
            "setItemCheckFlag() characNo=%u, item_flag=%u, index=%d, flag=%d",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            item_flag,
            index,
            bit);
    }
}

// IDA_EA: 0x811DA32
// IDA_NAME: _ZNK15CItemDictionary16getItemCheckFlagEjj
// DEMANGLED: CItemDictionary::getItemCheckFlag(unsigned int,unsigned int)const
bool CItemDictionary::getItemCheckFlag(unsigned int charac_no,
                                       unsigned int item_flag) const {
    const unsigned int index = item_flag >> 3;
    const unsigned int bit = item_flag & 7u;
    if (index <= 0x1387u && bit <= 7u) {
        return ((check_flags_[index] >> bit) & 1u) != 0u;
    }

    LogManager::logFormat(
        1,
        "ItemDictionary.cpp",
        "bool CItemDictionary::getItemCheckFlag(unsigned int, unsigned int) const",
        0x7D,
        "getItemCheckFlag() characNo=%u, item_flag=%u, index=%d, flag=%d",
        charac_no,
        item_flag,
        index,
        bit);
    return true;
}

// IDA_EA: 0x811DAEE
// IDA_NAME: _ZNK15CItemDictionary16getGainItemCountEv
// DEMANGLED: CItemDictionary::getGainItemCount(void)const
int CItemDictionary::getGainItemCount() const {
    int count = 0;
    for (unsigned int index = 0; index <= 4999u; ++index) {
        for (unsigned int bit = 0; bit <= 7u; ++bit) {
            if (((check_flags_[index] >> bit) & 1u) != 0u) {
                ++count;
            }
        }
    }
    return count;
}

// IDA_EA: 0x811DB60
// IDA_NAME: _ZNK15CItemDictionary15getLengthBinaryEv
// DEMANGLED: CItemDictionary::getLengthBinary(void)const
int CItemDictionary::getLengthBinary() const {
    int length = 0;
    for (unsigned int index = 0; index <= 4999u; ++index) {
        if (check_flags_[index]) {
            length = static_cast<int>(index + 1u);
        }
    }
    return length;
}

// IDA_EA: 0x811DBA6
// IDA_NAME: _ZN15CItemDictionary15getGainItemListEv
// DEMANGLED: CItemDictionary::getGainItemList(void)
std::vector<int> CItemDictionary::getGainItemList() {
    std::vector<int> items;
    items.clear();
    const std::map<int, int>& reverse =
        G_CDataManager()->get_item_dictionary_flag_to_item_map();
    for (unsigned int index = 0; index <= 4999u; ++index) {
        const uint8_t flags = check_flags_[index];
        for (unsigned int bit = 0; bit <= 7u; ++bit) {
            if (((flags >> bit) & 1u) == 0u) {
                continue;
            }
            const int item_flag = static_cast<int>(bit + 8u * index);
            std::map<int, int>::const_iterator found =
                reverse.find(item_flag);
            if (found != reverse.end()) {
                items.push_back(found->second);
            }
        }
    }
    return items;
}

// IDA_EA: 0x811DCE6
// IDA_NAME: _ZN15CItemDictionary22sendItemDictionaryInfoEP5CUserj
// DEMANGLED: CItemDictionary::sendItemDictionaryInfo(CUser *,unsigned int)
void CItemDictionary::sendItemDictionaryInfo(CUser* user,
                                             unsigned int item_id) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 385);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(item_id));
    packet.finalize(1);
    user->Send(packet);
}

// IDA_EA: 0x811DD9A
// IDA_NAME: _ZN15CItemDictionary27sendItemDictionaryInfoTotalEP5CUser
// DEMANGLED: CItemDictionary::sendItemDictionaryInfoTotal(CUser *)
void CItemDictionary::sendItemDictionaryInfoTotal(CUser* user) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 394);
    const int length = getLengthBinary();
    if (length) {
        packet.put_int(length);
        packet.put_binary(
            reinterpret_cast<const char*>(check_flags_), length);
    } else {
        packet.put_int(0);
    }
    packet.finalize(1);
    user->Send(packet);
}
