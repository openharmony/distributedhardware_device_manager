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
#include <random>

#include "securec.h"
#include "softbus_bus_center.h"
#include "softbus_adapter.h"
#include "anonymous_string.h"
#include "auth_manager.h"
#include "constants.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "dm_ability_manager.h"
#include "encrypt_utils.h"
#include "message_processing.h"
#include "get_trustdevice_rsp.h"
#include "get_authenticationparam_rsp.h"



namespace OHOS {
namespace DistributedHardware {

IMPLEMENT_SINGLE_INSTANCE(MessageProcessing);

int32_t MessageProcessing::CheckParamValid(nlohmann::json &extraJson, const DmAppImageInfo &imageInfo)
{
    if (!extraJson.contains(APP_NAME_KEY) ||
        !extraJson.contains(APP_DESCRIPTION_KEY) ||
        !extraJson.contains(AUTH_TYPE)) {
        DMLOG(DM_LOG_ERROR, "Invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    std::string appName = extraJson[APP_NAME_KEY];
    std::string appDescription = extraJson[APP_DESCRIPTION_KEY];

    if (appName.empty() || appDescription.empty()) {
        DMLOG(DM_LOG_ERROR, "Invalid app image info");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    if (extraJson[AUTH_TYPE] != AUTH_TYPE_PIN) {
        DMLOG(DM_LOG_ERROR, "invalid auth type, only support pin auth");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    return DEVICEMANAGER_OK;
}

int32_t MessageProcessing::GenRandInt(int32_t randMin, int32_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

int32_t MessageProcessing::GetTrustedDeviceList(std::string &pkgName, std::string &extra,
    DmDeviceInfo **info, int32_t *infoNum, std::shared_ptr<GetTrustdeviceRsp> prsp)
{
    if (info == nullptr || infoNum == nullptr) {
        return DEVICEMANAGER_NULLPTR;
    }
    DMLOG(DM_LOG_INFO, "In, pkgName: %s", pkgName.c_str());

    std::vector<DmDeviceInfo> deviceInfoVec;

    NodeBasicInfo *nodeInfo = nullptr;
    *info = nullptr;
    *infoNum = 0;

    int32_t ret = SoftbusAdapter::GetTrustDevices(pkgName, &nodeInfo, infoNum);
    if (ret != DEVICEMANAGER_OK || *infoNum <= 0 || nodeInfo == nullptr) {
        DMLOG(DM_LOG_ERROR, "GetTrustDevices errCode:%d, num:%d", ret, *infoNum);
        return ret;
    }

    *info = (DmDeviceInfo *)malloc(sizeof(DmDeviceInfo) * (*infoNum));
    if (*info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return DEVICEMANAGER_MALLOC_ERROR;
    }

    for (int32_t i = 0; i < *infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo *deviceInfo = *info + i;
        if (memcpy_s(deviceInfo->deviceId, sizeof(deviceInfo->deviceId), nodeBasicInfo->networkId,
            std::min(sizeof(deviceInfo->deviceId), sizeof(nodeBasicInfo->networkId))) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "memcpy failed");
        }
        if (memcpy_s(deviceInfo->deviceName, sizeof(deviceInfo->deviceName), nodeBasicInfo->deviceName,
            std::min(sizeof(deviceInfo->deviceName), sizeof(nodeBasicInfo->deviceName))) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "memcpy failed");
        }
        deviceInfo->deviceTypeId = (DMDeviceType)nodeBasicInfo->deviceTypeId;
        deviceInfoVec.emplace_back(*deviceInfo);
    }
    prsp->SetDeviceVec(deviceInfoVec);
    
    FreeNodeInfo(nodeInfo);
    free(info);
    DMLOG(DM_LOG_INFO, "success, pkgName:%s, deviceCount %d", pkgName.c_str(), *infoNum);
    return DEVICEMANAGER_OK;
}

int32_t MessageProcessing::StartDeviceDiscovery(std::string &pkgName,const DmSubscribeInfo &dmSubscribeInfo)
{
    DMLOG(DM_LOG_INFO, "In, pkgName: %s, subscribeId %d", pkgName.c_str(),
        (int32_t)dmSubscribeInfo.subscribeId);

    DMLOG(DM_LOG_INFO, "capability: %s", dmSubscribeInfo.capability);
    SubscribeInfo subscribeInfo;

    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = (DiscoverMode)dmSubscribeInfo.mode;
    subscribeInfo.medium = (ExchanageMedium)dmSubscribeInfo.medium;
    subscribeInfo.freq = (ExchangeFreq)dmSubscribeInfo.freq;
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = dmSubscribeInfo.capability;
    subscribeInfo.capabilityData = nullptr;
    subscribeInfo.dataLen = 0;
    return SoftbusAdapter::StartDiscovery(pkgName, &subscribeInfo);
}

int32_t MessageProcessing::StopDiscovery(std::string &pkgName, uint16_t subscribeId)
{
    DMLOG(DM_LOG_INFO, "In, pkgName: %s, subscribeId %d", pkgName.c_str(), (int32_t)subscribeId);
    return SoftbusAdapter::StopDiscovery(pkgName, subscribeId);
}

int32_t MessageProcessing::SetUserOperation(std::string &pkgName, int32_t action)
{
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    AuthManager::GetInstance().OnUserOperate(action);
    return SUCCESS;
}

int32_t MessageProcessing::GetAuthenticationParam(std::string &pkgName, DmAuthParam &authParam, std::shared_ptr<GetAuthParamRsp> prsp)
{
    DmAuthParam authParamside = {
        .packageName = "",
        .appName     = "",
        .appDescription = "",
        .authType    = 0,
        .business    = 0,
        .pincode     = 0,
        .direction   = 0,
        .pinToken    = 0
    };
    
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }

    DmAbilityManager::GetInstance().StartAbilityDone();
    AuthManager::GetInstance().GetAuthenticationParam(authParam);

    authParamside.direction = authParam.direction;
    authParamside.authType = authParam.authType;
    if (authParamside.direction == AUTH_SESSION_SIDE_CLIENT) {
        authParamside.pinToken = authParam.pinToken;
        prsp->SetAuthParam(authParamside);
        
        DMLOG(DM_LOG_DEBUG, "DeviceManagerStub::is Client so just return direction");
        return DEVICEMANAGER_OK;
    }

    authParamside.packageName = authParam.packageName;
    authParamside.appName     = authParam.appName;
    authParamside.appDescription = authParam.appDescription;
    authParamside.business = authParam.business;
    authParamside.pincode  = authParam.pincode;

    int32_t appIconLen = authParam.imageinfo.GetAppIconLen();
    int32_t appThumbnailLen = authParam.imageinfo.GetAppThumbnailLen();
    uint8_t *appIconBuffer = nullptr;
    uint8_t *appThumbBuffer = nullptr;
    if (appIconLen > 0 && authParam.imageinfo.GetAppIcon() != nullptr) {
        appIconBuffer = const_cast<uint8_t *>(authParam.imageinfo.GetAppIcon());
    }
    if (appThumbnailLen > 0 && authParam.imageinfo.GetAppThumbnail() != nullptr) {
        appThumbBuffer = const_cast<uint8_t *>(authParam.imageinfo.GetAppThumbnail());
    }
    
    authParamside.imageinfo.Reset(appIconBuffer, appIconLen, appThumbBuffer, appThumbnailLen);
    prsp->SetAuthParam(authParamside);
    return DEVICEMANAGER_OK;
}

int32_t MessageProcessing::AuthenticateDevice(std::string &pkgName, const DmDeviceInfo &deviceInfo,
    const DmAppImageInfo &imageInfo, std::string &extra)
{
    if (pkgName.empty() || extra.empty()) {
        DMLOG(DM_LOG_ERROR, "invalid para");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(extra, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice extra jsonStr error");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    int32_t ret = CheckParamValid(jsonObject, imageInfo);
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice para invalid, ret %d", ret);
        return ret;
    }
    DMLOG(DM_LOG_INFO, "AuthenticateDevice In, pkgName: %s, deviceId %s", pkgName.c_str(),
        GetAnonyString(deviceInfo.deviceId).c_str());

    AuthManager::GetInstance().AuthDeviceGroup(pkgName, deviceInfo, imageInfo, extra);
    return DEVICEMANAGER_OK;
}

int32_t MessageProcessing::CheckAuthentication(std::string &authPara)
{
    if (authPara.empty()) {
        DMLOG(DM_LOG_INFO, " DeviceManagerIpcAdapter::CheckAuthentication check authPara failed");
        return DEVICEMANAGER_INVALID_VALUE;
    }
    DMLOG(DM_LOG_INFO, " DeviceManagerIpcAdapter::CheckAuthentication");
    return AuthManager::GetInstance().CheckAuthentication(authPara);
}

} // namespace DistributedHardware
} // namespace OHOS
