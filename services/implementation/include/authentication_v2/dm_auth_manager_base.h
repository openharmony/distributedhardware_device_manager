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

#ifndef OHOS_DM_AUTH_ADAPTER_V2_H
#define OHOS_DM_AUTH_ADAPTER_V2_H

#include <string>
#include <memory>
#include <map>
#include <cstdint>

#include "softbus_session_callback.h"
#include "softbus_connector_callback.h"
#include "softbus_leavelnn_callback.h"
#include "hichain_connector_callback.h"
#include "auth_request_state.h"
#include "auth_response_state.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
extern const char* TAG_DMVERSION;
extern const char* TAG_DM_VERSION_V2;           // compatible for new and old protocol
extern const char* TAG_EDITION;
extern const char* TAG_DATA;
extern const char* TAG_DATA_LEN;
extern const char* TAG_BUNDLE_NAME;
extern const char* TAG_BUNDLE_NAME_V2;
extern const char* TAG_PEER_BUNDLE_NAME;
extern const char* TAG_PEER_BUNDLE_NAME_V2;
extern const char* TAG_PEER_PKG_NAME;
extern const char* TAG_BIND_LEVEL;
extern const char* TAG_REPLY;
extern const char* TAG_APP_THUMBNAIL2;          // Naming Add 2 to resolve conflicts with TAG_APP_THUMBNAIL
extern const char* TAG_AUTH_FINISH;
extern const char* TAG_LOCAL_USERID;
extern const char* TAG_LOCAL_DEVICE_ID;
extern const char* TAG_IDENTICAL_ACCOUNT;
extern const char* TAG_ACCOUNT_GROUPID;
extern const char* TAG_HAVE_CREDENTIAL;
extern const char* TAG_ISONLINE;
extern const char* TAG_AUTHED;
extern const char* TAG_LOCAL_ACCOUNTID;
extern const char* TAG_TOKENID;
extern const char* TAG_HOST_PKGLABEL;
extern const char* TAG_REMOTE_DEVICE_NAME;
extern const char* TAG_HOST;
extern const char* TAG_PROXY_CONTEXT_ID;
extern const char* TAG_IS_NEED_AUTHENTICATE;
extern const char* APP_OPERATION_KEY;
extern const char* TARGET_PKG_NAME_KEY;
extern const char* CUSTOM_DESCRIPTION_KEY;
extern const char* CANCEL_DISPLAY_KEY;
extern const char* BUNDLE_NAME_KEY;

extern const char* AUTHENTICATE_TIMEOUT_TASK;
extern const char* NEGOTIATE_TIMEOUT_TASK;
extern const char* CONFIRM_TIMEOUT_TASK;
extern const char* INPUT_TIMEOUT_TASK;
extern const char* SESSION_HEARTBEAT_TIMEOUT_TASK;
extern const char* WAIT_REQUEST_TIMEOUT_TASK;
extern const char* AUTH_DEVICE_TIMEOUT_TASK;
extern const char* WAIT_PIN_AUTH_TIMEOUT_TASK;
extern const char* WAIT_NEGOTIATE_TIMEOUT_TASK;
extern const char* ADD_TIMEOUT_TASK;
extern const char* WAIT_SESSION_CLOSE_TIMEOUT_TASK;
extern const char* CLOSE_SESSION_TASK_SEPARATOR;
extern const char* TAG_DM_CERT_CHAIN;
extern const char* TAG_CERT_COUNT;
extern const char* TAG_CERT;
extern const char* TAG_IS_COMMON_FLAG;
extern const char* TAG_CERT_RANDOM;

extern const int32_t AUTHENTICATE_TIMEOUT;
extern const int32_t CONFIRM_TIMEOUT;
extern const int32_t NEGOTIATE_TIMEOUT;
extern const int32_t INPUT_TIMEOUT;
extern const int32_t ADD_TIMEOUT;
extern const int32_t WAIT_NEGOTIATE_TIMEOUT;
extern const int32_t WAIT_REQUEST_TIMEOUT;
extern const int32_t CLONE_AUTHENTICATE_TIMEOUT;
extern const int32_t CLONE_CONFIRM_TIMEOUT;
extern const int32_t CLONE_NEGOTIATE_TIMEOUT;
extern const int32_t CLONE_ADD_TIMEOUT;
extern const char* GET_ULTRASONIC_PIN_TIMEOUT_TASK;
extern const int32_t CLONE_WAIT_NEGOTIATE_TIMEOUT;
extern const int32_t CLONE_WAIT_REQUEST_TIMEOUT;
extern const int32_t CLONE_SESSION_HEARTBEAT_TIMEOUT;
extern const int32_t CLONE_PIN_AUTH_TIMEOUT;
extern const int32_t HML_SESSION_TIMEOUT;
extern const int32_t SESSION_HEARTBEAT_TIMEOUT;
extern const int32_t PIN_AUTH_TIMEOUT;
extern const int32_t EVENT_TIMEOUT;

extern const int32_t DM_AUTH_TYPE_MAX;
extern const int32_t DM_AUTH_TYPE_MIN;
extern const int32_t MIN_PIN_TOKEN;
extern const int32_t MAX_PIN_TOKEN;

using CleanNotifyCallback = std::function<void(uint64_t, int32_t)>;

