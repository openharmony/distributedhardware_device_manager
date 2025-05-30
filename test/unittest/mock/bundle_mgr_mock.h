/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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

#ifndef OHOS_DM_BUNDLE_MGR_MOCK_H
#define OHOS_DM_BUNDLE_MGR_MOCK_H

#include <gmock/gmock.h>

#include "bundle_mgr_interface.h"
#include "iremote_stub.h"

using OHOS::AppExecFwk::BundleInfo;

namespace OHOS {
namespace DistributedHardware {
class BundleMgrMock : public IRemoteStub<AppExecFwk::IBundleMgr> {
public:
    BundleMgrMock() = default;
    ~BundleMgrMock() override = default;

    MOCK_METHOD(ErrCode, GetNameForUid, (const int, std::string &));
    MOCK_METHOD(ErrCode, GetBundleInfoV9, (const std::string &, int32_t, BundleInfo &, int32_t));
    MOCK_METHOD(ErrCode, GetBundleInfoForSelf, (int32_t, BundleInfo &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_BUNDLE_MGR_MOCK_H
