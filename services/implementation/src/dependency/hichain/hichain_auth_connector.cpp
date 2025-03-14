/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hichain_auth_connector.h"

#include "dm_log.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "hichain_connector_callback.h"
#include "parameter.h"
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<IDmDeviceAuthCallback> HiChainAuthConnector::dmDeviceAuthCallback_ = nullptr;
std::mutex HiChainAuthConnector::dmDeviceAuthCallbackMutex_;

void HiChainAuthConnector::FreeJsonString(char *jsonStr)
{
    if (jsonStr != nullptr) {
        cJSON_free(jsonStr);
        jsonStr = nullptr;
    }
}

HiChainAuthConnector::HiChainAuthConnector()
{
    deviceAuthCallback_ = {.onTransmit = HiChainAuthConnector::onTransmit,
                           .onSessionKeyReturned = HiChainAuthConnector::onSessionKeyReturned,
                           .onFinish = HiChainAuthConnector::onFinish,
                           .onError = HiChainAuthConnector::onError,
                           .onRequest = HiChainAuthConnector::onRequest};
    LOGI("hichain GetGaInstance success.");
}

HiChainAuthConnector::~HiChainAuthConnector()
{
    LOGI("HiChainAuthConnector::destructor.");
}

int32_t HiChainAuthConnector::RegisterHiChainAuthCallback(std::shared_ptr<IDmDeviceAuthCallback> callback)
{
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallback_ = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector::AuthDevice(int32_t pinCode, int32_t osAccountId, std::string udid, int64_t requestId)
{
    LOGI("HiChainAuthConnector::AuthDevice start.");
    JsonObject authParamJson;
    authParamJson["osAccountId"] = osAccountId;
    authParamJson["pinCode"] = std::to_string(pinCode);
    authParamJson["acquireType"] = AcquireType::P2P_BIND;
    std::string authParam = SafetyDump(authParamJson);
    LOGI("StartAuthDevice authParam %{public}s ,requestId %{public}" PRId64, GetAnonyString(authParam).c_str(),
        requestId);
    int32_t ret = StartAuthDevice(requestId, authParam.c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain authDevice failed, ret is %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::ProcessAuthData(int64_t requestId, std::string authData, int32_t osAccountId)
{
    LOGI("HiChainAuthConnector::ProcessAuthData start.");
    JsonObject jsonAuthParam;
    jsonAuthParam["osAccountId"] = osAccountId;
    jsonAuthParam["data"] = authData;
    int32_t ret = ProcessAuthDevice(requestId, SafetyDump(jsonAuthParam).c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain processData failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

bool HiChainAuthConnector::onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthDevice onTransmit, requestId %{public}" PRId64, requestId);
    if (dmDeviceAuthCallback_ == nullptr) {
        LOGE("HiChainAuthConnector::onTransmit dmDeviceAuthCallback_ is nullptr.");
        return false;
    }
    return dmDeviceAuthCallback_->AuthDeviceTransmit(requestId, data, dataLen);
}

char *HiChainAuthConnector::onRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("HiChainAuthConnector::onRequest start.");
    (void)requestId;
    (void)reqParams;
    if (dmDeviceAuthCallback_ == nullptr) {
        LOGE("HiChainAuthConnector::onRequest dmDeviceAuthCallback_ is nullptr.");
        return nullptr;
    }
    JsonObject jsonObj;
    int32_t pinCode = INVALID_PINCODE;
    if (dmDeviceAuthCallback_->GetPinCode(pinCode) == ERR_DM_FAILED || pinCode == INVALID_PINCODE) {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
        jsonObj[FIELD_PIN_CODE] = std::to_string(pinCode);
    }
    std::string deviceId = "";
    dmDeviceAuthCallback_->GetRemoteDeviceId(deviceId);
    jsonObj[FIELD_PEER_CONN_DEVICE_ID] = deviceId;
    std::string jsonStr = SafetyDump(jsonObj);
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

void HiChainAuthConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    LOGI("HiChainAuthConnector::onFinish reqId:%{public}" PRId64 ", operation:%{public}d.",
        requestId, operationCode);
    (void)returnData;
    if (dmDeviceAuthCallback_ == nullptr) {
        LOGE("HiChainAuthConnector::onFinish dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    dmDeviceAuthCallback_->AuthDeviceFinish(requestId);
}

void HiChainAuthConnector::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    LOGI("HichainAuthenCallBack::onError reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
        requestId, operationCode, errorCode);
    (void)operationCode;
    (void)errorReturn;
    if (dmDeviceAuthCallback_ == nullptr) {
        LOGE("HiChainAuthConnector::onError dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    int32_t dmErrorCode = ERR_DM_FAILED;
    if (errorCode == PROOF_MISMATCH) {
        dmErrorCode = ERR_DM_HICHAIN_PROOFMISMATCH;
    }
    dmDeviceAuthCallback_->AuthDeviceError(requestId, dmErrorCode);
}

void HiChainAuthConnector::onSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("HiChainAuthConnector::onSessionKeyReturned start.");
    if (dmDeviceAuthCallback_ == nullptr) {
        LOGE("HiChainAuthConnector::onSessionKeyReturned dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    dmDeviceAuthCallback_->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
}

int32_t HiChainAuthConnector::GenerateCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey)
{
    LOGI("HiChainAuthConnector::GenerateCredential start.");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_CREATE, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain generate credential failed.");
        FreeJsonString(returnData);
        return ERR_DM_FAILED;
    }
    std::string returnDataStr = static_cast<std::string>(returnData);
    FreeJsonString(returnData);
    JsonObject jsonObject(returnDataStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("Decode generate return data jsonStr error.");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, "result") || !IsString(jsonObject, "publicKey") ||
        jsonObject["result"].Get<int32_t>() != HC_SUCCESS) {
        LOGE("Hichain generate public key jsonObject invalied.");
        return ERR_DM_FAILED;
    }
    if (jsonObject["result"].Get<int32_t>() != 0) {
        LOGE("Hichain generate public key failed");
        return ERR_DM_FAILED;
    }
    publicKey = jsonObject["publicKey"].Get<std::string>();
    return DM_OK;
}

bool HiChainAuthConnector::QueryCredential(std::string &localUdid, int32_t osAccountId)
{
    LOGI("HiChainAuthConnector::QueryCredential start.");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_QUERY, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return false;
    }
    std::string returnDataStr = static_cast<std::string>(returnData);
    FreeJsonString(returnData);
    JsonObject jsonObject(returnDataStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("Decode query return data jsonStr error.");
        return false;
    }
    if (!IsInt32(jsonObject, "result") || jsonObject["result"].Get<int32_t>() == -1) {
        LOGE("Hichain generate public key failed.");
        return false;
    }
    if (!IsString(jsonObject, "publicKey") || jsonObject["result"].Get<int32_t>() == 1) {
        LOGI("Credential not exist.");
        return false;
    }
    return true;
}

int32_t HiChainAuthConnector::GetCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey)
{
    LOGI("HiChainAuthConnector::GetCredential");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_QUERY, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return ERR_DM_FAILED;
    }
    std::string returnDataStr = static_cast<std::string>(returnData);
    FreeJsonString(returnData);
    JsonObject jsonObject(returnDataStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("Decode query return data jsonStr error.");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, "result") || jsonObject["result"].Get<int32_t>() == -1) {
        LOGE("Hichain generate public key failed.");
        return ERR_DM_FAILED;
    }
    if (!IsString(jsonObject, "publicKey") || jsonObject["result"].Get<int32_t>() == 1) {
        LOGI("Credential not exist.");
        return ERR_DM_FAILED;
    }
    publicKey = jsonObject["publicKey"].Get<std::string>();
    return DM_OK;
}

