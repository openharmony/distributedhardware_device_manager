/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc_model_codec.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* UK_SEPARATOR = "#";
constexpr int32_t MAX_ICON_SIZE = 4 * 1024 * 1024;
constexpr uint32_t IPC_VECTOR_MAX_SIZE = 1000;
}

#define READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        bool ret = (parcel).Read##type((out)); \
        if (!ret) { \
            LOGE("read value failed!"); \
            return failRet; \
        } \
    } while (0)

#define CHECK_CPY_SIZE_RETURN(destSize, srcSize) \
    do { \
        if ((destSize) <= (srcSize)) { \
            LOGE("params size invalid"); \
            return; \
        } \
    } while (0)

//LCOV_EXCL_START
void IpcModelCodec::DecodeDmDeviceBasicInfo(MessageParcel &parcel, DmDeviceBasicInfo &devInfo)
{
    std::string deviceIdStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_ID_LEN, deviceIdStr.size());
    if (strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceId failed!");
        return;
    }
    std::string deviceNameStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_NAME_LEN, deviceNameStr.size());
    if (strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceName failed!");
        return;
    }
    devInfo.deviceTypeId = parcel.ReadUint16();
    std::string networkIdStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_ID_LEN, networkIdStr.size());
    if (strcpy_s(devInfo.networkId, networkIdStr.size() + 1, networkIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s networkId failed!");
        return;
    }
    devInfo.extraData = parcel.ReadString();
}

bool IpcModelCodec::EncodePeerTargetId(const PeerTargetId &targetId, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(targetId.deviceId));
    bRet = (bRet && parcel.WriteString(targetId.brMac));
    bRet = (bRet && parcel.WriteString(targetId.bleMac));
    bRet = (bRet && parcel.WriteString(targetId.wifiIp));
    bRet = (bRet && parcel.WriteUint16(targetId.wifiPort));
    bRet = (bRet && parcel.WriteInt64(targetId.serviceId));
    return bRet;
}

void IpcModelCodec::DecodePeerTargetId(MessageParcel &parcel, PeerTargetId &targetId)
{
    targetId.deviceId = parcel.ReadString();
    targetId.brMac = parcel.ReadString();
    targetId.bleMac = parcel.ReadString();
    targetId.wifiIp = parcel.ReadString();
    targetId.wifiPort = parcel.ReadUint16();
    targetId.serviceId = parcel.ReadInt64();
}

bool IpcModelCodec::EncodeDmAccessCaller(const DmAccessCaller &caller, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(caller.accountId));
    bRet = (bRet && parcel.WriteString(caller.pkgName));
    bRet = (bRet && parcel.WriteString(caller.networkId));
    bRet = (bRet && parcel.WriteInt32(caller.userId));
    bRet = (bRet && parcel.WriteUint64(caller.tokenId));
    bRet = (bRet && parcel.WriteString(caller.extra));
    return bRet;
}

bool IpcModelCodec::EncodeDmAccessCallee(const DmAccessCallee &callee, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(callee.accountId));
    bRet = (bRet && parcel.WriteString(callee.networkId));
    bRet = (bRet && parcel.WriteString(callee.peerId));
    bRet = (bRet && parcel.WriteString(callee.pkgName));
    bRet = (bRet && parcel.WriteInt32(callee.userId));
    bRet = (bRet && parcel.WriteString(callee.extra));
    bRet = (bRet && parcel.WriteUint64(callee.tokenId));
    return bRet;
}

