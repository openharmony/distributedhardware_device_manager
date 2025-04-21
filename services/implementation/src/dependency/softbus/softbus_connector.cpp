/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "softbus_connector.h"

#include <algorithm>
#include <securec.h>
#include <unistd.h>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "dm_softbus_cache.h"
#include "json_object.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t SOFTBUS_SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t SOFTBUS_DISCOVER_DEVICE_INFO_MAX_SIZE = 100;
const int32_t SOFTBUS_TRUSTDEVICE_UUIDHASH_INFO_MAX_SIZE = 100;

constexpr const char* WIFI_IP = "WIFI_IP";
constexpr const char* WIFI_PORT = "WIFI_PORT";
constexpr const char* BR_MAC = "BR_MAC";
constexpr const char* BLE_MAC = "BLE_MAC";
constexpr const char* ETH_IP = "ETH_IP";
constexpr const char* ETH_PORT = "ETH_PORT";
constexpr const char* NCM_IP = "NCM_IP";
constexpr const char* NCM_PORT = "NCM_PORT";
namespace {
    const char* TAG_ACL = "accessControlTable";
    const char* TAG_DMVERSION = "dmVersion";
}

std::string SoftbusConnector::remoteUdidHash_ = "";
std::map<std::string, std::shared_ptr<DeviceInfo>> SoftbusConnector::discoveryDeviceInfoMap_ = {};
std::unordered_map<std::string, std::string> SoftbusConnector::deviceUdidMap_ = {};
std::vector<ProcessInfo> SoftbusConnector::processInfoVec_ = {};
std::mutex SoftbusConnector::discoveryDeviceInfoMutex_;
std::mutex SoftbusConnector::deviceUdidLocks_;
std::mutex SoftbusConnector::processInfoVecMutex_;
std::shared_ptr<ISoftbusConnectorCallback> SoftbusConnector::connectorCallback_ = nullptr;

SoftbusConnector::SoftbusConnector()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    softbusSession_ = std::make_shared<SoftbusSession>();
    hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
#endif
    LOGD("SoftbusConnector constructor.");
}

SoftbusConnector::~SoftbusConnector()
{
    LOGD("SoftbusConnector destructor.");
}

