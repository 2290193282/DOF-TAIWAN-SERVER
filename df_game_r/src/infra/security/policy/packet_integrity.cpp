#include "infra/security/policy/packet_integrity.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/security/crypto/md5_legacy.h"
#include "infra/security/policy/pi_sender_manager.h"

#include <cstring>
#include <utility>


uint32_t Encryption::crc_table_[256] = {};
bool Encryption::crc_table_initialized_ = false;

// IDA_EA: 0x808C8B5
// IDA_NAME: _Z13_NS_MakeHash_PcsRi
// DEMANGLED: _NS_MakeHash_(char *,short,int &)
int _NS_MakeHash_(char* src, int16_t n400, int& p_n603) {
    uint8_t p_s[24];

    if (n400 <= 0 || n400 > 400) {
        return 0;
    }
    md5(reinterpret_cast<uint8_t*>(src), n400, p_s);
    p_s[0] ^= p_s[8] ^ p_s[10] ^ p_s[13] ^ 0x81;
    p_s[1] ^= p_s[5] ^ p_s[12] ^ p_s[9] ^ 0x78;
    p_s[2] ^= p_s[14] ^ p_s[4] ^ p_s[6] ^ 0x1A;
    p_s[3] ^= p_s[11] ^ p_s[7] ^ p_s[15] ^ 0xBF;
    p_n603 = *reinterpret_cast<int*>(p_s);
    return 1;
}

// IDA_EA: 0x82746F3
// IDA_NAME: _Z20_NS_PI_MakeHash_NOTIP9PacketBuf
// DEMANGLED: _NS_PI_MakeHash_NOTI(PacketBuf*)
void _NS_PI_MakeHash_NOTI(PacketBuf* packet) {
    if (packet != 0) {
        int p_n603;
        char* packetEi = packet->get_packet(0);
        const int16_t n400 =
            static_cast<int16_t>(packet->get_index() - sizeof(SC_Header));
        if (static_cast<uint8_t>(_NS_MakeHash_(
                packetEi + sizeof(SC_Header), n400, p_n603)) == 1) {
            packet->put_int(p_n603);
        }
    }
}

// IDA_EA: 0x808CC17
// IDA_NAME: _Z30_NS_PI_2ND_MakeChecksumTo1BytePc
// DEMANGLED: _NS_PI_2ND_MakeChecksumTo1Byte(char*)
char* _NS_PI_2ND_MakeChecksumTo1Byte(char* a1) {
    char* result = a1;
    *a1 ^= a1[2] ^ a1[1] ^ a1[3] ^ 0x18;
    return result;
}

// IDA_EA: 0x808CC4B
// IDA_NAME: _Z31_NS_PI_2ND_MakeChecksumTo1Byte2RjS_
// DEMANGLED: _NS_PI_2ND_MakeChecksumTo1Byte2(unsigned int &,unsigned int &)
uint32_t& _NS_PI_2ND_MakeChecksumTo1Byte2(uint32_t& server_checksum,
                                          uint32_t& client_checksum) {
    server_checksum = static_cast<uint8_t>(server_checksum);
    client_checksum = static_cast<uint8_t>(client_checksum);
    return client_checksum;
}

// IDA_EA: 0x808CA60
// IDA_NAME: _Z39_NS_PI_2ND_ReconstructCheckSumRandomkeyRjj
// DEMANGLED: _NS_PI_2ND_ReconstructCheckSumRandomkey(unsigned int &,unsigned int)
uint32_t* _NS_PI_2ND_ReconstructCheckSumRandomkey(uint32_t& a1, uint32_t a2) {
    uint32_t* result = &a1;
    a1 = (a2 << 8) | static_cast<uint8_t>(a1);
    return result;
}

// IDA_EA: 0x82772A9
// IDA_NAME: _Z25_NS_PI_2ND_GetPacketIndexPc
// DEMANGLED: _NS_PI_2ND_GetPacketIndex(char *)
int _NS_PI_2ND_GetPacketIndex(char* packetEi) {
    uint16_t packet_id = 0;
    std::memcpy(&packet_id,
                packetEi + offsetof(CS_Header, type),
                sizeof(packet_id));
    return packet_id;
}