int32_t IpcModelCodec::DecodeDmDeviceProfileInfoFilterOptions(MessageParcel &parcel,
    DmDeviceProfileInfoFilterOptions &filterOptions)
{
    filterOptions.isCloud = parcel.ReadBool();
    uint32_t size = parcel.ReadUint32();
    if (size > MAX_DEVICE_PROFILE_SIZE) {
        LOGE("size more than %{public}d,", MAX_DEVICE_PROFILE_SIZE);
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (size > 0) {
        for (uint32_t i = 0; i < size; i++) {
            std::string item = parcel.ReadString();
            filterOptions.deviceIdList.emplace_back(item);
        }
    }
    return DM_OK;
}

bool IpcModelCodec::EncodeDmDeviceProfileInfoFilterOptions(const DmDeviceProfileInfoFilterOptions &filterOptions,
    MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteBool(filterOptions.isCloud));
    uint32_t size = filterOptions.deviceIdList.size();
    bRet = (bRet && parcel.WriteUint32(size));
    if (bRet && size > 0) {
        for (const auto& item : filterOptions.deviceIdList) {
            bRet = (bRet && parcel.WriteString(item));
        }
    }
    return bRet;
}

void IpcModelCodec::DecodeDmServiceProfileInfo(MessageParcel &parcel, DmServiceProfileInfo &svrInfo)
{
    svrInfo.deviceId = parcel.ReadString();
    svrInfo.serviceId = parcel.ReadString();
    svrInfo.serviceType = parcel.ReadString();
    int32_t num = parcel.ReadInt32();
    if (num > 0 && num <= MAX_DEVICE_PROFILE_SIZE) {
        for (int32_t i = 0; i < num; ++i) {
            std::string key = parcel.ReadString();
            std::string value = parcel.ReadString();
            svrInfo.data[key] = value;
        }
    }
}

void IpcModelCodec::DecodeDmServiceProfileInfos(MessageParcel &parcel, std::vector<DmServiceProfileInfo> &svrInfos)
{
    int32_t svrNum = parcel.ReadInt32();
    if (svrNum > 0 && svrNum <= MAX_DEVICE_PROFILE_SIZE) {
        for (int32_t i = 0; i < svrNum; ++i) {
            DmServiceProfileInfo svrInfo;
            DecodeDmServiceProfileInfo(parcel, svrInfo);
            svrInfos.emplace_back(svrInfo);
        }
    }
}

void IpcModelCodec::DecodeDmDeviceProfileInfo(MessageParcel &parcel, DmDeviceProfileInfo &devInfo)
{
    devInfo.deviceId = parcel.ReadString();
    devInfo.deviceSn = parcel.ReadString();
    devInfo.mac = parcel.ReadString();
    devInfo.model = parcel.ReadString();
    devInfo.internalModel = parcel.ReadString();
    devInfo.deviceType = parcel.ReadString();
    devInfo.manufacturer = parcel.ReadString();
    devInfo.deviceName = parcel.ReadString();
    devInfo.productName = parcel.ReadString();
    devInfo.productId = parcel.ReadString();
    devInfo.subProductId = parcel.ReadString();
    devInfo.sdkVersion = parcel.ReadString();
    devInfo.bleMac = parcel.ReadString();
    devInfo.sleMac = parcel.ReadString();
    devInfo.firmwareVersion = parcel.ReadString();
    devInfo.hardwareVersion = parcel.ReadString();
    devInfo.softwareVersion = parcel.ReadString();
    devInfo.protocolType = parcel.ReadInt32();
    devInfo.setupType = parcel.ReadInt32();
    devInfo.wiseDeviceId = parcel.ReadString();
    devInfo.wiseUserId = parcel.ReadString();
    devInfo.registerTime = parcel.ReadString();
    devInfo.modifyTime = parcel.ReadString();
    devInfo.shareTime = parcel.ReadString();
    devInfo.isLocalDevice = parcel.ReadBool();
    DecodeDmServiceProfileInfos(parcel, devInfo.services);
}

bool IpcModelCodec::EncodeDmServiceProfileInfo(const DmServiceProfileInfo &svrInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(svrInfo.deviceId));
    bRet = (bRet && parcel.WriteString(svrInfo.serviceId));
    bRet = (bRet && parcel.WriteString(svrInfo.serviceType));
    if (bRet) {
        if (!parcel.WriteInt32((int32_t)svrInfo.data.size())) {
            return false;
        }
        for (const auto &[key, value] : svrInfo.data) {
            if (!parcel.WriteString(key)) {
                return false;
            }
            if (!parcel.WriteString(value)) {
                return false;
            }
        }
    }
    return bRet;
}

