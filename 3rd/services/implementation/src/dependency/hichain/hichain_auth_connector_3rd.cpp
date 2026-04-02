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

#include "hichain_auth_connector_3rd.h"

#include <cstdlib>

#include "parameter.h"

#include "dm_anonymous_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t HICHAIN_DATA_SIZE = 10240;
}
std::map<int64_t, std::shared_ptr<IDmDeviceAuthCallback3rd>> HiChainAuthConnector3rd::dmDeviceAuthCallbackMap_;
ffrt::mutex HiChainAuthConnector3rd::dmDeviceAuthCallbackMutex_;
HiChainAuthConnector3rd::HiChainAuthConnector3rd()
{
    deviceAuthCallback_ = {.onTransmit = HiChainAuthConnector3rd::onTransmit,
                           .onSessionKeyReturned = HiChainAuthConnector3rd::onSessionKeyReturned,
                           .onFinish = HiChainAuthConnector3rd::onFinish,
                           .onError = HiChainAuthConnector3rd::onError,
                           .onRequest = HiChainAuthConnector3rd::onRequest};
    LOGI("hichain GetGaInstance success.");
}

HiChainAuthConnector3rd::~HiChainAuthConnector3rd()
{
    for (auto& pair : dmDeviceAuthCallbackMap_) {
        pair.second = nullptr;
    }
    dmDeviceAuthCallbackMap_.clear();
    LOGI("HiChainAuthConnector3rd::destructor.");
}

// 当前id为tokenId对应生成的requestId
int32_t HiChainAuthConnector3rd::RegisterHiChainAuthCallbackById(int64_t id,
    std::shared_ptr<IDmDeviceAuthCallback3rd> callback)
{
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
    dmDeviceAuthCallbackMap_[id] = callback;
    return DM_OK;
}

int32_t HiChainAuthConnector3rd::UnRegisterHiChainAuthCallbackById(int64_t id)
{
    std::lock_guard<ffrt::mutex> lock(dmDeviceAuthCallbackMutex_);
    if (dmDeviceAuthCallbackMap_.find(id) != dmDeviceAuthCallbackMap_.end()) {
        dmDeviceAuthCallbackMap_[id] = nullptr;
    }
    return DM_OK;
}

std::shared_ptr<IDmDeviceAuthCallback3rd> HiChainAuthConnector3rd::GetDeviceAuthCallback(int64_t id)
{
    if (dmDeviceAuthCallbackMap_.find(id) != dmDeviceAuthCallbackMap_.end()) {
        LOGD("HiChainAuthConnector3rd::GetDeviceAuthCallback dmDeviceAuthCallbackMap_ id: %{public}" PRId64 ".", id);
        return dmDeviceAuthCallbackMap_[id];
    }
    return nullptr;
}

int32_t HiChainAuthConnector3rd::AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId,
    const std::string &pinCode)
{
    std::string pinCodeHash = GetAnonyString(Crypto3rd::Sha256(pinCode));
    LOGI("AuthCredentialPinCode pinCodeHash: %{public}s", pinCodeHash.c_str());
    if (static_cast<int32_t>(pinCode.size()) < MIN_PINCODE_SIZE) {
        LOGE("HiChainAuthConnector3rd::AuthCredentialPinCode failed, pinCode size is %{public}zu.", pinCode.size());
        return ERR_DM_FAILED;
    }

    JsonObject jsonAuthParam;

    jsonAuthParam[FIELD_PIN_CODE] = pinCode;
    jsonAuthParam[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);

    std::string authParams = jsonAuthParam.Dump();

    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->authCredential(osAccountId, authReqId, authParams.c_str(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("HiChainAuthConnector3rd::AuthCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

bool HiChainAuthConnector3rd::onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthDevice onTransmit, requestId %{public}" PRId64, requestId);
    if (dataLen > HICHAIN_DATA_SIZE) {
        LOGE("dataLen = %{public}u is invalid.", dataLen);
        return false;
    }
    CHECK_NULL_RETURN(data, false);
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector3rd::onTransmit dmDeviceAuthCallback is nullptr.");
        return false;
    }
    return dmDeviceAuthCallback->AuthDeviceTransmit(requestId, data, dataLen);
}

char *HiChainAuthConnector3rd::onRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("HiChainAuthConnector3rd::onRequest start.");
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector3rd::onRequest dmDeviceAuthCallback is nullptr.");
        return nullptr;
    }
    return dmDeviceAuthCallback->AuthDeviceRequest(requestId, operationCode, reqParams);
}

void HiChainAuthConnector3rd::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    LOGI("HiChainAuthConnector3rd::onFinish reqId:%{public}" PRId64 ", operation:%{public}d.",
        requestId, operationCode);
    (void)returnData;
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector3rd::onFinish dmDeviceAuthCallback is nullptr.");
        return;
    }
    dmDeviceAuthCallback->AuthDeviceFinish(requestId);
}

void HiChainAuthConnector3rd::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    LOGI("onError reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
        requestId, operationCode, errorCode);
    (void)operationCode;
    (void)errorReturn;
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector3rd::onError dmDeviceAuthCallback is nullptr.");
        return;
    }
    int32_t dmErrorCode = ERR_DM_FAILED;
    if (errorCode == PROOF_MISMATCH) {
        dmErrorCode = ERR_DM_HICHAIN_PROOFMISMATCH;
    }
    dmDeviceAuthCallback->AuthDeviceError(requestId, dmErrorCode);
}

void HiChainAuthConnector3rd::onSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("onSessionKeyReturned start, reqId:%{public}" PRId64 "", requestId);
    if (sessionKeyLen > HICHAIN_DATA_SIZE) {
        LOGE("sessionKeyLen = %{public}u is invalid.", sessionKeyLen);
        return;
    }
    CHECK_NULL_VOID(sessionKey);
    auto dmDeviceAuthCallback = GetDeviceAuthCallback(requestId);
    if (dmDeviceAuthCallback == nullptr) {
        LOGE("HiChainAuthConnector3rd::onSessionKeyReturned dmDeviceAuthCallback is nullptr.");
        return;
    }
    dmDeviceAuthCallback->AuthDeviceSessionKey(requestId, sessionKey, sessionKeyLen);
}

int32_t HiChainAuthConnector3rd::ProcessCredData(int64_t authReqId, const std::string &data)
{
    LOGI("HiChainAuthConnector3rd::ProcessAuthData start.");
    const CredAuthManager *credAuthManager = GetCredAuthInstance();
    int32_t ret = credAuthManager->processCredData(authReqId, reinterpret_cast<const uint8_t *>(data.c_str()),
        data.length(), &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("Hichain processData failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS