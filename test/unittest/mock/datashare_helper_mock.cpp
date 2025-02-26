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

#include "datashare_helper_mock.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<DataShareHelperMock> DataShareHelperMock::instance_;

std::shared_ptr<DataShareHelperMock> DataShareHelperMock::GetOrCreateInstance()
{
    if (!instance_) {
        instance_ = std::make_shared<DataShareHelperMock>();
    }
    return instance_;
}

void DataShareHelperMock::ReleaseInstance()
{
    instance_.reset();
    instance_ = nullptr;
}
} // namespace DistributedHardware
namespace DataShare {
std::pair<int, std::shared_ptr<DataShareHelper>> DataShareHelper::Create(const sptr<IRemoteObject> &token,
    const std::string &strUri, const std::string &extUri, const int waitTime)
{
    return std::make_pair(E_OK, DataShareHelperMock::GetOrCreateInstance());
}
} // namespace DataShare
} // namespace OHOS
