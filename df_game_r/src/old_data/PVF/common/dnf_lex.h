#ifndef OLDSRV_OLD_DATA_PVF_COMMON_DNF_LEX_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_DNF_LEX_H_

#include <istream>
#include <stack>
#include <stdint.h>
#include <string>
#include <vector>

#include "old_data/PVF/common/dnf_flex_lexer.h"

typedef char TCHAR;

class ScriptStringManager;
class DNFLexWrapper;

// IDA_EA: 0x8AD0130 / 0x8AD0460.  These helpers decode the length-prefixed
// strings embedded in real compiled RDAR streams.  The caller owns the stream
// bounds; both functions preserve the old-server offset advance even when the
// fixed destination form truncates its copy.
int readStringFromBuffer(char* buffer, int max_size, char*& offset);
void readStringFromBuffer(std::string& string_buffer, char*& offset);

// IDA unit: 99_misc_global/DNFLexWrapper and RDARScript.cpp.
// IDA_EA: DNFLex ctor @0x8ACD140; DNFLexWrapper ctor @0x8ACD5B0;
// loadRDARScriptFile @0x8ACD490.
enum DNFLEX_TOKEN {
    DNF_TOK_EOF = 0,
    DNF_TOK_ERROR = 1,
    DNF_TOK_DECIMAL = 2,
    DNF_TOK_HEXADECIMAL = 3,
    DNF_TOK_FLOAT = 4,
    DNF_TOK_TYPE = 5,
    DNF_TOK_CUSTOM_DATA = 6,
    DNF_TOK_STRING = 7,
    DNF_TOK_CONNECTOR = 8,
    DNF_TOK_PAIR_INT = 9,
    DNF_TOK_PAIR_STR = 10,
    DNF_TOK_INCLUDE = 11,
    DNF_TOK_MAX = 12
};

class DNFLex : public __dnf_script__FlexLexer {
public:
    struct stream_data_t {
        std::istream* stream;
        unsigned int size;
        char* buffer;

        void clear();
        void destroy();
    };

    struct stream_t {
        std::string dir;
        std::string filename;
        stream_data_t s;
        unsigned int line_number;

        ~stream_t();
    };

    typedef bool (*TIsDbcsLeadChar)(char);
    typedef void (*TLexerOutput)(const char*, int);
    typedef void (*TLexerError)(const char*);
    typedef bool (*TLoadStream)(const TCHAR*, const TCHAR*, stream_data_t*);

    DNFLex();
    virtual ~DNFLex();

    void SetHandler(TIsDbcsLeadChar is_dbcs_lead_char,
                    TLexerOutput lexer_output,
                    TLexerError lexer_error,
                    TLoadStream load_stream);
    bool LoadStream(const TCHAR* dir,
                    const TCHAR* filename,
                    stream_data_t* stream_data);
    bool IsStream();
    stream_t* GetInputStream();
    int getLineNumber();
    void DisableInclusion();
    void EnableInclusion();
    std::string getStreamName();
    bool PopInputStream(bool pop_all);
    void CloseInputStream();
    bool PushInputStream(const TCHAR* dir,
                         const TCHAR* filename,
                         stream_data_t* stream_data);
    bool SwitchInputStream(const TCHAR* dir,
                           const TCHAR* filename,
                           stream_data_t* stream_data);
    DNFLEX_TOKEN get_next_token();
    virtual int yylex();
    const char* tok_str();
    int tok_str_len();
    virtual void LexerOutput(const char* buffer, int size);
    virtual void LexerError(const char* message);

private:
    friend class DNFLexWrapper;

