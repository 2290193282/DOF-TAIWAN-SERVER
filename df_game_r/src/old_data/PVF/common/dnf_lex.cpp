#define OLDSRV_DISABLE_LEX_STRING_TABLE_INSTALL_MACRO
#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/dnf_lex_dfa_tables.inc"

#include "old_data/PVF/common/script_string_manager.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <new>
#include <cstdio>
#include <strstream>


DNFLexWrapper g_dnfLex;
void (*g_onLoadScriptFileFunction)() = 0;

namespace {

int ReadIntFromBuffer(char*& offset) {
    int value = 0;
    if (offset != 0) {
        std::memcpy(&value, offset, sizeof(value));
        offset += sizeof(value);
    }
    return value;
}

float ReadFloatFromBuffer(char*& offset) {
    float value = 0.0f;
    if (offset != 0) {
        std::memcpy(&value, offset, sizeof(value));
        offset += sizeof(value);
    }
    return value;
}

uint16_t ReadWordFromBuffer(char*& offset) {
    uint16_t value = 0;
    std::memcpy(&value, offset, sizeof(value));
    offset += sizeof(value);
    return value;
}

DNFLEX_TOKEN ReadCharTokenFromBuffer(char*& offset) {
    if (offset == 0) {
        return DNF_TOK_EOF;
    }
    const unsigned char value = static_cast<unsigned char>(*offset);
    ++offset;
    return static_cast<DNFLEX_TOKEN>(value);
}

bool IsValueStringToken(DNFLEX_TOKEN token) {
    return token >= DNF_TOK_TYPE && token < DNF_TOK_MAX &&
           token != DNF_TOK_PAIR_INT;
}

}  // namespace

// IDA_EA: 0x8AD0130
// IDA_NAME: _Z20readStringFromBufferPciRS_
// DEMANGLED: readStringFromBuffer(char *,int,char *&)
int readStringFromBuffer(char* buffer, int max_size, char*& offset) {
    const uint16_t length = ReadWordFromBuffer(offset);
    char* const source = offset;
    int copied = static_cast<int>(length);
    if (length > max_size - 1) {
        copied = max_size - 1;
    }
    std::memcpy(buffer, source, static_cast<size_t>(copied));
    buffer[copied] = 0;
    offset += length;
    return copied;
}

// IDA_EA: 0x8AD0460
// IDA_NAME: _Z20readStringFromBufferRSsRPc
// DEMANGLED: readStringFromBuffer(std::string &,char *&)
void readStringFromBuffer(std::string& string_buffer, char*& offset) {
    const uint16_t length = ReadWordFromBuffer(offset);
    string_buffer.resize(static_cast<size_t>(length) + 1u, '\0');
    string_buffer.assign(offset, static_cast<size_t>(length));
    offset += length;
}

// IDA_EA: 0x8ACD140
// IDA_NAME: _ZN6DNFLex13stream_data_t5clearEv
// DEMANGLED: DNFLex::stream_data_t::clear(void)
void DNFLex::stream_data_t::clear() {
    stream = 0;
    size = 0;
    buffer = 0;
}

// IDA_EA: 0x8ACD450
// IDA_NAME: _ZN6DNFLex13stream_data_t7destroyEv
// DEMANGLED: DNFLex::stream_data_t::destroy(void)
void DNFLex::stream_data_t::destroy() {
    if (stream != 0) {
        delete stream;
        stream = 0;
    }
    if (buffer != 0) {
        delete[] buffer;
        buffer = 0;
    }
}

// IDA_EA: 0x8ACDC30
// IDA_NAME: _ZN6DNFLex8stream_tD2Ev
// DEMANGLED: DNFLex::stream_t::~stream_t()
DNFLex::stream_t::~stream_t() {}

// IDA_EA: 0x8AD1320
// IDA_NAME: _ZN14CompiledDNFLex8stream_tD2Ev
// DEMANGLED: CompiledDNFLex::stream_t::~stream_t()
CompiledDNFLex::stream_t::~stream_t() {}

// IDA_EA: 0x8ACD3A0
// IDA_NAME: _ZN6DNFLexC2Ev
// DEMANGLED: DNFLex::DNFLex(void)
DNFLex::DNFLex()
    : __dnf_script__FlexLexer(0, 0),
      stream_stack_(),
      m_str_tok_pos_(0),
      is_dbcs_(false),
      allow_inclusion_(true),
      IsDbcsLeadChar_(0),
      LexerOutput_(0),
      LexerError_(0),
      LoadStream_(0) {
    std::memset(ERR_BUF_, 0, sizeof(ERR_BUF_));
    std::memset(STR_TOK_BUF_, 0, sizeof(STR_TOK_BUF_));
}

// IDA_EA: 0x8ACD5F0
// IDA_NAME: _ZN6DNFLexD2Ev
// DEMANGLED: DNFLex::~DNFLex()
DNFLex::~DNFLex() {
    CloseInputStream();
}

// IDA_EA: 0x8ACD160
// IDA_NAME: _ZN6DNFLex10SetHandlerEPFbcEPFvPKciEPFvS3_EPFbS3_S3_PNS_13stream_data_tEE
// DEMANGLED: DNFLex::SetHandler(bool (*)(char),void (*)(char const*,int),void (*)(char const*),bool (*)(char const*,char const*,DNFLex::stream_data_t *))
void DNFLex::SetHandler(TIsDbcsLeadChar is_dbcs_lead_char,
                        TLexerOutput lexer_output,
                        TLexerError lexer_error,
                        TLoadStream load_stream) {
    IsDbcsLeadChar_ = is_dbcs_lead_char;
    LexerOutput_ = lexer_output;
    LexerError_ = lexer_error;
    LoadStream_ = load_stream;
}

// IDA_EA: 0x8ACD190
// IDA_NAME: _ZN6DNFLex11LexerOutputEPKci
// DEMANGLED: DNFLex::LexerOutput(char const*,int)
void DNFLex::LexerOutput(const char* buffer, int size) {
    if (LexerOutput_ != 0) {
        LexerOutput_(buffer, size);
    }
}

// IDA_EA: 0x8ACD1C0
// IDA_NAME: _ZN6DNFLex10LexerErrorEPKc
// DEMANGLED: DNFLex::LexerError(char const*)
void DNFLex::LexerError(const char* message) {
    if (LexerError_ != 0) {
        LexerError_(message);
    }
}

// IDA_EA: 0x8ACD1F0
// IDA_NAME: _ZN6DNFLex7tok_strEv
// DEMANGLED: DNFLex::tok_str(void)
const char* DNFLex::tok_str() {
    return STR_TOK_BUF_;
}

// IDA_EA: 0x8ACD200
// IDA_NAME: _ZN6DNFLex11tok_str_lenEv
// DEMANGLED: DNFLex::tok_str_len(void)
int DNFLex::tok_str_len() {
    if (m_str_tok_pos_ == 0) {
        return 0;
    }
    return static_cast<int>(m_str_tok_pos_ - STR_TOK_BUF_);
}

