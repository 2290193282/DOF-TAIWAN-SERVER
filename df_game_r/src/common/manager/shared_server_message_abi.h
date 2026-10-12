#ifndef OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_ABI_H_
#define OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

struct CUser;

// IDA unit: 99_misc_global/stSERVER_MESSAGE.
// IDA_EA: dtor @0x8602146.
struct stSERVER_MESSAGE {
    ~stSERVER_MESSAGE();

    uint8_t kind;                 // +0x00
    uint8_t message_index;        // +0x01
    uint8_t reserved_02[2];       // +0x02
    uint32_t message_len;         // +0x04
    char message[52];             // +0x08
    uint32_t charac_name_len;     // +0x3c
    char charac_name[20];         // +0x40
    uint32_t update_time;         // +0x54
};

static_assert(sizeof(stSERVER_MESSAGE) == 0x58u,
              "stSERVER_MESSAGE IDA ABI size must stay 0x58");
static_assert(offsetof(stSERVER_MESSAGE, message_index) == 0x01u,
              "stSERVER_MESSAGE message_index offset must stay 0x01");
static_assert(offsetof(stSERVER_MESSAGE, message_len) == 0x04u,
              "stSERVER_MESSAGE message_len offset must stay 0x04");
static_assert(offsetof(stSERVER_MESSAGE, message) == 0x08u,
              "stSERVER_MESSAGE message offset must stay 0x08");
static_assert(offsetof(stSERVER_MESSAGE, charac_name_len) == 0x3cu,
              "stSERVER_MESSAGE charac_name_len offset must stay 0x3c");
static_assert(offsetof(stSERVER_MESSAGE, charac_name) == 0x40u,
              "stSERVER_MESSAGE charac_name offset must stay 0x40");
static_assert(offsetof(stSERVER_MESSAGE, update_time) == 0x54u,
              "stSERVER_MESSAGE update_time offset must stay 0x54");

// Trivial DB/wire record embedded by SIG_EVENT_SERVER_MESSAGE.  This is
// deliberately distinct from the vector-owned stSERVER_MESSAGE: the old
// SIG_LOAD_ETC destructor does not run 50 stSERVER_MESSAGE destructors.
struct EventServerMessagePayload {
    int8_t kind;                    // +0x00
    int8_t message_index;           // +0x01
    uint8_t reserved_02[2];         // +0x02
    uint32_t message_length;        // +0x04
    char message[52];               // +0x08
    uint32_t charac_name_length;    // +0x3c
    char charac_name[20];           // +0x40
    int32_t update_time;            // +0x54
};

static_assert(sizeof(EventServerMessagePayload) == 0x58u,
              "event-server-message DB record must stay 0x58 bytes");

// IDA DB/network payloads used by the shared-server-message path.
// IDA_EA: CStreamGuard::GetInBuffer<SIG_EVENT_SERVER_MESSAGE> @0x84532C4.
struct SIG_EVENT_SERVER_MESSAGE {
    uint8_t count;                              // +0x0000
    uint8_t reserved_01[3];                     // +0x0001
    EventServerMessagePayload messages[50];     // +0x0004
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_UPDATE_SERVER_MESSAGE> @0x84531B2.
struct SIG_UPDATE_SERVER_MESSAGE {
    uint32_t server_group;          // +0x00
    uint32_t channel_no;            // +0x04
    uint8_t kind;                   // +0x08
    uint8_t message_index;          // +0x09
    uint8_t reserved_0a[2];         // +0x0a
    uint32_t message_len;           // +0x0c
    char message[52];               // +0x10
    uint32_t charac_name_len;       // +0x44
    char charac_name[20];           // +0x48
    uint32_t update_time;           // +0x5c
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_SELECT_SERVER_MESSAGE> @0x8601CA6.
struct SIG_SELECT_SERVER_MESSAGE {
    uint16_t server_group;
    uint16_t channel_no;
};

static_assert(sizeof(SIG_EVENT_SERVER_MESSAGE) == 0x1134u,
              "SIG_EVENT_SERVER_MESSAGE size must stay 0x1134");
static_assert(sizeof(SIG_UPDATE_SERVER_MESSAGE) == 0x60u,
              "SIG_UPDATE_SERVER_MESSAGE size must stay 0x60");
static_assert(sizeof(SIG_SELECT_SERVER_MESSAGE) == 0x04u,
              "SIG_SELECT_SERVER_MESSAGE size must stay 0x04");

// IDA unit: 99_misc_global/stSharedServerMessageInfo.
// IDA_EA: ctor/dtor @0x836FA92/@0x836FAFE.
struct stSharedServerMessageInfo {
    stSharedServerMessageInfo();
    ~stSharedServerMessageInfo();

    std::vector<int> onoff;                         // +0x00
    std::vector<std::pair<int, int> > index_kind;   // +0x0c
    std::vector<std::pair<int, int> > event_kind;   // +0x18
};

// IDA unit: 11_event_runtime/EVENT_SERVER_MESSAGE public container boundary.
// IDA_EA: clear/ctor/dtor @0x86019D0/@0x8601A1A/@0x8601A30.
// C/11 owns the function bodies; A exposes the public layout/declarations.
struct EVENT_SERVER_MESSAGE {
    EVENT_SERVER_MESSAGE();
    ~EVENT_SERVER_MESSAGE();

