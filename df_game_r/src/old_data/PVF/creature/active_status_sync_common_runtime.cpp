#include "old_data/PVF/creature/active_status_sync_server.h"

#include <algorithm>

// IDA_EA: 0x88924C1
// IDA_NAME: _Z26GetUniformVelocityFloatSDCffii
// DEMANGLED: GetUniformVelocityFloatSDC(float,float,int,int)
float GetUniformVelocityFloatSDC(float first,
                                 float last,
                                 int position,
                                 int maximum_position) {
    if (position >= maximum_position) {
        return last;
    }
    if (position > 0) {
        return static_cast<float>(position) * (last - first) /
                   static_cast<float>(maximum_position) +
               first;
    }
    return first;
}

namespace {

bool HasSDCObjectType(uint32_t object_type_mask, uint32_t required_mask) {
    // IDA_EA: 0x80950E8, _SDC::isObjectType(int,int).
    return (object_type_mask & required_mask) == required_mask;
}

}  // namespace

// IDA_EA: 0x808D268
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon29getActiveStatusDurationClientEiiP15IRDActiveObject
int SDCActiveStatusSyncCommon::getActiveStatusDurationClient(
    int,
    int,
    IRDActiveObject*) {
    return 0;
}

// IDA_EA: 0x808D272
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon29getActiveStatusDurationServerEiiiii
int SDCActiveStatusSyncCommon::getActiveStatusDurationServer(
    int active_status,
    int level,
    int object_type_mask,
    int,
    int raw_31) {
    int duration = 0;
    const unsigned int status = static_cast<unsigned int>(active_status);
    if (status <= 0x11u) {
        const unsigned int status_bit = 1u << status;
        if ((status_bit & 0x385AAu) != 0u) {
            duration = static_cast<int>(
                getActiveStatusPower1(active_status, level) * 1000.0f);
        } else if ((status_bit & 0x3A55u) != 0u) {
            duration = 1000 * ACTIVESTATUS_DURATION[status];
        } else if ((status_bit & 0x4000u) != 0u) {
            duration = 0;
        }
    }

    if (HasSDCObjectType(static_cast<uint32_t>(object_type_mask), 529u)) {
        const float half = static_cast<float>(raw_31) / 2.0f;
        const float rate = half / static_cast<float>(raw_31);
        duration -= static_cast<int>(static_cast<float>(duration) * rate);
    }
    return duration < 0 ? 0 : duration;
}

// IDA_EA: 0x808D37A
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon21getActiveStatusPower1Eii
float SDCActiveStatusSyncCommon::getActiveStatusPower1(int active_status,
                                                        int level) {
    return GetUniformVelocityFloatSDC(ACTIVESTATUS_POWER1[2 * active_status],
                                      ACTIVESTATUS_POWER1[2 * active_status + 1],
                                      level - 1,
                                      ACTIVESTATUS_MAXLEVEL - 1);
}

// IDA_EA: 0x808D3DC
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon21getActiveStatusPower2Eii
float SDCActiveStatusSyncCommon::getActiveStatusPower2(int active_status,
                                                        int level) {
    return GetUniformVelocityFloatSDC(ACTIVESTATUS_POWER2[2 * active_status],
                                      ACTIVESTATUS_POWER2[2 * active_status + 1],
                                      level - 1,
                                      ACTIVESTATUS_MAXLEVEL - 1);
}

// IDA_EA: 0x808D43E
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon27getActiveStatusEtcParameterEiii
float SDCActiveStatusSyncCommon::getActiveStatusEtcParameter(
    int active_status,
    int level,
    int parameter_index) {
    const int index = 8 * active_status + 2 * parameter_index;
    return GetUniformVelocityFloatSDC(ACTIVESTATUS_ETC_PARAMETERS[index],
                                      ACTIVESTATUS_ETC_PARAMETERS[index + 1],
                                      level - 1,
                                      ACTIVESTATUS_MAXLEVEL - 1);
}

// IDA_EA: 0x808D4B4
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon17makeEtcParametersE17ENUM_ACTIVESTATUSiPvPSt6vectorIiSaIiEEPi
int SDCActiveStatusSyncCommon::makeEtcParameters(
    ENUM_ACTIVESTATUS active_status,
    int level,
    void* active_status_data,
    std::vector<int>* parameters,
    int* duration) {
    if (duration) {
        *duration = getDuration(active_status, level, active_status_data);
    }
    return makeEtcParameters(active_status, level, *parameters);
}

// IDA_EA: 0x808D50E
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon17makeEtcParametersE17ENUM_ACTIVESTATUSiRSt6vectorIiSaIiEE
int SDCActiveStatusSyncCommon::makeEtcParameters(
    ENUM_ACTIVESTATUS active_status,
    int level,
    std::vector<int>& parameters) {
    const int status = static_cast<int>(active_status);
    switch (status) {
        case 0:
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 0)));
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 1)));
            break;
        case 2:
        case 6:
        case 7:
        case 10:
        case 11:
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 0)));
            break;
        case 4:
        case 13:
            for (int index = 0; index != 4; ++index) {
                parameters.push_back(static_cast<int>(
                    getActiveStatusEtcParameter(status, level, index)));
            }
            break;
        case 5:
        case 12:
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 0)));
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 1)));
            break;
        case 9:
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 0)));
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 1)));
            parameters.push_back(1);
            parameters.push_back(3000);
            break;
        case 15:
            parameters.push_back(1);
            break;
        case 17:
            parameters.push_back(
                static_cast<int>(getActiveStatusEtcParameter(status, level, 0)));
            break;
        default:
            return status;
    }
    return status;
}

