#ifndef OLDSRV_GLOBAL_CHATTING_EMOTICON_H_
#define OLDSRV_GLOBAL_CHATTING_EMOTICON_H_

#include <stdint.h>

#include <map>
#include <set>
#include <vector>

#include "old_data/PVF/data/emoticon_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"


class CUser;

// IDA_EA: CChattingEmoticonList @0x837E418/0x837E1AA; Load_list @0x80E53A8;
// initChattingEmoticonScriptList @0x88E30F8.  CGameOption's typed ABI can be
// seen first, so share the exact declarations rather than redeclaring them.
#ifndef OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED
#define OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED

// IDA unit: 99_misc_global/CChattingEmoticonConfigRow. Old i386 size: 0x16.
struct CChattingEmoticonConfigRow {
    CChattingEmoticonConfigRow();
    ~CChattingEmoticonConfigRow();

    CChattingEmoticonConfigRow* clear();

    char text[20];
    uint16_t icon_id;
} __attribute__((packed));

// IDA unit: 99_misc_global/CChattingEmoticonConfig. Old i386 size: 0x210.
struct CChattingEmoticonConfig {
    CChattingEmoticonConfig();

    bool clear();

    CChattingEmoticonConfigRow rows[24];
};

#endif  // OLDSRV_GLOBAL_CHATTING_EMOTICON_CONFIG_TYPES_DEFINED

class ChattingEmoticonBase {
public:
    ChattingEmoticonBase();
    virtual ~ChattingEmoticonBase();

    virtual int isUsable(const CUser& user) const = 0;
    virtual int GetStartArrayIndex() const = 0;
    virtual int GetEndArrayIndex() const = 0;
    virtual int AddPremiumIndex(int premium_index);
    virtual int isCommonEmoticon(const CUser& user) const;

    int GetIndex() const;
    ChattingEmoticonBase* SetIndex(int index);

protected:
    int32_t index_;
    std::set<int> premium_indices_;
};

class ChattingEmoticon_byPremium : public ChattingEmoticonBase {
public:
    ChattingEmoticon_byPremium();
    virtual ~ChattingEmoticon_byPremium();

    virtual int isUsable(const CUser& user) const;
    virtual int GetStartArrayIndex() const;
    virtual int GetEndArrayIndex() const;
};

class ChattingEmoticon_byLevel : public ChattingEmoticonBase {
public:
    explicit ChattingEmoticon_byLevel(int level);
    virtual ~ChattingEmoticon_byLevel();

    virtual int isUsable(const CUser& user) const;
    virtual int GetStartArrayIndex() const;
    virtual int GetEndArrayIndex() const;
    virtual int AddPremiumIndex(int premium_index);
    virtual int isCommonEmoticon(const CUser& user) const;

private:
    int32_t level_;
};

class ChattingEmoticon_byEvent : public ChattingEmoticonBase {
public:
    ChattingEmoticon_byEvent();
    virtual ~ChattingEmoticon_byEvent();

    virtual int isUsable(const CUser& user) const;
    virtual int GetStartArrayIndex() const;
    virtual int GetEndArrayIndex() const;
};

class CChattingEmoticonList {
public:
    CChattingEmoticonList();
    ~CChattingEmoticonList();

    ChattingEmoticonBase* Factory(
        const EmoticonType::EmoticonScript& script);
    int Load_list();
    ChattingEmoticonBase* find_emoticon(int index);
    int ReCalc(CChattingEmoticonConfig& config, const CUser& user);

private:
    typedef std::map<int, ChattingEmoticonBase*> EmoticonByIndexMap;
    typedef std::map<int, std::vector<ChattingEmoticonBase*> >
        EmoticonByTypeMap;

    EmoticonByIndexMap by_index_;
    EmoticonByTypeMap by_type_;
    std::map<int, int> reserved_;

    CChattingEmoticonList(const CChattingEmoticonList&);
    CChattingEmoticonList& operator=(const CChattingEmoticonList&);
};

extern char g_ChattingEmoticonScriptBaseDirectory[256];
extern STScriptFileList g_ChattingEmoticonScriptFileList;

int initChattingEmoticonScriptList(char* base_dir, char* filename);
int ImportChattingEmoticon(EmoticonType::EmoticonScript& script,
                           const char* filename);

static_assert(sizeof(CChattingEmoticonConfigRow) == 0x16u,
              "CChattingEmoticonConfigRow IDA ABI size must stay 0x16");
static_assert(sizeof(CChattingEmoticonConfig) == 0x210u,
              "CChattingEmoticonConfig IDA ABI size must stay 0x210");
static_assert(sizeof(ChattingEmoticonBase) == 0x20u,
              "ChattingEmoticonBase IDA ABI size must stay 0x20");
static_assert(sizeof(ChattingEmoticon_byPremium) == 0x20u,
              "ChattingEmoticon_byPremium IDA ABI size must stay 0x20");
static_assert(sizeof(ChattingEmoticon_byLevel) == 0x24u,
              "ChattingEmoticon_byLevel IDA ABI size must stay 0x24");
static_assert(sizeof(CChattingEmoticonList) == 0x48u,
              "CChattingEmoticonList IDA ABI size must stay 0x48");


#endif  // OLDSRV_GLOBAL_CHATTING_EMOTICON_H_
