#include "old_data/PVF/common/arad_script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>


typedef char AradScript_Size_Must_Be_0x54[(sizeof(Arad_Script) == 0x54) ? 1 : -1];

// IDA_EA: 0x817B474
// IDA_NAME: _ZN11Arad_ScriptC2Ev
// DEMANGLED: Arad_Script::Arad_Script(void)
Arad_Script::Arad_Script() {}

// IDA_EA: 0x817B532
// IDA_NAME: _ZN11Arad_ScriptC2EPKc
// DEMANGLED: Arad_Script::Arad_Script(char const*)
Arad_Script::Arad_Script(const char* file_name) {
    const char* path = make_file_path(file_name);
    file_path_.append(path);
}

// IDA_EA: 0x817B624
// IDA_NAME: _ZN11Arad_ScriptD2Ev
// DEMANGLED: Arad_Script::~Arad_Script()
Arad_Script::~Arad_Script() {
    if (cells_) {
        delete[] cells_;
    }
}

// IDA_EA: 0x817B70A
// IDA_NAME: _ZN11Arad_Script14make_file_pathEPKc
// DEMANGLED: Arad_Script::make_file_path(char const*)
char* Arad_Script::make_file_path(const char* file_name) {
    static char full_path[1024];
    strcpy(full_path, file_name);
    return full_path;
}

// IDA_EA: 0x817B72A
// IDA_NAME: _ZN11Arad_Script6CreateEv
// DEMANGLED: Arad_Script::Create(void)
int Arad_Script::Create() {
    col_capacity_ = 32;
    row_capacity_ = 256;
    buffer_size_ = (col_capacity_ * row_capacity_) << 7;
    cells_ = new (std::nothrow) char[buffer_size_];
    if (!cells_) {
        return 0;
    }
    memset(cells_, 0, buffer_size_);
    col_count_ = 0;
    row_count_ = 0;
    col_index_.clear();
    row_index_.clear();
    return 1;
}

// IDA_EA: 0x817B800
// IDA_NAME: _ZN11Arad_Script9ExpandColEv
// DEMANGLED: Arad_Script::ExpandCol(void)
int Arad_Script::ExpandCol() {
    col_capacity_ += 32;
    buffer_size_ = (row_capacity_ * col_capacity_) << 7;
    char* next_cells = new (std::nothrow) char[buffer_size_];
    if (!next_cells) {
        return 0;
    }
    memset(next_cells, 0, buffer_size_);
    if (cells_) {
        delete[] cells_;
    }
    cells_ = next_cells;
    return 1;
}

// IDA_EA: 0x817B892
// IDA_NAME: _ZN11Arad_Script9ExpandRowEv
// DEMANGLED: Arad_Script::ExpandRow(void)
int Arad_Script::ExpandRow() {
    const int old_size = buffer_size_;
    row_capacity_ *= 2;
    buffer_size_ = (row_capacity_ * col_capacity_) << 7;
    char* next_cells = new (std::nothrow) char[buffer_size_];
    if (!next_cells) {
        return 0;
    }
    memset(next_cells + old_size, 0, buffer_size_ - old_size);
    memcpy(next_cells, cells_, old_size);
    if (cells_) {
        delete[] cells_;
    }
    cells_ = next_cells;
    return 1;
}

