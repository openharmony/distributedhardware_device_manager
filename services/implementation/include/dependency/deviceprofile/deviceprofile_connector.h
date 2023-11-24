/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_DEVICEPROFILE_CONNECTOR_H
#define OHOS_DM_DEVICEPROFILE_CONNECTOR_H
#include "access_control_profile.h"
#include "accessee.h"
#include "accesser.h"
#include "distributed_device_profile_client.h"
#include "distributed_device_profile_enums.h"

constexpr uint32_t ALLOW_AUTH_ONCE = 1;
constexpr uint32_t ALLOW_AUTH_ALWAYS = 2;

constexpr uint32_t INVALIED_TYPE = 0;
constexpr uint32_t APP_LEVEL_BIND_TYPE = 1;
constexpr uint32_t DEVICE_LEVEL_BIND_TYPE = 2;
constexpr uint32_t IDENTICAL_ACCOUNT_TYPE = 3;

constexpr uint32_t DM_IDENTICAL_ACCOUNT = 1;
constexpr uint32_t DM_POINT_TO_POINT = 256;
constexpr uint32_t DM_ACROSS_ACCOUNT = 1282;

constexpr uint32_t DEVICE = 1;
constexpr uint32_t SERVICE = 2;
constexpr uint32_t APP = 3;

constexpr uint32_t INACTIVE = 0;
constexpr uint32_t ACTIVE = 1;

typedef struct DmDiscoveryInfo {
    std::string pkgname;
    std::string localDeviceId;
    std::string remoteDeviceIdHash;
} DmDiscoveryInfo;

typedef struct DmAclInfo {
    std::string sessionKey;
    int32_t bindType;
    int32_t state;
    std::string trustDeviceId;
    int32_t bindLevel;
    int32_t authenticationType;
    std::string deviceIdHash;
} DmAclInfo;

typedef struct DmAccesser {
    uint64_t requestTokenId;
    std::string requestBundleName;
    int32_t requestUserId;
    std::string requestAccountId;
    std::string requestDeviceId;
    int32_t requestTargetClass;
} DmAccesser;

typedef struct DmAccessee {
    uint64_t trustTokenId;
    std::string trustBundleName;
    int32_t trustUserId;
    std::string trustAccountId;
    std::string trustDeviceId;
    int32_t trustTargetClass;
} DmAccessee;

typedef struct DmOfflineParam {
    uint32_t bindType;
    std::vector<std::string> pkgNameVec;
    int32_t leftAclNumber;
} DmOfflineParam;

namespace OHOS {
namespace DistributedHardware {
class DeviceProfileConnector {
public:
    DeviceProfileConnector();
    ~DeviceProfileConnector();

    std::vector<AccessControlProfile> GetAccessControlProfile();
    uint32_t CheckBindType(std::string trustDeviceId, std::string requestDeviceId);
    int32_t PutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser, DmAccessee dmAccessee);
    int32_t UpdateAccessControlList(int32_t userId, std::string &oldAccountId, std::string &newAccountId);
    std::vector<std::string> GetAppTrustDeviceList(const std::string &pkgName, const std::string &deviceId);
    DmOfflineParam GetOfflineParamFromAcl(std::string trustDeviceId, std::string requestDeviceId);
    std::vector<int32_t> GetBindTypeByPkgName(std::string pkgName, std::string requestDeviceId,
        std::string trustUdidHash);
    std::vector<int32_t> SyncAclByBindType(std::string pkgName, std::vector<int32_t> bindTypeVec,
        std::string localDeviceId, std::string targetDeviceId);
    int32_t GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isonline, int32_t &authForm);
    int32_t DeleteAccessControlList(int32_t userId, std::string &accountId);
    DmOfflineParam DeleteAccessControlList(std::string pkgName, std::string localDeviceId,
        std::string remoteDeviceId);
    std::vector<std::string> GetPkgNameFromAcl(std::string &localDeviceId, std::string &targetDeviceId);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICEPROFILE_CONNECTOR_H
