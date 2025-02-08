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

namespace OHOS {
namespace DistributedHardware {
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
} // namespace DistributedHardware
} // namespace OHOS