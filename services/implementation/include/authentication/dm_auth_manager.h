/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_MANAGER_H
#define OHOS_DM_AUTH_MANAGER_H

#include <map>
#include <string>

#include "auth_request_state.h"
#include "auth_response_state.h"
#include "authentication.h"
#include "idevice_manager_service_listener.h"
#include "dm_ability_manager.h"
#include "dm_adapter_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_timer.h"
#include "hichain_connector.h"
#include "softbus_connector.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {
typedef enum AuthState {
    AUTH_REQUEST_INIT = 1,
    AUTH_REQUEST_NEGOTIATE,
    AUTH_REQUEST_NEGOTIATE_DONE,
    AUTH_REQUEST_REPLY,
    AUTH_REQUEST_JOIN,
    AUTH_REQUEST_NETWORK,
    AUTH_REQUEST_FINISH,
    AUTH_RESPONSE_INIT = 20,
    AUTH_RESPONSE_NEGOTIATE,
    AUTH_RESPONSE_CONFIRM,
    AUTH_RESPONSE_GROUP,
    AUTH_RESPONSE_SHOW,
    AUTH_RESPONSE_FINISH,
} AuthState;

enum DmMsgType : int32_t {
    MSG_TYPE_UNKNOWN = 0,
    MSG_TYPE_NEGOTIATE = 80,
    MSG_TYPE_RESP_NEGOTIATE = 90,
    MSG_TYPE_REQ_AUTH = 100,
    MSG_TYPE_INVITE_AUTH_INFO = 102,
    MSG_TYPE_REQ_AUTH_TERMINATE = 104,
    MSG_TYPE_RESP_AUTH = 200,
    MSG_TYPE_JOIN_AUTH_INFO = 201,
    MSG_TYPE_RESP_AUTH_TERMINATE = 205,
    MSG_TYPE_CHANNEL_CLOSED = 300,
    MSG_TYPE_SYNC_GROUP = 400,
    MSG_TYPE_AUTH_BY_PIN = 500,
};

typedef struct DmAuthRequestContext {
    int32_t authType;
    std::string localDeviceId;
    std::string deviceId;
    std::string deviceName;
    std::string deviceTypeId;
    int32_t sessionId;
    int32_t groupVisibility;
    bool cryptoSupport;
    std::string cryptoName;
    std::string cryptoVer;
    std::string hostPkgName;
    std::string targetPkgName;
    std::string appName;
    std::string appDesc;
    std::string appIcon;
    std::string appThumbnail;
    std::string token;
    int32_t reason;
    std::vector<std::string> syncGroupList;
} DmAuthRequestContext;

typedef struct DmAuthResponseContext {
    int32_t authType;
    std::string deviceId;
    std::string localDeviceId;
    int32_t msgType;
    int32_t sessionId;
    bool cryptoSupport;
    bool isIdenticalAccount;
    std::string cryptoName;
    std::string cryptoVer;
    int32_t reply;
    std::string networkId;
    std::string groupId;
    std::string groupName;
    std::string hostPkgName;
    std::string targetPkgName;
    std::string appName;
    std::string appDesc;
    std::string appIcon;
    std::string appThumbnail;
    std::string token;
    std::string authToken;
    int32_t pageId;
    int64_t requestId;
    int32_t code;
    int32_t state;
    std::vector<std::string> syncGroupList;
} DmAuthResponseContext;

class AuthMessageProcessor;

