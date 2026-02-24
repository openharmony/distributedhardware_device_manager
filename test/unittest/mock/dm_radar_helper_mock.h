/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_RADAR_HELPER_H
#define OHOS_DM_RADAR_HELPER_H

#include <string>
#include <gmock/gmock.h>

#include "dm_radar_helper.h"

namespace OHOS {
namespace DistributedHardware {
class DmRadarHelperMock {
public:
    virtual ~DmRadarHelperMock() = default;
    virtual std::string GetAnonyUdid(std::string udid) = 0;

    static inline std::shared_ptr<DmRadarHelperMock> dmRadarHelperMock = nullptr;
};

class DmRadarHelperMockImpl : public DmRadarHelperMock {
public:
    MOCK_METHOD(std::string, GetAnonyUdid, (std::string));
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DM_RADAR_HELPER_H
