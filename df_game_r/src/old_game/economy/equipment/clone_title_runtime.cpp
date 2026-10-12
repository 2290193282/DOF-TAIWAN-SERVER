#include "old_game/economy/equipment/clone_title_runtime.h"

#include <limits.h>
#include <map>

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "old_data/db/character/db_clone_title.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_game/player/core/title_book_owned_items.h"


namespace clone_title {
namespace {

enum CacheStatus {
    kCacheLoading = 1,
    kCacheLoaded = 2,
};

struct CharacterCloneTitleState {
    CharacterCloneTitleState()
        : item_id(0), status(static_cast<uint8_t>(kCacheLoading)) {}
    CharacterCloneTitleState(uint32_t value, CacheStatus state)
        : item_id(value), status(static_cast<uint8_t>(state)) {}

    uint32_t item_id;
    uint8_t status;
};

typedef std::map<uint32_t, CharacterCloneTitleState> CharacterCloneTitleMap;
typedef std::map<CUser*, CharacterCloneTitleMap> UserCloneTitleMap;

Mutex& StateMutex() {
    static Mutex mutex;
    return mutex;
}

UserCloneTitleMap& States() {
    static UserCloneTitleMap states;
    return states;
}

uint32_t ResolveCharacterNo(CUser* user, const CInventory* inventory) {
    if (!user || !inventory) {
        return 0;
    }

    const _Charac_info* const current = user->getCurCharacR();
    if (current && &current->fields_.inventory_ == inventory &&
        current->fields_.character_no_ > 0) {
        return static_cast<uint32_t>(current->fields_.character_no_);
    }

    const _Charac_info* const tag = user->getTagCharacR();
    if (tag && &tag->fields_.inventory_ == inventory &&
        tag->fields_.character_no_ > 0) {
        return static_cast<uint32_t>(tag->fields_.character_no_);
    }
    return 0;
}

bool IsTitlePrototype(uint32_t item_id) {
    if (!item_id || item_id > static_cast<uint32_t>(INT_MAX)) {
        return false;
    }
    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        return false;
    }
    CItem* const item = data_manager->find_item(static_cast<int>(item_id));
    return item && !item->is_stackable() && item->vtable_ &&
           item->vtable_->get_item_type(item) ==
               oldsrv::config::kEquipmentTypeTitle;
}

bool IsOwnedByInventory(CUser* user,
                        const CInventory* inventory,
                        uint32_t item_id) {
    if (!user || !inventory || !IsTitlePrototype(item_id)) {
        return false;
    }

    // CInventory::check_item_exist scans the ordinary inventory ranges; it
    // deliberately does not inspect the equipped table.  Cover the title
    // equipment slot explicitly before scanning the bag.
    const Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeTitle);
    if ((equipped &&
         static_cast<uint32_t>(equipped->item_id_) == item_id) ||
        inventory->check_item_exist(static_cast<int>(item_id)) != -1) {
        return true;
    }

    // Character-expand title-book data belongs to the current character.
    // Tag-character inventories are still validated by their equipment/bag
    // ownership above, but must never borrow the current title book.
    if (inventory != user->getCurCharacInvenR()) {
        return false;
    }

    CTitleBook* const owned_title_book = static_cast<CTitleBook*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)));
    if (!owned_title_book) {
        return false;
    }

    // _getFitSlot searches the PVF's fixed collection requirements, not
    // ownership.  Wildcard/event slots accept other titles legitimately.
    return title_book::ContainsOwnedTitle(owned_title_book, item_id);
}

bool IsOwnedByCurrentCharacter(CUser* user, uint32_t item_id) {
    return user && IsOwnedByInventory(
                       user, user->getCurCharacInvenR(), item_id);
}

bool IsCharacterInSession(CUser* user, uint32_t charac_no) {
    if (!user || !charac_no) {
        return false;
    }
    const int slot = user->get_charac_idx(charac_no);
    return slot >= 0 && static_cast<unsigned char>(slot) != 0xffu;
}

bool QueueMessage(CUser* user,
                  uint32_t charac_no,
                  uint32_t item_id,
                  uint8_t operation,
                  uint8_t reply_kind) {
    if (!user || !charac_no || user->get_acc_id() <= 0) {
        return false;
    }

    clone_title_db::Message message;
    message.account_id = static_cast<uint32_t>(user->get_acc_id());
    message.charac_no = charac_no;
    message.item_id = item_id;
    message.server_group = user->GetServerGroup();
    message.operation = operation;
    message.result = clone_title_db::kResultFailed;
    message.reply_kind = reply_kind;
    message.reserved = 0;
    return clone_title_db::DB_CloneTitle::Queue(
        user->GetUID(), message);
}

}  // namespace


bool RequestLoad(CUser* user, uint32_t charac_no) {
    if (!IsCharacterInSession(user, charac_no)) {
        return false;
    }

    {
        MutexGuard guard(&StateMutex());
        CharacterCloneTitleMap& character_states = States()[user];
        if (character_states.find(charac_no) != character_states.end()) {
            return true;
        }
        character_states.insert(CharacterCloneTitleMap::value_type(
            charac_no,
            CharacterCloneTitleState(0u, kCacheLoading)));
    }

    if (QueueMessage(user,
                     charac_no,
                     0u,
                     clone_title_db::kOperationLoad,
                     clone_title_db::kReplyBackground)) {
        return true;
    }

    // Queue acquisition can fail during startup/shutdown.  Remove only the
    // loading sentinel installed by this request so a later packet build can
    // retry without touching an already applied response.
    MutexGuard guard(&StateMutex());
    UserCloneTitleMap::iterator user_it = States().find(user);
    if (user_it != States().end()) {
        CharacterCloneTitleMap::iterator state =
            user_it->second.find(charac_no);
        if (state != user_it->second.end() &&
            state->second.status == kCacheLoading) {
            user_it->second.erase(state);
            if (user_it->second.empty()) {
                States().erase(user_it);
            }
        }
    }
    return false;
}

