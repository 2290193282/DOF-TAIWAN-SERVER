#include "old_data/db/auction_cashshop/arad_buy_item_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_instance.h"
#include "infra/log/format/trace.h"
#include "old_data/db/auction_cashshop/arad_item_query.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/arad_purchase_count_buffer.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


namespace {

const ENUM_DB_HANDLE_IDX kBuyItemCheckDbHandle =
    static_cast<ENUM_DB_HANDLE_IDX>(7);

}  // namespace


template <>
ARAD::Arad_Dispatcher_Manager*
GlobalInstance<ARAD::Arad_Dispatcher_Manager>::inst();

template <>
ARAD::Arad_Dispatcher_Manager*
GlobalInstance<ARAD::Arad_Dispatcher_Manager>::create();

template <>
ARAD::Arad_Dispatcher_Manager*
GlobalInstance<ARAD::Arad_Dispatcher_Manager>::m_p = 0;


namespace ARAD {

// IDA_EA: 0x818EDD0
// IDA_NAME: _ZN4ARAD15Arad_DispatcherC2Ei
// DEMANGLED: ARAD::Arad_Dispatcher::Arad_Dispatcher(int)
Arad_Dispatcher::Arad_Dispatcher(int dispatch_code)
    : dispatch_code_(dispatch_code) {
}

// IDA_EA: 0x818E900
// IDA_NAME: _ZN4ARAD15Arad_DispatcherD2Ev
// DEMANGLED: ARAD::Arad_Dispatcher::~Arad_Dispatcher()
// IDA_EA: 0x818E926
// IDA_NAME: _ZN4ARAD15Arad_DispatcherD0Ev
// DEMANGLED: ARAD::Arad_Dispatcher::~Arad_Dispatcher()
Arad_Dispatcher::~Arad_Dispatcher() {
}

// IDA_EA: 0x818E84C
// IDA_NAME: _ZN4ARAD15Arad_Dispatcher11db_dispatchEiiP6Stream
// DEMANGLED: ARAD::Arad_Dispatcher::db_dispatch(int,int,Stream *)
int Arad_Dispatcher::db_dispatch(int, int, Stream*) {
    return 0;
}

// IDA_EA: 0x818E856
// IDA_NAME: _ZN4ARAD15Arad_Dispatcher14inter_dispatchEP5CUserPci
// DEMANGLED: ARAD::Arad_Dispatcher::inter_dispatch(CUser *,char *,int)
int Arad_Dispatcher::inter_dispatch(CUser*, char*, int) {
    return 0;
}

// IDA_EA: 0x818E860
// IDA_NAME: _ZN4ARAD15Arad_Dispatcher19register_dispatcherEPS0_
// DEMANGLED: ARAD::Arad_Dispatcher::register_dispatcher(ARAD::Arad_Dispatcher *)
int Arad_Dispatcher::register_dispatcher(Arad_Dispatcher* dispatcher) {
    Arad_Dispatcher_Manager* const manager =
        GlobalInstance<Arad_Dispatcher_Manager>::inst();
    return manager->register_dispatcher(dispatcher);
}

// IDA_EA: 0x818E87C
// IDA_NAME: _ZN4ARAD15Arad_Dispatcher21unregister_dispatcherEi
// DEMANGLED: ARAD::Arad_Dispatcher::unregister_dispatcher(int)
int Arad_Dispatcher::unregister_dispatcher(int dispatch_code) {
    Arad_Dispatcher_Manager* const manager =
        GlobalInstance<Arad_Dispatcher_Manager>::inst();
    return manager->unregister_dispatcher(dispatch_code);
}

// IDA_EA: 0x818E898
// IDA_NAME: _ZN4ARAD15Arad_Dispatcher13dispathc_codeEv
// DEMANGLED: ARAD::Arad_Dispatcher::dispathc_code(void)
int Arad_Dispatcher::dispathc_code() {
    return dispatch_code_;
}

// IDA_EA: 0x818E9CC
// IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplC2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::Arad_Dispatcher_BuyItemCheck_Impl(void)
Arad_Dispatcher_BuyItemCheck_Impl::Arad_Dispatcher_BuyItemCheck_Impl()
    : Arad_Dispatcher(506) {
}

// IDA_EA: 0x818E9F0
// IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplD2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::~Arad_Dispatcher_BuyItemCheck_Impl()
// IDA_EA: 0x818EA20
// IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_ImplD0Ev
// DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::~Arad_Dispatcher_BuyItemCheck_Impl()
Arad_Dispatcher_BuyItemCheck_Impl::~Arad_Dispatcher_BuyItemCheck_Impl() {
}

// IDA_EA: 0x818EA3E
// IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_Impl14inter_dispatchEP5CUserPci
// DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::inter_dispatch(CUser *,char *,int)
int Arad_Dispatcher_BuyItemCheck_Impl::inter_dispatch(CUser* user,
                                                       char* payload,
                                                       int) {
    if (payload[0] == 1) {
        const MSG_BUY_CERASHOP_ITEM& request =
            *reinterpret_cast<const MSG_BUY_CERASHOP_ITEM*>(payload + 1);
        if (static_cast<unsigned char>(GlobalData::s_pItemVendingMachine->BuyItem(
                user,
                request)) == 1u) {
            return 0;
        }

        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatcher_BuyItemCheck_Impl.cpp",
            "virtual int ARAD::Arad_Dispatcher_BuyItemCheck_Impl::inter_dispatch(CUser*, char*, int)",
            0x40,
            "[ERROR] BuyItem() Result Error ItemIdx(%d)",
            static_cast<int>(request.items[0].goods_no));
        return -1;
    }

