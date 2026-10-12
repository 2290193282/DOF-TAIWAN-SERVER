#include "old_game/activity/schedule/packet_dispatcher_take_magic_lamp.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_take_magic_lamp.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <string>

namespace {

const char kMagicLampPvf[] = "tw_magiclamp.evt";
const int kMagicLampItemId = 690001710;

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct MagicLampConfig {
    MagicLampConfig() : item_id(0), mail_title_key(), mail_content_key() {}

    int item_id;
    std::string mail_title_key;
    std::string mail_content_key;
};

class MagicLampConfigCache {
public:
    MagicLampConfigCache() : attempted_(false), valid_(false), config_() {}

    const MagicLampConfig* get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile("Event/", kMagicLampPvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        MagicLampConfig parsed;
        bool has_item = false;
        bool has_title = false;
        bool has_content = false;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag == "[magic lamp item index]") {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success || has_item) {
                    return false;
                }
                parsed.item_id = value;
                has_item = true;
            } else if (tag == "[mail title]") {
                if (has_title || ScanStr(&parsed.mail_title_key) != 1) {
                    return false;
                }
                has_title = true;
            } else if (tag == "[mail content]") {
                if (has_content || ScanStr(&parsed.mail_content_key) != 1) {
                    return false;
                }
                has_content = true;
            }
        }

        CDataManager* const manager = G_CDataManager();
        CItem* const item = manager && parsed.item_id > 0
                                ? manager->find_item(parsed.item_id)
                                : 0;
        if (!has_item || !has_title || !has_content ||
            parsed.item_id != kMagicLampItemId ||
            parsed.mail_title_key.empty() || parsed.mail_content_key.empty() ||
            !item || !item->vtable_) {
            return false;
        }
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    MagicLampConfig config_;
};

MagicLampConfigCache& ConfigCache() {
    static MagicLampConfigCache cache;
    return cache;
}

bool IsReadyUser(CUser* user) {
    return user && user->get_state() == 3 && user->get_acc_id() > 0 &&
           user->get_charac_no(-1) > 0 && user->getCurCharacR();
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace(
        "CMD723 Dispatcher_TakeMagicLamp0828/Inter_TakeMagicLamp0828",
        __LINE__,
        5);
    trace("[0828 MagicLamp] %s account(%d) charac(%d)",
          reason ? reason : "unknown",
          user ? user->get_acc_id() : 0,
          user ? user->get_charac_no(-1) : 0);
}

bool SubmitRewardMail(CUser* user,
                      uint32_t reward_charac_no,
                      int reward_server_group,
                      const MagicLampConfig& config) {
    CDataManager* const manager = G_CDataManager();
    CItem* const prototype = manager ? manager->find_item(config.item_id) : 0;
    if (!user || reward_charac_no == 0u || reward_server_group < 0 ||
        reward_server_group >= magic_lamp::kMaximumServerGroupCount ||
        !prototype || !prototype->vtable_) {
        return false;
    }

    const char* const title = g_scriptStringManager_.findString(
        4, config.mail_title_key.c_str(), 0);
    const char* const content = g_scriptStringManager_.findString(
        4, config.mail_content_key.c_str(), 0);
    const char* const safe_title = title ? title : config.mail_title_key.c_str();
    const char* const safe_content =
        content ? content : config.mail_content_key.c_str();
    if (!safe_title[0] || !safe_content[0]) {
        return false;
    }

    Inven_Item reward;
    prototype->vtable_->make_item(prototype, reward);
    reward.set_add_info(1);

    SIG_MAILBOX_PERSISTENCE_CONTEXT persistence;
    std::memset(&persistence, 0, sizeof(persistence));
    persistence.consumer =
        MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP;
    persistence.account_id = static_cast<uint32_t>(user->get_acc_id());

    // NO_XA_BOUNDARY: queue acceptance is not persistence.  The tracked
    // signal-128 tail is echoed only after postal exec(true); Inter128 then
    // queues the idempotent SQL014 state transition.  A crash/queue failure
    // between those two databases leaves state 0 retryable (and may duplicate
    // mail), but can no longer permanently mark an unpersisted reward done.
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMailTracked(
        safe_title,
        reward,
        0,
        static_cast<unsigned int>(reward_charac_no),
        safe_content,
        static_cast<int>(std::strlen(safe_content)),
        14,
        static_cast<ENUM_SERVER_GROUP>(reward_server_group),
        false,
        false,
        persistence) == 0;
}

}  // namespace

Dispatcher_TakeMagicLamp::Dispatcher_TakeMagicLamp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_TakeMagicLamp::~Dispatcher_TakeMagicLamp() {}

int Dispatcher_TakeMagicLamp::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_690710 begins CMD723 at 0x690763 and invokes
    // the finalizer at 0x690768 without any payload writer.  It also closes
    // the confirmation window immediately; the client registers no CMD723
    // response handler.  Never emit a synthetic OK/error response.
    if (!IsReadyUser(user)) {
        return 0;
    }
    const MagicLampConfig* const config = ConfigCache().get();
    if (!config) {
        TraceFailure("PVF configuration unavailable", user);
        return 0;
    }
    if (!magic_lamp::DB_TakeMagicLamp::makeReserveRequest(
            user->GetUID(),
            static_cast<uint32_t>(user->get_acc_id()),
            static_cast<uint32_t>(user->get_charac_no(-1)),
            static_cast<uint32_t>(config->item_id),
            user->GetServerGroup())) {
        TraceFailure("DB request queue failed", user);
    }
    return 0;
}

int Dispatcher_TakeMagicLamp::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_TakeMagicLamp::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_TakeMagicLamp::Inter_TakeMagicLamp() : ISigDispatcher() {}

Inter_TakeMagicLamp::~Inter_TakeMagicLamp() {}

int Inter_TakeMagicLamp::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    typedef magic_lamp::ClaimMessage ClaimMessage;
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(ClaimMessage))) {
        return 0;
    }
    const ClaimMessage* const result =
        reinterpret_cast<const ClaimMessage*>(payload);
    if (!IsReadyUser(user) ||
        result->account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        result->request_charac_no !=
            static_cast<uint32_t>(user->get_charac_no(-1)) ||
        result->operation != magic_lamp::CLAIM_OPERATION_RESERVE ||
        result->request_server_group != user->GetServerGroup()) {
        return 0;
    }

    if (result->result == magic_lamp::CLAIM_PENDING_RECENT ||
        result->result == magic_lamp::CLAIM_ALREADY_SUBMITTED) {
        return 0;
    }
    if (result->result != magic_lamp::CLAIM_GRANTED &&
        result->result != magic_lamp::CLAIM_RETRY_PENDING) {
        TraceFailure("database claim failed", user);
        return 0;
    }

    const MagicLampConfig* const config = ConfigCache().get();
    if (!config || result->reward_charac_no == 0u ||
        result->reward_item_id != static_cast<uint32_t>(config->item_id) ||
        !SubmitRewardMail(user,
                          result->reward_charac_no,
                          result->reward_server_group,
                          *config)) {
        TraceFailure("reward mail submission failed after claim", user);
        return 0;
    }
    // Do not advance SQL014 here: this only proves that DB_Q accepted the
    // request.  Inter_MailBox_Req_Send consumes the correlated signal-128
    // persistence acknowledgement and performs the idempotent state change.

    // ENUM_NOTIPACKET_MAGIC_LAMP_EVENT (696) is present in PacketEnum but the
    // 0828 client receive registry has no handler for it.  Sending it here
    // would be an invented packet, so successful delivery is observed only
    // through the native system-mail path.
    return 0;
}
