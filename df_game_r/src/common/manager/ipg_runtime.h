#ifndef OLDSRV_GLOBAL_IPG_RUNTIME_H_
#define OLDSRV_GLOBAL_IPG_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>
#include <queue>

#include "old_game/economy/inventory/taiwan_billing_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/concurrent/thread/thread_interface.h"

class CUser;
class Stream;

namespace Taiwan {
struct BillTransaction;
class TaiwanCash;
}

namespace WongWork {
namespace IPG {

// IDA_EA: 0x8100B3E. These values select the seven independently sized
// original IPG wire contracts.
enum IPG_DATA_TYPE {
    IPG_INPUT = 0,
    IPG_INPUT_CANCEL = 1,
    IPG_OUTPUT = 2,
    IPG_OUTPUT_CANCEL = 3,
    IPG_QUERY = 4,
    IPG_OUTPUT_CONFIRM = 5,
    IPG_INPUT_POINT = 6,
};

// IDA_EA: 0x80EE3DE
struct stIPGStatus_t {
    stIPGStatus_t();

    int remain;
    int size;
    int request_count;
};

// All payload layouts below begin at SIPGData + 0x28.  They are distinct IPG
// operations sharing the original 0x2b0-byte wire union, not substitutes for
// a different protocol packet.
struct SIPGInputWire {
    // The generic input header is 70 bytes.  IPGInput's charge overload
    // intentionally writes a 32-byte account name at +0 and a 30-byte
    // character name at +0x28, overlapping the normal account/game fields.
    // Both are genuine views of the same old wire region.
    union AccountHeader {
        struct Normal {
            char account_id[20];
            char account_name[40];
            char game_code[5];
            char input_prefix[5];
        } normal;
        struct Charge {
            char account_name[32];
            char account_overlap[8];
            char character_name[30];
        } charge;
    } account_header;
    char charge_prefix[42];
    uint32_t price;
    char input_prefix[65];
    char event_ipg_code[11];
    char amount_decimal[12];
    char unique_transaction_id[20];
    char event_name[100];
    char character_name[100];
    char user_name[50];
    char ssn[14];
    char email[60];
    char input_reserved[33];
    uint8_t charge_kind;
    // The 0627 IPG_INPUT reply writes the textual charge result at wire
    // +0x246.  It is also the string printed by Inter_IPGResult on a failed
    // asynchronous request; it is therefore a real wire member, not tail
    // padding.
    char charge_result_decimal[106];

};

struct SIPGOutputWire {
    char account_id[20];
    char account_name[40];
    char game_code[5];
    char product_id_decimal[8];
    char price_decimal[12];
    char order_id[20];
    char product_name[100];
    char product_description[100];
    char response_reserved[26];
    char result_code_decimal[4];
    char result_message[100];
    Taiwan::BillTransaction transaction;
    char character_name[58];
    char output_reserved[2];