void SoftbusConnector::SyncAclList(int32_t userId, std::string credId,
    int32_t sessionKeyId, int32_t aclId)
{
    LOGI("SyncAclList userId:%{public}d, credId:%{public}s, sessionKeyId:%{public}d, aclId:%{public}d",
        userId, credId.c_str(), sessionKeyId, aclId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    // 根据skid删除sk，删除skid
    int32_t ret = DeviceProfileConnector::GetInstance().DeleteSessionKey(userId, sessionKeyId);
    if (ret != DM_OK) {
        LOGE("SyncAclList DeleteSessionKey failed.");
    }
    if (hiChainAuthConnector_ != nullptr) {
        // 根据凭据id 删除sink端多余的凭据
        ret = hiChainAuthConnector_->DeleteCredential(userId, credId);
        if (ret != DM_OK) {
            LOGE("SyncAclList DeleteCredential failed.");
        }
    }
    // 删除本条acl
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(aclId);
#endif
}


#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
int32_t SoftbusConnector::SyncLocalAclList5_1_0(const std::string localUdid, const std::string remoteUdid,
    DistributedDeviceProfile::AccessControlProfile &localAcl, std::vector<std::string> &acLStrList)
{
    bool res = DeviceProfileConnector::GetInstance().ChecksumAcl(localAcl, acLStrList);
    if (res) {
        return DM_OK;
    }
    if (localAcl.GetAccesser().GetAccesserDeviceId() == localUdid &&
        localAcl.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
        LOGI("SyncLocalAclListProcess Src.");
        SyncAclList(localAcl.GetAccesser().GetAccesserUserId(),
            localAcl.GetAccesser().GetAccesserCredentialIdStr(),
            localAcl.GetAccesser().GetAccesserSessionKeyId(), localAcl.GetAccessControlId());
    }
    if (localAcl.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
        localAcl.GetAccessee().GetAccesseeDeviceId() == localUdid) {
        LOGI("SyncLocalAclListProcess Sink.");
        SyncAclList(localAcl.GetAccessee().GetAccesseeUserId(),
            localAcl.GetAccessee().GetAccesseeCredentialIdStr(),
            localAcl.GetAccessee().GetAccesseeSessionKeyId(), localAcl.GetAccessControlId());
    }
    return DM_OK;
}

int32_t SoftbusConnector::ParaseAclChecksumList(const std::string &jsonString,
    std::vector<AclHashItem> &remoteAllAclList)
{
    JsonObject aclChecksumjson(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    aclChecksumjson.Parse(jsonString);
    if (aclChecksumjson.IsDiscarded()) {
        LOGE("ParseSyncMessage aclChecksumjson error");
        return ERR_DM_FAILED;
    }
    DeviceProfileConnector::GetInstance().AclHashVecFromJson(aclChecksumjson, remoteAllAclList);
    return DM_OK;
}

int32_t SoftbusConnector::GetLocalVersion(const std::string localUdid, const std::string remoteUdid,
    std::string &localVersion, DistributedDeviceProfile::AccessControlProfile &localAcl)
{
    int32_t ret = ERR_DM_FAILED;
    if (localAcl.GetAccesser().GetAccesserDeviceId() == localUdid &&
        localAcl.GetAccessee().GetAccesseeDeviceId() == remoteUdid) {
        std::string extraInfo = localAcl.GetAccesser().GetAccesserExtraData();
        ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, localVersion);
    } else if (localAcl.GetAccesser().GetAccesserDeviceId() == remoteUdid &&
        localAcl.GetAccessee().GetAccesseeDeviceId() == localUdid) {
            std::string extraInfo = localAcl.GetAccessee().GetAccesseeExtraData();
        ret = DeviceProfileConnector::GetInstance().GetVersionByExtra(extraInfo, localVersion);
    }
    return DM_OK;
}
#endif

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void SoftbusConnector::SortAclListDesc(const std::vector<AclHashItem> &remoteAllAclList,
    std::vector<std::string> &aclVerDesc, std::map<std::string, AclHashItem> &remoteAllAclMap)
{
    aclVerDesc.clear();
    remoteAllAclMap.clear();
    for (const auto &item : remoteAllAclList) {
        aclVerDesc.push_back(item.version);
        remoteAllAclMap[item.version] = item;
    }

    std::sort(aclVerDesc.begin(), aclVerDesc.end(), [](const std::string &a, const std::string &b) {
        return CompareVersion(a, b);
    });
}

std::string SoftbusConnector::MatchTargetVersion(const std::string &localVersion,
    const std::vector<std::string> &remoteVerDesc)
{
    if (remoteVerDesc.empty()) {
        return localVersion;
    }

    // if local version bigger than remote max version, use remote max version to process acl aging.
    if (CompareVersion(localVersion, remoteVerDesc[0])) {
        return remoteVerDesc[0];
    }

    // if local version equal or smaller than remote max version, use local version to process acl aging.
    return localVersion;
}
#endif

int32_t SoftbusConnector::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::vector<AclHashItem> remoteAllAclList;
    int32_t ret = ParaseAclChecksumList(remoteAclList, remoteAllAclList);
    if (ret != DM_OK) {
        LOGE("SyncLocalAclListProcess TAG_ACL error");
        return ret;
    }

    std::vector<std::string> aclVerDesc;
    std::map<std::string, AclHashItem> remoteAllAclMap;
    SortAclListDesc(remoteAllAclList, aclVerDesc, remoteAllAclMap);
    std::string matchVersion = MatchTargetVersion(DM_CURRENT_VERSION, aclVerDesc);

    std::vector<std::string> remoteAclHashList = {};
    if (remoteAllAclMap.find(matchVersion) != remoteAllAclMap.end()) {
        remoteAclHashList = remoteAllAclMap[matchVersion].aclHashList;
    }
    std::vector<DistributedDeviceProfile::AccessControlProfile> localAclList =
        DeviceProfileConnector::GetInstance().GetAclList(localDevUserInfo.deviceId, localDevUserInfo.userId,
            remoteDevUserInfo.deviceId, remoteDevUserInfo.userId);

    for (auto &localAcl : localAclList) {
        int32_t versionNum = 0;
        if (!GetVersionNumber(matchVersion, versionNum)) {
            LOGE("SyncLocalAclList GetVersionNumber error");
            continue;
        }
        switch (versionNum) {
            case DM_VERSION_INT_5_1_0:
                ret = SyncLocalAclList5_1_0(localDevUserInfo.deviceId, remoteDevUserInfo.deviceId, localAcl,
                    remoteAclHashList);
            default:
                LOGE("versionNum is invaild, ver: %{public}d", versionNum);
                break;
        }
    }
    return DM_OK;
#else
    (void)localDevUserInfo;
    (void)remoteDevUserInfo;
    (void)remoteAclList;
    return DM_OK;
#endif
}

