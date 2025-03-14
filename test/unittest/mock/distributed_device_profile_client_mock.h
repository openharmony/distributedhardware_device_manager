/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DISTRIBUTED_DEVICE_PROFILE_CLIENT_MOCK_H
#define OHOS_DISTRIBUTED_DEVICE_PROFILE_CLIENT_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "distributed_device_profile_client.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class DpDistributedDeviceProfileClient {
public:
    virtual ~DpDistributedDeviceProfileClient() = default;
public:
    virtual int32_t GetAccessControlProfile(std::map<std::string, std::string> params,
        std::vector<AccessControlProfile>& accessControlProfiles) = 0;
    virtual int32_t PutAccessControlProfile(const AccessControlProfile& accessControlProfile) = 0;
    virtual int32_t GetAllAccessControlProfile(std::vector<AccessControlProfile>& accessControlProfiles) = 0;
    virtual int32_t SubscribeDeviceProfileInited(int32_t saId, sptr<IDpInitedCallback> initedCb) = 0;
    virtual int32_t UnSubscribeDeviceProfileInited(int32_t saId) = 0;
    virtual int32_t PutAllTrustedDevices(const std::vector<DistributedDeviceProfile::TrustedDeviceInfo> &deviceInfos)
        = 0;
    virtual int32_t PutSessionKey(uint32_t userId, const std::vector<uint8_t>& sessionKey, int32_t& sessionKeyId) = 0;
public:
    static inline std::shared_ptr<DpDistributedDeviceProfileClient> dpDistributedDeviceProfileClient = nullptr;
};

class DistributedDeviceProfileClientMock : public DpDistributedDeviceProfileClient {
public:
    MOCK_METHOD(int32_t, GetAccessControlProfile, ((std::map<std::string, std::string>),
        std::vector<AccessControlProfile>&));
    MOCK_METHOD(int32_t, PutAccessControlProfile, (const AccessControlProfile&));
    MOCK_METHOD(int32_t, GetAllAccessControlProfile, (std::vector<AccessControlProfile>&));
    MOCK_METHOD(int32_t, SubscribeDeviceProfileInited, (int32_t, sptr<IDpInitedCallback>));
    MOCK_METHOD(int32_t, UnSubscribeDeviceProfileInited, (int32_t));
    MOCK_METHOD(int32_t, PutAllTrustedDevices, (const std::vector<DistributedDeviceProfile::TrustedDeviceInfo> &));
    MOCK_METHOD(int32_t, PutSessionKey, (uint32_t, (const std::vector<uint8_t>&), int32_t&));
};
}
}
#endif
