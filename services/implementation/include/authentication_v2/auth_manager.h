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

    // External API begin
    /**
     * @tc.name: DmAuthManager::OnUserOperation
     * @tc.desc: User Operation of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    virtual int32_t OnUserOperation(int32_t action, const std::string &params) = 0;

    /**
     * @tc.name: AuthManager::GeneratePincode
     * @tc.desc: Generate Pincode of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t BindTarget(const std::string &sessionName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId);

    /**
     * @tc.name: AuthManager::OnUserOperation
     * @tc.desc: User Operation of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    std::string GeneratePincode();

    /**
     * @tc.name: AuthManager::ImportAuthCode
     * @tc.desc: Import auth code
     * @tc.type: FUNC
     */
    int32_t ImportAuthCode(const std::string &sessionName, const std::string &authCode);

    /**
     * @tc.name: AuthManager::RegisterUiStateCallback
     * @tc.desc: Register ui state callback
     * @tc.type: FUNC
     */
    int32_t RegisterUiStateCallback(const std::string sessionName);

    /**
     * @tc.name: AuthManager::UnRegisterUiStateCallback
     * @tc.desc: Unregister ui state callback
     * @tc.type: FUNC
     */
    int32_t UnRegisterUiStateCallback(const std::string sessionName);

    /**
     * @tc.name: AuthManager::UnAuthenticateDevice
     * @tc.desc: UnAuthenticate Device of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t UnAuthenticateDevice(const std::string &sessionName, const std::string &udid, int32_t bindLevel);

    /**
     * @brief UnBind device.
     * @param sessionName package name.
     * @param deviceId device id.
     * @return Return 0 if success.
     */
    int32_t UnBindDevice(const std::string &sessionName, const std::string &udid,
        int32_t bindLevel, const std::string &extra);

    void HandleDeviceNotTrust(const std::string &udid);

    int32_t RegisterAuthenticationType(int32_t authenticationType);
    void OnScreenLocked();
    int32_t StopAuthenticateDevice(const std::string &sessionName);
    // External API begin end

    // Internal API begin
    void SetAuthContext(std::shared_ptr<DmAuthContext> context);
    std::shared_ptr<DmAuthContext> GetAuthContext();
    static bool IsHmlSessionType(const std::string &sessionType);
    int32_t GetTokenIdByBundleName(int32_t userId, std::string &bundleName, int64_t &tokenId);
    void GetBindTargetParams(std::string &pkgName, PeerTargetId &targetId,
        std::map<std::string, std::string> &bindParam);
    void SetBindTargetParams(const PeerTargetId &targetId);
    int32_t GetReason();

    // Internal API end
    void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
    void ClearSoftbusSessionCallback();
    void PrepareSoftbusSessionCallback();
protected:
    std::shared_ptr<DmAuthContext> context_;
    std::map<std::string, std::string> bindParam_;
    PeerTargetId targetId_;

    int32_t GetPinCode(std::string &code);
    void GetRemoteDeviceId(std::string &deviceId);
private:
    void ParseHmlInfoInJsonObject(const JsonObject &jsonObject);
    void ParseJsonObject(const JsonObject &jsonObject);
    void GetAuthParam(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    std::string GetBundleName(const JsonObject &jsonObject);
    void SetAuthType(int32_t authType);
    bool IsAuthTypeSupported(const int32_t &authType);
    bool IsAuthCodeReady(const std::string &sessionName);
    int32_t CheckAuthParamVaild(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    void InitAuthState(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    int32_t AuthenticateDevice(const std::string &sessionName, int32_t authType,
        const std::string &deviceId, const std::string &extra);
    void ParseUltrasonicSide(const JsonObject &jsonObject);
};

class AuthSrcManager : public AuthManager {
public:
    AuthSrcManager(std::shared_ptr<SoftbusConnector> softbusConnector,
        std::shared_ptr<HiChainConnector> hiChainConnector,
        std::shared_ptr<IDeviceManagerServiceListener> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSrcManager() override = default;

    // External API begin
    int32_t OnUserOperation(int32_t action, const std::string &params) override;
    // External API end

    // IDmDeviceAuthCallback implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;
    // IDmDeviceAuthCallback implement end

    // ISoftbusSessionCallback implement begin
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnSessionDisable() override;
    void OnDataReceived(int32_t sessionId, std::string message) override;

    bool GetIsCryptoSupport() override;
    // ISoftbusSessionCallback implement end
};

class AuthSinkManager : public AuthManager {
public:
    AuthSinkManager(std::shared_ptr<SoftbusConnector> softbusConnector,
        std::shared_ptr<HiChainConnector> hiChainConnector,
        std::shared_ptr<IDeviceManagerServiceListener> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSinkManager() override = default;

    // External API begin
    int32_t OnUserOperation(int32_t action, const std::string &params) override;
    // External API end

    // IDmDeviceAuthCallback implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;
    // IDmDeviceAuthCallback implement end

    // ISoftbusSessionCallback implement begin
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnDataReceived(int32_t sessionId, std::string message) override;
    bool GetIsCryptoSupport() override;
    // ISoftbusSessionCallback implement end
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_MANAGER_V2_H