int32_t SoftbusConnector::GetAclListHash(const DevUserInfo &localDevUserInfo,
    const DevUserInfo &remoteDevUserInfo, std::string &aclList)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DeviceProfileConnector::GetInstance().GetAclListHashStr(localDevUserInfo, remoteDevUserInfo, aclList);
#else
    (void)localDevUserInfo;
    (void)remoteDevUserInfo;
    (void)aclList;
    return DM_OK;
#endif
}

int32_t SoftbusConnector::RegisterConnectorCallback(std::shared_ptr<ISoftbusConnectorCallback> callback)
{
    connectorCallback_ = callback;
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterConnectorCallback()
{
    connectorCallback_ = nullptr;
    return DM_OK;
}

int32_t SoftbusConnector::RegisterSoftbusStateCallback(const std::shared_ptr<ISoftbusStateCallback> callback)
{
    deviceStateManagerCallback_ = callback;
    return DM_OK;
}

int32_t SoftbusConnector::UnRegisterSoftbusStateCallback()
{
    deviceStateManagerCallback_ = nullptr;
    return DM_OK;
}

void SoftbusConnector::JoinLnn(const std::string &deviceId, bool isForceJoin)
{
    std::string connectAddr;
    LOGI("start, deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
    auto addrInfo = GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("addrInfo is nullptr.");
        return;
    }
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BLE &&
        Crypto::ConvertHexStringToBytes(addrInfo->info.ble.udidHash, UDID_HASH_LEN,
        remoteUdidHash_.c_str(), remoteUdidHash_.length()) != DM_OK) {
        LOGE("convert remoteUdid hash failed, remoteUdidHash_: %{public}s.", GetAnonyString(remoteUdidHash_).c_str());
        return;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo.get(), OnSoftbusJoinLNNResult, isForceJoin);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNN failed, ret: %{public}d.", ret);
    }
    return;
}

void SoftbusConnector::JoinLnn(const std::string &deviceId, const std::string &remoteUdidHash)
{
    std::string connectAddr;
    LOGI("start, deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
    auto addrInfo = GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("addrInfo is nullptr.");
        return;
    }
    if (addrInfo->type == ConnectionAddrType::CONNECTION_ADDR_BLE &&
        Crypto::ConvertHexStringToBytes(addrInfo->info.ble.udidHash, UDID_HASH_LEN,
        remoteUdidHash.c_str(), remoteUdidHash.length()) != DM_OK) {
        LOGE("convert remoteUdid hash failed, remoteUdidHash_: %{public}s.", GetAnonyString(remoteUdidHash).c_str());
        return;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo.get(), OnSoftbusJoinLNNResult, false);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNN failed, ret: %{public}d.", ret);
    }
    return;
}

void SoftbusConnector::JoinLNNBySkId(int32_t sessionId, int32_t sessionKeyId, int32_t remoteSessionKeyId,
    std::string udid, std::string udidHash)
{
    LOGI("start, JoinLNNBySkId sessionId: %{public}d, udid: %{public}s.", sessionId, GetAnonyString(udid).c_str());
    std::string connectAddr;
    auto addrInfo = GetConnectAddr(udid, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("addrInfo is nullptr.");
        return;
    }
    LOGI("addrInfo->type: %{public}d", addrInfo->type);
    if (addrInfo->type == CONNECTION_ADDR_BLE) {
        if (Crypto::ConvertHexStringToBytes(addrInfo->info.ble.udidHash, UDID_HASH_LEN, udidHash.c_str(),
                                            udidHash.length()) != DM_OK) {
            LOGE("convert remoteUdid hash failed, udidHash: %{public}s.", GetAnonyString(udidHash).c_str());
            return;
        }
    }

    addrInfo->deviceKeyId.hasDeviceKeyId = true;  // 总线修改后适配
    if (sessionKeyId > 0 && remoteSessionKeyId > 0) {
        addrInfo->deviceKeyId.localDeviceKeyId = sessionKeyId; // 总线修改后适配
        addrInfo->deviceKeyId.remoteDeviceKeyId = remoteSessionKeyId; // 总线修改后适配
        LOGI("sessionKeyId valid");
    } else {
        addrInfo->deviceKeyId.localDeviceKeyId = 0; // 总线修改后适配
        addrInfo->deviceKeyId.remoteDeviceKeyId = 0; // 总线修改后适配
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, addrInfo.get(), OnSoftbusJoinLNNResult, false);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNNBySkId failed, ret: %{public}d.", ret);
    }
}