// IDA_EA: 0x8ACD220
// IDA_NAME: _ZN6DNFLex10LoadStreamEPKcS1_PNS_13stream_data_tE
// DEMANGLED: DNFLex::LoadStream(char const*,char const*,DNFLex::stream_data_t *)
bool DNFLex::LoadStream(const TCHAR* dir,
                        const TCHAR* filename,
                        stream_data_t* stream_data) {
    if (LoadStream_ != 0) {
        return LoadStream_(dir, filename, stream_data);
    }
    return false;
}

// IDA_EA: 0x8ACD260
// IDA_NAME: _ZN6DNFLex8IsStreamEv
// DEMANGLED: DNFLex::IsStream(void)
bool DNFLex::IsStream() {
    return !stream_stack_.empty();
}

// IDA_EA: 0x8ACD280
// IDA_NAME: _ZN6DNFLex14GetInputStreamEv
// DEMANGLED: DNFLex::GetInputStream(void)
DNFLex::stream_t* DNFLex::GetInputStream() {
    if (stream_stack_.empty()) {
        return 0;
    }
    return &stream_stack_.back();
}

// IDA_EA: 0x8ACD2B0
// IDA_NAME: _ZN6DNFLex13getLineNumberEv
// DEMANGLED: DNFLex::getLineNumber(void)
int DNFLex::getLineNumber() {
    return yylineno_;
}

// IDA_EA: 0x8ACD2C0
// IDA_NAME: _ZN6DNFLex16DisableInclusionEv
// DEMANGLED: DNFLex::DisableInclusion(void)
void DNFLex::DisableInclusion() {
    allow_inclusion_ = false;
}

// IDA_EA: 0x8ACD2D0
// IDA_NAME: _ZN6DNFLex15EnableInclusionEv
// DEMANGLED: DNFLex::EnableInclusion(void)
void DNFLex::EnableInclusion() {
    allow_inclusion_ = true;
}

// IDA_EA: 0x8ACD310
// IDA_NAME: _ZN6DNFLex13getStreamNameEv
// DEMANGLED: DNFLex::getStreamName(void)
std::string DNFLex::getStreamName() {
    if (stream_stack_.empty()) {
        return "NONE";
    }
    return stream_stack_.back().dir + stream_stack_.back().filename;
}

// IDA_EA: 0x8ACD490
// IDA_NAME: _ZN6DNFLex14PopInputStreamEb
// DEMANGLED: DNFLex::PopInputStream(bool)
bool DNFLex::PopInputStream(bool pop_all) {
    if (stream_stack_.empty()) {
        return false;
    }
    if (!pop_all && stream_stack_.size() <= 1) {
        return false;
    }

    yypop_buffer_state();
    stream_stack_.back().s.destroy();
    stream_stack_.pop_back();
    yylineno_ = stream_stack_.empty() ? 1 : stream_stack_.back().line_number;
    yy_start_ = 1;
    return true;
}

// IDA_EA: 0x8ACD5B0
// IDA_NAME: _ZN6DNFLex16CloseInputStreamEv
// DEMANGLED: DNFLex::CloseInputStream(void)
void DNFLex::CloseInputStream() {
    while (PopInputStream(true)) {}
}

// IDA_EA: 0x8ACD7D0
// IDA_NAME: _ZN6DNFLex15PushInputStreamEPKcS1_PNS_13stream_data_tE
// DEMANGLED: DNFLex::PushInputStream(char const*,char const*,DNFLex::stream_data_t *)
bool DNFLex::PushInputStream(const TCHAR* dir,
                             const TCHAR* filename,
                             stream_data_t* stream_data) {
    stream_t stream_info;
    stream_info.dir.assign(dir != 0 ? dir : "");
    stream_info.filename.assign(filename != 0 ? filename : "");

    if (stream_data != 0) {
        stream_info.s = *stream_data;
    } else if (!LoadStream(dir, filename, &stream_info.s)) {
        std::sprintf(ERR_BUF_, "%s - DNFLex::ReadStream failed.",
                     "PushInputStream");
        LexerError(ERR_BUF_);
        return false;
    }

    if (!stream_stack_.empty()) {
        stream_stack_.back().line_number = yylineno_;
    }
    stream_stack_.push_back(stream_info);
    yypush_buffer_state(yy_create_buffer(stream_info.s.stream,
                                          static_cast<int>(stream_info.s.size)));
    yy_start_ = 1;
    yylineno_ = 1;
    return true;
}

// IDA_EA: 0x8ACDA50
// IDA_NAME: _ZN6DNFLex17SwitchInputStreamEPKcS1_PNS_13stream_data_tE
// DEMANGLED: DNFLex::SwitchInputStream(char const*,char const*,DNFLex::stream_data_t *)
bool DNFLex::SwitchInputStream(const TCHAR* dir,
                               const TCHAR* filename,
                               stream_data_t* stream_data) {
    CloseInputStream();
    return PushInputStream(dir, filename, stream_data);
}

// IDA_EA: 0x8ACDAA0
// IDA_NAME: _ZN6DNFLex14get_next_tokenEv
// DEMANGLED: DNFLex::get_next_token(void)
DNFLEX_TOKEN DNFLex::get_next_token() {
    if (stream_stack_.empty()) {
        return DNF_TOK_EOF;
    }

    DNFLEX_TOKEN token = static_cast<DNFLEX_TOKEN>(yylex());
    if (!allow_inclusion_) {
        return token;
    }
    if (token != DNF_TOK_INCLUDE) {
        if (token == DNF_TOK_EOF && PopInputStream(false)) {
            return static_cast<DNFLEX_TOKEN>(yylex());
        }
        return token;
    }

    if (static_cast<DNFLEX_TOKEN>(yylex()) == DNF_TOK_STRING &&
        PushInputStream("", STR_TOK_BUF_, 0)) {
        return static_cast<DNFLEX_TOKEN>(yylex());
    }

    stream_t* stream = GetInputStream();
    std::snprintf(ERR_BUF_, sizeof(ERR_BUF_),
                  "%s - %s/%s, line %d, invalid inclusion.",
                  "get_next_token",
                  stream != 0 ? stream->dir.c_str() : "",
                  stream != 0 ? stream->filename.c_str() : "",
                  yylineno_);
    LexerError(ERR_BUF_);
    return DNF_TOK_ERROR;
}

