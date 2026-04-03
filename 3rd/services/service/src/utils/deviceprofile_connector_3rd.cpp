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

#include <cstdio>

#include "deviceprofile_connector_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "multiple_user_connector_3rd.h"
#include "distributed_device_profile_client.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"

using namespace OHOS::DistributedDeviceProfile;

const uint32_t INVALID_TYPE = 0;
const uint32_t APP_PEER_TO_PEER_TYPE = 1;
const uint32_t APP_ACROSS_ACCOUNT_TYPE = 2;
const uint32_t SERVICE_PEER_TO_PEER_TYPE = 3;
const uint32_t SERVICE_ACROSS_ACCOUNT_TYPE = 4;
const uint32_t SHARE_TYPE = 5;
const uint32_t DEVICE_PEER_TO_PEER_TYPE = 6;
const uint32_t DEVICE_ACROSS_ACCOUNT_TYPE = 7;
const uint32_t IDENTICAL_ACCOUNT_TYPE = 8;
const uint32_t DM_INVALID_TYPE = 2048;
const uint32_t SERVICE = 2;
const uint32_t APP = 3;
const uint32_t USER = 1;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

const char* TAG_PEER_BUNDLE_NAME = "peerBundleName";
const char* TAG_PEER_TOKENID = "peerTokenId";
const char* TAG_ACL = "accessControlTable";
const char* TAG_DMVERSION = "dmVersion";
const char* TAG_ACL_HASH_KEY_VERSION = "aclVersion";
const char* TAG_ACL_HASH_KEY_ACLHASHLIST = "aclHashList";

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t DM_SUPPORT_ACL_AGING_VERSION_NUM = 1;
constexpr uint32_t AUTH_EXT_WHITE_LIST_NUM = 1;
constexpr const static char* g_extWhiteList[AUTH_EXT_WHITE_LIST_NUM] = {
    "CastEngineService",
};
enum DmDevBindType : int32_t {
    DEVICE_PEER_TO_PEER_BIND_TYPE = 3,
    DEVICE_ACROSS_ACCOUNT_BIND_TYPE = 4,
    IDENTICAL_ACCOUNT_BIND_TYPE = 5
};

}
IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector3rd);

int32_t DeviceProfileConnector3rd::PutSessionKey(int32_t userId, const std::vector<unsigned char> &sessionKeyArray,
    int32_t &sessionKeyId)
{
    if (sessionKeyArray.empty() || sessionKeyArray.size() > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey size invalid");
        return ERR_DM_FAILED;
    }
    int32_t ret = DistributedDeviceProfileClient::GetInstance().PutSessionKey(static_cast<uint32_t>(userId),
        sessionKeyArray, sessionKeyId);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector3rd::DeleteSessionKey(int32_t userId, int32_t sessionKeyId)
{
    int32_t ret =
        DistributedDeviceProfileClient::GetInstance().DeleteSessionKey(static_cast<uint32_t>(userId), sessionKeyId);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceProfileConnector3rd::GetSessionKey(int32_t userId, int32_t sessionKeyId,
    std::vector<unsigned char> &sessionKeyArray)
{
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetSessionKey(static_cast<uint32_t>(userId),
        sessionKeyId, sessionKeyArray);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    return DM_OK;
}
IDeviceProfileConnector *CreateDpConnectorInstance()
{
    return &DeviceProfileConnector3rd::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