void SoftbusConnector::JoinLnnByHml(int32_t sessionId, int32_t sessionKeyId, int32_t remoteSessionKeyId)
{
    LOGI("start, JoinLnnByHml sessionId: %{public}d.", sessionId);
    ConnectionAddr addrInfo;
    addrInfo.type = CONNECTION_ADDR_SESSION_WITH_KEY;
    addrInfo.info.session.sessionId = sessionId;
    addrInfo.deviceKeyId.hasDeviceKeyId = true;
    if (sessionKeyId > 0 && remoteSessionKeyId > 0) {
        addrInfo.deviceKeyId.localDeviceKeyId = sessionKeyId;
        addrInfo.deviceKeyId.remoteDeviceKeyId = remoteSessionKeyId;
        LOGI("sessionKeyId valid");
    } else {
        addrInfo.deviceKeyId.localDeviceKeyId = 0;
        addrInfo.deviceKeyId.remoteDeviceKeyId = 0;
    }
    int32_t ret = ::JoinLNN(DM_PKG_NAME, &addrInfo, OnSoftbusJoinLNNResult, false);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]JoinLNN failed, ret: %{public}d.", ret);
    }
}

int32_t SoftbusConnector::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    LOGI("start, networkId: %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    return SoftbusCache::GetInstance().GetUdidFromCache(networkId, udid);
}

int32_t SoftbusConnector::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    LOGI("start, networkId: %{public}s.", GetAnonyString(std::string(networkId)).c_str());
    return SoftbusCache::GetInstance().GetUuidFromCache(networkId, uuid);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
std::shared_ptr<SoftbusSession> SoftbusConnector::GetSoftbusSession()
{
    return softbusSession_;
}
#endif

bool SoftbusConnector::HaveDeviceInMap(std::string deviceId)
{
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    auto iter = discoveryDeviceInfoMap_.find(deviceId);
    if (iter == discoveryDeviceInfoMap_.end()) {
        LOGE("deviceInfo not found by deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        return false;
    }
    return true;
}

ConnectionAddr *SoftbusConnector::GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type)
{
    if (deviceInfo == nullptr) {
        return nullptr;
    }
    for (uint32_t i = 0; i < deviceInfo->addrNum; ++i) {
        if (deviceInfo->addr[i].type == type) {
            return &deviceInfo->addr[i];
        }
    }
    return nullptr;
}

