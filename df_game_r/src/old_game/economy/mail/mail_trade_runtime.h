#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAIL_TRADE_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAIL_TRADE_RUNTIME_H_

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_game/economy/mail/mailbox_public_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/collision_detection_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

class CUser;
class CTradeSpace;
class PacketGuard;
enum ENUM_ERROR : int;

struct MSG_CHECK_MAIL_RECEIVER;
struct MSG_MAILBOX_SEND;
struct SIG_MAILBOX_SEND_NEW_MAIL;
struct SIG_MAILBOX_PERSISTENCE_CONTEXT;
class Stream;

// The old server owns this concrete template specialization as an in-object
// CMailBox member.  Only StaticPool<CMailBox::CMail,20> is instantiated; the
// template declaration keeps its original Itanium C++ ABI identity instead
// of replacing it with a local pool lookalike.
template <typename Type, int Count>
class StaticPool {
public:
    class CNode : public Type {
    public:
        CNode();
        ~CNode();

        uint32_t dummy;
        uint8_t reference_count;
    };

    StaticPool();
    ~StaticPool();

    int Size();
    Type* Free(Type* node);
    int Remain();
    Type* Acquire();
    Type* Get(int index);
    int GetIndex(Type* node);
    int GetLastErrorCode();

    CNode* nodes;
    std::queue<CNode*> free_nodes;
    int error_state;
};

namespace WongWork {

struct CMailBoxLayoutVerifier;
struct CMailBoxPoolLayoutVerifier;
struct CMailBoxHelper;

// Canonical C-owned mailbox runtime layout.  All mailbox state-machine
// members are declared here; numbered 10/ contains no second CMailBox class.
// IDA unit: 10_mail_trade_auction_cash/WongWork::CMailBox.
class CMailBox {
public:
    struct CMail {
        // Old IDA address: 0x85575C0.
        CMail();
        // Old IDA address: 0x85575D6.
        ~CMail();

        // Old IDA address: 0x85519E2.
        int Init();
        // Old IDA address: 0x85519FE.
        CMail* ResetPackage();
        // Old IDA address: 0x85574D4.
        int GetLetterId();
        // Old IDA address: 0x85574E0.
        bool IsTimeOver(long cur_sec) const;
        // Old IDA address: 0x85574F2.
        long GetPeriod() const;
        // Old IDA address: 0x85574FE.
        const char* GetSenderName() const;
        // Old IDA address: 0x855750A.
        int GetSenderCharacNo() const;
        // Old IDA address: 0x8557516.
        int GetPackageMoney() const;
        // Old IDA address: 0x8557522.
        Inven_Item GetPackageItem() const;
        // Old IDA address: 0x8557590.
        int isUnlimit() const;
        // Old IDA address: 0x855759C.
        bool isAvatarPackage() const;
        // Old IDA address: 0x85575AE.
        bool isCreaturePackage() const;
        // Old IDA address: 0x869782E.
        int getMailIdx() const;
        // IDA_EA: 0x8551A36
        // IDA_NAME: _ZNK8WongWork8CMailBox5CMail10MakePacketEP11PacketGuardlPK5CUser
        // DEMANGLED: WongWork::CMailBox::CMail::MakePacket(PacketGuard *,long,CUser const*)const
        int MakePacket(PacketGuard* packet,
                       long current_sec,
                       const CUser* user) const;

        unsigned int mail_idx;
        int sender_charac_no;
        char sender_name[0x20];
        int package_money;
        Inven_Item package_item;
        uint8_t pad_69[3];
        long period;
        int package_type;
        uint8_t unlimit;
        uint8_t pad_75[3];
        unsigned int letter_id;
        int raw_7c;
    };

    struct stAddNewMailInput {
        // Old IDA address: 0x85578AE.
        stAddNewMailInput();

        unsigned int mail_idx;
        int sender_charac_no;
        int package_money;
        long period;
        unsigned int letter_id;
        int package_type;
        long package_period;
        uint8_t unlimit;
        uint8_t pad_1d[3];
        const char* sender_name;
        const Inven_Item* package_item;
    };

    // IDA_EA: 0x85579B2.
    CMailBox();
    // IDA_EA: 0x8557AAC.
    ~CMailBox();

    // IDA_EA: 0x8551D98
    // IDA_NAME: _ZN8WongWork8CMailBox4InitEv
    // DEMANGLED: WongWork::CMailBox::Init(void)
    int Init();
    // IDA_EA: 0x8551F5A.
    CMail* AddNewMail(const stAddNewMailInput& input);
    // IDA_EA: 0x8552138.
    int RemoveMail(unsigned int mail_idx);
    // Old IDA address: 0x823454C.
    int GetLoadedLetterCount();
    // Old IDA address: 0x823455A.
    int GetNotLoadedMailCount();
    // Old IDA address: 0x8234566.
    int GetRecvSize();
    // Old IDA address: 0x84ED216.
    int getMailLoadCount() const;
    // Old IDA address: 0x84ED222.
    int InsertLetterKeepCount(unsigned int letter_id);
    // Old IDA address: 0x84ED280.
    int DeleteLetterKeepCount(unsigned int letter_id);
    // Old IDA address: 0x84ED2FC.
    CMailBox* DecLoadedLetterCount();
    // Old IDA address: 0x84ED330.
    CMailBox* IncNotLoadedMailCount();
    // Old IDA address: 0x8551F4A.
    CMailBox* reset();
    // Old IDA address: 0x85522C4.
    CMail* GetMail(unsigned int mail_idx) const;
    // Old IDA address: 0x855232E.
    CMailBox* setMailIterator();
    // Old IDA address: 0x855236A.
    CMail* getNextMail();
    // Old IDA address: 0x85575DC.
    CMailBox* incMailLoadCount();
    // Old IDA address: 0x8557608.
    bool GetPackageLoadLack(unsigned int* out, unsigned int count);
    // Old IDA address: 0x8557676.
    bool SetPackageLoadLack(const unsigned int* values,
                            int count,
                            std::set<unsigned int>& changed);
    // Old IDA address: 0x85577AC.
    bool FindPackageLoadLack(unsigned int mail_idx);
    // Old IDA address: 0x855784C.
    int GetLastLoadLetterIdx();
    // Old IDA address: 0x8557858.
    CMailBox* SetLastLoadLetterIdx(unsigned int letter_idx);
    // Old IDA address: 0x8557866.
    CMailBox* ClearLetterKeepCount();
    // Old IDA address: 0x8557886.
    int GetLetterKeepCount();
    // Old IDA address: 0x855789C.
    CMailBox* SetLoadedLetterCount(int count);
    // Old IDA address: 0x8557914.
    CMailBox* SetNotLoadedMailCount(int count);
    // Old IDA address: 0x8557922.
    int GetRemainSize();
    // Old IDA address: 0x8557938.
    int IsLoaded();
    // Old IDA address: 0x8557944.
    int GetLastLoadIdx();
    // Old IDA address: 0x8557950.
    CMailBox* SetLoadState(bool loaded, long last_load_idx);

public:
    friend struct CMailBoxLayoutVerifier;
    friend struct CMailBoxPoolLayoutVerifier;
    friend struct CMailBoxHelper;

