#include "old_game/activity/schedule/growth_creature_event.h"

#include "common/config.h"
#include <cstring>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/character/character_create_creature_gift_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_event_save_give_grow_creature.h"
#include "old_data/db/event/db_event_update_give_grow_creature.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/creature_guild_params.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const int kGrowthCreatureEventId = 103;

}  // namespace

// IDA_EA: 0x81B7458
// IDA_NAME: _ZN19GrowthCreatureEvent15eventCharacInfoC2Ev
// DEMANGLED: GrowthCreatureEvent::eventCharacInfo::eventCharacInfo(void)
GrowthCreatureEvent::eventCharacInfo::eventCharacInfo()
    : job(0),
      charac_no(0),
      creature_id(0),
      creature_name() {}

// IDA_EA: 0x81B7E16
// IDA_NAME: _ZN19GrowthCreatureEvent15eventCharacInfoD2Ev
// DEMANGLED: GrowthCreatureEvent::eventCharacInfo::~eventCharacInfo()
GrowthCreatureEvent::eventCharacInfo::~eventCharacInfo() {}

// IDA_EA: 0x86970A4
// IDA_NAME: _ZN19GrowthCreatureEventC2Ev
// DEMANGLED: GrowthCreatureEvent::GrowthCreatureEvent(void)
GrowthCreatureEvent::GrowthCreatureEvent() : event_characters() {
    event_characters.clear();
}

// IDA_EA: 0x8697C16
// IDA_NAME: _ZN19GrowthCreatureEventD2Ev
// DEMANGLED: GrowthCreatureEvent::~GrowthCreatureEvent()
GrowthCreatureEvent::~GrowthCreatureEvent() {}

// IDA_EA: 0x81B695C
// IDA_NAME: _ZN21GiveGrowCreatureEventC2Ev
GiveGrowCreatureEvent::GiveGrowCreatureEvent() : CEventBase() {}

// IDA_EA: 0x81B6978
// IDA_NAME: _ZN21GiveGrowCreatureEventD2Ev
// DEMANGLED: GiveGrowCreatureEvent::~GiveGrowCreatureEvent()
// IDA_EA: 0x81B69A8
// IDA_NAME: _ZN21GiveGrowCreatureEventD0Ev
// DEMANGLED: GiveGrowCreatureEvent::~GiveGrowCreatureEvent() deleting destructor
GiveGrowCreatureEvent::~GiveGrowCreatureEvent() {}

// IDA_EA: 0x81B69C6
// IDA_NAME: _ZN21GiveGrowCreatureEvent10StartEventEv
void GiveGrowCreatureEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void GiveGrowCreatureEvent::StartEvent()", 24, 0);
    trace("GiveGrowCreatureEvent Event start!");
}

// IDA_EA: 0x81B6A18
// IDA_NAME: _ZN21GiveGrowCreatureEvent10StartEventE10Word_Param
void GiveGrowCreatureEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("virtual void GiveGrowCreatureEvent::StartEvent(Word_Param)",
                   30,
                   0);
    trace("GiveGrowCreatureEvent Event start! [monitor request]");
}

// IDA_EA: 0x81B6A6A
// IDA_NAME: _ZN21GiveGrowCreatureEvent8EndEventEv
void GiveGrowCreatureEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void GiveGrowCreatureEvent::EndEvent()", 36, 0);
    trace("GiveGrowCreatureEvent Event end!");
}

// IDA_EA: 0x81B6ABC
// IDA_NAME: _ZN21GiveGrowCreatureEvent11StartActionEv
void GiveGrowCreatureEvent::StartAction() {
    G_GameWorld()->ProcessAllUsers([this](CUser* user) {
        if (user && user->get_state() > 2) {
            SendGrowthCreatureEventInfo(*user);
        }
    });
}

