#include "old_data/PVF/etc/title_book_info.h"

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/itemshop/quest_shop_status.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string.h>

#include <string>
#include <utility>


typedef char stTitleElement_Size_Must_Be_0x28[
    (sizeof(stTitleElement) == 0x28) ? 1 : -1];
typedef char stTitleSection_Size_Must_Be_0x40[
    (sizeof(stTitleSection) == 0x40) ? 1 : -1];
typedef char stCustomConditionMsg_Size_Must_Be_0x0C[
    (sizeof(stCustomConditionMsg) == 0x0C) ? 1 : -1];
typedef char stAdjust_Size_Must_Be_0x0C[
    (sizeof(stAdjust) == 0x0C) ? 1 : -1];
typedef char stTitleBookInfo_Size_Must_Be_0x58[
    (sizeof(stTitleBookInfo) == 0x58) ? 1 : -1];

// IDA_EA: 0x828D926
// IDA_NAME: _ZN14stTitleElementD2Ev
// DEMANGLED: stTitleElement::~stTitleElement()
stTitleElement::~stTitleElement() {}

// IDA_EA: 0x828F43E
// IDA_NAME: _ZN14stTitleElementC2ERKS_
// DEMANGLED: stTitleElement::stTitleElement(stTitleElement const&)
stTitleElement::stTitleElement(const stTitleElement& other)
    : title_index(other.title_index),
      condition_values(other.condition_values),
      value_10(other.value_10),
      value_14(other.value_14),
      value_18(other.value_18),
      condition_type(other.condition_type),
      value_20(other.value_20),
      value_24(other.value_24) {}

// IDA_EA: 0x8AA95AE
// IDA_NAME: _ZN14stTitleElementC2Ev
// DEMANGLED: stTitleElement::stTitleElement(void)
stTitleElement::stTitleElement()
    : condition_values(), value_18() {
    condition_type = -1;
    title_index = -1;
    value_10 = -1;
    value_14 = 0;
    value_20 = 0;
    value_24 = -1;
}

// IDA_EA: 0x8AA9E2A
// IDA_NAME: _ZN14stTitleElementaSERKS_
// DEMANGLED: stTitleElement::operator=(stTitleElement const&)
stTitleElement& stTitleElement::operator=(const stTitleElement& other) {
    title_index = other.title_index;
    condition_values = other.condition_values;
    value_10 = other.value_10;
    value_14 = other.value_14;
    value_18 = other.value_18;
    condition_type = other.condition_type;
    value_20 = other.value_20;
    value_24 = other.value_24;
    return *this;
}

// IDA_EA: 0x828D972
// IDA_NAME: _ZN14stTitleSectionD2Ev
// DEMANGLED: stTitleSection::~stTitleSection()
stTitleSection::~stTitleSection() {}

// IDA_EA: 0x828DAC2
// IDA_NAME: _ZN14stTitleSectionC2ERKS_
// DEMANGLED: stTitleSection::stTitleSection(stTitleSection const&)
stTitleSection::stTitleSection(const stTitleSection& other)
    : category(other.category),
      section_name(other.section_name),
      value_08(other.value_08),
      start_slot(other.start_slot),
      end_slot(other.end_slot),
      name(other.name),
      image_path(other.image_path),
      image_frame(other.image_frame),
      effect_path(other.effect_path),
      section_icon(other.section_icon),
      section_icon_frame(other.section_icon_frame),
      frame_type(other.frame_type),
      lock_message(other.lock_message),
      rewards(other.rewards) {}

// IDA_EA: 0x8AA979E
// IDA_NAME: _ZN14stTitleSectionC2Ev
// DEMANGLED: stTitleSection::stTitleSection(void)
stTitleSection::stTitleSection()
    : section_name(),
      value_08(),
      name(),
      image_path(),
      effect_path(),
      section_icon(),
      lock_message(),
      rewards() {
    // 0..4 are the five target-client tabs.  Stay explicitly invalid until
    // the enclosing [section] assigns a real tab; malformed input must not
    // silently alias the event tab.
    category = 5;
    start_slot = -1;
    end_slot = -1;
    image_frame = -1;
    section_icon_frame = -1;
    frame_type = 0;
}

// IDA_EA: 0x8AAA242
// IDA_NAME: _ZN14stTitleSectionaSERKS_
// DEMANGLED: stTitleSection::operator=(stTitleSection const&)
stTitleSection& stTitleSection::operator=(const stTitleSection& other) {
    category = other.category;
    section_name = other.section_name;
    value_08 = other.value_08;
    start_slot = other.start_slot;
    end_slot = other.end_slot;
    name = other.name;
    image_path = other.image_path;
    image_frame = other.image_frame;
    effect_path = other.effect_path;
    section_icon = other.section_icon;
    section_icon_frame = other.section_icon_frame;
    frame_type = other.frame_type;
    lock_message = other.lock_message;
    rewards = other.rewards;
    return *this;
}