bool IpcModelCodec::EncodeDmServiceProfileInfos(const std::vector<DmServiceProfileInfo> &svrInfos,
    MessageParcel &parcel)
{
    if (!parcel.WriteInt32((int32_t)svrInfos.size())) {
        return false;
    }
    for (const auto &svrInfo : svrInfos) {
        if (!EncodeDmServiceProfileInfo(svrInfo, parcel)) {
            LOGE("write dm service profile info failed");
            return false;
        }
    }
    return true;
}

bool IpcModelCodec::EncodeDmDeviceProfileInfo(const DmDeviceProfileInfo &devInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(devInfo.deviceId));
    bRet = (bRet && parcel.WriteString(devInfo.deviceSn));
    bRet = (bRet && parcel.WriteString(devInfo.mac));
    bRet = (bRet && parcel.WriteString(devInfo.model));
    bRet = (bRet && parcel.WriteString(devInfo.internalModel));
    bRet = (bRet && parcel.WriteString(devInfo.deviceType));
    bRet = (bRet && parcel.WriteString(devInfo.manufacturer));
    bRet = (bRet && parcel.WriteString(devInfo.deviceName));
    bRet = (bRet && parcel.WriteString(devInfo.productName));
    bRet = (bRet && parcel.WriteString(devInfo.productId));
    bRet = (bRet && parcel.WriteString(devInfo.subProductId));
    bRet = (bRet && parcel.WriteString(devInfo.sdkVersion));
    bRet = (bRet && parcel.WriteString(devInfo.bleMac));
    bRet = (bRet && parcel.WriteString(devInfo.sleMac));
    bRet = (bRet && parcel.WriteString(devInfo.firmwareVersion));
    bRet = (bRet && parcel.WriteString(devInfo.hardwareVersion));
    bRet = (bRet && parcel.WriteString(devInfo.softwareVersion));
    bRet = (bRet && parcel.WriteInt32(devInfo.protocolType));
    bRet = (bRet && parcel.WriteInt32(devInfo.setupType));
    bRet = (bRet && parcel.WriteString(devInfo.wiseDeviceId));
    bRet = (bRet && parcel.WriteString(devInfo.wiseUserId));
    bRet = (bRet && parcel.WriteString(devInfo.registerTime));
    bRet = (bRet && parcel.WriteString(devInfo.modifyTime));
    bRet = (bRet && parcel.WriteString(devInfo.shareTime));
    bRet = (bRet && parcel.WriteBool(devInfo.isLocalDevice));
    bRet = (bRet && EncodeDmServiceProfileInfos(devInfo.services, parcel));
    return bRet;
}

std::string IpcModelCodec::GetDeviceIconInfoUniqueKey(const DmDeviceIconInfoFilterOptions &iconFiter)
{
    return iconFiter.productId + UK_SEPARATOR + iconFiter.subProductId + UK_SEPARATOR +
        iconFiter.internalModel + UK_SEPARATOR + iconFiter.imageType + UK_SEPARATOR + iconFiter.specName;
}

std::string IpcModelCodec::GetDeviceIconInfoUniqueKey(const DmDeviceIconInfo &iconInfo)
{
    return iconInfo.productId + UK_SEPARATOR + iconInfo.subProductId + UK_SEPARATOR +
        iconInfo.internalModel + UK_SEPARATOR + iconInfo.imageType + UK_SEPARATOR + iconInfo.specName;
}

