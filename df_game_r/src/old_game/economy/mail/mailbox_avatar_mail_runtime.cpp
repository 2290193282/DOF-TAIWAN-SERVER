#include "old_game/economy/mail/mail_trade_runtime.h"

#include <cstring>

#include "old_data/db/mail/mailbox_payload.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/mailbox_dbwork_stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

struct MailItemPrototypeVTable {
    void (*destructor)(CItem* item);
    void (*deleting_destructor)(CItem* item);
    void (*make_item)(const CItem* item, Inven_Item& inventory_item);
    int (*get_item_type)(const CItem* item);
    bool (*is_avatar)(const CItem* item);
    bool (*is_creature)(const CItem* item);
};

inline __attribute__((always_inline))
bool IsAvatarPrototype(const CItem* prototype) {
    const MailItemPrototypeVTable* vtable = 0;
    std::memcpy(&vtable, prototype, sizeof(vtable));
    return vtable->is_avatar(prototype);
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x85561B0
// IDA_NAME: _ZN8WongWork14CMailBoxHelper22ReqDBSendNewAvatarMailEP5CUseriiiccPKcS4_i
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(CUser *,int,int,int,char,char,char const*,char const*,int)
void CMailBoxHelper::ReqDBSendNewAvatarMail(CUser* user,
                                            int receiver_charac_no,
                                            int item_id,
                                            int expire_time,
                                            char ability_no,
                                            char avatar_select,
                                            const char* ipg_no,
                                            const char* text,
                                            int text_length) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("MailBox.cpp", 2013), true);
    *guard << 197;
    *guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();
    payload->package_type = 0;
    std::strncpy(payload->sender_charac_name,
                 user->getCurCharacName(),
                 29u);
    payload->receiver_charac_no = receiver_charac_no;
    payload->sender_charac_no = user->getCurCharacNo();
    payload->package.avatar.set(receiver_charac_no,
                                static_cast<unsigned int>(item_id),
                                0,
                                expire_time,
                                true,
                                ability_no,
                                ipg_no,
                                2,
                                0ul);
    payload->letter_text_length = 0;
    std::memset(payload->letter_text, 0, sizeof(payload->letter_text));
    if (text && text_length) {
        payload->letter_text_length =
            static_cast<uint16_t>(text_length);
        std::strncpy(payload->letter_text,
                     text,
                     payload->letter_text_length);
    }

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (prototype && IsAvatarPrototype(prototype)) {
        reinterpret_cast<CEquipItem*>(prototype)->getAvatarSocket(
            avatar_select, emblem);
    }
    payload->package.avatar.setJewelSocket(emblem);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8556F16
// IDA_NAME: _ZN8WongWork14CMailBoxHelper30ReqDBSendNewAvatarMailCashShopEP5CUseriiiccPKcS4_i
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(CUser *,int,int,int,char,char,char const*,char const*,int)
void CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(
    CUser* user,
    int receiver_charac_no,
    int item_id,
    int expire_time,
    char ability_no,
    char avatar_select,
    const char* ipg_no,
    const char* text,
    int text_length) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("MailBox.cpp", 2422), true);
    *guard << 197;
    *guard << user->GetUID();

    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();
    payload->package_type = 0;
    std::strncpy(payload->sender_charac_name,
                 user->getCurCharacName(),
                 29u);
    payload->receiver_charac_no = receiver_charac_no;
    payload->sender_charac_no = user->getCurCharacNo();
    payload->package.avatar.set(receiver_charac_no,
                                static_cast<unsigned int>(item_id),
                                0,
                                expire_time,
                                true,
                                ability_no,
                                ipg_no,
                                2,
                                0ul);

    // 0x8556F16 sets the CashShop/unlimit flag at the exact DBWORK +0x1a2
    // position.  The ordinary avatar sender intentionally leaves it clear.
    payload->unlimit_flag = 1;
    payload->letter_text_length = 0;
    std::memset(payload->letter_text, 0, sizeof(payload->letter_text));
    if (text && text_length) {
        payload->letter_text_length = static_cast<uint16_t>(text_length);
        std::strncpy(payload->letter_text,
                     text,
                     payload->letter_text_length);
    }

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (prototype && IsAvatarPrototype(prototype)) {
        reinterpret_cast<CEquipItem*>(prototype)->getAvatarSocket(
            avatar_select, emblem);
    }
    payload->package.avatar.setJewelSocket(emblem);
    // The CashShop branch alone initializes this package tail at avatar +0x4f
    // (DBWORK payload +0x7b).
    payload->package.avatar.expansion_info.init();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

}  // namespace WongWork
