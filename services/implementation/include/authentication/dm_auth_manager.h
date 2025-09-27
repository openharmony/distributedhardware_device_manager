/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#define TYPE_TV_ID 0x9C

#include <map>
#include <string>

#include "auth_request_state.h"
#include "auth_response_state.h"
#include "auth_ui_state_manager.h"
#include "authentication.h"
#include "idevice_manager_service_listener.h"
#include "deviceprofile_connector.h"
#include "dm_ability_manager.h"
#include "dm_adapter_manager.h"
#include "dm_auth_manager_base.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_timer.h"
#include "ffrt.h"
#include "hichain_auth_connector.h"
#include "hichain_connector.h"
#include "service_info_unique_key.h"
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
    AUTH_REQUEST_CREDENTIAL,
    AUTH_REQUEST_CREDENTIAL_DONE,
    AUTH_REQUEST_AUTH_FINISH,
    AUTH_REQUEST_RECHECK_MSG,
    AUTH_REQUEST_RECHECK_MSG_DONE,

    AUTH_RESPONSE_INIT = 20,
    AUTH_RESPONSE_NEGOTIATE,
    AUTH_RESPONSE_CONFIRM,
    AUTH_RESPONSE_GROUP,
    AUTH_RESPONSE_SHOW,
    AUTH_RESPONSE_FINISH,
    AUTH_RESPONSE_CREDENTIAL,
    AUTH_RESPONSE_AUTH_FINISH,
    AUTH_RESPONSE_RECHECK_MSG,
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

    MSG_TYPE_RESP_AUTH_EXT,
    MSG_TYPE_REQ_PUBLICKEY,
    MSG_TYPE_RESP_PUBLICKEY,
    MSG_TYPE_REQ_RECHECK_MSG,
    MSG_TYPE_RESP_RECHECK_MSG,
    MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE = 600,
    MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE = 700,
};

enum DmAuthType : int32_t {
    AUTH_TYPE_CRE = 0,
    AUTH_TYPE_PIN,
    AUTH_TYPE_QR_CODE,
    AUTH_TYPE_NFC,
    AUTH_TYPE_NO_INTER_ACTION,
    AUTH_TYPE_IMPORT_AUTH_CODE,
    AUTH_TYPE_UNKNOW,
};

typedef struct DmAuthRequestContext {
    int32_t authType;
    std::string localDeviceId;
    std::string localDeviceName;
    int32_t localDeviceTypeId;
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
    std::string bundleName;
    std::string peerBundleName;
    std::string appOperation;
    std::string appDesc;
    std::string appName;
    std::string customDesc;
    std::string appThumbnail;
    std::string token;
    int32_t reason;
    std::vector<std::string> syncGroupList;
    std::string dmVersion;
    int32_t localUserId;
    std::string localAccountId;
    std::vector<int32_t> bindType;
    bool isOnline;
    bool authed;
    int32_t bindLevel;
    int64_t tokenId;
    std::string remoteAccountId;
    int32_t remoteUserId;
    std::string addr;
    std::string hostPkgLabel;
    int32_t closeSessionDelaySeconds = 0;
    std::string remoteDeviceName;
    std::string connSessionType;
    int32_t hmlActionId = 0;
    bool hmlEnable160M = false;
} DmAuthRequestContext;

