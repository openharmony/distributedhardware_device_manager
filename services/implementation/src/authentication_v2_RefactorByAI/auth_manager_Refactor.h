/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_MANAGER_V2_H
#define OHOS_DM_AUTH_MANAGER_V2_H

#include <memory>

#include "auth_ui_state_manager.h"
#include "dm_auth_manager_base.h"
#include "dm_device_info.h"
#include "ffrt.h"
#include "hichain_auth_connector.h"
#include "hichain_connector.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthContext;

class AuthManager : public AuthManagerBase,
                    public std::enable_shared_from_this<AuthManager> {
public:
    AuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                std::shared_ptr<HiChainConnector> hiChainConnector,
                std::shared_ptr<IDeviceManagerServiceListener> listener,
                std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthManager();

    virtual int32_t OnUserOperation(int32_t action, const std::string &params) = 0;

    int32_t BindTarget(const std::string &sessionName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId);

    std::string GeneratePincode();

    int32_t ImportAuthCode(const std::string &sessionName, const std::string &authCode);

    int32_t RegisterUiStateCallback(const std::string sessionName);

    int32_t UnRegisterUiStateCallback(const std::string sessionName);

    int32_t UnAuthenticateDevice(const std::string &sessionName, const std::string &udid, int32_t bindLevel);

    int32_t UnBindDevice(const std::string &sessionName, const std::string &udid,
        int32_t bindLevel, const std::string &extra);

    void HandleDeviceNotTrust(const std::string &udid);

    int32_t RegisterAuthenticationType(int32_t authenticationType);
    void OnScreenLocked();
    int32_t StopAuthenticateDevice(const std::string &sessionName);
    void NotifyRemoteFailed(int32_t sessionId, int32_t reason, uint64_t logicalSessionId);

    void SetAuthContext(std::shared_ptr<DmAuthContext> context);
    std::shared_ptr<DmAuthContext> GetAuthContext();
    void GetBindTargetParams(std::string &pkgName, PeerTargetId &targetId,
        std::map<std::string, std::string> &bindParam);
    void GetAuthCodeAndPkgName(std::string &pkgName, std::string &authCode);
    void SetBindTargetParams(const PeerTargetId &targetId);
    int32_t GetReason();
    int32_t CheckSrcNegotiateSrvParam(const std::string &pkgName, int64_t serviceId,
        const std::map<std::string, std::string> &bindParam);

    void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
    void RegisterStopTimerAndDelDpCallback(StopTimerAndDelDpCallback stopTimerAndDelDpCallback);
    void ClearSoftbusSessionCallback();
    void PrepareSoftbusSessionCallback();
    void DeleteTimer();

    int32_t HandleBusinessEvents(const std::string &businessId, int32_t action);

protected:
    std::shared_ptr<DmAuthContext> context_;
    ffrt::mutex bindParamMutex_;
    std::map<std::string, std::string> bindParam_;
    PeerTargetId targetId_;

    int32_t GetPinCode(std::string &code);
    void GetRemoteDeviceId(std::string &deviceId);
    bool ValidateAuthContext(const std::shared_ptr<DmAuthContext> &ctx);
private:
    void ParseHmlInfoInJsonObject(const JsonObject &jsonObject);
    void ParseProxyJsonObject(const JsonObject &jsonObject);
    void ParseServiceJsonObject(const JsonObject &jsonObject);
    void GetBindLevelByBundleName(std::string &bundleName, int32_t userId, int32_t &bindLevel);
    void ParseJsonObject(const JsonObject &jsonObject);
    void ParseAccessJsonObject(const JsonObject &jsonObject);
    void GetAuthParam(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    std::string GetBundleName(const JsonObject &jsonObject);
    void SetAuthType(int32_t authType);
    bool IsAuthTypeSupported(const int32_t &authType);
    bool IsAuthCodeReady(const std::string &sessionName);
    int32_t CheckAuthParamVaild(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    int32_t CheckProxyAuthParamVaild(const std::string &extra);
    int32_t CheckServiceAuthParamVaild(const JsonObject &jsonObject);
    void InitAuthState(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    int32_t AuthenticateDevice(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    void ParseUltrasonicSide(const JsonObject &jsonObject);
    void GetBindCallerInfo();
    int32_t GetBindLevel(int32_t bindLevel);
    void GetIsNeedJoinLnnParam(const std::map<std::string, std::string> &bindParam);
    void ParseServiceInfo(const std::string &extra);
    void GetConnDelayCloseTime(const std::string &extra);
    int32_t GetSrcUserIdByDisplayIdAndDeviceType(int32_t displayId, DmDeviceType deviceType);
    int32_t GetSrcCarUserIdByDisplayId(int32_t displayId);
    void GetDelayCloseConnTime(const JsonObject &jsonObject);
};

class AuthSrcManager : public AuthManager {
public:
    AuthSrcManager(std::shared_ptr<SoftbusConnector> softbusConnector,
        std::shared_ptr<HiChainConnector> hiChainConnector,
        std::shared_ptr<IDeviceManagerServiceListener> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSrcManager() override = default;

    int32_t OnUserOperation(int32_t action, const std::string &params) override;

    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;

    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnSessionDisable() override;
    void OnDataReceived(int32_t sessionId, std::string message) override;

    bool GetIsCryptoSupport() override;
};

class AuthSinkManager : public AuthManager {
public:
    AuthSinkManager(std::shared_ptr<SoftbusConnector> softbusConnector,
        std::shared_ptr<HiChainConnector> hiChainConnector,
        std::shared_ptr<IDeviceManagerServiceListener> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSinkManager() override = default;

    int32_t OnUserOperation(int32_t action, const std::string &params) override;

    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;

    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnDataReceived(int32_t sessionId, std::string message) override;
    bool GetIsCryptoSupport() override;
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_MANAGER_V2_H