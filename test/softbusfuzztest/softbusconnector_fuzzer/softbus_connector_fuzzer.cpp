/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "softbus_connector_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_bus_center.h"
#include "softbus_connector.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {

void SoftBusConnectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int) + sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    ConnectionAddr *addr = nullptr;
    const char *networkId = reinterpret_cast<const char*>(data);
    int32_t result = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusJoinLNNResult(addr, networkId, result);
}

void SoftBusConnectorFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string credId = fdp.ConsumeRandomLengthString();
    int32_t sessionKeyId = fdp.ConsumeIntegral<int32_t>();
    int32_t aclId = fdp.ConsumeIntegral<int32_t>();
    std::string localUdid = "localDeviceId";
    std::string remoteUdid = "remoteDeviceId";
    DistributedDeviceProfile::AccessControlProfile localAcl;
    std::vector<std::string> acLStrList;
    std::string jsonString = fdp.ConsumeRandomLengthString();
    std::vector<AclHashItem> remoteAllAclList;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->SyncAclList(userId, credId, sessionKeyId, aclId);
    DistributedDeviceProfile::Accesser acer;
    DistributedDeviceProfile::Accessee acee;
    acee.SetAccesseeDeviceId("localDeviceId");
    acer.SetAccesserDeviceId("remoteDeviceId");
    localAcl.SetTrustDeviceId("remoteDeviceId");
    localAcl.SetAccesser(acer);
    localAcl.SetAccessee(acee);
    softbusConnector->SyncLocalAclList5_1_0(localUdid, remoteUdid, localAcl, acLStrList);
    acer.SetAccesserDeviceId("localDeviceId");
    acee.SetAccesseeDeviceId("remoteDeviceId");
    localAcl.SetTrustDeviceId("remoteDeviceId");
    localAcl.SetAccesser(acer);
    localAcl.SetAccessee(acee);
    softbusConnector->SyncLocalAclList5_1_0(localUdid, remoteUdid, localAcl, acLStrList);
    softbusConnector->ParaseAclChecksumList(jsonString, remoteAllAclList);
    std::string localVersion = fdp.ConsumeRandomLengthString();
    softbusConnector->GetLocalVersion(localUdid, remoteUdid, localVersion, localAcl);
    std::vector<std::string> aclVerDesc;
    std::map<std::string, AclHashItem> remoteAllAclMap;
    softbusConnector->SortAclListDesc(remoteAllAclList, aclVerDesc, remoteAllAclMap);
    std::vector<std::string> remoteVerDesc;
    softbusConnector->MatchTargetVersion(localVersion, remoteVerDesc);
    remoteVerDesc.push_back(fdp.ConsumeRandomLengthString());
    softbusConnector->MatchTargetVersion(localVersion, remoteVerDesc);
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string remoteUdidHash = fdp.ConsumeRandomLengthString();
    softbusConnector->JoinLnn(deviceId, remoteUdidHash);
}

void SoftBusConnectorSecondFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t sessionKeyId = fdp.ConsumeIntegral<int32_t>();
    std::string udid = fdp.ConsumeRandomLengthString();
    std::string udidHash = fdp.ConsumeRandomLengthString();
    int32_t remoteSessionKeyId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    softbusConnector->JoinLNNBySkId(sessionId, sessionKeyId, remoteSessionKeyId, udid, udidHash);
    softbusConnector->JoinLnnByHml(sessionId, sessionKeyId, remoteSessionKeyId);
    int32_t tempSessionKeyId = 1;
    int32_t tempRemoteSessionKeyId = 2;
    sessionKeyId = tempSessionKeyId;
    remoteSessionKeyId = tempRemoteSessionKeyId;
    softbusConnector->JoinLnnByHml(sessionId, sessionKeyId, remoteSessionKeyId);
    DeviceInfo deviceInfo;
    DmDeviceInfo dmDeviceInfo;
    memcpy_s(deviceInfo.devId, sizeof(deviceInfo.devId), "deviceId", sizeof("deviceId"));
    memcpy_s(deviceInfo.devName, sizeof(deviceInfo.devName), "deviceName", sizeof("deviceName"));
    softbusConnector->ConvertDeviceInfoToDmDevice(deviceInfo, dmDeviceInfo);
    DmDeviceBasicInfo dmDeviceBasicInfo;
    softbusConnector->ConvertDeviceInfoToDmDevice(deviceInfo, dmDeviceBasicInfo);
    softbusConnector->GetLocalDeviceNetworkId();
    ProcessInfo processInfo;
    softbusConnector->SetProcessInfo(processInfo);
    std::vector<ProcessInfo> processInfoVec;
    softbusConnector->SetProcessInfoVec(processInfoVec);
    softbusConnector->GetProcessInfo();
    softbusConnector->ClearProcessInfo();
    softbusConnector->SetChangeProcessInfo(processInfo);
    softbusConnector->GetChangeProcessInfo();
    softbusConnector->ClearChangeProcessInfo();
    softbusConnector->DeleteOffLineTimer(udidHash);
    softbusConnector->CheckIsNeedJoinLnn(udid, deviceId);
    softbusConnector->CheckIsNeedJoinLnn("udid", "deviceId");
    NodeBasicInfo nodeBasicInfo;
    memcpy_s(nodeBasicInfo.networkId, sizeof(nodeBasicInfo.networkId), "networkId", sizeof("networkId"));
    memcpy_s(nodeBasicInfo.deviceName, sizeof(nodeBasicInfo.deviceName), "deviceName", sizeof("deviceName"));
    softbusConnector->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    DevUserInfo localUserInfo;
    DevUserInfo remoteUserInfo;
    std::string remoteAclList = fdp.ConsumeRandomLengthString();
    softbusConnector->SyncLocalAclListProcess(localUserInfo, remoteUserInfo, remoteAclList);
    softbusConnector->GetAclListHash(localUserInfo, remoteUserInfo, remoteAclList);
}

void AddMemberToDiscoverMapFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t maxStringLength = 64;
    std::string deviceId = fdp.ConsumeRandomLengthString(maxStringLength);

    auto deviceInfo = std::make_shared<DeviceInfo>();
    deviceInfo->addrNum = fdp.ConsumeIntegral<int32_t>();
    deviceInfo->devType = static_cast<DeviceType>(fdp.ConsumeIntegral<int32_t>());
    fdp.ConsumeData(deviceInfo->devId, sizeof(deviceInfo->devId));
    fdp.ConsumeData(deviceInfo->devName, sizeof(deviceInfo->devName));

    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->AddMemberToDiscoverMap(deviceId, deviceInfo);
    softbusConnector->AddMemberToDiscoverMap("", deviceInfo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusConnectorFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusConnectorFirstFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusConnectorSecondFuzzTest(data, size);
    OHOS::DistributedHardware::AddMemberToDiscoverMapFuzzTest(data, size);
    return 0;
}