std::shared_ptr<DeviceInfo> SoftbusConnector::GetDeviceInfoFromMap(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    auto iter = discoveryDeviceInfoMap_.find(deviceId);
    if (iter == discoveryDeviceInfoMap_.end()) {
        LOGE("deviceInfo not found by deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<ConnectionAddr> SoftbusConnector::SetAddrAndJson(const ConnectionAddr *addr,
    JsonObject &jsonPara, std::string &connectAddr)
{
    if (addr == nullptr) {
        LOGE("Param  not valid, addr is null.");
        return nullptr;
    }
    std::shared_ptr<ConnectionAddr> connectAddrPtr = std::make_shared<ConnectionAddr>();
    *connectAddrPtr = *addr;
    connectAddr = jsonPara.Dump();
    return connectAddrPtr;
}

std::shared_ptr<ConnectionAddr> SoftbusConnector::GetConnectAddr(const std::string &deviceId, std::string &connectAddr)
{
    std::shared_ptr<DeviceInfo> deviceInfo = GetDeviceInfoFromMap(deviceId);
    if (deviceInfo == nullptr || deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        LOGE("deviceInfo addrNum not valid, addrNum: %{public}d.", (deviceInfo != nullptr) ? deviceInfo->addrNum : 0);
        return nullptr;
    }
    JsonObject jsonPara;
    ConnectionAddr *addr = GetConnectAddrByType(deviceInfo.get(), ConnectionAddrType::CONNECTION_ADDR_ETH);
    if (addr != nullptr) {
        LOGI("[SOFTBUS]get ETH ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        jsonPara[ETH_IP] = addr->info.ip.ip;
        jsonPara[ETH_PORT] = addr->info.ip.port;
        return SetAddrAndJson(addr, jsonPara, connectAddr);
    }
    addr = GetConnectAddrByType(deviceInfo.get(), ConnectionAddrType::CONNECTION_ADDR_WLAN);
    if (addr != nullptr) {
        jsonPara[WIFI_IP] = addr->info.ip.ip;
        jsonPara[WIFI_PORT] = addr->info.ip.port;
        LOGI("[SOFTBUS]get WLAN ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        return SetAddrAndJson(addr, jsonPara, connectAddr);
    }
    addr = GetConnectAddrByType(deviceInfo.get(), ConnectionAddrType::CONNECTION_ADDR_BR);
    if (addr != nullptr) {
        jsonPara[BR_MAC] = addr->info.br.brMac;
        LOGI("[SOFTBUS]get BR ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        return SetAddrAndJson(addr, jsonPara, connectAddr);
    }
    addr = GetConnectAddrByType(deviceInfo.get(), ConnectionAddrType::CONNECTION_ADDR_BLE);
    if (addr != nullptr) {
        jsonPara[BLE_MAC] = addr->info.ble.bleMac;
        addr->info.ble.priority = BLE_PRIORITY_HIGH;
        return SetAddrAndJson(addr, jsonPara, connectAddr);
    }
    addr = GetConnectAddrByType(deviceInfo.get(), ConnectionAddrType::CONNECTION_ADDR_NCM);
    if (addr != nullptr) {
        LOGI("[SOFTBUS]get NCM ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        jsonPara[NCM_IP] = addr->info.ip.ip;
        jsonPara[NCM_PORT] = addr->info.ip.port;
        return SetAddrAndJson(addr, jsonPara, connectAddr);
    }
    LOGE("[SOFTBUS]failed to get ConnectionAddr for deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
    return nullptr;
}

void SoftbusConnector::ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceInfo &dmDeviceInfo)
{
    if (memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice memset_s failed.");
        return;
    }

    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), deviceInfo.devId,
                 std::min(sizeof(dmDeviceInfo.deviceId), sizeof(deviceInfo.devId))) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed.");
        return;
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), deviceInfo.devName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(deviceInfo.devName))) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
        return;
    }

    dmDeviceInfo.deviceTypeId = deviceInfo.devType;
    dmDeviceInfo.range = deviceInfo.range;
}

void SoftbusConnector::ConvertDeviceInfoToDmDevice(const DeviceInfo &deviceInfo, DmDeviceBasicInfo &dmDeviceBasicInfo)
{
    if (memset_s(&dmDeviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice memset_s failed.");
        return;
    }

    if (memcpy_s(dmDeviceBasicInfo.deviceId, sizeof(dmDeviceBasicInfo.deviceId), deviceInfo.devId,
                 std::min(sizeof(dmDeviceBasicInfo.deviceId), sizeof(deviceInfo.devId))) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceId data failed.");
        return;
    }

    if (memcpy_s(dmDeviceBasicInfo.deviceName, sizeof(dmDeviceBasicInfo.deviceName), deviceInfo.devName,
                 std::min(sizeof(dmDeviceBasicInfo.deviceName), sizeof(deviceInfo.devName))) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
        return;
    }

    dmDeviceBasicInfo.deviceTypeId = deviceInfo.devType;
}

void SoftbusConnector::OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result)
{
    (void)networkId;
    LOGI("[SOFTBUS]OnSoftbusJoinLNNResult, result: %{public}d.", result);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (addr == nullptr) {
        LOGE("addr is null.");
        return;
    }
    if (addr->type != CONNECTION_ADDR_SESSION_WITH_KEY) {
        LOGI("addr type is not session.");
        return;
    }
    int32_t sessionId = addr->info.session.sessionId;
    CHECK_NULL_VOID(connectorCallback_);
    connectorCallback_->OnSoftbusJoinLNNResult(sessionId, networkId, result);
#else
    (void)addr;
    (void)networkId;
    (void)result;
#endif
}

