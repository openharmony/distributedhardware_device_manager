/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
#include "message_processing.h"
#include "command_dispatch.h"
#include "device_manager_log.h"
#include "message_def.h"
#include "constants.h"
#include "device_manager_errno.h"
#include "server_stub.h"
#include "securec.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "get_trustdevice_req.h"
#include "start_discovery_req.h"
#include "stop_discovery_req.h"
#include "set_useroperation_req.h"
#include "authenticate_device_req.h"
#include "check_authenticate_req.h"
#include "get_authenticationparam_rsp.h"
#include "get_trustdevice_rsp.h"

namespace OHOS {
namespace DistributedHardware{
IMPLEMENT_SINGLE_INSTANCE(CommandDispatch);
int32_t CommandDispatch::MessageSendCmd(int32_t cmdCode, std::shared_ptr<MessageReq> req, std::shared_ptr<MessageRsp> rsp)
{
    if (req == nullptr || rsp == nullptr) {
        DMLOG(DM_LOG_INFO, "Message req or rsp is null");
        return DEVICEMANAGER_NULLPTR;
    }
    int32_t ret = CommandDispatch::CmdProcessing(cmdCode, req, rsp);
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "MessageSendCmd Failed with ret %d", ret);
        return ret;
    }
    return DEVICEMANAGER_OK;
}

int32_t CommandDispatch::CmdProcessing(int32_t cmdCode, std::shared_ptr<MessageReq> req, std::shared_ptr<MessageRsp> rsp)
{
    int32_t ret = 1;
    DMLOG(DM_LOG_INFO, "SendCmd:%d", cmdCode);
    switch (cmdCode) {
        case GET_TRUST_DEVICE_LIST: {
            std::shared_ptr<GetTrustdeviceReq> pReq = std::static_pointer_cast<GetTrustdeviceReq>(req);
            std::shared_ptr<GetTrustdeviceRsp> prsp = std::static_pointer_cast<GetTrustdeviceRsp>(rsp);
            std::string pkgName = pReq->GetPkgName();
            std::string extra = pReq->GetExtra();

            DMLOG(DM_LOG_INFO, "enter GetTrustedDeviceList.");
            DmDeviceInfo *info = nullptr;
            int32_t infoNum = 0;
            ret = MessageProcessing::GetInstance().GetTrustedDeviceList(pkgName, extra, &info, &infoNum, prsp);
            if (prsp == nullptr) {
                DMLOG(DM_LOG_INFO, "Message rsp is null.");
                return DEVICEMANAGER_NULLPTR;
                }
            prsp->SetErrCode(ret);
            break;
        }
        case START_DEVICE_DISCOVER: {
            std::shared_ptr<StartDiscoveryReq> pReq = std::static_pointer_cast<StartDiscoveryReq>(req);
            std::string pkgName = pReq->GetPkgName();
            const DmSubscribeInfo dmSubscribeInfo = pReq->GetSubscribeInfo();
            DMLOG(DM_LOG_INFO, "StartDeviceDiscovery service listener.");

            ret = MessageProcessing::GetInstance().StartDeviceDiscovery(pkgName, dmSubscribeInfo);
            rsp->SetErrCode(ret);
            break; 
        }
        case STOP_DEVICE_DISCOVER: {
            DMLOG(DM_LOG_INFO, "StopDeviceDiscovery service listener.");

            std::shared_ptr<StopDiscoveryReq> pReq = std::static_pointer_cast<StopDiscoveryReq>(req);
            std::string pkgName = pReq->GetPkgName();
            uint16_t subscribeId = pReq->GetSubscribeId();
            ret = MessageProcessing::GetInstance().StopDiscovery(pkgName, subscribeId);
            rsp->SetErrCode(ret);
            break;
        } 
        case SERVER_USER_AUTHORIZATION_OPERATION: {
            std::shared_ptr<GetOperationReq> pReq = std::static_pointer_cast<GetOperationReq>(req);
            std::string pkgName= pReq->GetPkgName();
            int32_t action = pReq->GetOperation();

            DMLOG(DM_LOG_INFO, "enter server user authorization operation.");
            ret = MessageProcessing::GetInstance().SetUserOperation(pkgName, action);
            rsp->SetErrCode(ret);
            break;
        }
        case SERVER_GET_AUTHENTCATION_INFO: {
            std::shared_ptr<MessageReq> pReq = std::static_pointer_cast<MessageReq>(req);
            std::shared_ptr<GetAuthParamRsp> prsp = std::static_pointer_cast<GetAuthParamRsp>(rsp);
            std::string pkgName = pReq->GetPkgName();
            DmAuthParam authParam = {
                .packageName = "",
                .appName     = "",
                .appDescription = "",
                .authType    = 0,
                .business    = 0,
                .pincode     = 0,
                .direction   = 0,
                .pinToken    = 0
            };

            DMLOG(DM_LOG_INFO, "DeviceManagerStub:: GET_AUTHENTCATION_INFO:pkgName:%s", pkgName.c_str());
            ret = MessageProcessing::GetInstance().GetAuthenticationParam(pkgName, authParam, prsp);
            prsp->SetErrCode(ret);
            break;
        }
        case AUTHENTICATE_DEVICE: {
            std::shared_ptr<AuthenticateDeviceReq> pReq = std::static_pointer_cast<AuthenticateDeviceReq>(req);
            std::string pkgName = pReq->GetPkgName();
            std::string extra   = pReq->GetExtra();
            DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
            DmAppImageInfo imageInfo(nullptr, 0, nullptr, 0);

            DMLOG(DM_LOG_INFO, "DeviceManagerStub:: AUTHENTCATION_DEVICE:pkgName:%s", pkgName.c_str());
            ret = MessageProcessing::GetInstance().AuthenticateDevice(pkgName, deviceInfo, imageInfo, extra);
            rsp->SetErrCode(ret);
            break;
        }
        case CHECK_AUTHENTICATION: {
            std::shared_ptr<CheckAuthenticateReq> pReq = std::static_pointer_cast<CheckAuthenticateReq>(req);
            std::string pkgName = pReq->GetPkgName();
            std::string authPara = pReq->GetAuthPara();

            DMLOG(DM_LOG_INFO, "DeviceManagerStub:: CHECK_AUTHENTCATION:pkgName:%s", pkgName.c_str());
            ret = MessageProcessing::GetInstance().CheckAuthentication(authPara);
            rsp->SetErrCode(ret);
            break;
        }

        default:
            break;
    }
    return ret;
}
}
}