uint32_t GetForInventory(CUser* user, const CInventory* inventory) {
    const uint32_t charac_no = ResolveCharacterNo(user, inventory);
    if (!charac_no) {
        return 0;
    }

    CharacterCloneTitleState cached;
    bool found = false;
    {
        MutexGuard guard(&StateMutex());
        UserCloneTitleMap::const_iterator user_it = States().find(user);
        if (user_it != States().end()) {
            CharacterCloneTitleMap::const_iterator charac_it =
                user_it->second.find(charac_no);
            if (charac_it != user_it->second.end()) {
                cached = charac_it->second;
                found = true;
            }
        }
    }

    if (!found) {
        // This function is reached from packet construction on Dispatch/Game/
        // Inter threads.  A miss may enqueue DB_Q work, but never obtains or
        // touches a MySQL handle synchronously.
        (void)RequestLoad(user, charac_no);
        return 0;
    }
    if (cached.status != kCacheLoaded || cached.item_id == 0u) {
        return 0;
    }
    if (IsOwnedByInventory(user, inventory, cached.item_id)) {
        return cached.item_id;
    }

    // A stale/corrupt row is fail-closed for this session.  Do not delete it
    // here: packet construction can race the last pieces of character data
    // loading, and only DB_Q may decide durable state changes.
    MutexGuard guard(&StateMutex());
    UserCloneTitleMap::iterator user_it = States().find(user);
    if (user_it != States().end()) {
        CharacterCloneTitleMap::iterator state =
            user_it->second.find(charac_no);
        if (state != user_it->second.end() &&
            state->second.status == kCacheLoaded &&
            state->second.item_id == cached.item_id) {
            state->second.item_id = 0u;
        }
    }
    return 0;
}

bool SetForCurrentCharacter(CUser* user, uint32_t item_id) {
    if (!user || !user->getCurCharacR()) {
        return false;
    }
    const int raw_charac_no = user->getCurCharacNo();
    if (raw_charac_no <= 0 ||
        (item_id && !IsOwnedByCurrentCharacter(user, item_id))) {
        return false;
    }

    // The command is acknowledged only by signal 822 after the dedicated
    // DB_Q owner commits this value.  No game/Inter thread obtains a handle.
    return QueueMessage(user,
                        static_cast<uint32_t>(raw_charac_no),
                        item_id,
                        clone_title_db::kOperationSave,
                        clone_title_db::kReplyCommand);
}

DatabaseReplyAction ApplyDatabaseResult(
    CUser* user,
    const clone_title_db::Message& message,
    uint32_t* applied_item_id) {
    if (applied_item_id) {
        *applied_item_id = 0u;
    }
    if (!user || user->get_acc_id() <= 0 ||
        static_cast<uint32_t>(user->get_acc_id()) != message.account_id ||
        user->GetServerGroup() != message.server_group ||
        !IsCharacterInSession(user, message.charac_no)) {
        return kDatabaseReplyIgnored;
    }

    if (message.operation == clone_title_db::kOperationLoad) {
        if (message.reply_kind != clone_title_db::kReplyBackground) {
            return kDatabaseReplyIgnored;
        }

        MutexGuard guard(&StateMutex());
        UserCloneTitleMap::iterator user_it = States().find(user);
        if (user_it == States().end()) {
            return kDatabaseReplyIgnored;
        }
        CharacterCloneTitleMap::iterator state =
            user_it->second.find(message.charac_no);
        if (state == user_it->second.end() ||
            state->second.status != kCacheLoading) {
            return kDatabaseReplyIgnored;
        }

        if (message.result != clone_title_db::kResultOk) {
            user_it->second.erase(state);
            if (user_it->second.empty()) {
                States().erase(user_it);
            }
            return kDatabaseReplyLoadFailed;
        }

        const uint32_t item_id =
            !message.item_id || IsTitlePrototype(message.item_id)
                ? message.item_id
                : 0u;
        state->second.item_id = item_id;
        state->second.status = kCacheLoaded;
        if (applied_item_id) {
            *applied_item_id = item_id;
        }
        return kDatabaseReplyLoadApplied;
    }

    if (message.operation != clone_title_db::kOperationSave ||
        (message.reply_kind != clone_title_db::kReplyBackground &&
         message.reply_kind != clone_title_db::kReplyCommand)) {
        return kDatabaseReplyIgnored;
    }
    if (message.result != clone_title_db::kResultOk ||
        (message.item_id && !IsTitlePrototype(message.item_id))) {
        return kDatabaseReplySaveFailed;
    }

    {
        MutexGuard guard(&StateMutex());
        States()[user][message.charac_no] = CharacterCloneTitleState(
            message.item_id, kCacheLoaded);
    }
    if (applied_item_id) {
        *applied_item_id = message.item_id;
    }
    return kDatabaseReplySaveSucceeded;
}

void ResetUser(CUser* user) {
    if (!user) {
        return;
    }
    MutexGuard guard(&StateMutex());
    States().erase(user);
}

}  // namespace clone_title
