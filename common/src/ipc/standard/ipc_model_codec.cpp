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
}

#define READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        bool ret = (parcel).Read##type((out)); \
        if (!ret) { \
            LOGE("read value failed!"); \
            return failRet; \
        } \
    } while (0)

void IpcModelCodec::DecodeDmDeviceBasicInfo(MessageParcel &parcel, DmDeviceBasicInfo &devInfo)
{
    std::string deviceIdStr = parcel.ReadString();
    if (strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceId failed!");
        return;
    }
    std::string deviceNameStr = parcel.ReadString();
    if (strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceName failed!");
        return;
    }
    devInfo.deviceTypeId = parcel.ReadUint16();
    std::string networkIdStr = parcel.ReadString();
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
    return bRet;
}

void IpcModelCodec::DecodePeerTargetId(MessageParcel &parcel, PeerTargetId &targetId)
{
    targetId.deviceId = parcel.ReadString();
    targetId.brMac = parcel.ReadString();
    targetId.bleMac = parcel.ReadString();
    targetId.wifiIp = parcel.ReadString();
    targetId.wifiPort = parcel.ReadUint16();
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
    bRet = (bRet && parcel.WriteInt32(callee.userId));
    bRet = (bRet && parcel.WriteString(callee.extra));
    return bRet;
}

int32_t IpcModelCodec::DecodeDmDeviceProfileInfoFilterOptions(MessageParcel &parcel,
    DmDeviceProfileInfoFilterOptions &filterOptions)
{
    filterOptions.isCloud = parcel.ReadBool();
    size_t size = parcel.ReadUint32();
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

void IpcModelCodec::DecodeDmProductInfo(MessageParcel &parcel, DmProductInfo &prodInfo)
{
    prodInfo.prodId = parcel.ReadString();
    prodInfo.model = parcel.ReadString();
    prodInfo.prodName = parcel.ReadString();
    prodInfo.prodShortName = parcel.ReadString();
    prodInfo.imageVersion = parcel.ReadString();
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
    devInfo.innerModel = parcel.ReadString();
    devInfo.deviceType = parcel.ReadString();
    devInfo.manufacturer = parcel.ReadString();
    devInfo.deviceName = parcel.ReadString();
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

bool IpcModelCodec::EncodeDmProductInfo(const DmProductInfo &prodInfo, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(prodInfo.prodId));
    bRet = (bRet && parcel.WriteString(prodInfo.model));
    bRet = (bRet && parcel.WriteString(prodInfo.prodName));
    bRet = (bRet && parcel.WriteString(prodInfo.prodShortName));
    bRet = (bRet && parcel.WriteString(prodInfo.imageVersion));
    return bRet;
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
    bRet = (bRet && parcel.WriteString(devInfo.innerModel));
    bRet = (bRet && parcel.WriteString(devInfo.deviceType));
    bRet = (bRet && parcel.WriteString(devInfo.manufacturer));
    bRet = (bRet && parcel.WriteString(devInfo.deviceName));
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
        iconFiter.imageType + UK_SEPARATOR + iconFiter.specName;
}

std::string IpcModelCodec::GetDeviceIconInfoUniqueKey(const DmDeviceIconInfo &iconInfo)
{
    return iconInfo.productId + UK_SEPARATOR + iconInfo.subProductId + UK_SEPARATOR +
        iconInfo.imageType + UK_SEPARATOR + iconInfo.specName;
}

void IpcModelCodec::DecodeDmDeviceIconInfo(MessageParcel &parcel, DmDeviceIconInfo &deviceIconInfo)
{
    deviceIconInfo.productId = parcel.ReadString();
    deviceIconInfo.subProductId = parcel.ReadString();
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

void IpcModelCodec::DecodeDmDeviceIconInfoFilterOptions(MessageParcel &parcel,
    DmDeviceIconInfoFilterOptions &filterOptions)
{
    filterOptions.productId = parcel.ReadString();
    filterOptions.subProductId = parcel.ReadString();
    filterOptions.imageType = parcel.ReadString();
    filterOptions.specName = parcel.ReadString();
}

bool IpcModelCodec::EncodeDmDeviceIconInfoFilterOptions(const DmDeviceIconInfoFilterOptions &filterOptions,
    MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(filterOptions.productId));
    bRet = (bRet && parcel.WriteString(filterOptions.subProductId));
    bRet = (bRet && parcel.WriteString(filterOptions.imageType));
    bRet = (bRet && parcel.WriteString(filterOptions.specName));
    return bRet;
}

void IpcModelCodec::DecodeDmDeviceInfo(MessageParcel &parcel, DmDeviceInfo &devInfo)
{
    std::string deviceIdStr = parcel.ReadString();
    if (strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceId failed!");
        return;
    }
    std::string deviceNameStr = parcel.ReadString();
    if (strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceName failed!");
        return;
    }
    devInfo.deviceTypeId = parcel.ReadUint16();
    std::string networkIdStr = parcel.ReadString();
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
} // namespace DistributedHardware
} // namespace OHOS