#include "old_data/PVF/common/dnf_flex_lexer.h"
#include "old_data/PVF/common/dnf_lex_dfa_tables.inc"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>

namespace {

enum EndOfBufferAction {
    END_OF_BUFFER_CONTINUE = 0,
    END_OF_BUFFER_EOF = 1,
    END_OF_BUFFER_LAST_MATCH = 2
};

const int kDefaultBufferSize = 0x4000;
const int kReadBufferSize = 0x2000;

}  // namespace

// IDA_EA: 0x8AD1910
// IDA_NAME: _ZN23__dnf_script__FlexLexer6yywrapEv
// DEMANGLED: __dnf_script__FlexLexer::yywrap(void)
int __dnf_script__FlexLexer::yywrap() {
    return 1;
}

// IDA_EA: 0x8AD1920
// IDA_NAME: _ZN23__dnf_script__FlexLexer5yylexEv
// DEMANGLED: __dnf_script__FlexLexer::yylex(void)
int __dnf_script__FlexLexer::yylex() {
    LexerError("yyFlexLexer::yylex invoked but %option yyclass used");
    return 0;
}

// IDA_EA: 0x8AD1A80
// IDA_NAME: _ZN23__dnf_script__FlexLexer21yy_get_previous_stateEv
// DEMANGLED: __dnf_script__FlexLexer::yy_get_previous_state(void)
int __dnf_script__FlexLexer::yy_get_previous_state() {
    using namespace oldsrv_dnf_lex_dfa;

    int yy_current_state = yy_start_;
    for (char* cursor = yytext_; yy_c_buf_p_ > cursor; ++cursor) {
        if (yy_accept[yy_current_state] != 0) {
            yy_last_accepting_state_ = yy_current_state;
            yy_last_accepting_cpos_ = cursor;
        }

        const unsigned char input = static_cast<unsigned char>(*cursor);
        if (input != 0) {
            int transition = yy_base[yy_current_state];
            while (yy_chk[transition + input] != yy_current_state) {
                yy_current_state = yy_def[yy_current_state];
                transition = yy_base[yy_current_state];
            }
            yy_current_state = yy_nxt[transition + input];
        } else {
            yy_current_state = yy_NUL_trans[yy_current_state];
        }
    }
    return yy_current_state;
}

// IDA_EA: 0x8AD1B50
// IDA_NAME: _ZN23__dnf_script__FlexLexer16yy_try_NUL_transEi
// DEMANGLED: __dnf_script__FlexLexer::yy_try_NUL_trans(int)
int __dnf_script__FlexLexer::yy_try_NUL_trans(int yy_current_state) {
    return oldsrv_dnf_lex_dfa::yy_NUL_trans[yy_current_state];
}

// IDA_EA: 0x8AD1940
// IDA_NAME: _ZN23__dnf_script__FlexLexerC2EPSiPSo
// DEMANGLED: __dnf_script__FlexLexer::__dnf_script__FlexLexer(std::istream *,std::ostream *)
__dnf_script__FlexLexer::__dnf_script__FlexLexer(std::istream* input,
                                                 std::ostream* output)
    : yyin_(input),
      yyout_(output),
      yy_c_buf_p_(0),
      yy_init_(0),
      yy_start_(0),
      yy_flex_debug_(0),
      yylineno_(1),
      yy_did_buffer_switch_on_eof_(0),
      yy_looking_for_trail_begin_(0),
      yy_more_flag_(0),
      yy_more_len_(0),
      yy_more_offset_(0),
      yy_prev_more_offset_(0),
      yy_start_stack_ptr_(0),
      yy_start_stack_depth_(0),
      yy_start_stack_(0),
      yy_buffer_stack_top_(0),
      yy_buffer_stack_max_(0),
      yy_buffer_stack_(0),
      yy_last_accepting_cpos_(0),
      yy_state_buf_(0),
      yy_state_ptr_(0),
      yy_full_match_(0),
      yy_full_state_(0) {}