    uint8_t loaded;
    uint8_t pad_01[3];
    unsigned int last_load_idx;
    int not_loaded_mail_count;
    int mail_load_count;
    StaticPool<CMail, 20> mail_pool;
    std::map<unsigned int, CMail*> mails;
    std::list<Stream*> pending_streams;
    std::map<unsigned int, CMail*>::const_iterator mail_iterator;
    std::map<int, bool> letter_keep_count;
    unsigned int last_load_letter_idx;
    int loaded_letter_count;
    std::vector<unsigned int> package_load_lack;
    std::map<unsigned int, SIG_LETTER_INFO> letter_infos_map;
};


// Canonical C-owned mail state-machine helper.  The original class is a
// stateless collection of static operations; there is no implicit object
// argument in any of these ABIs.
struct CMailBoxHelper {
    // IDA data: _ZN8WongWork14CMailBoxHelper13g_MailBoxPoolE
    // at 0x943E000.  The original helper owns the one outer mailbox pool;
    // it is not a CUser-side surrogate allocation.
    static StaticPool<CMailBox, 600> g_MailBoxPool;

    // IDA uses this exact vector ABI for the staged package preflight and
    // final commit.  The first value is ENUM_ITEMSPACE and the second is the
    // resulting inventory slot.
    typedef std::vector<std::pair<int, int> > IntPairVector;

    // IDA_EA: 0x85523E2
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper19SetPeriodCheckTimerEP5CUser
    // DEMANGLED: WongWork::CMailBoxHelper::SetPeriodCheckTimer(CUser *)
    static int SetPeriodCheckTimer(CUser* user);

    // IDA_EA: 0x85525A6
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper14SetMailBoxInfoEP5CUserPK16SIG_MAILBOX_LIST
    // DEMANGLED: WongWork::CMailBoxHelper::SetMailBoxInfo(CUser *,SIG_MAILBOX_LIST const*)
    static int SetMailBoxInfo(CUser* user, const SIG_MAILBOX_LIST* list);

    // IDA_EA: 0x8555118
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper21MakeMailBoxListPacketEP5CUserP11PacketGuard
    // DEMANGLED: WongWork::CMailBoxHelper::MakeMailBoxListPacket(CUser *,PacketGuard *)
    static CMailBox* MakeMailBoxListPacket(CUser* user, PacketGuard* packet);

    // IDA_EA: 0x8553672
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper17_DoGetPackageCoinEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
    static bool _DoGetPackageCoin(CUser* user,
                                  const CMailBox::CMail* mail,
                                  IntPairVector& inserted_slots,
                                  bool is_received,
                                  ENUM_ERROR& error);

    // IDA_EA: 0x8553912
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper15IsCeraPointItemEm
    static bool IsCeraPointItem(unsigned long item_id);

    // IDA_EA: 0x8553936
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper17_DoGetPackageItemEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
    static bool _DoGetPackageItem(CUser* user,
                                  const CMailBox::CMail* mail,
                                  IntPairVector& inserted_slots,
                                  bool is_received,
                                  ENUM_ERROR& error);

    // IDA_EA: 0x855419C
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper19_DoGetPackageAvatarEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
    static bool _DoGetPackageAvatar(CUser* user,
                                    const CMailBox::CMail* mail,
                                    IntPairVector& inserted_slots,
                                    bool is_received,
                                    ENUM_ERROR& error);

    // IDA_EA: 0x855447C
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper21_DoGetPackageCreatureEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
    static bool _DoGetPackageCreature(CUser* user,
                                      const CMailBox::CMail* mail,
                                      IntPairVector& inserted_slots,
                                      bool is_received,
                                      ENUM_ERROR& error);

    // IDA_EA: 0x8554650
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper13_DoGetPackageEP5CUserPKNS_8CMailBox5CMailERSt6vectorISt4pairIiiESaIS9_EEbR10ENUM_ERROR
    static bool _DoGetPackage(CUser* user,
                              const CMailBox::CMail* mail,
                              IntPairVector& inserted_slots,
                              bool is_received,
                              ENUM_ERROR& error);

    // IDA_EA: 0x85549DE
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper10GetPackegeEP5CUserj
    // DEMANGLED: WongWork::CMailBoxHelper::GetPackege(CUser *,unsigned int)
    // The dispatcher invokes this canonical mailbox-package state machine;
    // its implementation remains owned by the mailbox helper restoration
    // unit rather than being duplicated in a packet handler.
    static void GetPackege(CUser* user, unsigned int mail_idx);

