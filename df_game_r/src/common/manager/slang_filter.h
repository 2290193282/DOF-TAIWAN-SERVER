#ifndef OLDSRV_GLOBAL_SLANG_FILTER_H_
#define OLDSRV_GLOBAL_SLANG_FILTER_H_

#include <iosfwd>
#include <pthread.h>
#include <stddef.h>
#include <string>
#include <tr1/unordered_map>

// IDA unit: 99_misc_global/FlexLexer,
// 99_misc_global/slang_filter__FlexLexer, and 99_misc_global/SlangFilter.
class FlexLexer {
public:
    FlexLexer();
    virtual ~FlexLexer();

    char* YYText() const;

protected:
    char* yytext_;            // +0x04
    int yyleng_;              // +0x08
    int yylineno_;            // +0x0c
    int yy_flex_debug_;       // +0x10
};

class __slang_filter__FlexLexer : public FlexLexer {
public:
    __slang_filter__FlexLexer(std::istream* input, std::ostream* output);
    virtual ~__slang_filter__FlexLexer();

protected:
    int yy_start_stack_ptr_;       // +0x14
    int yy_start_stack_depth_;     // +0x18
    int* yy_start_stack_;          // +0x1c
    std::istream* yyin_;           // +0x20
    std::ostream* yyout_;          // +0x24
    char yy_hold_char_;            // +0x28
    char reserved_29_[3];          // +0x29
    int yy_n_chars_;               // +0x2c
    char* yy_c_buf_p_;             // +0x30
    int yy_init_;                  // +0x34
    int yy_start_;                 // +0x38
    int yy_did_buffer_switch_;     // +0x3c
    unsigned int buffer_top_;      // +0x40
    unsigned int buffer_max_;      // +0x44
    void** buffer_stack_;          // +0x48
    int last_accepting_state_;     // +0x4c
    char* last_accepting_cpos_;    // +0x50
    int* state_buf_;               // +0x54
    int* state_ptr_;               // +0x58
    char* full_match_;             // +0x5c
    int* full_state_;              // +0x60
    int full_lp_;                  // +0x64
    int lp_;                       // +0x68
    int looking_for_trail_;        // +0x6c
    int more_flag_;                // +0x70
    int more_len_;                 // +0x74
    int more_offset_;              // +0x78
    int prev_more_offset_;         // +0x7c
};

class SlangFilter : public __slang_filter__FlexLexer {
public:
    SlangFilter();
    virtual ~SlangFilter();

    virtual int LexerOutput(const char* text, int length);
    virtual int LexerError(const char* text);
};

// IDA_EA: SlangNode @0x862E910/0x862E92A; CSyncSlangFilter
// GetInstance/FreeInstance @0x8235001/0x82A7147.
// IDA unit: 99_misc_global/SlangNode.
class SlangNode {
public:
    SlangNode();
    ~SlangNode();

    SlangNode* FindChild(unsigned int value) const;
    SlangNode* AddChild(unsigned int value);
    int IsEndNode() const;
    SlangNode* SetEndNode(bool end_node);

private:
    typedef std::tr1::unordered_map<unsigned int, SlangNode*> ChildMap;

    ChildMap children_;
    bool end_node_;

    SlangNode(const SlangNode&);
    SlangNode& operator=(const SlangNode&);
};

// IDA unit: 99_misc_global/CSyncSlangFilter.
class CSyncSlangFilter {
public:
    struct IMPL {
        IMPL();
        ~IMPL();

        SlangNode* root;
    };

    static CSyncSlangFilter* GetInstance();
    static void FreeInstance();

    CSyncSlangFilter();
    ~CSyncSlangFilter();

    int AddSlangList();
    int AddSlangListName();
    unsigned int MatchName(const std::string& text) const;
    int HasSlangName(const std::string& text);
    int AddSlangName(const std::string& text);
    unsigned int match(const std::string& text) const;
    std::string Filter(const std::string& text);
    int HasSlang(const std::string& text);
    int AddSlang(const std::string& text);

private:
    IMPL* slang_;
    IMPL* name_slang_;
    pthread_rwlock_t rwlock_;

    static CSyncSlangFilter* instance_;

    CSyncSlangFilter(const CSyncSlangFilter&);
    CSyncSlangFilter& operator=(const CSyncSlangFilter&);
};


using ::CSyncSlangFilter;
using ::SlangNode;


#endif  // OLDSRV_GLOBAL_SLANG_FILTER_H_
