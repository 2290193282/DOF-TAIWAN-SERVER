#include "common/manager/slang_filter.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

static_assert(sizeof(FlexLexer) == 0x14u,
              "FlexLexer IDA layout size must stay 0x14");
static_assert(sizeof(__slang_filter__FlexLexer) == 0x80u,
              "__slang_filter__FlexLexer IDA layout size must stay 0x80");
static_assert(sizeof(SlangFilter) == 0x80u,
              "SlangFilter IDA layout size must stay 0x80");

// IDA_EA: 0x8094E56
// IDA_NAME: _ZN9FlexLexerC2Ev
// DEMANGLED: FlexLexer::FlexLexer(void)
FlexLexer::FlexLexer()
    : yytext_(0), yyleng_(0), yylineno_(1), yy_flex_debug_(0) {}

// IDA_EA: 0x8094E12
// IDA_NAME: _ZN9FlexLexerD2Ev
// DEMANGLED: FlexLexer::~FlexLexer()
FlexLexer::~FlexLexer() {}

// IDA_EA: 0x80950C6
// IDA_NAME: _ZNK9FlexLexer6YYTextEv
// DEMANGLED: FlexLexer::YYText(void)const
char* FlexLexer::YYText() const { return yytext_; }

// IDA_EA: 0x8093834
// IDA_NAME: _ZN25__slang_filter__FlexLexerC2EPSiPSo
// DEMANGLED: __slang_filter__FlexLexer::__slang_filter__FlexLexer(std::istream *,std::ostream *)
__slang_filter__FlexLexer::__slang_filter__FlexLexer(
    std::istream* input, std::ostream* output)
    : FlexLexer(),
      yy_start_stack_ptr_(0), yy_start_stack_depth_(0), yy_start_stack_(0),
      yyin_(input), yyout_(output), yy_hold_char_(0), reserved_29_{0, 0, 0},
      yy_n_chars_(0), yy_c_buf_p_(0), yy_init_(0), yy_start_(0),
      yy_did_buffer_switch_(0), buffer_top_(0), buffer_max_(0),
      buffer_stack_(0), last_accepting_state_(0), last_accepting_cpos_(0),
      state_buf_(0), state_ptr_(0), full_match_(0), full_state_(0),
      full_lp_(0), lp_(0), looking_for_trail_(0), more_flag_(0),
      more_len_(0), more_offset_(0), prev_more_offset_(0) {}

// IDA_EA: 0x809391A
// IDA_NAME: _ZN25__slang_filter__FlexLexerD2Ev
// DEMANGLED: __slang_filter__FlexLexer::~__slang_filter__FlexLexer()
__slang_filter__FlexLexer::~__slang_filter__FlexLexer() {
    delete[] state_buf_;
    state_buf_ = 0;
    std::free(yy_start_stack_);
    yy_start_stack_ = 0;
    std::free(buffer_stack_);
    buffer_stack_ = 0;
}

// IDA_EA: 0x8094E78
// IDA_NAME: _ZN11SlangFilterC2Ev
// DEMANGLED: SlangFilter::SlangFilter(void)
SlangFilter::SlangFilter() : __slang_filter__FlexLexer(0, 0) {}

// IDA_EA: 0x8094EA4
// IDA_NAME: _ZN11SlangFilterD2Ev
// DEMANGLED: SlangFilter::~SlangFilter()
SlangFilter::~SlangFilter() {}

// IDA_EA: 0x8095032
// IDA_NAME: _ZN11SlangFilter11LexerOutputEPKci
// DEMANGLED: SlangFilter::LexerOutput(char const*,int)
int SlangFilter::LexerOutput(const char* text, int length) {
    (void)length;
    return std::fprintf(stdout, text);
}

// IDA_EA: 0x809504E
// IDA_NAME: _ZN11SlangFilter10LexerErrorEPKc
// DEMANGLED: SlangFilter::LexerError(char const*)
int SlangFilter::LexerError(const char* text) {
    return std::fprintf(stderr, text);
}

CSyncSlangFilter* CSyncSlangFilter::instance_ = 0;

// IDA_EA: 0x862E910
// IDA_NAME: _ZN9SlangNodeC2Ev
// DEMANGLED: SlangNode::SlangNode(void)
SlangNode::SlangNode() : children_(), end_node_(false) {}

// IDA_EA: 0x862E92A
// IDA_NAME: _ZN9SlangNodeD2Ev
// DEMANGLED: SlangNode::~SlangNode()
SlangNode::~SlangNode() {
    for (ChildMap::iterator it = children_.begin(); it != children_.end();
         ++it) {
        delete it->second;
    }
    children_.clear();
}