    // __dnf_script__FlexLexer is the original 0x80-byte base.  Keeping it as
    // a real base class is required for the original Flex buffer/vtable ABI.
    std::vector<stream_t> stream_stack_;  // +0x0080
    char ERR_BUF_[8192];                  // +0x008c
    char STR_TOK_BUF_[8192];              // +0x208c
    char* m_str_tok_pos_;                 // +0x408c
    bool is_dbcs_;                        // +0x4090
    bool allow_inclusion_;                // +0x4091
    char pad_4092_[2];
    TIsDbcsLeadChar IsDbcsLeadChar_;  // +0x4094
    TLexerOutput LexerOutput_;        // +0x4098
    TLexerError LexerError_;          // +0x409c
    TLoadStream LoadStream_;          // +0x40a0
};

class CompiledDNFLex {
public:
    struct stream_t {
        std::string dir;
        std::string filename;
        unsigned int size;
        char* buffer;
        char* offset;

        ~stream_t();
    };

    typedef void (*TLexerError)(const char*);
    typedef bool (*TLoadStream)(const TCHAR*, const TCHAR*,
                                DNFLex::stream_data_t*);

    CompiledDNFLex();
    // IDA_EA: 0x8AD0580.  The supplied table remains caller-owned.
    explicit CompiledDNFLex(ScriptStringManager* string_manager);
    ~CompiledDNFLex();

    void SetHandler(TLexerError lexer_error, TLoadStream load_stream);
    void LexerError(const char* message);
    bool LoadStream(const TCHAR* dir,
                    const TCHAR* filename,
                    DNFLex::stream_data_t* stream_data);
    bool IsStream() const;
    stream_t* GetInputStream();
    const stream_t* GetInputStream() const;
    int GetOffset() const;
    int getInt(bool* result);
    float getFloat(bool* result);
    bool isEnd();
    void setStringManager(ScriptStringManager* string_manager,
                          bool allow_owner);
    ScriptStringManager* getStringManager();
    void DisableInclusion();
    void EnableInclusion();
    bool getStr(std::string& string_buffer);
    bool getStr(char* buffer, int max_size);
    const char* getStr(bool* result);
    std::string getStreamName() const;
    void destroy();
    void create(bool create_string_table);
    void CloseInputStream();
    bool PushInputStream(const TCHAR* dir,
                         const TCHAR* filename,
                         char* buffer,
                         unsigned int buffer_size);
    bool PopInputStream(bool pop_all);
    bool SwitchInputStream(const TCHAR* dir,
                           const TCHAR* filename,
                           char* buffer,
                           unsigned int buffer_size);
    DNFLEX_TOKEN getToken();

private:
    friend class DNFLexWrapper;

    void* vftable_;                    // +0x0000
    char* offset_;                     // +0x0004
    DNFLEX_TOKEN lastToken_;           // +0x0008
    bool stringManagerDelete_;         // +0x000c
    char pad_000d_[3];
    ScriptStringManager* stringManager_;  // +0x0010
    char* stringBuffer_;               // +0x0014
    char ERR_BUF_[8192];               // +0x0018
    std::stack<stream_t> stream_stack_;  // +0x2018
    bool allow_inclusion_;             // +0x2040
    char pad_2041_[3];
    TLexerError LexerError_;           // +0x2044
    TLoadStream LoadStream_;           // +0x2048
};

class DNFLexWrapper {
public:
    enum ENUM_LEX_TYPE {
        LEX_TYPE_NORMAL = 0,
        LEX_TYPE_COMPILED = 1
    };

    typedef void (*TLexerError)(const char*);
    typedef bool (*TLoadStream)(const TCHAR*, const TCHAR*,
                                DNFLex::stream_data_t*);

    DNFLexWrapper();
    ~DNFLexWrapper();