    GlobalData::s_pCeraShop->ProcessError(user, 0x74u, 0, 0, 1, 1, 0);
    return 0;
}

// IDA_EA: 0x818EB0C
// IDA_NAME: _ZN4ARAD33Arad_Dispatcher_BuyItemCheck_Impl11db_dispatchEiiP6Stream
// DEMANGLED: ARAD::Arad_Dispatcher_BuyItemCheck_Impl::db_dispatch(int,int,Stream *)
int Arad_Dispatcher_BuyItemCheck_Impl::db_dispatch(int source_1,
                                                    int source_2,
                                                    Stream* stream) {
    // The old function performs this lookup before parsing; its return value
    // is intentionally not examined here.
    (void)GlobalData::s_db_mgr->GetDBHandle(kBuyItemCheckDbHandle,
                                             SERVER_GROUP_DEFAULT);

    ARAD_DATAS::BuyItemInfo info;
    *stream >> info.member_id;
    *stream >> info.charac_no;
    *stream >> info.ipg_no;
    *stream >> info.check_type;
    *stream >> info.maximum_count_wire;

    MSG_BUY_CERASHOP_ITEM* const request =
        stream->GetOutBuffer<MSG_BUY_CERASHOP_ITEM>();
    if (!request) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatcher_BuyItemCheck_Impl.cpp",
            "virtual int ARAD::Arad_Dispatcher_BuyItemCheck_Impl::db_dispatch(int, int, Stream*)",
            0x66,
            "[ERROR] MSG_BUY_CERASHOP_ITEM GetOutBuffer => MID[%d], CHARIDX[%d], ITEMIDX[%d], CHECKTYPE[%d]",
            info.member_id,
            info.charac_no,
            info.ipg_no,
            info.check_type);
        return -1;
    }

    if (DATABASE::ITEM_QUERY::arad_sp_buy_item_check(info) != 0) {
        return -1;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_Dispatcher_BuyItemCheck_Impl.cpp",
                           113),
                       true);
    guard << source_1;
    guard << source_2;

    INTERNAL_PACKET::PURCHASE_COUNT_CHECK* const response =
        guard.GetInBuffer<INTERNAL_PACKET::PURCHASE_COUNT_CHECK>();
    if (response) {
        response->result = info.result;
        std::memcpy(&response->request, request, sizeof(*request));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatcher_BuyItemCheck_Impl.cpp",
            "virtual int ARAD::Arad_Dispatcher_BuyItemCheck_Impl::db_dispatch(int, int, Stream*)",
            0x7b,
            "ITEM_PURCHASE_COUNT_CHECK MID[%d], CHARIDX[%d], ITEMIDX[%d], CHECKTYPE[%d]",
            info.member_id,
            info.charac_no,
            info.ipg_no,
            info.check_type);
    }
    return 0;
}

// IDA_EA: 0x818FDD4
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_ManagerC2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::Arad_Dispatcher_Manager(void)
Arad_Dispatcher_Manager::Arad_Dispatcher_Manager() {
    for (int index = 0; index <= 851; ++index) {
        inter_dispatchers_[index] = 0;
    }
    for (int index = 0; index <= 851; ++index) {
        db_dispatchers_[index] = 0;
    }
}

// IDA_EA: 0x818FE32
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_ManagerD2Ev
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::~Arad_Dispatcher_Manager()
Arad_Dispatcher_Manager::~Arad_Dispatcher_Manager() {
}

// IDA_EA: 0x818FE38
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager4openEv
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::open(void)
int Arad_Dispatcher_Manager::open() {
    Arad_Dispatcher_BuyItemCheck_Impl* const dispatcher =
        new Arad_Dispatcher_BuyItemCheck_Impl();
    Arad_Dispatcher_Manager* const manager =
        GlobalInstance<Arad_Dispatcher_Manager>::inst();
    manager->register_dispatcher(dispatcher);
    return 0;
}

