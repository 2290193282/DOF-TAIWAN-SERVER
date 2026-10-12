#include "old_data/PVF/data/data_manager.h"

#include <cstring>

#include "common/manager/chatting_emoticon.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x835FBA2
// IDA_NAME: _ZN12CDataManager21rewriteUsableEmoticonERK5CUserPc
// DEMANGLED: CDataManager::rewriteUsableEmoticon(CUser const&,char *)
void CDataManager::rewriteUsableEmoticon(const CUser& user, char* message) {
    if (!message) {
        return;
    }

    const std::size_t message_length = std::strlen(message);
    CChattingEmoticonList* const list = chatting_emoticon_list_;
    for (std::size_t index = 0; index + 2 < message_length; ++index) {
        if (message[index] != 0x1e || message[index + 1] != 0x20) {
            continue;
        }

        // The old body reaches the initialized list at CDataManager+0x28 and
        // masks only the third byte when the ID is absent or unusable.
        ChattingEmoticonBase* const emoticon =
            list->find_emoticon(static_cast<signed char>(message[index + 2]));
        if (!emoticon || emoticon->isUsable(user) != 1) {
            message[index + 2] = static_cast<char>(0xff);
        }
    }
}

// IDA_EA: 0x835FC9A
// IDA_NAME: _ZN12CDataManager17IsUsableEmoticonsERK5CUserRKSt6vectorIiSaIiEE
// DEMANGLED: CDataManager::IsUsableEmoticons(
//             CUser const&,std::vector<int> const&)
std::vector<int> CDataManager::IsUsableEmoticons(
    const CUser& user, const std::vector<int>& emoticons) {
    std::vector<int> usable_emoticons;
    CChattingEmoticonList* const list = chatting_emoticon_list_;

    if (!list) {
        return usable_emoticons;
    }

    for (std::vector<int>::const_iterator it = emoticons.begin();
         it != emoticons.end(); ++it) {
        ChattingEmoticonBase* const emoticon = list->find_emoticon(*it);
        if (emoticon && emoticon->isUsable(user) == 1) {
            usable_emoticons.push_back(*it);
        }
    }
    return usable_emoticons;
}