std::string SoftbusConnector::GetDeviceUdidByUdidHash(const std::string &udidHash)
{
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    for (auto &iter : deviceUdidMap_) {
        if (iter.second == udidHash) {
            return iter.first;
        }
    }
    LOGE("fail to GetUdidByUdidHash, udidHash: %{public}s", GetAnonyString(udidHash).c_str());
    return udidHash;
}

std::string SoftbusConnector::GetDeviceUdidHashByUdid(const std::string &udid)
{
    {
        std::lock_guard<std::mutex> lock(deviceUdidLocks_);
        auto iter = deviceUdidMap_.find(udid);
        if (iter != deviceUdidMap_.end()) {
            return deviceUdidMap_[udid];
        }
    }

    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(udid, reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(udid).c_str());
        return "";
    }
    LOGI("get udidhash: %{public}s by udid: %{public}s.", GetAnonyString(udidHash).c_str(),
        GetAnonyString(udid).c_str());
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    deviceUdidMap_[udid] = udidHash;
    return udidHash;
}

void SoftbusConnector::EraseUdidFromMap(const std::string &udid)
{
    std::lock_guard<std::mutex> lock(deviceUdidLocks_);
    auto iter = deviceUdidMap_.find(udid);
    if (iter == deviceUdidMap_.end()) {
        return;
    }
    size_t mapSize = deviceUdidMap_.size();
    if (mapSize >= SOFTBUS_TRUSTDEVICE_UUIDHASH_INFO_MAX_SIZE) {
        deviceUdidMap_.erase(udid);
    }
}

std::string SoftbusConnector::GetLocalDeviceName()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return "";
    }
    return nodeBasicInfo.deviceName;
}

int32_t SoftbusConnector::GetLocalDeviceTypeId()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        return DmDeviceType::DEVICE_TYPE_UNKNOWN;
    }
    return nodeBasicInfo.deviceTypeId;
}

std::string SoftbusConnector::GetLocalDeviceNetworkId()
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalDeviceNetworkId failed, ret: %{public}d.", ret);
        return "";
    }
    return nodeBasicInfo.networkId;
}

int32_t SoftbusConnector::AddMemberToDiscoverMap(const std::string &deviceId, std::shared_ptr<DeviceInfo> deviceInfo)
{
    if (deviceId.empty()) {
        LOGE("AddMemberToDiscoverMap failed, deviceId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(discoveryDeviceInfoMutex_);
    discoveryDeviceInfoMap_[deviceId] = std::move(deviceInfo);
    deviceInfo = nullptr;
    return DM_OK;
}

std::string SoftbusConnector::GetNetworkIdByDeviceId(const std::string &deviceId)
{
    LOGI("SoftbusConnector::GetNetworkIdByDeviceId");
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return "";
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, reinterpret_cast<char *>(nodeBasicInfo->networkId),
            NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
        }
        if (reinterpret_cast<char *>(mUdid) == deviceId) {
            FreeNodeInfo(nodeInfo);
            return static_cast<std::string>(nodeBasicInfo->networkId);
        }
    }
    FreeNodeInfo(nodeInfo);
    return "";
}

void SoftbusConnector::SetProcessInfo(ProcessInfo processInfo)
{
    LOGI("SoftbusConnector::SetProcessInfo");
    std::lock_guard<std::mutex> lock(processInfoVecMutex_);
    processInfoVec_.push_back(processInfo);
}

void SoftbusConnector::SetProcessInfoVec(std::vector<ProcessInfo> processInfoVec)
{
    LOGI("SoftbusConnector::SetProcessInfoVec");
    std::lock_guard<std::mutex> lock(processInfoVecMutex_);
    processInfoVec_ = processInfoVec;
}

std::vector<ProcessInfo> SoftbusConnector::GetProcessInfo()
{
    LOGI("In");
    std::lock_guard<std::mutex> lock(processInfoVecMutex_);
    return processInfoVec_;
}