// IDA_EA: 0x8AD1A00
// IDA_NAME: _ZN23__dnf_script__FlexLexer14switch_streamsEPSiPSo
// DEMANGLED: __dnf_script__FlexLexer::switch_streams(std::istream *,std::ostream *)
void __dnf_script__FlexLexer::switch_streams(std::istream* new_input,
                                             std::ostream* new_output) {
    if (new_input) {
        yy_buffer_state* buffer = 0;
        if (yy_buffer_stack_) {
            buffer = yy_buffer_stack_[yy_buffer_stack_top_];
        }
        yy_delete_buffer(buffer);
        yy_switch_to_buffer(yy_create_buffer(new_input, kDefaultBufferSize));
    }
    if (new_output) {
        yyout_ = new_output;
    }
}

// IDA_EA: 0x8AD1B60
// IDA_NAME: _ZN23__dnf_script__FlexLexer7yyunputEiPc
// DEMANGLED: __dnf_script__FlexLexer::yyunput(int,char *)
void __dnf_script__FlexLexer::yyunput(int value, char* token_begin) {
    char* cursor = yy_c_buf_p_;
    *cursor = yy_hold_char_;

    yy_buffer_state* buffer = yy_buffer_stack_[yy_buffer_stack_top_];
    if (cursor < buffer->yy_ch_buf + 2) {
        char* source = buffer->yy_ch_buf + yy_n_chars_ + 2;
        char* destination = buffer->yy_ch_buf + buffer->yy_buf_size + 2;
        while (source > buffer->yy_ch_buf) {
            *--destination = *--source;
        }
        const std::ptrdiff_t shift = destination - source;
        cursor += shift;
        token_begin += shift;
        buffer = yy_buffer_stack_[yy_buffer_stack_top_];
        yy_n_chars_ = static_cast<int>(buffer->yy_buf_size);
        buffer->yy_n_chars = yy_n_chars_;
        if (cursor < buffer->yy_ch_buf + 2) {
            LexerError("flex scanner push-back overflow");
        }
    }

    *--cursor = static_cast<char>(value);
    if (value == '\n') {
        --yylineno_;
    }
    yytext_ = token_begin;
    yy_c_buf_p_ = cursor;
    yy_hold_char_ = *cursor;
}

// IDA_EA: 0x8AD1C40
// IDA_NAME: _ZN23__dnf_script__FlexLexer20yy_load_buffer_stateEv
// DEMANGLED: __dnf_script__FlexLexer::yy_load_buffer_state(void)
void __dnf_script__FlexLexer::yy_load_buffer_state() {
    yy_buffer_state* buffer = yy_buffer_stack_[yy_buffer_stack_top_];
    yy_n_chars_ = buffer->yy_n_chars;
    yy_c_buf_p_ = buffer->yy_buf_pos;
    yytext_ = buffer->yy_buf_pos;
    yyin_ = buffer->yy_input_file;
    yy_hold_char_ = *buffer->yy_buf_pos;
}

// IDA_EA: 0x8AD1C70
// IDA_NAME: _ZN23__dnf_script__FlexLexer15yy_flush_bufferEP15yy_buffer_state
// DEMANGLED: __dnf_script__FlexLexer::yy_flush_buffer(yy_buffer_state *)
void __dnf_script__FlexLexer::yy_flush_buffer(yy_buffer_state* buffer) {
    if (!buffer) {
        return;
    }

    buffer->yy_n_chars = 0;
    buffer->yy_ch_buf[0] = 0;
    buffer->yy_ch_buf[1] = 0;
    buffer->yy_at_bol = 1;
    buffer->yy_buffer_status = 0;
    buffer->yy_buf_pos = buffer->yy_ch_buf;
    if (yy_buffer_stack_ &&
        buffer == yy_buffer_stack_[yy_buffer_stack_top_]) {
        yy_load_buffer_state();
    }
}

// IDA_EA: 0x8AD1D00
// IDA_NAME: _ZN23__dnf_script__FlexLexer18yypop_buffer_stateEv
// DEMANGLED: __dnf_script__FlexLexer::yypop_buffer_state(void)
void __dnf_script__FlexLexer::yypop_buffer_state() {
    if (!yy_buffer_stack_ ||
        !yy_buffer_stack_[yy_buffer_stack_top_]) {
        return;
    }

    yy_delete_buffer(yy_buffer_stack_[yy_buffer_stack_top_]);
    yy_buffer_stack_[yy_buffer_stack_top_] = 0;
    if (yy_buffer_stack_top_ != 0) {
        --yy_buffer_stack_top_;
    }
    if (yy_buffer_stack_[yy_buffer_stack_top_]) {
        yy_load_buffer_state();
        yy_did_buffer_switch_on_eof_ = 1;
    }
}