// IDA_EA: 0x8AD2860
// IDA_NAME: _ZN6DNFLex5yylexEv
// DEMANGLED: DNFLex::yylex(void)
int DNFLex::yylex() {
    using namespace oldsrv_dnf_lex_dfa;

    int yy_current_state = 0;
    int yy_act = 0;
    int yy_amount_of_matched_text = 0;
    char* yy_bp = 0;

    if (!yy_init_) {
        yy_init_ = 1;
        if (!yy_start_) {
            yy_start_ = 1;
        }
        if (!yyin_) {
            yyin_ = &std::cin;
        }
        if (!yyout_) {
            yyout_ = &std::cout;
        }
        if (!yy_buffer_stack_ || !yy_buffer_stack_[yy_buffer_stack_top_]) {
            yyensure_buffer_stack();
            yy_buffer_stack_[yy_buffer_stack_top_] =
                yy_create_buffer(yyin_, 0x4000);
        }
        yy_load_buffer_state();
    }

yy_match:
    *yy_c_buf_p_ = yy_hold_char_;
    yy_bp = yy_c_buf_p_;
    yy_current_state = yy_start_;

yy_match_loop:
    for (;;) {
        if (yy_accept[yy_current_state] != 0) {
            yy_last_accepting_state_ = yy_current_state;
            yy_last_accepting_cpos_ = yy_c_buf_p_;
        }

        const unsigned char input = static_cast<unsigned char>(yy_hold_char_);
        int transition = yy_base[yy_current_state];
        while (yy_chk[transition + input] != yy_current_state) {
            yy_current_state = yy_def[yy_current_state];
            transition = yy_base[yy_current_state];
        }

        ++yy_c_buf_p_;
        yy_current_state = yy_nxt[transition + input];
        if (yy_base[yy_current_state] == 1913) {
            break;
        }
        yy_hold_char_ = *yy_c_buf_p_;
    }

yy_find_action:
    yy_act = yy_accept[yy_current_state];
    if (yy_act == 0) {
        yy_c_buf_p_ = yy_last_accepting_cpos_;
        yy_act = yy_accept[yy_last_accepting_state_];
    }
    yytext_ = yy_bp;
    yyleng_ = static_cast<int>(yy_c_buf_p_ - yy_bp);
    yy_hold_char_ = *yy_c_buf_p_;
    *yy_c_buf_p_ = 0;

    if (yy_act != 21 && yy_rule_can_match_eol[yy_act] != 0) {
        for (int index = 0; index < yyleng_; ++index) {
            if (yytext_[index] == '\n') {
                ++yylineno_;
            }
        }
    }

yy_action:
    switch (yy_act) {
        case 0:
            *yy_c_buf_p_ = yy_hold_char_;
            yy_c_buf_p_ = yy_last_accepting_cpos_;
            yy_current_state = yy_last_accepting_state_;
            goto yy_find_action;
        case 1:
            return DNF_TOK_DECIMAL;
        case 2:
            return DNF_TOK_HEXADECIMAL;
        case 3:
            return DNF_TOK_FLOAT;
        case 4:
            return DNF_TOK_INCLUDE;
        case 5:
            return DNF_TOK_TYPE;
        case 6:
            return DNF_TOK_CUSTOM_DATA;
        case 7:
            return DNF_TOK_CONNECTOR;
        case 8:
            return DNF_TOK_PAIR_INT;
        case 9:
            return DNF_TOK_PAIR_STR;
        case 10:
            yy_start_ = 3;
            goto yy_match;
        case 11:
        case 14:
            yy_start_ = 1;
            goto yy_match;
        case 12:
        case 15:
        case 18:
            goto yy_match;
        case 13:
            yy_start_ = 5;
            goto yy_match;
        case 16:
            is_dbcs_ = false;
            yy_start_ = 7;
            m_str_tok_pos_ = STR_TOK_BUF_;
            goto yy_match;
        case 17:
            if (is_dbcs_) {
                *m_str_tok_pos_++ = *yytext_;
                is_dbcs_ = false;
                goto yy_match;
            }
            if (*yytext_ == '`') {
                *m_str_tok_pos_ = 0;
                yy_start_ = 1;
                return DNF_TOK_STRING;
            }
            if (m_str_tok_pos_ < &STR_TOK_BUF_[8190]) {
                const char input = *yytext_;
                *m_str_tok_pos_++ = input;
                is_dbcs_ = IsDbcsLeadChar_(input);
                goto yy_match;
            }
            if (IsStream()) {
                stream_t* stream = GetInputStream();
                std::sprintf(ERR_BUF_,
                             "%s - %s/%s, line %d, string buffer overflow detected.",
                             "yylex", stream->dir.c_str(),
                             stream->filename.c_str(), yylineno_);
            } else {
                std::sprintf(ERR_BUF_, "%s - the input stream is empty.",
                             "yylex");
            }
            LexerError(ERR_BUF_);
            return DNF_TOK_ERROR;
        case 19:
            if (!IsStream()) {
                goto yy_match;
            }
            {
                stream_t* stream = GetInputStream();
                const char input = *yytext_;
                if (input != 0) {
                    std::sprintf(ERR_BUF_,
                                 "%s - %s/%s, line %d, unrecognized character '%c'.",
                                 "yylex", stream->dir.c_str(),
                                 stream->filename.c_str(), yylineno_, input);
                } else {
                    std::sprintf(ERR_BUF_,
                                 "%s - %s/%s, line %d, unrecognized character 'NULL'.",
                                 "yylex", stream->dir.c_str(),
                                 stream->filename.c_str(), yylineno_);
                }
                LexerError(ERR_BUF_);
                return DNF_TOK_ERROR;
            }
        case 20:
            LexerOutput(yytext_, yyleng_);
            goto yy_match;
        case 21:
            yy_amount_of_matched_text =
                static_cast<int>(yy_c_buf_p_ - yytext_ - 1);
            *yy_c_buf_p_ = yy_hold_char_;
            {
                yy_buffer_state* buffer =
                    yy_buffer_stack_[yy_buffer_stack_top_];
                if (buffer->yy_buffer_status == 0) {
                    yy_n_chars_ = buffer->yy_n_chars;
                    buffer->yy_buffer_status = 1;
                    buffer->yy_input_file = yyin_;
                }

                if (yy_c_buf_p_ <= buffer->yy_ch_buf + yy_n_chars_) {
                    yy_c_buf_p_ = yytext_ + yy_amount_of_matched_text;
                    yy_current_state = yy_get_previous_state();
                    if (yy_NUL_trans[yy_current_state] != 0) {
                        yy_current_state = yy_NUL_trans[yy_current_state];
                        ++yy_c_buf_p_;
                        yy_bp = yytext_;
                        yy_hold_char_ = *yy_c_buf_p_;
                        goto yy_match_loop;
                    }
                    yy_bp = yytext_;
                    goto yy_find_action;
                }
            }

            switch (yy_get_next_buffer()) {
                case 0:
                    yy_c_buf_p_ = yytext_ + yy_amount_of_matched_text;
                    yy_current_state = yy_get_previous_state();
                    yy_bp = yytext_;
                    yy_hold_char_ = *yy_c_buf_p_;
                    goto yy_match_loop;
                case 2:
                    yy_c_buf_p_ =
                        yy_buffer_stack_[yy_buffer_stack_top_]->yy_ch_buf +
                        yy_n_chars_;
                    yy_current_state = yy_get_previous_state();
                    yy_bp = yytext_;
                    goto yy_find_action;
                case 1:
                    yy_did_buffer_switch_on_eof_ = 0;
                    if (yywrap()) {
                        yy_c_buf_p_ = yytext_;
                        yy_act = (yy_start_ - 1) / 2 + 22;
                        goto yy_action;
                    }
                    if (!yy_did_buffer_switch_on_eof_) {
                        yyrestart(yyin_);
                    }
                    goto yy_match;
                default:
                    goto yy_match;
            }
        case 22:
        case 24:
        case 25:
            return DNF_TOK_EOF;
        case 23:
            std::strcpy(ERR_BUF_, "comment nest mismatch");
            LexerError(ERR_BUF_);
            return DNF_TOK_ERROR;
        default:
            LexerError("fatal flex scanner internal error--no action found");
            goto yy_match;
    }
}

