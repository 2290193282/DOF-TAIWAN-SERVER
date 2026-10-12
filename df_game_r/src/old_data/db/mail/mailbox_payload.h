#ifndef DB_MAILBOX_PAYLOAD_H_
#define DB_MAILBOX_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mail/mail_trade_params.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_data/db/creature/creature_item_payload.h"


#pragma pack(push, 1)

// IDA_EA: Stream::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER> @0x845503E
// and CStreamGuard::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER> @0x84515AE.
struct SIG_MAILBOX_CHECK_RECEIVER {
    char receiver_name[29];                 // +0x0000
    int32_t receiver_charac_no;             // +0x001D
    int32_t receiver_level;                 // +0x0021
    int32_t receiver_delete_flag;           // +0x0025
    uint32_t receiver_account_id;           // +0x0029
    int32_t sender_charac_no;               // +0x002D
    MSG_CHECK_MAIL_RECEIVER message;        // +0x0031
    int32_t black_list_result;              // +0x3FB9
    int32_t multi_mail_text_length;         // +0x3FBD
    char sender_charac_name[29];            // +0x3FC1
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL> @0x8197F8E.
struct SIG_MAILBOX_SEND_NEW_MAIL {
    struct Request {
        char sender_charac_name[30];        // +0x000
        int32_t sender_charac_no;           // +0x01E
        int32_t receiver_charac_no;         // +0x022
        int32_t gold;                       // +0x026
        uint8_t seal_flag;                  // +0x02A
        uint32_t item_id;                   // +0x02B
        uint32_t add_info;                  // +0x02F
        uint8_t upgrade;                    // +0x033
        uint16_t endurance;                 // +0x034
        uint32_t extend_info;               // +0x036
        uint8_t item_guid[10];              // +0x03A
        uint8_t amplify_option;             // +0x044
        uint16_t amplify_value;             // +0x045
        uint8_t random_option[14];          // +0x047
        uint8_t separate_upgrade;           // +0x055
        uint8_t avatar_flag;                // +0x056
        uint8_t unlimit_flag;               // +0x057
        uint8_t creature_flag;              // +0x058
        uint16_t letter_text_length;        // +0x059
        char letter_text[256];              // +0x05B
        int32_t occ_time;                   // +0x15B
        int32_t server_group;               // +0x15F
        int32_t old_letter_id;              // +0x163
        uint8_t db_work;                    // +0x167
        // 140828 native extension.  The legacy prefix remains byte-exact;
        // both rebuilt game and DB processes consume this appended value.
        uint32_t chronicle_value;            // +0x168
    };

    struct Result {
        int32_t result;                     // +0x000
        int32_t receiver_charac_no;         // +0x004
        int32_t server_group;               // +0x008
    };