// IDA_EA: 0x817B950
// IDA_NAME: _ZN11Arad_Script4LoadEPKcS1_
// DEMANGLED: Arad_Script::Load(char const*,char const*)
int Arad_Script::Load(const char* buffer, const char* section_name) {
    if (Create() != 1) {
        return 0;
    }

    char section[1024];
    char token[512];
    memset(section, 0, sizeof(section));
    sprintf(section, "[%s]", section_name);

    size_t matched = 0;
    const size_t section_len = strlen(section);
    int ch = 0;
    do {
        do {
            ch = static_cast<signed char>(*buffer++);
            if (ch == -1) {
                return 0;
            }
        } while (ch <= 32 || ch > 126);

        if (section[matched] == ch) {
            ++matched;
        } else {
            matched = 0;
        }
    } while (matched != section_len);

    memset(token, 0, sizeof(token));
    int token_len = 0;
    do {
        do {
            ch = static_cast<signed char>(*buffer++);
            if (ch == -1) {
                return 0;
            }
            if (ch == 32 || ch == 9 || ch == 10 || ch == 13) {
                break;
            }
            token[token_len++] = static_cast<char>(ch);
        } while (1);

        if (token_len > 0) {
            token[token_len] = 0;
            token_len = 0;

            char* col_name = token;
            const size_t token_len_total = strlen(token);
            for (size_t i = 0; i < token_len_total; ++i) {
                if (token[i] == '/') {
                    token[i] = 0;
                    col_index_[std::string(col_name)] = col_count_;
                    col_name = &token[i + 1];
                }
            }

            col_index_[std::string(col_name)] = col_count_;
            ++col_count_;
            if (col_count_ + 1 >= col_capacity_ && ExpandCol() != 1) {
                return 0;
            }
        }
    } while (ch != 10);

    do {
        int col = -1;
        token_len = 0;
        do {
            do {
                ch = static_cast<signed char>(*buffer++);
                if (ch == 32 || ch == 9 || ch == 10 || ch == 13 || ch == -1) {
                    break;
                }
                token[token_len++] = static_cast<char>(ch);
            } while (1);

            if (token_len <= 0) {
                ch = -1;
                goto load_memory_next_row;
            }

            token[token_len] = 0;
            token_len = 0;
            if (col == -1) {
                row_index_[std::string(token)] = row_count_;
            } else {
                strcpy(cells_ + ((row_count_ * col_capacity_ + col) << 7), token);
            }

            if (col_count_ == ++col) {
                while (ch != -1 && ch != 10) {
                    ch = static_cast<signed char>(*buffer++);
                }
            }
        } while (ch != 10 && ch != -1);

        if (col_count_ != col) {
            return 0;
        }
        if (++row_count_ + 1 >= row_capacity_ && ExpandRow() != 1) {
            return 0;
        }

load_memory_next_row:
        ;
    } while (ch != -1);

    return 1;
}

// IDA_EA: 0x817BEA4
// IDA_NAME: _ZN11Arad_Script4LoadEPKc
// DEMANGLED: Arad_Script::Load(char const*)
int Arad_Script::Load(const char* section_name) {
    FILE* fp = fopen(file_path_.c_str(), "rt");
    if (!fp) {
        return 0;
    }

    if (Create() != 1) {
        return 0;
    }

    char section[1024];
    char token[512];
    memset(section, 0, sizeof(section));
    sprintf(section, "[%s]", section_name);

    size_t matched = 0;
    const size_t section_len = strlen(section);
    int ch = 0;
    do {
        do {
            ch = fgetc(fp);
            if (ch == -1) {
                return 0;
            }
        } while (ch <= 32 || ch > 126);

        if (section[matched] == ch) {
            ++matched;
        } else {
            matched = 0;
        }
    } while (matched != section_len);

    memset(token, 0, sizeof(token));
    int token_len = 0;
    do {
        do {
            ch = fgetc(fp);
            if (ch == -1) {
                return 0;
            }
            if (ch == 32 || ch == 9 || ch == 10 || ch == 13) {
                break;
            }
            token[token_len++] = static_cast<char>(ch);
        } while (1);

        if (token_len > 0) {
            token[token_len] = 0;
            token_len = 0;

            char* col_name = token;
            const size_t token_len_total = strlen(token);
            for (size_t i = 0; i < token_len_total; ++i) {
                if (token[i] == '/') {
                    token[i] = 0;
                    col_index_[std::string(col_name)] = col_count_;
                    col_name = &token[i + 1];
                }
            }

            col_index_[std::string(col_name)] = col_count_;
            ++col_count_;
            if (col_count_ + 1 >= col_capacity_ && ExpandCol() != 1) {
                return 0;
            }
        }
    } while (ch != 10);

    do {
        int col = -1;
        token_len = 0;
        do {
            do {
                ch = fgetc(fp);
                if (ch == 32 || ch == 9 || ch == 10 || ch == 13 || ch == -1) {
                    break;
                }
                token[token_len++] = static_cast<char>(ch);
            } while (1);

            if (token_len <= 0) {
                ch = -1;
                goto load_file_next_row;
            }

            token[token_len] = 0;
            token_len = 0;
            if (col == -1) {
                row_index_[std::string(token)] = row_count_;
            } else {
                strcpy(cells_ + ((row_count_ * col_capacity_ + col) << 7), token);
            }

            if (col_count_ == ++col) {
                while (ch != -1 && ch != 10) {
                    ch = fgetc(fp);
                }
            }
        } while (ch != 10 && ch != -1);

        if (col_count_ != col) {
            return 0;
        }
        if (++row_count_ + 1 >= row_capacity_ && ExpandRow() != 1) {
            return 0;
        }

load_file_next_row:
        ;
    } while (ch != -1);

    fclose(fp);
    return 1;
}

