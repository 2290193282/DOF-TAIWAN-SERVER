#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/crypto/cipher.h"

extern int get_rand_int(int limit) __asm__("_Z12get_rand_inti");

// IDA_EA: 0x8680706
// IDA_NAME: _ZN5CUser27sendBackSecurityAuthRequestEv
// DEMANGLED: CUser::sendBackSecurityAuthRequest(void)
void CUser::sendBackSecurityAuthRequest() {
    unsigned char question[8] = {};
    *reinterpret_cast<short*>(question) = -1;
    unsigned char encrypted_question[2048] = {};
    int encrypted_size = sizeof(encrypted_question);

    WongWork::CSecurityCard* const security_card = getSecurityCard();
    security_card->generateQuestion(
        *reinterpret_cast<short*>(question + 0),
        *reinterpret_cast<short*>(question + 2),
        *reinterpret_cast<short*>(question + 4),
        *reinterpret_cast<short*>(question + 6));

    int sequence = 1;
    do {
        sequence = get_rand_int(14);
    } while (sequence == 0 || sequence == 5);

    Sanicova::CPad* const pad = getPad();
    pad->GenerateRandomData();
    const int cipher_index = pad->GetSelectedCipherIdx();
    GlobalData::s_secu_cipher_keystring_manager_[cipher_index]->Encrypt(
        sequence,
        question,
        sizeof(question),
        encrypted_question,
        &encrypted_size);

    PacketGuard response;
    response.put_header(0, 152);
    response.put_byte(sequence);
    response.put_short(pad->GetKeyIdx(0));
    response.put_short(pad->GetKeyIdx(1));
    response.put_byte(pad->GetShakeData(0));
    response.put_byte(pad->GetShakeData(1));
    response.put_byte(pad->GetShakeData(2));
    response.put_byte(pad->GetShakeData(3));
    response.put_int(encrypted_size);
    response.put_binary(reinterpret_cast<const char*>(encrypted_question),
                        encrypted_size);
    response.put_byte(security_card->getCertFlag());
    response.finalize(true);
    Send(response);

    if (security_card->isActivate()) {
        security_card->setRequestState(
            static_cast<WongWork::CSecurityCard::ENUM_REQUEST_STATE>(1));
    } else if (pad->isActivate()) {
        pad->setRequestState(Sanicova::CPad::ENUM_REQUEST_STATE_PENDING);
    }
}
