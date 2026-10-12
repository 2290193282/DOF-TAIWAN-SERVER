#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <cstring>

namespace {

const uint32_t kMailPoolDummy = 36641u;

typedef StaticPool<WongWork::CMailBox::CMail, 20> MailPool;

typedef char MailPool_Node_Size_Must_Be_0x88[
    (sizeof(MailPool::CNode) == 0x88) ? 1 : -1];
typedef char MailPool_Size_Must_Be_0x30[
    (sizeof(MailPool) == 0x30) ? 1 : -1];
typedef char MailPool_Nodes_Offset_Must_Be_0x00[
    (offsetof(MailPool, nodes) == 0x00) ? 1 : -1];
typedef char MailPool_FreeNodes_Offset_Must_Be_0x04[
    (offsetof(MailPool, free_nodes) == 0x04) ? 1 : -1];
typedef char MailPool_ErrorState_Offset_Must_Be_0x2c[
    (offsetof(MailPool, error_state) == 0x2c) ? 1 : -1];

}  // namespace

// IDA_EA: 0x8558D24
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EE5CNodeC2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::CNode::CNode(void)
template <typename Type, int Count>
StaticPool<Type, Count>::CNode::CNode()
    : Type(),
      dummy(kMailPoolDummy),
      reference_count(0) {}

// IDA_EA: 0x8557C26
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EE5CNodeD2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::CNode::~CNode()
template <typename Type, int Count>
StaticPool<Type, Count>::CNode::~CNode() {}

// IDA_EA: 0x8557C3A
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EEC2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::StaticPool(void)
template <typename Type, int Count>
StaticPool<Type, Count>::StaticPool()
    : free_nodes() {
    nodes = new CNode[Count];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }

    for (int index = 0; index < Count; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

// IDA_EA: 0x8557DEC
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EED2Ev
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::~StaticPool()
template <typename Type, int Count>
StaticPool<Type, Count>::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x8558232
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EE4FreeEPS2_
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::Free(WongWork::CMailBox::CMail*)
template <typename Type, int Count>
Type* StaticPool<Type, Count>::Free(Type* node) {
    if (!node) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* pool_node = static_cast<CNode*>(node);
    free_nodes.push(pool_node);
    if (pool_node->dummy != kMailPoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              pool_node->dummy,
              pool_node);
        error_state = 5;
    }
    if (pool_node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              pool_node->reference_count,
              pool_node);
        error_state = 6;
        PrintBackTrace();
    }
    --pool_node->reference_count;
    return node;
}

// IDA_EA: 0x8557BFC
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EE6RemainEv
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::Remain(void)
template <typename Type, int Count>
int StaticPool<Type, Count>::Remain() {
    return static_cast<int>(free_nodes.size());
}

// IDA_EA: 0x8558462
// IDA_NAME: _ZN10StaticPoolIN8WongWork8CMailBox5CMailELi20EE7AcquireEv
// DEMANGLED: StaticPool<WongWork::CMailBox::CMail,20>::Acquire(void)
template <typename Type, int Count>
Type* StaticPool<Type, Count>::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kMailPoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy,
              node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count,
              node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<Type*>(node);
}

// Emit only the original old-server specialization.  The extern-template
// declaration in the canonical header prevents consumers from creating
// another provider.
template class StaticPool<WongWork::CMailBox::CMail, 20>;