    // IPGOutputCancel/Confirm(SIPGData*, SIPGData*) copy 21 bytes from the
    // result's absolute +0x16e, which is wire +0x146.  That span overlaps
    // the response-reserved/result-code boundary; it is not result_message.
    char* cancel_code_from_result() { return response_reserved + 21; }
    const char* cancel_code_from_result() const {
        return response_reserved + 21;
    }
};

// IPG_INPUT_CANCEL is a distinct 0x1cb-byte request.  Its 11-byte product
// field moves every following member three bytes past the corresponding
// IPG_OUTPUT_CANCEL / IPG_OUTPUT_CONFIRM request.
struct SIPGInputCancelWire {
    char account_id[20];
    char account_name[40];
    char game_code[5];
    char cancel_code[21];
    char product_id_decimal[11];
    char price_decimal[12];
    char order_id[20];
    char product_name[100];
    char product_description[100];
    char cancel_reserved[130];
};

struct SIPGOutputCancelWire {
    char account_id[20];
    char account_name[40];
    char game_code[5];
    char cancel_code[21];
    char product_id_decimal[8];
    char price_decimal[12];
    char order_id[20];
    char product_name[100];
    char product_description[100];
    char cancel_reserved[130];
    Taiwan::BillTransaction transaction;
};

struct SIPGQueryWire {
    char account_id[20];
    char account_name[40];
    char game_code[5];
    // Six independent, NUL-terminated, twelve-byte decimal balance fields.
    // The original query consumer sums all six sources for the displayed
    // Cera total.
    char balance_decimal[6][12];
    char result_code_decimal[4];
    char point_decimal[12];
};

struct SIPGCeraPointWire {
    char account_id[20];
    char account_name[40];
    char character_name[40];
    char point_value[8];
    char point_context[21];
    char cera_reserved[3];
    uint32_t price;
    char command;
    char charge_kind;
    char point_kind;
    char result_code_decimal[4];
};

union SIPGWireData {
    SIPGInputWire input;
    SIPGOutputWire output;
    SIPGInputCancelWire input_cancel;
    SIPGOutputCancelWire output_cancel;
    SIPGQueryWire query;
    SIPGCeraPointWire cera_point;
};

struct SIPGData {
    uint32_t wire_size;                   // +0x000
    char endpoint_ip[20];                 // +0x004
    uint16_t endpoint_port;               // +0x018
    uint16_t endpoint_reserved;           // +0x01a
    IPG_DATA_TYPE data_type;              // +0x01c
    int32_t user_uid;                     // +0x020
    uint8_t queue_result;                 // +0x024
    uint8_t queue_reserved[3];            // +0x025
    SIPGWireData wire;                    // +0x028
    Stream* callback_stream;              // +0x2d8
};

class CIPGMgr;
class CIPGHelper;

// IDA_EA: 0x82A7182. The old interface has no virtual destructor: the first
// virtual slot is IPGInput and CIPGHelper owns the complete 16-slot table.
class IIPGHelperInterface {
public:
    IIPGHelperInterface();

    virtual int IPGInput(CUser* user, const char* source, const char* source2,
                         const char* source3, const char* source4,
                         const char* source5, void* callback) = 0;
    virtual int IPGInput(CUser* user, unsigned char charge_kind,
                         unsigned int price, const char* source,
                         const char* source2, const char* source3,
                         const char* source4, const char* source5,
                         void* callback) = 0;
    virtual int IPGInputCancel(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               const char* source6, void* callback) = 0;
    virtual int IPGInputPoint(CUser* user, unsigned int price,
                              char charge_kind, char point_kind,
                              void* callback) = 0;
    virtual int IPGOutputPoint(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               void* callback) = 0;
    virtual int IPGOutput(CUser* user, const char* source,
                          const char* source2, const char* source3,
                          const char* source4, const char* source5,
                          void* callback) = 0;
    virtual int IPGOutputSimulation(CUser* user, const char* source,
                                    const char* source2, const char* source3,
                                    const char* source4, const char* source5,
                                    void* callback) = 0;
    virtual int IPGOutputCancel(CUser* user, const char* source,
                                const char* source2, const char* source3,
                                const char* source4, const char* source5,
                                const char* source6, void* callback,
                                Taiwan::BillTransaction* transaction) = 0;
    virtual int IPGOutputConfirm(CUser* user, const char* source,
                                 const char* source2, const char* source3,
                                 const char* source4, const char* source5,
                                 const char* source6, void* callback,
                                 Taiwan::BillTransaction* transaction) = 0;
    virtual int IPGOutputCancel(SIPGData* request, SIPGData* result) = 0;
    virtual int IPGOutputConfirm(SIPGData* request, SIPGData* result) = 0;
    virtual int IPGQuery(CUser* user) = 0;
    virtual int IPGRequest(SIPGData* request) = 0;
    virtual void IPGResult(SIPGData* request, SIPGData* result) = 0;
    virtual stIPGStatus_t* getIPGStatus(stIPGStatus_t& status) = 0;
    virtual CIPGMgr* GetIPGMgr() = 0;
};

// IDA unit: WongWork::IPG::CIPGMgr.  Consumers such as the Taiwan auction
// path call getBillingCash() directly, so the original class declaration must
// be visible at that boundary instead of being hidden in ipg_runtime.cpp.
class CIPGMgr {
public:
    class CIPGWorker : public ThreadInterface {
    public:
        CIPGWorker();
        virtual ~CIPGWorker();

