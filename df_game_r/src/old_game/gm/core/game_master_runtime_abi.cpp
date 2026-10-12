#include "old_game/gm/core/game_master_runtime_abi.h"

#include <cstdlib>
#include <new>

namespace game_master {
namespace {

// CParser::C2 at 0x82353AF stores a constant pointer at +0x00.  Its original
// address belongs to the old executable's private vtable section, so only the
// required non-null construction invariant can be reproduced in a rebuilt
// image; no recovered provider observes this marker.
const char kParserAbiMarker = 0;

}  // namespace

CParser::CParser()
    : abi_marker_(&kParserAbiMarker) {
    // IDA C2 constructs only the three strings and vector.  The caller owns
    // initialization of user_, raw_command_buffer_, split_index_ and
    // command_id_, so deliberately do not turn them into reset-state fields.
}

CParser::~CParser() {}

int CParser::parse() {
    // 0x84A8AB6 copies the raw +0x1c buffer, finds the first literal space,
    // assigns the prefix/tail strings, trims the prefix and parses its integer
    // value.  Keep std::string::npos + 1 wrap behaviour of the old unsigned
    // arithmetic instead of rejecting a command with no space.
    working_command_ = raw_command_buffer_;
    split_index_ = static_cast<unsigned int>(working_command_.find(" ", 0));
    first_token_ = working_command_.substr(0, split_index_);
    command_tail_ = working_command_.substr(split_index_ + 1,
                                             std::string::npos);

    // trim @0x829F068 considers literal spaces only.  In particular, an
    // all-space token is deliberately left unchanged; do not normalize it to
    // an empty or tab-stripped string.
    const std::string::size_type begin = first_token_.find_first_not_of(" ");
    if (begin != std::string::npos) {
        first_token_.erase(0, begin);
    }
    const std::string::size_type end = first_token_.find_last_not_of(" ");
    if (end != std::string::npos) {
        first_token_.erase(end + 1, std::string::npos);
    }
    command_id_ = std::atoi(first_token_.c_str());
    return command_id_;
}

CCommand::CCommand() : parser_(0) {}

void CCommand::parse() {}

bool CCommand::HasJuniorPermission() { return true; }

bool CCommand::HasSeniorPermission() { return true; }

CCommand* CCommand::SetParser(CParser* parser) {
    parser_ = parser;
    return this;
}

int CCommand::executeTemplate() {
    parse();
    execute();
    // The old implementation's ABI returns the volatile EAX left by a void
    // virtual execute slot.  No recovered caller consumes that value; retain
    // the typed command behavior and the original call order instead of
    // inventing per-command success values.
    return 0;
}

CUser* CCommand::GetUser() const {
    return parser_ ? parser_->user() : 0;
}

IGameMaster::IGameMaster(IGameMaster* next) : next_(next) {}

int IGameMaster::HandleRequest(CCommand* command) {
    return next_ ? next_->HandleRequest(command) : 0;
}

CJuniorGameMaster::CJuniorGameMaster(IGameMaster* next)
    : IGameMaster(next) {}

int CJuniorGameMaster::HandleRequest(CCommand* command) {
    if (command && command->HasJuniorPermission()) {
        return command->executeTemplate();
    }
    return IGameMaster::HandleRequest(command);
}

CSeniorGameMaster::CSeniorGameMaster(IGameMaster* next)
    : IGameMaster(next) {}

int CSeniorGameMaster::HandleRequest(CCommand* command) {
    return (command && command->HasSeniorPermission())
               ? command->executeTemplate()
               : 0;
}

CCommandMap::CCommandMap() : named_commands_(10) {}

CCommandMap::~CCommandMap() {
    // The old D2 at 0x82A7D88 calls operator delete directly for every stored
    // pointer before destroying the containers.  There is intentionally no
    // virtual destructor dispatch here.
    for (std::map<int, CCommand*>::iterator it = number_commands_.begin();
         it != number_commands_.end(); ++it) {
        if (it->second) {
            ::operator delete(it->second);
        }
    }
    for (std::unordered_map<std::string, CCommand*>::iterator it =
             named_commands_.begin();
         it != named_commands_.end(); ++it) {
        if (it->second) {
            ::operator delete(it->second);
        }
    }
}

// IDA_EA: 0x82A7C76
// IDA_NAME: _ZN11game_master11CCommandMap3addEiPNS_8CCommandE
// DEMANGLED: game_master::CCommandMap::add(int,game_master::CCommand *)
int CCommandMap::add(int command_id, CCommand* command) {
    return number_commands_.insert(std::make_pair(command_id, command)).second
               ? 1
               : 0;
}

// IDA_EA: 0x82A7CDC
// IDA_NAME: _ZN11game_master11CCommandMap3addESsPNS_8CCommandE
// DEMANGLED: game_master::CCommandMap::add(std::string,game_master::CCommand *)
void CCommandMap::add(std::string command_name, CCommand* command) {
    named_commands_.insert(std::make_pair(command_name, command));
}

// IDA_EA: 0x84B3FC8
// IDA_NAME: _ZN11game_master11CCommandMap3getEi
// DEMANGLED: game_master::CCommandMap::get(int)
CCommand* CCommandMap::get(int command_id) {
    std::map<int, CCommand*>::const_iterator it =
        number_commands_.find(command_id);
    return it == number_commands_.end() ? 0 : it->second;
}

// IDA_EA: 0x84B402C
// IDA_NAME: _ZN11game_master11CCommandMap3getERSs
// DEMANGLED: game_master::CCommandMap::get(std::string &)
CCommand* CCommandMap::get(std::string& command_name) {
    std::unordered_map<std::string, CCommand*>::const_iterator it =
        named_commands_.find(command_name);
    return it == named_commands_.end() ? 0 : it->second;
}

}  // namespace game_master