// IDA_EA: 0x8AD07C0
// IDA_NAME: _ZN14CompiledDNFLexC2Ev
// DEMANGLED: CompiledDNFLex::CompiledDNFLex(void)
CompiledDNFLex::CompiledDNFLex()
    : vftable_(0),
      offset_(0),
      lastToken_(DNF_TOK_EOF),
      stringManagerDelete_(false),
      stringManager_(0),
      stringBuffer_(0),
      stream_stack_(),
      allow_inclusion_(true),
      LexerError_(0),
      LoadStream_(0) {
    std::memset(ERR_BUF_, 0, sizeof(ERR_BUF_));
    create(true);
}

// IDA_EA: 0x8AD0580
// IDA_NAME: _ZN14CompiledDNFLexC2EP19ScriptStringManager
// DEMANGLED: CompiledDNFLex::CompiledDNFLex(ScriptStringManager *)
CompiledDNFLex::CompiledDNFLex(ScriptStringManager* string_manager)
    : vftable_(0),
      offset_(0),
      lastToken_(DNF_TOK_EOF),
      stringManagerDelete_(false),
      stringManager_(string_manager),
      stringBuffer_(0),
      stream_stack_(),
      allow_inclusion_(true),
      LexerError_(0),
      LoadStream_(0) {
    std::memset(ERR_BUF_, 0, sizeof(ERR_BUF_));
    create(false);
}

// IDA_EA: 0x8AD0ED0
// IDA_NAME: _ZN14CompiledDNFLexD2Ev
// DEMANGLED: CompiledDNFLex::~CompiledDNFLex()
CompiledDNFLex::~CompiledDNFLex() {
    CloseInputStream();
    destroy();
}

// IDA_EA: 0x8ACFDF0
// IDA_NAME: _ZN14CompiledDNFLex10SetHandlerEPFvPKcEPFbS1_S1_PN6DNFLex13stream_data_tEE
// DEMANGLED: CompiledDNFLex::SetHandler(void (*)(char const*),bool (*)(char const*,char const*,DNFLex::stream_data_t *))
void CompiledDNFLex::SetHandler(TLexerError lexer_error,
                                TLoadStream load_stream) {
    LexerError_ = lexer_error;
    LoadStream_ = load_stream;
}

// IDA_EA: 0x8ACFE10
// IDA_NAME: _ZN14CompiledDNFLex10LexerErrorEPKc
// DEMANGLED: CompiledDNFLex::LexerError(char const*)
void CompiledDNFLex::LexerError(const char* message) {
    if (LexerError_ != 0) {
        LexerError_(message);
    }
}

// IDA_EA: 0x8ACFE40
// IDA_NAME: _ZN14CompiledDNFLex10LoadStreamEPKcS1_PN6DNFLex13stream_data_tE
// DEMANGLED: CompiledDNFLex::LoadStream(char const*,char const*,DNFLex::stream_data_t *)
bool CompiledDNFLex::LoadStream(const TCHAR* dir,
                                const TCHAR* filename,
                                DNFLex::stream_data_t* stream_data) {
    if (LoadStream_ != 0) {
        return LoadStream_(dir, filename, stream_data);
    }
    return false;
}

// IDA_EA: 0x8ACFE80
// IDA_NAME: _ZNK14CompiledDNFLex8IsStreamEv
// DEMANGLED: CompiledDNFLex::IsStream(void)const
bool CompiledDNFLex::IsStream() const {
    return !stream_stack_.empty();
}

// IDA_EA: 0x8ACFEA0
// IDA_NAME: _ZN14CompiledDNFLex14GetInputStreamEv
// DEMANGLED: CompiledDNFLex::GetInputStream(void)
CompiledDNFLex::stream_t* CompiledDNFLex::GetInputStream() {
    if (stream_stack_.empty()) {
        return 0;
    }
    return &stream_stack_.top();
}

// IDA_EA: 0x8ACFEE0
// IDA_NAME: _ZNK14CompiledDNFLex14GetInputStreamEv
// DEMANGLED: CompiledDNFLex::GetInputStream(void)const
const CompiledDNFLex::stream_t* CompiledDNFLex::GetInputStream() const {
    if (stream_stack_.empty()) {
        return 0;
    }
    return &stream_stack_.top();
}

// IDA_EA: 0x8ACFF20
// IDA_NAME: _ZNK14CompiledDNFLex9GetOffsetEv
// DEMANGLED: CompiledDNFLex::GetOffset(void)const
int CompiledDNFLex::GetOffset() const {
    const stream_t* stream = GetInputStream();
    if (stream == 0 || stream->buffer == 0 || offset_ == 0) {
        return -1;
    }
    return static_cast<int>(offset_ - stream->buffer);
}

// IDA_EA: 0x8ACFF70
// IDA_NAME: _ZN14CompiledDNFLex6getIntEPb
// DEMANGLED: CompiledDNFLex::getInt(bool *)
int CompiledDNFLex::getInt(bool* result) {
    if ((lastToken_ >= DNF_TOK_DECIMAL && lastToken_ <= DNF_TOK_HEXADECIMAL) ||
        lastToken_ == DNF_TOK_PAIR_INT) {
        if (result != 0) {
            *result = true;
        }
        return ReadIntFromBuffer(offset_);
    }
    if (result != 0) {
        *result = false;
    }
    return 0;
}

// IDA_EA: 0x8ACFFC0
// IDA_NAME: _ZN14CompiledDNFLex8getFloatEPb
// DEMANGLED: CompiledDNFLex::getFloat(bool *)
float CompiledDNFLex::getFloat(bool* result) {
    if (lastToken_ == DNF_TOK_FLOAT) {
        if (result != 0) {
            *result = true;
        }
        return ReadFloatFromBuffer(offset_);
    }
    if (result != 0) {
        *result = false;
    }
    return 0.0f;
}

// IDA_EA: 0x8AD0000
// IDA_NAME: _ZN14CompiledDNFLex5isEndEv
// DEMANGLED: CompiledDNFLex::isEnd(void)
bool CompiledDNFLex::isEnd() {
    const stream_t* stream = GetInputStream();
    return stream != 0 && offset_ >= stream->buffer + stream->size;
}

// IDA_EA: 0x8AD0050
// IDA_NAME: _ZN14CompiledDNFLex16setStringManagerEP19ScriptStringManagerb
// DEMANGLED: CompiledDNFLex::setStringManager(ScriptStringManager *,bool)
void CompiledDNFLex::setStringManager(ScriptStringManager* string_manager,
                                      bool allow_owner) {
    stringManagerDelete_ = allow_owner;
    stringManager_ = string_manager;
}

// IDA_EA: 0x8AD0070
// IDA_NAME: _ZN14CompiledDNFLex16getStringManagerEv
// DEMANGLED: CompiledDNFLex::getStringManager(void)
ScriptStringManager* CompiledDNFLex::getStringManager() {
    return stringManager_;
}

