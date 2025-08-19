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

#include <memory>

#include "app_manager.h"
#include "business_event.h"
#include "distributed_device_profile_client.h"
#include "softbus_common.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "deviceprofile_connector.h"
#include "multiple_user_connector.h"

#include "auth_manager.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_random.h"
#include "dm_log.h"
#include "dm_timer.h"
#include "dm_radar_helper.h"
#include "dm_device_info.h"
#include "dm_anonymous.h"
#include "dm_auth_state_machine.h"
#include "dm_auth_context.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state.h"
#include "json_object.h"

#ifdef SUPPORT_MSDP
#include "spatial_awareness_mgr_client.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {

constexpr int32_t MIN_PIN_CODE = 100000;
constexpr int32_t MAX_PIN_CODE = 999999;
constexpr int32_t DM_ULTRASONIC_FORWARD = 0;
constexpr int32_t DM_ULTRASONIC_REVERSE = 1;
const char* IS_NEED_JOIN_LNN = "IsNeedJoinLnn";
constexpr const char* NEED_JOIN_LNN = "0";
constexpr const char* NO_NEED_JOIN_LNN = "1";
const char* DM_REJECT_KEY = "business_id_cast+_reject_event";
const char* DM_AUTH_DIALOG_REJECT = "is_auth_dialog_reject";
const char* DM_TIMESTAMP = "timestamp";

int32_t GetCloseSessionDelaySeconds(std::string &delaySecondsStr)
{
    if (!IsNumberString(delaySecondsStr)) {
        LOGE("Invalid parameter, param is not number.");
        return 0;
    }
    const int32_t closeSessionDelaySecondsMax = 10;
    int32_t delaySeconds = std::atoi(delaySecondsStr.c_str());
    if (delaySeconds < 0 || delaySeconds > closeSessionDelaySecondsMax) {
        LOGE("Invalid parameter, param out of range.");
        return 0;
    }
    return delaySeconds;
}

std::string GetBundleLabel(const std::string &bundleName)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get ability manager failed");
        return bundleName;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (object == nullptr) {
        LOGE("object is NULL.");
        return bundleName;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bms = iface_cast<OHOS::AppExecFwk::IBundleMgr>(object);
    if (bms == nullptr) {
        LOGE("bundle manager service is NULL.");
        return bundleName;
    }

    auto bundleResourceProxy = bms->GetBundleResourceProxy();
    if (bundleResourceProxy == nullptr) {
        LOGE("GetBundleResourceProxy fail");
        return bundleName;
    }
    AppExecFwk::BundleResourceInfo resourceInfo;
    auto result = bundleResourceProxy->GetBundleResourceInfo(bundleName,
        static_cast<uint32_t>(OHOS::AppExecFwk::ResourceFlag::GET_RESOURCE_INFO_ALL), resourceInfo);
    if (result != ERR_OK) {
        LOGE("GetBundleResourceInfo failed");
        return bundleName;
    }
    LOGI("bundle resource label is %{public}s ", (resourceInfo.label).c_str());
    return resourceInfo.label;
}

std::string ParseExtraFromMap(const std::map<std::string, std::string> &bindParam)
{
    auto iter = bindParam.find(PARAM_KEY_BIND_EXTRA_DATA);
    if (iter != bindParam.end()) {
        return iter->second;
    }
    return ConvertMapToJsonString(bindParam);
}

}  // namespace

bool AuthManager::IsHmlSessionType(const std::string &sessionType)
{
    return sessionType == CONN_SESSION_TYPE_HML;
}

AuthManager::AuthManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                         std::shared_ptr<HiChainConnector> hiChainConnector,
                         std::shared_ptr<IDeviceManagerServiceListener> listener,
                         std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
{
    LOGI("DmAuthManager constructor");
    context_ = std::make_shared<DmAuthContext>();
    context_->softbusConnector = softbusConnector;
    context_->listener = listener;
    context_->hiChainConnector = hiChainConnector;
    context_->hiChainAuthConnector = hiChainAuthConnector;
    context_->authUiStateMgr = std::make_shared<AuthUiStateManager>(context_->listener);
    context_->authenticationMap[AUTH_TYPE_PIN] = nullptr;
    context_->authenticationMap[AUTH_TYPE_IMPORT_AUTH_CODE] = nullptr;
    context_->authenticationMap[AUTH_TYPE_PIN_ULTRASONIC] = nullptr;
    context_->authenticationMap[AUTH_TYPE_NFC] = nullptr;
    context_->authenticationMap[AUTH_TYPE_CRE] = nullptr;
    context_->accesser.dmVersion = DM_CURRENT_VERSION;
    context_->accessee.dmVersion = DM_CURRENT_VERSION;
    context_->timer = std::make_shared<DmTimer>();
    context_->authMessageProcessor = std::make_shared<DmAuthMessageProcessor>();
    context_->businessId = "";
}

AuthManager::~AuthManager()
{
    if (context_ != nullptr) {
        context_->successFinished = true;
        context_->authStateMachine->Stop();  // Stop statemMachine thread

        if (context_->timer != nullptr) {
            context_->timer->DeleteAll();
        }
        LOGI("AuthManager context variables destroy successful.");
    }
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam_.clear();
    }
    LOGI("DmAuthManager destructor");
}

void AuthManager::RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback)
{
    context_->cleanNotifyCallback = cleanNotifyCallback;
    return;
}

void AuthManager::SetAuthContext(std::shared_ptr<DmAuthContext> context)
{
    this->context_ = context;
}

std::shared_ptr<DmAuthContext> AuthManager::GetAuthContext()
{
    return this->context_;
}

std::string AuthManager::GeneratePincode()
{
    LOGI("AuthManager::GeneratePincode start");
    int32_t pinCode = GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
    context_->pinCode = std::to_string(pinCode);
    return context_->pinCode;
}

int32_t AuthManager::RegisterUiStateCallback(const std::string pkgName)
{
    LOGI("AuthManager::RegisterUiStateCallback start");
    if (context_->authUiStateMgr == nullptr) {
        LOGE("AuthManager::RegisterUiStateCallback context_->authUiStateMgr is null.");
        return ERR_DM_FAILED;
    }
    context_->authUiStateMgr->RegisterUiStateCallback(pkgName);
    return DM_OK;
}

int32_t AuthManager::UnRegisterUiStateCallback(const std::string pkgName)
{
    LOGI("AuthManager::UnRegisterUiStateCallback start");
    if (context_->authUiStateMgr == nullptr) {
        LOGE("AuthManager::UnRegisterUiStateCallback context_->authUiStateMgr is null.");
        return ERR_DM_FAILED;
    }
    context_->authUiStateMgr->UnRegisterUiStateCallback(pkgName);
    return DM_OK;
}

int32_t AuthManager::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel)
{
    LOGI("AuthManager::UnAuthenticateDevice start");
    return ERR_DM_FAILED;
}

int32_t AuthManager::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (authCode.empty() || pkgName.empty()) {
        LOGE("ImportAuthCode failed, authCode or pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    context_->importAuthCode = authCode;
    context_->importPkgName = pkgName;
    context_->pinCode = authCode;
    LOGI("AuthManager::ImportAuthCode ok");
    return DM_OK;
}

int32_t AuthManager::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    LOGI("AuthManager::UnBindDevice start");
    return ERR_DM_FAILED;
}

int32_t AuthManager::StopAuthenticateDevice(const std::string &pkgName)
{
    (void)pkgName;
    LOGI("AuthManager::StopAuthenticateDevice start");

    context_->reason = STOP_BIND;
    if (context_->direction == DM_AUTH_SOURCE) {
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    } else {
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    }
    return DM_OK;
}

void AuthManager::OnScreenLocked()
{
    LOGI("AuthManager::OnScreenLocked start");
    if (DmAuthState::IsImportAuthCodeCompatibility(context_->authType)) {
        LOGI("OnScreenLocked authtype is: %{public}d, no need stop bind.", context_->authType);
        return;
    }
    context_->reason = ERR_DM_BIND_USER_CANCEL;
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
}

void AuthManager::HandleDeviceNotTrust(const std::string &udid)
{
    LOGI("AuthManager::HandleDeviceNotTrust start");
}

int32_t AuthManager::RegisterAuthenticationType(int32_t authenticationType)
{
    context_->confirmOperation = static_cast<UiAction>(authenticationType);
    return DM_OK;
}

int32_t AuthManager::GetReason()
{
    return context_->reason;
}

// Save the key
void AuthSrcManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("AuthSrcManager::AuthDeviceSessionKey start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("AuthSrcManager::AuthDeviceSessionKey failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("AuthSrcManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("AuthSrcManager::AuthDeviceSessionKey, save session key error, ret: %{public}d", ret);
    }

    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
    LOGI("AuthSrcManager::AuthDeviceSessionKey leave.");
}

char *AuthSrcManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSrcManager::AuthDeviceRequest start");
    return nullptr;
}

void AuthManager::SetAuthType(int32_t authType)
{
    context_->authType = (DmAuthType)authType;
}

bool AuthManager::IsAuthTypeSupported(const int32_t &authType)
{
    if (context_->authenticationMap.find(authType) == context_->authenticationMap.end()) {
        LOGE("IsAuthTypeSupported failed, authType is not supported.");
        return false;
    }
    return true;
}

bool AuthManager::IsAuthCodeReady(const std::string &pkgName)
{
    if (context_->importAuthCode.empty() || context_->importPkgName.empty()) {
        LOGE("AuthManager::IsAuthCodeReady, auth code not ready with authCode %{public}s and pkgName %{public}s.",
            GetAnonyString(context_->importAuthCode).c_str(), context_->importPkgName.c_str());
        return false;
    }
    if (pkgName != context_->importPkgName) {
        LOGE("AuthManager::IsAuthCodeReady pkgName %{public}s not supported with import pkgName %{public}s.",
            pkgName.c_str(), context_->importPkgName.c_str());
        return false;
    }
    return true;
}

