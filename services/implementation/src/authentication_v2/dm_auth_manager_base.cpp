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

#include "multiple_user_connector.h"
#include "os_account_manager.h"

#include "dm_constants.h"
#include "dm_error_type.h"
#include "dm_auth_manager_base.h"
#include "power_mgr_client.h"
#if defined(SUPPORT_SCREENLOCK)
#include "screenlock_manager.h"
#endif

#ifdef OS_ACCOUNT_PART_EXISTS
#include "os_account_manager.h"
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_EXISTS

namespace OHOS {
namespace DistributedHardware {
const char* TAG_DMVERSION = "dmVersion";
const char* TAG_DM_VERSION_V2 = "dmVersionV2";
const char* TAG_EDITION = "edition";
const char* TAG_DATA = "data";
const char* TAG_DATA_LEN = "dataLen";
const char* TAG_BUNDLE_NAME = "bundleName";
const char* TAG_BUNDLE_NAME_V2 = "bundleNameV2";
const char* TAG_PEER_BUNDLE_NAME = "PEER_BUNDLE_NAME";
const char* TAG_PEER_BUNDLE_NAME_V2 = "PEER_BUNDLE_NAME_V2";
const char* TAG_PEER_PKG_NAME = "PEER_PKG_NAME";
const char* TAG_BIND_LEVEL = "bindLevel";
const char* TAG_REPLY = "REPLY";
const char* TAG_APP_THUMBNAIL2 = "appThumbnail";    // Naming Add 2 to resolve conflicts with TAG_APP_THUMBNAIL
const char* TAG_AUTH_FINISH = "isFinish";
const char* TAG_LOCAL_USERID = "localUserId";
const char* TAG_LOCAL_DEVICE_ID = "LOCALDEVICEID";
const char* TAG_IDENTICAL_ACCOUNT = "IDENTICALACCOUNT";
const char* TAG_ACCOUNT_GROUPID = "ACCOUNTGROUPID";
const char* TAG_HAVE_CREDENTIAL = "haveCredential";
const char* TAG_ISONLINE = "isOnline";
const char* TAG_AUTHED = "authed";
const char* TAG_LOCAL_ACCOUNTID = "localAccountId";
const char* TAG_TOKENID = "tokenId";
const char* TAG_HOST_PKGLABEL = "hostPkgLabel";
const char* TAG_REMOTE_DEVICE_NAME = "REMOTE_DEVICE_NAME";
const char* TAG_HOST = "HOST";
const char* TAG_PROXY_CONTEXT_ID = "proxyContextId";
const char* TAG_IS_NEED_AUTHENTICATE = "isNeedAuthenticate";
const char* APP_OPERATION_KEY = "appOperation";
const char* TARGET_PKG_NAME_KEY = "targetPkgName";
const char* CUSTOM_DESCRIPTION_KEY = "customDescription";
const char* CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";
const char* BUNDLE_NAME_KEY = "bundleName";

const char* AUTHENTICATE_TIMEOUT_TASK = "deviceManagerTimer:authenticate";
const char* NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:negotiate";
const char* CONFIRM_TIMEOUT_TASK = "deviceManagerTimer:confirm";
const char* INPUT_TIMEOUT_TASK = "deviceManagerTimer:input";
const char* SESSION_HEARTBEAT_TIMEOUT_TASK = "deviceManagerTimer:sessionHeartbeat";
const char* WAIT_REQUEST_TIMEOUT_TASK = "deviceManagerTimer:waitRequest";
const char* AUTH_DEVICE_TIMEOUT_TASK = "deviceManagerTimer:authDevice_";
const char* WAIT_PIN_AUTH_TIMEOUT_TASK = "deviceManagerTimer:waitPinAuth";
const char* WAIT_NEGOTIATE_TIMEOUT_TASK = "deviceManagerTimer:waitNegotiate";
const char* GET_ULTRASONIC_PIN_TIMEOUT_TASK = "deviceManagerTimer:getUltrasonicPin";
const char* ADD_TIMEOUT_TASK = "deviceManagerTimer:add";
const char* WAIT_SESSION_CLOSE_TIMEOUT_TASK = "deviceManagerTimer:waitSessionClose";
const char* CLOSE_SESSION_TASK_SEPARATOR = "#";
const char* TAG_DM_CERT_CHAIN = "dmCertChain";
const char* TAG_CERT_COUNT = "certCount";
const char* TAG_CERT = "cert";
const char* TAG_IS_COMMON_FLAG = "isCommonFlag";
const char* TAG_CERT_RANDOM = "certRandom";

const int32_t AUTHENTICATE_TIMEOUT = 120;
const int32_t CONFIRM_TIMEOUT = 60;
const int32_t NEGOTIATE_TIMEOUT = 10;
const int32_t INPUT_TIMEOUT = 60;
const int32_t ADD_TIMEOUT = 10;
const int32_t WAIT_NEGOTIATE_TIMEOUT = 10;
const int32_t WAIT_REQUEST_TIMEOUT = 10;
const int32_t CLONE_AUTHENTICATE_TIMEOUT = 20;
const int32_t CLONE_CONFIRM_TIMEOUT = 10;
const int32_t CLONE_NEGOTIATE_TIMEOUT = 10;
const int32_t CLONE_ADD_TIMEOUT = 10;
const int32_t CLONE_WAIT_NEGOTIATE_TIMEOUT = 10;
const int32_t CLONE_WAIT_REQUEST_TIMEOUT = 10;
const int32_t CLONE_SESSION_HEARTBEAT_TIMEOUT = 20;
const int32_t CLONE_PIN_AUTH_TIMEOUT = 10;
const int32_t HML_SESSION_TIMEOUT = 10;
const int32_t SESSION_HEARTBEAT_TIMEOUT = 50;
const int32_t PIN_AUTH_TIMEOUT = 60;
const int32_t EVENT_TIMEOUT = 5000; // 5000 ms

constexpr uint16_t OPEN_PROCESS_NAME_WHITE_LIST_NUM = 1;
constexpr uint16_t CLOSE_PROCESS_NAME_WHITE_LIST_NUM = 4;
constexpr const static char* OPEN_PROCESS_NAME_WHITE_LIST[OPEN_PROCESS_NAME_WHITE_LIST_NUM] = {
    "com.example.myapplication"
};
constexpr const static char* CLOSE_PROCESS_NAME_WHITE_LIST[CLOSE_PROCESS_NAME_WHITE_LIST_NUM] = {
    "CollaborationFwk",
    "gameservice_server",
    "wear_link_service",
    "watch_system_service"
};

constexpr const static char* PROCESS_NAME_PROXY_ADAPTATION_LIST[] = {
    "gameservice_server"
};
constexpr uint32_t PROCESS_NAME_PROXY_ADAPTATION_LIST_NUM = std::size(PROCESS_NAME_PROXY_ADAPTATION_LIST);

int32_t AuthManagerBase::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGE("AuthenticateDevice is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel)
{
    LOGE("UnAuthenticateDevice is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    LOGE("UnBindDevice is not implemented in the current version");
    return ERR_DM_FAILED;
}

void AuthManagerBase::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGE("OnSessionOpened is not implemented in the current version");
}

void AuthManagerBase::OnSessionClosed(const int32_t sessionId)
{
    LOGE("OnSessionClosed is not implemented in the current version");
}

void AuthManagerBase::OnSessionDisable()
{
    LOGE("OnSessionDisable is not implemented in the current version");
}

void AuthManagerBase::OnDataReceived(const int32_t sessionId, const std::string message)
{
    LOGE("OnDataReceived is not implemented in the current version");
}

void AuthManagerBase::OnSoftbusJoinLNNResult(const int32_t sessionId, const char *networkId, int32_t result)
{
    LOGE("OnSoftbusJoinLNNResult is not implemented in the current version");
}

void AuthManagerBase::OnGroupCreated(int64_t requestId, const std::string &groupId)
{
    LOGE("OnGroupCreated is not implemented in the current version");
}

void AuthManagerBase::OnMemberJoin(int64_t requestId, int32_t status, int32_t operationCode)
{
    LOGE("OnMemberJoin is not implemented in the current version");
}

int32_t AuthManagerBase::EstablishAuthChannel(const std::string &deviceId)
{
    LOGE("EstablishAuthChannel is not implemented in the current version");
    return ERR_DM_FAILED;
}

void AuthManagerBase::StartNegotiate(const int32_t &sessionId)
{
    LOGE("StartNegotiate is not implemented in the current version");
}

void AuthManagerBase::RespNegotiate(const int32_t &sessionId)
{
    LOGE("RespNegotiate is not implemented in the current version");
}

void AuthManagerBase::SendAuthRequest(const int32_t &sessionId)
{
    LOGE("SendAuthRequest is not implemented in the current version");
}

int32_t AuthManagerBase::StartAuthProcess(const int32_t &action)
{
    LOGE("StartAuthProcess is not implemented in the current version");
    return ERR_DM_FAILED;
}

void AuthManagerBase::StartRespAuthProcess()
{
    LOGE("StartRespAuthProcess is not implemented in the current version");
}

int32_t AuthManagerBase::CreateGroup()
{
    LOGE("CreateGroup is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::ProcessPincode(const std::string &pinCode)
{
    LOGE("ProcessPincode is not implemented in the current version");
    return ERR_DM_FAILED;
}

std::string AuthManagerBase::GetConnectAddr(std::string deviceId)
{
    LOGE("GetConnectAddr is not implemented in the current version");
    return "";
}

int32_t AuthManagerBase::JoinNetwork()
{
    LOGE("JoinNetwork is not implemented in the current version");
    return ERR_DM_FAILED;
}

void AuthManagerBase::AuthenticateFinish()
{
    LOGE("AuthenticateFinish is not implemented in the current version");
}

bool AuthManagerBase::GetIsCryptoSupport()
{
    LOGE("GetIsCryptoSupport is not implemented in the current version");
    return false;
}

int32_t AuthManagerBase::SetAuthRequestState(std::shared_ptr<AuthRequestState> authRequestState)
{
    LOGE("SetAuthRequestState is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::SetAuthResponseState(std::shared_ptr<AuthResponseState> authResponseState)
{
    LOGE("SetAuthResponseState is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::GetPinCode(std::string &code)
{
    LOGE("GetPinCode is not implemented in the current version");
    return ERR_DM_FAILED;
}

std::string AuthManagerBase::GenerateGroupName()
{
    LOGE("GenerateGroupName is not implemented in the current version");
    return "";
}

void AuthManagerBase::HandleAuthenticateTimeout(std::string name)
{
    LOGE("HandleAuthenticateTimeout is not implemented in the current version");
}

std::string AuthManagerBase::GeneratePincode()
{
    LOGE("GeneratePincode is not implemented in the current version");
    return "";
}

void AuthManagerBase::ShowConfigDialog()
{
    LOGE("ShowConfigDialog is not implemented in the current version");
}

void AuthManagerBase::ShowAuthInfoDialog(bool authDeviceError)
{
    LOGE("ShowAuthInfoDialog is not implemented in the current version");
}

void AuthManagerBase::ShowStartAuthDialog()
{
    LOGE("ShowStartAuthDialog is not implemented in the current version");
}

int32_t AuthManagerBase::OnUserOperation(int32_t action, const std::string &params)
{
    LOGE("OnUserOperation is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::SetPageId(int32_t pageId)
{
    LOGE("SetPageId is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::SetReasonAndFinish(int32_t reason, int32_t state)
{
    LOGE("SetReasonAndFinish is not implemented in the current version");
    return ERR_DM_FAILED;
}

bool AuthManagerBase::IsIdenticalAccount()
{
    LOGE("IsIdenticalAccount is not implemented in the current version");
    return false;
}

int32_t AuthManagerBase::RegisterUiStateCallback(const std::string pkgName)
{
    LOGE("RegisterUiStateCallback is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::UnRegisterUiStateCallback(const std::string pkgName)
{
    LOGE("UnRegisterUiStateCallback is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    LOGE("ImportAuthCode is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId)
{
    LOGE("BindTarget is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::RegisterAuthenticationType(int32_t authenticationType)
{
    LOGE("RegisterAuthenticationType is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::StopAuthenticateDevice(const std::string &pkgName)
{
    LOGE("StopAuthenticateDevice is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::DeleteGroup(const std::string &pkgName, const std::string &deviceId)
{
    LOGE("DeleteGroup is not implemented in the current version");
    return ERR_DM_FAILED;
}

int32_t AuthManagerBase::GetReason()
{
    LOGE("GetReason is not implemented in the current version");
    return ERR_DM_FAILED;
}

void AuthManagerBase::GetBindTargetParams(std::string &pkgName, PeerTargetId &targetId,
    std::map<std::string, std::string> &bindParam)
{
    LOGE("GetBindTargetParams is not implemented in the current version");
    return;
}

void AuthManagerBase::GetAuthCodeAndPkgName(std::string &pkgName, std::string &authCode)
{
    LOGE("GetAuthCodeAndPkgName is not implemented in the current version");
    return;
}

void AuthManagerBase::SetBindTargetParams(const PeerTargetId &targetId)
{
    LOGE("SetBindTargetParams is not implemented in the current version");
    return;
}

void AuthManagerBase::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    LOGE("RegisterCleanNotifyCallback is not implemented in the current version");
    return;
}

std::string AuthManagerBase::ConvertSrcVersion(const std::string &version, const std::string &edition)
{
    std::string srcVersion = "";
    if (version == "" && edition != "") {
        srcVersion = edition;
    } else if (version == "" && edition == "") {
        srcVersion = DM_VERSION_5_1_0;
    } else if (version != "" && edition == "") {
        srcVersion = version;
    }
    LOGI("ConvertSrcVersion version %{public}s, edition %{public}s, srcVersion is %{public}s.",
        version.c_str(), edition.c_str(), srcVersion.c_str());
    return srcVersion;
}

// Scenario 1: The remote side specifies userId -> Verify if it is a front-end user
// Scenario 2: The remote side does not specify userId
// Scenario 2.1: Single user -> Use the current unique front-end user
// Scenario 2.2: Multiple users -> Use the current main screen user
int32_t AuthManagerBase::DmGetUserId(int32_t displayId)
{
    int32_t ret;
    int32_t userId = -1;

    std::vector<int32_t> userIds;
    ret = MultipleUserConnector::GetForegroundUserIds(userIds);
    if (ret != DM_OK) {
        LOGE("RespQueryTokenId: GetForegroundUserIds failed, ret: %{public}d", ret);
        return -1;
    }
    // Scenario 1: The remote side specifies userId -> Verify if it is a front-end user
    // Scenario 2: The remote side does not specify userId
    // Scenario 2.1: Single user -> Use the current unique front-end user
    // Scenario 2.2: Multiple users -> Use the current main screen user
    if (userIds.size() == 0) {
        LOGE("RespQueryTokenId: GetForegroundUserIds no foreground users");
        return -1;
    }

    if (displayId != -1) {
        ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(displayId, userId);
        if (ret != DM_OK) {
            LOGE("RespQueryTokenId: fail to get userId by displayId %{public}d", displayId);
            return -1;
        }
        return userId;
    }
    if (userIds.size() == 1) {
        return userIds[0];
    } else {
        // If userIds.size() > 1, we need to find the main screen user
#ifdef OS_ACCOUNT_PART_EXISTS
    ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (ret != DM_OK) {
        LOGE("AuthManagerBase::DmGetUserId: get foreground user failed in multi users with error %{public}d", ret);
        return -1;
    }
    return userId;
#else
    LOGE("AuthManagerBase::DmGetUserId: get foreground user failed because no OsAcccountManager");
    return -1;
#endif
    }
}

bool AuthManagerBase::IsTransferReady()
{
    return isTransferReady_;
}

void AuthManagerBase::SetTransferReady(bool version)
{
    isTransferReady_ = version;
}

void AuthManagerBase::ClearSoftbusSessionCallback()
{}

void AuthManagerBase::PrepareSoftbusSessionCallback()
{}

void AuthManagerBase::EnableInsensibleSwitching()
{
    insensibleSwitching = true;
}

void AuthManagerBase::DisableInsensibleSwitching()
{
    insensibleSwitching = false;
}

bool AuthManagerBase::NeedInsensibleSwitching()
{
    return insensibleSwitching;
}

int32_t AuthManagerBase::ParseAuthType(const std::map<std::string, std::string> &bindParam, int32_t &authType)
{
    auto iter = bindParam.find(PARAM_KEY_AUTH_TYPE);
    if (iter == bindParam.end()) {
        LOGE("AuthManagerBase::ParseAuthType bind param key: %{public}s not exist.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string authTypeStr = iter->second;
    if (authTypeStr.empty()) {
        LOGE("AuthManagerBase::ParseAuthType bind param %{public}s is empty.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (authTypeStr.length() > 1) {
        LOGE("AuthManagerBase::ParseAuthType bind param %{public}s length is unsupported.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!isdigit(authTypeStr[0])) {
        LOGE("AuthManagerBase::ParseAuthType bind param %{public}s fromat is unsupported.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    authType = std::atoi(authTypeStr.c_str());
    return DM_OK;
}

void AuthManagerBase::OnAuthDeviceDataReceived(int32_t sessionId, std::string message)
{
    LOGE("OnAuthDeviceDataReceived is not used in the new protocol");
}

int32_t AuthManagerBase::EndDream()
{
    auto &powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    if (!powerMgrClient.IsScreenOn()) {
        LOGW("screen not on");
        return ERR_DM_FAILED;
    }
#if defined(SUPPORT_SCREENLOCK)
    CHECK_NULL_RETURN(OHOS::ScreenLock::ScreenLockManager::GetInstance(), ERR_DM_FAILED);
    if (!OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked()) {
        LOGI("screen not locked");
        return DM_OK;
    }
#endif
    PowerMgr::PowerErrors ret =
        powerMgrClient.WakeupDevice(PowerMgr::WakeupDeviceType::WAKEUP_DEVICE_END_DREAM, "end_dream");
    if (ret != OHOS::PowerMgr::PowerErrors::ERR_OK) {
        LOGE("fail to end dream, err:%{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("end dream success");
    return DM_OK;
}

bool AuthManagerBase::CheckProcessNameInWhiteList(const std::string &processName)
{
    LOGI("AuthManagerBase::CheckProcessNameInWhiteList start");
    if (processName.empty()) {
        LOGE("processName is empty");
        return false;
    }
    uint16_t index = 0;
#ifdef DEVICE_MANAGER_COMMON_FLAG
    for (; index < OPEN_PROCESS_NAME_WHITE_LIST_NUM; ++index) {
        std::string whitePkgName(OPEN_PROCESS_NAME_WHITE_LIST[index]);
        if (processName == whitePkgName) {
            LOGI("processName = %{public}s in whiteList.", processName.c_str());
            return true;
        }
    }
#else
    for (; index < CLOSE_PROCESS_NAME_WHITE_LIST_NUM; ++index) {
        std::string whitePkgName(CLOSE_PROCESS_NAME_WHITE_LIST[index]);
        if (processName == whitePkgName) {
            LOGI("processName = %{public}s in whiteList.", processName.c_str());
            return true;
        }
    }
#endif
    LOGI("CheckProcessNameInWhiteList: %{public}s invalid.", processName.c_str());
    return false;
}

bool AuthManagerBase::CheckProcessNameInProxyAdaptationList(const std::string &processName)
{
    LOGI("start");
    if (processName.empty()) {
        LOGE("processName is empty");
        return false;
    }
    uint32_t index = 0;
    for (; index < PROCESS_NAME_PROXY_ADAPTATION_LIST_NUM; ++index) {
        std::string whitePkgName(PROCESS_NAME_PROXY_ADAPTATION_LIST[index]);
        if (processName == whitePkgName) {
            LOGI("processName = %{public}s in adaptation List.", processName.c_str());
            return true;
        }
    }
    LOGI("processName = %{public}s not in adaptation List.", processName.c_str());
    return false;
}

void AuthManagerBase::DeleteTimer()
{}

void AuthManagerBase::OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId, int32_t retCode)
{}
}  // namespace DistributedHardware
}  // namespace OHOS