namespace {

int GetReceiveHashTypeClass1(uint16_t id) {
    // CLIENT_IDA_EVIDENCE: the 0627 function assigns hash types to command
    // semantics. 0828 wire IDs moved and gained new commands, so carrying
    // the old numeric ranges forward would classify unrelated packets.
    switch (id) {
    case ENUM_CMDPACKET_CHECK_CONNECTION: return 1;
    case ENUM_CMDPACKET_LOGIN: return 2;
    case ENUM_CMDPACKET_SET_UDP_IP_PORT: return 3;
    case ENUM_CMDPACKET_EXIT: return 4;
    case ENUM_CMDPACKET_SELECT_CHARACTER: return 5;
    case ENUM_CMDPACKET_CREATE_CHARACTER: return 6;
    case ENUM_CMDPACKET_DELETE_CHARACTER: return 7;
    case ENUM_CMDPACKET_RETURN_SELECT_CHARACTER: return 8;
    case ENUM_CMDPACKET_GET_USERINFO: return 9;
    case ENUM_CMDPACKET_RECOVER_STAMINA: return 10;
    case ENUM_CMDPACKET_REQUEST_PEER: return 11;
    case ENUM_CMDPACKET_RESPONSE_PEER: return 12;
    case ENUM_CMDPACKET_SET_PARTY_INFO: return 13;
    case ENUM_CMDPACKET_LEAVE_PARTY: return 14;
    case ENUM_CMDPACKET_WALKOUT_PARTY_MEMBER: return 15;
    // CLIENT_IDA_EVIDENCE: 0828 renamed 0627 START_GAME at the same wire ID;
    // the dispatcher and payload owner remain the ordinary dungeon-entry
    // path.
    case ENUM_CMDPACKET_ENTER_SELECT_DUNGEON: return 16;
    case ENUM_CMDPACKET_SELECT_DUNGEON: return 17;
    case ENUM_CMDPACKET_SEND_MESSAGE: return 18;
    case ENUM_CMDPACKET_DELETE_ITEM: return 19;
    case ENUM_CMDPACKET_MOVE_ITEMSPACE: return 20;
    case ENUM_CMDPACKET_SORT_ITEM: return 21;
    case ENUM_CMDPACKET_BUY_ITEM: return 22;
    case ENUM_CMDPACKET_SELL_ITEM: return 25;
    case ENUM_CMDPACKET_REPAIR_EQUIPMENT: return 26;
    case ENUM_CMDPACKET_SET_ITEMTRADE_STATE: return 27;
    case ENUM_CMDPACKET_COMPOUND_ITEM: return 28;
    case ENUM_CMDPACKET_DISJOINT_ITEM: return 29;
    case ENUM_CMDPACKET_USE_LOTTERY_ITEM: return 30;
    case ENUM_CMDPACKET_CHANGE_SKILLSLOT: return 31;
    case ENUM_CMDPACKET_BUY_SKILL: return 32;
    case ENUM_CMDPACKET_INCREASE_STATUS: return 33;
    case ENUM_CMDPACKET_ACCEPT_QUEST: return 34;
    case ENUM_CMDPACKET_GIVEUP_QUEST: return 35;
    case ENUM_CMDPACKET_SET_QUEST_TRIGGER: return 36;
    case ENUM_CMDPACKET_FINISH_QUEST: return 37;
    case ENUM_CMDPACKET_SET_USER_POSITION: return 38;
    case ENUM_CMDPACKET_SET_USER_AREA: return 39;
    case ENUM_CMDPACKET_FINISH_LOADING: return 41;
    case ENUM_CMDPACKET_USE_SKILL: return 42;
    case ENUM_CMDPACKET_DIE_MONSTER: return 43;
    case ENUM_CMDPACKET_DIE_CHARACTER: return 44;
    case ENUM_CMDPACKET_USE_COIN: return 45;
    case ENUM_CMDPACKET_GIVEUP_GAME: return 46;
    case ENUM_CMDPACKET_GET_ITEM: return 47;
    case ENUM_CMDPACKET_USE_STACKABLE: return 48;
    case ENUM_CMDPACKET_MOVE_MAP: return 49;
    case ENUM_CMDPACKET_SET_PLAY_RESULT: return 50;
    case ENUM_CMDPACKET_DROP_ITEM: return 51;
    case ENUM_CMDPACKET_DECREASE_DURABILITY: return 52;
    case ENUM_CMDPACKET_REPORT_BAD_P2P_USER: return 53;
    case ENUM_CMDPACKET_MAKE_PVP_ROOM: return 54;
    case ENUM_CMDPACKET_ENTER_PVP_ROOM: return 55;
    case ENUM_CMDPACKET_SET_PVP_SEAT_STATE: return 56;
    case ENUM_CMDPACKET_SET_PVP_READY_STATE: return 57;
    case ENUM_CMDPACKET_SET_PVP_TEAM_MODE: return 58;
    case ENUM_CMDPACKET_DIE_PVP_CHARACTER: return 59;
    case ENUM_CMDPACKET_PVP_TIME_OUT: return 60;
    case ENUM_CMDPACKET_END_PVP_RESULT: return 61;
    case ENUM_CMDPACKET_RES_PVP_RANK: return 62;
    case ENUM_CMDPACKET_SET_PVP_MAP_INDEX: return 63;
    case ENUM_CMDPACKET_ADD_FRIEND: return 64;
    case ENUM_CMDPACKET_REMOVE_FRIEND: return 65;
    case ENUM_CMDPACKET_DEBUG_COMMAND: return 66;
    case ENUM_CMDPACKET_CERA: return 67;
    case ENUM_CMDPACKET_BUY_CERASHOP_ITEM: return 68;
    case ENUM_CMDPACKET_GEN_CERATICKET: return 69;
    case ENUM_CMDPACKET_REQUEST_PVPEXP_OF_WEEK: return 70;
    case ENUM_CMDPACKET_GUILD_MEMER_LIST: return 71;
    case ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT: return 72;
    case ENUM_CMDPACKET_SCORE_SCROLL_STATE: return 73;
    case ENUM_CMDPACKET_CARD_SELECT_RIGHT_STATE: return 74;
    case ENUM_CMDPACKET_SELECT_CARD: return 75;
    case ENUM_CMDPACKET_EPLP_COMMAND: return 76;
    case ENUM_CMDPACKET_CALL_GUILD_LEVEL_UP: return 77;
    case ENUM_CMDPACKET_GUILD_INFO: return 78;
    case ENUM_CMDPACKET_REQUEST_GUILD_ENTER: return 79;
    case ENUM_CMDPACKET_REQUEST_MEMBER_ENTER: return 80;
    case ENUM_CMDPACKET_MEMBER_ENTER_REPLY: return 81;
    case ENUM_CMDPACKET_MEMBER_SECEDE: return 82;
    case ENUM_CMDPACKET_CALL_MEMER_LIST: return 83;
    case ENUM_CMDPACKET_UPGRADE_ITEM: return 84;
    case ENUM_CMDPACKET_RESET_ITEM_ATTR: return 85;
    case ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM: return 86;
    case ENUM_CMDPACKET_ENTER_PRIVATE_STORE: return 87;
    case ENUM_CMDPACKET_EXIT_PRIVATE_STORE: return 88;
    case ENUM_CMDPACKET_CREATE_PRIVATE_STORE: return 89;
    case ENUM_CMDPACKET_REMOVE_PRIVATE_STORE: return 90;
    case ENUM_CMDPACKET_COMPLETE_DISPLAY: return 91;
    case ENUM_CMDPACKET_MOVE_TO_GATE: return 92;
    case ENUM_CMDPACKET_MAKE_WARROOM_TEMP: return 93;
    case ENUM_CMDPACKET_ENTER_WARROOM: return 94;
    case ENUM_CMDPACKET_SET_WARROOM_SEAT_STATE: return 95;
    case ENUM_CMDPACKET_DIE_WARROOM_CHARACTER: return 96;
    case ENUM_CMDPACKET_START_WARROOM_TEMP: return 97;
    case ENUM_CMDPACKET_MAILBOX_SEND: return 98;
    case ENUM_CMDPACKET_MAILBOX_EXTRACT_ITEM: return 99;
    case ENUM_CMDPACKET_MAILBOX_OPEN: return 100;
    case ENUM_CMDPACKET_PEER_CONNECT_RESULT: return 101;
    case ENUM_CMDPACKET_QUICK_JOIN_ROOM: return 102;
    case ENUM_CMDPACKET_COMPOUND_AVATAR: return 103;
    case ENUM_CMDPACKET_RENAME_CREATURE: return 104;
    case ENUM_CMDPACKET_RESPONSE_CREATURE: return 105;
    case ENUM_CMDPACKET_HATCH_CREATURE: return 106;
    case ENUM_CMDPACKET_BUY_AUTOMAT_ITEM: return 107;
    case ENUM_CMDPACKET_GATHERING_PARTY_STATUS: return 113;
    case ENUM_CMDPACKET_GM_COMMAND: return 122;
    case ENUM_CMDPACKET_REPORT_4_HACK: return 123;
    case ENUM_CMDPACKET_GUILD_WAR_INFO: return 124;
    case ENUM_CMDPACKET_PVP_HEART_BEAT: return 125;
    case ENUM_CMDPACKET_CODE_CHECKSUM: return 126;
    case ENUM_CMDPACKET_PVP_REQUEST_FIGHT: return 127;
    case ENUM_CMDPACKET_MOUSEREGISTER: return 128;
    case ENUM_CMDPACKET_CREATURE_SEND_MESSAGE: return 129;
    case ENUM_CMDPACKET_TRACE_ERROR: return 130;
    case ENUM_CMDPACKET_OTHER_USER_INFO: return 131;
    case ENUM_CMDPACKET_BOSS_DIE_CHECK: return 132;
    case ENUM_CMDPACKET_REGISITER_TO_BLACKLIST: return 133;
    case ENUM_CMDPACKET_DELETE_TO_BLACKLIST: return 134;
    case ENUM_CMDPACKET_REQUEST_BLACKLIST: return 135;
    case ENUM_CMDPACKET_CHANGE_HOST: return 136;
    case ENUM_CMDPACKET_CREATURE_SCRIPT_MESSAGE: return 137;
    case ENUM_CMDPACKET_CHARACTER_STATISTIC: return 138;
    case ENUM_CMDPACKET_GUILDMEMBER_NAMING: return 140;
    case ENUM_CMDPACKET_SET_SUB_GUILD_MASTER: return 141;
    case ENUM_CMDPACKET_EXCHANGE_SERVER_INFO: return 142;
    case ENUM_CMDPACKET_EXCHANGE_SERVER_INFO_RET: return 143;
    case ENUM_CMDPACKET_EXCHANGE_SERVER_CHARAC_INFO: return 144;
    case ENUM_CMDPACKET_EXCHANGE_SERVER_CHARAC_INFO_RET: return 145;
    case ENUM_CMDPACKET_TIME_CHECK: return 146;
    case ENUM_CMDPACKET_BACK_2_VILLAGE: return 147;
    case ENUM_CMDPACKET_CHANGE_LETTER_STAT: return 149;
    case ENUM_CMDPACKET_CHANGE_CHARAC_NAME: return 150;
    case ENUM_CMDPACKET_QUERY_CHARAC_INFO: return 151;
    case ENUM_CMDPACKET_REPORT_MANNERLESS_USER: return 152;
    case ENUM_CMDPACKET_ALLDIE_MONSTER: return 153;
    case ENUM_CMDPACKET_GUILD_MEMER_LIST_NEXT: return 154;
    case ENUM_CMDPACKET_GUILD_ALL_MEMBER_LIST: return 155;
    case ENUM_CMDPACKET_GUILD_ALL_MEMBER_LIST_NEXT: return 156;
    case ENUM_CMDPACKET_RPY_HUMAN_CERTIFY: return 157;
    case ENUM_CMDPACKET_CHANGE_TUTORIAL_FLAG: return 158;
    case ENUM_CMDPACKET_DIE_AI_CHARACTER: return 159;
    case ENUM_CMDPACKET_COMPLETE_LOAD_ASSAULT: return 161;
    case ENUM_CMDPACKET_CONNECT_P2P_ASSAULT: return 162;
    case ENUM_CMDPACKET_DIE_ASSAULT_PLAYER: return 163;
    case ENUM_CMDPACKET_REVIVAL_ASSAULT_PLAYER: return 164;
    case ENUM_CMDPACKET_CHANGE_HP: return 165;
    case ENUM_CMDPACKET_BVHACKINFO: return 166;
    case ENUM_CMDPACKET_CALL_GUILD_INVITE: return 167;
    case ENUM_CMDPACKET_REPLY_GUILD_INVITE: return 168;
    case ENUM_CMDPACKET_REQ_GUILD_SECEDE: return 169;
    case ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD: return 170;
    case ENUM_CMDPACKET_GUILD_MASTER_DELEGATE: return 171;
    case ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE: return 172;
    case ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE: return 173;
    case ENUM_CMDPACKET_OPEN_GUILD_CREATE_WINDOW: return 174;
    case ENUM_CMDPACKET_DEATH_TOWER_STAGE_CMD: return 175;
    case ENUM_CMDPACKET_USE_BOOSTER_ITEM: return 176;
    case ENUM_CMDPACKET_SECURITY_CARD_ISSUE: return 177;
    case ENUM_CMDPACKET_SECURITY_CARD_DISUSE: return 178;
    case ENUM_CMDPACKET_SECURITY_CARD_AUTH_REQ: return 179;
    case ENUM_CMDPACKET_SECURITY_CARD_AUTH_RPY: return 180;
    case ENUM_CMDPACKET_SECURITY_CARD_CERT_KEY: return 181;
    case ENUM_CMDPACKET_CALL_PARTY_MEMBER_REALTIME_INFO: return 182;
    case ENUM_CMDPACKET_EVADE_ASSAULT: return 183;
    case ENUM_CMDPACKET_AGREE_ENCHANT: return 184;
    case ENUM_CMDPACKET_TRY_ENCHANT: return 185;
    case ENUM_CMDPACKET_PUT_ITEM_FOR_ENCHANT: return 186;
    case ENUM_CMDPACKET_CLIENT_SPEC_STATISTIC: return 187;
    case ENUM_CMDPACKET_SECURITY_CARD_AUTH_CANCEL: return 188;
    case ENUM_CMDPACKET_GMDEBUG_COMMAND: return 192;
    case ENUM_CMDPACKET_JOIN_POWER: return 193;
    case ENUM_CMDPACKET_SECEDE_POWER: return 194;
    case ENUM_CMDPACKET_CHANGE_GUILD_NAME: return 195;
    case ENUM_CMDPACKET_SDC_DAMAGE_CHECK: return 196;
    case ENUM_CMDPACKET_SDC_ACTIVESTATUS_CHECK: return 197;
    case ENUM_CMDPACKET_AUCTION_ASK_AVERAGE_PRICE: return 198;
    case ENUM_CMDPACKET_AUCTION_REGIST_ITEM: return 199;
    case ENUM_CMDPACKET_AUCTION_REGIST_CANCEL: return 200;
    case ENUM_CMDPACKET_AUCTION_BIDDING: return 201;
    case ENUM_CMDPACKET_AUCTION_SEARCH_BY_ITEMKEY: return 202;
    case ENUM_CMDPACKET_AUCTION_SEARCH_BY_NOITEMKEY: return 203;
    case ENUM_CMDPACKET_AUCTION_MY_REGISTED_ITEM_INFO: return 204;
    case ENUM_CMDPACKET_AUCTION_MY_BIDDING_INFO: return 205;
    case ENUM_CMDPACKET_AUCTION_MY_AUCTION_HISTORY: return 206;
    case ENUM_CMDPACKET_DUNGEON_EVENT_STORY_PAUSE: return 209;
    case ENUM_CMDPACKET_JOIN_POWER_WAR: return 210;
    case ENUM_CMDPACKET_GOBLIN_PAD_STATUS: return 211;
    case ENUM_CMDPACKET_FRAME_LAG_STATISTICS: return 212;
    case ENUM_CMDPACKET_PVP_CHANNEL_INFO: return 213;
    case ENUM_CMDPACKET_REQUEST_MATCH: return 214;
    case ENUM_CMDPACKET_SAVE_GAME_OPTION_1: return 215;
    case ENUM_CMDPACKET_SAVE_GAME_OPTION_2: return 216;
    case ENUM_CMDPACKET_SECURITY_CARD_RETRANSFER: return 217;
    case ENUM_CMDPACKET_USE_EMBLEM: return 219;
    case ENUM_CMDPACKET_DISJOINT_AVATAR: return 220;
    case ENUM_CMDPACKET_PURIFY_ITEM: return 222;
    case ENUM_CMDPACKET_INVEST_ITEM_AMPLIFY_OPTION: return 223;
    case ENUM_CMDPACKET_ADD_AVATAR_SOCKET: return 224;
    case ENUM_CMDPACKET_SHOP_COIN_EVENT: return 225;
    case ENUM_CMDPACKET_USE_RANDOMBOX_ITEM: return 226;
    case ENUM_CMDPACKET_UDP_CHARACTERISTIC: return 227;
    case ENUM_CMDPACKET_ONEDAY_LETHE: return 228;
    case ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_LIST: return 231;
    case ENUM_CMDPACKET_REQUEST_PC_ROOM_PLAYER_COUNT: return 232;
    case ENUM_CMDPACKET_USE_VENDING_MACHINE: return 233;
    case ENUM_CMDPACKET_ASSERT_INFO: return 234;
    case ENUM_CMDPACKET_OVERFLOW_INFO: return 235;
    case ENUM_CMDPACKET_SERVER_MESSAGE_SEND: return 236;
    case ENUM_CMDPACKET_SERVER_MESSAGE_CHECK: return 237;
    case ENUM_CMDPACKET_GIVE_GIFT_TO_NPC: return 240;
    case ENUM_CMDPACKET_GOBLIN_PAD_RESPONSE_CRYPT_KEY: return 241;
    case ENUM_CMDPACKET_WRITE_GUILD_MEMBER_MEMO: return 242;
    case ENUM_CMDPACKET_CHECK_CREATE_GUILD_AGIT: return 245;
    case ENUM_CMDPACKET_CREATE_GUILD_AGIT: return 246;
    case ENUM_CMDPACKET_DELETE_GUILD_AGIT: return 247;
    case ENUM_CMDPACKET_POWER_WAR_INFO: return 248;
    case ENUM_CMDPACKET_UPGRADE_GUILD_AGIT: return 249;
    case ENUM_CMDPACKET_POWER_WAR_PROCESS_INFO: return 251;
    case ENUM_CMDPACKET_CREATE_DISJOINT_STORE: return 253;
    case ENUM_CMDPACKET_REQUEST_DISJOINT_ITEM: return 254;
    case ENUM_CMDPACKET_REPAIR_DISJOINT_MACHINE: return 255;
    case ENUM_CMDPACKET_TELEPORT: return 256;
    case ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB: return 257;
    case ENUM_CMDPACKET_GIVEUP_EXPERT_JOB: return 258;
    case ENUM_CMDPACKET_UPGRADE_DISJOINT_MACHINE: return 259;
    case ENUM_CMDPACKET_ENTER_DISJOINT_STORE: return 260;
    case ENUM_CMDPACKET_CLOSE_DISJOINT_STORE: return 261;
    case ENUM_CMDPACKET_REPORT_ABUSE_USER: return 262;
    case ENUM_CMDPACKET_LOAD_COMPLETE_AFTER_ASSAULT: return 263;
    case ENUM_CMDPACKET_CONNECT_P2P_AFTER_ASSAULT: return 264;
    case ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM: return 266;
    case ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM: return 267;
    case ENUM_CMDPACKET_GUILD_CARGO_MOVE_ITEM: return 268;
    case ENUM_CMDPACKET_LODING_TIME_REPORT: return 269;
    case ENUM_CMDPACKET_USE_SHARED_EFFECT_ITEM: return 270;
    case ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM: return 271;
    case ENUM_CMDPACKET_ADD_HACKTYPE_CNT: return 272;
    case ENUM_CMDPACKET_CHANGE_EMOTION: return 273;
    case ENUM_CMDPACKET_DIE_BLOOD_MONSTER: return 274;
    case ENUM_CMDPACKET_COMPOUND_EMBLEM: return 275;
    case ENUM_CMDPACKET_MOTION_RESULT: return 276;
    case ENUM_CMDPACKET_BLOOD_ROUND_UI_PREPARE_FINISH_: return 277;
    case ENUM_CMDPACKET_CHANGE_ANOTHER_SKILL_TREE: return 279;
    case ENUM_CMDPACKET_GUILD_CARGO: return 280;
    case ENUM_CMDPACKET_GUILD_CARGO_HISTORY: return 281;
    case ENUM_CMDPACKET_FIGHT_VILLAGE_MONSTER: return 282;
    case ENUM_CMDPACKET_FINISH_VILLAGE_MONSTER_FIGHTING: return 283;
    case ENUM_CMDPACKET_UPGRADE_GUILD_CARGO: return 284;
    case ENUM_CMDPACKET_REQUEST_ITEM_LOCK: return 286;
    case ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK: return 287;
    case ENUM_CMDPACKET_REQUEST_ITEM_UNLOCK_CANCEL: return 288;
    case ENUM_CMDPACKET_UPGRADE_CHRONICLE: return 289;
    case ENUM_CMDPACKET_ENCHANT_BY_BEAD: return 290;
    case ENUM_CMDPACKET_DUNGEON_NPC_BUFF_INFO: return 291;
    case ENUM_CMDPACKET_LAG_STATISTICS: return 297;
    case ENUM_CMDPACKET_HT_PS: return 298;
    case ENUM_CMDPACKET_HT_IS: return 299;
    case ENUM_CMDPACKET_PI: return 301;
    case ENUM_CMDPACKET_VERIFY_GOLD: return 302;
    case ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD: return 303;
    case ENUM_CMDPACKET_REQUEST_ADD_PVP_BUDDY: return 304;
    case ENUM_CMDPACKET_RESPONSE_ADD_PVP_BUDDY: return 305;
    case ENUM_CMDPACKET_REMOVE_PVP_BUDDY: return 306;
    case ENUM_CMDPACKET_PVP_BUDDY_CONN_LIST: return 307;
    case ENUM_CMDPACKET_NCC: return 309;
    case ENUM_CMDPACKET_MI: return 310;
    case ENUM_CMDPACKET_CHANGE_CHARAC_SLOT: return 311;
    case ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM: return 312;
    case ENUM_CMDPACKET_SECRET_SHOP_OPEN_CLOSE: return 313;
    case ENUM_CMDPACKET_COMPLETE_LOAD_PVP: return 314;
    case ENUM_CMDPACKET_CONNECT_P2P_PVP: return 315;
    case ENUM_CMDPACKET_BIDDING_ROUTING_ITEM: return 316;
    case ENUM_CMDPACKET_CREATE_ACCOUNT_CARGO: return 322;
    case ENUM_CMDPACKET_UPGRADE_ACCOUNT_CARGO: return 323;
    case ENUM_CMDPACKET_DEPOSIT_MONEY: return 324;
    case ENUM_CMDPACKET_WITHDRAW_MONEY: return 325;
    case ENUM_CMDPACKET_REDEEM_LIST: return 326;
    case ENUM_CMDPACKET_REDEEM: return 327;
    case ENUM_CMDPACKET_SECU_DATA_CONTROL: return 328;
    case ENUM_CMDPACKET_CONNECT_LINK_CHARAC: return 329;
    case ENUM_CMDPACKET_DISCONNECT_LINK_CHARAC: return 330;
    case ENUM_CMDPACKET_CHANGE_CHARAC_LINK_TYPE: return 331;
    case ENUM_CMDPACKET_MULTI_MAILBOX_SEND: return 332;
    case ENUM_CMDPACKET_OPERATE_RIDABLE_OBJECT: return 333;
    case ENUM_CMDPACKET_SELECT_ULTIMATE_DIFFICULTY: return 334;
    case ENUM_CMDPACKET_PIV: return 335;
    case ENUM_CMDPACKET_PID: return 336;
    case ENUM_CMDPACKET_QUERY_CHARAC_INFO_MAILBOX: return 337;
    case ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE: return 338;
    case ENUM_CMDPACKET_CHANGE_PARTY_POSITION: return 341;
    case ENUM_CMDPACKET_ONE_TO_ONE_CHAT_STATE: return 342;
    case ENUM_CMDPACKET_FIND_CHAR_NAME_USE_CHARAC_NO: return 343;
    case ENUM_CMDPACKET_SKILL_COMMAND_CUSTOMIZING: return 344;
    case ENUM_CMDPACKET_SKILL_COMMAND_ALL_DEFAULT: return 345;
    case ENUM_CMDPACKET_HACK_SCRIPT_HASH: return 346;
    case ENUM_CMDPACKET_AUCTION_BUY_ITEM_APIECE: return 347;
    case ENUM_CMDPACKET_CHANGE_PARTY_MEMBER_POSITION: return 348;
    case ENUM_CMDPACKET_SCAN_BOT_BY_DLL: return 350;
    case ENUM_CMDPACKET_USE_LIMIT_CUBE: return 351;
    case ENUM_CMDPACKET_REFRESH_GUILD_INFO: return 352;
    case ENUM_CMDPACKET_OPEN_GUILD_BOARD: return 353;
    case ENUM_CMDPACKET_WRITE_ON_THE_GUILDBOARD: return 354;
    case ENUM_CMDPACKET_DELETE_GUILDBOARD_TEXT: return 355;
    case ENUM_CMDPACKET_REFRESH_GUILDBOARD: return 356;
    case ENUM_CMDPACKET_DONATE_GUILD_FUND: return 359;
    case ENUM_CMDPACKET_CHECK_JOIN_GUILD: return 360;
    case ENUM_CMDPACKET_REQUEST_JOIN_GUILD: return 361;
    case ENUM_CMDPACKET_CANCEL_JOIN_GUILD: return 362;
    case ENUM_CMDPACKET_APPROVE_JOIN_GUILD: return 363;
    case ENUM_CMDPACKET_DENY_JOIN_GUILD: return 364;
    case ENUM_CMDPACKET_GUILD_JOIN_LIST: return 365;
    case ENUM_CMDPACKET_GUILD_ATTENDANCE_INFO: return 368;
    case ENUM_CMDPACKET_DUNGEON_MOTION: return 370;
    case ENUM_CMDPACKET_COMPOUND_EQUIPMENT_UPGRADE_CARD: return 373;
    case ENUM_CMDPACKET_ANTIBOT: return 374;
    case ENUM_CMDPACKET_DPROTO: return 375;
    case ENUM_CMDPACKET_DPROTO_CALLBACK: return 376;
    case ENUM_CMDPACKET_END: return 378;
    default: return 0;
    }
}

int GetReceiveHashTypeClass0(uint16_t id) {
    // CLIENT_IDA_EVIDENCE: 0828 renamed 0627 START_GAME at ID 27 to
    // ENTER_SELECT_DUNGEON, so the unchanged low-ID rule intentionally
    // preserves its old hash type.
    if (id <= ENUM_NOTIPACKET_REVIVAL_CREATURE)
        return static_cast<int>(id) + 379;
    if ((id >= ENUM_NOTIPACKET_EVENT_INFO &&
         id <= ENUM_NOTIPACKET_BOSS_DIE_CHECK) ||
        (id >= ENUM_NOTIPACKET_GUILD_WAR_START &&
         id <= ENUM_NOTIPACKET_PARTY_MEMBER_REALTIME_INFO) ||
        (id >= ENUM_NOTIPACKET_TOTAL_POWER_WAR_POINT &&
         id <= ENUM_NOTIPACKET_POWER_WAR_PROLONG))
        return static_cast<int>(id) + 391;
    if (id >= ENUM_NOTIPACKET_DUNGEON_EVENT_STORY_PAUSE &&
        id <= ENUM_NOTIPACKET_LOAD_COOLTIME_ITEM_INFO)
        return static_cast<int>(id) + 392;
    if ((id >= ENUM_NOTIPACKET_GAME_OPTION &&
         id <= ENUM_NOTIPACKET_AUCTION_NOTIFY_AUCTION_SERVICE) ||
        (id >= ENUM_NOTIPACKET_POWER_WAR_WINNER_SIDE &&
         id <= ENUM_NOTIPACKET_SERVER_MESSAGE_INFO) ||
        (id >= ENUM_NOTIPACKET_DISCONNECT_REASON &&
         id <= ENUM_NOTIPACKET_REQUEST_DISJOINT_ITEM) ||
        (id >= ENUM_NOTIPACKET_EXPERT_JOB_INFO &&
         id <= ENUM_NOTIPACKET_POWER_WAR_POWER_UP_INFO) ||
        (id >= ENUM_NOTIPACKET_POWER_WAR_RESULT &&
         id <= ENUM_NOTIPACKET_REVIVAL_CHARAC_TIMEOUT) ||
        (id >= ENUM_NOTIPACKET_BLACKLIST_PROBLEM_CHILD_TIME &&
         id <= ENUM_NOTIPACKET_TAKE_SCREEN_SHOT) ||
        (id >= ENUM_NOTIPACKET_CREATE_VILLAGE_MONSTER &&
         id <= ENUM_NOTIPACKET_ADD_EFFECT_ITEM_INFO) ||
        (id >= ENUM_NOTIPACKET_ITEM_EFFECT &&
         id <= ENUM_NOTIPACKET_BREAK_AWAY_REWARD) ||
        (id >= ENUM_NOTIPACKET_SECRET_SHOP_NPC &&
         id <= ENUM_NOTIPACKET_ROUTING_RESULT) ||
        (id >= ENUM_NOTIPACKET_REDEEM_LIST &&
         id <= ENUM_NOTIPACKET_MEMBER_BONUS_FATIGUE_LIMIT) ||
        (id >= ENUM_NOTIPACKET_PREMIUM_GOLD_CARD &&
         id <= ENUM_NOTIPACKET_RETURN_USER))
        return static_cast<int>(id) + 393;
    switch (id) {
    case ENUM_NOTIPACKET_AA_STARTINFO: return 556;
    case ENUM_NOTIPACKET_AA_TIME: return 557;
    case ENUM_NOTIPACKET_HAS_NO_GO_WITH_APC_QUEST_IN_PARTY: return 559;
    case ENUM_NOTIPACKET_AUCTION_ALARM: return 560;
    case ENUM_NOTIPACKET_PCROOM_PLAYER_LOGIN_LOGOUT: return 579;
    case ENUM_NOTIPACKET_NPC_FAVOR: return 587;
    case ENUM_NOTIPACKET_NPC_MOOD: return 588;
    case ENUM_NOTIPACKET_GUILD_MEMBER_MEMO: return 590;
    case ENUM_NOTIPACKET_ASK_LEAVE_PARTY: return 623;
    case ENUM_NOTIPACKET_POWER_WAR_USER_JOIN_INFO: return 624;
    case ENUM_NOTIPACKET_REQUEST_EXIT: return 626;
    case ENUM_NOTIPACKET_LIMIT_ITEM_USAGE: return 680;
    case ENUM_NOTIPACKET_TODAY_GUILD_MEMBER: return 697;
    case ENUM_NOTIPACKET_JOIN_GUILD_INFO: return 698;
    case ENUM_NOTIPACKET_ACHIEVE_GUILD_ATTENDANCE: return 701;
    // CLIENT_IDA_EVIDENCE: these three moved from 560..562 to the 0828
    // semantic tail. Keeping the old numeric range would instead classify
    // unrelated 0828 NOTIs.
    case ENUM_NOTIPACKET_ANTIBOT: return 702;
    case ENUM_NOTIPACKET_DPROTO: return 703;
    case ENUM_NOTIPACKET_END: return 704;
    default: return 0;
    }
}

}  // namespace

