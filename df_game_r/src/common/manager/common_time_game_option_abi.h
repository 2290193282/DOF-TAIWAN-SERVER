#ifndef OLDSRV_GLOBAL_COMMON_TIME_GAME_OPTION_ABI_H_
#define OLDSRV_GLOBAL_COMMON_TIME_GAME_OPTION_ABI_H_

#include "common/config.h"

#include <stddef.h>
#include <stdint.h>
#include <ctime>

struct CommonTime {
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char week_day;
    unsigned char raw_06[2];
    int32_t create_time;
    int32_t current_time;

    // IDA_EA: 0x822D0E6
    // IDA_NAME: _ZN10CommonTime13SetCreateTimeEl
    // DEMANGLED: CommonTime::SetCreateTime(long)
    CommonTime* SetCreateTime(long timer);
    // IDA_EA: 0x86947D8
    // IDA_NAME: _ZN10CommonTime5clearEv
    // DEMANGLED: CommonTime::clear(void)
    CommonTime* clear();
    CommonTime* SetCurTime();
    int GetCreateTime(char* destination) const;
    CommonTime* SetCreateTime(char* source);
    // IDA_EA: 0x822D16E
    // IDA_NAME: _ZNK10CommonTime8MakeTimeER2tm
    // DEMANGLED: CommonTime::MakeTime(tm &)const
    tm* MakeTime(tm& output) const;
    // IDA_EA: 0x822D1DC
    // IDA_NAME: _ZNK10CommonTimecvlEv
    // DEMANGLED: CommonTime::operator long(void)const
    operator long() const;
};

static_assert(sizeof(CommonTime) == 0x10u,
              "CommonTime IDA ABI size must stay 0x10");

// IDA units: CHotkeyOption, CETCOption, CQuickChattingOption,
// CChattingEmoticonConfigRow, CChattingEmoticonConfig, and CGameOption.
// These are direct CUser children (CGameOption at CUser+0x6fd64), so their
// lifecycle cannot remain an opaque pointer/offset convention.
struct CHotkeyOption {
    // The 0828 client serializes I01..I90, C00, J00, J01 and C01.
    // 0x500 covers their complete signed-int16 representation (1034 bytes)
    // and leaves bounded headroom without turning the packet into an
    // unbounded string protocol.
    enum { kCapacity = 0x500 };

    CHotkeyOption();
    bool clear();
    bool isEmpty() const;
    unsigned int getTextLength() const;
    bool isValidText(unsigned int text_length) const;

    // 140828 CMD198 serializes the hotkey table as canonical UTF-8 text:
    //     <database key>\t<decimal key code>\n
    // The wire carries only the declared text length.  The remaining bytes
    // are zero-filled; a full-capacity value deliberately has no terminator.
    char bytes_[kCapacity];
};

struct CETCOption {
    CETCOption();
    bool clear();
    int getETCOption(int etc_option) const;
    int setETCOption(int etc_option, short value);

    int16_t values_[oldsrv::config::kGameEtcOptionCount];  // +0x00, 0x4c bytes
};

struct CQuickChattingOption {
    CQuickChattingOption();
    void* clear();

    uint8_t bytes_[0x12c];
};

#ifndef OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED
#define OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED

// Kept in lockstep with global/chatting_emoticon.h.  The expanded CGameOption
// places this packed row at an odd address (+0x76f).
struct CChattingEmoticonConfigRow {
    CChattingEmoticonConfigRow();
    ~CChattingEmoticonConfigRow();
    CChattingEmoticonConfigRow* clear();

    char text[20];
    uint16_t icon_id;  // +0x14
} __attribute__((packed));

struct CChattingEmoticonConfig {
    CChattingEmoticonConfig();
    bool clear();

    CChattingEmoticonConfigRow rows[24];
};

#endif  // OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED

struct CGameOption {
    // IDA_EA: 0x84B6C58
    // IDA_NAME: _ZN11CGameOptionC2Ev
    // DEMANGLED: CGameOption::CGameOption(void)
    CGameOption();

