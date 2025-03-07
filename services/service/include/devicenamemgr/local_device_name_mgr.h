/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_NAME_MGR_H
#define OHOS_DEVICE_NAME_MGR_H

#include "datashare_helper.h"

namespace OHOS {
namespace DistributedHardware {
class LocalDeviceNameMgr : public std::enable_shared_from_this<LocalDeviceNameMgr> {
public:
    LocalDeviceNameMgr();
    virtual ~LocalDeviceNameMgr();
    int32_t QueryLocalDeviceName();
    void RegisterDeviceNameChangeCb();
    int32_t QueryLocalDisplayName();
    void RegisterDisplayNameChangeCb();
    std::string GetLocalDisplayName() const;
    std::string GetLocalDeviceName() const;

private:
    std::shared_ptr<DataShare::DataShareHelper> GetDataShareHelper();
    int32_t GetDeviceNameFromDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
        std::shared_ptr<Uri> uri, const char *key, std::string &deviceName);
    int32_t GetDefaultDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
        std::string &deviceName);
    int32_t GetUserDefinedDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
        std::string &deviceName);
    int32_t GetDisplayDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
        std::string &deviceName);
    int32_t GetActiveOsAccountIds();
private:
    static std::mutex devNameMtx_;
    std::string localDeviceName_;
    std::string localDisplayName_;
};
} // DistributedHardware
} // OHOS

#endif // OHOS_DEVICE_NAME_MGR_H