// IDA_EA: 0x8AD1D80
// IDA_NAME: _ZN23__dnf_script__FlexLexer12yy_pop_stateEv
// DEMANGLED: __dnf_script__FlexLexer::yy_pop_state(void)
void __dnf_script__FlexLexer::yy_pop_state() {
    --yy_start_stack_ptr_;
    if (yy_start_stack_ptr_ < 0) {
        LexerError("start-condition stack underflow");
    }
    yy_start_ = 2 * yy_start_stack_[yy_start_stack_ptr_] + 1;
}

// IDA_EA: 0x8AD1DD0
// IDA_NAME: _ZN23__dnf_script__FlexLexer12yy_top_stateEv
// DEMANGLED: __dnf_script__FlexLexer::yy_top_state(void)
int __dnf_script__FlexLexer::yy_top_state() {
    return yy_start_stack_[yy_start_stack_ptr_ - 1];
}

// IDA_EA: 0x8AD1E30
// IDA_NAME: _ZN23__dnf_script__FlexLexer16yy_delete_bufferEP15yy_buffer_state
// DEMANGLED: __dnf_script__FlexLexer::yy_delete_buffer(yy_buffer_state *)
void __dnf_script__FlexLexer::yy_delete_buffer(yy_buffer_state* buffer) {
    if (!buffer) {
        return;
    }
    if (yy_buffer_stack_ &&
        buffer == yy_buffer_stack_[yy_buffer_stack_top_]) {
        yy_buffer_stack_[yy_buffer_stack_top_] = 0;
    }
    if (buffer->yy_is_our_buffer) {
        std::free(buffer->yy_ch_buf);
    }
    std::free(buffer);
}

// IDA_EA: 0x8AD1EA0
// IDA_NAME: _ZN23__dnf_script__FlexLexer18yy_get_next_bufferEv
// DEMANGLED: __dnf_script__FlexLexer::yy_get_next_buffer(void)
int __dnf_script__FlexLexer::yy_get_next_buffer() {
    yy_buffer_state* buffer = yy_buffer_stack_[yy_buffer_stack_top_];
    char* destination = buffer->yy_ch_buf;
    char* source = yytext_;
    if (yy_c_buf_p_ > buffer->yy_ch_buf + yy_n_chars_ + 1) {
        LexerError(
            "fatal flex scanner internal error--end of buffer missed");
    }
    if (!buffer->yy_fill_buffer) {
        return (yy_c_buf_p_ - source == 1) ? END_OF_BUFFER_EOF
                                           : END_OF_BUFFER_LAST_MATCH;
    }

    const int number_to_move = static_cast<int>(yy_c_buf_p_ - source) - 1;
    for (int index = 0; index < number_to_move; ++index) {
        destination[index] = source[index];
    }

    if (buffer->yy_buffer_status == 2) {
        yy_n_chars_ = 0;
        buffer->yy_n_chars = 0;
    } else {
        int available = static_cast<int>(buffer->yy_buf_size) -
                        number_to_move - 1;
        while (available <= 0) {
            if (!buffer->yy_is_our_buffer) {
                buffer->yy_ch_buf = 0;
                LexerError("fatal error - scanner input buffer overflow");
            }

            const std::ptrdiff_t cursor_offset =
                yy_c_buf_p_ - buffer->yy_ch_buf;
            std::size_t new_size = buffer->yy_buf_size * 2;
            if (static_cast<int>(new_size) <= 0) {
                new_size = buffer->yy_buf_size +
                           buffer->yy_buf_size / 8;
            }
            char* grown = static_cast<char*>(
                std::realloc(buffer->yy_ch_buf, new_size + 2));
            if (!grown) {
                LexerError("fatal error - scanner input buffer overflow");
            }
            buffer->yy_ch_buf = grown;
            buffer->yy_buf_size = new_size;
            yy_c_buf_p_ = grown + cursor_offset;
            available = static_cast<int>(new_size) -
                        number_to_move - 1;
        }

        if (available > kReadBufferSize) {
            available = kReadBufferSize;
        }
        yy_n_chars_ = LexerInput(buffer->yy_ch_buf + number_to_move,
                                 available);
        if (yy_n_chars_ < 0) {
            LexerError("input in flex scanner failed");
        }
        buffer->yy_n_chars = yy_n_chars_;
    }

    int action = END_OF_BUFFER_CONTINUE;
    if (yy_n_chars_ == 0) {
        if (number_to_move != 0) {
            action = END_OF_BUFFER_LAST_MATCH;
            buffer->yy_buffer_status = 2;
        } else {
            action = END_OF_BUFFER_EOF;
            yyrestart(yyin_);
            buffer = yy_buffer_stack_[yy_buffer_stack_top_];
        }
    }

    const int total = yy_n_chars_ + number_to_move;
    if (total > static_cast<int>(buffer->yy_buf_size)) {
        const std::size_t new_size = static_cast<std::size_t>(total) +
            static_cast<std::size_t>(yy_n_chars_ / 2);
        char* grown = static_cast<char*>(
            std::realloc(buffer->yy_ch_buf, new_size));
        if (!grown) {
            LexerError("out of dynamic memory in yy_get_next_buffer()");
        }
        buffer->yy_ch_buf = grown;
    }

    yy_n_chars_ = total;
    buffer->yy_ch_buf[total] = 0;
    buffer->yy_ch_buf[total + 1] = 0;
    yytext_ = buffer->yy_ch_buf;
    return action;
}

