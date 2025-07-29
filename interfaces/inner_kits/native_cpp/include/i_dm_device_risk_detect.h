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
#ifndef OHOS_I_DM_DEVICE_RISK_DETECT_H
#define OHOS_I_DM_DEVICE_RISK_DETECT_H
namespace OHOS {
namespace DistributedHardware {
class IDMDeviceRiskDetect {
public:
    virtual ~IDMDeviceRiskDetect() = default;
    virtual int32_t Initialize() = 0;
    virtual int32_t Release() = 0;
    virtual bool IsDeviceHasRisk() = 0;
};

using CreateDMDeviceRiskDetectFuncPtr = IDMDeviceRiskDetect *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_DEVICE_RISK_DETECT_H
