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

#include "hichain_listener.h"

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t MAX_DATA_LEN = 65536;
    constexpr uint32_t DM_IDENTICAL_ACCOUNT = 1;
}

static DataChangeListener dataChangeListener_ = {
    .onDeviceUnBound = HichainListener::onHichainDeviceUnBound,
};

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end() && jsonObject.at(FIELD_GROUP_TYPE).is_number_integer()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }
    // FIELD_USER_ID是osAccountId
    if (jsonObject.find(FIELD_USER_ID) != jsonObject.end() && jsonObject.at(FIELD_USER_ID).is_string()) {
        groupInfo.osAccountId = jsonObject.at(FIELD_USER_ID).get<std::string>();
    }
    // FIELD_OS_ACCOUNT_ID是userId
    if (jsonObject.find(FIELD_OS_ACCOUNT_ID) != jsonObject.end() && jsonObject.at(FIELD_OS_ACCOUNT_ID).is_string()) {
        groupInfo.userId = jsonObject.at(FIELD_OS_ACCOUNT_ID).get<int32_t>();
    }
}

HichainListener::HichainListener()
{
    LOGI("HichainListener constructor start.");
    InitDeviceAuthService();
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    LOGI("HichainListener::constructor success.");
}

HichainListener::~HichainListener()
{
    LOGI("HichainListener::destructor.");
}

void HichainListener::RegisterDataChangeCb()
{
    LOGI("HichainListener::RegisterDataChangeCb start");
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is null!");
        return;
    }
    int32_t ret = deviceGroupManager_->regDataChangeListener(DM_PKG_NAME, &dataChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]regDataChangeListener failed with ret: %{public}d.", ret);
        return;
    }
    LOGI("RegisterDataChangeCb success!");
}

void HichainListener::onHichainDeviceUnBound(const char *peerUdid, const char *groupInfo) 
{
    LOGI("HichainListener::onDeviceUnBound start");
    if (peerUdid == nullptr || groupInfo == nullptr) {
        LOGE("peerUdid or groupInfo is null!");
        return;
    }
    if (strlen(peerUdid) > MAX_DATA_LEN || strlen(groupInfo) > MAX_DATA_LEN) {
        LOGE("peerUdid or groupInfo is invalid");
        return;
    }
    LOGI("peerUdid = %{public}s, groupInfo = %{public}s", peerUdid, groupInfo);
    nlohmann::json groupInfoJsonObj = nlohmann::json::parse(string(groupInfo), nullptr, false);
    GroupInfo hichainGroupInfo;
    from_json(groupInfoJsonObj, hichainGroupInfo);
    if (hichainGroupInfo.groupType != DM_IDENTICAL_ACCOUNT) {
        LOGI("groupType is %{public}d, not idential account.", hichainGroupInfo.groupType);
        return;
    }
    DeviceManagerService::GetInstance().HandleDataChange(peerUdid, hichainGroupInfo);
}

} // namespace DistributedHardware
} // namespace OHOS