// IDA_EA: 0x8AD0080
// IDA_NAME: _ZN14CompiledDNFLex16DisableInclusionEv
// DEMANGLED: CompiledDNFLex::DisableInclusion(void)
void CompiledDNFLex::DisableInclusion() {
    allow_inclusion_ = false;
}

// IDA_EA: 0x8AD0090
// IDA_NAME: _ZN14CompiledDNFLex15EnableInclusionEv
// DEMANGLED: CompiledDNFLex::EnableInclusion(void)
void CompiledDNFLex::EnableInclusion() {
    allow_inclusion_ = true;
}

// IDA_EA: 0x8AD00D0
// IDA_NAME: _ZN14CompiledDNFLex6getStrERSs
// DEMANGLED: CompiledDNFLex::getStr(std::string &)
bool CompiledDNFLex::getStr(std::string& string_buffer) {
    if (!IsValueStringToken(lastToken_)) {
        return false;
    }
    const int index = ReadIntFromBuffer(offset_);
    if (stringManager_ == 0) {
        string_buffer.clear();
        return false;
    }
    return stringManager_->getString(index, &string_buffer);
}

// IDA_EA: 0x8AD0190
// IDA_NAME: _ZN14CompiledDNFLex6getStrEPci
// DEMANGLED: CompiledDNFLex::getStr(char *,int)
bool CompiledDNFLex::getStr(char* buffer, int max_size) {
    if (buffer == 0 || max_size <= 0 || !IsValueStringToken(lastToken_)) {
        return false;
    }
    std::string value;
    const bool found = getStr(value);
    if (value.size() > static_cast<size_t>(max_size)) {
        std::memcpy(buffer, value.data(), static_cast<size_t>(max_size - 1));
        buffer[max_size - 1] = 0;
        return false;
    }
    std::memcpy(buffer, value.data(), value.size());
    buffer[value.size()] = 0;
    return found;
}

// IDA_EA: 0x8AD0250
// IDA_NAME: _ZN14CompiledDNFLex6getStrEPb
// DEMANGLED: CompiledDNFLex::getStr(bool *)
const char* CompiledDNFLex::getStr(bool* result) {
    const bool ok = getStr(stringBuffer_, 1024);
    if (result != 0) {
        *result = ok;
    }
    return stringBuffer_;
}

// IDA_EA: 0x8AD0290
// IDA_NAME: _ZNK14CompiledDNFLex13getStreamNameEv
// DEMANGLED: CompiledDNFLex::getStreamName(void)const
std::string CompiledDNFLex::getStreamName() const {
    const stream_t* stream = GetInputStream();
    if (stream == 0) {
        return "NONE";
    }
    return stream->dir + stream->filename;
}

// IDA_EA: 0x8AD0370
// IDA_NAME: _ZN14CompiledDNFLex7destroyEv
// DEMANGLED: CompiledDNFLex::destroy(void)
void CompiledDNFLex::destroy() {
    if (stringManagerDelete_) {
        delete stringManager_;
        stringManagerDelete_ = false;
    }
    stringManager_ = 0;
    delete[] stringBuffer_;
    stringBuffer_ = 0;
}

// IDA_EA: 0x8AD03D0
// IDA_NAME: _ZN14CompiledDNFLex6createEb
// DEMANGLED: CompiledDNFLex::create(bool)
void CompiledDNFLex::create(bool create_string_table) {
    if (create_string_table) {
        if (stringManager_ == 0) {
            stringManager_ = new ScriptStringManager();
        }
        stringManagerDelete_ = true;
    }
    if (stringBuffer_ == 0) {
        stringBuffer_ = new char[0x400];
        std::memset(stringBuffer_, 0, 0x400);
    }
}

// IDA_EA: 0x8AD0E90
// IDA_NAME: _ZN14CompiledDNFLex16CloseInputStreamEv
// DEMANGLED: CompiledDNFLex::CloseInputStream(void)
void CompiledDNFLex::CloseInputStream() {
    while (PopInputStream(true)) {}
    offset_ = 0;
}

// IDA_EA: 0x8AD0A00
// IDA_NAME: _ZN14CompiledDNFLex15PushInputStreamEPKcS1_Pcj
// DEMANGLED: CompiledDNFLex::PushInputStream(char const*,char const*,char *,unsigned int)
bool CompiledDNFLex::PushInputStream(const TCHAR* dir,
                                     const TCHAR* filename,
                                     char* buffer,
                                     unsigned int buffer_size) {
    if (buffer == 0 || buffer_size == 0) {
        return false;
    }

    stream_t stream_info;
    stream_info.dir.assign(dir != 0 ? dir : "");
    stream_info.filename.assign(filename != 0 ? filename : "");
    stream_info.size = buffer_size;
    stream_info.buffer = buffer;
    stream_info.offset = buffer;

    // IDA 0x8AD0A00 saves the active parent's cursor in its stream_t before
    // pushing an included compiled stream.  PopInputStream restores this
    // field, so omitting the save restarts the parent at its first token after
    // the child reaches EOF instead of resuming after DNF_TOK_INCLUDE.
    if (!stream_stack_.empty()) {
        stream_stack_.top().offset = offset_;
    }
    stream_stack_.push(stream_info);
    offset_ = buffer;
    if (buffer_size > 1 &&
        *reinterpret_cast<const uint16_t*>(buffer) == 0xD0B0u) {
        offset_ = buffer + 2;
    }
    return true;
}

// IDA_EA: 0x8AD0C80
// IDA_NAME: _ZN14CompiledDNFLex14PopInputStreamEb
// DEMANGLED: CompiledDNFLex::PopInputStream(bool)
bool CompiledDNFLex::PopInputStream(bool pop_all) {
    if (stream_stack_.empty()) {
        return false;
    }
    if (!pop_all && stream_stack_.size() <= 1) {
        return false;
    }

    delete[] stream_stack_.top().buffer;
    stream_stack_.pop();
    if (!stream_stack_.empty()) {
        offset_ = stream_stack_.top().offset;
    } else {
        offset_ = 0;
    }
    return true;
}

// IDA_EA: 0x8AD0F40
// IDA_NAME: _ZN14CompiledDNFLex17SwitchInputStreamEPKcS1_Pcj
// DEMANGLED: CompiledDNFLex::SwitchInputStream(char const*,char const*,char *,unsigned int)
bool CompiledDNFLex::SwitchInputStream(const TCHAR* dir,
                                       const TCHAR* filename,
                                       char* buffer,
                                       unsigned int buffer_size) {
    CloseInputStream();
    return PushInputStream(dir, filename, buffer, buffer_size);
}

