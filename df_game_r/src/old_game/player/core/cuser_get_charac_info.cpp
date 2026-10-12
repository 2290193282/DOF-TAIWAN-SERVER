#include "old_game/player/core/cuser.h"

#include "infra/log/format/trace.h"
#include "old_data/db/character/load_charac_payload.h"

// IDA_EA: 0x864BC04
// IDA_NAME: _ZNK5CUser13GetCharacInfoEP15SIG_LOAD_CHARAC
// DEMANGLED: CUser::GetCharacInfo(SIG_LOAD_CHARAC *)const
int CUser::GetCharacInfo(SIG_LOAD_CHARAC* payload) const {
    payload->member_id = static_cast<unsigned int>(get_acc_id());
    payload->character_no = static_cast<unsigned int>(
        getCurCharacR()->fields_.character_no_);

    const int charac_idx = get_charac_idx(payload->character_no);
    payload->result = 0;
    if (getCharacInfoDetail(charac_idx, payload->character_no,
                            &payload->charac_data) == 1) {
        payload->charac_data.village =
            static_cast<char>(getCurCharacVill4Save());

        const unsigned int tag_character_no = static_cast<unsigned int>(
            getCurCharacTagCharacNo());
        const int tag_charac_idx = get_charac_idx(tag_character_no);
        if (tag_charac_idx >= 0 &&
            characters_.size() > static_cast<size_t>(tag_charac_idx)) {
            payload->charac_data.tag_character_no = tag_character_no;
            payload->charac_data.striker_skill_index =
                static_cast<uint8_t>(getCurCharacStrikerSkillIndex());
        }

        getTagCharacInfo(tag_charac_idx,
                         payload->charac_data.tag_character_no,
                         &payload->tag_charac_data);

        cMyTrace trace("bool CUser::GetCharacInfo(SIG_LOAD_CHARAC*) const",
                       4198, 0);
        trace("CharLastPlayTick char(%d) t(%d)", payload->character_no);
        return 1;
    }

    cMyTrace trace("bool CUser::GetCharacInfo(SIG_LOAD_CHARAC*) const",
                   4178, 0);
    trace("getCharacInfoDetail() return = false");
    payload->result = 1;
    return 0;
}
