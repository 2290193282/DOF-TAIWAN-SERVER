#ifndef OLDSRV_GLOBAL_MAILBOX_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_MAILBOX_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "infra/network/packet/stream.h"

// IDA unit: 99_misc_global/SIG_LETTER_INFO and 99_misc_global/Global mailbox
// helpers. Mailbox runtime and DB request bodies remain owned by 10/13.
// IDA_EA: SIG_LETTER_INFO::~SIG_LETTER_INFO @0x845543E.
// IDA_EA: GetOnlyMailCount @0x841CC6A, IsContainedLetter @0x841CCB6.
// IDA_EA: RemoveMailByLatestLetter @0x841CD03,
// IDA_EA: RemoveMailByMailIndex @0x841CE06,
// IDA_EA: FindLastOccTimeMail @0x841CF5A.
// IDA_EA: CompareLetter @0x841D032,
// IDA_EA: FindLastLetterIndexMail @0x841D080,
// IDA_EA: CompareLetterIndex @0x841D12F.
// IDA_EA: ArrangeMailAndLetterIndex @0x841D16D,
// IDA_EA: PrintLetter @0x841D2EA,
// IDA_EA: ArrangeMailAndLetter @0x841D2EF.
// IDA_EA: DoChangeMailList @0x841D470, isUnlimitMail @0x855254C.
static_assert(sizeof(SIG_LETTER_INFO) == 0x12Bu,
              "SIG_LETTER_INFO IDA ABI size must stay 0x12B");
static_assert(offsetof(SIG_LETTER_INFO, send_charac_name) == 0x08u,
              "SIG_LETTER_INFO sender name offset must stay 0x08");
static_assert(offsetof(SIG_LETTER_INFO, letter_text) == 0x25u,
              "SIG_LETTER_INFO text offset must stay 0x25");
static_assert(offsetof(SIG_LETTER_INFO, reg_time) == 0x125u,
              "SIG_LETTER_INFO reg_time offset must stay 0x125");
static_assert(offsetof(SIG_LETTER_INFO, stat) == 0x129u,
              "SIG_LETTER_INFO stat offset must stay 0x129");
using ::SIG_LETTER_INFO;
using ::SIG_MAILBOX_LIST;

int GetOnlyMailCount(SIG_MAILBOX_LIST* list)
    __asm__("_Z16GetOnlyMailCountP16SIG_MAILBOX_LIST");
int IsContainedLetter(SIG_MAILBOX_LIST* list, int letter_id)
    __asm__("_Z17IsContainedLetterP16SIG_MAILBOX_LISTi");
int RemoveMailByLatestLetter(SIG_MAILBOX_LIST* list, int mail_slot)
    __asm__("_Z24RemoveMailByLatestLetterP16SIG_MAILBOX_LISTi");
int RemoveMailByMailIndex(SIG_MAILBOX_LIST* list, int mail_index)
    __asm__("_Z21RemoveMailByMailIndexP16SIG_MAILBOX_LISTi");
int FindLastOccTimeMail(SIG_MAILBOX_LIST* list, int occ_time)
    __asm__("_Z19FindLastOccTimeMailP16SIG_MAILBOX_LISTi");
bool CompareLetter(const SIG_LETTER_INFO& lhs, const SIG_LETTER_INFO& rhs)
    __asm__("_Z13CompareLetterRK15SIG_LETTER_INFOS1_");
int FindLastLetterIndexMail(SIG_MAILBOX_LIST* list, int letter_id)
    __asm__("_Z23FindLastLetterIndexMailP16SIG_MAILBOX_LISTi");
bool CompareLetterIndex(const SIG_LETTER_INFO& lhs,
                        const SIG_LETTER_INFO& rhs)
    __asm__("_Z18CompareLetterIndexRK15SIG_LETTER_INFOS1_");
int ArrangeMailAndLetterIndex(SIG_MAILBOX_LIST* list,
                              std::vector<SIG_LETTER_INFO>& letters,
                              int count,
                              std::vector<SIG_LETTER_INFO>& keep_letters)
    __asm__("_Z25ArrangeMailAndLetterIndexP16SIG_MAILBOX_LISTRSt6vectorI15SIG_LETTER_INFOSaIS2_EEiS5_");
void PrintLetter(SIG_LETTER_INFO letter)
    __asm__("_Z11PrintLetter15SIG_LETTER_INFO");
int ArrangeMailAndLetter(SIG_MAILBOX_LIST* list,
                         std::vector<SIG_LETTER_INFO>& letters,
                         int count,
                         std::vector<SIG_LETTER_INFO>& keep_letters)
    __asm__("_Z20ArrangeMailAndLetterP16SIG_MAILBOX_LISTRSt6vectorI15SIG_LETTER_INFOSaIS2_EEiS5_");
bool DoChangeMailList(SIG_MAILBOX_LIST* list,
                      unsigned int* output,
                      int last_mail_index)
    __asm__("_Z16DoChangeMailListP16SIG_MAILBOX_LISTPji");
int isUnlimitMail(const SIG_MAILBOX_LIST* list, int letter_id)
    __asm__("_Z13isUnlimitMailPK16SIG_MAILBOX_LISTi");

#endif  // OLDSRV_GLOBAL_MAILBOX_PUBLIC_ABI_H_
