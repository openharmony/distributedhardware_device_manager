/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_PUBLISH_H
#define OHOS_DM_SOFTBUS_PUBLISH_H

#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
void PublishCommonEventCallback(int32_t bluetoothState, int32_t wifiState, int32_t screenState);
class SoftbusPublish {
public:
    SoftbusPublish();
    ~SoftbusPublish();

    static void OnParameterChgCallback(const char *key, const char *value, void *context);
    static void OnSoftbusPublishResult(int publishId, PublishResult result);

    int32_t PublishSoftbusLNN();
    int32_t StopPublishSoftbusLNN(int32_t publishId);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_PUBLISH_H