void SoftbusConnector::ClearProcessInfo()
{
    LOGI("In");
    std::lock_guard<std::mutex> lock(processInfoVecMutex_);
    processInfoVec_.clear();
}

void SoftbusConnector::HandleDeviceOnline(std::string deviceId, int32_t authForm)
{
    LOGI("SoftbusConnector::HandleDeviceOnline");
    deviceStateManagerCallback_->OnDeviceOnline(deviceId, authForm);
    return;
}

void SoftbusConnector::HandleDeviceOffline(std::string deviceId)
{
    LOGI("SoftbusConnector::HandleDeviceOffline");
    deviceStateManagerCallback_->OnDeviceOffline(deviceId);
    return;
}

void SoftbusConnector::DeleteOffLineTimer(std::string &udidHash)
{
    LOGI("SoftbusConnector::DeleteOffLineTimer");
    remoteUdidHash_ = udidHash;
    if (deviceStateManagerCallback_ != nullptr) {
        deviceStateManagerCallback_->DeleteOffLineTimer(udidHash);
    }
}

// isHash：传入的deviceId是否为哈希值
bool SoftbusConnector::CheckIsOnline(const std::string &targetDeviceIdHash, bool isHash)
{
    LOGI("Check the device is online.");
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return false;
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, reinterpret_cast<char *>(nodeBasicInfo->networkId),
            NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
        }
        std::string udid = reinterpret_cast<char *>(mUdid);
        if ((isHash == false && udid == targetDeviceIdHash) ||
            (isHash == true && Crypto::Sha256(udid).find(targetDeviceIdHash) == 0)) {
            LOGI("The device is online.");
            FreeNodeInfo(nodeInfo);
            return true;
        }
    }
    LOGI("The device is not online.");
    FreeNodeInfo(nodeInfo);
    return false;
}

bool SoftbusConnector::CheckIsOnline(const std::string &targetDeviceId)
{
    return CheckIsOnline(targetDeviceId, false);
}

DmDeviceInfo SoftbusConnector::GetDeviceInfoByDeviceId(const std::string &deviceId)
{
    LOGI("SoftbusConnector::GetDeviceInfoBydeviceId");
    DmDeviceInfo info;
    int32_t deviceCount = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    if (GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount) != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed.");
        return info;
    }
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(deviceId, reinterpret_cast<uint8_t *>(deviceIdHash)) != DM_OK) {
        LOGE("get deviceIdHash by deviceId: %{public}s failed.", GetAnonyString(deviceId).c_str());
        FreeNodeInfo(nodeInfo);
        return info;
    }
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        uint8_t mUdid[UDID_BUF_LEN] = {0};
        if (GetNodeKeyInfo(DM_PKG_NAME, nodeBasicInfo->networkId, NodeDeviceInfoKey::NODE_KEY_UDID,
            mUdid, sizeof(mUdid)) != DM_OK) {
            LOGE("[SOFTBUS]GetNodeKeyInfo failed.");
            FreeNodeInfo(nodeInfo);
            return info;
        }
        std::string udid = reinterpret_cast<char *>(mUdid);
        if (udid != deviceId) {
            continue;
        } else {
            ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, info);
            if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, deviceIdHash, DM_MAX_DEVICE_ID_LEN) != 0) {
                LOGE("Get deviceId: %{public}s failed.", GetAnonyString(deviceId).c_str());
            }
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    return info;
}

void SoftbusConnector::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo)
{
    if (memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice memset_s failed.");
        return;
    }

    if (memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.networkId), sizeof(nodeBasicInfo.networkId))) != EOK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed.");
        return;
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != EOK) {
        LOGE("ConvertDeviceInfoToDmDevice copy deviceName data failed.");
        return;
    }

    dmDeviceInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    std::string extraData = dmDeviceInfo.extraData;
    JsonObject extraJson;
    if (!extraData.empty()) {
        extraJson.Parse(extraData);
    }
    if (!extraJson.IsDiscarded()) {
        extraJson[PARAM_KEY_OS_TYPE] = nodeBasicInfo.osType;
        extraJson[PARAM_KEY_OS_VERSION] = std::string(nodeBasicInfo.osVersion);
        dmDeviceInfo.extraData = ToString(extraJson);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
