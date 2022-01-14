/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "profile_connector.h"

#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
std::map<std::string, std::shared_ptr<IProfileConnectorCallback>> ProfileEventCallback::profileConnectorCallback_ = {};
std::shared_ptr<ProfileEventCallback> ProfileConnector::profileEventCallback_ =
    std::make_shared<ProfileEventCallback>();

int32_t ProfileConnector::RegisterProfileCallback(const std::string &pkgName, const std::string &deviceId,
                                                  std::shared_ptr<IProfileConnectorCallback> callback)
{
    LOGI("ProfileConnector::RegisterProfileCallback");
    profileEventCallback_->RegisterProfileCallback(pkgName, callback);
    SubscribeProfileEvents({"system", "device", "fakeStorage", "fakeSystem"}, deviceId);
    return DM_OK;
}

int32_t ProfileConnector::UnRegisterProfileCallback(const std::string &pkgName)
{
    LOGI("ProfileConnector::UnRegisterProfileCallback");
    profileEventCallback_->UnRegisterProfileCallback(pkgName);
    return DM_OK;
}

int32_t ProfileConnector::SubscribeProfileEvents(const std::list<std::string> &serviceIds, const std::string &deviceId)
{
    ExtraInfo extraInfo;
    extraInfo["deviceId"] = deviceId;
    extraInfo["serviceIds"] = serviceIds;

    std::list<SubscribeInfo> subscribeInfos;

    SubscribeInfo eventSync;
    eventSync.profileEvent = ProfileEvent::EVENT_SYNC_COMPLETED;
    subscribeInfos.emplace_back(eventSync);

    std::list<ProfileEvent> failedEvents;
    u_int32_t errCode;
    errCode = DistributedDeviceProfileClient::GetInstance().SubscribeProfileEvents(subscribeInfos,
                                                                                   profileEventCallback_, failedEvents);
    LOGI("ProfileConnector::SubscribeProfileEvents result=%d", errCode);
    return DM_OK;
}

int32_t ProfileConnector::UnSubscribeProfileEvents()
{
    std::list<ProfileEvent> profileEvents;
    profileEvents.emplace_back(ProfileEvent::EVENT_PROFILE_CHANGED);
    profileEvents.emplace_back(ProfileEvent::EVENT_SYNC_COMPLETED);
    std::list<ProfileEvent> failedEvents;
    u_int32_t errCode;
    errCode = DistributedDeviceProfileClient::GetInstance().UnsubscribeProfileEvents(
        profileEvents, profileEventCallback_, failedEvents);
    LOGI("ProfileConnector::UnSubscribeProfileEvents result=%d", errCode);
    return DM_OK;
}

int32_t ProfileEventCallback::RegisterProfileCallback(const std::string &pkgName,
                                                      std::shared_ptr<IProfileConnectorCallback> callback)
{
    LOGI("ProfileEventCallback::RegisterProfileCallback");
    profileConnectorCallback_.emplace(pkgName, callback);
    return DM_OK;
}

int32_t ProfileEventCallback::UnRegisterProfileCallback(const std::string &pkgName)
{
    LOGI("ProfileEventCallback::UnRegisterProfileCallback");
    profileConnectorCallback_.erase(pkgName);
    return DM_OK;
}

void ProfileEventCallback::OnSyncCompleted(const SyncResult &syncResults)
{
    std::string deviceId;
    u_int32_t SyncStatus;
    for (auto &iterResult : syncResults) {
        deviceId = iterResult.first;
        SyncStatus = iterResult.second;
    }
    LOGI("ProfileEventCallback::OnSyncCompleted, deviceId = %s", deviceId.c_str());
    for (auto &iter : profileConnectorCallback_) {
        iter.second->OnProfileComplete(iter.first, deviceId);
    }
}
} // namespace DistributedHardware
} // namespace OHOS