    int set_etc_option(int etc_option, short value);
    int get_etc_option(int etc_option);
    char* get_hotkey_option(unsigned char key_type);
    unsigned int get_hotkey_option_length(unsigned char key_type) const;
    bool reset_load_hotkey_flag();
    int get_load_hotkey_flag(unsigned char key_type);
    int isChanged_QuickChatData();
    CGameOption* resetFlag_QuickChatChnage();
    CGameOption* setFlag_QuickChatChange();

    int SetHotKeyOption(unsigned char key_type, CHotkeyOption& option);
    int IsHotKeyOptionChanged(unsigned char key_type,
                              CHotkeyOption& option);
    uint16_t* SetEtcOption(CETCOption& option);
    uint16_t* SetQuickChatOption(const CQuickChattingOption& option,
                                 unsigned int quick_chat_type);
    int IsEtcOptionChanged(CETCOption& option);
    int IsQuickChatOptionChanged(const CQuickChattingOption& option,
                                 unsigned int quick_chat_type);
    void CopyQuickChatting(CQuickChattingOption& destination,
                           unsigned int quick_chat_type);
    int SetEmoticonInfo(const CChattingEmoticonConfig& config);
    CChattingEmoticonConfig getEmoticonInfo() const;
    CGameOption* SetFlag(bool changed);
    int isEmoticonChanged();
    bool isEqualEmoticon(const CChattingEmoticonConfig& config);

    CHotkeyOption hotkeys_[2];                     // +0x000
    uint8_t hotkey_load_flags_[2];                 // +0xa00
    CETCOption etc_option_;                        // +0xa02
    CQuickChattingOption quick_chat_[2];           // +0xa4e
    uint8_t quick_chat_changed_;                   // +0xca6
    CChattingEmoticonConfig emoticon_config_;      // +0xca7
    uint8_t emoticon_changed_;                     // +0xeb7
};

#if defined(__i386__)
static_assert(sizeof(CHotkeyOption) == 0x500u,
              "140828 hotkey text capacity must stay 1280 bytes");
static_assert(sizeof(CETCOption) == 0x4cu,
              "CETCOption must preserve the 140828 i386 extent");
static_assert(sizeof(CQuickChattingOption) == 0x12cu,
              "CQuickChattingOption must preserve the old i386 extent");
static_assert(sizeof(CChattingEmoticonConfigRow) == 0x16u,
              "CChattingEmoticonConfigRow must preserve the old i386 extent");
static_assert(sizeof(CChattingEmoticonConfig) == 0x210u,
              "CChattingEmoticonConfig must preserve the old i386 extent");
static_assert(sizeof(CGameOption) == 0xeb8u,
              "expanded CGameOption i386 extent must stay 0xeb8");
static_assert(offsetof(CGameOption, hotkeys_) == 0x000u,
              "expanded CGameOption hotkey storage offset drift");
static_assert(offsetof(CGameOption, hotkey_load_flags_) == 0xa00u,
              "expanded CGameOption hotkey flag offset drift");
static_assert(offsetof(CGameOption, etc_option_) == 0xa02u,
              "expanded CGameOption ETC offset drift");
static_assert(offsetof(CGameOption, quick_chat_) == 0xa4eu,
              "expanded CGameOption quick-chat offset drift");
static_assert(offsetof(CGameOption, quick_chat_changed_) == 0xca6u,
              "expanded CGameOption quick-chat flag offset drift");
static_assert(offsetof(CGameOption, emoticon_config_) == 0xca7u,
              "expanded CGameOption emoticon offset drift");
static_assert(offsetof(CGameOption, emoticon_changed_) == 0xeb7u,
              "expanded CGameOption emoticon flag offset drift");
#endif

long getDailyInitializeTime(int tm_hour, int tm_min, int tm_sec)
    __asm__("_Z22getDailyInitializeTimeiii");
tm* __GetCurrentDNFDate(tm& out_time)
    __asm__("_Z19__GetCurrentDNFDateR2tm");

#endif  // OLDSRV_GLOBAL_COMMON_TIME_GAME_OPTION_ABI_H_