void IpcModelCodec::DecodeDmDeviceIconInfo(MessageParcel &parcel, DmDeviceIconInfo &deviceIconInfo)
{
    deviceIconInfo.productId = parcel.ReadString();
    deviceIconInfo.subProductId = parcel.ReadString();
    deviceIconInfo.internalModel = parcel.ReadString();
    deviceIconInfo.imageType = parcel.ReadString();
    deviceIconInfo.specName = parcel.ReadString();
    deviceIconInfo.version = parcel.ReadString();
    deviceIconInfo.url = parcel.ReadString();
    int32_t length = parcel.ReadInt32();
    if (length > 0 && length <= MAX_ICON_SIZE) {
        const unsigned char *buffer = nullptr;
        if ((buffer = reinterpret_cast<const unsigned char *>(parcel.ReadRawData((size_t)length))) == nullptr) {
            LOGE("read raw data failed, length = %{public}d", length);
            return;
        }
        std::vector<uint8_t> icon(buffer, buffer + length);
        deviceIconInfo.icon = icon;
    }
}

bool IpcModelCodec::EncodeDmDeviceIconInfo(const DmDeviceIconInfo &deviceIconInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(deviceIconInfo.productId));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.subProductId));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.internalModel));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.imageType));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.specName));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.version));
    bRet = (bRet && parcel.WriteString(deviceIconInfo.url));
    int32_t length = static_cast<int32_t>(deviceIconInfo.icon.size());
    bRet = (bRet && parcel.WriteInt32(length));
    if (bRet && length > 0) {
        const unsigned char *buffer = reinterpret_cast<const unsigned char *>(deviceIconInfo.icon.data());
        bRet = (bRet && parcel.WriteRawData(buffer, length));
    }
    return bRet;
}

bool IpcModelCodec::EncodeServiceIds(const std::vector<int64_t> &serviceIds, MessageParcel &parcel)
{
    int32_t size = static_cast<int32_t>(serviceIds.size());
    if (!parcel.WriteInt32(size)) {
        return false;
    }
    for (const auto &id : serviceIds) {
        if (!parcel.WriteInt64(id)) {
            return false;
        }
    }
    return true;
}

void IpcModelCodec::DecodeServiceIds(std::vector<int64_t> &serviceIds, MessageParcel &parcel)
{
    const int32_t MAX_SERVICE_IDS_SIZE = 1024;
    int32_t size = parcel.ReadInt32();
    if (size > MAX_SERVICE_IDS_SIZE) {
        LOGE("DecodeServiceIds: size %d", size);
        return;
    }
    serviceIds.resize(size);
    for (int32_t i = 0; i < size; ++i) {
        serviceIds[i] = parcel.ReadInt64();
    }
}

void IpcModelCodec::DecodeDmDeviceIconInfoFilterOptions(MessageParcel &parcel,
    DmDeviceIconInfoFilterOptions &filterOptions)
{
    filterOptions.productId = parcel.ReadString();
    filterOptions.subProductId = parcel.ReadString();
    filterOptions.internalModel = parcel.ReadString();
    filterOptions.imageType = parcel.ReadString();
    filterOptions.specName = parcel.ReadString();
}

bool IpcModelCodec::EncodeDmDeviceIconInfoFilterOptions(const DmDeviceIconInfoFilterOptions &filterOptions,
    MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(filterOptions.productId));
    bRet = (bRet && parcel.WriteString(filterOptions.subProductId));
    bRet = (bRet && parcel.WriteString(filterOptions.internalModel));
    bRet = (bRet && parcel.WriteString(filterOptions.imageType));
    bRet = (bRet && parcel.WriteString(filterOptions.specName));
    return bRet;
}

void IpcModelCodec::DecodeDmDeviceInfo(MessageParcel &parcel, DmDeviceInfo &devInfo)
{
    std::string deviceIdStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_ID_LEN, deviceIdStr.size());
    if (strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceId failed!");
        return;
    }
    std::string deviceNameStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_NAME_LEN, deviceNameStr.size());
    if (strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceName failed!");
        return;
    }
    devInfo.deviceTypeId = parcel.ReadUint16();
    std::string networkIdStr = parcel.ReadString();
    CHECK_CPY_SIZE_RETURN(DM_MAX_DEVICE_ID_LEN, networkIdStr.size());
    if (strcpy_s(devInfo.networkId, networkIdStr.size() + 1, networkIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s networkId failed!");
        return;
    }
    devInfo.range = parcel.ReadInt32();
    devInfo.networkType = parcel.ReadInt32();
    devInfo.authForm = static_cast<DmAuthForm>(parcel.ReadInt32());
    devInfo.extraData = parcel.ReadString();
}