typedef struct DmAuthResponseContext {
    int32_t authType;
    std::string deviceId;
    std::string localDeviceId;
    std::string deviceName;
    int32_t deviceTypeId;
    int32_t msgType;
    int32_t sessionId;
    bool cryptoSupport;
    bool isIdenticalAccount;
    bool isAuthCodeReady;
    bool isShowDialog;
    std::string cryptoName;
    std::string cryptoVer;
    int32_t reply;
    std::string networkId;
    std::string groupId;
    std::string groupName;
    std::string hostPkgName;
    std::string targetPkgName;
    std::string bundleName;
    std::string peerBundleName;
    std::string appOperation;
    std::string appDesc;
    std::string customDesc;
    std::string appIcon;
    std::string appThumbnail;
    std::string token;
    std::string authToken;
    int32_t pageId;
    int64_t requestId;
    std::string code = "";
    int32_t state;
    std::vector<std::string> syncGroupList;
    std::string accountGroupIdHash;
    std::string publicKey;
    bool isOnline;
    int32_t bindLevel;
    bool haveCredential;
    int32_t confirmOperation;
    std::string localAccountId;
    int32_t localUserId;
    int64_t tokenId;
    int64_t remoteTokenId;
    bool authed;
    std::string dmVersion;
    std::vector<int32_t> bindType;
    std::string remoteAccountId;
    int32_t remoteUserId;
    std::string targetDeviceName;
    std::string importAuthCode;
    std::string hostPkgLabel;
    bool isFinish = false;
    std::string edition;
    int32_t localBindLevel;
    std::string remoteDeviceName;
    int32_t localSessionKeyId = 0;
    int32_t remoteSessionKeyId = 0;
} DmAuthResponseContext;

class AuthMessageProcessor;

class DmAuthManager final : public AuthManagerBase,
                            public std::enable_shared_from_this<DmAuthManager> {
public:
    DmAuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                  std::shared_ptr<HiChainConnector> hiChainConnector,
                  std::shared_ptr<IDeviceManagerServiceListener> listener,
                  std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
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
    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel);

    /**
     * @brief UnBind device.
     * @param pkgName package name.
     * @param deviceId device id.
     * @return Return 0 if success.
     */
    int32_t UnBindDevice(const std::string &pkgName, const std::string &udid,
        int32_t bindLevel, const std::string &extra);

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
     * @tc.name: DmAuthManager::OnSoftbusJoinLNNResult
     * @tc.desc: OnSoftbus JoinLNN Result of the SoftbusConnector
     * @tc.type: FUNC
     */
    void OnSoftbusJoinLNNResult(const int32_t sessionId, const char *networkId, int32_t result);

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
    void OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode);

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
    int32_t StartAuthProcess(const int32_t &action);

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
     * @tc.name: DmAuthManager::ProcessPincode
     * @tc.desc: Process pin code of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t ProcessPincode(const std::string &pinCode);

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
    int32_t GetPinCode(std::string &code);

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
     * @tc.name: DmAuthManager::GeneratePincode
     * @tc.desc: Generate Pincode of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    std::string GeneratePincode();

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
    void ShowAuthInfoDialog(bool authDeviceError = false);

    /**
     * @tc.name: DmAuthManager::ShowStartAuthDialog
     * @tc.desc: Show Start Auth Dialog of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    void ShowStartAuthDialog();

    /**
     * @tc.name: DmAuthManager::OnUserOperation
     * @tc.desc: User Operation of the DeviceManager Authenticate Manager
     * @tc.type: FUNC
     */
    int32_t OnUserOperation(int32_t action, const std::string &params);

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

    /**
     * @tc.name: DmAuthManager::RegisterUiStateCallback
     * @tc.desc: Register ui state callback
     * @tc.type: FUNC
     */
    int32_t RegisterUiStateCallback(const std::string pkgName);

    /**
     * @tc.name: DmAuthManager::UnRegisterUiStateCallback
     * @tc.desc: Unregister ui state callback
     * @tc.type: FUNC
     */
    int32_t UnRegisterUiStateCallback(const std::string pkgName);

    /**
     * @tc.name: DmAuthManager::ImportAuthCode
     * @tc.desc: Import auth code
     * @tc.type: FUNC
     */
    int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode);

    /**
     * @tc.name: DmAuthManager::BindTarget
     * @tc.desc: Bind Target
     * @tc.type: FUNC
     */
    int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId);

    void ClearSoftbusSessionCallback();
    void PrepareSoftbusSessionCallback();

    void HandleSessionHeartbeat(std::string name);

    int32_t RegisterAuthenticationType(int32_t authenticationType);
    static bool IsPinCodeValid(const std::string strpin);
    static bool IsPinCodeValid(int32_t numpin);
    bool IsImportedAuthCodeValid();
    bool IsSrc();
    void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId, int32_t retCode);