// IDA_EA: 0x862E9F6
// IDA_NAME: _ZNK9SlangNode9FindChildEj
// DEMANGLED: SlangNode::FindChild(unsigned int)const
SlangNode* SlangNode::FindChild(unsigned int value) const {
    ChildMap::const_iterator it = children_.find(value);
    if (it != children_.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x862EA5A
// IDA_NAME: _ZN9SlangNode8AddChildEj
// DEMANGLED: SlangNode::AddChild(unsigned int)
SlangNode* SlangNode::AddChild(unsigned int value) {
    ChildMap::iterator it = children_.find(value);
    if (it == children_.end()) {
        SlangNode* child = new SlangNode();
        it = children_.insert(std::make_pair(value, child)).first;
    }
    return it->second;
}

// IDA_EA: 0x862EB3C
// IDA_NAME: _ZNK9SlangNode9IsEndNodeEv
// DEMANGLED: SlangNode::IsEndNode(void)const
int SlangNode::IsEndNode() const {
    return end_node_;
}

// IDA_EA: 0x862EB48
// IDA_NAME: _ZN9SlangNode10SetEndNodeEb
// DEMANGLED: SlangNode::SetEndNode(bool)
SlangNode* SlangNode::SetEndNode(bool end_node) {
    end_node_ = end_node;
    return this;
}

// IDA_EA: 0x862EB60
// IDA_NAME: _ZN16CSyncSlangFilter4IMPLC2Ev
// DEMANGLED: CSyncSlangFilter::IMPL::IMPL(void)
CSyncSlangFilter::IMPL::IMPL() : root(new SlangNode()) {}

// IDA_EA: 0x862EBAA
// IDA_NAME: _ZN16CSyncSlangFilter4IMPLD2Ev
// DEMANGLED: CSyncSlangFilter::IMPL::~IMPL()
CSyncSlangFilter::IMPL::~IMPL() {
    delete root;
    root = 0;
}

// IDA_EA: 0x8235001
// IDA_NAME: _ZN16CSyncSlangFilter11GetInstanceEv
// DEMANGLED: CSyncSlangFilter::GetInstance(void)
CSyncSlangFilter* CSyncSlangFilter::GetInstance() {
    if (!instance_) {
        instance_ = new CSyncSlangFilter();
    }
    return instance_;
}

// IDA_EA: 0x82A7147
// IDA_NAME: _ZN16CSyncSlangFilter12FreeInstanceEv
// DEMANGLED: CSyncSlangFilter::FreeInstance(void)
void CSyncSlangFilter::FreeInstance() {
    if (instance_) {
        delete instance_;
        instance_ = 0;
    }
}

// IDA_EA: 0x862DBA4
// IDA_NAME: _ZN16CSyncSlangFilterC2Ev
// DEMANGLED: CSyncSlangFilter::CSyncSlangFilter(void)
CSyncSlangFilter::CSyncSlangFilter()
    : slang_(new IMPL()), name_slang_(new IMPL()) {
    pthread_rwlock_init(&rwlock_, 0);
}

// IDA_EA: 0x862DC36
// IDA_NAME: _ZN16CSyncSlangFilterD2Ev
// DEMANGLED: CSyncSlangFilter::~CSyncSlangFilter()
CSyncSlangFilter::~CSyncSlangFilter() {
    delete slang_;
    slang_ = 0;
    delete name_slang_;
    name_slang_ = 0;
    pthread_rwlock_destroy(&rwlock_);
}

// IDA_EA: 0x862DCCC
// IDA_NAME: _ZN16CSyncSlangFilter12AddSlangListEv
// DEMANGLED: CSyncSlangFilter::AddSlangList(void)
int CSyncSlangFilter::AddSlangList() {
    MySQL* handle = GlobalData::s_db_mgr
                                ? GlobalData::s_db_mgr->GetDBHandle(
                                      DB_HANDLE_1, SERVER_GROUP_DEFAULT)
                                : 0;
    if (!handle) {
        return 0;
    }

    handle->set_query("seLect slang from slang_list");
    if (handle->exec(true) != true) {
        return 0;
    }

    for (int row = 0; row < handle->get_n_rows(); ++row) {
        if (handle->fetch() != true) {
            return 0;
        }

        char slang[51];
        ::memset(slang, 0, sizeof(slang));
        if (handle->get_str(0, slang, sizeof(slang)) != true) {
            cMyTrace trace("bool CSyncSlangFilter::AddSlangList()",
                                   188, 5);
            trace("CSyncSlangFilter::AddSlangList");
            return 0;
        }
        AddSlang(std::string(slang));
    }

    return 1;
}

// IDA_EA: 0x862DEC6
// IDA_NAME: _ZN16CSyncSlangFilter16AddSlangListNameEv
// DEMANGLED: CSyncSlangFilter::AddSlangListName(void)
int CSyncSlangFilter::AddSlangListName() {
    MySQL* handle = GlobalData::s_db_mgr
                                ? GlobalData::s_db_mgr->GetDBHandle(
                                      DB_HANDLE_1, SERVER_GROUP_DEFAULT)
                                : 0;
    if (!handle) {
        return 0;
    }

    handle->set_query("seLect slang from slang_list_name");
    if (handle->exec(true) != true) {
        return 0;
    }

    for (int row = 0; row < handle->get_n_rows(); ++row) {
        if (handle->fetch() != true) {
            return 0;
        }

        char slang[51];
        ::memset(slang, 0, sizeof(slang));
        if (handle->get_str(0, slang, sizeof(slang)) != true) {
            cMyTrace trace(
                "bool CSyncSlangFilter::AddSlangListName()", 217, 5);
            trace("CSyncSlangFilter::AddSlangList");
            return 0;
        }
        AddSlangName(std::string(slang));
    }

    return 1;
}

// IDA_EA: 0x862E0C0
// IDA_NAME: _ZNK16CSyncSlangFilter9MatchNameERKSs
// DEMANGLED: CSyncSlangFilter::MatchName(std::string const&)const
unsigned int CSyncSlangFilter::MatchName(const std::string& text) const {
    if (text.empty() || !name_slang_ || !name_slang_->root) {
        return 0;
    }

    SlangNode* child = name_slang_->root;
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        child = child->FindChild(
            static_cast<unsigned char>(text[index]));
        if (!child) {
            return 0;
        }
        if (child->IsEndNode()) {
            return static_cast<unsigned int>(index + 1);
        }
    }
    return 0;
}

// IDA_EA: 0x862E16E
// IDA_NAME: _ZN16CSyncSlangFilter12HasSlangNameERKSs
// DEMANGLED: CSyncSlangFilter::HasSlangName(std::string const&)
int CSyncSlangFilter::HasSlangName(const std::string& text) {
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        pthread_rwlock_rdlock(&rwlock_);
        const bool matched = MatchName(text.substr(index)) != 0;
        pthread_rwlock_unlock(&rwlock_);
        if (matched) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x862E276
// IDA_NAME: _ZN16CSyncSlangFilter12AddSlangNameERKSs
// DEMANGLED: CSyncSlangFilter::AddSlangName(std::string const&)
int CSyncSlangFilter::AddSlangName(const std::string& text) {
    char buffer[257];
    ::memset(buffer, 0, sizeof(buffer));
    ::snprintf(buffer, 0x100u, "%s", text.c_str());
    buffer[255] = 0;

    SlangNode* node = name_slang_->root;
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        pthread_rwlock_wrlock(&rwlock_);
        node = node->AddChild(
            static_cast<unsigned char>(buffer[index]));
        pthread_rwlock_unlock(&rwlock_);
    }
    node->SetEndNode(true);
    return 1;
}

// IDA_EA: 0x862E3BE
// IDA_NAME: _ZNK16CSyncSlangFilter5matchERKSs
// DEMANGLED: CSyncSlangFilter::match(std::string const&)const
unsigned int CSyncSlangFilter::match(const std::string& text) const {
    if (text.empty() || !slang_ || !slang_->root) {
        return 0;
    }

    SlangNode* child = slang_->root;
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        child = child->FindChild(
            static_cast<unsigned char>(text[index]));
        if (!child) {
            return 0;
        }
        if (child->IsEndNode()) {
            return static_cast<unsigned int>(index + 1);
        }
    }
    return 0;
}