// IDA_EA: 0x8AD21C0
// IDA_NAME: _ZN23__dnf_script__FlexLexer7yyinputEv
// DEMANGLED: __dnf_script__FlexLexer::yyinput(void)
int __dnf_script__FlexLexer::yyinput() {
    for (;;) {
        *yy_c_buf_p_ = yy_hold_char_;
        if (*yy_c_buf_p_ != 0) {
            break;
        }

        yy_buffer_state* buffer = yy_buffer_stack_[yy_buffer_stack_top_];
        if (yy_c_buf_p_ < buffer->yy_ch_buf + yy_n_chars_) {
            *yy_c_buf_p_ = 0;
            break;
        }

        char* old_cursor = yy_c_buf_p_++;
        const int offset = static_cast<int>(old_cursor - yytext_);
        const int action = yy_get_next_buffer();
        if (action != END_OF_BUFFER_EOF) {
            if (action == END_OF_BUFFER_LAST_MATCH) {
                yyrestart(yyin_);
            } else if (action == END_OF_BUFFER_CONTINUE) {
                yy_c_buf_p_ = yytext_ + offset;
            }
            break;
        }

        if (yywrap()) {
            return -1;
        }
        if (!yy_did_buffer_switch_on_eof_) {
            yyrestart(yyin_);
        }
    }

    const int value = static_cast<unsigned char>(*yy_c_buf_p_);
    *yy_c_buf_p_ = 0;
    ++yy_c_buf_p_;
    yy_hold_char_ = *yy_c_buf_p_;
    if (value == '\n') {
        ++yylineno_;
    }
    return value;
}

// IDA_EA: 0x8AD22B0
// IDA_NAME: _ZN23__dnf_script__FlexLexer13yy_push_stateEi
// DEMANGLED: __dnf_script__FlexLexer::yy_push_state(int)
void __dnf_script__FlexLexer::yy_push_state(int new_state) {
    if (yy_start_stack_ptr_ >= yy_start_stack_depth_) {
        yy_start_stack_depth_ += 25;
        int* grown = yy_start_stack_
            ? static_cast<int*>(std::realloc(
                  yy_start_stack_,
                  sizeof(int) * yy_start_stack_depth_))
            : static_cast<int*>(std::malloc(
                  sizeof(int) * yy_start_stack_depth_));
        yy_start_stack_ = grown;
        if (!yy_start_stack_) {
            LexerError("out of memory expanding start-condition stack");
        }
    }
    yy_start_stack_[yy_start_stack_ptr_++] = (yy_start_ - 1) / 2;
    yy_start_ = 2 * new_state + 1;
}