bool IpcModelCodec::EncodeLocalServiceInfo(const DMLocalServiceInfo &serviceInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(serviceInfo.bundleName));
    bRet = (bRet && parcel.WriteInt32(serviceInfo.authBoxType));
    bRet = (bRet && parcel.WriteInt32(serviceInfo.authType));
    bRet = (bRet && parcel.WriteInt32(serviceInfo.pinExchangeType));
    bRet = (bRet && parcel.WriteString(serviceInfo.pinCode));
    bRet = (bRet && parcel.WriteString(serviceInfo.description));
    bRet = (bRet && parcel.WriteString(serviceInfo.extraInfo));
    return bRet;
}

bool IpcModelCodec::EncodeLocalServiceInfos(const std::vector<DMLocalServiceInfo> &serviceInfos, MessageParcel &parcel)
{
    uint32_t num = static_cast<uint32_t>(serviceInfos.size());
    if (!parcel.WriteUint32(num)) {
        LOGE("WriteUint32 num failed");
        return false;
    }
    bool bRet = true;
    if (num > IPC_VECTOR_MAX_SIZE) {
        LOGE("num is Invalid value, num = %{public}u", num);
        return false;
    }
    for (uint32_t k = 0; k < num; k++) {
        DMLocalServiceInfo serviceInfo = serviceInfos[k];
        bRet = EncodeLocalServiceInfo(serviceInfo, parcel);
        if (!bRet) {
            LOGE("EncodeLocalServiceInfo failed");
            break;
        }
    }
    return bRet;
}

bool IpcModelCodec::DecodeLocalServiceInfo(MessageParcel &parcel, DMLocalServiceInfo &serviceInfo)
{
    READ_HELPER_RET(parcel, String, serviceInfo.bundleName, false);
    READ_HELPER_RET(parcel, Int32, serviceInfo.authBoxType, false);
    READ_HELPER_RET(parcel, Int32, serviceInfo.authType, false);
    READ_HELPER_RET(parcel, Int32, serviceInfo.pinExchangeType, false);
    READ_HELPER_RET(parcel, String, serviceInfo.pinCode, false);
    READ_HELPER_RET(parcel, String, serviceInfo.description, false);
    READ_HELPER_RET(parcel, String, serviceInfo.extraInfo, false);
    return true;
}

bool IpcModelCodec::DecodeLocalServiceInfos(MessageParcel &parcel, std::vector<DMLocalServiceInfo> &serviceInfos)
{
    uint32_t num = 0;
    READ_HELPER_RET(parcel, Uint32, num, false);
    bool bRet = true;
    if (num > IPC_VECTOR_MAX_SIZE || num < 0) {
        LOGE("num is Invalid value, num = %{public}u", num);
        return false;
    }
    for (uint32_t k = 0; k < num; k++) {
        DMLocalServiceInfo serviceInfo;
        bRet = DecodeLocalServiceInfo(parcel, serviceInfo);
        if (!bRet) {
            LOGE("DecodeLocalServiceInfo failed");
            break;
        }
        serviceInfos.emplace_back(serviceInfo);
    }
    return bRet;
}

bool IpcModelCodec::EncodeNetworkIdQueryFilter(const NetworkIdQueryFilter &queryFilter, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(queryFilter.wiseDeviceId));
    bRet = (bRet && parcel.WriteInt32(queryFilter.onlineStatus));
    bRet = (bRet && parcel.WriteString(queryFilter.deviceType));
    bRet = (bRet && parcel.WriteString(queryFilter.deviceProductId));
    bRet = (bRet && parcel.WriteString(queryFilter.deviceModel));
    return bRet;
}

