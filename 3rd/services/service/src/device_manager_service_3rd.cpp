/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "device_manager_service_3rd.h"

#include <random>
#include <dlfcn.h>
#include <functional>
#include <cstring>
#include <securec.h>

#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "deviceprofile_connector_3rd.h"
#include "kv_adapter_manager_3rd.h"
#include "multiple_user_connector_3rd.h"
#include "permission_manager_3rd.h"

constexpr const char* LIB_IMPL_3RD_NAME = "libdevicemanagerserviceimpl3rd.z.so";

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE_3RD(DeviceManagerService3rd);

namespace {
const int32_t DM_MIN_RANDOM = 1;
const int32_t DM_MAX_RANDOM = 9;
const int32_t DM_MIN_PINCODE_SIZE = 6;
const int32_t DM_MAX_PINCODE_SIZE = 1024;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
}

DeviceManagerService3rd::DeviceManagerService3rd()
{
    LOGI("DeviceManagerService3rd constructor");
    KVAdapterManager3rd::GetInstance().Init();
}

DeviceManagerService3rd::~DeviceManagerService3rd()
{
    LOGI("DeviceManagerService3rd destructor");
    KVAdapterManager3rd::GetInstance().UnInit();
}

bool DeviceManagerService3rd::IsDMServiceImpl3rdReady()
{
    std::lock_guard<ffrt::mutex> lock(isImpl3rdLoadLock_);
    if (isImpl3rdSoLoaded_ && (dmServiceImpl3rd_ != nullptr)) {
        return true;
    }
    LOGI("libDeviceManagerServiceImpl3rd start load.");
    dmServiceImpl3rdSoHandle_ = dlopen(LIB_IMPL_3RD_NAME, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (dmServiceImpl3rdSoHandle_ == nullptr) {
        dmServiceImpl3rdSoHandle_ = dlopen(LIB_IMPL_3RD_NAME, RTLD_NOW | RTLD_NODELETE);
    }
    if (dmServiceImpl3rdSoHandle_ == nullptr) {
        LOGE("load libDeviceManagerServiceImpl3rd so failed, errMsg: %{public}s.", dlerror());
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceImpl3rdFuncPtr)dlsym(dmServiceImpl3rdSoHandle_, "CreateDMServiceImpl3rdObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(dmServiceImpl3rdSoHandle_);
        dmServiceImpl3rdSoHandle_ = nullptr;
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImpl3rd_ = std::shared_ptr<IDeviceManagerServiceImpl3rd>(func());
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener3rd>();
    }
    if (dmServiceImpl3rd_->Initialize(listener_) != DM_OK) {
        dmServiceImpl3rd_ = nullptr;
        isImpl3rdSoLoaded_ = false;
        return false;
    }
    isImpl3rdSoLoaded_ = true;
    LOGI("Sussess.");
    return true;
}

int32_t DeviceManagerService3rd::ImportPinCode3rd(const std::string &businessName, const std::string &pinCode)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission.", businessName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (businessName.empty() || pinCode.empty()) {
        LOGE("Invalid parameter, businessName or pinCode is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, businessName: %{public}s", businessName.c_str());

    if (!IsDMServiceImpl3rdReady()) {
        LOGE("failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    int32_t ret = dmServiceImpl3rd_->ImportPinCode3rd(businessName, pinCode);
    LOGI("Completed");
    return ret;
}

int32_t DeviceManagerService3rd::GenRandInt(int32_t randMin, int32_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

std::string DeviceManagerService3rd::GeneratePinCode(uint32_t pinLength)
{
    if (pinLength < DM_MIN_PINCODE_SIZE || pinLength > DM_MAX_PINCODE_SIZE) {
        LOGE("pinLength error: Invalid para");
        return "";
    }
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::string pinCode = std::to_string(GenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM));
    uint32_t left_digit_count = pinLength - 1;
    while (pinCode.length() < left_digit_count) {
        uint64_t rest_num = gen();
        pinCode += std::to_string(rest_num);
    }
    pinCode = pinCode.substr(0, pinLength);
    return pinCode;
}

int32_t DeviceManagerService3rd::GeneratePinCode(uint32_t pinLength, std::string &pincode)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (pinLength < DM_MIN_PINCODE_SIZE || pinLength > DM_MAX_PINCODE_SIZE) {
        LOGE("Invalid pinLength: %{public}u", pinLength);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pinLength: %{public}u", pinLength);
    std::string generatedPinCode = GeneratePinCode(pinLength);
    int32_t length = static_cast<int32_t>(generatedPinCode.length());
    for (int32_t i = 0; i < length; i++) {
        if (!isdigit(generatedPinCode[i])) {
            LOGE("ImportAuthCode error: Invalid para, authCode format error.");
            return ERR_DM_INPUT_PARA_INVALID;
        }
    }
   
    pincode = generatedPinCode;
    LOGI("completed, pincode: %{public}zu", pincode.size());
    return DM_OK;
}

bool DeviceManagerService3rd::IsInvalidPeerTargetId(const PeerTargetId3rd &targetId)
{
    return targetId.deviceId.empty() && targetId.brMac.empty() && targetId.bleMac.empty() && targetId.wifiIp.empty();
}

int32_t DeviceManagerService3rd::AuthPincode(const PeerTargetId3rd &targetId,
    std::map<std::string, std::string> &authParam)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("Invalid parameter, params are empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, deviceId: %{public}s", targetId.deviceId.c_str());
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    int32_t ret = dmServiceImpl3rd_->AuthPincode(targetId, authParam);
    if (ret != DM_OK) {
        LOGE("AuthPincode failed, ret: %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int DeviceManagerService3rd::OnAuth3rdAclSessionOpened(int sessionId, int result)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnSessionOpened failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl3rd_->OnAuth3rdAclSessionOpened(sessionId, result);
}

void DeviceManagerService3rd::OnAuth3rdAclSessionClosed(int sessionId)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnSessionClosed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl3rd_->OnAuth3rdAclSessionClosed(sessionId);
}

void DeviceManagerService3rd::OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl3rd_->OnAuth3rdAclBytesReceived(sessionId, data, dataLen);
}

int DeviceManagerService3rd::OnAuth3rdSessionOpened(int sessionId, int result)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnSessionOpened failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl3rd_->OnAuth3rdSessionOpened(sessionId, result);
}

