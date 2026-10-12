#include "old_game/economy/mail/mail_trade_runtime.h"

#include <algorithm>

namespace WongWork {

struct CMailBoxLayoutVerifier {
    static_assert(offsetof(CMailBox, loaded) == 0x00,
                  "CMailBox::loaded old ABI offset");
    static_assert(offsetof(CMailBox, last_load_idx) == 0x04,
                  "CMailBox::last_load_idx old ABI offset");
    static_assert(offsetof(CMailBox, not_loaded_mail_count) == 0x08,
                  "CMailBox::not_loaded_mail_count old ABI offset");
    static_assert(offsetof(CMailBox, mail_load_count) == 0x0c,
                  "CMailBox::mail_load_count old ABI offset");
    static_assert(offsetof(CMailBox, letter_keep_count) == 0x64,
                  "CMailBox::letter_keep_count old ABI offset");
    static_assert(offsetof(CMailBox, last_load_letter_idx) == 0x7c,
                  "CMailBox::last_load_letter_idx old ABI offset");
    static_assert(offsetof(CMailBox, loaded_letter_count) == 0x80,
                  "CMailBox::loaded_letter_count old ABI offset");
    static_assert(offsetof(CMailBox, package_load_lack) == 0x84,
                  "CMailBox::package_load_lack old ABI offset");
};

// IDA_EA: 0x823454C
// IDA_NAME: _ZN8WongWork8CMailBox20GetLoadedLetterCountEv
// DEMANGLED: WongWork::CMailBox::GetLoadedLetterCount(void)
int CMailBox::GetLoadedLetterCount() {
    return loaded_letter_count;
}

// IDA_EA: 0x823455A
// IDA_NAME: _ZN8WongWork8CMailBox21GetNotLoadedMailCountEv
// DEMANGLED: WongWork::CMailBox::GetNotLoadedMailCount(void)
int CMailBox::GetNotLoadedMailCount() {
    return not_loaded_mail_count;
}

// IDA_EA: 0x84ED216
// IDA_NAME: _ZNK8WongWork8CMailBox16getMailLoadCountEv
// DEMANGLED: WongWork::CMailBox::getMailLoadCount(void)const
int CMailBox::getMailLoadCount() const {
    return mail_load_count;
}

// IDA_EA: 0x84ED222
// IDA_NAME: _ZN8WongWork8CMailBox21InsertLetterKeepCountEj
// DEMANGLED: WongWork::CMailBox::InsertLetterKeepCount(unsigned int)
int CMailBox::InsertLetterKeepCount(unsigned int letter_id_value) {
    return letter_keep_count.insert(std::make_pair(letter_id_value, true))
        .second;
}

// IDA_EA: 0x84ED280
// IDA_NAME: _ZN8WongWork8CMailBox21DeleteLetterKeepCountEj
// DEMANGLED: WongWork::CMailBox::DeleteLetterKeepCount(unsigned int)
int CMailBox::DeleteLetterKeepCount(unsigned int letter_id_value) {
    std::map<int, bool>::iterator found =
        letter_keep_count.find(letter_id_value);
    if (found == letter_keep_count.end()) {
        return 0;
    }
    letter_keep_count.erase(found);
    return 1;
}

// IDA_EA: 0x84ED2FC
// IDA_NAME: _ZN8WongWork8CMailBox20DecLoadedLetterCountEv
// DEMANGLED: WongWork::CMailBox::DecLoadedLetterCount(void)
CMailBox* CMailBox::DecLoadedLetterCount() {
    --loaded_letter_count;
    if (loaded_letter_count < 0) {
        loaded_letter_count = 0;
    }
    return this;
}

// IDA_EA: 0x84ED330
// IDA_NAME: _ZN8WongWork8CMailBox21IncNotLoadedMailCountEv
// DEMANGLED: WongWork::CMailBox::IncNotLoadedMailCount(void)
CMailBox* CMailBox::IncNotLoadedMailCount() {
    ++not_loaded_mail_count;
    return this;
}

// IDA_EA: 0x8551F4A
// IDA_NAME: _ZN8WongWork8CMailBox5resetEv
// DEMANGLED: WongWork::CMailBox::reset(void)
CMailBox* CMailBox::reset() {
    mail_load_count = 0;
    return this;
}

// IDA_EA: 0x85575DC
// IDA_NAME: _ZN8WongWork8CMailBox16incMailLoadCountEv
// DEMANGLED: WongWork::CMailBox::incMailLoadCount(void)
CMailBox* CMailBox::incMailLoadCount() {
    ++mail_load_count;
    if (static_cast<unsigned int>(mail_load_count) > 0x7FFFFFFEu) {
        mail_load_count = 0;
    }
    return this;
}

// IDA_EA: 0x8557608
// IDA_NAME: _ZN8WongWork8CMailBox18GetPackageLoadLackEPjj
// DEMANGLED: WongWork::CMailBox::GetPackageLoadLack(unsigned int *,unsigned int)
bool CMailBox::GetPackageLoadLack(unsigned int* out, unsigned int count) {
    unsigned int index = 0;
    while (index < count && package_load_lack.size() > index) {
        out[index] = package_load_lack[index];
        ++index;
    }
    return index < count && package_load_lack.size() > index;
}

// IDA_EA: 0x8557676
// IDA_NAME: _ZN8WongWork8CMailBox18SetPackageLoadLackEPKjiRSt3setIjSt4lessIjESaIjEE
// DEMANGLED: WongWork::CMailBox::SetPackageLoadLack(unsigned int const*,int,std::set<unsigned int,std::less<unsigned int>,std::allocator<unsigned int>> &)
bool CMailBox::SetPackageLoadLack(const unsigned int* values,
                                  int count,
                                  std::set<unsigned int>& changed) {
    for (std::vector<unsigned int>::iterator it = package_load_lack.begin();
         it != package_load_lack.end(); ++it) {
        changed.insert(*it);
    }

    package_load_lack.clear();
    int index = 0;
    while (index < count) {
        package_load_lack.push_back(values[index]);
        std::set<unsigned int>::iterator found = changed.find(values[index]);
        if (found != changed.end()) {
            changed.erase(found);
        }
        ++index;
    }
    return index < count;
}

// IDA_EA: 0x85577AC
// IDA_NAME: _ZN8WongWork8CMailBox19FindPackageLoadLackEj
// DEMANGLED: WongWork::CMailBox::FindPackageLoadLack(unsigned int)
bool CMailBox::FindPackageLoadLack(unsigned int mail_idx_value) {
    if (!mail_idx_value) {
        return false;
    }
    return std::find(package_load_lack.begin(),
                     package_load_lack.end(),
                     mail_idx_value) != package_load_lack.end();
}

// IDA_EA: 0x855784C
// IDA_NAME: _ZN8WongWork8CMailBox20GetLastLoadLetterIdxEv
// DEMANGLED: WongWork::CMailBox::GetLastLoadLetterIdx(void)
int CMailBox::GetLastLoadLetterIdx() {
    return last_load_letter_idx;
}

// IDA_EA: 0x8557858
// IDA_NAME: _ZN8WongWork8CMailBox20SetLastLoadLetterIdxEj
// DEMANGLED: WongWork::CMailBox::SetLastLoadLetterIdx(unsigned int)
CMailBox* CMailBox::SetLastLoadLetterIdx(unsigned int letter_idx) {
    last_load_letter_idx = letter_idx;
    return this;
}

// IDA_EA: 0x8557866
// IDA_NAME: _ZN8WongWork8CMailBox20ClearLetterKeepCountEv
// DEMANGLED: WongWork::CMailBox::ClearLetterKeepCount(void)
CMailBox* CMailBox::ClearLetterKeepCount() {
    letter_keep_count.clear();
    last_load_letter_idx = 0;
    return this;
}

// IDA_EA: 0x8557886
// IDA_NAME: _ZN8WongWork8CMailBox18GetLetterKeepCountEv
// DEMANGLED: WongWork::CMailBox::GetLetterKeepCount(void)
int CMailBox::GetLetterKeepCount() {
    return letter_keep_count.size();
}

// IDA_EA: 0x855789C
// IDA_NAME: _ZN8WongWork8CMailBox20SetLoadedLetterCountEi
// DEMANGLED: WongWork::CMailBox::SetLoadedLetterCount(int)
CMailBox* CMailBox::SetLoadedLetterCount(int count) {
    loaded_letter_count = count;
    return this;
}

// IDA_EA: 0x8557914
// IDA_NAME: _ZN8WongWork8CMailBox21SetNotLoadedMailCountEi
// DEMANGLED: WongWork::CMailBox::SetNotLoadedMailCount(int)
CMailBox* CMailBox::SetNotLoadedMailCount(int count) {
    not_loaded_mail_count = count;
    return this;
}

// IDA_EA: 0x8557938
// IDA_NAME: _ZN8WongWork8CMailBox8IsLoadedEv
// DEMANGLED: WongWork::CMailBox::IsLoaded(void)
int CMailBox::IsLoaded() {
    return loaded;
}

// IDA_EA: 0x8557944
// IDA_NAME: _ZN8WongWork8CMailBox14GetLastLoadIdxEv
// DEMANGLED: WongWork::CMailBox::GetLastLoadIdx(void)
int CMailBox::GetLastLoadIdx() {
    return last_load_idx;
}

// IDA_EA: 0x8557950
// IDA_NAME: _ZN8WongWork8CMailBox12SetLoadStateEbl
// DEMANGLED: WongWork::CMailBox::SetLoadState(bool,long)
CMailBox* CMailBox::SetLoadState(bool loaded_value, long last_load_idx_value) {
    loaded = loaded_value;
    if (loaded_value) {
        last_load_idx = last_load_idx_value;
    }
    return this;
}

}  // namespace WongWork
