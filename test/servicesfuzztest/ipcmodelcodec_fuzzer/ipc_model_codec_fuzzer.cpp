/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ipc_model_codec_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "ipc_model_codec.h"


namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 64;
void DecodeDmDeviceBasicInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(uint16_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint16(fdp.ConsumeIntegral<uint16_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    DmDeviceBasicInfo devInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceBasicInfo(parcel, devInfo);
}

void EncodePeerTargetIdFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(uint16_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    PeerTargetId targetId;
    targetId.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    targetId.brMac = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    targetId.bleMac = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    targetId.wifiIp = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    targetId.wifiPort = fdp.ConsumeIntegral<uint16_t>();

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodePeerTargetId(targetId, parcel);
}

void DecodePeerTargetIdFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(uint16_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint16(fdp.ConsumeIntegral<uint16_t>());

    PeerTargetId targetId;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodePeerTargetId(parcel, targetId);
}

void EncodeDmAccessCallerFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + sizeof(uint64_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmAccessCaller caller;
    caller.accountId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    caller.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    caller.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    caller.userId = fdp.ConsumeIntegral<int32_t>();
    caller.tokenId = fdp.ConsumeIntegral<uint64_t>();
    caller.extra = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmAccessCaller(caller, parcel);
}

void EncodeDmAccessCalleeFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + sizeof(uint64_t) + MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmAccessCallee callee;
    callee.accountId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    callee.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    callee.peerId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    callee.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    callee.userId = fdp.ConsumeIntegral<int32_t>();
    callee.extra = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    callee.tokenId = fdp.ConsumeIntegral<uint64_t>();

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmAccessCallee(callee, parcel);
}

void DecodeDmDeviceProfileInfoFilterOptionsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t) + sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteBool(fdp.ConsumeBool());
    uint32_t minRangeSize = 0;
    uint32_t middleRangeSize = 500;
    uint32_t maxRangeSize = 1000;
    uint32_t listSize = fdp.ConsumeIntegralInRange<uint32_t>(middleRangeSize, maxRangeSize);
    parcel.WriteUint32(listSize);

    DmDeviceProfileInfoFilterOptions filterOptions;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceProfileInfoFilterOptions(parcel, filterOptions);

    listSize = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, middleRangeSize);
    parcel.WriteUint32(listSize);
    ipcModelCodec->DecodeDmDeviceProfileInfoFilterOptions(parcel, filterOptions);
}

void EncodeDmDeviceProfileInfoFilterOptionsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t) + MAX_STRING_LENGTH)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmDeviceProfileInfoFilterOptions filterOptions;
    filterOptions.isCloud = fdp.ConsumeBool();
    uint32_t minRangeSize = 0;
    uint32_t maxRangeSize = 100;
    uint32_t listSize = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, maxRangeSize);
    for (uint32_t i = 0; i < listSize; ++i) {
        filterOptions.deviceIdList.emplace_back(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }
    MessageParcel parcel;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmDeviceProfileInfoFilterOptions(filterOptions, parcel);
}

void DecodeDmServiceProfileInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    int32_t minRangeSize = 0;
    int32_t maxRangeSize = 100;
    int32_t num = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, maxRangeSize);
    parcel.WriteInt32(num);
    for (int32_t i = 0; i < num; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }
    DmServiceProfileInfo svrInfo;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmServiceProfileInfo(parcel, svrInfo);
}

void DecodeDmServiceProfileInfosFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    int32_t minRangeSize = 0;
    int32_t middleRangeSize = 50;
    int32_t maxRangeSize = 100;
    int32_t svrNum = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, maxRangeSize);
    parcel.WriteInt32(svrNum);
    for (int32_t i = 0; i < svrNum; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        int32_t num = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, middleRangeSize);
        parcel.WriteInt32(num);
        for (int32_t j = 0; j < num; ++j) {
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        }
    }
    std::vector<DmServiceProfileInfo> svrInfos;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmServiceProfileInfos(parcel, svrInfos);
}

void DecodeDmDeviceProfileInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH * 22;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteBool(fdp.ConsumeBool());
    int32_t serviceCount = 10;
    parcel.WriteInt32(serviceCount);
    for (int32_t i = 0; i < serviceCount; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        int32_t dataCount = 5;
        parcel.WriteInt32(dataCount);
        for (int32_t j = 0; j < dataCount; ++j) {
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
            parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        }
    }
    DmDeviceProfileInfo devInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceProfileInfo(parcel, devInfo);
}

void EncodeDmServiceProfileInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmServiceProfileInfo svrInfo;
    svrInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    svrInfo.serviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    svrInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t minRangeSize = 0;
    int32_t maxRangeSize = 100;
    int32_t dataSize = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, maxRangeSize);
    for (int32_t i = 0; i < dataSize; ++i) {
        std::string key = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        svrInfo.data[key] = value;
    }
    MessageParcel parcel;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmServiceProfileInfo(svrInfo, parcel);
}

void EncodeDmServiceProfileInfosFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t minRangeSize = 0;
    int32_t middleRangeSize = 50;
    int32_t maxRangeSize = 100;
    std::vector<DmServiceProfileInfo> svrInfos;
    int32_t listSize = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, maxRangeSize);
    for (int32_t i = 0; i < listSize; ++i) {
        DmServiceProfileInfo svrInfo;
        svrInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        svrInfo.serviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        svrInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

        int32_t dataSize = fdp.ConsumeIntegralInRange<int32_t>(minRangeSize, middleRangeSize);
        for (int32_t j = 0; j < dataSize; ++j) {
            std::string key = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            svrInfo.data[key] = value;
        }
        svrInfos.emplace_back(svrInfo);
    }
    MessageParcel parcel;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmServiceProfileInfos(svrInfos, parcel);
}

void EncodeDmDeviceProfileInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH * 22;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DmDeviceProfileInfo devInfo;
    devInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.deviceSn = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.mac = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.model = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.deviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.manufacturer = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.deviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.productName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.sdkVersion = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.bleMac = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.sleMac = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.firmwareVersion = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.hardwareVersion = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.softwareVersion = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.protocolType = fdp.ConsumeIntegral<int32_t>();
    devInfo.setupType = fdp.ConsumeIntegral<int32_t>();
    devInfo.wiseDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.wiseUserId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.registerTime = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.modifyTime = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.shareTime = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    devInfo.isLocalDevice = fdp.ConsumeBool();
    int32_t serviceCount = 10;
    for (int32_t i = 0; i < serviceCount; ++i) {
        DmServiceProfileInfo serviceInfo;
        serviceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.serviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        int32_t dataCount = 5;
        for (int32_t j = 0; j < dataCount; ++j) {
            std::string key = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
            serviceInfo.data[key] = value;
        }
        devInfo.services.emplace_back(serviceInfo);
    }
    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmDeviceProfileInfo(devInfo, parcel);
}

void GetDeviceIconInfoUniqueKeyFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmDeviceIconInfoFilterOptions iconFilter;
    iconFilter.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->GetDeviceIconInfoUniqueKey(iconFilter);
}

void GetDeviceIconInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmDeviceIconInfo iconInfo;
    iconInfo.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->GetDeviceIconInfoUniqueKey(iconInfo);
}

void DecodeDmDeviceIconInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + sizeof(uint8_t) + MAX_STRING_LENGTH * 7;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    int32_t minRangeLength = 0;
    int32_t maxRangeLength = 100;
    int32_t iconLength = fdp.ConsumeIntegralInRange<int32_t>(minRangeLength, maxRangeLength);
    parcel.WriteInt32(iconLength);
    if (iconLength > 0) {
        std::vector<uint8_t> iconData = fdp.ConsumeBytes<uint8_t>(iconLength);
        parcel.WriteRawData(iconData.data(), iconData.size());
    }
    DmDeviceIconInfo deviceIconInfo;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceIconInfo(parcel, deviceIconInfo);
}

void EncodeDmDeviceIconInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + sizeof(uint8_t) + MAX_STRING_LENGTH * 7;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmDeviceIconInfo deviceIconInfo;
    deviceIconInfo.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.version = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    deviceIconInfo.url = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    int32_t minRangeLength = 0;
    int32_t maxRangeLength = 100;
    int32_t iconLength = fdp.ConsumeIntegralInRange<int32_t>(minRangeLength, maxRangeLength);
    deviceIconInfo.icon = fdp.ConsumeBytes<uint8_t>(iconLength);

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmDeviceIconInfo(deviceIconInfo, parcel);
}

void DecodeDmDeviceIconInfoFilterOptionsFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    DmDeviceIconInfoFilterOptions filterOptions;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceIconInfoFilterOptions(parcel, filterOptions);
}

void EncodeDmDeviceIconInfoFilterOptionsFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = MAX_STRING_LENGTH * 5;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DmDeviceIconInfoFilterOptions filterOptions;
    filterOptions.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    filterOptions.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    filterOptions.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    filterOptions.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    filterOptions.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmDeviceIconInfoFilterOptions(filterOptions, parcel);
}

void DecodeDmDeviceInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 3 + sizeof(uint16_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint16(fdp.ConsumeIntegral<uint16_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    DmDeviceInfo devInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmDeviceInfo(parcel, devInfo);
}

void EncodeLocalServiceInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 3 + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    DMLocalServiceInfo serviceInfo;
    serviceInfo.bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.authBoxType = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.authType = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.pinExchangeType = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.pinCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.extraInfo = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    MessageParcel parcel;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeLocalServiceInfo(serviceInfo, parcel);
}

void EncodeLocalServiceInfosFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 6 + sizeof(uint32_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    uint32_t minRangeSize = 0;
    uint32_t maxRangeSize = 100;
    std::vector<DMLocalServiceInfo> serviceInfos;
    uint32_t listSize = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, maxRangeSize);
    for (uint32_t i = 0; i < listSize; ++i) {
        DMLocalServiceInfo serviceInfo;
        serviceInfo.bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.authBoxType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.authType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.pinExchangeType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.pinCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.extraInfo = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfos.emplace_back(serviceInfo);
    }
    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeLocalServiceInfos(serviceInfos, parcel);
    serviceInfos.clear();
    uint32_t outListSize = 1001;
    for (uint32_t i = 0; i < outListSize; ++i) {
        DMLocalServiceInfo serviceInfo;
        serviceInfo.bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.authBoxType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.authType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.pinExchangeType = fdp.ConsumeIntegral<int32_t>();
        serviceInfo.pinCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfo.extraInfo = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
        serviceInfos.emplace_back(serviceInfo);
    }
    ipcModelCodec->EncodeLocalServiceInfos(serviceInfos, parcel);
}

void DecodeLocalServiceInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 3 + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    DMLocalServiceInfo serviceInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeLocalServiceInfo(parcel, serviceInfo);
}

void DecodeLocalServiceInfosFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 3 + sizeof(uint32_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    uint32_t minRangeSize = 0;
    uint32_t maxRangeSize = 100;
    MessageParcel parcel;
    uint32_t num = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, maxRangeSize);
    parcel.WriteUint32(num);
    for (uint32_t i = 0; i < num; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
        parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>()); 
        parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }

    std::vector<DMLocalServiceInfo> serviceInfos;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeLocalServiceInfos(parcel, serviceInfos);
}

void EncodeNetworkIdQueryFilterFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    NetworkIdQueryFilter queryFilter;
    queryFilter.wiseDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    queryFilter.onlineStatus = fdp.ConsumeIntegral<int32_t>();
    queryFilter.deviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    queryFilter.deviceProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    queryFilter.deviceModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeNetworkIdQueryFilter(queryFilter, parcel);
}

void DecodeNetworkIdQueryFilterFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    NetworkIdQueryFilter queryFilter;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeNetworkIdQueryFilter(parcel, queryFilter);
}

void EncodeStringVectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    uint32_t minRangeSize = 0;
    uint32_t maxRangeSize = 100;

    std::vector<std::string> vec;
    uint32_t listSize = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, maxRangeSize);
    for (uint32_t i = 0; i < listSize; ++i) {
        vec.emplace_back(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }
    MessageParcel parcel;

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeStringVector(vec, parcel);
}

void DecodeStringVectorFuzzTest(const uint8_t* data, size_t size)
{
    size_t count = 2;
    if ((data == nullptr) || (size < sizeof(uint32_t) * count)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    uint32_t minRangeSize = 0;
    uint32_t maxRangeSize = 1000;
    MessageParcel parcel;
    uint32_t num = fdp.ConsumeIntegralInRange<uint32_t>(minRangeSize, maxRangeSize);
    parcel.WriteUint32(num);
    for (uint32_t i = 0; i < num; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }

    std::vector<std::string> vec;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeStringVector(parcel, vec);

    uint32_t outMinRangeSize = 1001;
    uint32_t outMaxRangeSize = 2000;
    uint32_t invalidNum = fdp.ConsumeIntegralInRange<uint32_t>(outMinRangeSize, outMaxRangeSize);
    MessageParcel invalidParcel;
    parcel.WriteUint32(invalidNum);
    ipcModelCodec->DecodeStringVector(invalidParcel, vec);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DecodeDmDeviceBasicInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodePeerTargetIdFuzzTest(data, size);
    OHOS::DistributedHardware::DecodePeerTargetIdFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmAccessCallerFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmAccessCalleeFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmDeviceProfileInfoFilterOptionsFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmDeviceProfileInfoFilterOptionsFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmServiceProfileInfoFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmServiceProfileInfosFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmDeviceProfileInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmServiceProfileInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmServiceProfileInfosFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmDeviceProfileInfoFuzzTest(data, size);
    OHOS::DistributedHardware::GetDeviceIconInfoUniqueKeyFuzzTest(data, size);
    OHOS::DistributedHardware::GetDeviceIconInfoFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmDeviceIconInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmDeviceIconInfoFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmDeviceIconInfoFilterOptionsFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeDmDeviceIconInfoFilterOptionsFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeDmDeviceInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeLocalServiceInfoFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeLocalServiceInfosFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeLocalServiceInfoFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeLocalServiceInfosFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeNetworkIdQueryFilterFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeNetworkIdQueryFilterFuzzTest(data, size);
    OHOS::DistributedHardware::EncodeStringVectorFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeStringVectorFuzzTest(data, size);
    return 0;
}