// IDA_EA: 0x8AD0FA0
// IDA_NAME: _ZN14CompiledDNFLex8getTokenEv
// DEMANGLED: CompiledDNFLex::getToken(void)
DNFLEX_TOKEN CompiledDNFLex::getToken() {
    const stream_t* stream = GetInputStream();
    if (stream == 0 || offset_ == 0 || offset_ >= stream->buffer + stream->size) {
        lastToken_ = DNF_TOK_EOF;
        if (!PopInputStream(false)) {
            return lastToken_;
        }
        return getToken();
    }

    DNFLEX_TOKEN token = ReadCharTokenFromBuffer(offset_);
    lastToken_ = token;
    if (allow_inclusion_ && token == DNF_TOK_INCLUDE) {
        token = ReadCharTokenFromBuffer(offset_);
        lastToken_ = token;
        if (token != DNF_TOK_STRING) {
            std::snprintf(ERR_BUF_, sizeof(ERR_BUF_),
                          "%s - %s, invalid inclusion.",
                          "getToken", getStreamName().c_str());
            LexerError(ERR_BUF_);
            return DNF_TOK_ERROR;
        }

        std::string include_path;
        if (getStr(include_path)) {
            DNFLex::stream_data_t stream_data;
            stream_data.clear();
            if (LoadStream("", include_path.c_str(), &stream_data)) {
                char* buffer = stream_data.buffer;
                if (buffer == 0 && stream_data.stream != 0) {
                    buffer = new char[stream_data.size];
                    stream_data.stream->read(buffer, stream_data.size);
                } else {
                    stream_data.buffer = 0;
                }
                const unsigned int buffer_size = stream_data.size;
                stream_data.destroy();
                if (PushInputStream("", include_path.c_str(), buffer,
                                    buffer_size)) {
                    return getToken();
                }
                delete[] buffer;
                std::snprintf(ERR_BUF_, sizeof(ERR_BUF_),
                              "%s - %s, PushInputStream failed.",
                              "getToken", include_path.c_str());
            } else {
                std::snprintf(ERR_BUF_, sizeof(ERR_BUF_),
                              "%s - %s, LoadStream failed.",
                              "getToken", include_path.c_str());
            }
        } else {
            std::snprintf(ERR_BUF_, sizeof(ERR_BUF_),
                          "%s - %s, invalid inclusion.",
                          "getToken", getStreamName().c_str());
        }
        LexerError(ERR_BUF_);
        return DNF_TOK_ERROR;
    }
    return token;
}

// IDA_EA: 0x8ACE290
// IDA_NAME: _ZN13DNFLexWrapper10setLexTypeENS_13ENUM_LEX_TYPEE
// DEMANGLED: DNFLexWrapper::setLexType(DNFLexWrapper::ENUM_LEX_TYPE)
void DNFLexWrapper::setLexType(ENUM_LEX_TYPE lex_type) {
    lexType_ = lex_type;
}

// IDA_EA: 0x8ACE2A0
// IDA_NAME: _ZN13DNFLexWrapper10getLexTypeEv
// DEMANGLED: DNFLexWrapper::getLexType(void)
DNFLexWrapper::ENUM_LEX_TYPE DNFLexWrapper::getLexType() {
    return lexType_;
}

// IDA_EA: 0x8ACE2B0
// IDA_NAME: _ZN13DNFLexWrapper10LoadStreamEPKcS1_PN6DNFLex13stream_data_tE
// DEMANGLED: DNFLexWrapper::LoadStream(char const*,char const*,DNFLex::stream_data_t *)
bool DNFLexWrapper::LoadStream(const TCHAR* dir,
                               const TCHAR* filename,
                               DNFLex::stream_data_t* stream_data) {
    if (LoadStream_ != 0) {
        return LoadStream_(dir, filename, stream_data);
    }
    return false;
}

// IDA_EA: 0x8ACE2F0
// IDA_NAME: _ZN13DNFLexWrapper10getDecimalEv
// DEMANGLED: DNFLexWrapper::getDecimal(void)
int DNFLexWrapper::getDecimal() {
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastIntValue_;
    }
    if (lexType_ != LEX_TYPE_NORMAL || dnfLex_.yytext_ == 0) {
        return 0;
    }

    char* yytext = dnfLex_.yytext_;
    char n32 = *yytext;
    if (n32 == 0) {
        return 0;
    }
    while (n32 == ' ') {
        n32 = *++yytext;
        if (n32 == 0) {
            return 0;
        }
    }

    bool negative = false;
    if (n32 == '+') {
        n32 = *++yytext;
    } else if (n32 == '-') {
        negative = true;
        n32 = *++yytext;
    } else if (static_cast<unsigned int>(
                   static_cast<unsigned char>(n32) - '0') > 9u) {
        return 0;
    }

    int result = 0;
    for (unsigned int digit =
             static_cast<unsigned char>(n32) - static_cast<unsigned char>('0');
         digit <= 9u;
         digit = static_cast<unsigned char>(*yytext) -
                 static_cast<unsigned char>('0')) {
        ++yytext;
        result = static_cast<int>(digit) + 10 * result;
    }
    return negative ? -result : result;
}

// IDA_EA: 0x8ACE390
// IDA_NAME: _ZN13DNFLexWrapper7getTypeEv
// DEMANGLED: DNFLexWrapper::getType(void)
const char* DNFLexWrapper::getType() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.yytext_;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    return 0;
}

// IDA_EA: 0x8ACE3C0
// IDA_NAME: _ZN13DNFLexWrapper13getCustomDataEv
// DEMANGLED: DNFLexWrapper::getCustomData(void)
const char* DNFLexWrapper::getCustomData() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.yytext_;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    return 0;
}

// IDA_EA: 0x8ACE3F0
// IDA_NAME: _ZN13DNFLexWrapper12getConnectorEv
// DEMANGLED: DNFLexWrapper::getConnector(void)
const char* DNFLexWrapper::getConnector() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.yytext_;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    return 0;
}

// IDA_EA: 0x8ACE420
// IDA_NAME: _ZN13DNFLexWrapper12getLastTokenEv
// DEMANGLED: DNFLexWrapper::getLastToken(void)
DNFLEX_TOKEN DNFLexWrapper::getLastToken() {
    return lastToken_;
}

// IDA_EA: 0x8ACE430
// IDA_NAME: _ZN13DNFLexWrapper16getLastTokenDataEv
// DEMANGLED: DNFLexWrapper::getLastTokenData(void)
const char* DNFLexWrapper::getLastTokenData() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.yytext_;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    return 0;
}

// IDA_EA: 0x8ACE490
// IDA_NAME: _ZN13DNFLexWrapper13getLastLineNoEv
// DEMANGLED: DNFLexWrapper::getLastLineNo(void)
int DNFLexWrapper::getLastLineNo() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.yylineno_;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return compiledDnfLex_.GetOffset();
    }
    return -1;
}

// IDA_EA: 0x8ACE4D0
// IDA_NAME: _ZN13DNFLexWrapper24getLastTokenDataAsStringEv
// DEMANGLED: DNFLexWrapper::getLastTokenDataAsString(void)
std::string DNFLexWrapper::getLastTokenDataAsString() {
    const char* data = getLastTokenData();
    return data != 0 ? std::string(data) : std::string();
}

// IDA_EA: 0x8ACE530
// IDA_NAME: _ZN13DNFLexWrapper22setStringTableFromFileEPKc
// DEMANGLED: DNFLexWrapper::setStringTableFromFile(char const*)
bool DNFLexWrapper::setStringTableFromFile(const TCHAR* file_name) {
    ScriptStringManager* string_manager =
        compiledDnfLex_.getStringManager();
    return string_manager->importFromFile(file_name);
}

