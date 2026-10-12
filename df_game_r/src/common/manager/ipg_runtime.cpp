#include "common/manager/ipg_runtime.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstdio>
#include <cstring>
#include <new>
#include <unistd.h>

namespace {

const uint32_t kIPGPoolMagic = 0x8f21u;
const char kIPGGameCode[] = "DUNF";
const char kIPGEmptyText[] = "";
const char kCeraPointDescription[] = "cera point";

template <size_t N>
void CopyIPGText(char (&dest)[N], const char* source) {
    std::strncpy(dest, source, N);
}

void CopyIPGHeader(WongWork::IPG::SIPGData* dest,
                   const WongWork::IPG::SIPGData* source,
                   WongWork::IPG::IPG_DATA_TYPE type) {
    dest->wire_size = source->wire_size;
    std::strncpy(dest->endpoint_ip, source->endpoint_ip,
                 sizeof(dest->endpoint_ip));
    dest->endpoint_port = source->endpoint_port;
    dest->data_type = type;
    dest->queue_result = source->queue_result;
}

}  // namespace

// IDA_EA: 0x814657C
// IDA_NAME: _Z15InitBillingCashRPN6Taiwan10TaiwanCashE
// DEMANGLED: InitBillingCash(Taiwan::TaiwanCash *&)
bool InitBillingCash(Taiwan::TaiwanCash*& billing_cash) {
    CEnvironment* const environment = G_CEnvironment();
    Taiwan::TaiwanCash* const created = new Taiwan::TaiwanCash(
        static_cast<unsigned char>(environment->get_server_group()), 1);
    billing_cash = created;
    // IDA 0x814657C intentionally obtains handle 14 with operator[].  The
    // original startup path therefore preserves the environment map's exact
    // lookup/insertion semantics instead of introducing a fail-closed branch.
    STDBConnInfo& connection = environment->db_conn_info_map()[
        static_cast<ENUM_DB_HANDLE_IDX>(14)];
    // IDA 0x814664C reports success when allocation yielded null, or when
    // TaiwanCash::init returns precisely 1.  Other nonzero values fail.
    return !billing_cash || billing_cash->init(
        connection.db_ip, connection.db_port, connection.db_name,
        connection.db_userid, connection.db_passwd) == 1;
}

namespace WongWork {
namespace IPG {

// IDA_EA: 0x80EE3DE
// IDA_NAME: _ZN8WongWork3IPG13stIPGStatus_tC2Ev
// DEMANGLED: WongWork::IPG::stIPGStatus_t::stIPGStatus_t(void)
stIPGStatus_t::stIPGStatus_t() : remain(0), size(0), request_count(0) {}

}  // namespace IPG
}  // namespace WongWork

// IDA_EA: 0x81021D6
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE5CNodeC2Ev
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::CNode::CNode(void)
template <>
DynamicPool<WongWork::IPG::SIPGData>::CNode::CNode()
    : WongWork::IPG::SIPGData(),
      dummy(kIPGPoolMagic),
      reference_count(0) {}

// SIPGData has no non-trivial members, so the original array-node teardown is
// empty.  It is nevertheless an out-of-line template member: CIPGMgr D2
// invokes it through delete[] for every allocated 1000-node block.
template <>
DynamicPool<WongWork::IPG::SIPGData>::CNode::~CNode() {}

// The manager C2 calls MoreAlloc directly.  Declare its explicit
// specialization before defining C2 so GCC does not instantiate the shared
// creature/avatar template body at that call site.
template <>
int DynamicPool<WongWork::IPG::SIPGData>::MoreAlloc(int count);

// IDA_EA: 0x81013EC
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEEC2Ei
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::DynamicPool(int)
template <>
DynamicPool<WongWork::IPG::SIPGData>::DynamicPool(int initial_count)
    : exhausted_callback_(0),
      error_state_(0),
      free_nodes_(),
      allocated_blocks_(),
      total_node_count_(0),
      cleared_bookkeeping_(0),
      initial_block_count_(static_cast<uint16_t>(initial_count)),
      growth_block_count_(static_cast<uint16_t>(initial_count)),
      allocated_node_map_() {
    allocated_blocks_.clear();
    MoreAlloc(initial_count);
    error_state_ = 0;
    exhausted_callback_ = 0;
}

// IDA_EA: 0x810150E
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEED2Ev
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::~DynamicPool()
template <>
DynamicPool<WongWork::IPG::SIPGData>::~DynamicPool() {
    for (std::list<CNode*>::iterator it = allocated_blocks_.begin();
         it != allocated_blocks_.end(); ++it) {
        delete[] *it;
    }
    allocated_blocks_.clear();
    exhausted_callback_ = 0;
}

// IDA_EA: 0x81019C2
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE9MoreAllocEi
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::MoreAlloc(int)
template <>
int DynamicPool<WongWork::IPG::SIPGData>::MoreAlloc(int count) {
    CNode* const allocation = new CNode[count];
    if (!allocation) {
        return 0;
    }

    allocated_blocks_.push_back(allocation);
    for (int index = 0; index < count; ++index) {
        CNode* const node = allocation + index;
        free_nodes_.push(node);
        allocated_node_map_[node] = 1;
    }
    total_node_count_ += count;
    return 1;
}

