#ifndef OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_RUNTIME_ABI_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_RUNTIME_ABI_H_

// Shared ABI root for the old GameMaster command runtime.
//
// IDA evidence:
//   CParser::C2  0x82353A4
//   CParser::D2  0x8235446
//   CParser::parse 0x84A8AB6
//   CCommandMap::{C2,D2,add,get} 0x82A7C76..0x84B402C
//   CCommand / IGameMaster / Junior / Senior 0x82A7C5C..0x84B3FBA
//
// This header deliberately contains no substitute command implementation.
// A command is registered only when its original parse, mutation and response
// transaction have been restored in its owning gameplay domain.

#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class CUser;

namespace game_master {

class CParser {
public:
    CParser();
    ~CParser();

    // IDA_EA: 0x84A8AB6.  Used by DoUserDefineCommand; CMD179 supplies the
    // token_vector_ directly instead, exactly as 0x820BDE0 does.
    int parse();

    CUser* user() const { return user_; }
    void set_user(CUser* user) { user_ = user; }

    std::string& first_token() { return first_token_; }
    const std::string& first_token() const { return first_token_; }
    std::string& command_tail() { return command_tail_; }
    const std::string& command_tail() const { return command_tail_; }
    int command_id() const { return command_id_; }

    char* raw_command_buffer() { return raw_command_buffer_; }
    const char* raw_command_buffer() const { return raw_command_buffer_; }

    std::vector<std::string>& token_vector() { return token_vector_; }
    const std::vector<std::string>& token_vector() const {
        return token_vector_;
    }

public:
    // This is an ABI object rather than a value object.  Keep its field map
    // visible so the restored command units can use IDA-verified offsets
    // without reintroducing raw pointer arithmetic.
    // +0x00 is written by C2 to the old compiler's static ABI marker.  It is
    // never read by the recovered code, but remains a non-null process-local
    // marker so the layout and construction invariant are retained.
    const void* abi_marker_;
    // +0x04
    CUser* user_;
    // +0x08, +0x0c, +0x10
    std::string first_token_;
    std::string command_tail_;
    std::string working_command_;
    // +0x14, +0x18
    unsigned int split_index_;
    int command_id_;
    // +0x1c .. +0x11b.  The old caller writes this buffer with sprintf.
    char raw_command_buffer_[0x100];
    // +0x11c
    std::vector<std::string> token_vector_;

};

// CCommand has no virtual destructor in the old vtable.  The slot order is
// execute, parse, HasJuniorPermission, HasSeniorPermission; executeTemplate
// at 0x84B3F90 explicitly invokes slots 1 then 0.
class CCommand {
public:
    CCommand();

    virtual void execute() = 0;
    virtual void parse();
    virtual bool HasJuniorPermission();
    virtual bool HasSeniorPermission();

    CCommand* SetParser(CParser* parser);
    int executeTemplate();
    CUser* GetUser() const;

protected:
    CParser* parser_;
};

class IGameMaster {
public:
    explicit IGameMaster(IGameMaster* next);

    virtual int HandleRequest(CCommand* command);

protected:
    IGameMaster* next_;
};

class CJuniorGameMaster : public IGameMaster {
public:
    explicit CJuniorGameMaster(IGameMaster* next);
    virtual int HandleRequest(CCommand* command);
};

class CSeniorGameMaster : public IGameMaster {
public:
    explicit CSeniorGameMaster(IGameMaster* next);
    virtual int HandleRequest(CCommand* command);
};

class CCommandMap {
public:
    CCommandMap();
    ~CCommandMap();

    // IDA_EA: 0x82A7C76
    // IDA_NAME: _ZN11game_master11CCommandMap3addEiPNS_8CCommandE
    int add(int command_id, CCommand* command);
    // IDA_EA: 0x82A7CDC
    // IDA_NAME: _ZN11game_master11CCommandMap3addESsPNS_8CCommandE
    // The original ABI passes the command name by value, rather than by a
    // const reference.  This matters for the exported C++ symbol.
    void add(std::string command_name, CCommand* command);
    // IDA_EA: 0x84B3FC8
    // IDA_NAME: _ZN11game_master11CCommandMap3getEi
    CCommand* get(int command_id);
    // IDA_EA: 0x84B402C
    // IDA_NAME: _ZN11game_master11CCommandMap3getERSs
    CCommand* get(std::string& command_name);

public:
    // IDA C2 puts the string table at +0x18 after the i386 std::map.
    std::map<int, CCommand*> number_commands_;
    std::unordered_map<std::string, CCommand*> named_commands_;
    // GCC 4.9's i386 unordered_map is 0x1c bytes, while the old server's
    // container occupied 0x20 bytes.  Preserve the original aggregate extent
    // without changing the recovered map behavior.
    uint32_t named_commands_abi_tail_;
};

#if defined(__i386__)
static_assert(offsetof(CParser, user_) == 0x04,
              "CParser::user_ must remain at old ABI +0x04");
static_assert(offsetof(CParser, first_token_) == 0x08,
              "CParser::first_token_ must remain at old ABI +0x08");
static_assert(offsetof(CParser, command_tail_) == 0x0c,
              "CParser::command_tail_ must remain at old ABI +0x0c");
static_assert(offsetof(CParser, working_command_) == 0x10,
              "CParser::working_command_ must remain at old ABI +0x10");
static_assert(offsetof(CParser, split_index_) == 0x14,
              "CParser::split_index_ must remain at old ABI +0x14");
static_assert(offsetof(CParser, command_id_) == 0x18,
              "CParser::command_id_ must remain at old ABI +0x18");
static_assert(offsetof(CParser, raw_command_buffer_) == 0x1c,
              "CParser raw command buffer must remain at old ABI +0x1c");
static_assert(offsetof(CParser, token_vector_) == 0x11c,
              "CParser token vector must remain at old ABI +0x11c");
static_assert(sizeof(CParser) == 0x128,
              "CParser must remain the old 0x128-byte object");
static_assert(offsetof(CCommandMap, named_commands_) == 0x18,
              "CCommandMap string table must remain at old ABI +0x18");
static_assert(sizeof(CCommandMap) == 0x38,
              "CCommandMap must remain the old 0x38-byte object");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_RUNTIME_ABI_H_
