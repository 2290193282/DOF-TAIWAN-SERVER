#include "old_data/Login/packet_dispatch/login_dispatcher.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/Login/garena/auth_garena_token.h"
#include "old_data/Login/packet_dispatch/mouse_register_dispatcher.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

const char kDispatchSignature[] =
    "virtual int DisPatcher_Login::dispatch_sig(CUser*, PacketBuf&)";

int LoginLineFailure(int line) {
    return LineFunc(line, kDispatchSignature, 0, 0);
}

int LoginAccountLineFailure(int line, CUser* user) {
    return LineFunc(line, kDispatchSignature, 0, user->get_acc_id());
}

}  // namespace

// IDA_EA: 0x85953D2
// IDA_NAME: _ZN16DisPatcher_LoginC2Ev
// DEMANGLED: DisPatcher_Login::DisPatcher_Login(void)
DisPatcher_Login::DisPatcher_Login() : version_mismatch_count_(0) {}

// IDA_EA: 0x8232A1C
// IDA_NAME: _ZN16DisPatcher_LoginD2Ev
// DEMANGLED: DisPatcher_Login::~DisPatcher_Login()
// IDA_EA: 0x8232A4C
// IDA_NAME: _ZN16DisPatcher_LoginD0Ev
// DEMANGLED: DisPatcher_Login::~DisPatcher_Login()
DisPatcher_Login::~DisPatcher_Login() {}

