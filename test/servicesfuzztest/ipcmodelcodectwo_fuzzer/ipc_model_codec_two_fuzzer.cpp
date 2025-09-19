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
#include "ipc_model_codec_two_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "ipc_model_codec.h"


namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 64;
void DecodeDmDeviceProfileInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmServiceProfileInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmServiceProfileInfosFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmDeviceProfileInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void GetDeviceIconInfoUniqueKeyFuzzTest(FuzzedDataProvider &fdp)
{
    DmDeviceIconInfoFilterOptions iconFilter;
    iconFilter.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconFilter.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->GetDeviceIconInfoUniqueKey(iconFilter);
}

void GetDeviceIconInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DmDeviceIconInfo iconInfo;
    iconInfo.productId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.subProductId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.internalModel = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.imageType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    iconInfo.specName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->GetDeviceIconInfoUniqueKey(iconInfo);
}

void DecodeDmDeviceIconInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmDeviceIconInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeDmDeviceIconInfoFilterOptionsFuzzTest(FuzzedDataProvider &fdp)
{
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

void IpcModelCodecTwoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DecodeDmDeviceProfileInfoFuzzTest(fdp);
    EncodeDmServiceProfileInfoFuzzTest(fdp);
    EncodeDmServiceProfileInfosFuzzTest(fdp);
    EncodeDmDeviceProfileInfoFuzzTest(fdp);
    GetDeviceIconInfoUniqueKeyFuzzTest(fdp);
    GetDeviceIconInfoFuzzTest(fdp);
    DecodeDmDeviceIconInfoFuzzTest(fdp);
    EncodeDmDeviceIconInfoFuzzTest(fdp);
    DecodeDmDeviceIconInfoFilterOptionsFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcModelCodecTwoFuzzTest(data, size);
    return 0;
}