// IDA_EA: 0x81B6B70
// IDA_NAME: _ZN21GiveGrowCreatureEvent27SendGrowthCreatureEventInfoER5CUser
void GiveGrowCreatureEvent::SendGrowthCreatureEventInfo(CUser& user) {
    PacketGuard packet;
    packet.put_header(0, 448);

    const int current_seconds = GlobalData::s_systemTime_.getCurSec();
    const int character_job = user.get_charac_job();
    GrowthCreatureEvent* const event_data = user.getGrowthCreatureEventdata();
    CDataManager* const data_manager = G_CDataManager();
    packet.put_byte(data_manager->getGiveGrowCreatureEventJobCount(current_seconds));

    const CharacterCreateCreatureGiftScript* const gift_script =
        data_manager->get_character_create_creature_gift_script();
    for (std::map<int, CharacterCreateCreatureGiftScript::eventJob>::const_iterator
             event_job = gift_script->event_jobs_by_job.begin();
         event_job != gift_script->event_jobs_by_job.end(); ++event_job) {
        if (event_job->second.start >= current_seconds ||
            event_job->second.end <= current_seconds) {
            continue;
        }

        packet.put_byte(event_job->first);
        if (event_data->event_characters.empty()) {
            packet.put_byte(event_job->first == character_job ? 1 : 0);
            continue;
        }

        for (std::vector<GrowthCreatureEvent::eventCharacInfo>::const_iterator
                 event_character = event_data->event_characters.begin();
             event_character != event_data->event_characters.end();
             ++event_character) {
            if (static_cast<int>(event_character->creature_id) !=
                event_job->first) {
                continue;
            }

            const int name_length = static_cast<int>(std::strlen(
                reinterpret_cast<const char*>(event_character->creature_name)));
            if (name_length != 0) {
                packet.put_byte(2);
                packet.put_int(name_length);
                packet.put_str(
                    reinterpret_cast<const char*>(event_character->creature_name),
                    name_length);
                break;
            }
            packet.put_byte(1);
        }
    }

    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x81B6E7A
// IDA_NAME: _ZN21GiveGrowCreatureEvent14CheckEventInfoER5CUser
int GiveGrowCreatureEvent::CheckEventInfo(CUser& user) {
    const int character_job = user.get_charac_job();
    const int current_seconds = GlobalData::s_systemTime_.getCurSec();
    CDataManager* const data_manager = G_CDataManager();
    if (data_manager->isAvailableGiveGrowCreatureEventJob(character_job,
                                                           current_seconds) != 1) {
        return 3;
    }

    GrowthCreatureEvent* const event_data = user.getGrowthCreatureEventdata();
    if (event_data->event_characters.empty()) {
        return 0;
    }
    for (std::vector<GrowthCreatureEvent::eventCharacInfo>::const_iterator
             event_character = event_data->event_characters.begin();
         event_character != event_data->event_characters.end();
         ++event_character) {
        if (static_cast<int>(event_character->creature_id) == character_job) {
            return 3;
        }
    }
    return 0;
}

// IDA_EA: 0x81B6F76
// IDA_NAME: _ZN21GiveGrowCreatureEvent12processEventER5CUser
int GiveGrowCreatureEvent::processEvent(CUser& user) {
    const int source_creature_id =
        user.getCurCharacInvenR()->GetCreatureMgrR()->GetCreatureItemId();
    CDataManager* const data_manager = G_CDataManager();
    const int infinity_creature_id =
        data_manager->getInfinityChangeCreatureIndex(source_creature_id);
    const int client_creature_index =
        data_manager->getClientChangeCreatureIndex(infinity_creature_id);
    if (infinity_creature_id == -1 || client_creature_index == -1) {
        return 22;
    }

    CItem* const item = data_manager->find_item(
        static_cast<unsigned int>(infinity_creature_id));
    if (!item) {
        return 22;
    }

    const int required_level = data_manager->getChangeAbleCreatureLev();
    if (item->vtable_->get_item_type(item) ==
            oldsrv::config::kEquipmentTypeCreature &&
        user.getCurCharacInvenW()->GetCreatureMgrW()->EventEvoluteCreature(
            infinity_creature_id, required_level, client_creature_index) != 1) {
        return 22;
    }

    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo db_record;
    db_record.job = static_cast<uint16_t>(user.GetServerGroupToPvP());
    db_record.charac_no = static_cast<uint32_t>(user.getCurCharacNo());
    db_record.creature_id = static_cast<uint32_t>(user.get_charac_job());
    std::strncpy(reinterpret_cast<char*>(db_record.creature_name),
                 user.getCurCharacName(),
                 0x1du);
    DB_EventSaveGiveGrowCreature::makeRequest(
        static_cast<unsigned int>(user.get_acc_id()), db_record);

    GrowthCreatureEvent::eventCharacInfo event_record;
    event_record.job = static_cast<uint16_t>(user.GetServerGroupToPvP());
    event_record.charac_no = static_cast<uint32_t>(user.getCurCharacNo());
    event_record.creature_id = static_cast<uint32_t>(user.get_charac_job());
    std::strncpy(reinterpret_cast<char*>(event_record.creature_name),
                 user.getCurCharacName(),
                 0x1du);
    user.getGrowthCreatureEventdata()->event_characters.push_back(event_record);
    return 0;
}

// IDA_EA: 0x81B7156
// IDA_NAME: _ZN21GiveGrowCreatureEvent22isGrowCreatureMaxLevelER5CUser
int GiveGrowCreatureEvent::isGrowCreatureMaxLevel(CUser& user) {
    return user.getCurCharacInvenR()->GetCreatureMgrR()->isGrowCreatureMaxLevel();
}

// IDA_EA: 0x81B717A
// IDA_NAME: _ZN21GiveGrowCreatureEvent16changeCharacNameEP5CUseriPci
void GiveGrowCreatureEvent::changeCharacName(CUser* user,
                                             int charac_no,
                                             char* name,
                                             int length) {
    (void)length;
    if (!user) {
        return;
    }

    const int server_group = user->GetServerGroupToPvP();
    GrowthCreatureEvent* const event_data = user->getGrowthCreatureEventdata();
    if (event_data->event_characters.empty()) {
        return;
    }

    bool changed = false;
    size_t changed_index = 0;
    for (size_t index = 0; index < event_data->event_characters.size(); ++index) {
        GrowthCreatureEvent::eventCharacInfo& event_character =
            event_data->event_characters[index];
        if (event_character.job == static_cast<uint16_t>(server_group) &&
            event_character.charac_no == static_cast<uint32_t>(charac_no) &&
            std::strcmp(reinterpret_cast<const char*>(event_character.creature_name),
                        name) != 0) {
            std::strncpy(reinterpret_cast<char*>(event_character.creature_name),
                         name,
                         0x1du);
            changed = true;
            changed_index = index;
            break;
        }
    }

    if (!changed) {
        return;
    }

    const GrowthCreatureEvent::eventCharacInfo& event_character =
        event_data->event_characters[changed_index];
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo db_record;
    db_record.job = event_character.job;
    db_record.charac_no = event_character.charac_no;
    db_record.creature_id = event_character.creature_id;
    std::strncpy(reinterpret_cast<char*>(db_record.creature_name),
                 reinterpret_cast<const char*>(event_character.creature_name),
                 0x1du);
    DB_EventUpdateGiveGrowCreature::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()), db_record);
}