// IDA_EA: 0x8275AF3
// IDA_NAME: _Z26_NS_PI_2ND_GetHackTypeEnumhs
int _NS_PI_2ND_GetHackTypeEnum(uint8_t packet_class, int16_t packet_id) {
    const uint16_t id = static_cast<uint16_t>(packet_id);
    if (packet_class == 1u) return GetReceiveHashTypeClass1(id);
    if (packet_class == 0u) return GetReceiveHashTypeClass0(id);
    return 0;
}

// IDA_EA: 0x8277359
// IDA_NAME: _Z22_NS_PI_2ND_GetHackTypejht
int _NS_PI_2ND_GetHackType(uint32_t extra,
                            uint8_t packet_class,
                            uint16_t packet_id) {
    const uint32_t hack_type = extra + 2u * static_cast<uint32_t>(
        _NS_PI_2ND_GetHackTypeEnum(packet_class,
                                   static_cast<int16_t>(packet_id)) + 1000);
    return hack_type <= 0xE0Eu ? static_cast<int>(hack_type) : 3599;
}

// IDA_EA: 0x8274F73
// IDA_NAME: _Z22_NS_PI_2ND_RecvHash_CSPcP5CUserj
int _NS_PI_2ND_RecvHash_CS(char* packet,
                            CUser* user,
                            uint32_t packet_length) {
    if (!packet || !user || packet_length < sizeof(CS_Header)) return 0;
    const CS_Header* const c2s_header =
        reinterpret_cast<const CS_Header*>(packet);
    const uint8_t packet_class = c2s_header->cmd;
    const uint16_t packet_id = c2s_header->type;
    const std::pair<unsigned char, unsigned short> key(packet_class,
                                                        packet_id);
    const bool already_seen = user->GetCSHashSet()->find(key) !=
                              user->GetCSHashSet()->end();
    const bool hash_ok = user->GetPIReceiverManager()->CheckHashReceived(
        reinterpret_cast<unsigned char*>(packet + sizeof(CS_Header)),
        packet_length - sizeof(CS_Header), c2s_header->checksum >> 8,
        packet_class == 1u && packet_id == 1u);
    if (hash_ok || already_seen) return 0;

    const int hack_type = _NS_PI_2ND_GetHackType(0, packet_class, packet_id);
    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type), 1u, 0u, 0u);
    user->lock();
    user->GetCSHashSet()->insert(key);
    user->unlock();
    return 1;
}