    int clear();

    uint8_t count;                         // +0x00
    uint8_t reserved_01[3];                // +0x01
    std::vector<stSERVER_MESSAGE> messages; // +0x04
};

// IDA unit: 99_misc_global/CSharedServerMessageManager.
// IDA_EA: manager @0x84ED182..0x8601A0E.
class CSharedServerMessageManager {
public:
    class CSharedServerMessageAdapter {
    public:
        // IDA_EA: 0x86019EC.
        CSharedServerMessageAdapter();
        // IDA_EA: 0x86019FA.
        ~CSharedServerMessageAdapter();

        // IDA_EA: 0x8601640.
        int GetMappedEventID(int message_index);
        // IDA_EA: 0x8601700.
        int IsOnIndex(int message_index);
        // IDA_EA: 0x8601794.
        int GetShareKindWithIndex(int message_index);
        // IDA_EA: 0x8601A00.
        CSharedServerMessageAdapter* SetServerMessageAddInfo(
            stSharedServerMessageInfo* info);

        stSharedServerMessageInfo* info; // +0x00
    };

    // IDA_EA: 0x860079C.
    CSharedServerMessageManager();
    // IDA_EA: 0x860080E.
    explicit CSharedServerMessageManager(stSharedServerMessageInfo* info);
    // IDA_EA: 0x8600892.
    ~CSharedServerMessageManager();

    // IDA_EA: 0x84ED182.
    int IsSetServerMessage();
    // IDA_EA: 0x84ED18E.
    EVENT_SERVER_MESSAGE* GetCurServerMessage();
    // IDA_EA: 0x86008D8.
    void SendServerMessageInfo(CUser* user,
                               SIG_EVENT_SERVER_MESSAGE* message,
                               char flag);
    // IDA_EA: 0x8600B7A.
    int SetServerMessage(SIG_EVENT_SERVER_MESSAGE* message);
    // IDA_EA: 0x8600C34.
    stSERVER_MESSAGE* getCurMessage(int message_index);
    // IDA_EA: 0x8600CC8.
    int IsChangableMessage(int message_index, int cur_sec);
    // IDA_EA: 0x8600D0C.
    CSharedServerMessageManager* DeleteSpecificMessage(int message_index);
    // IDA_EA: 0x8600DCC.
    int CheckUserCondition(CUser* user, int message_index, int& result);
    // IDA_EA: 0x8600EC0.
    void UpdateServerMessage(CUser* user,
                             int message_index,
                             char* message,
                             char* charac_name,
                             int cur_sec);
    // IDA_EA: 0x8601476.
    int UpdateServerMessageByBroadcast(int message_index,
                                       char* message,
                                       char* charac_name,
                                       int cur_sec);
    // IDA_EA: 0x8601878.
    void LoadServerMessageFromDB();
    // IDA_EA: 0x8601A0E.
    CSharedServerMessageManager* setLoadingComplete();

    CSharedServerMessageAdapter adapter; // +0x00
    uint8_t loading_complete;            // +0x04
    uint8_t reserved_05[3];              // +0x05
    EVENT_SERVER_MESSAGE current;        // +0x08
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stSharedServerMessageInfo) == 0x24u,
              "stSharedServerMessageInfo IDA ABI size must stay 0x24");
static_assert(offsetof(stSharedServerMessageInfo, index_kind) == 0x0cu,
              "stSharedServerMessageInfo index_kind offset must stay 0x0c");
static_assert(offsetof(stSharedServerMessageInfo, event_kind) == 0x18u,
              "stSharedServerMessageInfo event_kind offset must stay 0x18");
static_assert(sizeof(EVENT_SERVER_MESSAGE) == 0x10u,
              "EVENT_SERVER_MESSAGE IDA ABI size must stay 0x10");
static_assert(offsetof(EVENT_SERVER_MESSAGE, messages) == 0x04u,
              "EVENT_SERVER_MESSAGE messages offset must stay 0x04");
static_assert(sizeof(CSharedServerMessageManager::CSharedServerMessageAdapter)
                  == 0x04u,
              "CSharedServerMessageAdapter size must stay 0x04");
static_assert(sizeof(CSharedServerMessageManager) == 0x18u,
              "CSharedServerMessageManager IDA ABI size must stay 0x18");
static_assert(offsetof(CSharedServerMessageManager, loading_complete) == 0x04u,
              "CSharedServerMessageManager loading flag offset must stay 0x04");
static_assert(offsetof(CSharedServerMessageManager, current) == 0x08u,
              "CSharedServerMessageManager current offset must stay 0x08");
#endif


using ::CSharedServerMessageManager;
using ::EVENT_SERVER_MESSAGE;
using ::SIG_EVENT_SERVER_MESSAGE;
using ::SIG_SELECT_SERVER_MESSAGE;
using ::SIG_UPDATE_SERVER_MESSAGE;
using ::stSERVER_MESSAGE;
using ::stSharedServerMessageInfo;


#endif  // OLDSRV_GLOBAL_SHARED_SERVER_MESSAGE_ABI_H_