class DmAuthManager final : public ISoftbusSessionCallback,
                            public IHiChainConnectorCallback,
                            public std::enable_shared_from_this<DmAuthManager> {
public:
    DmAuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                  std::shared_ptr<IDeviceManagerServiceListener> listener,
                  std::shared_ptr<HiChainConnector> hiChainConnector_);
    ~DmAuthManager();

    /**
     * @tc.name: DmAuthManager::AuthenticateDevice
     * @tc.desc: Authenticate Device of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                               const std::string &extra);

    /**
     * @tc.name: DmAuthManager::UnAuthenticateDevice
     * @tc.desc: UnAuthenticate Device of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId);

    /**
     * @tc.name: DmAuthManager::VerifyAuthentication
     * @tc.desc: Verify Authentication of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t VerifyAuthentication(const std::string &authParam);

    /**
     * @tc.name: DmAuthManager::OnSessionOpened
     * @tc.desc: Opened Session of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result);

    /**
     * @tc.name: DmAuthManager::OnSessionClosed
     * @tc.desc: Closed Session of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void OnSessionClosed(const int32_t sessionId);

    /**
     * @tc.name: DmAuthManager::OnDataReceived
     * @tc.desc: Received Data of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void OnDataReceived(const int32_t sessionId, const std::string message);

    /**
     * @tc.name: DmAuthManager::OnGroupCreated
     * @tc.desc: Created Group of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void OnGroupCreated(int64_t requestId, const std::string &groupId);

    /**
     * @tc.name: DmAuthManager::OnMemberJoin
     * @tc.desc: Join Member of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void OnMemberJoin(int64_t requestId, int32_t status);

    /**
     * @tc.name: DmAuthManager::EstablishAuthChannel
     * @tc.desc: Establish Auth Channel of the DeviceManager Authenticate Manager, auth state machine
     * @tc.type: FUNC
     */
    int32_t EstablishAuthChannel(const std::string &deviceId);

    /**
     * @tc.name: DmAuthManager::StartNegotiate
     * @tc.desc: Start Negotiate of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void StartNegotiate(const int32_t &sessionId);

    /**
     * @tc.name: DmAuthManager::RespNegotiate
     * @tc.desc: Resp Negotiate of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void RespNegotiate(const int32_t &sessionId);

    /**
     * @tc.name: DmAuthManager::SendAuthRequest
     * @tc.desc: Send Auth Request of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void SendAuthRequest(const int32_t &sessionId);

    /**
     * @tc.name: DmAuthManager::StartAuthProcess
     * @tc.desc: Start Auth Process of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t StartAuthProcess(const int32_t &authType);

    /**
     * @tc.name: DmAuthManager::StartRespAuthProcess
     * @tc.desc: Start Resp Auth Process of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void StartRespAuthProcess();

    /**
     * @tc.name: DmAuthManager::CreateGroup
     * @tc.desc: Create Group of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t CreateGroup();

    /**
     * @tc.name: DmAuthManager::AddMember
     * @tc.desc: Add Member of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t AddMember(int32_t pinCode);

    /**
     * @tc.name: DmAuthManager::GetConnectAddr
     * @tc.desc: Get Connect Addr of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    std::string GetConnectAddr(std::string deviceId);

    /**
     * @tc.name: DmAuthManager::JoinNetwork
     * @tc.desc: Join Net work of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t JoinNetwork();

    /**
     * @tc.name: DmAuthManager::AuthenticateFinish
     * @tc.desc: Finish Authenticate of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void AuthenticateFinish();

    /**
     * @tc.name: DmAuthManager::GetIsCryptoSupport
     * @tc.desc: Get Cryp to Support of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    bool GetIsCryptoSupport();

    /**
     * @tc.name: DmAuthManager::SetAuthRequestState
     * @tc.desc: Set Auth Request State of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t SetAuthRequestState(std::shared_ptr<AuthRequestState> authRequestState);

    /**
     * @tc.name: DmAuthManager::SetAuthResponseState
     * @tc.desc: Set Auth Response State of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t SetAuthResponseState(std::shared_ptr<AuthResponseState> authResponseState);

    /**
     * @tc.name: DmAuthManager::GetPinCode
     * @tc.desc: Get Pin Code of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t GetPinCode();

    /**
     * @tc.name: DmAuthManager::GenerateGroupName
     * @tc.desc: Generate Group Name of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    std::string GenerateGroupName();

    /**
     * @tc.name: DmAuthManager::HandleAuthenticateTimeout
     * @tc.desc: Handle Authenticate Timeout of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void HandleAuthenticateTimeout(std::string name);

    /**
     * @tc.name: DmAuthManager::CancelDisplay
     * @tc.desc: Cancel Display of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void CancelDisplay();

    /**
     * @tc.name: DmAuthManager::UpdateInputDialogDisplay
     * @tc.desc: Update InputDialog Display of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void UpdateInputDialogDisplay(bool isShow);

    /**
     * @tc.name: DmAuthManager::GeneratePincode
     * @tc.desc: Generate Pincode of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t GeneratePincode();

    /**
     * @tc.name: DmAuthManager::ShowConfigDialog
     * @tc.desc: Show Config Dialog of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void ShowConfigDialog();

    /**
     * @tc.name: DmAuthManager::ShowAuthInfoDialog
     * @tc.desc: Show AuthInfo Dialog of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void ShowAuthInfoDialog();

    /**
     * @tc.name: DmAuthManager::ShowStartAuthDialog
     * @tc.desc: Show Start Auth Dialog of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void ShowStartAuthDialog();

    /**
     * @tc.name: DmAuthManager::GetAuthenticationParam
     * @tc.desc: Get Authentication Param of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t GetAuthenticationParam(DmAuthParam &authParam);

    /**
     * @tc.name: DmAuthManager::OnUserOperation
     * @tc.desc: User Operation of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t OnUserOperation(int32_t action, const std::string &params);

    /**
     * @tc.name: DmAuthManager::UserSwitchEventCallback
     * @tc.desc: User Switch Event Callback of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void UserSwitchEventCallback(int32_t userId);

    /**
     * @tc.name: DmAuthManager::SetPageId
     * @tc.desc: Set PageId of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t SetPageId(int32_t pageId);

    /**
     * @tc.name: DmAuthManager::SetReasonAndFinish
     * @tc.desc: Set Reason of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t SetReasonAndFinish(int32_t reason, int32_t state);

    /**
     * @tc.name: DmAuthManager::IsIdenticalAccount
     * @tc.desc: judge IdenticalAccount or not
     * @tc.type: FUNC
     */
    bool IsIdenticalAccount();
private:
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::shared_ptr<DmAdapterManager> adapterMgr_;
    std::map<int32_t, std::shared_ptr<IAuthentication>> authenticationMap_;
    std::shared_ptr<AuthRequestState> authRequestState_ = nullptr;
    std::shared_ptr<AuthResponseState> authResponseState_ = nullptr;
    std::shared_ptr<DmAuthRequestContext> authRequestContext_;
    std::shared_ptr<DmAuthResponseContext> authResponseContext_;
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor_;
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<DmAbilityManager> dmAbilityMgr_;
    bool isCryptoSupport_ = false;
    bool isFinishOfLocal_ = true;
    int32_t authTimes_ = 0;
    std::shared_ptr<IAuthentication> authPtr_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MANAGER_H