// IDA_EA: 0x810547E
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE7AcquireEv
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::Acquire(void)
template <>
WongWork::IPG::SIPGData* DynamicPool<WongWork::IPG::SIPGData>::Acquire() {
    if (free_nodes_.empty()) {
        MoreAlloc(growth_block_count_);
        if (exhausted_callback_) {
            exhausted_callback_();
        }
    }
    if (free_nodes_.empty()) {
        return 0;
    }

    CNode* const node = free_nodes_.front();
    free_nodes_.pop();
    if (node->dummy != kIPGPoolMagic) {
        error_state_ = 3;
    }
    if (node->reference_count != 0) {
        error_state_ = 4;
    }
    ++node->reference_count;
    return static_cast<WongWork::IPG::SIPGData*>(node);
}

// IDA_EA: 0x81056D4
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE4FreeEPS2_
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::Free(WongWork::IPG::SIPGData*)
template <>
int DynamicPool<WongWork::IPG::SIPGData>::Free(
    WongWork::IPG::SIPGData* data) {
    if (!data) {
        return 0;
    }

    CNode* const node = static_cast<CNode*>(data);
    free_nodes_.push(node);
    if (node->dummy != kIPGPoolMagic) {
        error_state_ = 5;
    }
    if (node->reference_count != 1) {
        error_state_ = 6;
    }
    --node->reference_count;
    return 1;
}

// IDA_EA: 0x81012B6
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE6RemainEv
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::Remain(void)
template <>
int DynamicPool<WongWork::IPG::SIPGData>::Remain() {
    return static_cast<int>(free_nodes_.size());
}

// IDA_EA: 0x81012CC
// IDA_NAME: _ZN11DynamicPoolIN8WongWork3IPG8SIPGDataEE4SizeEv
// DEMANGLED: DynamicPool<WongWork::IPG::SIPGData>::Size(void)
template <>
int DynamicPool<WongWork::IPG::SIPGData>::Size() {
    return static_cast<int>(total_node_count_);
}

