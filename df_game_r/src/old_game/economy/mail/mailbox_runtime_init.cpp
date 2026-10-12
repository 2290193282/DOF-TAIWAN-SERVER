#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_pool.h"

namespace WongWork {

// IDA_EA: 0x8551D98
// IDA_NAME: _ZN8WongWork8CMailBox4InitEv
// DEMANGLED: WongWork::CMailBox::Init(void)
int CMailBox::Init() {
    if (loaded != 1) {
        return loaded ^ 1;
    }

    for (std::map<unsigned int, CMail*>::iterator it = mails.begin();
         it != mails.end(); ++it) {
        CMail* mail = it->second;
        mail->ResetPackage();
        mail_pool.Free(mail);
    }
    mails.clear();

    loaded = 0;
    last_load_idx = 0;
    not_loaded_mail_count = 0;

    if (!pending_streams.empty()) {
        for (std::list<Stream*>::iterator it = pending_streams.begin();
             it != pending_streams.end(); ++it) {
            GlobalData::s_stream_pool->Free(*it);
        }
    }
    pending_streams.clear();

    letter_keep_count.clear();
    last_load_letter_idx = 0;
    loaded_letter_count = 0;
    package_load_lack.clear();
    letter_infos_map.clear();
    return 0;
}

}  // namespace WongWork