bool IpcModelCodec::DecodeNetworkIdQueryFilter(MessageParcel &parcel, NetworkIdQueryFilter &queryFilter)
{
    READ_HELPER_RET(parcel, String, queryFilter.wiseDeviceId, false);
    READ_HELPER_RET(parcel, Int32, queryFilter.onlineStatus, false);
    READ_HELPER_RET(parcel, String, queryFilter.deviceType, false);
    READ_HELPER_RET(parcel, String, queryFilter.deviceProductId, false);
    READ_HELPER_RET(parcel, String, queryFilter.deviceModel, false);
    return true;
}

bool IpcModelCodec::EncodeStringVector(const std::vector<std::string> &vec, MessageParcel &parcel)
{
    uint32_t num = static_cast<uint32_t>(vec.size());
    if (!parcel.WriteUint32(num)) {
        LOGE("WriteUint32 num failed");
        return false;
    }
    bool bRet = true;
    for (uint32_t k = 0; k < num; k++) {
        std::string str = vec[k];
        bRet = parcel.WriteString(vec[k]);
        if (!bRet) {
            LOGE("EncodeStringVector failed");
            break;
        }
    }
    return bRet;
}

bool IpcModelCodec::DecodeStringVector(MessageParcel &parcel, std::vector<std::string> &vec)
{
    uint32_t num = 0;
    READ_HELPER_RET(parcel, Uint32, num, false);
    if (num > IPC_VECTOR_MAX_SIZE || num < 0) {
        LOGE("num is Invalid value, num = %{public}u", num);
        return false;
    }
    for (uint32_t k = 0; k < num; k++) {
        std::string str = "";
        READ_HELPER_RET(parcel, String, str, false);
        vec.emplace_back(str);
    }
    return true;
}

bool IpcModelCodec::EncodeSrvDiscParam(const DiscoveryServiceParam &param, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(param.serviceName));
    bRet = (bRet && parcel.WriteString(param.serviceType));
    bRet = (bRet && parcel.WriteInt32(param.discoveryServiceId));
    bRet = (bRet && parcel.WriteInt32(param.freq));
    bRet = (bRet && parcel.WriteInt32(param.medium));
    bRet = (bRet && parcel.WriteInt32(param.mode));
    return bRet;
}

bool IpcModelCodec::DecodeSrvDiscParam(MessageParcel &parcel, DiscoveryServiceParam &param)
{
    READ_HELPER_RET(parcel, String, param.serviceName, false);
    READ_HELPER_RET(parcel, String, param.serviceType, false);
    READ_HELPER_RET(parcel, Int32, param.discoveryServiceId, false);
    int32_t freq = 0;
    READ_HELPER_RET(parcel, Int32, freq, false);
    param.freq = static_cast<DmExchangeFreq>(freq);
    int32_t medium = 0;
    READ_HELPER_RET(parcel, Int32, medium, false);
    param.medium = static_cast<DMSrvMediumType>(medium);
    int32_t mode = 0;
    READ_HELPER_RET(parcel, Int32, mode, false);
    param.mode = static_cast<DMSrvDiscoveryMode>(mode);
    return true;
}

bool IpcModelCodec::EncodeSrvDiscServiceInfo(const DiscoveryServiceInfo &serviceInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(serviceInfo.pkgName));
    bRet = (bRet && parcel.WriteInt64(serviceInfo.serviceInfo.serviceId));
    bRet = (bRet && parcel.WriteString(serviceInfo.serviceInfo.serviceType));
    bRet = (bRet && parcel.WriteString(serviceInfo.serviceInfo.serviceName));
    bRet = (bRet && parcel.WriteString(serviceInfo.serviceInfo.serviceDisplayName));
    return bRet;
}

bool IpcModelCodec::DecodeSrvDiscServiceInfo(MessageParcel &parcel, DiscoveryServiceInfo &serviceInfo)
{
    READ_HELPER_RET(parcel, String, serviceInfo.pkgName, false);
    READ_HELPER_RET(parcel, Int64, serviceInfo.serviceInfo.serviceId, false);
    READ_HELPER_RET(parcel, String, serviceInfo.serviceInfo.serviceType, false);
    READ_HELPER_RET(parcel, String, serviceInfo.serviceInfo.serviceName, false);
    READ_HELPER_RET(parcel, String, serviceInfo.serviceInfo.serviceDisplayName, false);
    return true;
}
//LCOV_EXCL_STOP