    // IDA_EA: 0x8697838
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper13AssignMailBoxEv
    // DEMANGLED: WongWork::CMailBoxHelper::AssignMailBox(void)
    static CMailBox* AssignMailBox();

    // IDA_EA: 0x869786A
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper11FreeMailBoxEPNS_8CMailBoxE
    // DEMANGLED: WongWork::CMailBoxHelper::FreeMailBox(WongWork::CMailBox *)
    static CMailBox* FreeMailBox(CMailBox* mailbox);

    // IDA_EA: 0x85552AC
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper13ReqDBLoadMailEP5CUserj
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBLoadMail(CUser *,unsigned int)
    static void ReqDBLoadMail(CUser* user, unsigned int charac_no);

    // IDA_EA: 0x8555464
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper18ReqDBCheckReceiverEP5CUserPKcP23MSG_CHECK_MAIL_RECEIVER
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBCheckReceiver(CUser *,char const*,MSG_CHECK_MAIL_RECEIVER *)
    static int ReqDBCheckReceiver(CUser* user,
                                  const char* receiver,
                                  MSG_CHECK_MAIL_RECEIVER* message);

    // IDA_EA: 0x8555486
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper27ReqDBCheckReceiver_forchinaEP5CUserPKcP23MSG_CHECK_MAIL_RECEIVER
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBCheckReceiver_forchina(CUser *,char const*,MSG_CHECK_MAIL_RECEIVER *)
    static int ReqDBCheckReceiver_forchina(
        CUser* user,
        const char* receiver,
        MSG_CHECK_MAIL_RECEIVER* message);

    // IDA_EA: 0x85555E8
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper22ReqDBSendNewSystemMailEPKcRK10Inven_ItemjjS2_ij17ENUM_SERVER_GROUPbb
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(char const*,Inven_Item const&,unsigned int,unsigned int,char const*,int,unsigned int,ENUM_SERVER_GROUP,bool,bool)
    static int ReqDBSendNewSystemMail(const char* title,
                                      const Inven_Item& item,
                                      unsigned int money,
                                      unsigned int charac_no,
                                      const char* text,
                                      int text_len,
                                      unsigned int occ_time,
                                      ENUM_SERVER_GROUP server_group,
                                      bool unlimit,
                                      bool db_work);

    // Rebuilt-server extension used when a caller must distinguish DB_Q
    // acceptance from durable postal insertion.  The original overload and
    // its Itanium ABI remain unchanged; the DB reply echoes persistence only
    // after signal 128's INSERT has actually completed.
    static int ReqDBSendNewSystemMailTracked(
        const char* title,
        const Inven_Item& item,
        unsigned int money,
        unsigned int charac_no,
        const char* text,
        int text_len,
        unsigned int occ_time,
        ENUM_SERVER_GROUP server_group,
        bool unlimit,
        bool db_work,
        const SIG_MAILBOX_PERSISTENCE_CONTEXT& persistence);

    // Native saga overload: the caller supplies the exact mail projection
    // captured in its durable plan.  This prevents make_item() from
    // regenerating GUID/expiration/random bytes during crash replay.
    static int ReqDBSendNewSystemMailTrackedRecord(
        const char* title,
        const InvenItemMailRecord& item,
        unsigned int money,
        unsigned int charac_no,
        const char* text,
        int text_len,
        unsigned int occ_time,
        ENUM_SERVER_GROUP server_group,
        bool unlimit,
        bool db_work,
        const SIG_MAILBOX_PERSISTENCE_CONTEXT& persistence);

    // IDA_EA: 0x85561B0
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper22ReqDBSendNewAvatarMailEP5CUseriiiccPKcS4_i
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(CUser *,int,int,int,char,char,char const*,char const*,int)
    static void ReqDBSendNewAvatarMail(CUser* user,
                                       int receiver_charac_no,
                                       int item_id,
                                       int expire_time,
                                       char ability_no,
                                       char avatar_select,
                                       const char* ipg_no,
                                       const char* text,
                                       int text_length);

    // IDA_EA: 0x8556F16
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper30ReqDBSendNewAvatarMailCashShopEP5CUseriiiccPKcS4_i
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(CUser *,int,int,int,char,char,char const*,char const*,int)
    // Unlike ordinary avatar mail, this marks DBWORK +0x1a2 and initializes
    // the avatar expansion subrecord at package +0x7b.
    static void ReqDBSendNewAvatarMailCashShop(CUser* user,
                                               int receiver_charac_no,
                                               int item_id,
                                               int expire_time,
                                               char ability_no,
                                               char avatar_select,
                                               const char* ipg_no,
                                               const char* text,
                                               int text_length);

    // IDA_EA: 0x8556422
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper24ReqDBSendNewCreatureMailEP5CUseriRK10Inven_ItemPKci
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewCreatureMail(CUser *,int,Inven_Item const&,char const*,int)
    static void ReqDBSendNewCreatureMail(CUser* user,
                                         int receiver_charac_no,
                                         const Inven_Item& item,
                                         const char* text,
                                         int text_length);

    // IDA_EA: 0x85571A0
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper32ReqDBSendNewCreatureMailCashShopEP5CUseriRK10Inven_ItemPKci
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewCreatureMailCashShop(CUser *,int,Inven_Item const&,char const*,int)
    // This is the DBWORK cash-shop creature path, not a wrapper around the
    // ordinary mail producer: the original writer sets unlimit_flag +0x1a2.
    static void ReqDBSendNewCreatureMailCashShop(CUser* user,
                                                 int receiver_charac_no,
                                                 const Inven_Item& item,
                                                 const char* text,
                                                 int text_length);