// IDA_EA: 0x8AD2350
// IDA_NAME: _ZN23__dnf_script__FlexLexer21yyensure_buffer_stackEv
// DEMANGLED: __dnf_script__FlexLexer::yyensure_buffer_stack(void)
void __dnf_script__FlexLexer::yyensure_buffer_stack() {
    if (!yy_buffer_stack_) {
        yy_buffer_stack_ = static_cast<yy_buffer_state**>(
            std::malloc(sizeof(yy_buffer_state*)));
        if (!yy_buffer_stack_) {
            LexerError(
                "out of dynamic memory in yyensure_buffer_stack()");
        }
        yy_buffer_stack_[0] = 0;
        yy_buffer_stack_max_ = 1;
        yy_buffer_stack_top_ = 0;
        return;
    }

    if (yy_buffer_stack_top_ >= yy_buffer_stack_max_ - 1) {
        const std::size_t old_max = yy_buffer_stack_max_;
        const std::size_t new_max = old_max + 8;
        yy_buffer_state** grown = static_cast<yy_buffer_state**>(
            std::realloc(yy_buffer_stack_,
                         sizeof(yy_buffer_state*) * new_max));
        yy_buffer_stack_ = grown;
        if (!yy_buffer_stack_) {
            LexerError(
                "out of dynamic memory in yyensure_buffer_stack()");
        }
        std::memset(yy_buffer_stack_ + old_max, 0,
                    sizeof(yy_buffer_state*) * 8);
        yy_buffer_stack_max_ = new_max;
    }
}

// IDA_EA: 0x8AD2420
// IDA_NAME: _ZN23__dnf_script__FlexLexer19yypush_buffer_stateEP15yy_buffer_state
// DEMANGLED: __dnf_script__FlexLexer::yypush_buffer_state(yy_buffer_state *)
void __dnf_script__FlexLexer::yypush_buffer_state(
    yy_buffer_state* new_buffer) {
    if (!new_buffer) {
        return;
    }

    yyensure_buffer_stack();
    if (yy_buffer_stack_[yy_buffer_stack_top_]) {
        *yy_c_buf_p_ = yy_hold_char_;
        yy_buffer_state* old_buffer =
            yy_buffer_stack_[yy_buffer_stack_top_];
        old_buffer->yy_buf_pos = yy_c_buf_p_;
        old_buffer->yy_n_chars = yy_n_chars_;
        ++yy_buffer_stack_top_;
        yyensure_buffer_stack();
    }
    yy_buffer_stack_[yy_buffer_stack_top_] = new_buffer;
    yy_load_buffer_state();
    yy_did_buffer_switch_on_eof_ = 1;
}

// IDA_EA: 0x8AD2500
// IDA_NAME: _ZN23__dnf_script__FlexLexer19yy_switch_to_bufferEP15yy_buffer_state
// DEMANGLED: __dnf_script__FlexLexer::yy_switch_to_buffer(yy_buffer_state *)
void __dnf_script__FlexLexer::yy_switch_to_buffer(
    yy_buffer_state* new_buffer) {
    yyensure_buffer_stack();
    if (yy_buffer_stack_[yy_buffer_stack_top_] == new_buffer) {
        return;
    }
    if (yy_buffer_stack_[yy_buffer_stack_top_]) {
        *yy_c_buf_p_ = yy_hold_char_;
        yy_buffer_state* old_buffer =
            yy_buffer_stack_[yy_buffer_stack_top_];
        old_buffer->yy_buf_pos = yy_c_buf_p_;
        old_buffer->yy_n_chars = yy_n_chars_;
    }
    yy_buffer_stack_[yy_buffer_stack_top_] = new_buffer;
    yy_load_buffer_state();
    yy_did_buffer_switch_on_eof_ = 1;
}

// IDA_EA: 0x8AD25A0
// IDA_NAME: _ZN23__dnf_script__FlexLexer14yy_init_bufferEP15yy_buffer_statePSi
// DEMANGLED: __dnf_script__FlexLexer::yy_init_buffer(yy_buffer_state *,std::istream *)
void __dnf_script__FlexLexer::yy_init_buffer(yy_buffer_state* buffer,
                                             std::istream* input) {
    const int saved_errno = errno;
    yy_flush_buffer(buffer);
    buffer->yy_fill_buffer = 1;
    buffer->yy_input_file = input;
    if (!yy_buffer_stack_ ||
        buffer != yy_buffer_stack_[yy_buffer_stack_top_]) {
        buffer->yy_bs_lineno = 1;
        buffer->yy_bs_column = 0;
    }
    buffer->yy_is_interactive = 0;
    errno = saved_errno;
}

