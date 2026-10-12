#include "old_data/channel/packet_dispatch/channel_packets.h"

#include <cstring>

typedef char Packet_ChannelType_Size_Must_Be_0x12[
    (sizeof(Packet_ChannelType) == 0x12) ? 1 : -1];
typedef char Packet_PvPChannelInfo_Size_Must_Be_0x17F8[
    (sizeof(Packet_PvPChannelInfo) == 0x17F8) ? 1 : -1];
typedef char Packet_Monitor_Other_Channel_Chat_Size_Must_Be_0x13B[
    (sizeof(Packet_Monitor_Other_Channel_Chat) == 0x13B) ? 1 : -1];
typedef char Packet_Monitor_Other_Channel_Chat_Hyper_Link_Size_Must_Be_0x274[
    (sizeof(Packet_Monitor_Other_Channel_Chat_Hyper_Link) == 0x274) ? 1 : -1];
typedef char Packet_Monitor_Guild_Chat_Hyper_Link_Size_Must_Be_0x24C[
    (sizeof(Packet_Monitor_Guild_Chat_Hyper_Link) == 0x24C) ? 1 : -1];
typedef char Packet_Monitor_Member_Chat_Hyper_Link_Size_Must_Be_0x248[
    (sizeof(Packet_Monitor_Member_Chat_Hyper_Link) == 0x248) ? 1 : -1];
typedef char Packet_Monitor_MegaPhone_Hyper_Link_Size_Must_Be_0x267[
    (sizeof(Packet_Monitor_MegaPhone_Hyper_Link) == 0x267) ? 1 : -1];

// IDA_EA: 0x82A4088
// IDA_NAME: _ZN18Packet_ChannelTypeC2Ev
// DEMANGLED: Packet_ChannelType::Packet_ChannelType(void)
Packet_ChannelType::Packet_ChannelType() : header(0xFA4, 0x12) {}