    // IDA_EA: 0x8555756
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper17_makeSendMailDataEP25SIG_MAILBOX_SEND_NEW_MAILPKciRK10Inven_ItemjjS4_iji
    // DEMANGLED: WongWork::CMailBoxHelper::_makeSendMailData(SIG_MAILBOX_SEND_NEW_MAIL *,char const*,int,Inven_Item const&,unsigned int,unsigned int,char const*,int,unsigned int,int)
    static SIG_MAILBOX_SEND_NEW_MAIL* _makeSendMailData(
        SIG_MAILBOX_SEND_NEW_MAIL* payload,
        const char* title,
        int sender_charac_no,
        const Inven_Item& item,
        unsigned int money,
        unsigned int charac_no,
        const char* text,
        int text_len,
        unsigned int occ_time,
        int unknown);

    // IDA_EA: 0x8555916
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper14_DoSendNewMailEP5CUserRK10Inven_ItemjjbPKcii
    // DEMANGLED: WongWork::CMailBoxHelper::_DoSendNewMail(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int,int)
    static int _DoSendNewMail(CUser* user,
                              const Inven_Item& item,
                              unsigned int money,
                              unsigned int charac_no,
                              bool send_now,
                              const char* text,
                              int text_len,
                              int unknown);

    // IDA_EA: 0x8555AC8
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper16ReqDBSendNewMailEP5CUserRK10Inven_ItemjjbPKci
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMail(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int)
    static int ReqDBSendNewMail(CUser* user,
                                const Inven_Item& item,
                                unsigned int money,
                                unsigned int charac_no,
                                bool send_now,
                                const char* text,
                                int text_len);

    // IDA_EA: 0x8555B14
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper16ReqDBSendNewMailEP5CUserP16MSG_MAILBOX_SENDjbji
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMail(CUser *,MSG_MAILBOX_SEND *,unsigned int,bool,unsigned int,int)
    // The multi-mail DB reply consumes one item at a time through this
    // transactional overload.  It charges the original postage, validates
    // and removes the source inventory record, then creates the mail row.
    static int ReqDBSendNewMail(CUser* user,
                                MSG_MAILBOX_SEND* message,
                                unsigned int receiver_charac_no,
                                bool send_now,
                                unsigned int receiver_account_id,
                                int multi_mail_text_length);

    // IDA_EA: 0x8556D5C
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper24ReqDBSendNewMailCashShopEP5CUserRK10Inven_ItemjjbPKci
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(CUser *,Inven_Item const&,unsigned int,unsigned int,bool,char const*,int)
    static int ReqDBSendNewMailCashShop(CUser* user,
                                        const Inven_Item& item,
                                        unsigned int money,
                                        unsigned int charac_no,
                                        bool send_now,
                                        const char* text,
                                        int text_len);

    // IDA_EA: 0x85566AE
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper19ReqDBSendStoredMailEP5CUser
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendStoredMail(CUser *)
    static int ReqDBSendStoredMail(CUser* user);

    // IDA_EA: 0x8556A14
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper25MakeSystemMultiMailPostalERSt6vectorISt4pairIiiESaIS3_EEP10Inven_Itemj
    // DEMANGLED: WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,Inven_Item *,unsigned int)
    static int MakeSystemMultiMailPostal(
        std::vector<std::pair<int, int> >& rewards,
        Inven_Item* items,
        unsigned int max_count);

    // IDA_EA: 0x8556B68
    static int ReqDBSendNewSystemMultiMail(const char* title,
                                           const Inven_Item* items,
                                           unsigned int item_count,
                                           unsigned int sender,
                                           unsigned int charac_no,
                                           const char* text,
                                           int text_len,
                                           unsigned int occ_time,
                                           ENUM_SERVER_GROUP server_group,
                                           bool unlimit);

    // IDA_EA: 0x8556772
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper15ReqDBRemoveMailEPK5CUserPjb
    // DEMANGLED: WongWork::CMailBoxHelper::ReqDBRemoveMail(CUser const*,unsigned int *,bool)
    static void ReqDBRemoveMail(const CUser* user,
                                unsigned int* postal_ids,
                                bool permanent_delete);

    // IDA_EA: 0x8554E90
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper16CheckMailTimeoutEP5CUserP11PacketGuard
    // DEMANGLED: WongWork::CMailBoxHelper::CheckMailTimeout(CUser *,PacketGuard *)
    static int CheckMailTimeout(CUser* user, PacketGuard* packet);

    // IDA_EA: 0x85568A6
    // IDA_NAME: _ZN8WongWork14CMailBoxHelper19ReqChangeLetterStatEP5CUserii
    // DEMANGLED: WongWork::CMailBoxHelper::ReqChangeLetterStat(CUser *,int,int)
    static int ReqChangeLetterStat(CUser* user,
                                   int letter_id,
                                   int stat);
};

}  // namespace WongWork

// Keep the one old mail specialization out of consumer objects.  Its real
// definitions and the single explicit instantiation live in
// mailbox_pool_registry_runtime.cpp.
extern template class StaticPool<WongWork::CMailBox::CMail, 20>;

namespace private_store {

class CPrivateStore;
class CPrivateStoreMgr;

// IDA_EA: 0x85CD22A.  This explicit specialization is a real private-store
// arithmetic ABI, not a local overflow helper.  Its repeated-addition
// contract (including the zero/negative operand behavior) is relied on by
// the private-store price transitions.
template <typename Type>
class COperator;

template <>
class COperator<int> {
public:
    static int Product(int left, int right, int& result);
};

// IDA_EA: 0x85CB93C.  CPrivateStoreMgr derives from this four-byte abstract
// IUserPos prefix.  The base C2 first installs its two pure slots, then the
// manager C2 installs the Exit/Enter vtable at the same address.
class IUserPos {
public:
    IUserPos();
    virtual int Exit(CUser* user) = 0;
    virtual int Enter(CUser* user) = 0;
};

// IDA_EA: 0x85CB6C8 / 0x85CB6DE
// IDA_NAME: _ZN13private_store16PrivateStoreZoneC2Ehh / _ZNK13private_store16PrivateStoreZoneltERKS0_
struct PrivateStoreZone {
    PrivateStoreZone(uint8_t village_value, uint8_t area_value);
    bool operator<(const PrivateStoreZone& rhs) const;

