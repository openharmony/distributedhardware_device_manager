/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_DEVICEPROFILE_CONNECTOR_3RD_H
#define OHOS_DM_DEVICEPROFILE_CONNECTOR_3RD_H
#include <algorithm>
#include <map>
#include <string>
#include <unordered_set>
#include "access_control_profile.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_single_instance_3rd.h"
#include "i_dp_inited_callback.h"
#include "json_object.h"
#include "local_service_info.h"
#include "parameter.h"
#include "service_info.h"
#include "single_instance.h"
#include "trusted_device_info.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

enum AllowAuthType {
    ALLOW_AUTH_ONCE = 1,
    ALLOW_AUTH_ALWAYS = 2
};

DM_EXPORT extern const uint32_t INVALIED_TYPE;
DM_EXPORT extern const uint32_t APP_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t APP_ACROSS_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t SHARE_TYPE;
DM_EXPORT extern const uint32_t DEVICE_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t DEVICE_ACROSS_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t IDENTICAL_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t SERVICE_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t SERVICE_ACROSS_ACCOUNT_TYPE;

DM_EXPORT extern const uint32_t DM_INVALIED_TYPE;
DM_EXPORT extern const uint32_t USER;
DM_EXPORT extern const uint32_t SERVICE;
DM_EXPORT extern const uint32_t APP;

extern const char* TAG_PEER_BUNDLE_NAME;
DM_EXPORT extern const char* TAG_PEER_TOKENID;

const uint32_t DM_IDENTICAL_ACCOUNT = 1;
const uint32_t DM_SHARE = 2;
const uint32_t DM_LNN = 3;
const uint32_t DM_POINT_TO_POINT = 256;
const uint32_t DM_ACROSS_ACCOUNT = 1282;
const int32_t DM_VERSION_INT_5_1_0 = 510;

enum ProfileState {
    INACTIVE = 0,
    ACTIVE = 1
};

typedef struct DmDiscoveryInfo {
    std::string pkgname;
    std::string localDeviceId;
    int32_t userId;
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
    std::string requestDeviceName;
} DmAccesser;

typedef struct DmAccessee {
    uint64_t trustTokenId;
    std::string trustBundleName;
    int32_t trustUserId;
    std::string trustAccountId;
    std::string trustDeviceId;
    int32_t trustTargetClass;
    std::string trustDeviceName;
} DmAccessee;

typedef struct DmAclIdParam {
    std::string udid;
    int32_t userId;
    int64_t accessControlId;
    int32_t skId;
    std::string credId;
    std::string pkgName;
    std::unordered_set<int64_t> tokenIds;
} DmAclIdParam;

typedef struct DmLocalUserRemovedInfo {
    std::string localUdid = "";
    int32_t preUserId = 0;
    std::vector<std::string> peerUdids = {};
} DmLocalUserRemovedInfo;

typedef struct DmRemoteUserRemovedInfo {
    std::string peerUdid = "";
    int32_t peerUserId = 0;
    std::vector<int32_t> localUserIds = {};
} DmRemoteUserRemovedInfo;

struct AclHashItem {
    std::string version;
    std::vector<std::string> aclHashList;
};

struct AuthOnceAclInfo {
    std::string peerUdid;
    int32_t peerUserId = 0;
    int32_t localUserId = 0;
    bool operator==(const AuthOnceAclInfo &other) const
    {
        return peerUserId == other.peerUserId && localUserId == other.localUserId && peerUdid == other.peerUdid;
    }
};

namespace OHOS {
namespace DistributedHardware {
class IDeviceProfileConnector {
public:
    virtual ~IDeviceProfileConnector() {}
};

class DeviceProfileConnector3rd : public IDeviceProfileConnector {
    DM_DECLARE_SINGLE_INSTANCE_3RD(DeviceProfileConnector3rd);
public:
    DM_EXPORT int32_t DeleteSessionKey(int32_t userId, int32_t sessionKeyId);
    DM_EXPORT int32_t PutSessionKey(int32_t userId, const std::vector<unsigned char> &sessionKeyArray,
        int32_t &sessionKeyId);
    DM_EXPORT int32_t GetSessionKey(int32_t userId, int32_t sessionKeyId,
        std::vector<unsigned char> &sessionKeyArray);
};

DM_EXPORT extern "C" IDeviceProfileConnector *CreateDpConnectorInstance();
using CreateDpConnectorFuncPtr = IDeviceProfileConnector *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICEPROFILE_CONNECTOR_3RD_H
