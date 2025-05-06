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

#include <cstdlib>
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
std::map<int64_t, std::shared_ptr<IDmDeviceAuthCallback>> HiChainAuthConnector::dmDeviceAuthCallbackMap_;
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
    for (auto& pair : dmDeviceAuthCallbackMap_) {
        pair.second = nullptr;
    }
    dmDeviceAuthCallbackMap_.clear();
    dmDeviceAuthCallback_ = nullptr;
    LOGI("HiChainAuthConnector::destructor.");
}

int32_t HiChainAuthConnector::RegisterHiChainAuthCallback(std::shared_ptr<IDmDeviceAuthCallback> callback)
{
    if (dmDeviceAuthCallbackMap_.find(id) == dmDeviceAuthCallbackMap_.end()) {
        return ERR_DM_FAILED;
    }
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallback_ = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector::UnRegisterHiChainAuthCallback()
{
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallback_ = nullptr;
    return DM_OK;
}

// 当前id为tokenId对应生成的requestId
int32_t HiChainAuthConnector::RegisterHiChainAuthCallbackById(int64_t id,
    std::shared_ptr<IDmDeviceAuthCallback> callback)
{
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallbackMap_[id] = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector::UnRegisterHiChainAuthCallbackById(int64_t id)
{
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallbackMap_[id] = nullptr;
    return DM_OK;
}

std::shared_ptr<IDmDeviceAuthCallback> HiChainAuthConnector::GetDeviceAuthCallback(int64_t id)
{
    if (dmDeviceAuthCallbackMap_.find(id) != dmDeviceAuthCallbackMap_.end()) {
        LOGD("HiChainAuthConnector::GetDeviceAuthCallback dmDeviceAuthCallbackMap_ id: %{public}" PRId64 ".", id);
        return dmDeviceAuthCallbackMap_[id];
    }
    LOGD("HiChainAuthConnector::GetDeviceAuthCallback dmDeviceAuthCallbackMap_ not found, id: %{public}"
        PRId64 ".", id);
    // If the callback registered by the new protocol ID cannot be found, the callback registered
    // by the old protocol is used. However, the old protocol callback may be empty.
    return dmDeviceAuthCallback_;
}

int32_t HiChainAuthConnector::AuthDevice(const std::string &pinCode, int32_t osAccountId, std::string udid,
    int64_t requestId)
{
    LOGI("HiChainAuthConnector::AuthDevice start.");
    JsonObject authParamJson;
    authParamJson["osAccountId"] = osAccountId;
    authParamJson["pinCode"] = pinCode;
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

int32_t HiChainAuthConnector::ProcessCredData(int64_t authReqId, const std::string &data)
{
    LOGI("HiChainAuthConnector::ProcessCredData start.");
    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->processCredData(authReqId, reinterpret_cast<const uint8_t *>(data.c_str()),
        data.length(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain processData failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    LOGI("HiChainAuthConnector::ProcessCredData leave.");
    return DM_OK;
}

int32_t HiChainAuthConnector::AddCredential(int32_t osAccountId, const std::string &authParams, std::string &credId)
{
    LOGI("HiChainAuthConnector::AddCredential start.");
    LOGI("HiChainAuthConnector::AddCredential osAccount=%{public}d, authParams=%{public}s\n",
        osAccountId, authParams.c_str());
    char *returnData = NULL;
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->addCredential(osAccountId, authParams.c_str(), &returnData);
    if (ret != HC_SUCCESS || returnData == NULL) {
        LOGE("Hichain addCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    LOGI("HiChainAuthConnector::AddCredential addCredential success ret=%{public}d, returnData=%{public}s.",
        ret, returnData);
    credId = std::string(returnData);
    LOGI("HiChainAuthConnector::AddCredential addCredId=%{public}s.", credId.c_str());
    credManager->destroyInfo(&returnData);
    LOGI("HiChainAuthConnector::AddCredential leave.");
    return DM_OK;
}

int32_t HiChainAuthConnector::ExportCredential(int32_t osAccountId, const std::string &credId, std::string &publicKey)
{
    LOGI("HiChainAuthConnector::ExportCredential start. osAccountId=%{public}d, credId=%{public}s",
        osAccountId, credId.c_str());
    char *returnData = NULL;
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->exportCredential(osAccountId, credId.c_str(), &returnData);
    if (ret != HC_SUCCESS || returnData == NULL) {
        LOGE("Hichain exportCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }

    JsonObject jsonAuthParam(returnData);
    credManager->destroyInfo(&returnData);
    if (jsonAuthParam.IsDiscarded() || !jsonAuthParam["keyValue"].IsString()) {
        LOGE("Hichain exportCredential failed, returnData is invalid.");
        return ERR_DM_FAILED;
    }

    publicKey = jsonAuthParam["keyValue"].Get<std::string>();
    LOGI("HiChainAuthConnector::ExportCredential leave. publicKey=%{public}s", publicKey.c_str());
    return DM_OK;
}

int32_t HiChainAuthConnector::AgreeCredential(int32_t osAccountId, const std::string selfCredId,
    const std::string &authParams, std::string &credId)
{
    LOGI("HiChainAuthConnector::AgreeCredential start.");
    char *returnData = NULL;
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->agreeCredential(osAccountId, selfCredId.c_str(), authParams.c_str(), &returnData);
    if (ret != HC_SUCCESS || returnData == NULL) {
        LOGE("Hichain agreeCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    credId = returnData;
    credManager->destroyInfo(&returnData);
    LOGI("HiChainAuthConnector::AgreeCredential leave agreeCredId=%{public}s.", credId.c_str());
    return DM_OK;
}

int32_t HiChainAuthConnector::DeleteCredential(int32_t osAccountId, const std::string &credId)
{
    LOGI("HiChainAuthConnector::DeleteCredential start. osAccountId=%{public}d, credId=%{public}s", osAccountId,
        credId.c_str());
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->deleteCredential(osAccountId, credId.c_str());
    if (ret != HC_SUCCESS) {
        LOGE("Hichain deleteCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    LOGI("HiChainAuthConnector::DeleteCredential leave.");
    return DM_OK;
}

int32_t HiChainAuthConnector::AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId,
    const std::string &pinCode)
{
    LOGI("HiChainAuthConnector::AuthCredential start. osAccountId=%{public}d, credId=%{public}s", osAccountId,
        credId.c_str());
    if (credId.empty() && pinCode.empty()) {
        LOGE("HiChainAuthConnector::AuthCredential failed, credId and pinCode is empty.");
        return ERR_DM_FAILED;
    }

    JsonObject jsonAuthParam;
    if (!credId.empty()) {
        jsonAuthParam["credId"] = credId;
    }
    if (!pinCode.empty()) {
        jsonAuthParam["pinCode"] = pinCode;
    }
    std::string authParams = jsonAuthParam.Dump();

    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->authCredential(osAccountId, authReqId, authParams.c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("HiChainAuthConnector::AuthCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    LOGI("HiChainAuthConnector::AuthCredential leave.");
    return DM_OK;
}

int32_t HiChainAuthConnector::AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode)
{
    LOGI("HiChainAuthConnector::AuthCredential start.");
    if (pinCode.size() < MIN_PINCODE_SIZE) {
        LOGE("HiChainAuthConnector::AuthCredentialPinCode failed, pinCode size is %{public}zu.", pinCode.size());
        return ERR_DM_FAILED;
    }

    JsonObject jsonAuthParam;

    jsonAuthParam[FIELD_PIN_CODE] = pinCode;
    jsonAuthParam[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);

    std::string authParams = jsonAuthParam.Dump();

    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->authCredential(osAccountId, authReqId, authParams.c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("HiChainAuthConnector::AuthCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

bool HiChainAuthConnector::onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthDevice onTransmit, requestId %{public}" PRId64, requestId);
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onTransmit dmDeviceAuthCallback_ is nullptr.");
        return false;
    }
    return dmDeviceAuthCallback->AuthDeviceTransmit(requestId, data, dataLen);
}

char *HiChainAuthConnector::onRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("HiChainAuthConnector::onRequest start.");
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onRequest dmDeviceAuthCallback_ is nullptr.");
        return nullptr;
    }
    return dmDeviceAuthCallback->AuthDeviceRequest(requestId, operationCode, reqParams);
}

void HiChainAuthConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    LOGI("HiChainAuthConnector::onFinish reqId:%{public}" PRId64 ", operation:%{public}d.",
        requestId, operationCode);
    (void)returnData;
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onFinish dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    dmDeviceAuthCallback->AuthDeviceFinish(requestId);
}

void HiChainAuthConnector::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    LOGI("HichainAuthenCallBack::onError reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
        requestId, operationCode, errorCode);
    (void)operationCode;
    (void)errorReturn;
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onError dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    int32_t dmErrorCode = ERR_DM_FAILED;
    if (errorCode == PROOF_MISMATCH) {
        dmErrorCode = ERR_DM_HICHAIN_PROOFMISMATCH;
    }
    dmDeviceAuthCallback->AuthDeviceError(requestId, dmErrorCode);
}

void HiChainAuthConnector::onSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("HiChainAuthConnector::onSessionKeyReturned start.");
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onSessionKeyReturned dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    dmDeviceAuthCallback->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
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
    if (!IsString(jsonObject, "publicKey")) {
        LOGE("Hichain generate public key jsonObject invalied.");
        return ERR_DM_FAILED;
    }
    publicKey = jsonObject["publicKey"].Get<std::string>();
    return DM_OK;
}

int32_t HiChainAuthConnector::QueryCredentialInfo(int32_t userId, const JsonObject &queryParams,
    JsonObject &resultJson)
{
    int32_t ret;

    const CredManager *cm = GetCredMgrInstance();
    char *credIdList = nullptr;
    ret = cm->queryCredentialByParams(userId, queryParams.Dump().c_str(),
        &credIdList);
    if (ret != DM_OK) {
        LOGE("HiChainAuthConnector::QueryCredentialInfo fail to query credential id list with ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    JsonObject credIdListJson(credIdList);
    FreeJsonString(credIdList);
    if (credIdListJson.IsDiscarded()) {
        LOGE("HiChainAuthConnector::QueryCredentialInfo credential id list to jsonStr error");
        return ERR_DM_FAILED;
    }

    for (const auto& element : credIdListJson.Items()) {
        if (!element.IsString()) {
            continue;
        }
        std::string credId = element.Get<std::string>();

        char *returnCredInfo = nullptr;
        ret = cm->queryCredInfoByCredId(userId, credId.c_str(), &returnCredInfo);
        if (ret != DM_OK) {
            LOGE("HiChainAuthConnector::QueryCredentialInfo fail to query credential info.");
            return ERR_DM_FAILED;
        }
        JsonObject credInfoJson(returnCredInfo);
        FreeJsonString(returnCredInfo);
        if (credInfoJson.IsDiscarded()) {
            LOGE("HiChainAuthConnector::QueryCredentialInfo credential info jsonStr error");
            return ERR_DM_FAILED;
        }

        resultJson.Insert(credId, credInfoJson);
    }

    return DM_OK;
}

int32_t HiChainAuthConnector::QueryCredInfoByCredId(int32_t userId, std::string credId, JsonObject &resultJson)
{
    const CredManager *cm = GetCredMgrInstance();
    char *returnCredInfo = nullptr;
    int32_t ret = cm->queryCredInfoByCredId(userId, credId.c_str(), &returnCredInfo);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]::QueryCredInfoByCredId failed, ret: %{public}d.", ret);
        return ret;
    }
    JsonObject credInfoJson(returnCredInfo);
    FreeJsonString(returnCredInfo);
    if (credInfoJson.IsDiscarded()) {
        LOGE("QueryCredInfoByCredId credential info jsonStr error");
        return ERR_DM_FAILED;
    }
    resultJson.Insert(credId, credInfoJson);
    return DM_OK;
}

bool HiChainAuthConnector::QueryCredential(std::string &localUdid, int32_t osAccountId, int32_t peerOsAccountId)
{
    LOGI("QueryCredential start, deviceId: %{public}s, peerOsAccountId: %{public}d",
        GetAnonyString(localUdid).c_str(), peerOsAccountId);
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["peerOsAccountId"] = peerOsAccountId;
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
    if (!IsString(jsonObject, "publicKey")) {
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
    if (!IsString(jsonObject, "publicKey")) {
        LOGI("Credential not exist.");
        return ERR_DM_FAILED;
    }
    publicKey = jsonObject["publicKey"].Get<std::string>();
    return DM_OK;
}

int32_t HiChainAuthConnector::ImportCredential(int32_t osAccountId, int32_t peerOsAccountId, std::string deviceId,
    std::string publicKey)
{
    LOGI("ImportCredential start, deviceId: %{public}s, peerOsAccountId: %{public}d",
        GetAnonyString(deviceId).c_str(), peerOsAccountId);
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["peerOsAccountId"] = peerOsAccountId;
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
    FreeJsonString(returnData);
    return DM_OK;
}

int32_t HiChainAuthConnector::DeleteCredential(const std::string &deviceId, int32_t userId, int32_t peerUserId)
{
    LOGI("DeleteCredential start, deviceId: %{public}s, peerUserId: %{public}d",
        GetAnonyString(deviceId).c_str(), peerUserId);
    JsonObject jsonObj;
    jsonObj["deviceId"] = deviceId;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["osAccountId"] = userId;
    jsonObj["peerOsAccountId"] = peerUserId;
    std::string requestParam = SafetyDump(jsonObj);
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_DELETE, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return ERR_DM_FAILED;
    }
    FreeJsonString(returnData);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
