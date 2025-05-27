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

#include <cstddef>
#include <cstdint>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>

#include "dm_constants.h"
#include "dm_device_info.h"
#include "device_manager_service.h"
#include "device_manager_impl.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "dm_softbus_listener_fuzzer.h"
#include "softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmSoftbusListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <  (sizeof(uint16_t) + (sizeof(int32_t) + sizeof(int32_t))) ||
        (size > DM_MAX_DEVICE_ID_LEN))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceTypeId = fdp.ConsumeIntegral<std::int16_t>();
    deviceInfo.range = fdp.ConsumeIntegral<std::int32_t>();
    deviceInfo.networkType = fdp.ConsumeIntegral<std::int32_t>();
    deviceInfo.extraData = fdp.ConsumeRandomLengthString();
    SoftbusListener::DeviceNameChange(deviceInfo);
    SoftbusListener::DeviceScreenStatusChange(deviceInfo);
    int32_t errcode = fdp.ConsumeIntegral<std::int32_t>();
    uint16_t deviceTypeId = fdp.ConsumeIntegral<std::uint16_t>();
    string proofInfoStr = fdp.ConsumeRandomLengthString();
    SoftbusListener::CredentialAuthStatusProcess(proofInfoStr, deviceTypeId, errcode);
    const char* proofInfo = proofInfoStr.c_str();
    DevUserInfo localDevUserInfo;
    localDevUserInfo.deviceId = fdp.ConsumeRandomLengthString();
    localDevUserInfo.userId = fdp.ConsumeIntegral<std::int32_t>();
    DevUserInfo remoteDevUserInfo;
    remoteDevUserInfo.deviceId = fdp.ConsumeRandomLengthString();
    remoteDevUserInfo.userId = fdp.ConsumeIntegral<std::int32_t>();
    string remoteAclList = fdp.ConsumeRandomLengthString();
    SoftbusListener::OnSyncLocalAclList(localDevUserInfo, remoteDevUserInfo, remoteAclList);
    string aclList = fdp.ConsumeRandomLengthString();
    SoftbusListener::OnGetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
    uint32_t proofLen = fdp.ConsumeIntegral<std::uint32_t>();
    SoftbusListener::OnCredentialAuthStatus(proofInfo, proofLen, deviceTypeId, errcode);
    uint16_t typeValue = fdp.ConsumeIntegral<std::uint16_t>();
    NodeStatusType type = static_cast<NodeStatusType>(typeValue);
    NodeStatus status;
    status.basicInfo.deviceTypeId = fdp.ConsumeIntegral<std::uint16_t>();
    status.basicInfo.osType = fdp.ConsumeIntegral<std::uint32_t>();
    status.reserved[0] = fdp.ConsumeIntegral<std::uint16_t>();
    SoftbusListener::OnDeviceScreenStatusChanged(type, &status);
    SoftbusListener listener;
    std::string networkIdStr = fdp.ConsumeRandomLengthString();
    int32_t networkType = fdp.ConsumeIntegral<std::int32_t>();
    listener.GetNetworkTypeByNetworkId(networkIdStr.c_str(), networkType);
    std::string name = fdp.ConsumeRandomLengthString();
    SoftbusListener::CloseDmRadarHelperObj(name);
    std::string msg = fdp.ConsumeRandomLengthString();
    listener.SendAclChangedBroadcast(msg);
    int32_t screenStatus = fdp.ConsumeIntegral<std::int32_t>();
    listener.GetDeviceScreenStatus(networkId, screenStatus);
    listener.DeleteCacheDeviceInfo();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmSoftbusListenerFuzzTest(data, size);
    return 0;
}