    union {
        Request request;
        Result result;
    };
};

// Rebuilt-server-only persistence correlation appended after the complete
// legacy signal-128 payload.  It is never sent to the client and does not
// change SIG_MAILBOX_SEND_NEW_MAIL's 0x16C ABI.  Exact frame length plus the
// magic/version pair prevents ordinary signal-128 replies from being treated
// as persistence acknowledgements.
enum MAILBOX_PERSISTENCE_CONSUMER {
    MAILBOX_PERSISTENCE_CONSUMER_NONE = 0,
    MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP = 1,
    MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE = 2,
    MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP = 3,
};

enum {
    MAILBOX_PERSISTENCE_CONTEXT_MAGIC = 0x4D41434B,  // "MACK"
    MAILBOX_PERSISTENCE_CONTEXT_VERSION = 1,
};

struct SIG_MAILBOX_PERSISTENCE_CONTEXT {
    uint32_t magic;                         // +0x00
    uint16_t version;                       // +0x04
    uint16_t consumer;                      // +0x06
    union {
        // Consumer-specific durable correlation.  The generic name is used
        // by the shared signal-128 framing checks; the aliases document the
        // key expected by each feature.  Existing u32 consumers occupy the
        // first four bytes; consumer 3 uses the complete 128-bit value.
        uint8_t correlation_bytes[16];       // +0x08
        uint8_t delivery_token[16];          // +0x08, Decide Level-Up
        uint32_t correlation_id;            // +0x08
        uint32_t account_id;                // +0x08, Magic Lamp
        uint32_t claim_token;               // +0x08, Facebook Like
    };
    uint32_t receiver_charac_no;            // +0x18
    uint32_t reward_item_id;                // +0x1C
    int32_t server_group;                   // +0x20
};

struct SIG_MAILBOX_SEND_NEW_MAIL_TRACKED {
    SIG_MAILBOX_SEND_NEW_MAIL mail;          // +0x000, byte-exact legacy part
    SIG_MAILBOX_PERSISTENCE_CONTEXT context; // +0x16C, rebuilt-only tail
};

// IDA_EA: the old signal-197 DBWORK stream specializations are
// CStreamGuard::GetInBuffer @0x8558B10, Stream::GetOutBuffer @0x845213E,
// and Stream::GetInBuffer @0x8559A7C.
struct SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK {
    int32_t package_type;                    // +0x000
    char sender_charac_name[30];             // +0x004
    uint8_t reserved_022[2];                 // +0x022
    int32_t receiver_charac_no;              // +0x024
    int32_t sender_charac_no;                // +0x028
    union Package {
        SIG_AVATAR_INSERT avatar;            // +0x02C
        SIG_CREATURE_ITEM creature;          // +0x02C
        uint8_t raw[0x74];
    } package;
    uint16_t letter_text_length;             // +0x0A0
    char letter_text[256];                   // +0x0A2
    uint8_t unlimit_flag;                    // +0x1A2
    uint8_t reserved_1a3;                    // +0x1A3
};

// IDA_EA: Stream::GetInBuffer<SIG_MAILBOX_DEL_MAIL> @0x8559AE6.
struct SIG_MAILBOX_DEL_MAIL {
    uint32_t count;                         // +0x00
    uint32_t postal_ids[20];                // +0x04
    uint32_t delete_flag;                   // +0x54
};

// IDA_EA: DB_ChangeLetterStat::makeRequest @0x84261FC.
struct SIG_CHANGE_LETTER_STAT {
    int32_t charac_no;                      // +0x00
    int32_t letter_id;                      // +0x04
    int16_t letter_stat;                    // +0x08
    uint8_t reserved_0a[2];                 // +0x0A
};

#pragma pack(pop)

inline bool MailboxPersistenceCorrelationValid(
    const SIG_MAILBOX_PERSISTENCE_CONTEXT& context) {
    const size_t correlation_size =
        context.consumer ==
                MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP
            ? sizeof(context.delivery_token)
            : sizeof(context.correlation_id);
    bool any_nonzero = false;
    for (size_t index = 0u; index < correlation_size; ++index) {
        any_nonzero = any_nonzero || context.correlation_bytes[index] != 0u;
    }
    if (!any_nonzero) return false;

    // The two pre-existing consumers use a u32 correlation and construct this
    // fixed message from a zero-filled object.  Reject non-canonical tails so
    // an uninitialized enlarged context cannot be echoed as a durable ACK.
    if (correlation_size == sizeof(context.correlation_id)) {
        for (size_t index = sizeof(context.correlation_id);
             index < sizeof(context.correlation_bytes); ++index) {
            if (context.correlation_bytes[index] != 0u) return false;
        }
    }
    return true;
}

static_assert(sizeof(SIG_MAILBOX_CHECK_RECEIVER) == 0x3FDEu,
              "SIG_MAILBOX_CHECK_RECEIVER must stay 0x3FDE");
static_assert(offsetof(SIG_MAILBOX_CHECK_RECEIVER, receiver_charac_no) == 0x1Du,
              "SIG_MAILBOX_CHECK_RECEIVER receiver charac offset must stay 0x1D");
static_assert(offsetof(SIG_MAILBOX_CHECK_RECEIVER, message) == 0x31u,
              "SIG_MAILBOX_CHECK_RECEIVER message offset must stay 0x31");
static_assert(
    offsetof(SIG_MAILBOX_CHECK_RECEIVER,
             message.payload.multi_mailbox_send.contents) == 0xDBu,
    "SIG_MAILBOX_CHECK_RECEIVER multi-mail text must stay at +0xDB");
static_assert(offsetof(SIG_MAILBOX_CHECK_RECEIVER, black_list_result) == 0x3FB9u,
              "SIG_MAILBOX_CHECK_RECEIVER black-list offset must stay 0x3FB9");
static_assert(offsetof(SIG_MAILBOX_CHECK_RECEIVER, multi_mail_text_length) == 0x3FBDu,
              "SIG_MAILBOX_CHECK_RECEIVER letter-length offset must stay 0x3FBD");
static_assert(offsetof(SIG_MAILBOX_CHECK_RECEIVER, sender_charac_name) == 0x3FC1u,
              "SIG_MAILBOX_CHECK_RECEIVER sender-name offset must stay 0x3FC1");

static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL::Request) == 0x16Cu,
              "0828 mail request must include the Chronicle value tail");