// IDA_EA: 0x8AD2620
// IDA_NAME: _ZN23__dnf_script__FlexLexer16yy_create_bufferEPSii
// DEMANGLED: __dnf_script__FlexLexer::yy_create_buffer(std::istream *,int)
yy_buffer_state* __dnf_script__FlexLexer::yy_create_buffer(
    std::istream* input, int size) {
    yy_buffer_state* buffer = static_cast<yy_buffer_state*>(
        std::malloc(sizeof(yy_buffer_state)));
    if (!buffer) {
        LexerError("out of dynamic memory in yy_create_buffer()");
    }
    buffer->yy_buf_size = static_cast<std::size_t>(size);
    buffer->yy_ch_buf = static_cast<char*>(
        std::malloc(static_cast<std::size_t>(size) + 2));
    if (!buffer->yy_ch_buf) {
        LexerError("out of dynamic memory in yy_create_buffer()");
    }
    buffer->yy_is_our_buffer = 1;
    yy_init_buffer(buffer, input);
    return buffer;
}

// IDA_EA: 0x8AD26C0
// IDA_NAME: _ZN23__dnf_script__FlexLexer9yyrestartEPSi
// DEMANGLED: __dnf_script__FlexLexer::yyrestart(std::istream *)
void __dnf_script__FlexLexer::yyrestart(std::istream* input) {
    if (!yy_buffer_stack_ ||
        !yy_buffer_stack_[yy_buffer_stack_top_]) {
        yyensure_buffer_stack();
        yy_buffer_stack_[yy_buffer_stack_top_] =
            yy_create_buffer(yyin_, kDefaultBufferSize);
    }
    yy_init_buffer(yy_buffer_stack_[yy_buffer_stack_top_], input);
    yy_load_buffer_state();
}

// IDA_EA: 0x8AD2760
// IDA_NAME: _ZN23__dnf_script__FlexLexer11LexerOutputEPKci
// DEMANGLED: __dnf_script__FlexLexer::LexerOutput(char const*,int)
void __dnf_script__FlexLexer::LexerOutput(const char* buffer, int size) {
    yyout_->write(buffer, size);
}

// IDA_EA: 0x8AD2780
// IDA_NAME: _ZN23__dnf_script__FlexLexer10LexerInputEPci
// DEMANGLED: __dnf_script__FlexLexer::LexerInput(char *,int)
int __dnf_script__FlexLexer::LexerInput(char* buffer, int max_size) {
    if (!yyin_ || yyin_->eof() || yyin_->bad() || yyin_->fail()) {
        return 0;
    }
    yyin_->read(buffer, max_size);
    if (yyin_->bad()) {
        return -1;
    }
    return static_cast<int>(yyin_->gcount());
}

// IDA_EA: 0x8AD27E0
// IDA_NAME: _ZN23__dnf_script__FlexLexerD2Ev
// DEMANGLED: __dnf_script__FlexLexer::~__dnf_script__FlexLexer()
__dnf_script__FlexLexer::~__dnf_script__FlexLexer() {
    delete[] yy_state_buf_;
    std::free(yy_start_stack_);
    yy_buffer_state* buffer = 0;
    if (yy_buffer_stack_) {
        buffer = yy_buffer_stack_[yy_buffer_stack_top_];
    }
    yy_delete_buffer(buffer);
    std::free(yy_buffer_stack_);
}

// IDA_EA: 0x8AD2EE0
// IDA_NAME: _ZN23__dnf_script__FlexLexer10LexerErrorEPKc
// DEMANGLED: __dnf_script__FlexLexer::LexerError(char const*)
void __dnf_script__FlexLexer::LexerError(const char* message) {
    if (message) {
        std::cerr << message;
    } else {
        std::cerr.setstate(std::ios::badbit);
    }
    std::cerr << std::endl;
    std::exit(2);
}