int32_t AuthManager::CheckAuthParamVaild(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("AuthManager::CheckAuthParamVaild start.");
    if (authType < DM_AUTH_TYPE_MIN || authType > DM_AUTH_TYPE_MAX) {
        LOGE("CheckAuthParamVaild failed, authType is illegal.");
        return ERR_DM_AUTH_FAILED;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("AuthManager::CheckAuthParamVaild failed, pkgName is %{public}s, deviceId is %{public}s, extra is"
            "%{public}s.", pkgName.c_str(), GetAnonyString(deviceId).c_str(), extra.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (context_->listener == nullptr || context_->authUiStateMgr == nullptr) {
        LOGE("AuthManager::CheckAuthParamVaild listener or authUiStateMgr is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (!IsAuthTypeSupported(authType)) {
        LOGE("AuthManager::CheckAuthParamVaild authType %{public}d not support.", authType);
        context_->listener->OnAuthResult(context_->processInfo, context_->peerTargetId.deviceId, "",
            STATUS_DM_AUTH_DEFAULT,
            ERR_DM_UNSUPPORTED_AUTH_TYPE);
        context_->listener->OnBindResult(context_->processInfo, context_->peerTargetId,
            ERR_DM_UNSUPPORTED_AUTH_TYPE, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_UNSUPPORTED_AUTH_TYPE;
    }
    JsonObject jsonObject(extra);
    if ((jsonObject.IsDiscarded() || !IsString(jsonObject, PARAM_KEY_CONN_SESSIONTYPE) ||
        jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>() != CONN_SESSION_TYPE_HML) &&
        !context_->softbusConnector->HaveDeviceInMap(deviceId)) {
        LOGE("CheckAuthParamVaild failed, the discoveryDeviceInfoMap_ not have this device.");
        context_->listener->OnAuthResult(context_->processInfo, context_->peerTargetId.deviceId, "",
            STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        context_->listener->OnBindResult(context_->processInfo, context_->peerTargetId,
            ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (DmAuthState::IsImportAuthCodeCompatibility(static_cast<DmAuthType>(authType)) &&
        (!IsAuthCodeReady(pkgName))) {
        LOGE("Auth code not exist.");
        context_->listener->OnAuthResult(context_->processInfo, context_->peerTargetId.deviceId, "",
            STATUS_DM_AUTH_DEFAULT, ERR_DM_INPUT_PARA_INVALID);
        context_->listener->OnBindResult(context_->processInfo, context_->peerTargetId,
            ERR_DM_INPUT_PARA_INVALID, STATUS_DM_AUTH_DEFAULT, "");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

void AuthManager::ParseHmlInfoInJsonObject(const JsonObject &jsonObject)
{
    if (jsonObject[PARAM_KEY_CONN_SESSIONTYPE].IsString()) {
        context_->connSessionType = jsonObject[PARAM_KEY_CONN_SESSIONTYPE].Get<std::string>();
        LOGI("connSessionType %{public}s", context_->connSessionType.c_str());
    }
    if (!IsHmlSessionType(context_->connSessionType)) {
        return;
    }
    if (context_->connDelayCloseTime == 0) {
        context_->connDelayCloseTime = HML_SESSION_TIMEOUT;
    }
    if (jsonObject[PARAM_KEY_HML_ENABLE_160M].IsBoolean()) {
        context_->hmlEnable160M = jsonObject[PARAM_KEY_HML_ENABLE_160M].Get<bool>();
        LOGI("hmlEnable160M %{public}d", context_->hmlEnable160M);
    }
    if (jsonObject[PARAM_KEY_HML_ACTIONID].IsString()) {
        std::string actionIdStr = jsonObject[PARAM_KEY_HML_ACTIONID].Get<std::string>();
        if (IsNumberString(actionIdStr)) {
            context_->hmlActionId = std::atoi(actionIdStr.c_str());
        }
        if (context_->hmlActionId <= 0) {
            context_->hmlActionId = 0;
        }
        LOGI("hmlActionId %{public}d", context_->hmlActionId);
    }

    return;
}

std::string AuthManager::GetBundleName(const JsonObject &jsonObject)
{
    if (!jsonObject.IsDiscarded() && jsonObject[BUNDLE_NAME_KEY].IsString()) {
        return jsonObject[BUNDLE_NAME_KEY].Get<std::string>();
    }
    bool isSystemSA = false;
    std::string bundleName;
    AppManager::GetInstance().GetCallerName(isSystemSA, bundleName);
    return bundleName;
}

void AuthManager::ParseJsonObject(const JsonObject &jsonObject)
{
    if (IsString(jsonObject, DM_BUSINESS_ID)) {
        context_->businessId = jsonObject[DM_BUSINESS_ID].Get<std::string>();
    }
    if (jsonObject[APP_OPERATION_KEY].IsString()) {
        context_->appOperation = jsonObject[APP_OPERATION_KEY].Get<std::string>();
    }
    if (jsonObject[CUSTOM_DESCRIPTION_KEY].IsString()) {
        context_->customData = jsonObject[CUSTOM_DESCRIPTION_KEY].Get<std::string>();
    }
    if (jsonObject[TAG_APP_THUMBNAIL2].IsString()) {
        context_->appThumbnail = jsonObject[TAG_APP_THUMBNAIL2].Get<std::string>();
    }
    context_->connDelayCloseTime = 0;
    if (jsonObject[PARAM_CLOSE_SESSION_DELAY_SECONDS].IsString()) {
        std::string delaySecondsStr = jsonObject[PARAM_CLOSE_SESSION_DELAY_SECONDS].Get<std::string>();
        context_->connDelayCloseTime = GetCloseSessionDelaySeconds(delaySecondsStr);
    }
    context_->accessee.bundleName = context_->accesser.bundleName;
    if (jsonObject[TAG_PEER_BUNDLE_NAME].IsString() && !jsonObject[TAG_PEER_BUNDLE_NAME].Get<std::string>().empty()) {
        context_->accessee.bundleName = jsonObject[TAG_PEER_BUNDLE_NAME].Get<std::string>();
        context_->accessee.oldBundleName = context_->accessee.bundleName;
    } else {
        context_->accessee.oldBundleName = context_->pkgName;
    }
    context_->accesser.pkgName = context_->pkgName;
    context_->accessee.pkgName = context_->accesser.pkgName;
    if (jsonObject[TAG_PEER_PKG_NAME].IsString()) {
        context_->accessee.pkgName = jsonObject[TAG_PEER_PKG_NAME].Get<std::string>();
    }
    if (jsonObject[TAG_PEER_DISPLAY_ID].IsNumberInteger()) {
        context_->accessee.displayId = jsonObject[TAG_PEER_DISPLAY_ID].Get<int32_t>();
    }
    if (jsonObject[TAG_LOCAL_USERID].IsNumberInteger()) {
        context_->accesser.userId = jsonObject[TAG_LOCAL_USERID].Get<int32_t>();
    } else {
        context_->accesser.userId = MultipleUserConnector::GetFirstForegroundUserId();
    }
    if (jsonObject[TAG_IS_NEED_AUTHENTICATE].IsString()) {
        context_->isNeedAuthenticate = std::atoi(jsonObject[TAG_IS_NEED_AUTHENTICATE].Get<std::string>().c_str());
        LOGI("isNeedAuthenticate: %{public}d.", context_->isNeedAuthenticate);
    }
    if (context_->authType == AUTH_TYPE_PIN_ULTRASONIC) {
        ParseUltrasonicSide(jsonObject);
    }
    ParseHmlInfoInJsonObject(jsonObject);
    ParseProxyJsonObject(jsonObject);
    return;
}

void AuthManager::ParseUltrasonicSide(const JsonObject &jsonObject)
{
    if (jsonObject[TAG_ULTRASONIC_SIDE].IsString()) {
        std::string tempInfo = jsonObject[TAG_ULTRASONIC_SIDE].Get<std::string>();
        if (tempInfo.length() > 0 && isdigit(tempInfo[0])) {
            int32_t intTempInfo = std::atoi(tempInfo.c_str());
            if (intTempInfo == DM_ULTRASONIC_REVERSE) {
                context_->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Reverse;
            } else if (intTempInfo == DM_ULTRASONIC_FORWARD) {
                context_->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Forward;
            } else {
                context_->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Invalid;
                return;
            }
        }
    }
    bool isSupport = true;
    if (context_->ultrasonicInfo == DM_Ultrasonic_Forward) {
#ifdef SUPPORT_MSDP
        isSupport = Msdp::SpatialAwarenessMgrClient::GetInstance().IsPinCodeAbilitySupport(
            Msdp::PinCodeMode::MODE_PIN_RECEIVE_CODE);
#endif
    }
    if (context_->ultrasonicInfo == DM_Ultrasonic_Reverse) {
#ifdef SUPPORT_MSDP
        isSupport = Msdp::SpatialAwarenessMgrClient::GetInstance().IsPinCodeAbilitySupport(
            Msdp::PinCodeMode::MODE_PIN_SEND_CODE);
#endif
    }
    if (!isSupport) {
        context_->authType = AUTH_TYPE_PIN;
    }
}

bool CheckBindLevel(const JsonItemObject &jsonObj, const std::string &key, int32_t &bindLevel)
{
    if (IsJsonValIntegerString(jsonObj, TAG_BIND_LEVEL)) {
        bindLevel = std::atoi(jsonObj[TAG_BIND_LEVEL].Get<std::string>().c_str());
        return true;
    }
    if (IsInt32(jsonObj, TAG_BIND_LEVEL)) {
        bindLevel = jsonObj[TAG_BIND_LEVEL].Get<int32_t>();
        return true;
    }
    return false;
}

int32_t AuthManager::GetBindLevel(int32_t bindLevel)
{
    LOGI("start.");
    std::string processName = "";
    bool isSystemSA = false;
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        if (bindParam_.find("bindCallerProcessName") != bindParam_.end()) {
            processName = bindParam_["bindCallerProcessName"];
        }
        if (bindParam_.find("bindCallerIsSystemSA") != bindParam_.end()) {
            isSystemSA = static_cast<bool>(std::atoi(bindParam_["bindCallerIsSystemSA"].c_str()));
        }
    }
    LOGI("processName = %{public}s, isSystemSA %{public}d.", GetAnonyString(processName).c_str(), isSystemSA);
    if (processName != "" && AuthManagerBase::CheckProcessNameInWhiteList(processName)) {
        return USER;
    }
    if (isSystemSA) {
        if (static_cast<uint32_t>(bindLevel) == INVALIED_TYPE || static_cast<uint32_t>(bindLevel) > APP ||
            static_cast<uint32_t>(bindLevel) < USER) {
            return USER;
        }
        return bindLevel;
    }
    if (static_cast<uint32_t>(bindLevel) == INVALIED_TYPE || (static_cast<uint32_t>(bindLevel) != APP &&
        static_cast<uint32_t>(bindLevel) != SERVICE)) {
        return APP;
    }
    return bindLevel;
}

void AuthManager::GetAuthParam(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("Get auth param with pkgName %{public}s and extra %{public}s.", pkgName.c_str(), extra.c_str());
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    context_->accesser.deviceId = std::string(localDeviceId);
    context_->pkgName = pkgName;
    context_->authType = (DmAuthType)authType;
    context_->accesser.deviceName = context_->listener->GetLocalDisplayDeviceName();
    context_->accesser.deviceType = context_->softbusConnector->GetLocalDeviceTypeId();
    context_->accesser.isOnline = false;

    context_->accessee.deviceId = deviceId;
    context_->accessee.addr = deviceId;
    JsonObject jsonObject(extra);
    if (jsonObject.IsDiscarded()) {
        LOGE("extra string not a json type.");
        return;
    }
    ParseJsonObject(jsonObject);
    context_->accesser.accountId = MultipleUserConnector::GetOhosAccountIdByUserId(context_->accesser.userId);

    // compatible for old version
    context_->accesser.oldBindLevel = INVALIED_TYPE;
    CheckBindLevel(jsonObject, TAG_BIND_LEVEL, context_->accesser.oldBindLevel);
    context_->accesser.oldBindLevel = GetBindLevel(context_->accesser.oldBindLevel);
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam_["bindCallerOldBindLevel"] = std::to_string(context_->accesser.oldBindLevel);
    }
    LOGI("bindCallerOldBindLevel %{public}d.", context_->accesser.oldBindLevel);
}

void AuthManager::InitAuthState(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    auto iter = context_->authenticationMap.find(authType);
    if (iter != context_->authenticationMap.end()) {
        context_->authPtr = iter->second;
    }

    context_->timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
        AUTHENTICATE_TIMEOUT,
        [this] (std::string name) {
            DmAuthState::HandleAuthenticateTimeout(context_, name);
        });
    GetAuthParam(pkgName, authType, deviceId, extra);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcStartState>());
    LOGI("AuthManager::AuthenticateDevice complete");

    return;
}

int32_t AuthManager::AuthenticateDevice(const std::string &pkgName, int32_t authType,
    const std::string &deviceId, const std::string &extra)
{
    LOGI("AuthManager::AuthenticateDevice start auth type %{public}d, extra %{public}s.", authType, extra.c_str());
    SetAuthType(authType);
    context_->processInfo.pkgName = pkgName;
    GetBindCallerInfo();
    int32_t ret = CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    if (ret != DM_OK) {
        LOGE("AuthManager::AuthenticateDevice failed, param is invaild.");
        return ret;
    }
    ret = CheckProxyAuthParamVaild(extra);
    if (ret != DM_OK) {
        LOGE("CheckProxyAuthParamVaild failed.");
        return ret;
    }
    context_->isAuthenticateDevice = true;
    if (authType == AUTH_TYPE_CRE) {
        LOGI("AuthManager::AuthenticateDevice for credential type, joinLNN directly.");
        context_->softbusConnector->JoinLnn(deviceId, true);
        context_->listener->OnAuthResult(context_->processInfo, context_->peerTargetId.deviceId,
            "", STATUS_DM_AUTH_DEFAULT, DM_OK);
        context_->listener->OnBindResult(context_->processInfo, context_->peerTargetId,
            DM_OK, STATUS_DM_AUTH_DEFAULT, "");
        return DM_OK;
    }
    InitAuthState(pkgName, authType, deviceId, extra);
    if (context_->ultrasonicInfo == DmUltrasonicInfo::DM_Ultrasonic_Invalid) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}

int32_t AuthManager::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam, int sessionId, uint64_t logicalSessionId)
{
    int ret = DM_OK;
    LOGI("AuthManager::BindTarget start. pkgName: %{public}s", pkgName.c_str());
    for (auto iter = bindParam.begin(); iter != bindParam.end(); iter++) {
        LOGI("AuthManager::BindTarget para: %{public}s : %{public}s ", iter->first.c_str(), iter->second.c_str());
    }

    struct RadarInfo info = {
        .funcName = "AuthenticateDevice",
        .stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC),
        .bizState = static_cast<int32_t>(BizState::BIZ_STATE_END),
    };
    if (!DmRadarHelper::GetInstance().ReportDiscoverUserRes(info)) {
        LOGE("ReportDiscoverUserRes failed");
    }
    GetIsNeedJoinLnnParam(bindParam);
    if (pkgName.empty()) {
        LOGE("AuthManager::BindTarget failed, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t authType = -1;
    if (ParseAuthType(bindParam, authType) != DM_OK) {
        LOGE("AuthManager::BindTarget failed, key: %{public}s error.", PARAM_KEY_AUTH_TYPE);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    context_->peerTargetId = targetId_;
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam_ = bindParam;
    }
    if (!targetId.deviceId.empty()) {
        ret = AuthenticateDevice(pkgName, authType, targetId.deviceId, ParseExtraFromMap(bindParam));
        if (ret != DM_OK) { return ret; }
    } else {
        LOGE("AuthManager::BindTarget failed, targetId is error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    context_->sessionId = sessionId;
    context_->logicalSessionId = logicalSessionId;
    context_->requestId = static_cast<int64_t>(logicalSessionId);
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcNegotiateStateMachine>());
    info = { .funcName = "BindTarget" };
    info.channelId = sessionId;
    DmRadarHelper::GetInstance().ReportAuthSendRequest(info);
    return ret;
}

void AuthManager::GetIsNeedJoinLnnParam(const std::map<std::string, std::string> &bindParam)
{
    std::string isNeedJoinLnnStr;
    if (bindParam.find(IS_NEED_JOIN_LNN) != bindParam.end()) {
        isNeedJoinLnnStr = bindParam.at(IS_NEED_JOIN_LNN);
    }
    if (isNeedJoinLnnStr == NEED_JOIN_LNN || isNeedJoinLnnStr == NO_NEED_JOIN_LNN) {
        context_->isNeedJoinLnn = std::atoi(isNeedJoinLnnStr.c_str());
        LOGI("isNeedJoinLnn: %{public}d.", context_->isNeedJoinLnn);
    }
}

AuthSinkManager::AuthSinkManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<HiChainConnector> hiChainConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener,
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : AuthManager(softbusConnector, hiChainConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_SINK;
    context_->authStateMachine = std::make_shared<DmAuthStateMachine>(context_);
}

void AuthSinkManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSinkManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("AuthSrcManager::OnSessionClosed sessionId = %{public}d", sessionId);
    context_->reason = ERR_DM_SESSION_CLOSED;
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
}

void AuthSinkManager::OnDataReceived(int32_t sessionId, std::string message)
{
    context_->sessionId = sessionId;
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    }

    return;
}

bool AuthSinkManager::GetIsCryptoSupport()
{
    return false;
}

void AuthManager::GetRemoteDeviceId(std::string &deviceId)
{
    deviceId = (context_->direction == DM_AUTH_SOURCE) ? context_->accessee.deviceId : context_->accesser.deviceId;
    return;
}

int32_t AuthSinkManager::OnUserOperation(int32_t action, const std::string &params)
{
    LOGI("AuthSinkManager::OnUserOperation start.");
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("OnUserOperation: Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }

    std::string businessId = context_->businessId;
    if (!businessId.empty()) {
        LOGI("AuthSinkManager::OnUserOperation found businessId: %{public}s", businessId.c_str());
        int32_t ret = HandleBusinessEvents(businessId, action);
        if (ret != DM_OK) {
            LOGE("AuthSinkManager::OnUserOperation failed to handle business events, ret: %{public}d", ret);
            return ret;
        }
    }

    switch (action) {
        case USER_OPERATION_TYPE_CANCEL_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH:
        case USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS:
            context_->confirmOperation = static_cast<UiAction>(action);
            context_->reply = USER_OPERATION_TYPE_ALLOW_AUTH;
            context_->userOperationParam = params;
            if (action == USER_OPERATION_TYPE_CANCEL_AUTH) {
                LOGI("AuthSinkManager::OnUserOperation USER_OPERATION_TYPE_CANCEL_AUTH.");
                context_->reply = USER_OPERATION_TYPE_CANCEL_AUTH;
            }
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_USER_OPERATION);
            break;
        case USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT:
            LOGI("AuthSinkManager::OnUserOperation USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT.");
            context_->confirmOperation = USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT;
            context_->reason = ERR_DM_PEER_CONFIRM_TIME_OUT;
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            break;
        case USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY:
            LOGI("AuthSinkManager::OnUserOperation USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY.");
            context_->confirmOperation = USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY;
            context_->reason = ERR_DM_BIND_USER_CANCEL_PIN_CODE_DISPLAY;
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    LOGI("AuthSinkManager::OnUserOperation leave.");
    return DM_OK;
}

AuthSrcManager::AuthSrcManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<HiChainConnector> hiChainConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener,
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : AuthManager(softbusConnector, hiChainConnector, listener, hiChainAuthConnector)
{
    context_->direction = DM_AUTH_SOURCE;
    context_->authStateMachine = std::make_shared<DmAuthStateMachine>(context_);
}

void AuthSrcManager::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    LOGI("sessionId = %{public}d and sessionSide = %{public}d result = %{public}d", sessionId, sessionSide, result);
}

void AuthSrcManager::OnSessionClosed(int32_t sessionId)
{
    LOGI("AuthSrcManager::OnSessionClosed sessionId = %{public}d", sessionId);
    context_->reason = ERR_DM_SESSION_CLOSED;
    context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
}

void AuthSrcManager::OnSessionDisable()
{
    context_->sessionId = -1;
}

void AuthSrcManager::OnDataReceived(int32_t sessionId, std::string message)
{
    context_->sessionId = sessionId;
    int32_t ret = context_->authMessageProcessor->ParseMessage(context_, message);
    if (ret != DM_OK) {
        LOGE("OnDataReceived failed, parse input message error.");
        context_->reason = ERR_DM_PARSE_MESSAGE_FAILED;
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    }

    return;
}

bool AuthSrcManager::GetIsCryptoSupport()
{
    return false;
}

int32_t AuthSrcManager::OnUserOperation(int32_t action, const std::string &params)
{
    LOGI("AuthSrcManager::OnUserOperation start.");
    if (context_ == nullptr || context_->authStateMachine == nullptr) {
        LOGE("OnUserOperation: Authenticate is not start");
        return ERR_DM_AUTH_NOT_START;
    }

    switch (action) {
        case USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT:
            LOGE("AuthSrcManager OnUserOperation user cancel");
            context_->pinInputResult = USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT;
            context_->reason = ERR_DM_BIND_USER_CANCEL_ERROR;
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
            break;
        case USER_OPERATION_TYPE_DONE_PINCODE_INPUT:
            LOGE("AuthSrcManager OnUserOperation user input done");
            context_->pinInputResult = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
            {
                if (!IsNumberString(params)) {
                    LOGE("OnUserOperation jsonStr error");
                    return ERR_DM_INPUT_PARA_INVALID;
                }
                context_->pinCode = params;
            }
            context_->authStateMachine->NotifyEventFinish(DmEventType::ON_USER_OPERATION);
            break;
        default:
            LOGE("this action id not support");
            break;
    }
    LOGI("AuthSrcManager::OnUserOperation leave.");
    return DM_OK;
}

void AuthSrcManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSrcManager::AuthDeviceError start.");
    CHECK_NULL_VOID(context_);
    if (requestId != context_->requestId) {
        LOGE("requestId: %{public}" PRId64", context_->requestId: %{public}" PRId64".", requestId, context_->requestId);
        return;
    }
    auto curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE ||
        curState == DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE) {
        LOGI("AuthSrcManager::AuthDeviceError Auth pin err.");
        if (context_->authType == DmAuthType::AUTH_TYPE_PIN) {
            context_->inputPinAuthFailTimes++;
        }
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_ERROR);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
    } else {
        LOGI("AuthSrcManager::AuthDeviceError unexpected err.");
        context_->reason = errorCode;
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    LOGI("AuthSrcManager::AuthDeviceError leave.");
}

void AuthSinkManager::AuthDeviceError(int64_t requestId, int32_t errorCode)
{
    LOGI("AuthSinkManager::AuthDeviceError start.");
    auto curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE) {
        LOGI("AuthSrcManager::AuthDeviceError Auth pin err.");
        if (context_->authType == DmAuthType::AUTH_TYPE_PIN) {
            context_->inputPinAuthFailTimes++;
        }
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_ERROR);
        context_->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
    } else {
        LOGI("AuthSinkManager::AuthDeviceError unexpected err.");
        context_->reason = errorCode;
        context_->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    LOGI("AuthSinkManager::AuthDeviceError leave.");
}

bool AuthSrcManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSrcManager::AuthDeviceTransmit start.");
    // check request id first
    if (requestId != context_->requestId) {
        LOGE("AuthSrcManager::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSrcManager::AuthDeviceTransmit leave.");
    return true;
}

bool AuthSinkManager::AuthDeviceTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("AuthSinkManager::AuthDeviceTransmit start.");
    // check request id first
    if (requestId != context_->requestId) {
        LOGE("AuthSinkManager::onTransmit requestId %{public}" PRId64"is error.", requestId);
        return false;
    }

    context_->transmitData = std::string(reinterpret_cast<const char *>(data), dataLen);
    context_->authStateMachine->NotifyEventFinish(ON_TRANSMIT);
    LOGI("AuthSinkManager::AuthDeviceTransmit leave.");
    return true;
}

void AuthSrcManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSrcManager::AuthDeviceFinish start.");
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    // Perform business processing based on the current state
    DmAuthStateType curState = context_->authStateMachine->GetCurState();
    switch (curState) {
        case DmAuthStateType::AUTH_SRC_PIN_AUTH_DONE_STATE:
            if (!context_->isNeedAuthenticate) {
                LOGI("skip authenticate.");
                context_->reason = ERR_DM_SKIP_AUTHENTICATE;
                context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
            } else {
                // ON_FINISH event occurs, start credential exchange
                context_->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialExchangeState>());
            }
            break;
        default:
            break;
    }
    LOGI("AuthSrcManager::AuthDeviceFinish leave.");
}

void AuthSinkManager::AuthDeviceFinish(int64_t requestId)
{
    LOGI("AuthSinkManager::AuthDeviceFinish start.");
    context_->authStateMachine->NotifyEventFinish(ON_FINISH);
    LOGI("AuthSinkManager::AuthDeviceFinish leave.");
}

void AuthSinkManager::AuthDeviceSessionKey(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    LOGI("AuthSrcManager::AuthDeviceSessionKey start. keyLen: %{public}u", sessionKeyLen);
    if (context_ == nullptr || context_->authMessageProcessor == nullptr || context_->authStateMachine == nullptr) {
        LOGE("AuthSrcManager::AuthDeviceSessionKey failed, auth context not initial.");
        return;
    }
    if (requestId != context_->requestId) {
        LOGE("AuthSrcManager::onTransmit requestId %{public}" PRId64 "is error.", requestId);
        return;
    }
    int32_t ret = context_->authMessageProcessor->SaveSessionKey(sessionKey, sessionKeyLen);
    if (ret != DM_OK) {
        LOGE("AuthSrcManager::AuthDeviceSessionKey, save session key error, ret: %{public}d", ret);
    }

    context_->authStateMachine->NotifyEventFinish(ON_SESSION_KEY_RETURNED);
}

