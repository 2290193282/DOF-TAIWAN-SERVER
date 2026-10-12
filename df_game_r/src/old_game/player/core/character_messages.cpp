#include "old_game/player/core/character_messages.h"

#include <cstring>
#include <new>


typedef char MSG_CHARAC_SELECT_Size_Must_Be_0x13[
    (sizeof(MSG_CHARAC_SELECT) == 0x13) ? 1 : -1];
typedef char MSG_REQUEST_CHARAC_SKILL_INFO_Size_Must_Be_0x0e[
    (sizeof(MSG_REQUEST_CHARAC_SKILL_INFO) == 0x0e) ? 1 : -1];
typedef char MSG_DIE_CHARACTER_Size_Must_Be_0x11[
    (sizeof(MSG_DIE_CHARACTER) == 0x11) ? 1 : -1];
typedef char MSG_EQUIPSLOT_SWITCH_Size_Must_Be_0x10[
    (sizeof(MSG_EQUIPSLOT_SWITCH) == 0x10) ? 1 : -1];
typedef char MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE_Size_Must_Be_0x0e[
    (sizeof(MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE) == 0x0e) ? 1 : -1];
typedef char MSG_SAVE_CHARACTER_OPTION_Size_Must_Be_0x211[
    (sizeof(MSG_SAVE_CHARACTER_OPTION) == 0x211) ? 1 : -1];
typedef char MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET_Size_Must_Be_0x0e[
    (sizeof(MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET) == 0x0e) ? 1 : -1];
typedef char MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON_slot_String_Size_Must_Be_0x06[
    (sizeof(MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String) == 0x06) ? 1 : -1];

// IDA_EA: 0x822D9D8
// IDA_NAME: _ZN17MSG_CHARAC_SELECTC2Ev
// DEMANGLED: MSG_CHARAC_SELECT::MSG_CHARAC_SELECT(void)
MSG_CHARAC_SELECT::MSG_CHARAC_SELECT()
    : MSG_BASE(),
      character_slot(0xff),
      tag_character_no(0),
      striker_skill_index(0) {}

// IDA_EA: 0x822E2B6
// IDA_NAME: _ZN29MSG_REQUEST_CHARAC_SKILL_INFOC2Ev
// DEMANGLED: MSG_REQUEST_CHARAC_SKILL_INFO::MSG_REQUEST_CHARAC_SKILL_INFO(void)
MSG_REQUEST_CHARAC_SKILL_INFO::MSG_REQUEST_CHARAC_SKILL_INFO()
    : MSG_BASE() {
    character_slot = 0xff;
}

// IDA_EA: 0x8255470
// IDA_NAME: _ZN25MSG_SAVE_CHARACTER_OPTIONC2Ev
// DEMANGLED: MSG_SAVE_CHARACTER_OPTION::MSG_SAVE_CHARACTER_OPTION(void)
MSG_SAVE_CHARACTER_OPTION::MSG_SAVE_CHARACTER_OPTION()
    : MSG_BASE(),
      option_11() {}

// IDA_EA: 0x8260434
// IDA_NAME: _ZN42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESETC2Ev
// DEMANGLED: MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET::MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET(void)
MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET::
    MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET()
    : MSG_BASE() {
    byte_0d = 0;
}

// IDA_EA: 0x8235134
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON12_slot_StringC2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::_slot_String(void)
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::_slot_String()
    : word_00(0) {
    new (static_cast<void*>(text_02)) std::string();
}

// IDA_EA: 0x823514A
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON12_slot_StringD2Ev
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::~_slot_String()
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::~_slot_String() {
    std::string* text = static_cast<std::string*>(
        static_cast<void*>(text_02));
    text->~basic_string();
}

// IDA_EA: 0x8239E66
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON12_slot_StringC2ERKS0_
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::_slot_String(MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String const&)
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::_slot_String(
    const _slot_String& rhs)
    : word_00(rhs.word_00) {
    const std::string* rhs_text = static_cast<const std::string*>(
        static_cast<const void*>(rhs.text_02));
    new (static_cast<void*>(text_02)) std::string(*rhs_text);
}

// IDA_EA: 0x8239F16
// IDA_NAME: _ZN38MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON12_slot_StringaSERKS0_
// DEMANGLED: MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::operator=(MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String const&)
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String&
MSG_SAVE_GAME_OPTION_CHATTING_EMOTICON::_slot_String::operator=(
    const _slot_String& rhs) {
    word_00 = rhs.word_00;
    std::string* text = static_cast<std::string*>(
        static_cast<void*>(text_02));
    const std::string* rhs_text = static_cast<const std::string*>(
        static_cast<const void*>(rhs.text_02));
    *text = *rhs_text;
    return *this;
}
