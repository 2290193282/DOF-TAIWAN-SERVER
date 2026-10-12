#include "old_game/economy/equipment/inter_clone_title.h"

#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/db/character/db_clone_title.h"
#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <stdint.h>


namespace {

const uint8_t kCloneTitleOperationFailed = 1u;

bool IsCurrentWorldCharacter(CUser* user, uint32_t charac_no) {
    return user && user->get_state() == 3 && user->getCurCharacR() &&
           user->getCurCharacNo() > 0 &&
           static_cast<uint32_t>(user->getCurCharacNo()) == charac_no;
}

void SendCloneTitleSuccess(CUser* user, uint32_t item_id) {
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_SET_CLONE_TITLE);
    response.put_byte(1u);
    response.put_int(item_id);
    response.finalize(true);
    user->Send(response);
}

void BroadcastFullAppearance(CUser* user) {
    GameWorld* const world = G_GameWorld();
    if (!user || !world) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(1u);
    packet.put_short(1u);
    if (user->make_basic_info(packet, 1) != 1) {
        return;
    }
    packet.finalize(true);
    world->send_all(packet, user);
}

}  // namespace


Inter_CloneTitle::Inter_CloneTitle() : ISigDispatcher() {}

Inter_CloneTitle::~Inter_CloneTitle() {}

int Inter_CloneTitle::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    if (!payload ||
        payload_size !=
            static_cast<int>(sizeof(clone_title_db::Message))) {
        return 0;
    }

    const clone_title_db::Message* const message =
        reinterpret_cast<const clone_title_db::Message*>(payload);
    uint32_t applied_item_id = 0;
    const clone_title::DatabaseReplyAction action =
        clone_title::ApplyDatabaseResult(
            user, *message, &applied_item_id);

    if (action == clone_title::kDatabaseReplyLoadFailed) {
        LogManager::logFormat(
            1,
            "inter_clone_title_0828.cpp",
            "Inter_CloneTitle0828::dispatch_sig",
            0,
            "clone title database load failed charac_no=%u",
            static_cast<unsigned int>(message->charac_no));
        return 0;
    }

    const bool current =
        IsCurrentWorldCharacter(user, message->charac_no);
    if (action == clone_title::kDatabaseReplyLoadApplied) {
        // If the reply beat world entry, make_basic_info will read the cache
        // naturally.  A late reply refreshes the already visible appearance.
        if (current) {
            BroadcastFullAppearance(user);
        }
        return 0;
    }

    if (action == clone_title::kDatabaseReplySaveFailed) {
        if (message->reply_kind == clone_title_db::kReplyCommand &&
            current) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_CLONE_TITLE,
                                     kCloneTitleOperationFailed);
        } else {
            LogManager::logFormat(
                1,
                "inter_clone_title_0828.cpp",
                "Inter_CloneTitle0828::dispatch_sig",
                0,
                "clone title database save failed charac_no=%u",
                static_cast<unsigned int>(message->charac_no));
        }
        return 0;
    }

    if (action == clone_title::kDatabaseReplySaveSucceeded &&
        message->reply_kind == clone_title_db::kReplyCommand &&
        current) {
        SendCloneTitleSuccess(user, applied_item_id);
        // ParseEquippedItemDetailList @0x5A3D0F consumes this cached u32 from
        // full equipment lists; USERINFO is the existing visibility owner.
        BroadcastFullAppearance(user);
    }
    return 0;
}