char *AuthSinkManager::AuthDeviceRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    LOGI("AuthSinkManager::AuthDeviceRequest start");
    (void)requestId;
    (void)reqParams;
    JsonObject jsonObj;

    DmAuthStateType curState = context_->authStateMachine->GetCurState();
    if (curState == DmAuthStateType::AUTH_SINK_PIN_AUTH_START_STATE ||
        curState == DmAuthStateType::AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE||
        curState == DmAuthStateType::AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE) {
        std::string pinCode = "";
        if (GetPinCode(pinCode) == ERR_DM_FAILED || pinCode == "") {
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_REJECTED;
        } else {
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
            jsonObj[FIELD_PIN_CODE] = pinCode;
        }
        std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode));
        LOGI("AuthDeviceRequest pinCodeHash: %{public}s", pinCodeHash.c_str());
    } else if (curState == DmAuthStateType::AUTH_SINK_CREDENTIAL_AUTH_START_STATE) {
        if (context_->isOnline) { // Non-first time certification
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
            jsonObj[FIELD_CRED_ID] = context_->accessee.transmitCredentialId;
        } else if (!context_->isAppCredentialVerified) { // First-time authentication &&  appCred auth
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
            jsonObj[FIELD_CRED_ID] = context_->accessee.transmitCredentialId;
        } else {  // First-time authentication && User credential authentication
            jsonObj[FIELD_CONFIRMATION] = RequestResponse::REQUEST_ACCEPTED;
            jsonObj[FIELD_CRED_ID] = context_->accessee.lnnCredentialId;
        }
    }
    jsonObj[FIELD_SERVICE_PKG_NAME] = std::string(DM_PKG_NAME);
    std::string jsonStr = jsonObj.Dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