    uint8_t village;
    uint8_t area;
};

// IDA_EA: 0x85CB6F6 / 0x85CB72C / 0x85CB784
struct PrivateStorePos {
    PrivateStorePos();
    PrivateStorePos(uint8_t village_value,
                    uint8_t area_value,
                    int16_t x_value,
                    int16_t y_value);
    PrivateStorePos* clear();

    PrivateStoreZone zone;
    int16_t x;
    int16_t y;
};

// IDA unit: private_store/CPacketBufPrivate.  This is the concrete packet
// builder shared by every private-store virtual transition.  Its PacketGuard
// base is part of the old 0x14-byte i386 object, so callers must not replace
// it with an unrelated temporary packet builder.
class CPacketBufPrivate : public PacketGuard {
public:
    CPacketBufPrivate();
    ~CPacketBufPrivate();

    int MakeCmdpacketCreatePrivateStore();
    int MakeNotipacketCreatePrivateStore(uint16_t seller_index,
                                         PrivateStorePos& position,
                                         bool doll_created);
    int MakeCmdpacketRemovePrivateStore();
    int MakeNotipacketRemovePrivateStore(uint16_t seller_index);
    int MakeNotipacketForceRemovePrivateStore();
    int MakeCmdpacketCompleteDisplay(bool doll_created);
    int MakeNotipacketCompleteDisplay(uint16_t seller_index,
                                      std::string& display_name,
                                      bool doll_created);
    int MakeCmdpacketBuyPrivateStoreItem(CUser* user,
                                         Inven_Item& item,
                                         int arg4,
                                         int arg5,
                                         int slot,
                                         int item_add_info);
    int MakeNotipacketBuyPrivateStoreItem(CUser* user,
                                          int arg3,
                                          int slot,
                                          int arg5,
                                          const char* text,
                                          int money,
                                          int arg8);
    int MakeNotipacketPrivateStoreItem(int slot, int value);
    int MakeCmdpacketEnterPrivateStore(
        CUser* user,
        uint16_t seller_index,
        std::vector<PrivateStoreItem>& items);
    int MakeCmdpacketExitPrivateStore();
    int MakeNotipacketPrivateStoreInfo(SIG_PRIVATE_STORE* payload);
    int MakeCmdpacketMoveToGate();
    int MakeErrorReturnPacket(int packet_class, int packet_id, int error_code);
};

// IDA unit: private_store/CCharacterControllerPrivate.  The original global
// has exactly one PrivateStorePos value and returns a player to that gate
// position when a store is closed or moved.
class CCharacterControllerPrivate {
public:
    CCharacterControllerPrivate();
    PrivateStorePos* GetStartPoint(CUser* user, PrivateStorePos& position);
    int MoveCharacterStartPoint(CUser* user);

private:
    PrivateStorePos start_position;
};

extern CCharacterControllerPrivate g_CharacterController;

// IDA unit: private_store/CBroadcastPrivate.  It is a stateless namespace
// helper in the original ABI: the first two values are village and area.
struct CBroadcastPrivate {
    static void Send(int village_index, int area_index, PacketGuard& packet);
};

// IDA_EA: 0x85C4C7C / 0x85CB7A8
struct CErrorHandler {
    int Do(int error_code);
    CErrorHandler* Set(CUser* user_value, int packet_class_value, int command_value);

    CUser* user;
    int packet_class;
    int command;
};

// IDA_EA: 0x85CB7C8 / 0x85CB7FC / 0x85CB85C / 0x85CCAE4
class CPrivateStorePos : public village_object::IObject {
public:
    CPrivateStorePos();
    CPrivateStorePos(CPrivateStore* store, int center_x, int center_y);

    bool include(const village_object::IObject& rhs) const;
    bool put_object_info(PacketGuard& packet);
    CPrivateStorePos& operator=(const CPrivateStorePos& rhs);