enum DmRole {
    DM_ROLE_UNKNOWN = 0,
    DM_ROLE_USER = 1,
    DM_ROLE_SA,
    DM_ROLE_FA,
};

class AuthManagerBase : public ISoftbusSessionCallback,
                        public ISoftbusConnectorCallback,
                        public IHiChainConnectorCallback,
                        public IDmDeviceAuthCallback,
                        public ISoftbusLeaveLNNCallback {
public:
    virtual int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                                       const std::string &extra);

    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel);

    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &udid,
        int32_t bindLevel, const std::string &extra);

    virtual void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result);

    virtual void OnSessionClosed(const int32_t sessionId);

    virtual void OnSessionDisable();

    virtual void OnDataReceived(const int32_t sessionId, const std::string message);

    virtual void OnAuthDeviceDataReceived(int32_t sessionId, std::string message);

    virtual void OnSoftbusJoinLNNResult(const int32_t sessionId, const char *networkId, int32_t result);

    virtual void OnGroupCreated(int64_t requestId, const std::string &groupId);

    virtual void OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode);

    virtual int32_t EstablishAuthChannel(const std::string &deviceId);

    virtual void StartNegotiate(const int32_t &sessionId);

    virtual void RespNegotiate(const int32_t &sessionId);

    virtual void SendAuthRequest(const int32_t &sessionId);

    virtual int32_t StartAuthProcess(const int32_t &action);

    virtual void StartRespAuthProcess();

    virtual int32_t CreateGroup();

    virtual int32_t ProcessPincode(const std::string &pinCode);

    virtual std::string GetConnectAddr(std::string deviceId);

    virtual int32_t JoinNetwork();

    virtual void AuthenticateFinish();

    virtual bool GetIsCryptoSupport();

    virtual int32_t SetAuthRequestState(std::shared_ptr<AuthRequestState> authRequestState);

    virtual int32_t SetAuthResponseState(std::shared_ptr<AuthResponseState> authResponseState);

    virtual int32_t GetPinCode(std::string &code);

    virtual std::string GenerateGroupName();

    virtual void HandleAuthenticateTimeout(std::string name);

    virtual std::string GeneratePincode();

    virtual void ShowConfigDialog();

    virtual void ShowAuthInfoDialog(bool authDeviceError = false);

    virtual void ShowStartAuthDialog();

    virtual int32_t OnUserOperation(int32_t action, const std::string &params);

    virtual int32_t SetPageId(int32_t pageId);

    virtual int32_t SetReasonAndFinish(int32_t reason, int32_t state);

    virtual bool IsIdenticalAccount();

    virtual int32_t RegisterUiStateCallback(const std::string pkgName);

    virtual int32_t UnRegisterUiStateCallback(const std::string pkgName);

    virtual int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode);

    virtual int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId);

    virtual int32_t RegisterAuthenticationType(int32_t authenticationType);

    virtual int32_t StopAuthenticateDevice(const std::string &pkgName);

    virtual void OnScreenLocked() = 0;

    virtual void HandleDeviceNotTrust(const std::string &udid) = 0;

    virtual int32_t DeleteGroup(const std::string &pkgName, const std::string &deviceId);

    // New interface added in version 5.1.0
    virtual int32_t GetReason();
    // When switching from the new protocol to the old protocol, the previous parameters
    // need to be obtained for use by the old protocol
    virtual void GetBindTargetParams(std::string &pkgName, PeerTargetId &targetId,
        std::map<std::string, std::string> &bindParam);
    virtual void GetAuthCodeAndPkgName(std::string &pkgName, std::string &authCode);

    virtual void SetBindTargetParams(const PeerTargetId &targetId);
    // Register the notification function when the auth_mgr event is complete.
    virtual void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);

    /**
     * @brief Classical authMgr is global, single-frame collabration with double-frame device
     *        need it. If we start bind with single-frame device, we need clear dsoftbus callback
     *        to avoid the classical authMgr affect the single to single logical.
     *        For new protocal, the source and sink authMgr lifecycle obey the bind session, no need
     *        to process it.
     */
    virtual void ClearSoftbusSessionCallback();
    /**
     * @brief Classical authMgr is global, if we use classical logic, we need prepare the callback.
     */
    virtual void PrepareSoftbusSessionCallback();
    virtual void DeleteTimer();
    virtual void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId, int32_t retCode);

    // Public functions
    static std::string ConvertSrcVersion(const std::string &version, const std::string &edition);
    static int32_t DmGetUserId(int32_t displayId);
    static int32_t EndDream();
    bool IsTransferReady();
    void SetTransferReady(bool version);
    void EnableInsensibleSwitching();
    void DisableInsensibleSwitching();
    int32_t ParseAuthType(const std::map<std::string, std::string> &bindParam, int32_t &authType);
    static bool CheckProcessNameInWhiteList(const std::string &processName);
    static bool CheckProcessNameInProxyAdaptationList(const std::string &processName);
protected:
    bool NeedInsensibleSwitching();
    bool isTransferReady_{true};
    /**
    For old protocol, when insensible switching is true, no new session needs to be created,
    and the actual method 80 message is not sent.
    */
    bool insensibleSwitching{false};
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_ADAPTER_V2_H