// IDA_EA: 0x818FE98
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager19register_dispatcherEPNS_15Arad_DispatcherE
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::register_dispatcher(ARAD::Arad_Dispatcher *)
int Arad_Dispatcher_Manager::register_dispatcher(Arad_Dispatcher* dispatcher) {
    inter_dispatchers_[dispatcher->dispathc_code()] = dispatcher;
    db_dispatchers_[dispatcher->dispathc_code()] = dispatcher;
    return 0;
}

// IDA_EA: 0x818FED6
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager21unregister_dispatcherEi
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::unregister_dispatcher(int)
int Arad_Dispatcher_Manager::unregister_dispatcher(int dispatch_code) {
    if (inter_dispatchers_[dispatch_code]) {
        if (db_dispatchers_[dispatch_code]) {
            return 1;
        }
        db_dispatchers_[dispatch_code] = 0;
        return 0;
    }

    inter_dispatchers_[dispatch_code] = 0;
    return 0;
}

// IDA_EA: 0x818FF6C
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager14inter_dispatchEiP5CUserPci
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::inter_dispatch(int,CUser *,char *,int)
int Arad_Dispatcher_Manager::inter_dispatch(int dispatch_code,
                                            CUser* user,
                                            char* payload,
                                            int length) {
    Arad_Dispatcher* const dispatcher = inter_dispatchers_[dispatch_code];
    if (!dispatcher) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatch_Manager.cpp",
            "bool ARAD::Arad_Dispatcher_Manager::inter_dispatch(ARAD::Arad_Dispatcher_Manager::INTERNAL_PACKET_CODE, CUser*, char*, int)",
            0x7d,
            "Not found Inter_Dispatch_Code[%d]",
            dispatch_code);
        return 0;
    }

    const int result = dispatcher->inter_dispatch(user, payload, length);
    if (result == 0) {
        return 1;
    }

    int account_id = 0;
    if (user) {
        account_id = user->get_acc_id();
        user->DisConnSig(static_cast<DISCONN_SIG>(2), true, result + 1000000);
    }
    LogManager::logFormat(
        1,
        "localjapan/Arad_Dispatch_Manager.cpp",
        "bool ARAD::Arad_Dispatcher_Manager::inter_dispatch(ARAD::Arad_Dispatcher_Manager::INTERNAL_PACKET_CODE, CUser*, char*, int)",
        0x8d,
        "Inter Dispatch Result Error Inter_Dispatch_Code[%d] user_acc_id[%d]",
        dispatch_code,
        account_id);
    return 0;
}

// IDA_EA: 0x8190080
// IDA_NAME: _ZN4ARAD23Arad_Dispatcher_Manager11db_dispatchEiiP6Stream
// DEMANGLED: ARAD::Arad_Dispatcher_Manager::db_dispatch(int,int,Stream *)
int Arad_Dispatcher_Manager::db_dispatch(int dispatch_code,
                                         int user_id,
                                         Stream* stream) {
    Arad_Dispatcher* const dispatcher = db_dispatchers_[dispatch_code];
    if (!dispatcher) {
        return 0;
    }

    const int result = dispatcher->db_dispatch(dispatch_code, user_id, stream);
    if (result != 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Dispatch_Manager.cpp",
            "bool ARAD::Arad_Dispatcher_Manager::db_dispatch(int, int, Stream*)",
            0xab,
            "[ERROR] DB Dispatch Result dipCode[%d] uid[%d], ErrLine(%d)",
            dispatch_code,
            user_id,
            result);
    }
    return 1;
}

}  // namespace ARAD


// IDA_EA: 0x818E944
// IDA_NAME: _ZN14GlobalInstanceIN4ARAD23Arad_Dispatcher_ManagerEE4instEv
// DEMANGLED: GlobalInstance<ARAD::Arad_Dispatcher_Manager>::inst(void)
template <>
ARAD::Arad_Dispatcher_Manager*
GlobalInstance<ARAD::Arad_Dispatcher_Manager>::inst() {
    GlobalInstance<ARAD::Arad_Dispatcher_Manager>::create();
    return m_p;
}

// IDA_EA: 0x818E956
// IDA_NAME: _ZN14GlobalInstanceIN4ARAD23Arad_Dispatcher_ManagerEE6createEv
// DEMANGLED: GlobalInstance<ARAD::Arad_Dispatcher_Manager>::create(void)
template <>
ARAD::Arad_Dispatcher_Manager*
GlobalInstance<ARAD::Arad_Dispatcher_Manager>::create() {
    ARAD::Arad_Dispatcher_Manager* result = m_p;
    if (!m_p) {
        result = new ARAD::Arad_Dispatcher_Manager();
        m_p = result;
    }
    return result;
}
