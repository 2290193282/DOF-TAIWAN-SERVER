#ifndef OLDSRV_OLD_DATA_PVF_COMMON_DNF_FLEX_LEXER_H_
#define OLDSRV_OLD_DATA_PVF_COMMON_DNF_FLEX_LEXER_H_

#include <cstddef>
#include <istream>
#include <ostream>

struct yy_buffer_state {
    std::istream* yy_input_file;
    char* yy_ch_buf;
    char* yy_buf_pos;
    std::size_t yy_buf_size;
    int yy_n_chars;
    int yy_is_our_buffer;
    int yy_is_interactive;
    int yy_at_bol;
    int yy_bs_lineno;
    int yy_bs_column;
    int yy_fill_buffer;
    int yy_buffer_status;
};

class __dnf_script__FlexLexer {
public:
    __dnf_script__FlexLexer(std::istream* input, std::ostream* output);
    virtual ~__dnf_script__FlexLexer();

    // IDA vtable order: switch/create/delete/restart/yylex/switch_streams/
    // yywrap/LexerInput/LexerOutput/LexerError.
    virtual void yy_switch_to_buffer(yy_buffer_state* new_buffer);
    virtual yy_buffer_state* yy_create_buffer(std::istream* input, int size);
    virtual void yy_delete_buffer(yy_buffer_state* buffer);
    virtual void yyrestart(std::istream* input);
    virtual int yylex();
    virtual void switch_streams(std::istream* new_input,
                                std::ostream* new_output);
    virtual int yywrap();
    virtual int LexerInput(char* buffer, int max_size);
    virtual void LexerOutput(const char* buffer, int size);
    virtual void LexerError(const char* message);

    int yy_get_previous_state();
    int yy_try_NUL_trans(int current_state);
    void yyunput(int value, char* token_begin);
    void yy_load_buffer_state();
    void yy_flush_buffer(yy_buffer_state* buffer);
    void yypop_buffer_state();
    void yy_pop_state();
    int yy_top_state();
    int yy_get_next_buffer();
    int yyinput();
    void yy_push_state(int new_state);
    void yyensure_buffer_stack();
    void yypush_buffer_state(yy_buffer_state* new_buffer);
    void yy_init_buffer(yy_buffer_state* buffer, std::istream* input);

protected:
    char* yytext_;
    int yyleng_;
    int yylineno_;
    int yy_flex_debug_;
    int yy_start_stack_ptr_;
    int yy_start_stack_depth_;
    int* yy_start_stack_;
    std::istream* yyin_;
    std::ostream* yyout_;
    char yy_hold_char_;
    int yy_n_chars_;
    char* yy_c_buf_p_;
    int yy_init_;
    int yy_start_;
    int yy_did_buffer_switch_on_eof_;
    std::size_t yy_buffer_stack_top_;
    std::size_t yy_buffer_stack_max_;
    yy_buffer_state** yy_buffer_stack_;
    int yy_last_accepting_state_;
    char* yy_last_accepting_cpos_;
    int* yy_state_buf_;
    int* yy_state_ptr_;
    char* yy_full_match_;
    int* yy_full_state_;
    int yy_full_lp_;
    int yy_lp_;
    int yy_looking_for_trail_begin_;
    int yy_more_flag_;
    int yy_more_len_;
    int yy_more_offset_;
    int yy_prev_more_offset_;
};

static_assert(sizeof(yy_buffer_state) == 0x30u,
              "yy_buffer_state IDA ABI size must stay 0x30");
static_assert(sizeof(__dnf_script__FlexLexer) == 0x80u,
              "__dnf_script__FlexLexer IDA ABI size must stay 0x80");

#endif
