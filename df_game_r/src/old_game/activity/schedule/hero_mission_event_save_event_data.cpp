#include "old_game/activity/schedule/hero_mission_runtime.h"

#include <cstring>
#include <sstream>

#include "common/manager/global_data.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8164BCA
// IDA_NAME: _ZN16HeroMissionValue14getStringTokenERSs
// DEMANGLED: HeroMissionValue::getStringToken(std::string &)
//
// The IDA tinfo calls the residual EAX from the final stringstream destructor
// an int return.  The caller at 0x8166EAF immediately ignores EAX and the
// Itanium member name does not encode a return type.  The source behavior is
// therefore the serializer below; it deliberately exposes no invented result
// contract.
void HeroMissionValue::getStringToken(std::string& token) {
    std::stringstream stream(std::ios::out | std::ios::in);
    if (token.length()) {
        stream << ',';
    }

    // The original token order is: mission, progress, state, reward_count,
    // value.  `setStringToken` stores the last two in the opposite physical
    // word order (+0x0a, then +0x08), which is why this ordering matters.
    stream << mission_no << ' ' << progress << ' ' << state << ' '
           << reward_count << ' ' << value;
    token.append(stream.str());
}

// IDA_EA: 0x8166D38
// IDA_NAME: _ZN16HeroMissionEvent13saveEventDataEP5CUserb
// DEMANGLED: HeroMissionEvent::saveEventData(CUser *,bool)
void HeroMissionEvent::saveEventData(CUser* user, bool force) {
    // CEventBase::IsEventing is the vtable slot at +0x34 in this concrete
    // object.  Preserve the original virtual dispatch rather than bypassing
    // it through the embedded event flag.
    if (IsEventing(0) != 1) {
        return;
    }

    if (!user) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::saveEventData(CUser*, bool)", 606,
            "[HeroMission] User is null.");
        return;
    }

    if (!user->get_charac_no(-1)) {
        return;
    }

    std::string serialized;
    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    std::map<unsigned int, std::vector<HeroMissionValue> >::iterator found =
        values_by_charac_.find(charac_no);
    if (found == values_by_charac_.end()) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::saveEventData(CUser*, bool)", 617,
            "[HeroMission] Not exist mission value. (charac:%u)",
            static_cast<unsigned int>(user->get_charac_no(-1)));
        return;
    }

    for (std::vector<HeroMissionValue>::iterator value =
             found->second.begin();
         value != found->second.end(); ++value) {
        value->getStringToken(serialized);
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Event/EventHeroMission.cpp", 625);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 17, user->GetUID());
    Taiwan::SigSaveHeroMissionData* const payload =
        guard.GetInBuffer<Taiwan::SigSaveHeroMissionData>();
    if (!payload) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::saveEventData(CUser*, bool)", 630,
            "[HeroMission] SigSaveHeroMissionData is null.");
        return;
    }

    payload->init();
    payload->charac_no = static_cast<unsigned int>(user->get_charac_no(-1));
    // IDA passes string::length directly to strncpy; preserve that original
    // producer-side contract rather than silently truncating the typed 0x100
    // payload.
    std::strncpy(payload->mission, serialized.c_str(), serialized.length());
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);

    if (force) {
        const unsigned int erase_charac_no =
            static_cast<unsigned int>(user->get_charac_no(-1));
        values_by_charac_.erase(erase_charac_no);
    }
}