        CIPGWorker* SetIPGMgr(CIPGMgr* manager);
        SIPGData* ProcIPGSyncForChina(SIPGData* request);
        SIPGData* ProcIPGSyncForTaiwan(SIPGData* request);
        SIPGData* ProcIPGSync(SIPGData* request);
        virtual void dispatch(void* user_data);

    private:
        CIPGMgr* manager_;
    };

    CIPGMgr(IIPGHelperInterface* helper);
    ~CIPGMgr();

    int Initialize(unsigned int worker_count, unsigned int worker_argument);
    CIPGMgr* Finalize();
    SIPGData* AllocIPGData();
    int FreeIPGData(SIPGData* data);
    int PutIPGRequest(SIPGData* request, bool async);
    SIPGData* _GetIPGRequest();
    stIPGStatus_t& getIPGStatus(stIPGStatus_t& status);
    CIPGHelper* GetIPGHelper();
    Taiwan::TaiwanCash* getBillingCash();

private:
    CIPGWorker* workers_;                         // +0x000
    uint32_t worker_count_;                       // +0x004
    uint32_t worker_argument_;                    // +0x008
    std::queue<SIPGData*> request_queue_;         // +0x00c
    std::queue<SIPGData*> response_queue_;        // +0x034
    DynamicPool<SIPGData> data_pool_;             // +0x05c
    IIPGHelperInterface* helper_;                 // +0x0b8
    Mutex data_mutex_;                            // +0x0bc
    Mutex request_mutex_;                         // +0x0d4
    Taiwan::TaiwanCash* billing_cash_;            // +0x0ec
};

class CIPGHelper : public IIPGHelperInterface {
public:
    // IDA_EA: 0x82A7190 / 0x82A71B6
    CIPGHelper();
    ~CIPGHelper();

    // IDA_EA: 0x80FFB0A through 0x81010A2, in exact vtable order.
    virtual int IPGInput(CUser* user, const char* source, const char* source2,
                         const char* source3, const char* source4,
                         const char* source5, void* callback);
    virtual int IPGInput(CUser* user, unsigned char charge_kind,
                         unsigned int price, const char* source,
                         const char* source2, const char* source3,
                         const char* source4, const char* source5,
                         void* callback);
    virtual int IPGInputCancel(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               const char* source6, void* callback);
    virtual int IPGInputPoint(CUser* user, unsigned int price,
                              char charge_kind, char point_kind,
                              void* callback);
    virtual int IPGOutputPoint(CUser* user, const char* source,
                               const char* source2, const char* source3,
                               const char* source4, const char* source5,
                               void* callback);
    virtual int IPGOutput(CUser* user, const char* source,
                          const char* source2, const char* source3,
                          const char* source4, const char* source5,
                          void* callback);
    virtual int IPGOutputSimulation(CUser* user, const char* source,
                                    const char* source2, const char* source3,
                                    const char* source4, const char* source5,
                                    void* callback);
    virtual int IPGOutputCancel(CUser* user, const char* source,
                                const char* source2, const char* source3,
                                const char* source4, const char* source5,
                                const char* source6, void* callback,
                                Taiwan::BillTransaction* transaction);
    virtual int IPGOutputConfirm(CUser* user, const char* source,
                                 const char* source2, const char* source3,
                                 const char* source4, const char* source5,
                                 const char* source6, void* callback,
                                 Taiwan::BillTransaction* transaction);
    virtual int IPGOutputCancel(SIPGData* request, SIPGData* result);
    virtual int IPGOutputConfirm(SIPGData* request, SIPGData* result);
    virtual int IPGQuery(CUser* user);
    virtual int IPGRequest(SIPGData* request);
    virtual void IPGResult(SIPGData* request, SIPGData* result);
    virtual stIPGStatus_t* getIPGStatus(stIPGStatus_t& status);
    virtual CIPGMgr* GetIPGMgr();

