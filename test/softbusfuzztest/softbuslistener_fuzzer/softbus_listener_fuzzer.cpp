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
#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_impl.h"
#include "dm_constants.h"
#include "softbus_listener.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"
#include "softbus_session.h"
#include <memory>
#include "softbus_listener_fuzzer.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr int32_t INT32NUM = 3;
    constexpr int32_t DATA_LEN = 20;
    constexpr int32_t CONNECTION_ADDR_USB_VALUE = 5;
    constexpr uint32_t MAX_DEVICE_LIST_LENGTH = 2001;
}

class ISoftbusDiscoveringCallbackTest : public ISoftbusDiscoveringCallback {
public:
    virtual ~ISoftbusDiscoveringCallbackTest()
    {
    }
    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info, bool isOnline) override
    {
        (void)pkgName;
        (void)info;
        (void)isOnline;
    }
    void OnDiscoveringResult(const std::string &pkgName, int32_t subscribeId, int32_t result) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)result;
    }
};

std::shared_ptr<SoftbusListener> softbusListener_ =  std::make_shared<SoftbusListener>();

void SoftBusListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(uint16_t))) {
        return;
    }
    std::string displayName(reinterpret_cast<const char*>(data), size);
    softbusListener_->SetLocalDisplayName(displayName);
    softbusListener_->DeleteCacheDeviceInfo();
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string ip(reinterpret_cast<const char*>(data), size);
    ConnectionAddrType addrType = ConnectionAddrType::CONNECTION_ADDR_ETH;
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    softbusListener_->GetIPAddrTypeFromCache(deviceId, ip, addrType);
    DmDeviceInfo deviceInfo;
    softbusListener_->DeviceOnLine(deviceInfo);
    softbusListener_->DeviceOffLine(deviceInfo);
    softbusListener_->DeviceNameChange(deviceInfo);
    std::string strMsg(reinterpret_cast<const char*>(data), size);
    softbusListener_->DeviceNotTrust(strMsg);
    softbusListener_->DeviceTrustedChange(strMsg);
    softbusListener_->DeviceUserIdCheckSumChange(strMsg);
    softbusListener_->DeviceScreenStatusChange(deviceInfo);
    std::string deviceList(reinterpret_cast<const char*>(data), size);
    FuzzedDataProvider fdp(data, size);
    int32_t errcode = fdp.ConsumeIntegral<int32_t>();
    uint16_t deviceTypeId = fdp.ConsumeIntegral<uint16_t>();
    softbusListener_->CredentialAuthStatusProcess(deviceList, deviceTypeId, errcode);
    uint32_t deviceListLen = static_cast<uint32_t>(deviceList.length());
    softbusListener_->OnCredentialAuthStatus(deviceList.data(), deviceListLen, deviceTypeId, errcode);
    NodeStatusType type = NodeStatusType::TYPE_SCREEN_STATUS;
    NodeStatus *status = nullptr;
    softbusListener_->OnDeviceScreenStatusChanged(type, status);
    NodeBasicInfo nodeBasicInfo = {
        .networkId = {"networkId"},
        .deviceName = {"deviceNameInfo"},
        .deviceTypeId = 1,
        .osType = 1,
        .osVersion = {1}
    };
    NodeStatus nodeStatus = {
        .basicInfo = nodeBasicInfo,
        .authStatus = 1,
        .dataBaseStatus = 1,
        .meshType = 1,
        .reserved = {1}
    };
    softbusListener_->OnDeviceScreenStatusChanged(type, &nodeStatus);
    type = NodeStatusType::TYPE_AUTH_STATUS;
    softbusListener_->OnDeviceScreenStatusChanged(type, &nodeStatus);
    softbusListener_->OnSoftbusDeviceOnline(&nodeBasicInfo);
}

void SoftBusListenerFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    NodeBasicInfo nodeBasicInfo = {
        .networkId = {"networkId"},
        .deviceName = {"deviceNameInfo"},
        .deviceTypeId = 1,
        .osType = 1,
        .osVersion = {1}
    };
    NodeBasicInfoType typeInfo = NodeBasicInfoType::TYPE_DEVICE_NAME;
    softbusListener_->OnSoftbusDeviceOffline(&nodeBasicInfo);
    softbusListener_->OnSoftbusDeviceInfoChanged(typeInfo, &nodeBasicInfo);
    typeInfo = NodeBasicInfoType::TYPE_NETWORK_INFO;
    softbusListener_->OnSoftbusDeviceOffline(&nodeBasicInfo);
    softbusListener_->OnSoftbusDeviceInfoChanged(typeInfo, &nodeBasicInfo);
    softbusListener_->OnLocalDevInfoChange();
    TrustChangeType changeType = TrustChangeType::DEVICE_NOT_TRUSTED;
    std::string strMsg(reinterpret_cast<const char*>(data), size);
    uint32_t msgLen = static_cast<uint32_t>(strMsg.length());
    softbusListener_->OnDeviceTrustedChange(changeType, strMsg.data(), msgLen);
    changeType = TrustChangeType::DEVICE_TRUST_RELATIONSHIP_CHANGE;
    softbusListener_->OnDeviceTrustedChange(changeType, strMsg.data(), msgLen);
    changeType = TrustChangeType::DEVICE_FOREGROUND_USERID_CHANGE;
    softbusListener_->OnDeviceTrustedChange(changeType, strMsg.data(), msgLen);
    DeviceInfo *device = nullptr;
    softbusListener_->OnSoftbusDeviceFound(device);
    DeviceInfo deviceInfo;
    softbusListener_->OnSoftbusDeviceFound(&deviceInfo);
    FuzzedDataProvider fdp(data, size);
    int32_t publishId = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->StopPublishSoftbusLNN(publishId);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<ISoftbusDiscoveringCallback> callback = nullptr;
    softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName, callback);
    callback = std::make_shared<ISoftbusDiscoveringCallbackTest>();
    softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName, callback);
    softbusListener_->UnRegisterSoftbusLnnOpsCbk(pkgName);
    std::string networkId(reinterpret_cast<const char*>(data), size);
    DmDeviceInfo dmInfo;
    softbusListener_->GetDeviceInfo(networkId, dmInfo);
    softbusListener_->GetLocalDeviceInfo(dmInfo);
    std::string udid(reinterpret_cast<const char*>(data), size);
    softbusListener_->GetNetworkIdByUdid(udid, networkId);
    bool isWakeUp = false;
    std::string callerId(reinterpret_cast<const char*>(data), size);
    softbusListener_->ShiftLNNGear(isWakeUp, callerId);
    softbusListener_->ShiftLNNGear(isWakeUp, "");
}

void SoftBusListenerSecondFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) * INT32NUM)) {
        return;
    }
    NodeBasicInfo nodeBasicInfo = {
        .networkId = {"networkId"},
        .deviceName = {"deviceNameInfo"},
        .deviceTypeId = 1,
        .osType = 1,
        .osVersion = {1}
    };
    FuzzedDataProvider fdp(data, size);
    int32_t devScreenStatus = fdp.ConsumeIntegral<int32_t>();
    DmDeviceInfo devInfo;
    softbusListener_->ConvertScreenStatusToDmDevice(nodeBasicInfo, devScreenStatus, devInfo);
    softbusListener_->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, devInfo);
    DmDeviceBasicInfo dmdevInfo;
    softbusListener_->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmdevInfo);
    uint8_t arr[DATA_LEN] = {1};
    size_t len = static_cast<size_t>(DATA_LEN);
    softbusListener_->ConvertBytesToUpperCaseHexString(arr, len);
    DeviceInfo deviceInfo = {
        .devId = "deviceId",
        .devType = (DeviceType)1,
        .devName = "11111",
        .addrNum = 1,
        .addr[0] = {
            .type = ConnectionAddrType::CONNECTION_ADDR_ETH,
            .info {
                .ip {
                    .ip = "172.0.0.1",
                    .port = 0,
                }
            }
        }
    };
    softbusListener_->OnSoftbusDeviceFound(&deviceInfo);
    softbusListener_->ConvertDeviceInfoToDmDevice(deviceInfo, devInfo);
    int32_t networkType = fdp.ConsumeIntegral<int32_t>();
    std::string networkId(reinterpret_cast<const char*>(data), size);
    softbusListener_->GetNetworkTypeByNetworkId(networkId.data(), networkType);
    int32_t securityLevel = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->GetDeviceSecurityLevel(networkId.data(), securityLevel);
    softbusListener_->CacheDiscoveredDevice(&deviceInfo);
    softbusListener_->GetDeviceScreenStatus(networkId.data(), securityLevel);
}

void SoftBusListenerThirdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    ConnectionAddr addrInfo = {
        .type = ConnectionAddrType::CONNECTION_ADDR_ETH,
        .info {
            .ip {
                .ip = "172.0.0.2",
                .port = 1,
                .udidHash = {1}
            }
        }
    };

    ConnectionAddr addrInfo1 = {
        .type = ConnectionAddrType::CONNECTION_ADDR_BR,
        .info {
            .br {
                .brMac = "1.0.0.1"
            }
        }
    };

    ConnectionAddr addrInfo2 = {
        .type = ConnectionAddrType::CONNECTION_ADDR_BLE,
        .info {
            .ble {
                .protocol = BleProtocolType::BLE_GATT,
                .bleMac = "4.0.0.2",
                .udidHash = {1},
                .psm = 1
            }
        }
    };
    JsonObject jsonObj;
    softbusListener_->ParseConnAddrInfo(&addrInfo, jsonObj);
    addrInfo.type = ConnectionAddrType::CONNECTION_ADDR_WLAN;
    softbusListener_->ParseConnAddrInfo(&addrInfo, jsonObj);
    addrInfo.type = ConnectionAddrType::CONNECTION_ADDR_NCM;
    softbusListener_->ParseConnAddrInfo(&addrInfo, jsonObj);
    addrInfo.type = ConnectionAddrType::CONNECTION_ADDR_SESSION;
    softbusListener_->ParseConnAddrInfo(&addrInfo, jsonObj);
    softbusListener_->ParseConnAddrInfo(&addrInfo1, jsonObj);
    softbusListener_->ParseConnAddrInfo(&addrInfo2, jsonObj);
    addrInfo.type = static_cast<ConnectionAddrType>(CONNECTION_ADDR_USB_VALUE);
    softbusListener_->ParseConnAddrInfo(&addrInfo, jsonObj);
    std::string remoteUdid(reinterpret_cast<const char*>(data), size);
    std::vector<uint32_t> userIds;
    softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUdid, userIds);
}

void SoftBusListenerForthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    NodeBasicInfo nodeBasicInfo = {
        .networkId = {"networkId"},
        .deviceName = {"deviceNameInfo"},
        .deviceTypeId = 1,
        .osType = 1,
        .osVersion = {1}
    };
    softbusListener_->OnSoftbusDeviceOffline(&nodeBasicInfo);

    DistributedDeviceProfile::AccessControlProfile profile;
    DmDeviceInfo deviceInfo;
    softbusListener_->ConvertAclToDeviceInfo(profile, deviceInfo);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);
    std::vector<DmDeviceInfo> deviceList;
    softbusListener_->GetAllTrustedDeviceList(pkgName, extra, deviceList);
    TrustChangeType changeType = TrustChangeType::DEVICE_NOT_TRUSTED;
    uint32_t msgLen = 0;
    char* msg = nullptr;
    softbusListener_->OnDeviceTrustedChange(changeType, msg, msgLen);
}

void SoftBusListenerFifthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DmDeviceInfo deviceInfo;
    softbusListener_->InitSoftbusListener();
    softbusListener_->DeviceOnLine(deviceInfo);
    softbusListener_->DeviceOffLine(deviceInfo);
    softbusListener_->DeviceNameChange(deviceInfo);
    std::string msg = fdp.ConsumeRandomLengthString();
    softbusListener_->DeviceNotTrust(msg);
    softbusListener_->DeviceScreenStatusChange(deviceInfo);
    std::string deviceList = fdp.ConsumeRandomLengthString();
    int32_t errcode = fdp.ConsumeIntegral<int32_t>();
    uint16_t deviceTypeId = fdp.ConsumeIntegral<uint16_t>();
    softbusListener_->CredentialAuthStatusProcess(deviceList, deviceTypeId, errcode);
    DevUserInfo localDevUserInfo;
    DevUserInfo remoteDevUserInfo;
    std::string remoteAclList = fdp.ConsumeRandomLengthString();
    softbusListener_->OnSyncLocalAclList(localDevUserInfo, remoteDevUserInfo, remoteAclList);
    std::string aclList = fdp.ConsumeRandomLengthString();
    softbusListener_->OnGetAclListHash(localDevUserInfo, remoteDevUserInfo, aclList);
    uint32_t deviceListLen = static_cast<uint32_t>(deviceList.length());
    softbusListener_->OnCredentialAuthStatus(deviceList.data(), deviceListLen, deviceTypeId, errcode);

    deviceListLen = MAX_DEVICE_LIST_LENGTH;
    softbusListener_->OnCredentialAuthStatus(deviceList.data(), deviceListLen, deviceTypeId, errcode);
    std::shared_ptr<NodeBasicInfo> info = nullptr;
    softbusListener_->UpdateDeviceName(info.get());
    info = std::make_shared<NodeBasicInfo>();
    softbusListener_->UpdateDeviceName(info.get());
    int32_t publishId = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->StopPublishSoftbusLNN(publishId);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<ISoftbusDiscoveringCallback> callback;
    softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName, callback);
    callback = std::make_shared<ISoftbusDiscoveringCallbackTest>();
    softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName, callback);
    softbusListener_->UnRegisterSoftbusLnnOpsCbk(pkgName);
    std::string networkId = fdp.ConsumeRandomLengthString();
    softbusListener_->GetDeviceInfo(networkId, deviceInfo);
    std::string udid = fdp.ConsumeRandomLengthString();
    softbusListener_->GetNetworkIdByUdid(udid, networkId);
    std::string deviceName = fdp.ConsumeRandomLengthString();
    softbusListener_->GetDeviceNameByUdid(udid, deviceName);
    bool isWakeUp = fdp.ConsumeBool();
    std::string callerId = fdp.ConsumeRandomLengthString();
    softbusListener_->ShiftLNNGear(isWakeUp, callerId);
}

void SoftBusListenerSixthFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DmDeviceInfo deviceInfo;
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string msg = fdp.ConsumeRandomLengthString();
    uint8_t arr[DATA_LEN] = { 1 };
    size_t len = static_cast<size_t>(DATA_LEN);
    softbusListener_->ConvertBytesToUpperCaseHexString(arr, len);
    int32_t networkType = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->GetNetworkTypeByNetworkId(networkId.data(), networkType);
    int32_t securityLevel = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->GetDeviceSecurityLevel(networkId.data(), securityLevel);
    std::string name = fdp.ConsumeRandomLengthString();
    softbusListener_->CloseDmRadarHelperObj(name);
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string ip = fdp.ConsumeRandomLengthString();
    ConnectionAddrType addrType = ConnectionAddrType::CONNECTION_ADDR_ETH;
    softbusListener_->GetIPAddrTypeFromCache(deviceId, ip, addrType);
    softbusListener_->SendAclChangedBroadcast(msg);
    softbusListener_->GetSoftbusRefreshCb();
    int32_t screenStatus = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->GetDeviceScreenStatus(networkId.data(), screenStatus);
    std::string remoteUserId = fdp.ConsumeRandomLengthString();
    std::vector<uint32_t> userIds;
    softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUserId, userIds);
    softbusListener_->DeleteCacheDeviceInfo();
    std::string displayName = fdp.ConsumeRandomLengthString();
    softbusListener_->SetLocalDisplayName(displayName);
    DistributedDeviceProfile::AccessControlProfile profile;
    softbusListener_->ConvertAclToDeviceInfo(profile, deviceInfo);
    std::string extra = fdp.ConsumeRandomLengthString();
    std::vector<DmDeviceInfo> devList;
    softbusListener_->GetAllTrustedDeviceList(pkgName, extra, devList);
    std::shared_ptr<cJSON> customDataJson = std::make_shared<cJSON>();
    int32_t actionId = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->GetAttrFromCustomData(customDataJson.get(), deviceInfo, actionId);
}

void SoftBusListenerSeventhFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    NodeBasicInfo nodeBasicInfo = { .networkId = { "networkId" },
        .deviceName = { "deviceNameInfo" },
        .deviceTypeId = 1,
        .osType = 1,
        .osVersion = "onVersion" };
    int32_t devScreenStatus = fdp.ConsumeIntegral<int32_t>();
    DmDeviceInfo devInfo;
    softbusListener_->ConvertScreenStatusToDmDevice(nodeBasicInfo, devScreenStatus, devInfo);
    softbusListener_->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, devInfo);
    DmDeviceBasicInfo dmdevInfo;
    softbusListener_->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmdevInfo);
}

void PutOstypeDataFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if ((data == nullptr) || (size < sizeof(int32_t) + maxStringLength)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string peerUdid = fdp.ConsumeRandomLengthString(maxStringLength);
    int32_t osType = fdp.ConsumeIntegral<int32_t>();
    softbusListener_->PutOstypeData(peerUdid, osType);
}

void CacheDeviceInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    std::string deviceId = "deviceId";
    auto deviceInfo = std::make_shared<DeviceInfo>();
    deviceInfo->addrNum = 1;
    softbusListener_->CacheDeviceInfo(deviceId, deviceInfo);

    deviceInfo->addr[0].type = static_cast<ConnectionAddrType>(fdp.ConsumeIntegral<int32_t>());
    fdp.ConsumeData(deviceInfo->addr[0].info.ip.ip, sizeof(deviceInfo->addr[0].info.ip.ip));
    deviceInfo->addr[0].info.ip.port = fdp.ConsumeIntegral<uint16_t>();
    softbusListener_->CacheDeviceInfo(deviceId, deviceInfo);
    deviceInfo->addrNum = 0;
    softbusListener_->CacheDeviceInfo(deviceId, deviceInfo);
    softbusListener_->CacheDeviceInfo("", deviceInfo);
}

void GetUuidByNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if ((data == nullptr) || (size < maxStringLength)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString(maxStringLength);
    std::string uuid = "";
    softbusListener_->GetUuidByNetworkId(networkId.c_str(), uuid);
}

void SetHostPkgNameFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if ((data == nullptr) || (size < maxStringLength)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::string hostName = fdp.ConsumeRandomLengthString(maxStringLength);
    softbusListener_->SetHostPkgName(hostName);
}

void GetAttrFromCustomDataFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = cJSON_Parse(jsonString);
    DmDeviceInfo dmDevInfo;
    int32_t actionId = fdp.ConsumeIntegral<int32_t>();

    softbusListener_->GetAttrFromCustomData(jsonObject, dmDevInfo, actionId);
    cJSON_Delete(jsonObject);
}

void ConvertOsTypeToJsonFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t osType = fdp.ConsumeIntegral<int32_t>();
    std::string osTypeStr = "";
    softbusListener_->ConvertOsTypeToJson(osType, osTypeStr);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftBusListenerFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerFirstFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerSecondFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerThirdFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerForthFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerFifthFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerSixthFuzzTest(data, size);
    OHOS::DistributedHardware::SoftBusListenerSeventhFuzzTest(data, size);
    OHOS::DistributedHardware::PutOstypeDataFuzzTest(data, size);
    OHOS::DistributedHardware::CacheDeviceInfoFuzzTest(data, size);
    OHOS::DistributedHardware::GetUuidByNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::SetHostPkgNameFuzzTest(data, size);
    OHOS::DistributedHardware::GetAttrFromCustomDataFuzzTest(data, size);
    OHOS::DistributedHardware::ConvertOsTypeToJsonFuzzTest(data, size);
    return 0;
}
