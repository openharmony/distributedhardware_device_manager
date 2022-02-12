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

#ifndef OHOS_DEVICE_MANAGER_MESSAGE_PROCESSING_H
#define OHOS_DEVICE_MANAGER_MESSAGE_PROCESSING_H

#include <string>

#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "nlohmann/json.hpp"
#include "get_authenticationparam_rsp.h"
#include "get_trustdevice_rsp.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class MessageProcessing {
DECLARE_SINGLE_INSTANCE(MessageProcessing);
public:
    int32_t ModuleInit();
    int32_t GetTrustedDeviceList(std::string &pkgName, std::string &extra, DmDeviceInfo **info, int32_t *infoNum, std::shared_ptr<GetTrustdeviceRsp> prsp);
    int32_t StartDeviceDiscovery(std::string &pkgName, const DmSubscribeInfo &dmSubscribeInfo);
    int32_t StopDiscovery(std::string &pkgName, uint16_t subscribeId);
    int32_t AuthenticateDevice(std::string &pkgName, const DmDeviceInfo &deviceInfo,
        const DmAppImageInfo &imageInfo, std::string &extra);
    int32_t CheckAuthentication(std::string &authPara);
    int32_t GetAuthenticationParam(std::string &pkgName, DmAuthParam &authParam, std::shared_ptr<GetAuthParamRsp> prsp);
    int32_t SetUserOperation(std::string &pkgName, int32_t action);
    static int32_t GenRandInt(int32_t minPinToken, int32_t maxPinToken);
private:
    int32_t CheckParamValid(nlohmann::json &extraJson, const DmAppImageInfo &imageInfo);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_MESSAGE_PROCESSING_H