    collision_detection::Rectangle rectangle;
    CPrivateStore* store;
};

// The old CPrivateStore object exposes two non-owning interface prefixes.
// Do not add a virtual destructor here: it would shift the observed slots.
// The abstract ISeller table is 0x8CBEBE8; CPrivateStore's actual primary
// vptr is 0x8CBEB68 (six seller slots followed by its buyer entries).
class ISeller {
public:
    virtual int CreatePrivateStore(uint8_t village,
                                   uint8_t channel,
                                   int16_t x,
                                   int16_t y,
                                   uint16_t doll_slot) = 0;
    virtual int RemovePrivateStore(bool force) = 0;
    virtual int CompleteDisplay(const char* display_name,
                                uint16_t doll_slot) = 0;
    virtual int MoveToGate() = 0;
    virtual int AddItem(PrivateStoreItem& item, int index) = 0;
    virtual int RecreatePrivateStore(SIG_PRIVATE_STORE* payload) = 0;
};

// The abstract IBuyer table is 0x8CBEBC0.  CPrivateStore's +0x04 secondary
// vptr is 0x8CBEB98 and contains the four this-adjusting buyer thunks.
class IBuyer {
public:
    virtual int BuyPrivateStoreItem(int slot, int count) = 0;
    virtual int VerifyPrivateStoreItem(int slot, int count) = 0;
    virtual int EnterPrivateStore() = 0;
    virtual int ExitPrivateStore() = 0;
};

// IDA ctor 0x85C4D10 initializes ISeller at +0 and IBuyer at +4 on i386.
// Every member below has an observed constructor, reset or direct accessor.
class CPrivateStore : public ISeller, public IBuyer {
public:
    CPrivateStore();
    ~CPrivateStore();
    CPrivateStore* Clear();
    // IDA_EA: 0x85C53EA
    // IDA_NAME: _ZN13private_store13CPrivateStore18CreatePrivateStoreEhhsst
    // DEMANGLED: private_store::CPrivateStore::CreatePrivateStore(unsigned char,unsigned char,short,short,unsigned short)
    virtual int CreatePrivateStore(uint8_t village,
                                   uint8_t channel,
                                   int16_t x,
                                   int16_t y,
                                   uint16_t doll_slot);
    // IDA_EA: 0x85C5FD0
    // IDA_NAME: _ZN13private_store13CPrivateStore18RemovePrivateStoreEb
    // DEMANGLED: private_store::CPrivateStore::RemovePrivateStore(bool)
    virtual int RemovePrivateStore(bool force);
    // IDA_EA: 0x85C7800
    // IDA_NAME: _ZN13private_store13CPrivateStore15CompleteDisplayEPKct
    // DEMANGLED: private_store::CPrivateStore::CompleteDisplay(char const*,unsigned short)
    virtual int CompleteDisplay(const char* display_name, uint16_t doll_slot);
    // IDA_EA: 0x85C751A
    // IDA_NAME: _ZN13private_store13CPrivateStore10MoveToGateEv
    // DEMANGLED: private_store::CPrivateStore::MoveToGate(void)
    virtual int MoveToGate();
    // IDA_EA: 0x85C6D48
    // IDA_NAME: _ZN13private_store13CPrivateStore7AddItemERNS_16PrivateStoreItemEi
    // DEMANGLED: private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem &,int)
    virtual int AddItem(PrivateStoreItem& item, int index);
    // IDA_EA: 0x85C5B60
    // IDA_NAME: _ZN13private_store13CPrivateStore20RecreatePrivateStoreEPNS_17SIG_PRIVATE_STOREE
    // DEMANGLED: private_store::CPrivateStore::RecreatePrivateStore(private_store::SIG_PRIVATE_STORE *)
    virtual int RecreatePrivateStore(SIG_PRIVATE_STORE* payload);
    // IDA_EA: 0x85C7E74
    // IDA_NAME: _ZN13private_store13CPrivateStore19BuyPrivateStoreItemEii
    // DEMANGLED: private_store::CPrivateStore::BuyPrivateStoreItem(int,int)
    virtual int BuyPrivateStoreItem(int slot, int count);
    // IDA_EA: 0x85C874C
    // IDA_NAME: _ZN13private_store13CPrivateStore22VerifyPrivateStoreItemEii
    // DEMANGLED: private_store::CPrivateStore::VerifyPrivateStoreItem(int,int)
    virtual int VerifyPrivateStoreItem(int slot, int count);
    // IDA_EA: 0x85C8DD6
    // IDA_NAME: _ZN13private_store13CPrivateStore17EnterPrivateStoreEv
    // DEMANGLED: private_store::CPrivateStore::EnterPrivateStore(void)
    virtual int EnterPrivateStore();
    // IDA_EA: 0x85C8FD4
    // IDA_NAME: _ZN13private_store13CPrivateStore16ExitPrivateStoreEv
    // DEMANGLED: private_store::CPrivateStore::ExitPrivateStore(void)
    virtual int ExitPrivateStore();

