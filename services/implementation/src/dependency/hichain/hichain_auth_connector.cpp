/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "dm_crypto.h"
#include "hichain_connector_callback.h"
#include "parameter.h"
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {

namespace {

constexpr int32_t HICHAIN_DATA_SIZE = 10240;

}

std::shared_ptr<IDmDeviceAuthCallback> HiChainAuthConnector::dmDeviceAuthCallback_ = nullptr;
std::map<int64_t, std::shared_ptr<IDmDeviceAuthCallback>> HiChainAuthConnector::dmDeviceAuthCallbackMap_;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
ffrt::mutex HiChainAuthConnector::dmDeviceAuthCallbackMutex_;
#else
std::mutex HiChainAuthConnector::dmDeviceAuthCallbackMutex_;
#endif

void HiChainAuthConnector::FreeJsonString(char *jsonStr)
{
    if (jsonStr != nullptr) {
        cJSON_free(jsonStr);
        jsonStr = nullptr;
    }
}

void HiChainAuthConnector::FreeCharArray(char *charArray)
{
    if (charArray != nullptr) {
        free(charArray);
        charArray = nullptr;
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
#else
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
#endif
    dmDeviceAuthCallback_ = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector::UnRegisterHiChainAuthCallback()
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
#else
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
#endif
    dmDeviceAuthCallback_ = nullptr;
    return DM_OK;
}

// 当前id为tokenId对应生成的requestId
int32_t HiChainAuthConnector::RegisterHiChainAuthCallbackById(int64_t id,
    std::shared_ptr<IDmDeviceAuthCallback> callback)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
#else
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
#endif
    dmDeviceAuthCallbackMap_[id] = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector::UnRegisterHiChainAuthCallbackById(int64_t id)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
#else
    std::lock_guard<std::mutex> lock(dmDeviceAuthCallbackMutex_);
#endif
    if (dmDeviceAuthCallbackMap_.find(id) != dmDeviceAuthCallbackMap_.end()) {
        dmDeviceAuthCallbackMap_[id] = nullptr;
    }
    return DM_OK;
}

std::shared_ptr<IDmDeviceAuthCallback> HiChainAuthConnector::GetDeviceAuthCallback(int64_t id)
{
    if (dmDeviceAuthCallbackMap_.find(id) != dmDeviceAuthCallbackMap_.end()) {
        LOGD("dmDeviceAuthCallbackMap_ id: %{public}" PRId64 ".", id);
        return dmDeviceAuthCallbackMap_[id];
    }
    LOGD("dmDeviceAuthCallbackMap_ not found, id: %{public}"
        PRId64 ".", id);
    // If the callback registered by the new protocol ID cannot be found, the callback registered
    // by the old protocol is used. However, the old protocol callback may be empty.
    return dmDeviceAuthCallback_;
}

int32_t HiChainAuthConnector::AuthDevice(const std::string &pinCode, int32_t osAccountId, std::string udid,
    int64_t requestId)
{
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode));
    LOGI("AuthDevice pinCodeHash: %{public}s", pinCodeHash.c_str());
    JsonObject authParamJson;
    authParamJson["osAccountId"] = osAccountId;
    authParamJson["pinCode"] = pinCode;
    authParamJson["acquireType"] = AcquireType::P2P_BIND;
    std::string authParam = authParamJson.Dump();
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
    LOGI("start.");
    JsonObject jsonAuthParam;
    jsonAuthParam["osAccountId"] = osAccountId;
    jsonAuthParam["data"] = authData;
    int32_t ret = ProcessAuthDevice(requestId, jsonAuthParam.Dump().c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain processData failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::ProcessCredData(int64_t authReqId, const std::string &data)
{
    LOGI("start.");
    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->processCredData(authReqId, reinterpret_cast<const uint8_t *>(data.c_str()),
        data.length(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain processData failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::AddCredential(int32_t osAccountId, const std::string &authParams, std::string &credId)
{
    char *returnData = NULL;
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->addCredential(osAccountId, authParams.c_str(), &returnData);
    if (ret != HC_SUCCESS || returnData == NULL) {
        LOGE("Hichain addCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    LOGI("success ret=%{public}d, returnData=%{public}s.", ret, GetAnonyString(returnData).c_str());
    credId = std::string(returnData);
    credManager->destroyInfo(&returnData);
    return DM_OK;
}

int32_t HiChainAuthConnector::ExportCredential(int32_t osAccountId, const std::string &credId, std::string &publicKey)
{
    LOGI("start. osAccountId=%{public}d, credId=%{public}s", osAccountId, GetAnonyString(credId).c_str());
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
    return DM_OK;
}

int32_t HiChainAuthConnector::AgreeCredential(int32_t osAccountId, const std::string selfCredId,
    const std::string &authParams, std::string &credId)
{
    LOGI("start.");
    char *returnData = NULL;
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->agreeCredential(osAccountId, selfCredId.c_str(), authParams.c_str(), &returnData);
    if (ret != HC_SUCCESS || returnData == NULL) {
        LOGE("Hichain agreeCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    credId = returnData;
    credManager->destroyInfo(&returnData);
    LOGI("leave agreeCredId=%{public}s.", GetAnonyString(credId).c_str());
    return DM_OK;
}

int32_t HiChainAuthConnector::DeleteCredential(int32_t osAccountId, const std::string &credId)
{
    LOGI("start. osAccountId=%{public}s, credId=%{public}s", GetAnonyInt32(osAccountId).c_str(),
        GetAnonyString(credId).c_str());
    const CredManager *credManager = GetCredMgrInstance();
    int32_t ret = credManager->deleteCredential(osAccountId, credId.c_str());
    if (ret != HC_SUCCESS) {
        LOGE("Hichain deleteCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId,
    const std::string &pinCode)
{
    LOGI("start. osAccountId=%{public}s, credId=%{public}s", GetAnonyInt32(osAccountId).c_str(),
        GetAnonyString(credId).c_str());
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
    return DM_OK;
}

int32_t HiChainAuthConnector::AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode)
{
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode));
    LOGI("AuthCredentialPinCode pinCodeHash: %{public}s", pinCodeHash.c_str());
    if (static_cast<int32_t>(pinCode.size()) < MIN_PINCODE_SIZE) {
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
    if (dataLen > HICHAIN_DATA_SIZE) {
        LOGE("dataLen = %{public}u is invalid.", dataLen);
        return false;
    }
    CHECK_NULL_RETURN(data, false);
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onTransmit dmDeviceAuthCallback_ is nullptr.");
        return false;
    }
    return dmDeviceAuthCallback->AuthDeviceTransmit(requestId, data, dataLen);
}

char *HiChainAuthConnector::onRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("start.");
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onRequest dmDeviceAuthCallback_ is nullptr.");
        return nullptr;
    }
    return dmDeviceAuthCallback->AuthDeviceRequest(requestId, operationCode, reqParams);
}

void HiChainAuthConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    LOGI("reqId:%{public}" PRId64 ", operation:%{public}d.",
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
    LOGI("reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
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
    LOGI("start.");
    if (sessionKeyLen > HICHAIN_DATA_SIZE) {
        LOGE("sessionKeyLen = %{public}u is invalid.", sessionKeyLen);
        return;
    }
    CHECK_NULL_VOID(sessionKey);
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector::onSessionKeyReturned dmDeviceAuthCallback_ is nullptr.");
        return;
    }
    dmDeviceAuthCallback->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
}

int32_t HiChainAuthConnector::GenerateCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey)
{
    LOGI("start.");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = jsonObj.Dump();
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
    ret = cm->queryCredentialByParams(userId, queryParams.Dump().c_str(), &credIdList);
    if (ret != DM_OK) {
        LOGE("HiChainAuthConnector::QueryCredentialInfo fail to query credential id list with ret %{public}d.", ret);
        FreeJsonString(credIdList);
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
            FreeJsonString(returnCredInfo);
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
        FreeJsonString(returnCredInfo);
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
    LOGI("start, deviceId: %{public}s, peerOsAccountId: %{public}s",
        GetAnonyString(localUdid).c_str(), GetAnonyInt32(peerOsAccountId).c_str());
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["peerOsAccountId"] = peerOsAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = jsonObj.Dump();
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
    LOGI("start");
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["deviceId"] = localUdid;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["flag"] = 1;
    std::string requestParam = jsonObj.Dump();
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
    LOGI("start, deviceId: %{public}s, peerOsAccountId: %{public}s",
        GetAnonyString(deviceId).c_str(), GetAnonyInt32(peerOsAccountId).c_str());
    JsonObject jsonObj;
    jsonObj["osAccountId"] = osAccountId;
    jsonObj["peerOsAccountId"] = peerOsAccountId;
    jsonObj["deviceId"] = deviceId;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["publicKey"] = publicKey;
    std::string requestParam = jsonObj.Dump();
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
    LOGI("start, deviceId: %{public}s, peerUserId: %{public}d",
        GetAnonyString(deviceId).c_str(), peerUserId);
    JsonObject jsonObj;
    jsonObj["deviceId"] = deviceId;
    jsonObj["acquireType"] = AcquireType::P2P_BIND;
    jsonObj["osAccountId"] = userId;
    jsonObj["peerOsAccountId"] = peerUserId;
    std::string requestParam = jsonObj.Dump();
    char *returnData = nullptr;
    if (ProcessCredential(CRED_OP_DELETE, requestParam.c_str(), &returnData) != HC_SUCCESS) {
        LOGE("Hichain query credential failed.");
        FreeJsonString(returnData);
        return ERR_DM_FAILED;
    }
    FreeJsonString(returnData);
    return DM_OK;
}

int32_t HiChainAuthConnector::AddTokensToCredential(const std::string &credId, int32_t userId,
    std::vector<std::string> &tokenIds)
{
    const CredManager *cm = GetCredMgrInstance();
    if (cm == nullptr) {
        LOGE("cm is null.");
        return ERR_DM_FAILED;
    }
    char *returnCredInfo = nullptr;
    int32_t ret = cm->queryCredInfoByCredId(userId, credId.c_str(), &returnCredInfo);
    if (ret != DM_OK) {
        FreeCharArray(returnCredInfo);
        LOGE("[HICHAIN]::QueryCredInfoByCredId failed, ret: %{public}d.", ret);
        return ret;
    }
    JsonObject credInfoJson(returnCredInfo);
    FreeCharArray(returnCredInfo);
    if (credInfoJson.IsDiscarded()) {
        LOGE("QueryCredInfoByCredId credential info jsonStr error");
        return ERR_DM_FAILED;
    }
    std::vector<std::string> appList;
    if (credInfoJson.Contains(FIELD_AUTHORIZED_APP_LIST)) {
        credInfoJson[FIELD_AUTHORIZED_APP_LIST].Get(appList);
    }
    appList.insert(appList.end(), tokenIds.begin(), tokenIds.end());
    JsonObject jsonObj;
    jsonObj[FIELD_AUTHORIZED_APP_LIST] = appList;
    ret = cm->updateCredInfo(userId, credId.c_str(), jsonObj.Dump().c_str());
    if (ret != DM_OK) {
        LOGE("[HICHAIN]::updateCredInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    return DM_OK;
}

int32_t HiChainAuthConnector::UpdateCredential(const std::string &credId, int32_t userId,
    std::vector<std::string> &tokenIds)
{
    const CredManager *cm = GetCredMgrInstance();
    if (cm == nullptr) {
        LOGE("cm is null.");
        return ERR_DM_FAILED;
    }
    char *returnCredInfo = nullptr;
    int32_t ret = cm->queryCredInfoByCredId(userId, credId.c_str(), &returnCredInfo);
    if (ret != DM_OK) {
        FreeCharArray(returnCredInfo);
        LOGE("[HICHAIN]::QueryCredInfoByCredId failed, ret: %{public}d.", ret);
        return ret;
    }
    JsonObject credInfoJson(returnCredInfo);
    FreeCharArray(returnCredInfo);
    if (credInfoJson.IsDiscarded()) {
        LOGE("QueryCredInfoByCredId credential info jsonStr error");
        return ERR_DM_FAILED;
    }
    JsonObject jsonObj;
    jsonObj[FIELD_AUTHORIZED_APP_LIST] = tokenIds;
    ret = cm->updateCredInfo(userId, credId.c_str(), jsonObj.Dump().c_str());
    if (ret != DM_OK) {
        LOGE("[HICHAIN]::updateCredInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