// IDA_EA: 0x817C44E
// IDA_NAME: _ZN11Arad_Script10GetIdx4ColEPc
// DEMANGLED: Arad_Script::GetIdx4Col(char *)
int Arad_Script::GetIdx4Col(char* col_name) {
    std::map<std::string, int>::iterator it = col_index_.find(std::string(col_name));
    if (it != col_index_.end()) {
        return it->second;
    }
    return -1;
}

// IDA_EA: 0x817C52E
// IDA_NAME: _ZN11Arad_Script10GetIdx4RowEPc
// DEMANGLED: Arad_Script::GetIdx4Row(char *)
int Arad_Script::GetIdx4Row(char* row_name) {
    std::map<std::string, int>::iterator it = row_index_.find(std::string(row_name));
    if (it != row_index_.end()) {
        return it->second;
    }
    return -1;
}

// IDA_EA: 0x817C60E
// IDA_NAME: _ZN11Arad_Script9GetStringEii
// DEMANGLED: Arad_Script::GetString(int,int)
char* Arad_Script::GetString(int row, int col) {
    return cells_ + ((row * col_capacity_ + col) << 7);
}

// IDA_EA: 0x817C630
// IDA_NAME: _ZN11Arad_Script9GetStringEPcS0_
// DEMANGLED: Arad_Script::GetString(char *,char *)
char* Arad_Script::GetString(char* row_name, char* col_name) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return GetString(row, col);
}

// IDA_EA: 0x817C676
// IDA_NAME: _ZN11Arad_Script9GetStringEPci
// DEMANGLED: Arad_Script::GetString(char *,int)
char* Arad_Script::GetString(char* row_name, int col) {
    int row = GetIdx4Row(row_name);
    return GetString(row, col);
}

// IDA_EA: 0x817C6A6
// IDA_NAME: _ZN11Arad_Script9GetStringEiPc
// DEMANGLED: Arad_Script::GetString(int,char *)
char* Arad_Script::GetString(int row, char* col_name) {
    int col = GetIdx4Col(col_name);
    return GetString(row, col);
}

// IDA_EA: 0x817C6D6
// IDA_NAME: _ZN11Arad_Script6GetIntEii
// DEMANGLED: Arad_Script::GetInt(int,int)
int Arad_Script::GetInt(int row, int col) {
    return atoi(cells_ + ((row * col_capacity_ + col) << 7));
}

// IDA_EA: 0x817C70A
// IDA_NAME: _ZN11Arad_Script6GetIntEPcS0_
// DEMANGLED: Arad_Script::GetInt(char *,char *)
int Arad_Script::GetInt(char* row_name, char* col_name) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return GetInt(row, col);
}

// IDA_EA: 0x817C750
// IDA_NAME: _ZN11Arad_Script6GetIntEPci
// DEMANGLED: Arad_Script::GetInt(char *,int)
int Arad_Script::GetInt(char* row_name, int col) {
    int row = GetIdx4Row(row_name);
    return GetInt(row, col);
}

// IDA_EA: 0x817C780
// IDA_NAME: _ZN11Arad_Script6GetIntEiPc
// DEMANGLED: Arad_Script::GetInt(int,char *)
int Arad_Script::GetInt(int row, char* col_name) {
    int col = GetIdx4Col(col_name);
    return GetInt(row, col);
}

// IDA_EA: 0x817C7B0
// IDA_NAME: _ZN11Arad_Script8GetFloatEii
// DEMANGLED: Arad_Script::GetFloat(int,int)
float Arad_Script::GetFloat(int row, int col) {
    return static_cast<float>(atof(cells_ + ((row * col_capacity_ + col) << 7)));
}

// IDA_EA: 0x817C7EA
// IDA_NAME: _ZN11Arad_Script8GetFloatEPcS0_
// DEMANGLED: Arad_Script::GetFloat(char *,char *)
float Arad_Script::GetFloat(char* row_name, char* col_name) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return GetFloat(row, col);
}

// IDA_EA: 0x817C83C
// IDA_NAME: _ZN11Arad_Script8GetFloatEPci
// DEMANGLED: Arad_Script::GetFloat(char *,int)
float Arad_Script::GetFloat(char* row_name, int col) {
    int row = GetIdx4Row(row_name);
    return GetFloat(row, col);
}