int32_t AuthManager::GetPinCode(std::string &code)
{
    if (context_ == nullptr) {
        LOGE("AuthManager failed to GetPinCode because context_ is nullptr");
        return ERR_DM_FAILED;
    }
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(context_->pinCode));
    LOGI("GetPinCode pinCodeHash: %{public}s", pinCodeHash.c_str());
    code = context_->pinCode;
    return DM_OK;
}

// Reacquire BindParams to rebuild the link, generally used when switching between old and new protocol objects
void AuthManager::GetBindTargetParams(std::string &pkgName, PeerTargetId &targetId,
    std::map<std::string, std::string> &bindParam)
{
    pkgName = context_->pkgName;
    targetId = targetId_;
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam = bindParam_;
    }
    LOGI("AuthManager::GetBindTargetParams get pkgName %{public}s to reuse", pkgName.c_str());
    return;
}

void AuthManager::GetAuthCodeAndPkgName(std::string &pkgName, std::string &authCode)
{
    if (context_ == nullptr || context_->importAuthCode.empty() || context_->importPkgName.empty()) {
        LOGE("GetAuthCodeAndPkgName failed, authCode or pkgName is empty");
        return;
    }
    authCode = context_->importAuthCode;
    pkgName = context_->importPkgName;
}

void AuthManager::SetBindTargetParams(const PeerTargetId &targetId)
{
    targetId_ = targetId;
    LOGI("AuthManager::SetBindTargetParams set targetId to reuse");
    return;
}

void AuthManager::ClearSoftbusSessionCallback()
{}

void AuthManager::PrepareSoftbusSessionCallback()
{}

void AuthManager::GetBindCallerInfo()
{
    LOGI("start.");
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        if (bindParam_.find("bindCallerUserId") != bindParam_.end()) {
            context_->processInfo.userId = std::atoi(bindParam_["bindCallerUserId"].c_str());
        }
        if (bindParam_.find("bindCallerTokenId") != bindParam_.end()) {
            context_->accesser.tokenId = std::atoi(bindParam_["bindCallerTokenId"].c_str());
        }
        if (bindParam_.find("bindCallerBindLevel") != bindParam_.end()) {
            context_->accesser.bindLevel = std::atoi(bindParam_["bindCallerBindLevel"].c_str());
        }
        if (bindParam_.find("bindCallerBundleName") != bindParam_.end()) {
            context_->accesser.bundleName = bindParam_["bindCallerBundleName"];
        }
        if (bindParam_.find("bindCallerHostPkgLabel") != bindParam_.end()) {
            context_->pkgLabel = bindParam_["bindCallerHostPkgLabel"];
        }
    }
}

void AuthManager::DeleteTimer()
{
    if (context_ != nullptr) {
        context_->successFinished = true;
        context_->authStateMachine->Stop();  // Stop statemMachine thread
        context_->timer->DeleteAll();
        LOGI("AuthManager context deleteTimer successful.");
    }
    {
        std::lock_guard<std::mutex> lock(bindParamMutex_);
        bindParam_.clear();
    }
    LOGI("end.");
}

int32_t AuthManager::HandleBusinessEvents(const std::string &businessId, int32_t action)
{
    LOGI("AuthManager::HandleBusinessEvents start.");
    DistributedDeviceProfile::BusinessEvent rejectEvent;
    rejectEvent.SetBusinessKey(DM_REJECT_KEY);
    JsonObject rejectJson;
    rejectJson[DM_BUSINESS_ID] = businessId;
    rejectJson[DM_AUTH_DIALOG_REJECT] = (action == USER_OPERATION_TYPE_CANCEL_AUTH);
    rejectJson[DM_TIMESTAMP] = std::to_string(GetCurrentTimestamp());
    rejectEvent.SetBusinessValue(rejectJson.Dump());
    int32_t ret = DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().PutBusinessEvent(rejectEvent);
    if (ret != DM_OK) {
        LOGE("HandleBusinessEvents failed to store reject_event, ret: %{public}d", ret);
        return ret;
    }
    LOGI("HandleBusinessEvents successfully stored reject_event.");
    return DM_OK;
}
int32_t AuthManager::CheckProxyAuthParamVaild(const std::string &extra)
{
    LOGI("start.");
    JsonObject jsonObject(extra);
    if (jsonObject.IsDiscarded() || !IsString(jsonObject, PARAM_KEY_IS_PROXY_BIND)) {
        return DM_OK;
    }
    if (jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<std::string>() != DM_VAL_TRUE) {
        return DM_OK;
    }
    if (!AppManager::GetInstance().IsSystemSA()) {
        LOGE("no proxy permission");
        return ERR_DM_NO_PERMISSION;
    }
    if (!jsonObject.Contains(PARAM_KEY_SUBJECT_PROXYED_SUBJECTS) ||
        !IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        LOGE("no subject proxyed apps");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!item.Contains(TAG_BUNDLE_NAME) || !IsString(item, TAG_BUNDLE_NAME)) {
            LOGE("bundleName invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        if (!item.Contains(TAG_TOKENID) || !IsInt64(item, TAG_TOKENID)) {
            LOGE("tokenId invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
    }
    return DM_OK;
}

void AuthManager::ParseProxyJsonObject(const JsonObject &jsonObject)
{
    if (context_ == nullptr || jsonObject.IsDiscarded() || !IsString(jsonObject, PARAM_KEY_IS_PROXY_BIND) ||
        jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<std::string>() != DM_VAL_TRUE) {
        return;
    }
    context_->IsProxyBind = true;
    if (IsString(jsonObject, PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT) &&
        jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT].Get<std::string>() == DM_VAL_FALSE) {
        context_->IsCallingProxyAsSubject = false;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        LOGE("no subject proxyed apps");
        return;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_BUNDLE_NAME)) {
            LOGE("bundleName invalid");
            return;
        }
        if (!IsInt64(item, TAG_TOKENID)) {
            LOGE("tokenId invalid");
            return;
        }
        std::string bundleName = item[TAG_BUNDLE_NAME].Get<std::string>();
        if (context_->accesser.bundleName == bundleName) {
            LOGE("proxy bundleName same as caller bundleName");
            return;
        }
        std::string peerBundleName = bundleName;
        if (item.Contains(PARAM_KEY_PEER_BUNDLE_NAME) && IsString(item, PARAM_KEY_PEER_BUNDLE_NAME)) {
            peerBundleName = item[PARAM_KEY_PEER_BUNDLE_NAME].Get<std::string>();
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = Crypto::Sha256(bundleName + peerBundleName);
        if (std::find(context_->subjectProxyOnes.begin(), context_->subjectProxyOnes.end(), proxyAuthContext) ==
            context_->subjectProxyOnes.end()) {
            proxyAuthContext.proxyAccesser.bundleName = bundleName;
            proxyAuthContext.proxyAccesser.tokenId = item[TAG_TOKENID].Get<int64_t>();
            proxyAuthContext.proxyAccesser.tokenIdHash =
                Crypto::GetTokenIdHash(std::to_string(proxyAuthContext.proxyAccesser.tokenId));
            proxyAuthContext.proxyAccessee.bundleName = peerBundleName;
            GetBindLevelByBundleName(bundleName, context_->accesser.userId, proxyAuthContext.proxyAccesser.bindLevel);
            context_->subjectProxyOnes.push_back(proxyAuthContext);
        }
    }
}

void AuthManager::GetBindLevelByBundleName(std::string &bundleName, int32_t userId, int32_t &bindLevel)
{
    int64_t tokenId = 0;
    if (AppManager::GetInstance().GetHapTokenIdByName(userId, bundleName, 0, tokenId) == DM_OK) {
        bindLevel = DmRole::DM_ROLE_FA;
    } else if (AppManager::GetInstance().GetNativeTokenIdByName(bundleName, tokenId) == DM_OK) {
        bindLevel = DmRole::DM_ROLE_SA;
    } else {
        LOGE("src not contain the bundlename %{public}s.", bundleName.c_str());
    }
}
}  // namespace DistributedHardware
}  // namespace OHOS