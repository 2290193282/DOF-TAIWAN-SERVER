#include "common/manager/common_time_game_option_abi.h"

#include "common/config.h"
#include <cstring>


// The 140828 hotkey text objects are direct 1280-byte CGameOption children.

// IDA_EA: 0x822AB72
// IDA_NAME: _ZNK10CETCOption12getETCOptionE15ENUM_OPTION_ETC
// DEMANGLED: CETCOption::getETCOption(ENUM_OPTION_ETC)const
int CETCOption::getETCOption(int etc_option) const {
    if (static_cast<unsigned int>(etc_option) <
        oldsrv::config::kGameEtcOptionCount) {
        return static_cast<uint16_t>(values_[etc_option]);
    }
    return -1;
}

// IDA_EA: 0x84B6CF0
// IDA_NAME: _ZN11CGameOption15SetHotKeyOptionEhR13CHotkeyOption
// DEMANGLED: CGameOption::SetHotKeyOption(unsigned char,CHotkeyOption &)
int CGameOption::SetHotKeyOption(unsigned char key_type,
                                 CHotkeyOption& option) {
    int result = key_type;
    if (key_type <= 1u) {
        std::memcpy(&hotkeys_[key_type], &option, sizeof(option));
        hotkey_load_flags_[key_type] = 1;
        result = key_type;
    }
    return result;
}

// IDA_EA: 0x84B6DA0
// IDA_NAME: _ZN11CGameOption21IsHotKeyOptionChangedEhR13CHotkeyOption
// DEMANGLED: CGameOption::IsHotKeyOptionChanged(unsigned char,CHotkeyOption &)
int CGameOption::IsHotKeyOptionChanged(unsigned char key_type,
                                       CHotkeyOption& option) {
    return key_type <= 1u &&
           std::memcmp(&hotkeys_[key_type], &option, sizeof(option)) != 0;
}

// IDA_EA: 0x84B6DF4
// IDA_NAME: _ZN11CGameOption12SetEtcOptionER10CETCOption
// DEMANGLED: CGameOption::SetEtcOption(CETCOption &)
uint16_t* CGameOption::SetEtcOption(CETCOption& option) {
    std::memcpy(&etc_option_, &option, sizeof(option));
    // The original i386 copy leaves EAX at the final two-byte destination.
    return reinterpret_cast<uint16_t*>(
        reinterpret_cast<uint8_t*>(&etc_option_) + sizeof(option) - 2u);
}

// IDA_EA: 0x84B6E74
// IDA_NAME: _ZN11CGameOption18SetQuickChatOptionERK20CQuickChattingOptionj
// DEMANGLED: CGameOption::SetQuickChatOption(CQuickChattingOption const&,unsigned int)
uint16_t* CGameOption::SetQuickChatOption(
    const CQuickChattingOption& option,
    unsigned int quick_chat_type) {
    if (!IsQuickChatOptionChanged(option, quick_chat_type)) {
        return 0;
    }

    setFlag_QuickChatChange();
    if (quick_chat_type <= 1u) {
        std::memcpy(&quick_chat_[quick_chat_type], &option, sizeof(option));
        // As with SetEtcOption, preserve the original final-copy EAX value.
        return reinterpret_cast<uint16_t*>(
            reinterpret_cast<uint8_t*>(&quick_chat_[quick_chat_type]) +
            sizeof(option) - 2u);
    }
    return reinterpret_cast<uint16_t*>(this);
}

// IDA_EA: 0x84B6F46
// IDA_NAME: _ZN11CGameOption18IsEtcOptionChangedER10CETCOption
// DEMANGLED: CGameOption::IsEtcOptionChanged(CETCOption &)
int CGameOption::IsEtcOptionChanged(CETCOption& option) {
    return std::memcmp(&etc_option_, &option, sizeof(option)) != 0;
}