bool IpcModelCodec::EncodeServiceRegInfo(const ServiceRegInfo &serRegInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteInt64(serRegInfo.serviceInfo.serviceId));
    bRet = (bRet && parcel.WriteString(serRegInfo.serviceInfo.serviceType));
    bRet = (bRet && parcel.WriteString(serRegInfo.serviceInfo.serviceName));
    bRet = (bRet && parcel.WriteString(serRegInfo.serviceInfo.serviceDisplayName));
    bRet = (bRet && parcel.WriteString(serRegInfo.customData));
    bRet = (bRet && parcel.WriteUint32(serRegInfo.dataLen));
    return bRet;
}

bool IpcModelCodec::DecodeServiceRegInfo(MessageParcel &parcel, ServiceRegInfo &serRegInfo)
{
    READ_HELPER_RET(parcel, Int64, serRegInfo.serviceInfo.serviceId, false);
    READ_HELPER_RET(parcel, String, serRegInfo.serviceInfo.serviceType, false);
    READ_HELPER_RET(parcel, String, serRegInfo.serviceInfo.serviceName, false);
    READ_HELPER_RET(parcel, String, serRegInfo.serviceInfo.serviceDisplayName, false);
    READ_HELPER_RET(parcel, String, serRegInfo.customData, false);
    READ_HELPER_RET(parcel, Uint32, serRegInfo.dataLen, false);
    return true;
}

bool IpcModelCodec::EncodePublishServiceParam(const PublishServiceParam &publishServiceParam, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteInt64(publishServiceParam.serviceInfo.serviceId));
    bRet = (bRet && parcel.WriteString(publishServiceParam.serviceInfo.serviceType));
    bRet = (bRet && parcel.WriteString(publishServiceParam.serviceInfo.serviceName));
    bRet = (bRet && parcel.WriteString(publishServiceParam.serviceInfo.serviceDisplayName));
    bRet = (bRet && parcel.WriteInt32(publishServiceParam.regServiceId));
    bRet = (bRet && parcel.WriteInt32(static_cast<int32_t>(publishServiceParam.discoverMode)));
    bRet = (bRet && parcel.WriteInt32(static_cast<int32_t>(publishServiceParam.media)));
    bRet = (bRet && parcel.WriteInt32(static_cast<int32_t>(publishServiceParam.freq)));
    return bRet;
}

bool IpcModelCodec::DecodePublishServiceParam(MessageParcel &parcel, PublishServiceParam &publishServiceParam)
{
    READ_HELPER_RET(parcel, Int64, publishServiceParam.serviceInfo.serviceId, false);
    READ_HELPER_RET(parcel, String, publishServiceParam.serviceInfo.serviceType, false);
    READ_HELPER_RET(parcel, String, publishServiceParam.serviceInfo.serviceName, false);
    READ_HELPER_RET(parcel, String, publishServiceParam.serviceInfo.serviceDisplayName, false);
    READ_HELPER_RET(parcel, Int32, publishServiceParam.regServiceId, false);
    int32_t discoverMode = static_cast<int32_t>(DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE);
    READ_HELPER_RET(parcel, Int32, discoverMode, false);
    publishServiceParam.discoverMode = static_cast<DMSrvDiscoveryMode>(discoverMode);
    int32_t media = static_cast<int32_t>(DMSrvMediumType::SERVICE_MEDIUM_TYPE_AUTO);
    READ_HELPER_RET(parcel, Int32, media, false);
    publishServiceParam.media = static_cast<DMSrvMediumType>(media);
    int32_t freq = static_cast<int32_t>(DmExchangeFreq::DM_LOW);
    READ_HELPER_RET(parcel, Int32, freq, false);
    publishServiceParam.freq = static_cast<DmExchangeFreq>(freq);
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