private:
    bool IsHmlSessionType();
    bool CanUsePincodeFromDp();
    bool IsServiceInfoAuthTypeValid(int32_t authType);
    bool IsServiceInfoAuthBoxTypeValid(int32_t authBoxType);
    bool IsServiceInfoPinExchangeTypeValid(int32_t pinExchangeType);
    bool IsLocalServiceInfoValid(const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo);
    void GetLocalServiceInfoInDp();
    bool CheckNeedShowAuthInfoDialog(int32_t errorCode);
    void UpdateInputPincodeDialog(int32_t errorCode);
    void JoinLnn(const std::string &deviceId, bool isForceJoin = false);
    int32_t CheckAuthParamVaild(const std::string &pkgName, int32_t authType, const std::string &deviceId,
        const std::string &extra);
    int32_t CheckAuthParamVaildExtra(const std::string &extra, const std::string &deviceId);
    bool CheckHmlParamValid(JsonObject &jsonObject);
    void ProcessSourceMsg();
    void ProcessSinkMsg();
    std::string GetAccountGroupIdHash();
    void AbilityNegotiate();
    void HandleMemberJoinImportAuthCode(const int64_t requestId, const int32_t status);
    int32_t DeleteAuthCode();
    int32_t GetAuthCode(const std::string &pkgName, std::string &pinCode);
    bool IsAuthTypeSupported(const int32_t &authType);
    bool IsAuthCodeReady(const std::string &pkgName);
    int32_t ParseConnectAddr(const PeerTargetId &targetId, std::string &deviceId, std::string &addrType);
    std::string ParseExtraFromMap(const std::map<std::string, std::string> &bindParam);
    std::string GenerateBindResultContent();
    void InitAuthState(const std::string &pkgName, int32_t authType, const std::string &deviceId,
        const std::string &extra);
    void CompatiblePutAcl();
    void ProcRespNegotiateExt(const int32_t &sessionId);
    void ProcRespNegotiate(const int32_t &sessionId);
    void GetAuthRequestContext();
    void SinkAuthDeviceFinish();
    void SrcAuthDeviceFinish();
    int32_t CheckTrustState();
    void ProcIncompatible(const int32_t &sessionId);
    void MemberJoinAuthRequest(int64_t requestId, int32_t status);
    void PutSrcAccessControlList(DmAccesser &accesser, DmAccessee &accessee, const std::string &localUdid);
    void PutSinkAccessControlList(DmAccesser &accesser, DmAccessee &accessee, const std::string &localUdid);

public:
    void RequestCredential();
    void GenerateCredential(std::string &publicKey);
    void RequestCredentialDone();
    void ResponseCredential();
    bool AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    void AuthDeviceFinish(int64_t requestId);
    void AuthDeviceError(int64_t requestId, int32_t errorCode);
    int32_t GetOutputState(int32_t state);
    void GetRemoteDeviceId(std::string &deviceId);
    void AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen);
    char *AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams);
    int32_t GetSessionKeyIdSync(int64_t requestId);
    void OnAuthDeviceDataReceived(const int32_t sessionId, const std::string message);
    void OnScreenLocked();
    void HandleDeviceNotTrust(const std::string &udid);
    int32_t DeleteGroup(const std::string &pkgName, const std::string &deviceId);
    int32_t DeleteGroup(const std::string &pkgName, int32_t userId, const std::string &deviceId);
    int32_t StopAuthenticateDevice(const std::string &pkgName);
    void RequestReCheckMsg();
    void ResponseReCheckMsg();
    void RequestReCheckMsgDone();
    void CloseAuthSession(const int32_t sessionId);