// IDA_EA: 0x81E8C78
// IDA_NAME: _ZN16DisPatcher_Login12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_Login::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_Login::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 1) {
        PacketGuard rejected;
        rejected.put_header(1, 3);
        rejected.put_byte(1);
        rejected.finalize(true);
        user->Send(rejected);
        return 0;
    }

    user->OnRecvEvent();

    GameWorld* const world = G_GameWorld();
    if (world->GetChannelType() == 6) {
        CEventManager* const events = GlobalData::s_event_manager;
        if (events->GetRepeatEvent(33)->IsEventing(0) != 1) {
            PacketGuard denied;
            denied.put_header(1, 1);
            denied.put_byte(0);
            denied.put_byte(66);
            denied.finalize(true);
            user->Send(denied);
            user->DisConnSig(static_cast<DISCONN_SIG>(22), true, 0);
            return 0;
        }
    }

    uint32_t patch_time = 0;
    if (packet.get_int(patch_time) != 1) {
        return LoginLineFailure(26113);
    }
    if (patch_time > 0xE10u) {
        return LoginLineFailure(26119);
    }

    uint32_t token_length = 0;
    if (packet.get_int(token_length) != 1) {
        return LoginLineFailure(26125);
    }
    // IDA names this function-local object
    // `DisPatcher_Login::dispatch_sig(...)::garenaToken`.  DispatchThread is
    // the sole client packet consumer, so retain the original static storage
    // lifetime instead of silently changing the login-token buffer to stack
    // ownership.
    static char token[2049];
    std::memset(token, 0, sizeof(token));
    if (packet.get_str(token, sizeof(token), static_cast<int>(token_length)) != 1) {
        return LoginLineFailure(26130);
    }
    Taiwan::GarenaAuthData* const auth_data = user->getGarenaAuthData();
    if (Taiwan::AuthGarenaToken::decrypt(token,
                                         static_cast<int>(token_length),
                                         *auth_data) != 0) {
        return LoginLineFailure(26136);
    }
    auth_data->setPatchTime(patch_time);

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, static_cast<RestrictCategory::Enum>(1), 33) != 0) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 209);
        return 0;
    }

    int user_name_length = 0;
    char user_name[24] = {};
    if (packet.get_int(user_name_length) != 1) {
        return LoginLineFailure(26207);
    }
    if (packet.get_str(user_name, sizeof(user_name), user_name_length) != 1) {
        return LoginLineFailure(26237);
    }

    uint32_t password_text_length = 0;
    char password_text[132] = {};
    if (packet.get_int(password_text_length) != 1) {
        return LoginLineFailure(26258);
    }
    if (password_text_length > 0x83u) {
        return LoginLineFailure(26264);
    }
    if (packet.get_str(password_text,
                       sizeof(password_text),
                       static_cast<int>(password_text_length)) != 1) {
        return LoginLineFailure(26280);
    }

    unsigned char login_kind = 0;
    if (packet.get_byte(login_kind) != 1) {
        return LoginLineFailure(26420);
    }
    if (login_kind > 2u) {
        return LoginAccountLineFailure(26424, user);
    }

    uint32_t encrypted_password_length = 0;
    if (packet.get_int(encrypted_password_length) != 1) {
        return LoginAccountLineFailure(26431, user);
    }
    if (encrypted_password_length != 8u) {
        return LoginAccountLineFailure(26433, user);
    }

    char encrypted_password[8];
    char decrypted_password[8];
    if (packet.get_binary(encrypted_password, sizeof(encrypted_password)) != 1) {
        return LoginLineFailure(26434);
    }
    if (Dispatcher_MouseRegister::decryptPassword(encrypted_password,
                                                   decrypted_password,
                                                   encrypted_password_length) != 1) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 1);
        return LoginAccountLineFailure(26446, user);
    }
    if (Dispatcher_MouseRegister::checkValiedPassword(decrypted_password) != 1) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(1), 1);
        return LoginAccountLineFailure(26452, user);
    }

    uint32_t client_antibot_version = 0;
    if (packet.get_int(client_antibot_version) != 1) {
        return LoginLineFailure(26460);
    }
    const uint32_t server_script_version =
        G_CDataManager()->get_login_client_version_gate();
    if (server_script_version != client_antibot_version) {
        // Diagnostic only.  The original branch below remains a hard reject;
        // record both adjacent, IDA-proven fields so a client/PVF mismatch is
        // distinguishable from a CDataManager-layout regression.
        LogManager::logFormat(
            1, "login_flow_diag.cpp",
            "int DisPatcher_Login::dispatch_sig(CUser*, PacketBuf&)", 0,
            "[DIAG0627][LOGIN_VERSION_GATE] client=%u script=%u antibot=%u",
            client_antibot_version,
            server_script_version,
            G_CDataManager()->get_antibot_server_version());
        PacketGuard mismatch;
        mismatch.put_header(1, 1);
        mismatch.put_byte(0);
        mismatch.put_byte(23);
        mismatch.finalize(true);
        user->Send(mismatch);
        ++version_mismatch_count_;
        user->DisConnSig(static_cast<DISCONN_SIG>(25), false, 0);
        return 0;
    }

    uint32_t token_state = 0;
    if (packet.get_int(token_state) != 1) {
        return LoginLineFailure(26651);
    }

    CStreamGuard guard(
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               26994),
        true);
    *guard << 3;
    // IDA 0x81E983C calls CStreamGuard::operator<<(int) directly with the
    // full eax result of CUser::GetUID().  DBTask::DBProcess @0x83FF3F4
    // reads this second stream word as an int, so narrowing it to char
    // shifts the SIG_LOGIN_DATA payload into the user-index field.
    *guard << user->GetUID();
    SIG_LOGIN_DATA* const login = guard.GetInBuffer<SIG_LOGIN_DATA>();
    std::memset(login, 0, sizeof(*login));

    // CLIENT_IDA_EVIDENCE: sub_944650 writes server_group as u8 at
    // 0x944B51. SIG_LOGIN_DATA keeps its wider DB/runtime field unchanged.
    uint8_t server_group = 0;
    if (packet.get_byte(server_group) != 1) {
        return LoginLineFailure(27008);
    }
    login->server_group = server_group;
    if (login->server_group >= 0x12) {
        login->server_group = 0;
    }
    if (world->IsIntegratedPvPBaseChannel()) {
        const unsigned int server_group =
            static_cast<unsigned short>(login->server_group);
        if (server_group <= 0xAu && ((1u << server_group) & 0x580u) != 0u) {
            return LoginLineFailure(27042);
        }
    } else {
        login->server_group = 0;
    }
    user->SetServerGroup(static_cast<ENUM_SERVER_GROUP>(login->server_group));

    int local_ip_address = 0;
    if (packet.get_int(local_ip_address) != 1) {
        return LoginLineFailure(27087);
    }
    user->set_local_ip_address(local_ip_address);

    unsigned char login_state = 0;
    if (packet.get_byte(login_state) != 1) {
        login_state = 0;
    }
    user->SetLogInOutState(login_state);

    std::strncpy(login->login_user_id,
                 NumberToString(auth_data->getUid(), 0),
                 sizeof(login->login_user_id));
    std::memset(login->login_password, 0, sizeof(login->login_password));
    login->token_timestamp = auth_data->getTimeStamp();
    login->game_server_group = world->IsIntegratedPvPBaseChannel()
        ? static_cast<uint32_t>(G_CEnvironment()->get_gc_no_hardcode(
              login->server_group))
        : static_cast<uint32_t>(G_CEnvironment()->get_server_group());
    user->network_.GetPeerIP2(login->login_ip, sizeof(login->login_ip));
    login->login_state = login_kind;
    std::memcpy(login->encrypted_password,
                decrypted_password,
                sizeof(login->encrypted_password));
    user->SetLoginTime(GlobalData::system_time()->getCurSec());
    login->token_state = token_state;

    user->login_session_connected_at_ =
        static_cast<uint32_t>(GlobalData::system_time()->getCurSec());
    user->network_.GetPeerIP2(user->login_session_peer_ip_,
                              sizeof(user->login_session_peer_ip_));
    user->login_session_active_ = 0;
    user->last_sell_item_tick_ = 0;
    user->sell_item_time_count_ = 0;
    user->item_time_check_.sample_count = 0;

    PacketGuard auction_status;
    auction_status.put_header(0, 183);
    auction_status.put_byte(0);
    auction_status.put_byte(GlobalData::s_auction_proxy->IsRunning() ? 1 : 0);
    auction_status.finalize(true);
    user->Send(auction_status);

    PacketGuard cera_auction_status;
    cera_auction_status.put_header(0, 183);
    cera_auction_status.put_byte(1);
    cera_auction_status.put_byte(
        GlobalData::s_cera_auction_proxy->IsRunning() ? 1 : 0);
    cera_auction_status.finalize(true);
    user->Send(cera_auction_status);

    // Do not log the authentication token, cipher text, account name, or IP.
    // This marker is sufficient to correlate CMD1 with DB_Login (signal 3).
    LogManager::logFormat(
        1, "login_flow_diag.cpp",
        "int DisPatcher_Login::dispatch_sig(CUser*, PacketBuf&)", 0,
        "[LOGIN_FLOW][DB_LOGIN_ENQUEUE] uid=%d signal=3 login_state=%d",
        static_cast<int>(user->GetUID()), login->login_state);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x8232A6A
// IDA_NAME: _ZN16DisPatcher_Login4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_Login::read(PacketBuf &,MSG_BASE &)
int DisPatcher_Login::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232A74
// IDA_NAME: _ZN16DisPatcher_Login7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_Login::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_Login::process(CUser* user, MSG_BASE& message, ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