    int GetState();
    int isDollCreated();
    CUser* GetSeller();
    const std::string* GetSubject() const;
    uint16_t GetSellerIndex() const;
    CPrivateStore* SetSeller(CUser* user_value);
    CPrivateStore* SetBuyer(CUser* user_value);
    CPrivateStore* SetParent(CPrivateStoreMgr* manager);
    PrivateStorePos* GetPos();
    CPrivateStore* SetDollCreated(bool value);
    unsigned long GetDollItemID() const;
    CPrivateStore* SetDollItemID(unsigned long value);
    // IDA_EA: 0x85C4F52
    // IDA_NAME: _ZN13private_store13CPrivateStore10ResetItemsEv
    // DEMANGLED: private_store::CPrivateStore::ResetItems(void)
    bool ResetItems();
    // IDA_EA: 0x85C4F9C
    // IDA_NAME: _ZN13private_store13CPrivateStore9CopyItemsEPNS_16PrivateStoreItemEi
    int CopyItems(PrivateStoreItem* source, int count);
    // IDA_EA: 0x85C4FF2
    // IDA_NAME: _ZN13private_store13CPrivateStore14IsLimitedSpaceERKNS_15PrivateStorePosE
    // DEMANGLED: private_store::CPrivateStore::IsLimitedSpace(private_store::PrivateStorePos const&)
    bool IsLimitedSpace(const PrivateStorePos& other);
    // IDA_EA: 0x85C5082
    // IDA_NAME: _ZN13private_store13CPrivateStore25IsAreaNearEntranceDungeonEP5CUser
    // DEMANGLED: private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser *)
    bool IsAreaNearEntranceDungeon(CUser* user);
    // IDA_EA: 0x85C532C
    // IDA_NAME: _ZN13private_store13CPrivateStore21CheckPrivateStoreDollEt
    // DEMANGLED: private_store::CPrivateStore::CheckPrivateStoreDoll(unsigned short)
    int CheckPrivateStoreDoll(uint16_t doll_slot);
    // IDA_EA: 0x85C76F0
    // IDA_NAME: _ZN13private_store13CPrivateStore19CheckTotalItemPriceEv
    // DEMANGLED: private_store::CPrivateStore::CheckTotalItemPrice(void)
    int CheckTotalItemPrice();
    // IDA_EA: 0x85C7ADC
    // IDA_NAME: _ZN13private_store13CPrivateStore9SendUsersEP11PacketGuardP5CUserS4_
    // DEMANGLED: private_store::CPrivateStore::SendUsers(PacketGuard *,CUser *,CUser *)
    int SendUsers(PacketGuard* packet, CUser* excluded_user, CUser* extra_user);
    // IDA_EA: 0x85C7BBC
    // IDA_NAME: _ZN13private_store13CPrivateStore17SubtractItemCountEPK5CItemRNS_16PrivateStoreItemEi
    // DEMANGLED: private_store::CPrivateStore::SubtractItemCount(CItem const*,private_store::PrivateStoreItem &,int)
    PrivateStoreItem* SubtractItemCount(const CItem* item,
                                        PrivateStoreItem& store_item,
                                        int count);
    // IDA_EA: 0x85C7C0A
    // IDA_NAME: _ZN13private_store13CPrivateStore14IsPossibleDealEijj
    // DEMANGLED: private_store::CPrivateStore::IsPossibleDeal(int,unsigned int,unsigned int)
    int IsPossibleDeal(int money);
    // IDA_EA: 0x85C7D04
    // IDA_NAME: _ZN13private_store13CPrivateStore12GetItemPriceEPK5CItemRNS_16PrivateStoreItemEiRiRjS7_
    // DEMANGLED: private_store::CPrivateStore::GetItemPrice(CItem const*,private_store::PrivateStoreItem &,int,int &,unsigned int &,unsigned int &)
    int GetItemPrice(const CItem* item,
                     PrivateStoreItem& store_item,
                     int count,
                     int& money,
                     unsigned int& first_value,
                     unsigned int& second_value);
    // IDA_EA: 0x85C91B6
    // IDA_NAME: _ZN13private_store13CPrivateStore16IsExistBuyerListEP5CUser
    // DEMANGLED: private_store::CPrivateStore::IsExistBuyerList(CUser *)
    bool IsExistBuyerList(CUser* user);
    // IDA_EA: 0x85C9B8C
    // IDA_NAME: _ZN13private_store13CPrivateStore20GetCommissionPercentEv
    // DEMANGLED: private_store::CPrivateStore::GetCommissionPercent(void)
    int GetCommissionPercent();
    // IDA_EA: 0x85C924C
    // IDA_NAME: _ZN13private_store13CPrivateStore7BuyItemEiR10Inven_ItemiiiRi
    // DEMANGLED: private_store::CPrivateStore::BuyItem(int,Inven_Item &,int,int,int,int &)
    int BuyItem(int slot,
                Inven_Item& item,
                int money,
                int first_value,
                int second_value,
                int& item_count);
    // IDA_EA: 0x85C97E2
    // IDA_NAME: _ZN13private_store13CPrivateStore8SellItemEiR10Inven_Itemiiii
    // DEMANGLED: private_store::CPrivateStore::SellItem(int,Inven_Item &,int,int,int,int)
    int SellItem(int slot,
                 Inven_Item& item,
                 int money,
                 int commission,
                 int item_count,
                 int original_item_count);
    // IDA_EA: 0x85C68B4
    // IDA_NAME: _ZN13private_store13CPrivateStore10DeleteItemERNS_16PrivateStoreItemE
    // DEMANGLED: private_store::CPrivateStore::DeleteItem(private_store::PrivateStoreItem &)
    int DeleteItem(PrivateStoreItem& store_item);
    // IDA_EA: 0x85C7246
    // IDA_NAME: _ZN13private_store13CPrivateStore9MoveItemsEv
    // DEMANGLED: private_store::CPrivateStore::MoveItems(void)
    int MoveItems();
    // IDA_EA: 0x85C9BB8
    // IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemE10INVEN_TYPEii
    // DEMANGLED: private_store::CPrivateStore::CheckValidItem(INVEN_TYPE,int,int)
    int CheckValidItem(INVEN_TYPE inventory_type, int inventory_slot,
                       int item_count);
    // IDA_EA: 0x85C9CC2
    // IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemEii
    // DEMANGLED: private_store::CPrivateStore::CheckValidItem(int,int)
    int CheckValidItem(int cargo_slot, int item_count);
    // IDA_EA: 0x85C9D4E
    // IDA_NAME: _ZN13private_store13CPrivateStore14CheckValidItemERNS_16PrivateStoreItemEi
    // DEMANGLED: private_store::CPrivateStore::CheckValidItem(private_store::PrivateStoreItem &,int)
    int CheckValidItem(PrivateStoreItem& store_item, int original_count);
    // IDA_EA: 0x85C9F22
    // IDA_NAME: _ZN13private_store13CPrivateStore11GetItemInfoEP5CUserRNS_16PrivateStoreItemE
    // DEMANGLED: private_store::CPrivateStore::GetItemInfo(CUser *,private_store::PrivateStoreItem &)
    int GetItemInfo(CUser* user, PrivateStoreItem& store_item);
    // IDA_EA: 0x85CA116
    // IDA_NAME: _ZN13private_store13CPrivateStore16SavePrivateStoreEv
    // DEMANGLED: private_store::CPrivateStore::SavePrivateStore(void)
    void SavePrivateStore();
    // IDA_EA: 0x85C5CEC
    // IDA_NAME: _ZN13private_store13CPrivateStore28RecallPrivateStoreItemToMailEv
    // DEMANGLED: private_store::CPrivateStore::RecallPrivateStoreItemToMail(void)
    int RecallPrivateStoreItemToMail();
    // IDA_EA: 0x85C63CA
    // IDA_NAME: _ZN13private_store13CPrivateStore22RecallPrivateStoreItemEv
    // DEMANGLED: private_store::CPrivateStore::RecallPrivateStoreItem(void)
    int RecallPrivateStoreItem();

public:
    bool doll_created;                         // +0x08
    uint8_t padding_09_0b[3];
    uint32_t doll_item_id;                     // +0x0c
    uint8_t state;                             // +0x10
    uint8_t padding_11_13[3];
    CErrorHandler error_handler;               // +0x14
    CPrivateStoreMgr* manager;                 // +0x20
    PrivateStorePos position;                  // +0x24
    uint8_t alignment_2a_2b[2];
    CUser* seller;                             // +0x2c
    CUser* buyer;                              // +0x30
    std::list<CUser*> buyers;                  // +0x34
    std::vector<PrivateStoreItem> items;       // +0x3c
    std::string subject;                       // +0x48
    CPrivateStorePos world_position;           // +0x4c
};

#if defined(__i386__)
static_assert(sizeof(PrivateStoreZone) == 0x02u,
              "PrivateStoreZone must retain its two-byte IDA layout");
static_assert(sizeof(PrivateStorePos) == 0x06u,
              "PrivateStorePos must retain its six-byte IDA layout");
static_assert(sizeof(CErrorHandler) == 0x0cu,
              "CErrorHandler must retain its IDA layout");
static_assert(sizeof(CPrivateStorePos) == 0x18u,
              "CPrivateStorePos must retain its IDA layout");
static_assert(sizeof(CCharacterControllerPrivate) == 0x06u,
              "CCharacterControllerPrivate must retain its IDA layout");
static_assert(offsetof(CPrivateStore, position) == 0x24u,
              "CPrivateStore position must remain at +0x24");
static_assert(offsetof(CPrivateStore, manager) == 0x20u,
              "CPrivateStore manager must remain at +0x20");
static_assert(offsetof(CPrivateStore, seller) == 0x2cu,
              "CPrivateStore seller must remain at +0x2c");
static_assert(offsetof(CPrivateStore, buyers) == 0x34u,
              "CPrivateStore buyer list must remain at +0x34");
static_assert(offsetof(CPrivateStore, world_position) == 0x4cu,
              "CPrivateStore world position must remain at +0x4c");
static_assert(sizeof(CPrivateStore) == 0x64u,
              "CPrivateStore must retain the 0x64-byte IDA layout");
#endif

class CPrivateStoreMgr : public IUserPos {
public:
    // IDA_EA: 0x85CA3D6 / 0x85CA4FE.
    CPrivateStoreMgr();
    ~CPrivateStoreMgr();