namespace WongWork {

struct CMailBoxPoolLayoutVerifier {
    static_assert(offsetof(CMailBox, loaded) == 0x00,
                  "CMailBox::loaded old ABI offset");
    static_assert(offsetof(CMailBox, last_load_idx) == 0x04,
                  "CMailBox::last_load_idx old ABI offset");
    static_assert(offsetof(CMailBox, not_loaded_mail_count) == 0x08,
                  "CMailBox::not_loaded_mail_count old ABI offset");
    static_assert(offsetof(CMailBox, mail_load_count) == 0x0c,
                  "CMailBox::mail_load_count old ABI offset");
    static_assert(offsetof(CMailBox, mail_pool) == 0x10,
                  "CMailBox::mail_pool old ABI offset");
    static_assert(offsetof(CMailBox, mails) == 0x40,
                  "CMailBox::mails old ABI offset");
    static_assert(offsetof(CMailBox, pending_streams) == 0x58,
                  "CMailBox::pending_streams old ABI offset");
    static_assert(offsetof(CMailBox, mail_iterator) == 0x60,
                  "CMailBox::mail_iterator old ABI offset");
    static_assert(offsetof(CMailBox, letter_keep_count) == 0x64,
                  "CMailBox::letter_keep_count old ABI offset");
    static_assert(offsetof(CMailBox, last_load_letter_idx) == 0x7c,
                  "CMailBox::last_load_letter_idx old ABI offset");
    static_assert(offsetof(CMailBox, loaded_letter_count) == 0x80,
                  "CMailBox::loaded_letter_count old ABI offset");
    static_assert(offsetof(CMailBox, package_load_lack) == 0x84,
                  "CMailBox::package_load_lack old ABI offset");
    static_assert(offsetof(CMailBox, letter_infos_map) == 0x90,
                  "CMailBox::letter_infos_map old ABI offset");
    static_assert(sizeof(CMailBox) == 0xa8,
                  "CMailBox old i386 ABI size");
};

// IDA_EA: 0x85575C0
// IDA_NAME: _ZN8WongWork8CMailBox5CMailC2Ev
// DEMANGLED: WongWork::CMailBox::CMail::CMail(void)
CMailBox::CMail::CMail()
    : package_item() {}

// IDA_EA: 0x85575D6
// IDA_NAME: _ZN8WongWork8CMailBox5CMailD2Ev
// DEMANGLED: WongWork::CMailBox::CMail::~CMail()
CMailBox::CMail::~CMail() {}

// IDA_EA: 0x85519E2
// IDA_NAME: _ZN8WongWork8CMailBox5CMail4InitEv
// DEMANGLED: WongWork::CMailBox::CMail::Init(void)
int CMailBox::CMail::Init() {
    mail_idx = 0;
    return static_cast<int>(
        reinterpret_cast<uintptr_t>(ResetPackage()));
}

// IDA_EA: 0x85578AE
// IDA_NAME: _ZN8WongWork8CMailBox17stAddNewMailInputC2Ev
// DEMANGLED: WongWork::CMailBox::stAddNewMailInput::stAddNewMailInput(void)
CMailBox::stAddNewMailInput::stAddNewMailInput()
    : mail_idx(0),
      sender_charac_no(0),
      package_money(0),
      period(0),
      letter_id(0),
      package_type(0),
      package_period(0),
      unlimit(0),
      sender_name(0),
      package_item(0) {}

// IDA_EA: 0x85579B2
// IDA_NAME: _ZN8WongWork8CMailBoxC2Ev
// DEMANGLED: WongWork::CMailBox::CMailBox(void)
CMailBox::CMailBox()
    : mail_pool(),
      mails(),
      pending_streams(),
      mail_iterator(),
      letter_keep_count(),
      package_load_lack(),
      letter_infos_map() {}

// IDA_EA: 0x8557AAC
// IDA_NAME: _ZN8WongWork8CMailBoxD2Ev
// DEMANGLED: WongWork::CMailBox::~CMailBox()
CMailBox::~CMailBox() {}

// IDA_EA: 0x8234566
// IDA_NAME: _ZN8WongWork8CMailBox11GetRecvSizeEv
// DEMANGLED: WongWork::CMailBox::GetRecvSize(void)
int CMailBox::GetRecvSize() {
    return static_cast<int>(mails.size());
}

// IDA_EA: 0x8551F5A
// IDA_NAME: _ZN8WongWork8CMailBox10AddNewMailERKNS0_17stAddNewMailInputE
// DEMANGLED: WongWork::CMailBox::AddNewMail(WongWork::CMailBox::stAddNewMailInput const&)
CMailBox::CMail* CMailBox::AddNewMail(const stAddNewMailInput& input) {
    CMail* mail = mail_pool.Acquire();
    if (!mail) {
        return 0;
    }

    mail->mail_idx = input.mail_idx;
    mail->sender_charac_no = input.sender_charac_no;
    mail->package_money = input.package_money;
    if (input.package_item) {
        // 0x8551F5A copies all 0x3d packet bytes.  Inven_Item::setCopy()
        // intentionally skips the physical ten-byte GUID, which would turn
        // a DB-loaded mail package into a different item identity.
        std::memcpy(&mail->package_item,
                    input.package_item,
                    sizeof(mail->package_item));
    } else {
        Inven_Item empty_item;
        std::memcpy(&mail->package_item,
                    &empty_item,
                    sizeof(mail->package_item));
    }
    mail->period = input.package_period;
    std::strncpy(mail->sender_name, input.sender_name, 0x1du);
    mail->package_type = input.package_type;
    mail->unlimit = input.unlimit;
    mail->letter_id = input.period;
    mails.insert(std::make_pair(mail->mail_idx, mail));
    mail->raw_7c = input.letter_id;
    return mail;
}

// IDA_EA: 0x8552138
// IDA_NAME: _ZN8WongWork8CMailBox10RemoveMailEj
// DEMANGLED: WongWork::CMailBox::RemoveMail(unsigned int)
int CMailBox::RemoveMail(unsigned int mail_idx_value) {
    std::map<unsigned int, CMail*>::iterator found =
        mails.find(mail_idx_value);
    if (found == mails.end()) {
        return 0;
    }

    CMail* mail = found->second;
    mails.erase(found);
    if (mail->GetLetterId()) {
        bool same_letter_exists = false;
        for (std::map<unsigned int, CMail*>::iterator it = mails.begin();
             it != mails.end();
             ++it) {
            if (it->second->GetLetterId() == mail->GetLetterId()) {
                same_letter_exists = true;
            }
        }
        if (!same_letter_exists) {
            ++loaded_letter_count;
        }
    }

    mail->ResetPackage();
    mail_pool.Free(mail);
    return 1;
}

// IDA_EA: 0x85522C4
// IDA_NAME: _ZNK8WongWork8CMailBox7GetMailEj
// DEMANGLED: WongWork::CMailBox::GetMail(unsigned int)const
CMailBox::CMail* CMailBox::GetMail(unsigned int mail_idx_value) const {
    std::map<unsigned int, CMail*>::const_iterator found =
        mails.find(mail_idx_value);
    if (found == mails.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x855232E
// IDA_NAME: _ZN8WongWork8CMailBox15setMailIteratorEv
// DEMANGLED: WongWork::CMailBox::setMailIterator(void)
CMailBox* CMailBox::setMailIterator() {
    mail_iterator = mails.begin();
    return this;
}

// IDA_EA: 0x855236A
// IDA_NAME: _ZN8WongWork8CMailBox11getNextMailEv
// DEMANGLED: WongWork::CMailBox::getNextMail(void)
CMailBox::CMail* CMailBox::getNextMail() {
    if (mail_iterator == mails.end()) {
        return 0;
    }
    CMail* mail = mail_iterator->second;
    ++mail_iterator;
    return mail;
}

// IDA_EA: 0x8557922
// IDA_NAME: _ZN8WongWork8CMailBox13GetRemainSizeEv
// DEMANGLED: WongWork::CMailBox::GetRemainSize(void)
int CMailBox::GetRemainSize() {
    return mail_pool.Remain();
}

}  // namespace WongWork