private:
    int32_t ImportCredential(std::string &deviceId, std::string &publicKey);
    void GetAuthParam(const std::string &pkgName, int32_t authType, const std::string &deviceId,
        const std::string &extra);
    void ParseJsonObject(JsonObject &jsonObject);
    void ParseHmlInfoInJsonObject(JsonObject &jsonObject);
    void PutSessionKeyAsync(int64_t requestId, std::vector<unsigned char> hash);
    int32_t DeleteAcl(const std::string &pkgName, const std::string &localUdid, const std::string &remoteUdid,
        int32_t bindLevel, const std::string &extra);
    void ProcessAuthRequestExt(const int32_t &sessionId);
    bool IsAuthFinish();
    void ProcessAuthRequest(const int32_t &sessionId);
    int32_t ConfirmProcess(const int32_t &action);
    int32_t ConfirmProcessExt(const int32_t &action);
    int32_t AddMember(const std::string &pinCode);
    int32_t AuthDevice(const std::string &pinCode);
    void PutAccessControlList();
    void SinkAuthenticateFinish();
    void SrcAuthenticateFinish();
    bool IsScreenLocked();
    void CheckAndEndTvDream();
    std::string ConvertSinkVersion(const std::string &version);
    void NegotiateRespMsg(const std::string &version);
    void SetAuthType(int32_t authType);
    int32_t GetTaskTimeout(const char* taskName, int32_t taskTimeOut);
    void GetPeerUdidHash(int32_t sessionId, std::string &peerUdidHash);
    void DeleteOffLineTimer(int32_t sessionId);
    int32_t GetBinderInfo();
    void SetProcessInfo();
    int32_t GetCloseSessionDelaySeconds(std::string &delaySecondsStr);
    void ConverToFinish();
    bool IsSinkMsgValid();
    bool IsSourceMsgValid();
    void ProcessReqPublicKey();
    void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
    void GetBindCallerInfo();
    void ProcessReqAuthTerminate();
    void ResetParams();

private:
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<AuthUiStateManager> authUiStateMgr_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::shared_ptr<DmAdapterManager> adapterMgr_;
    std::map<int32_t, std::shared_ptr<IAuthentication>> authenticationMap_;
    std::shared_ptr<AuthRequestState> authRequestState_ = nullptr;
    std::shared_ptr<AuthRequestState> authRequestStateTemp_ = nullptr;
    std::shared_ptr<AuthResponseState> authResponseState_ = nullptr;
    std::shared_ptr<DmAuthRequestContext> authRequestContext_;
    std::shared_ptr<DmAuthResponseContext> authResponseContext_;
    std::shared_ptr<AuthMessageProcessor> authMessageProcessor_;
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<DmAbilityManager> dmAbilityMgr_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    bool isCryptoSupport_ = false;
    bool isFinishOfLocal_ = true;
    int32_t authTimes_ = 0;
    int32_t action_ = USER_OPERATION_TYPE_CANCEL_AUTH;
    std::shared_ptr<IAuthentication> authPtr_;
    bool isAddingMember_ = false;
    std::string importPkgName_ = "";
    std::string importAuthCode_ = "";
    PeerTargetId peerTargetId_;
    std::string remoteDeviceId_ = "";
    std::string dmVersion_ = "";
    bool isAuthDevice_ = false;
    bool isAuthenticateDevice_ = false;
    bool isNeedJoinLnn_ = true;
    int32_t authForm_ = DmAuthForm::ACROSS_ACCOUNT;
    std::string remoteVersion_ = "";
    std::atomic<int32_t> authType_ = AUTH_TYPE_UNKNOW;
    std::string remoteUdidHash_ = "";
    ProcessInfo processInfo_;
    ffrt::mutex srcReqMsgLock_;
    bool isNeedProcCachedSrcReqMsg_ = false;
    std::string srcReqMsg_ = "";
    int32_t authenticationType_ = USER_OPERATION_TYPE_ALLOW_AUTH;
    DistributedDeviceProfile::LocalServiceInfo serviceInfoProfile_;
    bool pincodeDialogEverShown_ = false;
    std::string bundleName_ = "";
    ffrt::mutex sessionKeyIdMutex_;
    ffrt::condition_variable sessionKeyIdCondition_;
    std::map<int64_t, std::optional<int32_t>> sessionKeyIdAsyncResult_;
    bool isWaitingJoinLnnCallback_ = false;
    CleanNotifyCallback cleanNotifyCallback_{nullptr};
    ffrt::mutex bindParamMutex_;
    std::map<std::string, std::string> bindParam_;
    ffrt::mutex groupMutex_;
    bool isAddMember_ = false;
    bool isCreateGroup_ = false;
    bool transitToFinishState_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MANAGER_H
