#include "old_game/economy/mail/mailbox_public_abi.h"

#include <algorithm>
#include <cstring>

// IDA_EA: 0x845543E
// IDA_NAME: _ZN15SIG_LETTER_INFOD2Ev
// DEMANGLED: SIG_LETTER_INFO::~SIG_LETTER_INFO()
SIG_LETTER_INFO::~SIG_LETTER_INFO() {}

// IDA_EA: 0x841CC6A
// IDA_NAME: _Z16GetOnlyMailCountP16SIG_MAILBOX_LIST
// DEMANGLED: GetOnlyMailCount(SIG_MAILBOX_LIST *)
int GetOnlyMailCount(SIG_MAILBOX_LIST* list) {
    int count = 0;
    for (int i = 0; list->response.mail_count > i; ++i) {
        if (!list->response.mails[i].letter_id) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x841CCB6
// IDA_NAME: _Z17IsContainedLetterP16SIG_MAILBOX_LISTi
// DEMANGLED: IsContainedLetter(SIG_MAILBOX_LIST *,int)
int IsContainedLetter(SIG_MAILBOX_LIST* list, int letter_id) {
    for (int i = 0; list->response.mail_count > i; ++i) {
        if (static_cast<int32_t>(list->response.mails[i].letter_id) ==
            letter_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x841CD03
// IDA_NAME: _Z24RemoveMailByLatestLetterP16SIG_MAILBOX_LISTi
// DEMANGLED: RemoveMailByLatestLetter(SIG_MAILBOX_LIST *,int)
int RemoveMailByLatestLetter(SIG_MAILBOX_LIST* list, int mail_slot) {
    if (list->response.mail_count - 1 != mail_slot) {
        for (int i = mail_slot; list->response.mail_count - 1 > i; ++i) {
            list->response.mails[i] = list->response.mails[i + 1];
        }
    }
    --list->response.mail_count;
    ++list->response.not_loaded_mail_count;
    return 1;
}

// IDA_EA: 0x841CE06
// IDA_NAME: _Z21RemoveMailByMailIndexP16SIG_MAILBOX_LISTi
// DEMANGLED: RemoveMailByMailIndex(SIG_MAILBOX_LIST *,int)
int RemoveMailByMailIndex(SIG_MAILBOX_LIST* list, int mail_index) {
    int slot = 0;
    char found = 0;
    for (int i = 0; list->response.mail_count > i; ++i) {
        if (static_cast<int32_t>(list->response.mails[i].postal_id) ==
            mail_index) {
            found = 1;
            slot = i;
            break;
        }
    }
    if (found != 1) {
        return 0;
    }
    for (int i = slot; list->response.mail_count - 1 > i; ++i) {
        list->response.mails[i] = list->response.mails[i + 1];
    }
    --list->response.mail_count;
    ++list->response.not_loaded_mail_count;
    return 1;
}

// IDA_EA: 0x841CF5A
// IDA_NAME: _Z19FindLastOccTimeMailP16SIG_MAILBOX_LISTi
// DEMANGLED: FindLastOccTimeMail(SIG_MAILBOX_LIST *,int)
int FindLastOccTimeMail(SIG_MAILBOX_LIST* list, int occ_time) {
    int latest_occ_time = 0;
    for (int i = 0; list->response.mail_count > i; ++i) {
        const int candidate = list->response.mails[i].expire_time - 1296000;
        if (candidate > occ_time && latest_occ_time < candidate) {
            latest_occ_time = candidate;
        }
    }

    int slot = -1;
    unsigned int latest_mail_index = 0;
    for (int i = 0; list->response.mail_count > i; ++i) {
        if (list->response.mails[i].expire_time - 1296000 ==
                latest_occ_time &&
            list->response.mails[i].postal_id > latest_mail_index) {
            latest_mail_index = list->response.mails[i].postal_id;
            slot = i;
        }
    }
    return slot;
}

// IDA_EA: 0x841D032
// IDA_NAME: _Z13CompareLetterRK15SIG_LETTER_INFOS1_
// DEMANGLED: CompareLetter(SIG_LETTER_INFO const&,SIG_LETTER_INFO const&)
bool CompareLetter(const SIG_LETTER_INFO& lhs, const SIG_LETTER_INFO& rhs) {
    if (lhs.reg_time < rhs.reg_time) {
        return true;
    }
    if (lhs.reg_time == rhs.reg_time) {
        return lhs.letter_id < rhs.letter_id;
    }
    return false;
}

// IDA_EA: 0x841D080
// IDA_NAME: _Z23FindLastLetterIndexMailP16SIG_MAILBOX_LISTi
// DEMANGLED: FindLastLetterIndexMail(SIG_MAILBOX_LIST *,int)
int FindLastLetterIndexMail(SIG_MAILBOX_LIST* list, int letter_id) {
    int latest_letter_id = 0;
    for (int i = 0; list->response.mail_count > i; ++i) {
        const int candidate =
            static_cast<int32_t>(list->response.mails[i].letter_id);
        if (candidate > letter_id && latest_letter_id < candidate) {
            latest_letter_id = candidate;
        }
    }

    int slot = -1;
    if (latest_letter_id) {
        for (int i = 0; list->response.mail_count > i; ++i) {
            if (static_cast<int32_t>(list->response.mails[i].letter_id) ==
                latest_letter_id) {
                return i;
            }
        }
    }
    return slot;
}

// IDA_EA: 0x841D12F
// IDA_NAME: _Z18CompareLetterIndexRK15SIG_LETTER_INFOS1_
// DEMANGLED: CompareLetterIndex(SIG_LETTER_INFO const&,SIG_LETTER_INFO const&)
bool CompareLetterIndex(const SIG_LETTER_INFO& lhs,
                        const SIG_LETTER_INFO& rhs) {
    if (lhs.letter_id < rhs.letter_id) {
        return true;
    }
    if (lhs.letter_id == rhs.letter_id) {
        return lhs.letter_id < rhs.letter_id;
    }
    return false;
}

// IDA_EA: 0x841D16D
// IDA_NAME: _Z25ArrangeMailAndLetterIndexP16SIG_MAILBOX_LISTRSt6vectorI15SIG_LETTER_INFOSaIS2_EEiS5_
// DEMANGLED: ArrangeMailAndLetterIndex(SIG_MAILBOX_LIST *,std::vector<SIG_LETTER_INFO,std::allocator<SIG_LETTER_INFO>> &,int,std::vector<SIG_LETTER_INFO,std::allocator<SIG_LETTER_INFO>> &)
int ArrangeMailAndLetterIndex(SIG_MAILBOX_LIST* list,
                              std::vector<SIG_LETTER_INFO>& letters,
                              int count,
                              std::vector<SIG_LETTER_INFO>& keep_letters) {
    std::sort(letters.begin(), letters.end(), CompareLetterIndex);
    for (unsigned int i =
             static_cast<unsigned int>(letters.size()) -
             static_cast<unsigned int>(count);
         letters.size() > i;
         ++i) {
        int mail_slot = -1;
        mail_slot = FindLastLetterIndexMail(
            list, static_cast<int>(letters[i].letter_id));
        if (mail_slot != -1) {
            const int letter_id = static_cast<int32_t>(
                list->response.mails[mail_slot].letter_id);
            if (RemoveMailByLatestLetter(list, mail_slot)) {
                if (letter_id) {
                    for (std::vector<SIG_LETTER_INFO>::iterator it =
                             keep_letters.begin();
                         it != keep_letters.end();
                         ++it) {
                        if (static_cast<int>(it->letter_id) == letter_id) {
                            keep_letters.erase(it);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// IDA_EA: 0x841D2EA
// IDA_NAME: _Z11PrintLetter15SIG_LETTER_INFO
// DEMANGLED: PrintLetter(SIG_LETTER_INFO)
void PrintLetter(SIG_LETTER_INFO letter) {
    (void)letter;
}

// IDA_EA: 0x841D2EF
// IDA_NAME: _Z20ArrangeMailAndLetterP16SIG_MAILBOX_LISTRSt6vectorI15SIG_LETTER_INFOSaIS2_EEiS5_
// DEMANGLED: ArrangeMailAndLetter(SIG_MAILBOX_LIST *,std::vector<SIG_LETTER_INFO,std::allocator<SIG_LETTER_INFO>> &,int,std::vector<SIG_LETTER_INFO,std::allocator<SIG_LETTER_INFO>> &)
int ArrangeMailAndLetter(SIG_MAILBOX_LIST* list,
                         std::vector<SIG_LETTER_INFO>& letters,
                         int count,
                         std::vector<SIG_LETTER_INFO>& keep_letters) {
    std::sort(letters.begin(), letters.end(), CompareLetter);
    for (unsigned int i =
             static_cast<unsigned int>(letters.size()) -
             static_cast<unsigned int>(count);
         letters.size() > i;
         ++i) {
        int mail_slot = -1;
        mail_slot = FindLastOccTimeMail(list, letters[i].reg_time);
        if (mail_slot != -1) {
            const int letter_id = static_cast<int32_t>(
                list->response.mails[mail_slot].letter_id);
            if (RemoveMailByLatestLetter(list, mail_slot)) {
                if (letter_id) {
                    for (std::vector<SIG_LETTER_INFO>::iterator it =
                             keep_letters.begin();
                         it != keep_letters.end();
                         ++it) {
                        if (static_cast<int>(it->letter_id) == letter_id) {
                            keep_letters.erase(it);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// IDA_EA: 0x841D470
// IDA_NAME: _Z16DoChangeMailListP16SIG_MAILBOX_LISTPji
// DEMANGLED: DoChangeMailList(SIG_MAILBOX_LIST *,unsigned int *,int)
bool DoChangeMailList(SIG_MAILBOX_LIST* list,
                      unsigned int* output,
                      int last_mail_index) {
    *output = 0;
    bool result;
    if (list->response.mail_count) {
        list->response.last_postal_id = 0;
        for (int i = 0;; ++i) {
            result = list->response.mail_count > i;
            if (list->response.mail_count <= i) {
                break;
            }
            const unsigned int candidate = list->response.mails[i].postal_id;
            list->response.last_postal_id =
                std::max(list->response.last_postal_id, candidate);
            output[++*output] =
                static_cast<unsigned int>(list->response.mails[i].add_info);
        }
    } else {
        result = list != 0;
        list->response.last_postal_id =
            static_cast<uint32_t>(last_mail_index);
    }
    return result;
}

// IDA_EA: 0x855254C
// IDA_NAME: _Z13isUnlimitMailPK16SIG_MAILBOX_LISTi
// DEMANGLED: isUnlimitMail(SIG_MAILBOX_LIST const*,int)
int isUnlimitMail(const SIG_MAILBOX_LIST* list, int letter_id) {
    for (int i = 0; list->response.mail_count > i; ++i) {
        if (static_cast<int32_t>(list->response.mails[i].letter_id) ==
            letter_id) {
            return list->response.mails[i].unlimit_flag;
        }
    }
    return 0;
}