// IDA_EA: 0x82751EE
// IDA_NAME: _Z22_NS_PI_2ND_SendHash_SCPctP5CUser
// DEMANGLED: _NS_PI_2ND_SendHash_SC(char *,unsigned short,CUser *)
uint32_t* _NS_PI_2ND_SendHash_SC(char* packet,
                                  uint16_t length,
                                  CUser* user) {
    SC_Header* const sc_header = reinterpret_cast<SC_Header*>(packet);
    const char packet_class = static_cast<char>(sc_header->cmd);
    const short packet_index = static_cast<short>(sc_header->type);
    const bool reset = packet_class == 0 && packet_index == 1;
    PISenderManager* manager = user->GetPISenderManager();
    const int hash_for_sending =
        manager->MakeHashForSending(
            reinterpret_cast<unsigned char*>(packet + sizeof(SC_Header)),
            static_cast<unsigned int>(
                static_cast<unsigned short>(length) - sizeof(SC_Header)),
            reset);
    return _NS_PI_2ND_ReconstructCheckSumRandomkey(
        sc_header->checksum2,
        static_cast<uint32_t>(hash_for_sending));
}

// IDA_EA: 0x848D3E4
// IDA_NAME: _ZN10Encryption14make_crc_tableEv
// DEMANGLED: Encryption::make_crc_table(void)
bool Encryption::make_crc_table() {
    uint32_t value = 1;
    crc_table_[0] = 0;
    for (uint32_t bit = 128; bit != 0; bit >>= 1) {
        uint32_t poly = 0;
        if ((value & 1) != 0) {
            // IDA 0x848D415: 1303941417 (0x4DB89129), not the
            // superficially similar 0x4DB88369.
            poly = 0x4DB89129u;
        }
        value = (value >> 1) ^ poly;
        for (uint32_t index = 0; index <= 0xFF; index += 2 * bit) {
            crc_table_[index + bit] = value ^ crc_table_[index];
        }
    }
    crc_table_initialized_ = true;
    return false;
}

