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
#include "ipc_model_codec_three_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "ipc_model_codec.h"


namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 64;
void EncodeDmDeviceIconInfoFilterOptionsFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeDmDeviceInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeLocalServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeLocalServiceInfosFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeLocalServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeLocalServiceInfosFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeNetworkIdQueryFilterFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeNetworkIdQueryFilterFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeStringVectorFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeStringVectorFuzzTest(FuzzedDataProvider &fdp)
{
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

void IpcModelCodecThreeFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    EncodeDmDeviceIconInfoFilterOptionsFuzzTest(fdp);
    DecodeDmDeviceInfoFuzzTest(fdp);
    EncodeLocalServiceInfoFuzzTest(fdp);
    EncodeLocalServiceInfosFuzzTest(fdp);
    DecodeLocalServiceInfoFuzzTest(fdp);
    DecodeLocalServiceInfosFuzzTest(fdp);
    EncodeNetworkIdQueryFilterFuzzTest(fdp);
    DecodeNetworkIdQueryFilterFuzzTest(fdp);
    EncodeStringVectorFuzzTest(fdp);
    DecodeStringVectorFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcModelCodecThreeFuzzTest(data, size);
    return 0;
}