    void setLexType(ENUM_LEX_TYPE lex_type);
    ENUM_LEX_TYPE getLexType();
    bool LoadStream(const TCHAR* dir,
                    const TCHAR* filename,
                    DNFLex::stream_data_t* stream_data);
    int getDecimal();
    const char* getType();
    const char* getCustomData();
    const char* getConnector();
    DNFLEX_TOKEN getLastToken();
    const char* getLastTokenData();
    int getLastLineNo();
    std::string getLastTokenDataAsString();
    bool setStringTableFromFile(const TCHAR* file_name);
    bool setStringTableFromBuffer(const unsigned char* buffer, int size);
    const char* getPairStr();
    int getPairInt();
    const char* getString();
    float getFloat();
    int getHexDecimal();
    DNFLEX_TOKEN getToken();
    std::string getStreamName();
    std::istream* getInputStream();
    int getInputStreamSize();
    char* getInputStreamBuffer();
    bool setStringStreamData(DNFLex::stream_data_t* stream_data,
                             char* buffer,
                             uint32_t buffer_size);
    void closeInputStream();
    bool switchInputStream(const TCHAR* dir,
                           const TCHAR* filename,
                           DNFLex::stream_data_t* stream_data);
    // IDA_EA: 0x8ACF060.
    // The channel loader supplies either an already-owned stream or a real
    // file path; it always selects the normal (non-compiled) lexer path.
    bool switchChannelInputStream(const TCHAR* filename,
                                  DNFLex::stream_data_t* stream_data);
    void SetHandler(DNFLex::TIsDbcsLeadChar is_dbcs_lead_char,
                    DNFLex::TLexerOutput lexer_output,
                    TLexerError lexer_error,
                    TLoadStream load_stream);
    bool setFileStreamData(DNFLex::stream_data_t* stream_data,
                           const TCHAR* path);

    // The installCompiledStringTableManager source macro operates on this
    // embedded compiled lexer.  Returning the named subobject keeps the
    // macro source-compatible without reopening DNFLexWrapper at +0x40BC.
    CompiledDNFLex& compiledDnfLex() { return compiledDnfLex_; }

private:
    void* vftable_;                    // +0x0000
    DNFLEX_TOKEN lastToken_;           // +0x0004
    int lastIntValue_;                 // +0x0008
    float lastFloatValue_;             // +0x000c
    const char* lastStringValue_;      // +0x0010
    ENUM_LEX_TYPE lexType_;            // +0x0014
    DNFLex dnfLex_;                    // +0x0018
    CompiledDNFLex compiledDnfLex_;    // +0x40bc
    TLexerError LexerError_;           // +0x6108
    TLoadStream LoadStream_;           // +0x610c
};

extern DNFLexWrapper g_dnfLex;
extern void (*g_onLoadScriptFileFunction)();

bool loadRDARScriptFile(const char* dir, const char* filename);
void unloadRDARScriptFile();

#ifndef OLDSRV_DISABLE_LEX_STRING_TABLE_INSTALL_MACRO
#define installCompiledStringTableManager() \
    do { \
        ::CompiledDNFLex* oldsrv_compiled_lex__ = \
            &::g_dnfLex.compiledDnfLex(); \
        oldsrv_compiled_lex__->destroy(); \
        oldsrv_compiled_lex__->create(false); \
        oldsrv_compiled_lex__->setStringManager(&g_script_string_table, false); \
    } while (0)
#endif

static_assert(sizeof(DNFLex::stream_data_t) == 0x0Cu,
              "DNFLex::stream_data_t IDA ABI size must stay 0x0C");
static_assert(sizeof(DNFLex::stream_t) == 0x18u,
              "DNFLex::stream_t IDA ABI size must stay 0x18");
static_assert(sizeof(DNFLex) == 0x40A4u,
              "DNFLex IDA ABI size must stay 0x40A4");
static_assert(sizeof(CompiledDNFLex::stream_t) == 0x14u,
              "CompiledDNFLex::stream_t IDA ABI size must stay 0x14");
static_assert(sizeof(CompiledDNFLex) == 0x204Cu,
              "CompiledDNFLex IDA ABI size must stay 0x204C");
static_assert(sizeof(DNFLexWrapper) == 0x6110u,
              "DNFLexWrapper/g_dnfLex IDA ABI size must stay 0x6110");


#endif  // OLDSRV_OLD_DATA_PVF_COMMON_DNF_LEX_H_
