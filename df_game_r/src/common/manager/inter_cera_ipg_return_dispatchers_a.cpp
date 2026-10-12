#include "common/manager/inter_cera_ipg_return_dispatchers_a.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/ipg_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/db/taiwan/cera_ipg_return_payloads.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_pool.h"

namespace {

void SendIPGNetworkError(CUser* user, PacketGuard& packet) {
    // The non-query failure reply uses BUY_CERASHOP_ITEM with this payload
    // order; its five trailing dwords distinguish the IPG network failure
    // from a normal CeraShop completion packet.
    packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_short(25);
    packet.put_byte(0);
    packet.put_byte(25);
    packet.put_int(0);
    packet.put_int(0);
    packet.put_int(0);
    packet.put_int(0);
    packet.put_int(-1);
    packet.finalize(true);
    user->Send(packet);
}

void LogIPGNetworkError() {
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "virtual int Inter_IPGResult::dispatch_sig(CUser*, char*, int)",
        0x2B96,
        "IPG ERROR (%s)",
        "IPG Network Error");
}

}  // namespace

// IDA_EA: 0x84CEA92
// IDA_NAME: _ZN23Inter_MonitorCeraUpdate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCeraUpdate::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCeraUpdate::dispatch_sig(CUser*, char* payload, int) {
    SIG_MONITOR_CERA_UPDATE* const update =
        reinterpret_cast<SIG_MONITOR_CERA_UPDATE*>(payload);
    // The original intentionally ignores the caller's CUser and finds the
    // live account owner again before issuing IPG_QUERY.
    CUser* const account_user =
        G_CGameManager()->GetUserByAccId(update->account_id);
    if (account_user) {
        // CIPGHelper vtable +0x2C, IDA slot 11: IPGQuery(CUser*).
        GlobalData::s_pIPGHelper->IPGQuery(account_user);
    }
    return 0;
}

// IDA_EA: 0x84EB2C2
// IDA_NAME: _ZN23Inter_MonitorCeraUpdateC2Ev
// IDA_EA: 0x84EB2DE
// IDA_NAME: _ZN23Inter_MonitorCeraUpdateD2Ev
// IDA_EA: 0x84EB30E
// IDA_NAME: _ZN23Inter_MonitorCeraUpdateD0Ev
Inter_MonitorCeraUpdate::Inter_MonitorCeraUpdate() : ISigDispatcher() {}
Inter_MonitorCeraUpdate::~Inter_MonitorCeraUpdate() {}

// IDA_EA: 0x84CEAEE
// IDA_NAME: _ZN15Inter_IPGResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_IPGResult::dispatch_sig(CUser *,char *,int)
int Inter_IPGResult::dispatch_sig(CUser*, char* payload, int) {
    SIG_IPG_RESULT* const response =
        reinterpret_cast<SIG_IPG_RESULT*>(payload);
    if (response->has_result != 0) {
        WongWork::IPG::SIPGData* const result =
            &response->body.success.result;
        WongWork::IPG::SIPGData* const request =
            &response->body.success.request;
        const int user_uid = result->user_uid;
        CUser* const user = G_CGameManager()->getUser(user_uid, 11);
        Stream* const callback_stream = result->callback_stream;

        if (user) {
            GlobalData::s_pItemVendingMachine->ProcessIPGUserData(
                user, request, result, callback_stream);
        }

        // Ownership transfers to this receiver irrespective of whether the
        // target user is still connected.  This mirrors 0x84CEE7B/8A.
        GlobalData::s_stream_pool->Free(callback_stream);
        return 0;
    }

    WongWork::IPG::SIPGData* const request =
        &response->body.failed_request;
    const int user_uid = request->user_uid;
    CUser* const user = G_CGameManager()->getUser(user_uid, 10);
    if (!user) {
        return 0;
    }

    // 0x84CEB4A constructs PacketGuard for all failure data types, including
    // the trace-only IPG_INPUT and IPG_INPUT_POINT branches.
    PacketGuard packet_guard;
    if (request->data_type == WongWork::IPG::IPG_QUERY) {
        user->SendCashData(true);
        LogIPGNetworkError();
        return 0;
    }

    if (request->data_type == WongWork::IPG::IPG_INPUT_POINT) {
        // The old virtual call at +0x2C is the one-argument IPGQuery.  It
        // resubmits the balance query before emitting the original trace.
        GlobalData::s_pIPGHelper->IPGQuery(user);
        cMyTrace trace(
            "virtual int Inter_IPGResult::dispatch_sig(CUser*, char*, int)",
            0x2B65,
            0);
        trace("[Taiwan, Billing] ChargePointQuery Result:%s, %s, %d, %s",
              request->wire.cera_point.account_name,
              request->wire.cera_point.character_name,
              static_cast<int>(request->wire.cera_point.charge_kind),
              request->wire.cera_point.result_code_decimal);
        return 0;
    }

    if (request->data_type == WongWork::IPG::IPG_INPUT) {
        cMyTrace trace(
            "virtual int Inter_IPGResult::dispatch_sig(CUser*, char*, int)",
            0x2B6E,
            0);
        trace("[Taiwan, Billing] ChargeCeraQuery Result:%s, %s, %s, %d",
              request->wire.input.account_header.charge.account_name,
              request->wire.input.account_header.charge.character_name,
              request->wire.input.charge_result_decimal,
              static_cast<int>(request->wire.input.charge_kind));
        return 0;
    }

    SendIPGNetworkError(user, packet_guard);
    LogIPGNetworkError();
    return 0;
}

// IDA_EA: 0x84EB32C
// IDA_NAME: _ZN15Inter_IPGResultC2Ev
// IDA_EA: 0x84EB348
// IDA_NAME: _ZN15Inter_IPGResultD2Ev
// IDA_EA: 0x84EB378
// IDA_NAME: _ZN15Inter_IPGResultD0Ev
Inter_IPGResult::Inter_IPGResult() : ISigDispatcher() {}
Inter_IPGResult::~Inter_IPGResult() {}
