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

#ifndef OHOS_DM_AUTH_PINCODE_MANAGER_3RD_H
#define OHOS_DM_AUTH_PINCODE_MANAGER_3RD_H

#include <memory>

#include "device_manager_data_struct_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "idevice_manager_service_listener_3rd.h"
#include "json_object.h"
#include "softbus_connector_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthPincodeContext;

class AuthPincodeManager3rd : public AuthManagerBase3rd,
                              public std::enable_shared_from_this<AuthPincodeManager3rd> {
public:
    AuthPincodeManager3rd(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
                std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
                std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector);
    virtual ~AuthPincodeManager3rd();
    int32_t ImportAuthCodeAndUid(const std::string &businessName, const std::string &authCode, int32_t uid);
    void GetAuthParam(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam);
    int32_t AuthPincode(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam,
        int32_t sessionId, uint64_t logicalSessionId);
    void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
protected:
    int32_t GetPinCode(std::string &code);
    std::shared_ptr<DmAuthPincodeContext> context_;
};

class AuthPincodeSrcManager : public AuthPincodeManager3rd {
public:
    AuthPincodeSrcManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
        std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
        std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector);
    virtual ~AuthPincodeSrcManager() override = default;

    // IDmDeviceAuthCallback3rd implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;
    // IDmDeviceAuthCallback3rd implement end

    // ISoftbusSessionCallback3rd implement begin
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnSessionDisable() override;
    void OnDataReceived(int32_t sessionId, const std::string &message) override;

    bool GetIsCryptoSupport() override;
    // ISoftbusSessionCallback3rd implement end
};

class AuthPincodeSinkManager : public AuthPincodeManager3rd {
public:
    AuthPincodeSinkManager(std::shared_ptr<SoftbusConnector3rd> softbusConnector,
        std::shared_ptr<IDeviceManagerServiceListener3rd> listener,
        std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector);
    virtual ~AuthPincodeSinkManager() override = default;

    // IDmDeviceAuthCallback3rd implement begin
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen) override;
    void AuthDeviceError(int64_t requestId, int32_t errorCode) override;
    void AuthDeviceFinish(int64_t requestId) override;
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen) override;
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams) override;
    // IDmDeviceAuthCallback3rd implement end

    // ISoftbusSessionCallback3rd implement begin
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) override;
    void OnSessionClosed(int32_t sessionId) override;
    void OnDataReceived(int32_t sessionId, const std::string &message) override;
    bool GetIsCryptoSupport() override;
    // ISoftbusSessionCallback3rd implement end
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_PINCODE_MANAGER_3RD_H