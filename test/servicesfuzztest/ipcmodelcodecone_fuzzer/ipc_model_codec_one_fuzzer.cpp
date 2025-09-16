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
#include "ipc_model_codec_one_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "ipc_model_codec.h"


namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 64;
void DecodeDmDeviceBasicInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodePeerTargetIdFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodePeerTargetIdFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmAccessCallerFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmAccessCalleeFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeDmDeviceProfileInfoFilterOptionsFuzzTest(FuzzedDataProvider &fdp)
{
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

void EncodeDmDeviceProfileInfoFilterOptionsFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeDmServiceProfileInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void DecodeDmServiceProfileInfosFuzzTest(FuzzedDataProvider &fdp)
{
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

void IpcModelCodecOneFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DecodeDmDeviceBasicInfoFuzzTest(fdp);
    EncodePeerTargetIdFuzzTest(fdp);
    DecodePeerTargetIdFuzzTest(fdp);
    EncodeDmAccessCallerFuzzTest(fdp);
    EncodeDmAccessCalleeFuzzTest(fdp);
    DecodeDmDeviceProfileInfoFilterOptionsFuzzTest(fdp);
    EncodeDmDeviceProfileInfoFilterOptionsFuzzTest(fdp);
    DecodeDmServiceProfileInfoFuzzTest(fdp);
    DecodeDmServiceProfileInfosFuzzTest(fdp);
    DecodeDmDeviceProfileInfoFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcModelCodecOneFuzzTest(data, size);
    return 0;
}