// IDA_EA: 0x8ACE560
// IDA_NAME: _ZN13DNFLexWrapper24setStringTableFromBufferEPKhi
// DEMANGLED: DNFLexWrapper::setStringTableFromBuffer(unsigned char const*,int)
bool DNFLexWrapper::setStringTableFromBuffer(const unsigned char* buffer,
                                             int size) {
    ScriptStringManager* string_manager =
        compiledDnfLex_.getStringManager();
    return string_manager->importFromBuffer(buffer, size);
}

// IDA_EA: 0x8ACE5A0
// IDA_NAME: _ZN13DNFLexWrapper10getPairStrEv
// DEMANGLED: DNFLexWrapper::getPairStr(void)
const char* DNFLexWrapper::getPairStr() {
    static char buffer[0x101];
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    if (lexType_ != LEX_TYPE_NORMAL || dnfLex_.yytext_ == 0) {
        return 0;
    }
    std::strncpy(buffer, dnfLex_.yytext_ + 1, 0x100u);
    buffer[0x100] = 0;
    const size_t length = std::strlen(buffer);
    if (length != 0) {
        buffer[length - 1] = 0;
    }
    return buffer;
}

// IDA_EA: 0x8ACE630
// IDA_NAME: _ZN13DNFLexWrapper10getPairIntEv
// DEMANGLED: DNFLexWrapper::getPairInt(void)
int DNFLexWrapper::getPairInt() {
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastIntValue_;
    }
    if (lexType_ != LEX_TYPE_NORMAL || dnfLex_.yytext_ == 0) {
        return 0;
    }

    char buffer[264];
    std::strncpy(buffer, dnfLex_.yytext_ + 1, 0x100u);
    buffer[0x100] = 0;
    const size_t length = std::strlen(buffer);
    if (length != 0) {
        buffer[length - 1] = 0;
    }
    return static_cast<int>(std::strtol(buffer, 0, 10));
}

// IDA_EA: 0x8ACE6F0
// IDA_NAME: _ZN13DNFLexWrapper9getStringEv
// DEMANGLED: DNFLexWrapper::getString(void)
const char* DNFLexWrapper::getString() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.tok_str();
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastStringValue_;
    }
    return 0;
}

// IDA_EA: 0x8ACE730
// IDA_NAME: _ZN13DNFLexWrapper8getFloatEv
// DEMANGLED: DNFLexWrapper::getFloat(void)
float DNFLexWrapper::getFloat() {
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastFloatValue_;
    }
    if (lexType_ == LEX_TYPE_NORMAL && dnfLex_.yytext_ != 0) {
        return static_cast<float>(std::strtod(dnfLex_.yytext_, 0));
    }
    return 0.0f;
}

// IDA_EA: 0x8ACE780
// IDA_NAME: _ZN13DNFLexWrapper13getHexDecimalEv
// DEMANGLED: DNFLexWrapper::getHexDecimal(void)
int DNFLexWrapper::getHexDecimal() {
    if (lexType_ == LEX_TYPE_COMPILED) {
        return lastIntValue_;
    }
    if (lexType_ != LEX_TYPE_NORMAL || dnfLex_.yytext_ == 0) {
        return 0;
    }
    int value = 0;
    std::sscanf(dnfLex_.yytext_ + 2, "%x", &value);
    return value;
}

// IDA_EA: 0x8ACE7E0
// IDA_NAME: _ZN13DNFLexWrapper8getTokenEv
// DEMANGLED: DNFLexWrapper::getToken(void)
DNFLEX_TOKEN DNFLexWrapper::getToken() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        DNFLEX_TOKEN token = dnfLex_.get_next_token();
        lastToken_ = token;
        return token;
    }
    if (lexType_ != LEX_TYPE_COMPILED) {
        return lastToken_;
    }

    DNFLEX_TOKEN token = compiledDnfLex_.getToken();
    lastToken_ = token;
    if (token > DNF_TOK_ERROR) {
        if ((token >= DNF_TOK_DECIMAL && token <= DNF_TOK_HEXADECIMAL) ||
            token == DNF_TOK_PAIR_INT) {
            lastIntValue_ = compiledDnfLex_.getInt(0);
            return lastToken_;
        }
        if (token == DNF_TOK_FLOAT) {
            lastFloatValue_ = compiledDnfLex_.getFloat(0);
        } else {
            lastStringValue_ = compiledDnfLex_.getStr(static_cast<bool*>(0));
        }
    }
    return lastToken_;
}

// IDA_EA: 0x8ACE8A0
// IDA_NAME: _ZN13DNFLexWrapper13getStreamNameEv
// DEMANGLED: DNFLexWrapper::getStreamName(void)
std::string DNFLexWrapper::getStreamName() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        return dnfLex_.getStreamName();
    }
    return "NONE";
}

// IDA_EA: 0x8ACE900
// IDA_NAME: _ZN13DNFLexWrapper14getInputStreamEv
// DEMANGLED: DNFLexWrapper::getInputStream(void)
std::istream* DNFLexWrapper::getInputStream() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        DNFLex::stream_t* stream = dnfLex_.GetInputStream();
        return stream != 0 ? stream->s.stream : 0;
    }
    return 0;
}

// IDA_EA: 0x8ACE930
// IDA_NAME: _ZN13DNFLexWrapper18getInputStreamSizeEv
// DEMANGLED: DNFLexWrapper::getInputStreamSize(void)
int DNFLexWrapper::getInputStreamSize() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        DNFLex::stream_t* stream = dnfLex_.GetInputStream();
        return stream != 0 ? static_cast<int>(stream->s.size) : -1;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        CompiledDNFLex::stream_t* stream = compiledDnfLex_.GetInputStream();
        return stream != 0 ? static_cast<int>(stream->size) : -1;
    }
    return -1;
}

// IDA_EA: 0x8ACE970
// IDA_NAME: _ZN13DNFLexWrapper20getInputStreamBufferEv
// DEMANGLED: DNFLexWrapper::getInputStreamBuffer(void)
char* DNFLexWrapper::getInputStreamBuffer() {
    if (lexType_ == LEX_TYPE_NORMAL) {
        DNFLex::stream_t* stream = dnfLex_.GetInputStream();
        return stream != 0 ? stream->s.buffer : 0;
    }
    if (lexType_ == LEX_TYPE_COMPILED) {
        CompiledDNFLex::stream_t* stream = compiledDnfLex_.GetInputStream();
        return stream != 0 ? stream->buffer : 0;
    }
    return 0;
}

// IDA_EA: 0x8ACE9B0
// IDA_NAME: _ZN13DNFLexWrapper19setStringStreamDataEPN6DNFLex13stream_data_tEPcj
// DEMANGLED: DNFLexWrapper::setStringStreamData(DNFLex::stream_data_t *,char *,unsigned int)
bool DNFLexWrapper::setStringStreamData(DNFLex::stream_data_t* stream_data,
                                        char* buffer,
                                        uint32_t buffer_size) {
    static char kEmptyBuf[] = " ";

    if (stream_data == 0) {
        return false;
    }

    stream_data->clear();
    if (buffer_size != 0) {
        stream_data->stream = new std::istrstream(buffer, buffer_size);
        stream_data->size = buffer_size;
        stream_data->buffer = buffer;
    } else {
        delete[] buffer;
        stream_data->stream = new std::istrstream(kEmptyBuf, 1);
        stream_data->size = 1;
        stream_data->buffer = 0;
    }
    return true;
}

