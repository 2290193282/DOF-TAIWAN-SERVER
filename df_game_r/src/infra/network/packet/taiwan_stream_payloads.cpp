#include "infra/network/packet/stream_guard.h"

#include "old_data/db/event/db_create_dnf_user_info.h"
#include "old_data/db/event/db_insert_give_box_event_reward.h"
#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_data/db/event/db_select_mobile_auth_reward_tw.h"
#include "old_data/db/legacy_dispatch/geo_reject_payload.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"

// These are the concrete old-server member-template bodies used by the
// Taiwan DB/Inter queue.  Keep them as real Stream/CStreamGuard specializations:
// a size-based void* facade would not provide the original symbols or consume
// the Stream cursors at the original call sites.

// IDA_EA: 0x845480C
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_SAVE_GEO_REJECTEEPT_v
template <>
SIG_SAVE_GEO_REJECT* Stream::GetOutBuffer<SIG_SAVE_GEO_REJECT>() {
    const int bytes = sizeof(SIG_SAVE_GEO_REJECT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_GEO_REJECT* const result =
        reinterpret_cast<SIG_SAVE_GEO_REJECT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x81718D0
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_SAVE_GEO_REJECTEEPT_v
template <>
SIG_SAVE_GEO_REJECT* Stream::GetInBuffer<SIG_SAVE_GEO_REJECT>() {
    const int bytes = sizeof(SIG_SAVE_GEO_REJECT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_GEO_REJECT* const result =
        reinterpret_cast<SIG_SAVE_GEO_REJECT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8171532
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_SAVE_GEO_REJECTEEPT_v
template <>
SIG_SAVE_GEO_REJECT* CStreamGuard::GetInBuffer<SIG_SAVE_GEO_REJECT>() {
    return stream_->GetInBuffer<SIG_SAVE_GEO_REJECT>();
}

// IDA_EA: 0x815E9B8
// IDA_NAME: _ZN6Stream12GetOutBufferIN6Taiwan21SigLoadRestrictPolicyEEEPT_v
template <>
Taiwan::SigLoadRestrictPolicy*
Stream::GetOutBuffer<Taiwan::SigLoadRestrictPolicy>() {
    const int bytes = sizeof(Taiwan::SigLoadRestrictPolicy);
    if (!enable_out(bytes)) {
        return 0;
    }
    Taiwan::SigLoadRestrictPolicy* result =
        reinterpret_cast<Taiwan::SigLoadRestrictPolicy*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F202
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan18SigLoadGeolocationEEEPT_v
template <>
Taiwan::SigLoadGeolocation*
Stream::GetInBuffer<Taiwan::SigLoadGeolocation>() {
    const int bytes = sizeof(Taiwan::SigLoadGeolocation);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigLoadGeolocation* result =
        reinterpret_cast<Taiwan::SigLoadGeolocation*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EA0C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan18SigLoadGeolocationEEEPT_v
template <>
Taiwan::SigLoadGeolocation*
CStreamGuard::GetInBuffer<Taiwan::SigLoadGeolocation>() {
    return stream_->GetInBuffer<Taiwan::SigLoadGeolocation>();
}

// IDA_EA: 0x815EA22
// IDA_NAME: _ZN6Stream12GetOutBufferIN6Taiwan16SigStayTimeEventEEEPT_v
template <>
Taiwan::SigStayTimeEvent*
Stream::GetOutBuffer<Taiwan::SigStayTimeEvent>() {
    const int bytes = sizeof(Taiwan::SigStayTimeEvent);
    if (!enable_out(bytes)) {
        return 0;
    }
    Taiwan::SigStayTimeEvent* result =
        reinterpret_cast<Taiwan::SigStayTimeEvent*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EAA2
// IDA_NAME: _ZN6Stream12GetOutBufferIN6Taiwan17SigUpdateExitTimeEEEPT_v
template <>
Taiwan::SigUpdateExitTime*
Stream::GetOutBuffer<Taiwan::SigUpdateExitTime>() {
    const int bytes = sizeof(Taiwan::SigUpdateExitTime);
    if (!enable_out(bytes)) {
        return 0;
    }
    Taiwan::SigUpdateExitTime* result =
        reinterpret_cast<Taiwan::SigUpdateExitTime*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F2D6
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan17SigUpdateExitTimeEEEPT_v
template <>
Taiwan::SigUpdateExitTime*
Stream::GetInBuffer<Taiwan::SigUpdateExitTime>() {
    const int bytes = sizeof(Taiwan::SigUpdateExitTime);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigUpdateExitTime* result =
        reinterpret_cast<Taiwan::SigUpdateExitTime*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EA8C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan17SigUpdateExitTimeEEEPT_v
template <>
Taiwan::SigUpdateExitTime*
CStreamGuard::GetInBuffer<Taiwan::SigUpdateExitTime>() {
    return stream_->GetInBuffer<Taiwan::SigUpdateExitTime>();
}

// IDA_EA: 0x816C14A
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan21SigResetStayTimeEventEEEPT_v
template <>
Taiwan::SigResetStayTimeEvent*
Stream::GetInBuffer<Taiwan::SigResetStayTimeEvent>() {
    const int bytes = sizeof(Taiwan::SigResetStayTimeEvent);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigResetStayTimeEvent* result =
        reinterpret_cast<Taiwan::SigResetStayTimeEvent*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x816C134
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan21SigResetStayTimeEventEEEPT_v
template <>
Taiwan::SigResetStayTimeEvent*
CStreamGuard::GetInBuffer<Taiwan::SigResetStayTimeEvent>() {
    return stream_->GetInBuffer<Taiwan::SigResetStayTimeEvent>();
}

// IDA_EA: 0x815ED72
// IDA_NAME: _ZN6Stream12GetOutBufferIN6Taiwan22SigLoadHeroMissionDataEEEPT_v
template <>
Taiwan::SigLoadHeroMissionData*
Stream::GetOutBuffer<Taiwan::SigLoadHeroMissionData>() {
    const int bytes = sizeof(Taiwan::SigLoadHeroMissionData);
    if (!enable_out(bytes)) {
        return 0;
    }
    Taiwan::SigLoadHeroMissionData* result =
        reinterpret_cast<Taiwan::SigLoadHeroMissionData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F5BC
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan22SigLoadHeroMissionDataEEEPT_v
template <>
Taiwan::SigLoadHeroMissionData*
Stream::GetInBuffer<Taiwan::SigLoadHeroMissionData>() {
    const int bytes = sizeof(Taiwan::SigLoadHeroMissionData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigLoadHeroMissionData* result =
        reinterpret_cast<Taiwan::SigLoadHeroMissionData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EDC6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan22SigLoadHeroMissionDataEEEPT_v
template <>
Taiwan::SigLoadHeroMissionData*
CStreamGuard::GetInBuffer<Taiwan::SigLoadHeroMissionData>() {
    return stream_->GetInBuffer<Taiwan::SigLoadHeroMissionData>();
}

// IDA_EA: 0x815EDDC
// IDA_NAME: _ZN6Stream12GetOutBufferIN6Taiwan22SigSaveHeroMissionDataEEEPT_v
template <>
Taiwan::SigSaveHeroMissionData*
Stream::GetOutBuffer<Taiwan::SigSaveHeroMissionData>() {
    const int bytes = sizeof(Taiwan::SigSaveHeroMissionData);
    if (!enable_out(bytes)) {
        return 0;
    }
    Taiwan::SigSaveHeroMissionData* result =
        reinterpret_cast<Taiwan::SigSaveHeroMissionData*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x816962A
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan22SigSaveHeroMissionDataEEEPT_v
template <>
Taiwan::SigSaveHeroMissionData*
Stream::GetInBuffer<Taiwan::SigSaveHeroMissionData>() {
    const int bytes = sizeof(Taiwan::SigSaveHeroMissionData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigSaveHeroMissionData* result =
        reinterpret_cast<Taiwan::SigSaveHeroMissionData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x816894E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan22SigSaveHeroMissionDataEEEPT_v
template <>
Taiwan::SigSaveHeroMissionData*
CStreamGuard::GetInBuffer<Taiwan::SigSaveHeroMissionData>() {
    return stream_->GetInBuffer<Taiwan::SigSaveHeroMissionData>();
}

// IDA_EA: 0x815EB0C
// IDA_NAME: _ZN6Stream12GetOutBufferIN25DBSelectCreateDnfUserInfo3MSGEEEPT_v
template <>
DBSelectCreateDnfUserInfo::MSG*
Stream::GetOutBuffer<DBSelectCreateDnfUserInfo::MSG>() {
    const int bytes = sizeof(DBSelectCreateDnfUserInfo::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBSelectCreateDnfUserInfo::MSG* result =
        reinterpret_cast<DBSelectCreateDnfUserInfo::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F340
// IDA_NAME: _ZN6Stream11GetInBufferIN25DBSelectCreateDnfUserInfo3MSGEEEPT_v
template <>
DBSelectCreateDnfUserInfo::MSG*
Stream::GetInBuffer<DBSelectCreateDnfUserInfo::MSG>() {
    const int bytes = sizeof(DBSelectCreateDnfUserInfo::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBSelectCreateDnfUserInfo::MSG* result =
        reinterpret_cast<DBSelectCreateDnfUserInfo::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EAF6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN25DBSelectCreateDnfUserInfo3MSGEEEPT_v
template <>
DBSelectCreateDnfUserInfo::MSG*
CStreamGuard::GetInBuffer<DBSelectCreateDnfUserInfo::MSG>() {
    return stream_->GetInBuffer<DBSelectCreateDnfUserInfo::MSG>();
}

// IDA_EA: 0x815EB76
// IDA_NAME: _ZN6Stream12GetOutBufferIN25DBInsertCreateDnfUserInfo3MSGEEEPT_v
template <>
DBInsertCreateDnfUserInfo::MSG*
Stream::GetOutBuffer<DBInsertCreateDnfUserInfo::MSG>() {
    const int bytes = sizeof(DBInsertCreateDnfUserInfo::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBInsertCreateDnfUserInfo::MSG* result =
        reinterpret_cast<DBInsertCreateDnfUserInfo::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F3AA
// IDA_NAME: _ZN6Stream11GetInBufferIN25DBInsertCreateDnfUserInfo3MSGEEEPT_v
template <>
DBInsertCreateDnfUserInfo::MSG*
Stream::GetInBuffer<DBInsertCreateDnfUserInfo::MSG>() {
    const int bytes = sizeof(DBInsertCreateDnfUserInfo::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBInsertCreateDnfUserInfo::MSG* result =
        reinterpret_cast<DBInsertCreateDnfUserInfo::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EB60
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN25DBInsertCreateDnfUserInfo3MSGEEEPT_v
template <>
DBInsertCreateDnfUserInfo::MSG*
CStreamGuard::GetInBuffer<DBInsertCreateDnfUserInfo::MSG>() {
    return stream_->GetInBuffer<DBInsertCreateDnfUserInfo::MSG>();
}

// IDA_EA: 0x815EBE0
// IDA_NAME: _ZN6Stream12GetOutBufferIN26DBSelectMobileAuthRewardTw3MSGEEEPT_v
template <>
DBSelectMobileAuthRewardTw::MSG*
Stream::GetOutBuffer<DBSelectMobileAuthRewardTw::MSG>() {
    const int bytes = sizeof(DBSelectMobileAuthRewardTw::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBSelectMobileAuthRewardTw::MSG* const result =
        reinterpret_cast<DBSelectMobileAuthRewardTw::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F414
// IDA_NAME: _ZN6Stream11GetInBufferIN26DBSelectMobileAuthRewardTw3MSGEEEPT_v
template <>
DBSelectMobileAuthRewardTw::MSG*
Stream::GetInBuffer<DBSelectMobileAuthRewardTw::MSG>() {
    const int bytes = sizeof(DBSelectMobileAuthRewardTw::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBSelectMobileAuthRewardTw::MSG* const result =
        reinterpret_cast<DBSelectMobileAuthRewardTw::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EBCA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN26DBSelectMobileAuthRewardTw3MSGEEEPT_v
template <>
DBSelectMobileAuthRewardTw::MSG*
CStreamGuard::GetInBuffer<DBSelectMobileAuthRewardTw::MSG>() {
    return stream_->GetInBuffer<DBSelectMobileAuthRewardTw::MSG>();
}

// IDA_EA: 0x815EC4A
// IDA_NAME: _ZN6Stream12GetOutBufferIN25DBSelectPcroomDailyReward3MSGEEEPT_v
template <>
DBSelectPcroomDailyReward::MSG*
Stream::GetOutBuffer<DBSelectPcroomDailyReward::MSG>() {
    const int bytes = sizeof(DBSelectPcroomDailyReward::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBSelectPcroomDailyReward::MSG* result =
        reinterpret_cast<DBSelectPcroomDailyReward::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F47E
// IDA_NAME: _ZN6Stream11GetInBufferIN25DBSelectPcroomDailyReward3MSGEEEPT_v
template <>
DBSelectPcroomDailyReward::MSG*
Stream::GetInBuffer<DBSelectPcroomDailyReward::MSG>() {
    const int bytes = sizeof(DBSelectPcroomDailyReward::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBSelectPcroomDailyReward::MSG* result =
        reinterpret_cast<DBSelectPcroomDailyReward::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EC34
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN25DBSelectPcroomDailyReward3MSGEEEPT_v
template <>
DBSelectPcroomDailyReward::MSG*
CStreamGuard::GetInBuffer<DBSelectPcroomDailyReward::MSG>() {
    return stream_->GetInBuffer<DBSelectPcroomDailyReward::MSG>();
}

// IDA_EA: 0x815ECB4
// IDA_NAME: _ZN6Stream12GetOutBufferIN25DBUpdatePcroomDailyReward3MSGEEEPT_v
template <>
DBUpdatePcroomDailyReward::MSG*
Stream::GetOutBuffer<DBUpdatePcroomDailyReward::MSG>() {
    const int bytes = sizeof(DBUpdatePcroomDailyReward::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBUpdatePcroomDailyReward::MSG* result =
        reinterpret_cast<DBUpdatePcroomDailyReward::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F4E8
// IDA_NAME: _ZN6Stream11GetInBufferIN25DBUpdatePcroomDailyReward3MSGEEEPT_v
template <>
DBUpdatePcroomDailyReward::MSG*
Stream::GetInBuffer<DBUpdatePcroomDailyReward::MSG>() {
    const int bytes = sizeof(DBUpdatePcroomDailyReward::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBUpdatePcroomDailyReward::MSG* result =
        reinterpret_cast<DBUpdatePcroomDailyReward::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EC9E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN25DBUpdatePcroomDailyReward3MSGEEEPT_v
template <>
DBUpdatePcroomDailyReward::MSG*
CStreamGuard::GetInBuffer<DBUpdatePcroomDailyReward::MSG>() {
    return stream_->GetInBuffer<DBUpdatePcroomDailyReward::MSG>();
}

// IDA_EA: 0x815ED1E
// IDA_NAME: _ZN6Stream12GetOutBufferIN26DBInsertGiveBoxEventReward3MSGEEEPT_v
template <>
DBInsertGiveBoxEventReward::MSG*
Stream::GetOutBuffer<DBInsertGiveBoxEventReward::MSG>() {
    const int bytes = sizeof(DBInsertGiveBoxEventReward::MSG);
    if (!enable_out(bytes)) {
        return 0;
    }
    DBInsertGiveBoxEventReward::MSG* result =
        reinterpret_cast<DBInsertGiveBoxEventReward::MSG*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x815F552
// IDA_NAME: _ZN6Stream11GetInBufferIN26DBInsertGiveBoxEventReward3MSGEEEPT_v
template <>
DBInsertGiveBoxEventReward::MSG*
Stream::GetInBuffer<DBInsertGiveBoxEventReward::MSG>() {
    const int bytes = sizeof(DBInsertGiveBoxEventReward::MSG);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    DBInsertGiveBoxEventReward::MSG* result =
        reinterpret_cast<DBInsertGiveBoxEventReward::MSG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815ED08
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN26DBInsertGiveBoxEventReward3MSGEEEPT_v
template <>
DBInsertGiveBoxEventReward::MSG*
CStreamGuard::GetInBuffer<DBInsertGiveBoxEventReward::MSG>() {
    return stream_->GetInBuffer<DBInsertGiveBoxEventReward::MSG>();
}