// IDA_EA: 0x848D472
// IDA_NAME: _ZN10Encryption5crc32EjPKcj
// DEMANGLED: Encryption::crc32(unsigned int,char const*,unsigned int)
uint32_t Encryption::crc32(uint32_t crc, const char* buf, unsigned int len) {
    if (!crc_table_initialized_) {
        make_crc_table();
    }
    uint32_t value = ~crc;
    for (unsigned int index = 0; index < len; ++index) {
        value = (value >> 8) ^
                crc_table_[static_cast<unsigned char>(value ^ buf[index])];
    }
    return ~value;
}

// IDA_EA: 0x848D4C8
// IDA_NAME: _ZN10Encryption6crc32NEPvi
// DEMANGLED: Encryption::crc32N(void*,int)
uint32_t Encryption::crc32N(void* payload, int payload_size) {
    uint32_t crc = crc32(0, 0, 0);
    crc = crc32(crc, static_cast<const char*>(payload),
                static_cast<unsigned int>(payload_size));
    _NS_PI_2ND_MakeChecksumTo1Byte(reinterpret_cast<char*>(&crc));
    return crc;
}

// IDA_EA: 0x848D51A
// IDA_NAME: _ZN10Encryption8SeedRandERj
// DEMANGLED: Encryption::SeedRand(unsigned int&)
int Encryption::SeedRand(uint32_t& seed) {
    const uint32_t first = 1103515245u * seed + 12345u;
    const uint32_t upper_first = (first >> 16) & 0x7FFu;
    const uint32_t second = 1103515245u * first + 12345u;
    const uint32_t third = 1103515245u * second + 12345u;
    seed = third;
    return static_cast<int>(((third >> 16) & 0x3FFu) ^
                            ((((second >> 16) & 0x3FFu) ^
                              (upper_first << 10))
                             << 10));
}

// IDA_EA: 0x848DA70
// IDA_NAME: _ZN10Encryption7EncryptEPviRj
// DEMANGLED: Encryption::Encrypt(void*,int,unsigned int&)
void Encryption::Encrypt(void* data, int size, uint32_t& seed) {
    uint32_t random_value = 71646901u;
    if (seed != 0) {
        random_value = static_cast<uint32_t>(SeedRand(seed));
    }

    const unsigned char rotate_count =
        static_cast<unsigned char>((random_value >> 8) & 7u);
    unsigned char* bytes = static_cast<unsigned char*>(data);
    for (int index = 0; index < size; ++index) {
        bytes[index] ^= static_cast<unsigned char>(random_value);
        bytes[index] = static_cast<unsigned char>(
            (static_cast<unsigned int>(bytes[index]) >> rotate_count) |
            (static_cast<unsigned int>(bytes[index]) << (8 - rotate_count)));
    }
}