// IDA_EA: 0x817C878
// IDA_NAME: _ZN11Arad_Script8GetFloatEiPc
// DEMANGLED: Arad_Script::GetFloat(int,char *)
float Arad_Script::GetFloat(int row, char* col_name) {
    int col = GetIdx4Col(col_name);
    return GetFloat(row, col);
}

// IDA_EA: 0x817C8B4
// IDA_NAME: _ZN11Arad_Script9GetDoubleEii
// DEMANGLED: Arad_Script::GetDouble(int,int)
double Arad_Script::GetDouble(int row, int col) {
    return atof(cells_ + ((row * col_capacity_ + col) << 7));
}

// IDA_EA: 0x817C8E8
// IDA_NAME: _ZN11Arad_Script9GetDoubleEPcS0_
// DEMANGLED: Arad_Script::GetDouble(char *,char *)
double Arad_Script::GetDouble(char* row_name, char* col_name) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return GetDouble(row, col);
}

// IDA_EA: 0x817C92E
// IDA_NAME: _ZN11Arad_Script9GetDoubleEPci
// DEMANGLED: Arad_Script::GetDouble(char *,int)
double Arad_Script::GetDouble(char* row_name, int col) {
    int row = GetIdx4Row(row_name);
    return GetDouble(row, col);
}

// IDA_EA: 0x817C95E
// IDA_NAME: _ZN11Arad_Script9GetDoubleEiPc
// DEMANGLED: Arad_Script::GetDouble(int,char *)
double Arad_Script::GetDouble(int row, char* col_name) {
    int col = GetIdx4Col(col_name);
    return GetDouble(row, col);
}

// IDA_EA: 0x817C98E
// IDA_NAME: _ZN11Arad_Script9SetStringEiiPc
// DEMANGLED: Arad_Script::SetString(int,int,char *)
char* Arad_Script::SetString(int row, int col, char* value) {
    return strcpy(cells_ + ((row * col_capacity_ + col) << 7), value);
}

// IDA_EA: 0x817C9C2
// IDA_NAME: _ZN11Arad_Script9SetStringEPcS0_S0_
// DEMANGLED: Arad_Script::SetString(char *,char *,char *)
char* Arad_Script::SetString(char* row_name, char* col_name, char* value) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return SetString(row, col, value);
}

// IDA_EA: 0x817CA10
// IDA_NAME: _ZN11Arad_Script9SetStringEPciS0_
// DEMANGLED: Arad_Script::SetString(char *,int,char *)
char* Arad_Script::SetString(char* row_name, int col, char* value) {
    int row = GetIdx4Row(row_name);
    return SetString(row, col, value);
}

// IDA_EA: 0x817CA48
// IDA_NAME: _ZN11Arad_Script9SetStringEiPcS0_
// DEMANGLED: Arad_Script::SetString(int,char *,char *)
char* Arad_Script::SetString(int row, char* col_name, char* value) {
    int col = GetIdx4Col(col_name);
    return SetString(row, col, value);
}

// IDA_EA: 0x817CA80
// IDA_NAME: _ZN11Arad_Script6SetIntEiii
// DEMANGLED: Arad_Script::SetInt(int,int,int)
int Arad_Script::SetInt(int row, int col, int value) {
    return sprintf(cells_ + ((row * col_capacity_ + col) << 7), "%d", value);
}

// IDA_EA: 0x817CABC
// IDA_NAME: _ZN11Arad_Script6SetIntEPcS0_i
// DEMANGLED: Arad_Script::SetInt(char *,char *,int)
int Arad_Script::SetInt(char* row_name, char* col_name, int value) {
    int col = GetIdx4Col(col_name);
    int row = GetIdx4Row(row_name);
    return SetInt(row, col, value);
}

// IDA_EA: 0x817CB0A
// IDA_NAME: _ZN11Arad_Script6SetIntEPcii
// DEMANGLED: Arad_Script::SetInt(char *,int,int)
int Arad_Script::SetInt(char* row_name, int col, int value) {
    int row = GetIdx4Row(row_name);
    return SetInt(row, col, value);
}

// IDA_EA: 0x817CB42
// IDA_NAME: _ZN11Arad_Script6SetIntEiPci
// DEMANGLED: Arad_Script::SetInt(int,char *,int)
int Arad_Script::SetInt(int row, char* col_name, int value) {
    int col = GetIdx4Col(col_name);
    return SetInt(row, col, value);
}

// IDA_EA: 0x81A427C
// IDA_NAME: _ZN11Arad_Script9GetRowNumEv
// DEMANGLED: Arad_Script::GetRowNum(void)
int Arad_Script::GetRowNum() {
    return row_count_;
}
