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

#ifndef OHOS_DM_AUTH_MANAGER_ACL_H
#define OHOS_DM_AUTH_MANAGER_ACL_H

#include <memory>
#include "dm_auth_manager_base.h"
#include "ffrt.h"
#include "hichain_auth_connector.h"
#include "idevice_manager_service_listener_3rd.h"
#include "json_object.h"
#include "softbus_connector_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthContext;

class AuthManager : public AuthManagerBase,
                    public std::enable_shared_from_this<AuthManager> {
public:
    AuthManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
                std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
                std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthManager();
    int32_t ImportAuthCodeAndUid(const std::string &businessName, const std::string &authCode, uint32_t uid);
    void GetAuthParam(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam);
    int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam,
        int32_t sessionId, uint64_t logicalSessionId);
    void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
protected:
    int32_t GetPinCode(std::string &code);
    void ParseProxyParam(const std::map<std::string, std::string> &authParam);
    void GetBindLevelByProcessName(std::string &processName, int32_t userId, int32_t &bindLevel);
    std::shared_ptr<DmAuthContext> context_;
};

class AuthSrcManager : public AuthManager {
public:
    AuthSrcManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
        std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSrcManager() override = default;

    // IDmDeviceAuthCallback implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
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
    AuthSinkManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
        std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
        std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    virtual ~AuthSinkManager() override = default;

    // IDmDeviceAuthCallback implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
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
#endif  // OHOS_DM_AUTH_MANAGER_ACL_H