    CPrivateStore* FindStoreBuyer(CUser* user);
    CPrivateStore* FindStoreSeller(CUser* user);
    int FreeStoreSeller(CUser* user);
    int FreeStoreBuyer(CUser* user);
    virtual int Exit(CUser* user);
    // IDA_EA: 0x85CAA8A
    // IDA_NAME: _ZN13private_store16CPrivateStoreMgr5EnterEP5CUser
    // DEMANGLED: private_store::CPrivateStoreMgr::Enter(CUser *)
    virtual int Enter(CUser* user);
    // IDA_EA: 0x85CA698
    // IDA_NAME: _ZN13private_store16CPrivateStoreMgr14GetStoreSellerEPPNS_7ISellerEP5CUser
    // DEMANGLED: private_store::CPrivateStoreMgr::GetStoreSeller(private_store::ISeller **,CUser *)
    int GetStoreSeller(ISeller** seller, CUser* user);
    // IDA_EA: 0x85CA7B8
    // IDA_NAME: _ZN13private_store16CPrivateStoreMgr13GetStoreBuyerEPPNS_6IBuyerEP5CUserS5_
    // DEMANGLED: private_store::CPrivateStoreMgr::GetStoreBuyer(private_store::IBuyer **,CUser *,CUser *)
    int GetStoreBuyer(IBuyer** buyer, CUser* user, CUser* seller);
    // IDA_EA: 0x85CA760
    // IDA_NAME: _ZN13private_store16CPrivateStoreMgr13GetStoreBuyerEPPNS_6IBuyerEP5CUsert
    // DEMANGLED: private_store::CPrivateStoreMgr::GetStoreBuyer(private_store::IBuyer **,CUser *,unsigned short)
    int GetStoreBuyer(IBuyer** buyer, CUser* user, unsigned short seller_index);
    // IDA_EA: 0x85CAB24
    // IDA_NAME: _ZN13private_store16CPrivateStoreMgr16LoadPrivateStoreEP5CUser
    // DEMANGLED: private_store::CPrivateStoreMgr::LoadPrivateStore(CUser *)
    void LoadPrivateStore(CUser* user);
    // IDA_EA: 0x85CAC16
    void LoadPrivateStoreResponse(CUser* user, SIG_PRIVATE_STORE* payload);
    // IDA_EA: 0x85CAAAA
    int HandleError(CUser* user, int packet_class, int packet_id, int error_code);
    int IsBusyPrivateStore(CUser* user);
    int GetPrivateStoreSize();

    CPrivateStore fallback_store;  // +0x04, after IUserPos base vptr
    StaticPool<CPrivateStore, 300> store_pool;  // +0x68
    std::map<CUser*, CPrivateStore*> sellers_098;
    std::map<CUser*, CPrivateStore*> buyers_0b0;
    std::map<PrivateStoreZone, std::list<CPrivateStore*>*> zones_0c8;
};

CPrivateStoreMgr* GetInstancePrivateStoreMgr();

typedef char CPrivateStoreMgr_Fallback_Offset_Must_Be_0x04[
    (offsetof(CPrivateStoreMgr, fallback_store) == 0x04) ? 1 : -1];
typedef char CPrivateStoreMgr_Pool_Offset_Must_Be_0x68[
    (offsetof(CPrivateStoreMgr, store_pool) == 0x68) ? 1 : -1];
typedef char CPrivateStoreMgr_SellerMap_Offset_Must_Be_0x98[
    (offsetof(CPrivateStoreMgr, sellers_098) == 0x98) ? 1 : -1];
typedef char CPrivateStoreMgr_BuyerMap_Offset_Must_Be_0xB0[
    (offsetof(CPrivateStoreMgr, buyers_0b0) == 0xB0) ? 1 : -1];
#if defined(__i386__)
static_assert(offsetof(CPrivateStoreMgr, zones_0c8) == 0xc8u,
              "CPrivateStoreMgr zone map must remain at +0xc8");
static_assert(sizeof(CPrivateStoreMgr) == 0xe0u,
              "CPrivateStoreMgr must retain the 0xe0-byte IDA layout");
#endif
}  // namespace private_store

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_MAIL_TRADE_RUNTIME_H_