namespace WongWork {
namespace IPG {

// IDA_EA: 0x81053E2
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorkerC2Ev
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::CIPGWorker(void)
CIPGMgr::CIPGWorker::CIPGWorker() : ThreadInterface() {}

// IDA_EA: 0x81053FE / 0x810542E
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorkerD2Ev
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::~CIPGWorker()
CIPGMgr::CIPGWorker::~CIPGWorker() {}

// IDA_EA: 0x81053D4
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorker9SetIPGMgrEPS1_
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::SetIPGMgr(WongWork::IPG::CIPGMgr *)
CIPGMgr::CIPGWorker* CIPGMgr::CIPGWorker::SetIPGMgr(CIPGMgr* manager) {
    manager_ = manager;
    return this;
}

// IDA_EA: 0x81048B4
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorker19ProcIPGSyncForChinaEPNS0_8SIPGDataE
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::ProcIPGSyncForChina(WongWork::IPG::SIPGData *)
SIPGData* CIPGMgr::CIPGWorker::ProcIPGSyncForChina(SIPGData* request) {
    (void)request;
    return 0;
}

// IDA_EA: 0x81048C6
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorker20ProcIPGSyncForTaiwanEPNS0_8SIPGDataE
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::ProcIPGSyncForTaiwan(WongWork::IPG::SIPGData *)
SIPGData* CIPGMgr::CIPGWorker::ProcIPGSyncForTaiwan(SIPGData* request) {
    Taiwan::TaiwanCash* const cash = manager_->billing_cash_;
    switch (request->data_type) {
    case IPG_INPUT: {
        Taiwan::BillTransaction transaction;
        const int result = cash->charge(
            request->wire.input.account_header.normal.account_id,
            request->wire.input.charge_kind,
            0, request->wire.input.price, const_cast<char*>(""),
            const_cast<char*>(""), const_cast<char*>(""),
            request->wire.input.account_header.charge.character_name, 0,
            transaction);
        if (result == 0) {
            cash->confirm(transaction);
        }
        return 0;
    }
    case IPG_OUTPUT: {
        const int product_id = std::atoi(request->wire.output.product_id_decimal);
        const int price = std::atoi(request->wire.output.price_decimal);
        Taiwan::BillTransaction transaction;
        const int result = cash->purchase(
            0, product_id, price, request->wire.output.account_name,
            request->wire.output.character_name, transaction);
        if (result != 0) {
            return 0;
        }

        SIPGData* const response = manager_->AllocIPGData();
        CopyIPGHeader(response, request, IPG_OUTPUT);
        std::memset(&response->wire.output, 0, sizeof(response->wire.output));
        std::strcpy(response->wire.output.account_id,
                    request->wire.output.account_id);
        std::strcpy(response->wire.output.account_name,
                    request->wire.output.account_name);
        std::strcpy(response->wire.output.game_code,
                    request->wire.output.game_code);
        std::strcpy(response->wire.output.product_id_decimal,
                    request->wire.output.product_id_decimal);
        std::strcpy(response->wire.output.price_decimal,
                    request->wire.output.price_decimal);
        std::strcpy(response->wire.output.order_id,
                    request->wire.output.order_id);
        std::strcpy(response->wire.output.product_name,
                    request->wire.output.product_name);
        std::strcpy(response->wire.output.product_description,
                    request->wire.output.product_description);
        response->wire.output.transaction = transaction;
        std::snprintf(response->wire.output.result_code_decimal,
                      sizeof(response->wire.output.result_code_decimal), "%d",
                      result);
        response->callback_stream = request->callback_stream;
        return response;
    }
    case IPG_OUTPUT_CANCEL:
        cash->cancel(request->wire.output_cancel.transaction);
        return 0;
    case IPG_QUERY: {
        int balance = 0;
        int point = 0;
        const int result = cash->getBalance(request->wire.query.account_name,
                                            balance, point);
        if (result != 0) {
            return 0;
        }
        SIPGData* const response = manager_->AllocIPGData();
        CopyIPGHeader(response, request, IPG_QUERY);
        std::memset(&response->wire.query, 0, sizeof(response->wire.query));
        CopyIPGText(response->wire.query.account_id,
                    request->wire.query.account_id);
        CopyIPGText(response->wire.query.account_name,
                    request->wire.query.account_name);
        CopyIPGText(response->wire.query.game_code,
                    request->wire.query.game_code);
        std::memset(response->wire.query.balance_decimal,
                    0,
                    sizeof(response->wire.query.balance_decimal));
        std::snprintf(response->wire.query.balance_decimal[0],
                      sizeof(response->wire.query.balance_decimal[0]),
                      "%d",
                      balance);
        std::snprintf(response->wire.query.result_code_decimal,
                      sizeof(response->wire.query.result_code_decimal), "%d", result);
        std::snprintf(response->wire.query.point_decimal,
                      sizeof(response->wire.query.point_decimal), "%d", point);
        response->callback_stream = request->callback_stream;
        return response;
    }
    case IPG_OUTPUT_CONFIRM:
        cash->confirm(request->wire.output_cancel.transaction);
        return 0;
    case IPG_INPUT_POINT: {
        const int result = cash->ceraPoint(
            request->wire.cera_point.account_name,
            request->wire.cera_point.character_name,
            request->wire.cera_point.command,
            request->wire.cera_point.point_value,
            request->wire.cera_point.point_context,
            request->wire.cera_point.charge_kind,
            request->wire.cera_point.point_kind,
            static_cast<int>(request->wire.cera_point.price));
        if (result != 0) {
            std::snprintf(request->wire.cera_point.result_code_decimal,
                          sizeof(request->wire.cera_point.result_code_decimal),
                          "%d", result);
            return 0;
        }
        if (request->wire.cera_point.command == 'A') {
            return 0;
        }

        SIPGData* const response = manager_->AllocIPGData();
        // The original changes the request type before emitting the output
        // payload; IPGResult subsequently propagates that exact type.
        request->data_type = IPG_OUTPUT;
        CopyIPGHeader(response, request, IPG_OUTPUT);
        std::memset(&response->wire.output, 0, sizeof(response->wire.output));
        CopyIPGText(response->wire.output.product_description,
                    kCeraPointDescription);
        CopyIPGText(response->wire.output.account_name,
                    request->wire.cera_point.account_name);
        CopyIPGText(response->wire.output.product_id_decimal,
                    request->wire.cera_point.point_value);
        CopyIPGText(response->wire.output.order_id,
                    request->wire.cera_point.point_context);
        std::snprintf(response->wire.output.price_decimal,
                      sizeof(response->wire.output.price_decimal), "%d",
                      request->wire.cera_point.price);
        std::snprintf(response->wire.output.result_code_decimal,
                      sizeof(response->wire.output.result_code_decimal), "%d",
                      result);
        response->callback_stream = request->callback_stream;
        return response;
    }
    default:
        return 0;
    }
}

// IDA_EA: 0x8105258
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorker11ProcIPGSyncEPNS0_8SIPGDataE
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::ProcIPGSync(WongWork::IPG::SIPGData *)
SIPGData* CIPGMgr::CIPGWorker::ProcIPGSync(SIPGData* request) {
    return ProcIPGSyncForTaiwan(request);
}

// IDA_EA: 0x8105276
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10CIPGWorker8dispatchEPv
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGWorker::dispatch(void *)
void CIPGMgr::CIPGWorker::dispatch(void* user_data) {
    (void)user_data;
    while (!GlobalData::server_stop()) {
        SIPGData* const request = manager_->_GetIPGRequest();
        if (!request) {
            ::usleep(10000);
            continue;
        }
        SIPGData* const response = ProcIPGSync(request);
        manager_->GetIPGHelper()->IPGResult(request, response);
        if (response) {
            manager_->FreeIPGData(response);
        }
        manager_->FreeIPGData(request);
    }
}

// IDA_EA: 0x8101108
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgrC2EPNS0_19IIPGHelperInterfaceE
// DEMANGLED: WongWork::IPG::CIPGMgr::CIPGMgr(WongWork::IPG::IIPGHelperInterface *)
CIPGMgr::CIPGMgr(IIPGHelperInterface* helper)
    : workers_(0),
      request_queue_(),
      response_queue_(),
      data_pool_(1000),
      helper_(helper),
      data_mutex_(),
      request_mutex_() {}

// IDA_EA: 0x8101222
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgrD2Ev
// DEMANGLED: WongWork::IPG::CIPGMgr::~CIPGMgr()
CIPGMgr::~CIPGMgr() {
    // IDA_EA 0x8101222 destroys only the two mutexes, pool and queues.  The
    // caller invokes Finalize before this destructor; TaiwanCash is not
    // reclaimed by the original manager destructor.
}

// IDA_EA: 0x8104494
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr10InitializeEjj
// DEMANGLED: WongWork::IPG::CIPGMgr::Initialize(unsigned int, unsigned int)
int CIPGMgr::Initialize(unsigned int worker_count, unsigned int worker_argument) {
    if (workers_) {
        return -1;
    }
    if (worker_count) {
        workers_ = new CIPGWorker[worker_count];
        // 0x8104564 intentionally uses workers_ on every iteration rather
        // than advancing by the 0x10-byte worker stride.  Preserve the
        // original first-worker start sequence.
        for (unsigned int index = 0; index < worker_count; ++index) {
            workers_->SetIPGMgr(this);
            workers_->begin();
        }
    }
    worker_argument_ = worker_argument;
    worker_count_ = worker_count;
    return ::InitBillingCash(billing_cash_) ? 0 : -1;
}

// IDA_EA: 0x8104600
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr8FinalizeEv
// DEMANGLED: WongWork::IPG::CIPGMgr::Finalize(void)
CIPGMgr* CIPGMgr::Finalize() {
    if (!workers_) {
        return 0;
    }
    // Likewise, 0x810460F invokes virtual stop on workers_ repeatedly before
    // delete[] runs every worker destructor.
    for (uint32_t index = 0; index < worker_count_; ++index) {
        workers_->stop();
    }
    delete[] workers_;
    workers_ = 0;
    return this;
}

// IDA_EA: 0x810466C
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr12AllocIPGDataEv
// DEMANGLED: WongWork::IPG::CIPGMgr::AllocIPGData(void)
SIPGData* CIPGMgr::AllocIPGData() {
    MutexGuard guard(&data_mutex_);
    SIPGData* const data = data_pool_.Acquire();
    if (data) {
        std::memset(data, 0, sizeof(*data));
    }
    return data;
}

// IDA_EA: 0x81046F0
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr11FreeIPGDataEPNS0_8SIPGDataE
// DEMANGLED: WongWork::IPG::CIPGMgr::FreeIPGData(WongWork::IPG::SIPGData *)
int CIPGMgr::FreeIPGData(SIPGData* data) {
    MutexGuard guard(&data_mutex_);
    return data_pool_.Free(data);
}

// IDA_EA: 0x8104754
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr13PutIPGRequestEPNS0_8SIPGDataEb
// DEMANGLED: WongWork::IPG::CIPGMgr::PutIPGRequest(WongWork::IPG::SIPGData *, bool)
int CIPGMgr::PutIPGRequest(SIPGData* request, bool async) {
    if (!async || !workers_) {
        return static_cast<int>(reinterpret_cast<intptr_t>(
            workers_->ProcIPGSync(request)));
    }
    MutexGuard guard(&request_mutex_);
    request_queue_.push(request);
    return 0;
}

// IDA_EA: 0x81047EA
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr14_GetIPGRequestEv
// DEMANGLED: WongWork::IPG::CIPGMgr::_GetIPGRequest(void)
SIPGData* CIPGMgr::_GetIPGRequest() {
    MutexGuard guard(&request_mutex_);
    if (request_queue_.empty()) {
        return 0;
    }
    SIPGData* const request = request_queue_.front();
    if (request) {
        request_queue_.pop();
    }
    return request;
}

// IDA_EA: 0x81010AE
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr12getIPGStatusERNS0_13stIPGStatus_tE
// DEMANGLED: WongWork::IPG::CIPGMgr::getIPGStatus(WongWork::IPG::stIPGStatus_t &)
stIPGStatus_t& CIPGMgr::getIPGStatus(stIPGStatus_t& status) {
    status.remain = data_pool_.Remain();
    status.size = data_pool_.Size();
    status.request_count = static_cast<int>(request_queue_.size());
    return status;
}

// IDA_EA: 0x81053C6
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr12GetIPGHelperEv
// DEMANGLED: WongWork::IPG::CIPGMgr::GetIPGHelper(void)
CIPGHelper* CIPGMgr::GetIPGHelper() {
    return static_cast<CIPGHelper*>(helper_);
}

// IDA_EA: 0x817A2EE
// IDA_NAME: _ZN8WongWork3IPG7CIPGMgr14getBillingCashEv
// DEMANGLED: WongWork::IPG::CIPGMgr::getBillingCash(void)
Taiwan::TaiwanCash* CIPGMgr::getBillingCash() {
    return billing_cash_;
}

// IDA_EA: 0x82A7182
// IDA_NAME: _ZN8WongWork3IPG19IIPGHelperInterfaceC2Ev
// DEMANGLED: WongWork::IPG::IIPGHelperInterface::IIPGHelperInterface(void)
IIPGHelperInterface::IIPGHelperInterface() {}

// IDA_EA: 0x82A7190 / 0x82A71B6
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelperC2Ev
// DEMANGLED: WongWork::IPG::CIPGHelper::CIPGHelper(void)
CIPGHelper::CIPGHelper() : ipg_manager_(0) {}

// IDA_EA: 0x82A71B6
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelperD2Ev
// DEMANGLED: WongWork::IPG::CIPGHelper::~CIPGHelper()
CIPGHelper::~CIPGHelper() {}

// IDA_EA: 0x80FFAE0
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper12getIPGStatusERNS0_13stIPGStatus_tE
// DEMANGLED: WongWork::IPG::CIPGHelper::getIPGStatus(WongWork::IPG::stIPGStatus_t &)
stIPGStatus_t* CIPGHelper::getIPGStatus(stIPGStatus_t& status) {
    return ipg_manager_ ? &ipg_manager_->getIPGStatus(status) : 0;
}

SIPGData* CIPGHelper::_SetIPGData(CUser* user, IPG_DATA_TYPE type,
                                   bool queue_result) {
    SIPGData* const data = ipg_manager_->AllocIPGData();
    const char* const account_id = NumberToString(
        static_cast<unsigned int>(user->get_acc_id()), 0);
    std::snprintf(data->wire.input.account_header.normal.account_id,
                  sizeof(data->wire.input.account_header.normal.account_id), "%s",
                  account_id);
    CopyIPGText(data->wire.input.account_header.normal.account_name,
                user->get_acc_name());
    CopyIPGText(data->wire.input.account_header.normal.game_code, kIPGGameCode);
    data->wire_size = wire_sizes_[type];
    std::strncpy(data->endpoint_ip, ipg_endpoint_, sizeof(data->endpoint_ip));
    data->endpoint_port = static_cast<uint16_t>(ports_[type]);
    data->user_uid = user->GetUID();
    data->data_type = type;
    data->queue_result = queue_result ? 1 : 0;
    return data;
}

// IDA_EA: 0x80FFB0A
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper8IPGInputEP5CUserPKcS5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGInput(CUser *, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGInput(CUser* user, const char* source, const char* source2,
                         const char* source3, const char* source4,
                         const char* source5, void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_INPUT, true);
    CopyIPGText(data->wire.input.event_ipg_code, source);
    CopyIPGText(data->wire.input.amount_decimal, source2);
    CopyIPGText(data->wire.input.unique_transaction_id, source3);
    if (source4) {
        CopyIPGText(data->wire.input.event_name, source4);
    }
    if (source5) {
        CopyIPGText(data->wire.input.character_name, source5);
    }
    CopyIPGText(data->wire.input.user_name, user->GetUserName());
    CopyIPGText(data->wire.input.ssn, user->GetSsnString());
    CopyIPGText(data->wire.input.email, user->GetUserEMail());
    std::memset(data->wire.input.input_reserved, 0, 7u);
    data->callback_stream = static_cast<Stream*>(callback);
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x80FFCA4
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper8IPGInputEP5CUserhjPKcS5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGInput(CUser *, unsigned char, unsigned int, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGInput(CUser* user, unsigned char charge_kind,
                         unsigned int price, const char* source,
                         const char* source2, const char* source3,
                         const char* source4, const char* source5,
                         void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_INPUT, true);
    CopyIPGText(data->wire.input.event_ipg_code, source);
    CopyIPGText(data->wire.input.amount_decimal, source2);
    CopyIPGText(data->wire.input.unique_transaction_id, source3);
    if (source4) {
        CopyIPGText(data->wire.input.event_name, source4);
    }
    if (source5) {
        CopyIPGText(data->wire.input.character_name, source5);
    }
    CopyIPGText(data->wire.input.user_name, user->GetUserName());
    CopyIPGText(data->wire.input.ssn, user->GetSsnString());
    CopyIPGText(data->wire.input.email, user->GetUserEMail());
    std::memset(data->wire.input.input_reserved, 0, 7u);
    std::strncpy(data->wire.input.account_header.charge.account_name,
                 user->get_acc_name(), 32u);
    std::strncpy(data->wire.input.account_header.charge.character_name,
                 user->getCurCharacName(), 30u);
    data->wire.input.charge_kind = charge_kind;
    data->wire.input.price = price;
    data->callback_stream = static_cast<Stream*>(callback);
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x80FFEA8
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper14IPGInputCancelEP5CUserPKcS5_S5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGInputCancel(CUser *, char const*, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGInputCancel(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               const char* source6, void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_INPUT_CANCEL, false);
    CopyIPGText(data->wire.input_cancel.cancel_code, source);
    CopyIPGText(data->wire.input_cancel.product_id_decimal, source2);
    CopyIPGText(data->wire.input_cancel.price_decimal, source3);
    CopyIPGText(data->wire.input_cancel.order_id, source4);
    if (source5) {
        CopyIPGText(data->wire.input_cancel.product_name, source5);
    }
    if (source6) {
        CopyIPGText(data->wire.input_cancel.product_description, source6);
    }
    data->callback_stream = static_cast<Stream*>(callback);
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x80FFFC0
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper13IPGInputPointEP5CUserjccPv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGInputPoint(CUser *, unsigned int, char, char, void *)
int CIPGHelper::IPGInputPoint(CUser* user, unsigned int price,
                              char charge_kind, char point_kind,
                              void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_INPUT_POINT, true);
    std::sprintf(data->wire.cera_point.account_id, "%d", user->get_acc_id());
    CopyIPGText(data->wire.cera_point.account_name, user->get_acc_name());
    CopyIPGText(data->wire.cera_point.character_name, user->getCurCharacName());
    CopyIPGText(data->wire.cera_point.point_value, kIPGEmptyText);
    CopyIPGText(data->wire.cera_point.point_context, kIPGEmptyText);
    data->wire.cera_point.price = price;
    data->wire.cera_point.command = 'A';
    data->wire.cera_point.charge_kind = charge_kind;
    data->wire.cera_point.point_kind = point_kind;
    (void)callback;
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x81000F6
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper14IPGOutputPointEP5CUserPKcS5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputPoint(CUser *, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGOutputPoint(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_INPUT_POINT, true);
    std::sprintf(data->wire.cera_point.account_id, "%d", user->get_acc_id());
    CopyIPGText(data->wire.cera_point.account_name, user->get_acc_name());
    CopyIPGText(data->wire.cera_point.character_name, user->getCurCharacName());
    CopyIPGText(data->wire.cera_point.point_value, source);
    CopyIPGText(data->wire.cera_point.point_context, source3);
    data->wire.cera_point.price = static_cast<uint32_t>(std::atoi(source2));
    data->wire.cera_point.command = 'U';
    data->wire.cera_point.charge_kind = 0;
    data->wire.cera_point.point_kind = 0;
    data->callback_stream = static_cast<Stream*>(callback);
    (void)source4;
    (void)source5;
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x810022E
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper9IPGOutputEP5CUserPKcS5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutput(CUser *, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGOutput(CUser* user, const char* source,
                          const char* source2, const char* source3,
                          const char* source4, const char* source5,
                          void* callback) {
    SIPGData* const data = _SetIPGData(user, IPG_OUTPUT, true);
    CopyIPGText(data->wire.output.product_id_decimal, source);
    CopyIPGText(data->wire.output.price_decimal, source2);
    CopyIPGText(data->wire.output.order_id, source3);
    if (source4) {
        CopyIPGText(data->wire.output.product_name, source4);
    }
    if (source5) {
        CopyIPGText(data->wire.output.product_description, source5);
    }
    CopyIPGText(data->wire.output.character_name, user->getCurCharacName());
    data->callback_stream = static_cast<Stream*>(callback);
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x810034E
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper19IPGOutputSimulationEP5CUserPKcS5_S5_S5_S5_Pv
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputSimulation(CUser *, char const*, char const*, char const*, char const*, char const*, void *)
int CIPGHelper::IPGOutputSimulation(CUser* user, const char* source,
                                    const char* source2, const char* source3,
                                    const char* source4, const char* source5,
                                    void* callback) {
    SIPGData* const request = _SetIPGData(user, IPG_OUTPUT, true);
    CopyIPGText(request->wire.output.product_id_decimal, source);
    CopyIPGText(request->wire.output.price_decimal, source2);
    CopyIPGText(request->wire.output.order_id, source3);
    if (source4) {
        CopyIPGText(request->wire.output.product_name, source4);
    }
    if (source5) {
        CopyIPGText(request->wire.output.product_description, source5);
    }
    request->callback_stream = static_cast<Stream*>(callback);

    SIPGData* const result = ipg_manager_->AllocIPGData();
    // 0x810034E copies exactly request->wire_size bytes from SIPGData +0x28
    // and only then writes the wire size/type header words.  In particular it
    // does not inherit endpoint, UID, or queue_result from the request.
    std::memcpy(&result->wire, &request->wire, request->wire_size);
    CopyIPGText(result->wire.output.result_code_decimal, "0");
    result->wire_size = request->wire_size;
    result->data_type = request->data_type;
    IPGResult(request, result);
    ipg_manager_->FreeIPGData(request);
    ipg_manager_->FreeIPGData(result);
    return static_cast<int>(reinterpret_cast<intptr_t>(request));
}

// IDA_EA: 0x81004DE
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper15IPGOutputCancelEP5CUserPKcS5_S5_S5_S5_S5_PvPN6Taiwan15BillTransactionE
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputCancel(CUser *, char const*, char const*, char const*, char const*, char const*, char const*, void *, Taiwan::BillTransaction *)
int CIPGHelper::IPGOutputCancel(CUser* user, const char* source,
                                const char* source2, const char* source3,
                                const char* source4, const char* source5,
                                const char* source6, void* callback,
                                Taiwan::BillTransaction* transaction) {
    SIPGData* const data = _SetIPGData(user, IPG_OUTPUT_CANCEL, false);
    CopyIPGText(data->wire.output_cancel.cancel_code, source);
    CopyIPGText(data->wire.output_cancel.product_id_decimal, source2);
    CopyIPGText(data->wire.output_cancel.price_decimal, source3);
    CopyIPGText(data->wire.output_cancel.order_id, source4);
    if (source5) {
        CopyIPGText(data->wire.output_cancel.product_name, source5);
    }
    if (source6) {
        CopyIPGText(data->wire.output_cancel.product_description, source6);
    }
    data->callback_stream = static_cast<Stream*>(callback);
    data->wire.output_cancel.transaction = *transaction;
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x8100616
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper15IPGOutputCancelEPNS0_8SIPGDataES3_
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputCancel(WongWork::IPG::SIPGData *, WongWork::IPG::SIPGData *)
int CIPGHelper::IPGOutputCancel(SIPGData* request, SIPGData* result) {
    SIPGData* const cancel = ipg_manager_->AllocIPGData();
    // 0x8100616 deliberately leaves the allocated header clear, copies only
    // the 0x41-byte common wire prefix, and clears queue_result.  Do not
    // normalize this into an output-cancel request; callers observe the old
    // serialized payload exactly as constructed.
    std::memcpy(&cancel->wire, &request->wire, 0x41u);
    CopyIPGText(cancel->wire.output_cancel.cancel_code,
                result->wire.output.cancel_code_from_result());
    CopyIPGText(cancel->wire.output_cancel.product_id_decimal,
                request->wire.output.product_id_decimal);
    CopyIPGText(cancel->wire.output_cancel.price_decimal,
                request->wire.output.price_decimal);
    CopyIPGText(cancel->wire.output_cancel.order_id, request->wire.output.order_id);
    CopyIPGText(cancel->wire.output_cancel.product_name,
                request->wire.output.product_name);
    CopyIPGText(cancel->wire.output_cancel.product_description,
                request->wire.output.product_description);
    cancel->wire.output_cancel.transaction = result->wire.output.transaction;
    cancel->queue_result = 0;
    return ipg_manager_->PutIPGRequest(cancel, true);
}

// IDA_EA: 0x8100790
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper8IPGQueryEP5CUser
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGQuery(CUser *)
int CIPGHelper::IPGQuery(CUser* user) {
    SIPGData* const data = _SetIPGData(user, IPG_QUERY, true);
    data->callback_stream = 0;
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x81007E8
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper10IPGRequestEPNS0_8SIPGDataE
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGRequest(WongWork::IPG::SIPGData *)
int CIPGHelper::IPGRequest(SIPGData* request) {
    request->wire_size = wire_sizes_[request->data_type];
    std::strncpy(request->endpoint_ip, ipg_endpoint_,
                 sizeof(request->endpoint_ip));
    request->endpoint_port = static_cast<uint16_t>(ports_[request->data_type]);
    return ipg_manager_->PutIPGRequest(request, true);
}

// IDA_EA: 0x810085E
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper9IPGResultEPNS0_8SIPGDataES3_
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGResult(WongWork::IPG::SIPGData *, WongWork::IPG::SIPGData *)
void CIPGHelper::IPGResult(SIPGData* request, SIPGData* result) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("IPGHelper.cpp", 406), true);
    guard << 72;
    guard << -1;
    if (result) {
        std::memcpy(&result->wire, &request->wire, 0x41u);
        result->user_uid = request->user_uid;
        result->callback_stream = request->callback_stream;
        result->data_type = request->data_type;
        // Official 0627 ELF 0x810093A calls CStreamGuard::operator<<(char).
        // Inter_IPGResult consumes the following SIPGData at payload+1, so
        // selecting the int overload shifts both result and request by three
        // bytes and corrupts the embedded user UID.
        guard << static_cast<char>(1);
        guard->put_binary(result, sizeof(*result));
    } else {
        // Official 0627 ELF 0x810099B uses the same one-byte flag here.
        guard << static_cast<char>(0);
    }
    guard->put_binary(request, sizeof(*request));
    if (request->queue_result) {
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
        return;
    }
    GlobalData::s_stream_pool->Free(guard);
}

// IDA_EA: 0x8100B3E
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper10InitializeEjj
// DEMANGLED: WongWork::IPG::CIPGHelper::Initialize(unsigned int, unsigned int)
int CIPGHelper::Initialize(unsigned int worker_count,
                           unsigned int worker_argument) {
    if (ipg_manager_) {
        Finalize();
    }
    wire_sizes_[IPG_INPUT] = 0x2b0u;
    wire_sizes_[IPG_INPUT_CANCEL] = 0x1cbu;
    wire_sizes_[IPG_OUTPUT] = 0x1fcu;
    wire_sizes_[IPG_OUTPUT_CANCEL] = 0x1d4u;
    wire_sizes_[IPG_QUERY] = 0x099u;
    wire_sizes_[IPG_OUTPUT_CONFIRM] = 0x1d4u;
    wire_sizes_[IPG_INPUT_POINT] = 0x090u;
    const char* const ipg_ip = G_CEnvironment()->get_ipg_ip();
    if (!*ipg_ip) {
        return -1;
    }
    std::strncpy(ipg_endpoint_, ipg_ip, sizeof(ipg_endpoint_));
    ports_[IPG_INPUT] = 23100;
    ports_[IPG_INPUT_CANCEL] = 23200;
    ports_[IPG_OUTPUT] = 23300;
    ports_[IPG_OUTPUT_CANCEL] = 23400;
    ports_[IPG_QUERY] = 23500;
    ports_[IPG_OUTPUT_CONFIRM] = 9006;
    ports_[IPG_INPUT_POINT] = 0;
    ipg_manager_ = new CIPGMgr(this);
    return ipg_manager_->Initialize(worker_count, worker_argument);
}

// IDA_EA: 0x8100C92
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper8FinalizeEv
// DEMANGLED: WongWork::IPG::CIPGHelper::Finalize(void)
CIPGHelper* CIPGHelper::Finalize() {
    ipg_manager_->Finalize();
    delete ipg_manager_;
    ipg_manager_ = 0;
    return this;
}

// IDA_EA: 0x8100CD2
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper16IPGOutputConfirmEP5CUserPKcS5_S5_S5_S5_S5_PvPN6Taiwan15BillTransactionE
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputConfirm(CUser *, char const*, char const*, char const*, char const*, char const*, char const*, void *, Taiwan::BillTransaction *)
int CIPGHelper::IPGOutputConfirm(CUser* user, const char* source,
                                 const char* source2, const char* source3,
                                 const char* source4, const char* source5,
                                 const char* source6, void* callback,
                                 Taiwan::BillTransaction* transaction) {
    SIPGData* const data = _SetIPGData(user, IPG_OUTPUT_CONFIRM, false);
    CopyIPGText(data->wire.output_cancel.cancel_code, source);
    CopyIPGText(data->wire.output_cancel.product_id_decimal, source2);
    CopyIPGText(data->wire.output_cancel.price_decimal, source3);
    CopyIPGText(data->wire.output_cancel.order_id, source4);
    if (source5) {
        CopyIPGText(data->wire.output_cancel.product_name, source5);
    }
    if (source6) {
        CopyIPGText(data->wire.output_cancel.product_description, source6);
    }
    data->callback_stream = static_cast<Stream*>(callback);
    data->wire.output_cancel.transaction = *transaction;
    return ipg_manager_->PutIPGRequest(data, true);
}

// IDA_EA: 0x8100E0A
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper16IPGOutputConfirmEPNS0_8SIPGDataES3_
// DEMANGLED: WongWork::IPG::CIPGHelper::IPGOutputConfirm(WongWork::IPG::SIPGData *, WongWork::IPG::SIPGData *)
int CIPGHelper::IPGOutputConfirm(SIPGData* request, SIPGData* result) {
    SIPGData* const confirm = ipg_manager_->AllocIPGData();
    // 0x8100E0A writes this reduced header before copying the same common
    // 0x41-byte wire prefix.  It is distinct from the user-facing confirm
    // request builder above.
    confirm->user_uid = -1;
    confirm->data_type = IPG_OUTPUT_CONFIRM;
    confirm->queue_result = 0;
    std::memcpy(&confirm->wire, &request->wire, 0x41u);
    CopyIPGText(confirm->wire.output_cancel.cancel_code,
                result->wire.output.cancel_code_from_result());
    CopyIPGText(confirm->wire.output_cancel.product_id_decimal,
                request->wire.output.product_id_decimal);
    CopyIPGText(confirm->wire.output_cancel.price_decimal,
                request->wire.output.price_decimal);
    CopyIPGText(confirm->wire.output_cancel.order_id, request->wire.output.order_id);
    CopyIPGText(confirm->wire.output_cancel.product_name,
                request->wire.output.product_name);
    CopyIPGText(confirm->wire.output_cancel.product_description,
                request->wire.output.product_description);
    confirm->wire.output_cancel.transaction = result->wire.output.transaction;
    confirm->queue_result = 0;
    return ipg_manager_->PutIPGRequest(confirm, true);
}

// IDA_EA: 0x81010A2
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper9GetIPGMgrEv
// DEMANGLED: WongWork::IPG::CIPGHelper::GetIPGMgr(void)
CIPGMgr* CIPGHelper::GetIPGMgr() {
    return ipg_manager_;
}

}  // namespace IPG
}  // namespace WongWork