// IDA_EA: 0x84B6F84
// IDA_NAME: _ZN11CGameOption24IsQuickChatOptionChangedERK20CQuickChattingOptionj
// DEMANGLED: CGameOption::IsQuickChatOptionChanged(CQuickChattingOption const&,unsigned int)
int CGameOption::IsQuickChatOptionChanged(
    const CQuickChattingOption& option,
    unsigned int quick_chat_type) {
    return quick_chat_type <= 1u &&
           std::memcmp(&quick_chat_[quick_chat_type],
                       &option,
                       sizeof(option)) != 0;
}

// IDA_EA: 0x84B6FE0
// IDA_NAME: _ZN11CGameOption17CopyQuickChattingER20CQuickChattingOptionj
// DEMANGLED: CGameOption::CopyQuickChatting(CQuickChattingOption &,unsigned int)
void CGameOption::CopyQuickChatting(CQuickChattingOption& destination,
                                    unsigned int quick_chat_type) {
    if (quick_chat_type <= 1u) {
        std::memcpy(&destination,
                    &quick_chat_[quick_chat_type],
                    sizeof(destination));
    }
}

// IDA_EA: 0x84B7080
// IDA_NAME: _ZN11CGameOption15SetEmoticonInfoERK23CChattingEmoticonConfig
// DEMANGLED: CGameOption::SetEmoticonInfo(CChattingEmoticonConfig const&)
int CGameOption::SetEmoticonInfo(const CChattingEmoticonConfig& config) {
    if (std::memcmp(&emoticon_config_, &config, sizeof(config)) == 0) {
        return 0;
    }
    std::memcpy(&emoticon_config_, &config, sizeof(config));
    return 1;
}

// IDA_EA: 0x84B7136
// IDA_NAME: _ZNK11CGameOption15getEmoticonInfoEv
// DEMANGLED: CGameOption::getEmoticonInfo(void)const
CChattingEmoticonConfig CGameOption::getEmoticonInfo() const {
    return emoticon_config_;
}

// IDA_EA: 0x84B71BC
// IDA_NAME: _ZN11CGameOption7SetFlagEb
// DEMANGLED: CGameOption::SetFlag(bool)
CGameOption* CGameOption::SetFlag(bool changed) {
    emoticon_changed_ = changed;
    return this;
}

// IDA_EA: 0x84B71D8
// IDA_NAME: _ZN11CGameOption17isEmoticonChangedEv
// DEMANGLED: CGameOption::isEmoticonChanged(void)
int CGameOption::isEmoticonChanged() {
    return emoticon_changed_;
}

// IDA_EA: 0x84B71E8
// IDA_NAME: _ZN11CGameOption15isEqualEmoticonERK23CChattingEmoticonConfig
// DEMANGLED: CGameOption::isEqualEmoticon(CChattingEmoticonConfig const&)
bool CGameOption::isEqualEmoticon(
    const CChattingEmoticonConfig& config) {
    return std::memcmp(&emoticon_config_, &config, sizeof(config)) == 0;
}

// IDA_EA: 0x84B7246
// IDA_NAME: _ZN11CGameOption14get_etc_optionE15ENUM_OPTION_ETC
// DEMANGLED: CGameOption::get_etc_option(ENUM_OPTION_ETC)
int CGameOption::get_etc_option(int etc_option) {
    return etc_option_.getETCOption(etc_option);
}

// IDA_EA: 0x822EFF2
// IDA_NAME: _ZN11CGameOption23isChanged_QuickChatDataEv
// DEMANGLED: CGameOption::isChanged_QuickChatData(void)
int CGameOption::isChanged_QuickChatData() {
    return quick_chat_changed_;
}

// IDA_EA: 0x84B737C
// IDA_NAME: _ZN11CGameOption23setFlag_QuickChatChangeEv
// DEMANGLED: CGameOption::setFlag_QuickChatChange(void)
CGameOption* CGameOption::setFlag_QuickChatChange() {
    quick_chat_changed_ = 1;
    return this;
}