void DeviceManagerService3rd::OnAuth3rdSessionClosed(int sessionId)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnSessionClosed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl3rd_->OnAuth3rdSessionClosed(sessionId);
}

void DeviceManagerService3rd::OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("OnBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl3rd_->OnAuth3rdBytesReceived(sessionId, data, dataLen);
}

int32_t DeviceManagerService3rd::AuthDevice3rd(const PeerTargetId3rd &targetId,
    const std::map<std::string, std::string> &authParam)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("AuthDevice3rd failed: invalid target id.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string authParamStr = ConvertMapToJsonString(authParam);
    if (!authParam.empty() && authParamStr.empty()) {
        LOGE("AuthDevice3rd failed: invalid authParam.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImpl3rdReady()) {
        LOGE("failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl3rd_->AuthDevice3rd(targetId, authParam);
}

int32_t DeviceManagerService3rd::QueryTrustRelation(const std::string &businessName,
    std::vector<TrustDeviceInfo3rd> &trustedDeviceList)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (businessName.empty()) {
        LOGE("QueryTrustRelation failed: businessName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> acls;
    int32_t ret = KVAdapterManager3rd::GetInstance().GetAllByPrefix(ACL_PREFIX, acls);
    if (ret != DM_OK) {
        LOGE("GetAllByPrefix failed ret: %{public}d.", ret);
        return ret;
    }
    int32_t currentUserId = MultipleUserConnector3rd::GetCurrentAccountUserID();
    for (const auto &it : acls) {
        JsonObject json(it.second);
        AccessControl3rd access = json.Get<AccessControl3rd>();
        Access3rd &selfAccess = (access.trustDeviceId ==  access.accesser.deviceId) ? access.accessee : access.accesser;
        if (selfAccess.userId != currentUserId) {
            continue;
        }
        TrustDeviceInfo3rd deviceInfo;
        deviceInfo.trustDeviceId = access.trustDeviceId;
        deviceInfo.sessionKeyId = access.sessionKeyId;
        deviceInfo.createTime = access.createTime;
        deviceInfo.userId = currentUserId;
        deviceInfo.extra = access.extra;
        QuerySessionKey(currentUserId, access.sessionKeyId, deviceInfo);
        trustedDeviceList.push_back(deviceInfo);
    }
    return DM_OK;
}

void DeviceManagerService3rd::QuerySessionKey(int32_t userId, int32_t skId, TrustDeviceInfo3rd &deviceInfo)
{
    std::vector<unsigned char> sessionKey;
    int32_t ret = DeviceProfileConnector3rd::GetInstance().GetSessionKey(userId, skId, sessionKey);
    if (ret != DM_OK) {
        LOGE("GetSessionKey failed: %{public}d", ret);
        return;
    }
    uint32_t keyLen = sessionKey.size();
    if (keyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey too long, len: %{public}d", keyLen);
        return;
    }
    deviceInfo.sessionKey.key = (uint8_t*)calloc(keyLen, sizeof(uint8_t));
    if (deviceInfo.sessionKey.key == nullptr) {
        LOGE("calloc failed.");
        return;
    }
    if (memcpy_s(deviceInfo.sessionKey.key, keyLen, sessionKey.data(), keyLen) != DM_OK) {
        LOGE("memcpy_s failed.");
        (void)memset_s(deviceInfo.sessionKey.key, keyLen, 0, keyLen);
        free(deviceInfo.sessionKey.key);
        deviceInfo.sessionKey.key = nullptr;
        deviceInfo.sessionKey.keyLen = 0;
        return;
    }
    deviceInfo.sessionKey.keyLen = keyLen;
}

int32_t DeviceManagerService3rd::DeleteTrustRelation(const std::string &businessName, const std::string &peerDeviceId,
    const std::map<std::string, std::string> &unbindParam)
{
    if (!PermissionManager3rd::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission.");
        return ERR_DM_NO_PERMISSION;
    }
    if (businessName.empty() || peerDeviceId.empty()) {
        LOGE("businessName or peerDeviceId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string peerBusinessName = "";
    if (unbindParam.find(TAG_PEER_BUSINESS_NAME) != unbindParam.end()) {
        peerBusinessName = unbindParam.at(TAG_PEER_BUSINESS_NAME);
    }
    std::vector<std::string> delKeyVec;
    delKeyVec.push_back(peerDeviceId + businessName + peerBusinessName);
    int32_t ret = GetProxyDelInfo(peerDeviceId, unbindParam, delKeyVec);
    if (ret != DM_OK) {
        LOGI("GetProxyDelInfo error %{public}d", ret);
        return ret;
    }
    std::map<std::string, std::string> acls;
    ret = KVAdapterManager3rd::GetInstance().GetAllByPrefix(ACL_PREFIX, acls);
    if (ret != DM_OK) {
        LOGE("GetAllByPrefix failed ret: %{public}d.", ret);
        return ret;
    }
    for (const auto &it : acls) {
        JsonObject json(it.second);
        AccessControl3rd access = json.Get<AccessControl3rd>();
        std::string key = access.trustDeviceId + access.accesser.businessName + access.accessee.businessName;
        auto keyIt = std::find(delKeyVec.begin(), delKeyVec.end(), key);
        if (keyIt != delKeyVec.end()) {
            ret = KVAdapterManager3rd::GetInstance().DeleteByKey(it.first);
            if (ret != DM_OK) {
                LOGE("DeleteByKey failed ret: %{public}d.", ret);
                return ret;
            }
            DeviceProfileConnector3rd::GetInstance().DeleteSessionKey(access.accesser.userId, access.sessionKeyId);
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService3rd::GetProxyDelInfo(const std::string &peerDeviceId,
    const std::map<std::string, std::string> &unbindParam, std::vector<std::string> &delKeyVec)
{
    if (unbindParam.find(PARAM_KEY_SUBJECT_PROXYED_SUBJECTS) == unbindParam.end()) {
        LOGI("no subject proxyed apps");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = unbindParam.at(PARAM_KEY_SUBJECT_PROXYED_SUBJECTS);
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    if (allProxyObj.IsDiscarded()) {
        LOGI("subjectProxyOnesStr error");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_BUSINESS_NAME)) {
            LOGE("businessName invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        std::string businessName = item[TAG_BUSINESS_NAME].Get<std::string>();
        if (!IsString(item, TAG_PEER_BUSINESS_NAME)) {
            LOGE("peerBusinessName invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        std::string peerBusinessName = item[TAG_PEER_BUSINESS_NAME].Get<std::string>();
        delKeyVec.push_back(peerDeviceId + businessName + peerBusinessName);
    }
    return DM_OK;
}

int32_t DeviceManagerService3rd::HandleUserRemoved(int32_t removedUserId)
{
    LOGI("HandleUserRemoved userId: %{public}d", removedUserId);
    std::map<std::string, std::string> acls;
    int32_t ret = KVAdapterManager3rd::GetInstance().GetAllByPrefix(ACL_PREFIX, acls);
    if (ret != DM_OK) {
        LOGE("GetAllByPrefix failed ret: %{public}d.", ret);
        return ret;
    }
    for (const auto &it : acls) {
        JsonObject json(it.second);
        AccessControl3rd access = json.Get<AccessControl3rd>();
        Access3rd &selfAccess = (access.trustDeviceId ==  access.accesser.deviceId) ? access.accessee : access.accesser;
        if (selfAccess.userId != removedUserId) {
            continue;
        }
        ret = KVAdapterManager3rd::GetInstance().DeleteByKey(it.first);
        if (ret != DM_OK) {
            LOGE("DeleteByKey failed ret: %{public}d.", ret);
            return ret;
        }
        DeviceProfileConnector3rd::GetInstance().DeleteSessionKey(removedUserId, access.sessionKeyId);
    }
    return DM_OK;
}

int32_t DeviceManagerService3rd::HandleAccountLogoutEvent(int32_t userId, const std::string &accountId)
{
    LOGI("start");
    std::map<std::string, std::string> acls;
    int32_t ret = KVAdapterManager3rd::GetInstance().GetAllByPrefix(ACL_PREFIX, acls);
    if (ret != DM_OK) {
        LOGE("GetAllByPrefix failed ret: %{public}d.", ret);
        return ret;
    }
    for (const auto &it : acls) {
        JsonObject json(it.second);
        AccessControl3rd access = json.Get<AccessControl3rd>();
        Access3rd &selfAccess = (access.trustDeviceId ==  access.accesser.deviceId) ? access.accessee : access.accesser;
        if (selfAccess.userId != userId || selfAccess.accountId != accountId) {
            continue;
        }
        ret = KVAdapterManager3rd::GetInstance().DeleteByKey(it.first);
        if (ret != DM_OK) {
            LOGE("DeleteByKey failed ret: %{public}d.", ret);
            return ret;
        }
        DeviceProfileConnector3rd::GetInstance().DeleteSessionKey(userId, access.sessionKeyId);
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS