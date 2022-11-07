/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "command_dispatch.h"
#include "dm_log.h"
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "message_def.h"
#include "get_trustdevice_req.h"
#include "get_trustdevice_rsp.h"
#include "start_discovery_req.h"
#include "stop_discovery_req.h"
#include "get_useroperation_req.h"
#include "authenticate_device_req.h"
#include "verify_authenticate_req.h"
#include "get_local_device_info_rsp.h"
#include "get_info_by_network_rsp.h"
#include "get_info_by_network_req.h"
#include "unauthenticate_device_req.h"
#include "get_dmfaparam_rsp.h"
#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {
DeviceManagerImpl &DeviceManagerImpl::GetInstance()
{
    static DeviceManagerImpl instance;
    return instance;
}

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
{
    LOGI("DeviceManager::InitDeviceManager start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || dmInitCallback == nullptr) {
        LOGE("InitDeviceManager error: Invalid parameter");
        return DM_INVALID_VALUE;
    }

    CommandDispatch::GetInstance().AddPkgName(pkgName);
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    LOGI("InitDeviceManager success");
    return DM_OK;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(const std::string &pkgName)
{
    LOGI("DeviceManager::UnInitDeviceManager start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("InitDeviceManager error: Invalid parameter");
        return DM_INVALID_VALUE;
    }

    CommandDispatch::GetInstance().DeletePkgName(pkgName);
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    LOGI("UnInitDeviceManager success");
    return DM_OK;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("DeviceManager::GetTrustedDeviceList start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("GetTrustedDeviceList error: Invalid para");
        return DM_INVALID_VALUE;
    }

    std::shared_ptr<GetTrustDeviceReq> req = std::make_shared<GetTrustDeviceReq>();
    std::shared_ptr<GetTrustDeviceRsp> rsp = std::make_shared<GetTrustDeviceRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);

    if (CommandDispatch::GetInstance().MessageSendCmd(GET_TRUST_DEVICE_LIST,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList error: failed ret: %d", ret);
        return ret;
    }

    deviceList = rsp->GetDeviceVec();
    LOGI("GetTrustedDeviceList completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    LOGI("DeviceManager::GetLocalDeviceInfo start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("GetLocalDeviceInfo error: Invalid para");
        return DM_INVALID_VALUE;
    }

    std::shared_ptr<MessageReq> req = std::make_shared<MessageReq>();
    std::shared_ptr<GetLocalDeviceInfoRsp> rsp = std::make_shared<GetLocalDeviceInfoRsp>();
    req->SetPkgName(pkgName);

    if (CommandDispatch::GetInstance().MessageSendCmd(GET_LOCAL_DEVICE_INFO, req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    if (rsp->GetErrCode() != DM_OK) {
        LOGE("GetLocalDeviceInfo error: failed ret: %d", rsp->GetErrCode());
        return DM_IPC_RESPOND_ERROR;
    }

    info = rsp->GetLocalDeviceInfo();
    LOGI("GetLocalDeviceInfo completed,pkgname%s", req->GetPkgName().c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra,
    std::shared_ptr<DeviceStateCallback> callback)
{
    LOGI("DeviceManager::RegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        LOGE("RegisterDevStateCallback error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    LOGI("RegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName)
{
    LOGI("DeviceManager::UnRegisterDevStateCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("UnRegisterDevStateCallback error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    LOGI("UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}
int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                const std::string &extra, std::shared_ptr<DiscoveryCallback> callback)
{
    LOGI("DeviceManager::StartDeviceDiscovery start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || callback == nullptr) {
        LOGE("StartDeviceDiscovery error: Invalid para");
        return DM_INVALID_VALUE;
    }

    LOGI("DeviceManager StartDeviceDiscovery in, pkgName %s", pkgName.c_str());
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeInfo.subscribeId, callback);

    std::shared_ptr<StartDiscoveryReq> req = std::make_shared<StartDiscoveryReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetSubscribeInfo(subscribeInfo);

    if (CommandDispatch::GetInstance().MessageSendCmd(START_DEVICE_DISCOVER,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StartDeviceDiscovery error: Failed with ret %d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("DeviceManager::StopDeviceDiscovery start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("StopDeviceDiscovery error: Invalid para");
        return DM_INVALID_VALUE;
    }

    LOGI("StopDeviceDiscovery in, pkgName %s", pkgName.c_str());
    std::shared_ptr<StopDiscoveryReq> req = std::make_shared<StopDiscoveryReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();
    req->SetPkgName(pkgName);
    req->SetSubscribeId(subscribeId);

    if (CommandDispatch::GetInstance().MessageSendCmd(STOP_DEVICE_DISCOVER,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("StopDeviceDiscovery error: Failed with ret %d", ret);
        return ret;
    }
        
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    LOGI("StopDeviceDiscovery completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                              const DmDeviceInfo &deviceInfo, const std::string &extra,
                                              std::shared_ptr<AuthenticateCallback> callback)
{
    LOGI("DeviceManager::AuthenticateDevice start , pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("AuthenticateDevice error: Invalid para");
        return DM_INVALID_VALUE;
    }

    std::string strDeviceId = deviceInfo.deviceId;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, strDeviceId, callback);
    std::shared_ptr<AuthenticateDeviceReq> req = std::make_shared<AuthenticateDeviceReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    req->SetAuthType(authType);
    req->SetDeviceInfo(deviceInfo);

    if (CommandDispatch::GetInstance().MessageSendCmd(AUTHENTICATE_DEVICE,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice error: Failed with ret %d", ret);
        return ret;
    }

    LOGI("DeviceManager::AuthenticateDevice completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceManager::UnAuthenticateDevice start , pkgName: %s, deviceId: %s", pkgName.c_str(), deviceId.c_str());
    if (deviceId.empty()) {
        LOGE("UnAuthenticateDevice error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DmDeviceInfo deviceInfo;
    strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, deviceId.c_str());
    std::shared_ptr<UnAuthenticateDeviceReq> req = std::make_shared<UnAuthenticateDeviceReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceInfo(deviceInfo);

    if (CommandDispatch::GetInstance().MessageSendCmd(UNAUTHENTICATE_DEVICE,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("UnAuthenticateDevice error: Failed with ret %d", ret);
        return ret;
    }

    LOGI("UnAuthenticateDevice completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDeviceManagerFaCallback(const std::string &packageName,
    std::shared_ptr<DeviceManagerUiCallback> callback)
{
    LOGI("DeviceManager::RegisterDeviceManagerFaCallback start, pkgName: %s", packageName.c_str());
    if (packageName.empty() || callback == nullptr) {
        LOGE("RegisterDeviceManagerFaCallback error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
    LOGI("DeviceManager::RegisterDevStateCallback completed, pkgName: %s", packageName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    LOGI("DeviceManager::UnRegisterDeviceManagerFaCallback start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("UnRegisterDeviceManagerFaCallback error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    LOGI("DeviceManager::UnRegisterDevStateCallback completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}
int32_t DeviceManagerImpl::VerifyAuthentication(const std::string &pkgName, const std::string &authPara,
                                                std::shared_ptr<VerifyAuthCallback> callback)
{
    LOGI("DeviceManager::VerifyAuthentication start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("VerifyAuthentication error: Invalid para");
        return DM_INVALID_VALUE;
    }

    DeviceManagerNotify::GetInstance().RegisterVerifyAuthenticationCallback(pkgName, authPara, callback);

    std::shared_ptr<VerifyAuthenticateReq> req = std::make_shared<VerifyAuthenticateReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();
    req->SetPkgName(pkgName);
    req->SetAuthPara(authPara);

    if (CommandDispatch::GetInstance().MessageSendCmd(VERIFY_AUTHENTICATION,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("VerifyAuthentication error: Failed with ret %d", ret);
        return ret;
    }

    LOGI("VerifyAuthentication completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam)
{
    LOGI("DeviceManager::GetFaParam start, pkgName: %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("GetFaParam failed, pkgName is empty");
        return DM_INVALID_VALUE;
    }

    std::shared_ptr<MessageReq> req = std::make_shared<MessageReq>();
    std::shared_ptr<GetDmFaParamRsp> rsp = std::make_shared<GetDmFaParamRsp>();
    req->SetPkgName(pkgName);

    if (CommandDispatch::GetInstance().MessageSendCmd(SERVER_GET_DMFA_INFO, req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    dmFaParam = rsp->GetDmAuthParam();
    LOGI("GetFaParam completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("SetUserOperation failed, pkgName or params is empty");
        return DM_INVALID_VALUE;
    }
    LOGI("DeviceManager::SetUserOperation start, pkgName: %s", pkgName.c_str());
    std::shared_ptr<GetUserOperationReq> req = std::make_shared<GetUserOperationReq>();
    std::shared_ptr<MessageRsp> rsp = std::make_shared<MessageRsp>();

    req->SetPkgName(pkgName);
    req->SetOperation(action);

    if (CommandDispatch::GetInstance().MessageSendCmd(SERVER_USER_AUTH_OPERATION,  req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }
    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("SetUserOperation Failed with ret %d", ret);
        return ret;
    }

    LOGI("SetUserOperation completed, pkgName: %s", pkgName.c_str());
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &udid)
{
    if (pkgName.empty()) {
        LOGE("GetUdidByNetworkId failed, pkgName is empty");
        return DM_INVALID_VALUE;
    }

    std::shared_ptr<GetInfoByNetWorkReq> req = std::make_shared<GetInfoByNetWorkReq>();
    std::shared_ptr<GetInfoByNetWorkRsp> rsp = std::make_shared<GetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    if (CommandDispatch::GetInstance().MessageSendCmd(GET_UDID_BY_NETWORK, req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("DeviceManager::GetUdidByNetworkId Failed with ret %d", ret);
        return ret;
    }

    udid = rsp->GetUdid();
    return DM_OK;
}

int32_t DeviceManagerImpl::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                              std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("GetUuidByNetworkId failed, pkgName is empty");
        return DM_INVALID_VALUE;
    }

    std::shared_ptr<GetInfoByNetWorkReq> req = std::make_shared<GetInfoByNetWorkReq>();
    std::shared_ptr<GetInfoByNetWorkRsp> rsp = std::make_shared<GetInfoByNetWorkRsp>();
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);

    if (CommandDispatch::GetInstance().MessageSendCmd(GET_UUID_BY_NETWORK, req, rsp) != DM_OK) {
        return DEVICEMANAGER_MESSAGE_FAILED;
    }

    int32_t ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId Failed with ret %d", ret);
        return ret;
    }
    uuid = rsp->GetUuid();
    return DM_OK;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
}
} // namespace DistributedHardware
} // namespace OHOS