    // IDA_EA: 0x8100A2E / 0x8100B3E / 0x8100C92
    SIPGData* _SetIPGData(CUser* user, IPG_DATA_TYPE type, bool queue_result);
    int Initialize(unsigned int worker_count, unsigned int worker_argument);
    CIPGHelper* Finalize();

private:
    CIPGMgr* ipg_manager_;                 // +0x04
    uint32_t wire_sizes_[7];               // +0x08
    char ipg_endpoint_[20];                // +0x24
    uint32_t ports_[7];                    // +0x38
};

}  // namespace IPG
}  // namespace WongWork

#if defined(__i386__)
static_assert(sizeof(WongWork::IPG::SIPGWireData) == 0x2b0u,
              "SIPGData wire union must remain 0x2b0 bytes");
static_assert(sizeof(WongWork::IPG::SIPGOutputWire) == 0x1fcu,
              "IPG_OUTPUT wire contract must remain 0x1fc bytes");
static_assert(sizeof(WongWork::IPG::SIPGInputWire::AccountHeader) == 0x46u,
              "IPG input header overlap must retain its 70-byte wire span");
static_assert(offsetof(WongWork::IPG::SIPGInputWire,
                       charge_result_decimal) == 0x246u,
              "IPG input charge result must remain at wire +0x246");
static_assert(offsetof(WongWork::IPG::SIPGOutputWire, response_reserved) + 21u ==
                  0x146u,
              "IPG output cancel code must remain at wire +0x146");
static_assert(offsetof(WongWork::IPG::SIPGOutputWire, transaction) == 0x1b4u,
              "IPG_OUTPUT transaction must remain at wire +0x1b4");
static_assert(offsetof(WongWork::IPG::SIPGOutputWire, character_name) == 0x1c0u,
              "IPG_OUTPUT character name must remain at wire +0x1c0");
static_assert(sizeof(WongWork::IPG::SIPGInputCancelWire) == 0x1cbu,
              "IPG_INPUT_CANCEL wire contract must remain 0x1cb bytes");
static_assert(sizeof(WongWork::IPG::SIPGOutputCancelWire) == 0x1d4u,
              "IPG_OUTPUT_CANCEL wire contract must remain 0x1d4 bytes");
static_assert(offsetof(WongWork::IPG::SIPGOutputCancelWire, transaction) == 0x1c8u,
              "IPG_OUTPUT_CANCEL transaction must remain at wire +0x1c8");
static_assert(sizeof(WongWork::IPG::SIPGQueryWire) == 0x099u,
              "IPG_QUERY wire contract must remain 0x99 bytes");
static_assert(offsetof(WongWork::IPG::SIPGQueryWire, balance_decimal) == 0x041u &&
                  offsetof(WongWork::IPG::SIPGQueryWire,
                           result_code_decimal) == 0x089u &&
                  offsetof(WongWork::IPG::SIPGQueryWire, point_decimal) ==
                      0x08du,
              "IPG_QUERY named decimal fields moved");
static_assert(sizeof(WongWork::IPG::SIPGCeraPointWire) == 0x090u,
              "IPG_INPUT_POINT wire contract must remain 0x90 bytes");
static_assert(offsetof(WongWork::IPG::SIPGData, wire) == 0x028u,
              "SIPGData wire union must begin at +0x28");
static_assert(offsetof(WongWork::IPG::SIPGData, callback_stream) == 0x2d8u,
              "SIPGData callback stream must remain at +0x2d8");
static_assert(sizeof(WongWork::IPG::SIPGData) == 0x2dcu,
              "SIPGData must retain its 0x2dc i386 ABI");
static_assert(sizeof(DynamicPool<WongWork::IPG::SIPGData>::CNode) == 0x2e4u,
              "SIPG dynamic-pool node must retain its 0x2e4-byte ABI");
static_assert(sizeof(WongWork::IPG::CIPGMgr::CIPGWorker) == 0x10u,
              "CIPGMgr worker must retain its 0x10-byte ABI");
static_assert(sizeof(WongWork::IPG::CIPGMgr) == 0x0f0u,
              "CIPGMgr must retain its 0xf0-byte ABI");
static_assert(sizeof(WongWork::IPG::CIPGHelper) == 0x54u,
              "CIPGHelper must retain its 0x54-byte i386 ABI");
#endif

#endif  // OLDSRV_GLOBAL_IPG_RUNTIME_H_