// IDA_EA: 0x8ACEA70
// IDA_NAME: _ZN13DNFLexWrapper16closeInputStreamEv
// DEMANGLED: DNFLexWrapper::closeInputStream(void)
void DNFLexWrapper::closeInputStream() {
    dnfLex_.CloseInputStream();
    compiledDnfLex_.CloseInputStream();
}

// IDA_EA: 0x8ACEAA0
// IDA_NAME: _ZN13DNFLexWrapper17switchInputStreamEPKcS1_PN6DNFLex13stream_data_tE
// DEMANGLED: DNFLexWrapper::switchInputStream(char const*,char const*,DNFLex::stream_data_t *)
bool DNFLexWrapper::switchInputStream(const TCHAR* dir,
                                      const TCHAR* filename,
                                      DNFLex::stream_data_t* stream_data) {
    DNFLex::stream_data_t created;
    DNFLex::stream_data_t* active_stream = stream_data;

    if (active_stream == 0) {
        created.clear();
        active_stream = &created;
        if (!LoadStream(dir, filename, active_stream)) {
            if (LexerError_ != 0) {
                char message[1024];
                std::snprintf(message, sizeof(message), "%s/%s Read failed!",
                              dir != 0 ? dir : "", filename != 0 ? filename : "");
                LexerError_(message);
            }
            return false;
        }
    }

    if (active_stream->stream == 0) {
        active_stream->destroy();
        return false;
    }

    uint16_t compiled_script_magic = 0;
    if (active_stream->size > 1) {
        active_stream->stream->read(
            reinterpret_cast<char*>(&compiled_script_magic), 2);
        active_stream->stream->clear();
        active_stream->stream->seekg(0, std::ios::beg);
    }

    if (active_stream->size > 1 && compiled_script_magic == 0xD0B0u) {
        lexType_ = LEX_TYPE_COMPILED;
        const unsigned int buffer_size = active_stream->size;
        if (active_stream->buffer != 0) {
            char* buffer = active_stream->buffer;
            active_stream->buffer = 0;
            active_stream->destroy();
            return compiledDnfLex_.SwitchInputStream(
                dir, filename, buffer, buffer_size);
        }

        char* buffer = new char[buffer_size];
        active_stream->stream->read(buffer, buffer_size);
        active_stream->destroy();
        return compiledDnfLex_.SwitchInputStream(
            dir, filename, buffer, buffer_size);
    }

    lexType_ = LEX_TYPE_NORMAL;
    return dnfLex_.SwitchInputStream(dir, filename, active_stream);
}

// IDA_EA: 0x8ACF060
// IDA_NAME: _ZN13DNFLexWrapper24switchChannelInputStreamEPKcPN6DNFLex13stream_data_tE
// DEMANGLED: DNFLexWrapper::switchChannelInputStream(char const*,DNFLex::stream_data_t *)
bool DNFLexWrapper::switchChannelInputStream(
    const TCHAR* filename,
    DNFLex::stream_data_t* stream_data) {
    DNFLex::stream_data_t created;
    DNFLex::stream_data_t* active_stream = stream_data;

    if (active_stream == 0) {
        created.clear();
        active_stream = &created;
        if (!setFileStreamData(active_stream, filename)) {
            return false;
        }
    }

    lexType_ = LEX_TYPE_NORMAL;
    return dnfLex_.SwitchInputStream("", filename, active_stream);
}

// IDA_EA: 0x8ACEC80
// IDA_NAME: _ZN13DNFLexWrapper10SetHandlerEPFbcEPFvPKciEPFvS3_EPFbS3_S3_PN6DNFLex13stream_data_tEE
// DEMANGLED: DNFLexWrapper::SetHandler(bool (*)(char),void (*)(char const*,int),void (*)(char const*),bool (*)(char const*,char const*,DNFLex::stream_data_t *))
void DNFLexWrapper::SetHandler(DNFLex::TIsDbcsLeadChar is_dbcs_lead_char,
                               DNFLex::TLexerOutput lexer_output,
                               TLexerError lexer_error,
                               TLoadStream load_stream) {
    dnfLex_.SetHandler(is_dbcs_lead_char, lexer_output, lexer_error,
                       load_stream);
    compiledDnfLex_.SetHandler(lexer_error, load_stream);
    LexerError_ = lexer_error;
    LoadStream_ = load_stream;
}

// IDA_EA: 0x8ACECF0
// IDA_NAME: _ZN13DNFLexWrapperD2Ev
// DEMANGLED: DNFLexWrapper::~DNFLexWrapper()
DNFLexWrapper::~DNFLexWrapper() {}

// IDA_EA: 0x8ACED60
// IDA_NAME: _ZN13DNFLexWrapperC2Ev
// DEMANGLED: DNFLexWrapper::DNFLexWrapper(void)
DNFLexWrapper::DNFLexWrapper()
    : vftable_(0),
      lastToken_(DNF_TOK_EOF),
      lastIntValue_(0),
      lastFloatValue_(0.0f),
      lastStringValue_(0),
      lexType_(LEX_TYPE_NORMAL),
      dnfLex_(),
      compiledDnfLex_(),
      LexerError_(0),
      LoadStream_(0) {}

// IDA_EA: 0x8ACEDE0
// IDA_NAME: _ZN13DNFLexWrapper17setFileStreamDataEPN6DNFLex13stream_data_tEPKc
// DEMANGLED: DNFLexWrapper::setFileStreamData(DNFLex::stream_data_t *,char const*)
bool DNFLexWrapper::setFileStreamData(DNFLex::stream_data_t* stream_data,
                                      const TCHAR* path) {
    if (path == 0 || stream_data == 0) {
        return false;
    }

    stream_data->clear();
    std::ifstream* input =
        new std::ifstream(path, std::ios::in | std::ios::binary);
    if (!input->is_open() || input->fail()) {
        delete input;
        return false;
    }

    input->seekg(0, std::ios::end);
    const std::streamoff size = input->tellg();
    input->clear();
    input->seekg(0, std::ios::beg);
    if (size > 0) {
        stream_data->stream = input;
        stream_data->size = static_cast<unsigned int>(size);
        stream_data->buffer = 0;
        return true;
    }

    delete input;
    return setStringStreamData(stream_data, 0, 0);
}

// IDA_EA: 0x88BBD29
// IDA_NAME: _Z18loadRDARScriptFilePKcS0_
// DEMANGLED: loadRDARScriptFile(char const*,char const*)
bool loadRDARScriptFile(const char* dir, const char* filename) {
    if (filename == 0) {
        return false;
    }
    if (g_onLoadScriptFileFunction != 0) {
        g_onLoadScriptFileFunction();
    }
    return g_dnfLex.switchInputStream(dir, filename, 0);
}

// IDA_EA: 0x88BC054
// IDA_NAME: _Z20unloadRDARScriptFilev
// DEMANGLED: unloadRDARScriptFile(void)
void unloadRDARScriptFile() {
    g_dnfLex.closeInputStream();
}