// IDA_EA: 0x862E46C
// IDA_NAME: _ZN16CSyncSlangFilter6FilterERKSs
// DEMANGLED: CSyncSlangFilter::Filter(std::string const&)
std::string CSyncSlangFilter::Filter(const std::string& text) {
    std::string result(text);
    std::string::size_type index = 0;
    while (index < text.size()) {
        pthread_rwlock_rdlock(&rwlock_);
        const unsigned int n = match(text.substr(index));
        pthread_rwlock_unlock(&rwlock_);
        if (n) {
            result.replace(index, n, std::string(n, '*'));
            index += n;
        } else {
            ++index;
        }
    }
    return result;
}

// IDA_EA: 0x862E622
// IDA_NAME: _ZN16CSyncSlangFilter8HasSlangERKSs
// DEMANGLED: CSyncSlangFilter::HasSlang(std::string const&)
int CSyncSlangFilter::HasSlang(const std::string& text) {
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        pthread_rwlock_rdlock(&rwlock_);
        const bool matched = match(text.substr(index)) != 0;
        pthread_rwlock_unlock(&rwlock_);
        if (matched) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x862E72A
// IDA_NAME: _ZN16CSyncSlangFilter8AddSlangERKSs
// DEMANGLED: CSyncSlangFilter::AddSlang(std::string const&)
int CSyncSlangFilter::AddSlang(const std::string& text) {
    char buffer[257];
    ::memset(buffer, 0, sizeof(buffer));
    ::snprintf(buffer, 0x100u, "%s", text.c_str());
    buffer[255] = 0;

    SlangNode* node = slang_->root;
    for (std::string::size_type index = 0; index < text.size(); ++index) {
        pthread_rwlock_wrlock(&rwlock_);
        node = node->AddChild(
            static_cast<unsigned char>(buffer[index]));
        pthread_rwlock_unlock(&rwlock_);
    }
    node->SetEndNode(true);
    return 1;
}