// IDA_EA: 0x83E629C
// IDA_NAME: _ZN20stCustomConditionMsgD2Ev
// DEMANGLED: stCustomConditionMsg::~stCustomConditionMsg()
stCustomConditionMsg::~stCustomConditionMsg() {}

// IDA_EA: 0x8AA9928
// IDA_NAME: _ZN20stCustomConditionMsgC2Ev
// DEMANGLED: stCustomConditionMsg::stCustomConditionMsg(void)
stCustomConditionMsg::stCustomConditionMsg() : message() {
    category = -1;
    condition_id = -1;
}

// IDA_EA: 0x8AAA67C
// IDA_NAME: _ZN20stCustomConditionMsgC2ERKS_
// DEMANGLED: stCustomConditionMsg::stCustomConditionMsg(stCustomConditionMsg const&)
stCustomConditionMsg::stCustomConditionMsg(
    const stCustomConditionMsg& other)
    : category(other.category),
      condition_id(other.condition_id),
      message(other.message) {}

// IDA_EA: 0x8AAA736
// IDA_NAME: _ZN20stCustomConditionMsgaSERKS_
// DEMANGLED: stCustomConditionMsg::operator=(stCustomConditionMsg const&)
stCustomConditionMsg& stCustomConditionMsg::operator=(
    const stCustomConditionMsg& other) {
    category = other.category;
    condition_id = other.condition_id;
    message = other.message;
    return *this;
}

// IDA_EA: 0x8AA9906
// IDA_NAME: _ZN8stAdjustC2Ev
// DEMANGLED: stAdjust::stAdjust(void)
stAdjust::stAdjust()
    : title_index(0), adjust_type(0), adjust_value(0) {}

// IDA_EA: 0x8AABE5E
// IDA_NAME: _ZN8stAdjustD2Ev
// DEMANGLED: stAdjust::~stAdjust()
stAdjust::~stAdjust() {}

// IDA_EA: 0x837D828
// IDA_NAME: _ZN15stTitleBookInfoC2Ev
// DEMANGLED: stTitleBookInfo::stTitleBookInfo(void)
stTitleBookInfo::stTitleBookInfo()
    : default_reward_values(),
      sections(),
      title_collections(),
      custom_condition_messages(),
      adjusts(),
      default_rewards() {}

// IDA_EA: 0x837D906
// IDA_NAME: _ZN15stTitleBookInfoD2Ev
// DEMANGLED: stTitleBookInfo::~stTitleBookInfo()
stTitleBookInfo::~stTitleBookInfo() {}

// IDA_EA: 0x8AA7D98
// IDA_NAME: _Z26convertTabTypeStringToEnumPKc
// DEMANGLED: convertTabTypeStringToEnum(char const*)
int convertTabTypeStringToEnum(const char* text) {
    if (strcmp(text, "general") == 0) {
        return 0;
    }
    if (strcmp(text, "specific") == 0) {
        return 1;
    }
    if (strcmp(text, "pvp") == 0) {
        return 2;
    }
    if (strcmp(text, "despair") == 0) {
        return 3;
    }
    if (strcmp(text, "event") == 0) {
        return 4;
    }
    return 5;
}

// IDA_EA: 0x8AA8B79
// IDA_NAME: _Z13importRewardsPSt6vectorIP18stTitleRewardBasisSaIS1_EESs
// DEMANGLED: importRewards(std::vector<stTitleRewardBasis *,std::allocator<stTitleRewardBasis *>> *,std::string)
int importRewards(std::vector<stTitleRewardBasis*>* rewards,
                  std::string reward_tag) {
    std::string value_type_text;
    const int appendage_type = loadAppendageType(reward_tag);
    if (appendage_type == -1) {
        if (reward_tag == "[give emotion]") {
            stRewardGiveEmotion* reward = new stRewardGiveEmotion;
            reward->emotion_id = ScanInt(static_cast<bool*>(0));
            rewards->push_back(reward);
        } else if (reward_tag == "[send postal]") {
            stRewardPostal* reward = new stRewardPostal;
            reward->first_dword =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
            reward->second_dword =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
            ScanStr(&reward->mail_title);
            ScanStr(&reward->mail_text);
            ScanStr(&reward->sender_name);
            rewards->push_back(reward);
        }
    } else {
        stRewardChangeStatus* reward = new stRewardChangeStatus;
        reward->status_type = appendage_type;
        ScanStr(&value_type_text);
        reward->value_type = loadValueType(value_type_text);
        const float status_value = ScanFloat(static_cast<bool*>(0));
        memcpy(&reward->status_value, &status_value, sizeof(status_value));
        rewards->push_back(reward);
    }
    return 1;
}