// IDA_EA: 0x822B8E6
// IDA_NAME: _ZN33Packet_Monitor_Other_Channel_ChatC2Ev
// DEMANGLED: Packet_Monitor_Other_Channel_Chat::Packet_Monitor_Other_Channel_Chat(void)
Packet_Monitor_Other_Channel_Chat::Packet_Monitor_Other_Channel_Chat()
    : header(0x3F3, 0x13B),
      account_id(0),
      sender_marker(0),
      charac_no(0),
      item_position(0),
      target_name_length(0),
      message_length(0) {
    std::memset(target_name, 0, sizeof(target_name));
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1904
// IDA_NAME: _ZN25Packet_Monitor_Guild_ChatC2Ev
// DEMANGLED: Packet_Monitor_Guild_Chat::Packet_Monitor_Guild_Chat(void)
Packet_Monitor_Guild_Chat::Packet_Monitor_Guild_Chat()
    : header(0x400, 0x113),
      guild_key(0),
      charac_no(0),
      message_length(0) {
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1A48
// IDA_NAME: _ZN26Packet_Monitor_Member_ChatC2Ev
// DEMANGLED: Packet_Monitor_Member_Chat::Packet_Monitor_Member_Chat(void)
Packet_Monitor_Member_Chat::Packet_Monitor_Member_Chat()
    : header(0x4BF, 0x10F),
      member_key(0),
      message_length(0) {
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1960
// IDA_NAME: _ZN24Packet_Monitor_MegaPhoneC2Ev
// DEMANGLED: Packet_Monitor_MegaPhone::Packet_Monitor_MegaPhone(void)
Packet_Monitor_MegaPhone::Packet_Monitor_MegaPhone()
    : header(0x546, 0x12E),
      reserved_0a(0),
      channel_no(0),
      unique_id(0),
      chat_type(0),
      message_length(0) {
    charac_name[0] = '\0';
    message[0] = '\0';
}

// IDA_EA: 0x86D19B0
// IDA_NAME: _ZN24Packet_Monitor_MegaPhone3setEhtPKchS1_h
// DEMANGLED: Packet_Monitor_MegaPhone::set(unsigned char,unsigned short,char const*,unsigned char,char const*,unsigned char)
Packet_Monitor_MegaPhone* Packet_Monitor_MegaPhone::set(
    unsigned char channel,
    uint16_t user_unique_id,
    const char* source_name,
    unsigned char source_message_length,
    const char* source_message,
    unsigned char type) {
    channel_no = channel;
    unique_id = user_unique_id;
    chat_type = type;
    std::strncpy(charac_name, source_name, sizeof(charac_name));
    std::strncpy(message, source_message, sizeof(message));
    message_length = source_message_length;
    header.size = static_cast<uint16_t>(source_message_length + 46u);
    return this;
}

// IDA_EA: 0x822C3E6
// IDA_NAME: _ZN44Packet_Monitor_Other_Channel_Chat_Hyper_LinkC2Ev
// DEMANGLED: Packet_Monitor_Other_Channel_Chat_Hyper_Link::Packet_Monitor_Other_Channel_Chat_Hyper_Link(void)
Packet_Monitor_Other_Channel_Chat_Hyper_Link::
    Packet_Monitor_Other_Channel_Chat_Hyper_Link()
    : header(0x2719, 0x274) {
    // The original C2 deliberately leaves chat_type (+0x0a) untouched.
    account_id = 0;
    sender_marker = 0;
    charac_no = 0;
    item_position = 0;
    target_name_length = 0;
    hyperlink_count = 0;
    message_length = 0;
    std::memset(target_name, 0, sizeof(target_name));
    std::memset(hyperlinks, 0, sizeof(hyperlinks));
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1A9A
// IDA_NAME: _ZN36Packet_Monitor_Guild_Chat_Hyper_LinkC2Ev
// DEMANGLED: Packet_Monitor_Guild_Chat_Hyper_Link::Packet_Monitor_Guild_Chat_Hyper_Link(void)
Packet_Monitor_Guild_Chat_Hyper_Link::
    Packet_Monitor_Guild_Chat_Hyper_Link()
    : header(0x2718, 0x24C),
      guild_key(0),
      charac_no(0),
      hyperlink_count(0),
      message_length(0) {
    std::memset(hyperlinks, 0, sizeof(hyperlinks));
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1C9C
// IDA_NAME: _ZN37Packet_Monitor_Member_Chat_Hyper_LinkC2Ev
// DEMANGLED: Packet_Monitor_Member_Chat_Hyper_Link::Packet_Monitor_Member_Chat_Hyper_Link(void)
Packet_Monitor_Member_Chat_Hyper_Link::
    Packet_Monitor_Member_Chat_Hyper_Link()
    : header(0x271A, 0x248),
      member_key(0),
      hyperlink_count(0),
      message_length(0) {
    std::memset(hyperlinks, 0, sizeof(hyperlinks));
    std::memset(message, 0, sizeof(message));
}

// IDA_EA: 0x86D1B20
// IDA_NAME: _ZN35Packet_Monitor_MegaPhone_Hyper_LinkC2Ev
// DEMANGLED: Packet_Monitor_MegaPhone_Hyper_Link::Packet_Monitor_MegaPhone_Hyper_Link(void)
Packet_Monitor_MegaPhone_Hyper_Link::
    Packet_Monitor_MegaPhone_Hyper_Link()
    : header(0x271B, 0x267),
      reserved_0a(0),
      channel_no(0),
      unique_id(0),
      hyperlink_count(0),
      message_length(0) {
    // IDA only clears the first character/message byte and the first
    // hyperlink record in C2.  set() fills the current dynamic record count.
    charac_name[0] = '\0';
    message[0] = '\0';
    std::memset(hyperlinks[0], 0, sizeof(hyperlinks[0]));
}

// IDA_EA: 0x86D1B9A
// IDA_NAME: _ZN35Packet_Monitor_MegaPhone_Hyper_Link3setEhtPKchS1_hhPK19hyperlink_item_info
// DEMANGLED: Packet_Monitor_MegaPhone_Hyper_Link::set(unsigned char,unsigned short,char const*,unsigned char,char const*,unsigned char,unsigned char,hyperlink_item_info const*)
Packet_Monitor_MegaPhone_Hyper_Link*
Packet_Monitor_MegaPhone_Hyper_Link::set(
    unsigned char source_channel_no,
    uint16_t source_unique_id,
    const char* source_charac_name,
    unsigned char source_message_length,
    const char* source_message,
    unsigned char source_chat_type,
    unsigned char source_hyperlink_count,
    const unsigned char source_hyperlinks[3][104]) {
    channel_no = source_channel_no;
    unique_id = source_unique_id;
    chat_type = source_chat_type;
    std::strncpy(charac_name, source_charac_name, sizeof(charac_name));
    std::strncpy(message, source_message, sizeof(message));
    message_length = source_message_length;
    hyperlink_count = source_hyperlink_count;
    for (int i = 0; i < source_hyperlink_count; ++i) {
        std::memcpy(hyperlinks[i], source_hyperlinks[i], sizeof(hyperlinks[i]));
    }
    header.size = static_cast<uint16_t>(source_message_length + 359u);
    return this;
}