// IDA_EA: 0x808DC7A
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon11getDurationE17ENUM_ACTIVESTATUSiPv
int SDCActiveStatusSyncCommon::getDuration(ENUM_ACTIVESTATUS active_status,
                                           int level,
                                           void* active_status_data) {
    const STSDCActiveStatusSyncDataSend* const data =
        static_cast<const STSDCActiveStatusSyncDataSend*>(active_status_data);
    return getActiveStatusDurationServer(static_cast<int>(active_status),
                                         level,
                                         static_cast<int>(data->object_type_mask),
                                         data->raw_2d,
                                         data->raw_31);
}

// IDA_EA: 0x808DCC4
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon15getPoisonDamageEP29STSDCActiveStatusSyncDataSendPvi
int SDCActiveStatusSyncCommon::getPoisonDamage(
    STSDCActiveStatusSyncDataSend* data,
    void*,
    int elapsed) {
    if (data->source_level > 99) {
        return 0;
    }
    const float damage = GetUniformVelocityFloatSDC(
        0.0f, static_cast<float>(data->damage_value), elapsed, data->duration);
    if (data->is_user_to_user) {
        return static_cast<int>(damage *
                                getUserToUserDamageRate(data->source_kind == 10));
    }
    return static_cast<int>(damage);
}

// IDA_EA: 0x808DD5C
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon16getDiseaseDamageEP29STSDCActiveStatusSyncDataSendi
int SDCActiveStatusSyncCommon::getDiseaseDamage(
    STSDCActiveStatusSyncDataSend* data,
    int elapsed) {
    if (data->source_level > 99) {
        return 0;
    }
    const int damage = static_cast<int>(GetUniformVelocityFloatSDC(
        0.0f, static_cast<float>(data->damage_value), elapsed, data->duration));
    if (data->is_user_to_user) {
        return static_cast<int>(static_cast<float>(damage) *
                                getUserToUserDamageRate(data->source_kind == 10));
    }
    return damage;
}

// IDA_EA: 0x808DE08
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon13getBurnDamageERbP29STSDCActiveStatusSyncDataSendPvi
int SDCActiveStatusSyncCommon::getBurnDamage(
    bool& valid,
    STSDCActiveStatusSyncDataSend* data,
    void*,
    int elapsed) {
    const float damage = GetUniformVelocityFloatSDC(
        0.0f, static_cast<float>(data->damage_value), elapsed, data->duration);
    if (data->damage_value > 0) {
        valid = false;
    }
    if (data->is_user_to_user) {
        return static_cast<int>(damage *
                                getUserToUserDamageRate(data->source_kind == 10));
    }
    return static_cast<int>(damage);
}

// IDA_EA: 0x808DE9E
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon23getBleedingDamageClientEP29STSDCActiveStatusSyncDataSendbPv
int SDCActiveStatusSyncCommon::getBleedingDamageClient(
    STSDCActiveStatusSyncDataSend* data,
    bool,
    void*) {
    const int divisor = std::max(1, data->duration / 500);
    return static_cast<int>(static_cast<float>(data->damage_value) /
                            static_cast<float>(divisor));
}

// IDA_EA: 0x808DF1A
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon23getBleedingDamageServerEP29STSDCActiveStatusSyncDataSend
int SDCActiveStatusSyncCommon::getBleedingDamageServer(
    STSDCActiveStatusSyncDataSend* data) {
    return static_cast<int>(static_cast<float>(data->damage_value) * 1.5f);
}

// IDA_EA: 0x808DF5C
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon23getUserToUserDamageRateEb
float SDCActiveStatusSyncCommon::getUserToUserDamageRate(bool death_tower) {
    return death_tower ? ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE_DEATH_TOWER
                       : ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE;
}

// IDA_EA: 0x808DF82
// IDA_NAME: _ZN25SDCActiveStatusSyncCommon35getActiveStatusDamageDurationForSDCEciPiS0_P29STSDCActiveStatusSyncDataSendPv
void SDCActiveStatusSyncCommon::getActiveStatusDamageDurationForSDC(
    char active_status,
    int level,
    int* duration,
    int* damage,
    STSDCActiveStatusSyncDataSend* data,
    void*) {
    if (!duration || !damage || !data) {
        return;
    }

    STSDCActiveStatusSyncDataSend local = *data;
    std::vector<int> parameters;
    makeEtcParameters(static_cast<ENUM_ACTIVESTATUS>(active_status),
                      level,
                      &local,
                      &parameters,
                      duration);
    if (!parameters.empty()) {
        local.damage_value = parameters[0];
        local.duration = *duration;
    }

    switch (active_status) {
        case 2:
            *damage = getPoisonDamage(&local, 0, local.duration);
            break;
        case 6:
        case 7:
            *damage = local.damage_value;
            break;
        case 9: {
            bool valid = false;
            *damage = getBurnDamage(valid, &local, 0, local.duration);
            break;
        }
        case 11:
            *damage = getBleedingDamageServer(&local);
            break;
        default:
            break;
    }
}
