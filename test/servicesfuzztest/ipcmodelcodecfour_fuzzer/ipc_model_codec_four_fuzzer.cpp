/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "ipc_model_codec_four_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "ipc_model_codec.h"


namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 256;

void EncodeDmRegServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    regServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    regServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmRegServiceInfo(regServiceInfo, parcel);
}

void DecodeDmRegServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));

    DmRegisterServiceInfo regServiceInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmRegServiceInfo(parcel, regServiceInfo);
}

void EncodeDmSrvDiscParamFuzzTest(FuzzedDataProvider &fdp)
{
    DmDiscoveryServiceParam param;
    param.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    param.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    param.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    param.freq = static_cast<DmExchangeFreq>(fdp.ConsumeIntegral<int32_t>());
    param.medium = static_cast<DMSrvMediumType>(fdp.ConsumeIntegral<int32_t>());
    param.mode = static_cast<DMSrvDiscoveryMode>(fdp.ConsumeIntegral<int32_t>());

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmSrvDiscParam(param, parcel);
}

void DecodeDmSrvDiscParamFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());

    DmDiscoveryServiceParam param;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmSrvDiscParam(parcel, param);
}

void EncodeDmPublishServiceParamFuzzTest(FuzzedDataProvider &fdp)
{
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = static_cast<DMSrvDiscoveryMode>(fdp.ConsumeIntegral<int32_t>());
    publishServiceParam.media = static_cast<DMSrvMediumType>(fdp.ConsumeIntegral<int32_t>());
    publishServiceParam.freq = static_cast<DmExchangeFreq>(fdp.ConsumeIntegral<int32_t>());

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmPublishServiceParam(publishServiceParam, parcel);
}

void DecodeDmPublishServiceParamFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());

    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmPublishServiceParam(parcel, publishServiceParam);
}

void EncodeDmServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DmServiceInfo serviceInfo;
    serviceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    serviceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    serviceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmServiceInfo(serviceInfo, parcel);
}

void DecodeDmServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());

    DmServiceInfo serviceInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmServiceInfo(parcel, serviceInfo);
}

void DecodeDmRegisterServiceStateFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());

    DmRegisterServiceState dmRegisterServiceState;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeDmRegisterServiceState(parcel, dmRegisterServiceState);
}

void EncodeDmRegisterServiceStateFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceState serviceInfo;
    serviceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.tokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeDmRegisterServiceState(serviceInfo, parcel);
}

void EncodeServiceSyncInfoFuzzTest(FuzzedDataProvider &fdp)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceSyncInfo.localUserId = fdp.ConsumeIntegral<int32_t>();
    serviceSyncInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceSyncInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceSyncInfo.callerUserId = fdp.ConsumeIntegral<int32_t>();
    serviceSyncInfo.callerTokenId = fdp.ConsumeIntegral<uint32_t>();

    MessageParcel parcel;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->EncodeServiceSyncInfo(serviceSyncInfo, parcel);
}

void DecodeServiceSyncInfoFuzzTest(FuzzedDataProvider &fdp)
{
    MessageParcel parcel;
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteString(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    parcel.WriteInt64(fdp.ConsumeIntegral<int64_t>());
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());

    ServiceSyncInfo serviceSyncInfo;
    std::shared_ptr<IpcModelCodec> ipcModelCodec = std::make_shared<IpcModelCodec>();
    ipcModelCodec->DecodeServiceSyncInfo(parcel, serviceSyncInfo);
}

void IpcModelCodecFourFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    EncodeDmRegServiceInfoFuzzTest(fdp);
    DecodeDmRegServiceInfoFuzzTest(fdp);
    EncodeDmSrvDiscParamFuzzTest(fdp);
    DecodeDmSrvDiscParamFuzzTest(fdp);
    EncodeDmPublishServiceParamFuzzTest(fdp);
    DecodeDmPublishServiceParamFuzzTest(fdp);
    EncodeDmServiceInfoFuzzTest(fdp);
    DecodeDmServiceInfoFuzzTest(fdp);
    DecodeDmRegisterServiceStateFuzzTest(fdp);
    EncodeDmRegisterServiceStateFuzzTest(fdp);
    EncodeServiceSyncInfoFuzzTest(fdp);
    DecodeServiceSyncInfoFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::IpcModelCodecFourFuzzTest(data, size);
    return 0;
}