int32_t HiChainAuthConnector::ImportCredential(int32_t osAccountId, std::string deviceId, std::string publicKey)
{
    LOGI("HiChainAuthConnector::ImportCredential");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = deviceId;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["publicKey"] = publicKey;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_IMPORT, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return ERR_DM_FAILED;
    }
    std::string returnDataStr = static_cast<std::string>(returnData);
    FreeJsonString(returnData);
    JsonObject jsonObject(returnDataStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("Decode import return data jsonStr error.");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, "result")) {
        LOGI("Hichain import public key jsonObject invalied.");
        return ERR_DM_FAILED;
    }
    int32_t result = jsonObject["result"].Get<int32_t>();
    if (result != 0) {
        LOGE("Hichain import public key result is %{public}d.", result);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::DeleteCredential(const std::string &deviceId, int32_t userId)
{
    LOGI("DeleteCredential start.");
    JsonObject jsonObj;
    jsonObj["deviceId"] = deviceId;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["osAccountId"] = userId;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_DELETE, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return false;
    }
    std::string returnDataStr = static_cast<std::string>(returnData);
    FreeJsonString(returnData);
    JsonObject jsonObject(returnDataStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("Decode import return data jsonStr error.");
        return false;
    }
    if (!IsInt32(jsonObject, "result")) {
        LOGI("Hichain delete credential result json key is invalid.");
        return ERR_DM_FAILED;
    }
    return jsonObject["result"].Get<int32_t>();
}
} // namespace DistributedHardware
} // namespace OHOS