// IDA_EA: 0x8AA8D65
// IDA_NAME: _Z18importTitleSectionP14stTitleSectioni
// DEMANGLED: importTitleSection(stTitleSection *,int)
int importTitleSection(stTitleSection* section, int section_index) {
    std::string tag;
    std::string text;
    section->start_slot = 10 * section_index;
    section->end_slot = section->start_slot + 10;

    while (ScanType(tag, true) == 1) {
        if (tag == "[/section]") {
            return 0;
        }
        if (tag == "[/reward type]") {
            return 1;
        }
        if (tag == "[section name]") {
            ScanStr(&section->section_name);
        } else if (tag == "[section icon]") {
            ScanStr(&section->section_icon);
            section->section_icon_frame = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[frame type]") {
            section->frame_type = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[lock message]") {
            ScanStr(&section->lock_message);
        } else if (tag == "[name]") {
            ScanStr(&section->name);
        } else if (tag == "[image path]") {
            ScanStr(&section->image_path);
            section->image_frame = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[effect path]") {
            ScanStr(&section->effect_path);
        }

        text = tag;
        importRewards(&section->rewards, text);
    }
    return 1;
}

// IDA_EA: 0x8AA94E1
// IDA_NAME: _Z18getTitleCollectionP15stTitleBookInfoi
// DEMANGLED: getTitleCollection(stTitleBookInfo *,int)
std::vector<stTitleElement>* getTitleCollection(stTitleBookInfo* info,
                                                int category) {
    std::map<int, std::vector<stTitleElement> >::iterator it =
        info->title_collections.find(category);
    if (it == info->title_collections.end()) {
        return 0;
    }
    return &info->title_collections[category];
}

// IDA_EA: 0x8AA8110
// IDA_NAME: _Z24importTitleBookEtcScriptPKcP15stTitleBookInfo
// DEMANGLED: importTitleBookEtcScript(char const*,stTitleBookInfo *)
int importTitleBookEtcScript(const char* filename, stTitleBookInfo* info) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[default reward]") {
            while (ScanType(text, true) == 1) {
                if (text == "[/default reward]") {
                    break;
                }
                importRewards(&info->default_rewards, text);
            }
        } else if (tag == "[title collection info]") {
            ScanStr(&text);
            const int category = convertTabTypeStringToEnum(text.c_str());
            const int element_count = ScanInt(static_cast<bool*>(0));
            for (int i = 0; i < element_count; ++i) {
                ScanInt(static_cast<bool*>(0));
                stTitleElement element;
                element.title_index = ScanInt(static_cast<bool*>(0));
                if (element.title_index != -1) {
                    element.condition_type = ScanInt(static_cast<bool*>(0));
                    const int condition_count =
                        ScanInt(static_cast<bool*>(0));
                    for (int j = 0; j < condition_count; ++j) {
                        const int condition_value =
                            ScanInt(static_cast<bool*>(0));
                        element.condition_values.push_back(condition_value);
                    }
                }
                info->title_collections[category].push_back(element);
            }
        } else if (tag == "[section]") {
            ScanStr(&text);
            const int category = convertTabTypeStringToEnum(text.c_str());
            int section_ok = 0;
            do {
                stTitleSection section;
                section.category = category;
                int same_category_count = 0;
                for (std::size_t index = 0; index < info->sections.size();
                     ++index) {
                    if (info->sections[index].category == category) {
                        ++same_category_count;
                    }
                }

                if (importTitleSection(&section, same_category_count)) {
                    std::vector<stTitleElement>* title_collection =
                        getTitleCollection(info, category);
                    if (title_collection) {
                        for (int slot = section.start_slot;
                             section.end_slot > slot;
                             ++slot) {
                            if (slot >= 0 &&
                                title_collection->size() >
                                    static_cast<std::size_t>(slot) &&
                                (*title_collection)[slot].title_index == -1) {
                                section.end_slot = slot;
                            }
                        }
                    }
                    info->sections.push_back(section);
                    section_ok = 1;
                } else {
                    section_ok = 0;
                }
            } while (section_ok);
        } else if (tag == "[custom condition message]") {
            stCustomConditionMsg message;
            info->custom_condition_messages.clear();
            while (ScanStr(&text) == 1) {
                message.category = convertTabTypeStringToEnum(text.c_str());
                message.condition_id = ScanInt(static_cast<bool*>(0));
                ScanStr(&message.message);
                info->custom_condition_messages.push_back(message);
            }
        } else if (tag == "[adjust list]") {
            stAdjust adjust;
            while (1) {
                adjust.title_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                adjust.adjust_type = ScanInt(static_cast<bool*>(0));
                adjust.adjust_value = ScanInt(static_cast<bool*>(0));
                info->adjusts.push_back(adjust);
            }
        } else if (tag == "[view titlebook eventmark]") {
            int eventmark = 0;
            if (ScanInt(&eventmark)) {
                info->view_titlebook_eventmark =
                    static_cast<uint8_t>(eventmark != 0);
            }
        }
    }

    return 1;
}