// IDA_EA: 0x81B739E
// IDA_NAME: _ZNK21GiveGrowCreatureEvent10IsEventingEP5CUser
int GiveGrowCreatureEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user);
}

// IDA_EA: 0x82436D6 / 0x8243826 / 0x8243830 / 0x825D43E / 0x825E1C6 /
//         0x8597EB6
template class IPacketDispatcher<MSG_BASE,
                                 stGrowthCreature,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8597EC2
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreatureC2Ev
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::Dispatcher_GrowthCreatureChangeInfinityCreature(void)
Dispatcher_GrowthCreatureChangeInfinityCreature::
    Dispatcher_GrowthCreatureChangeInfinityCreature()
    : IPacketDispatcher<MSG_BASE, stGrowthCreature, static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8243A80
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreatureD2Ev
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::~Dispatcher_GrowthCreatureChangeInfinityCreature()
// IDA_EA: 0x8243AB0
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreatureD0Ev
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::~Dispatcher_GrowthCreatureChangeInfinityCreature() deleting destructor
Dispatcher_GrowthCreatureChangeInfinityCreature::
    ~Dispatcher_GrowthCreatureChangeInfinityCreature() {}

// IDA_EA: 0x81E85A2
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GrowthCreatureChangeInfinityCreature::read(PacketBuf& packet,
                                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81E85AC
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GrowthCreatureChangeInfinityCreature::process(CUser* user,
                                                              MSG_BASE& message,
                                                              ParamBase& param) {
    stGrowthCreature& result = static_cast<stGrowthCreature&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(
            25826,
            "virtual int Dispatcher_GrowthCreatureChangeInfinityCreature::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result,
            0);
    }
    result.result = process_event(user);
    return 0;
}

// IDA_EA: 0x81E863A
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature13process_eventEP5CUser
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::process_event(CUser *)
int Dispatcher_GrowthCreatureChangeInfinityCreature::process_event(CUser* user) {
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId);
    if (base_event->IsEventing(0) == 1) {
        GiveGrowCreatureEvent* const event = static_cast<GiveGrowCreatureEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId));
        if (event) {
            return event->processEvent(*user);
        }
    }
    return 0;
}

// IDA_EA: 0x81E86A6
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::check_error(CUser *,MSG_BASE &)
int Dispatcher_GrowthCreatureChangeInfinityCreature::check_error(CUser* user,
                                                                  MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 19;
    }
    return check_error_event(user);
}

// IDA_EA: 0x81E86FC
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature17check_error_eventEP5CUser
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::check_error_event(CUser *)
int Dispatcher_GrowthCreatureChangeInfinityCreature::check_error_event(CUser* user) {
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId);
    if (base_event->IsEventing(0) == 1) {
        GiveGrowCreatureEvent* const event = static_cast<GiveGrowCreatureEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId));
        if (event) {
            return event->CheckEventInfo(*user);
        }
    }
    return 1;
}

// IDA_EA: 0x81E8768
// IDA_NAME: _ZN47Dispatcher_GrowthCreatureChangeInfinityCreature4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GrowthCreatureChangeInfinityCreature::send(CUser *,ParamBase &)
void Dispatcher_GrowthCreatureChangeInfinityCreature::send(CUser* user,
                                                            ParamBase& param) {
    const stGrowthCreature& result = static_cast<const stGrowthCreature&>(param);
    if (result.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_EQUIPED_CREATURE_CHANGE_INFINITY_CREATURE,
            static_cast<unsigned char>(result.result));
        return;
    }

    user->SendCmdOkPacket(ENUM_CMDPACKET_EQUIPED_CREATURE_CHANGE_INFINITY_CREATURE);
    CEventBase* const base_event =
        GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId);
    if (base_event->IsEventing(0) == 1) {
        GiveGrowCreatureEvent* const event = static_cast<GiveGrowCreatureEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(kGrowthCreatureEventId));
        if (event) {
            event->SendGrowthCreatureEventInfo(*user);
        }
    }
}