static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL::Result) == 0x0Cu,
              "legacy signal-128 result prefix must remain 12 bytes");
static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL) == 0x16Cu,
              "0828 mail signal must include the Chronicle value tail");
static_assert(sizeof(SIG_MAILBOX_PERSISTENCE_CONTEXT) == 0x24u,
              "mail persistence context must remain 36 bytes");
static_assert(offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, correlation_id) ==
                  0x08u &&
                  offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, account_id) ==
                      0x08u &&
                  offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, claim_token) ==
                      0x08u &&
                  offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, delivery_token) ==
                      0x08u,
              "tracked-mail consumer correlation aliases must stay at +0x08");
static_assert(
    offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, receiver_charac_no) == 0x18u &&
        offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, reward_item_id) == 0x1cu &&
        offsetof(SIG_MAILBOX_PERSISTENCE_CONTEXT, server_group) == 0x20u,
    "tracked-mail persistence identity offsets must remain fixed");
static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED) == 0x190u,
              "tracked mail signal must remain legacy 0x16C + 0x24");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED, context) == 0x16Cu,
              "tracked mail context must follow the complete legacy signal");
static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK) == 0x1A4u,
              "SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK must stay 0x1A4");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK, package) ==
                  0x2Cu,
              "mail DBWORK package offset must stay 0x2C");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK,
                       letter_text_length) == 0xA0u,
              "mail DBWORK text length offset must stay 0xA0");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK, unlimit_flag) ==
                  0x1A2u,
              "mail DBWORK unlimit offset must stay 0x1A2");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL::Request, receiver_charac_no) == 0x22u,
              "SIG_MAILBOX_SEND_NEW_MAIL receiver offset must stay 0x22");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL::Request, letter_text) == 0x5Bu,
              "SIG_MAILBOX_SEND_NEW_MAIL letter-text offset must stay 0x5B");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL::Request, server_group) == 0x15Fu,
              "SIG_MAILBOX_SEND_NEW_MAIL server-group offset must stay 0x15F");
static_assert(offsetof(SIG_MAILBOX_SEND_NEW_MAIL::Request,
                       chronicle_value) == 0x168u,
              "0828 mail Chronicle value must remain an appended field");

static_assert(sizeof(SIG_MAILBOX_DEL_MAIL) == 0x58u,
              "SIG_MAILBOX_DEL_MAIL must stay 0x58");
static_assert(sizeof(SIG_CHANGE_LETTER_STAT) == 0x0Cu,
              "SIG_CHANGE_LETTER_STAT must stay 0x0C");

#endif  // DB_MAILBOX_PAYLOAD_H_
