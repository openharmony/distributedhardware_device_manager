/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "device_auth.h"
#include "dm_constants.h"

namespace {
int32_t (*deleteGroup)(int64_t requestId, const char *appId, const char *disbandParams)
{
    if (disbandParams == 0) {
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

int32_t (*getRelatedGroups)(const char *appId, const char *peerDeviceId, char **returnGroupVec, uint32_t *groupNum)
{
    (void)appId;
    (void)peerDeviceId;
    (void)returnGroupVec;
    (void)groupNum;
    return 0;
}

int32_t (*createGroup)(int64_t requestId, const char *appId, const char *createParams)
{
    if (requestId == 0) {
        return ERR_DM_FAILED;
    }
    (void)appId;
    (void)createParams;
    return 0;
}

DeviceGroupManager g_deviceGroupManager = {
    .regCallback = nullptr,
    .unRegCallback = nullptr,
    .regDataChangeListener = nullptr,
    .unRegDataChangeListener = nullptr,
    .createGroup = createGroup,
    .deleteGroup = deleteGroup,
    .addMemberToGroup = nullptr,
    .deleteMemberFromGroup = nullptr,
    .processData = nullptr,
    .addMultiMembersToGroup = nullptr,
    .delMultiMembersFromGroup = nullptr,
    .confirmRequest = nullptr,
    .bindPeer = nullptr,
    .unbindPeer = nullptr,
    .processLiteData = nullptr,
    .authKeyAgree = nullptr,
    .processKeyAgreeData = nullptr,
    .processCredential = nullptr,
    .getRegisterInfo = nullptr,
    .getLocalConnectInfo = nullptr,
    .checkAccessToGroup = nullptr,
    .getPkInfoList = nullptr,
    .addGroupManager = nullptr,
    .addGroupFriend = nullptr,
    .deleteGroupManager = nullptr,
    .deleteGroupFriend = nullptr,
    .getGroupManagers = nullptr,
    .getGroupFriends = nullptr,
    .getGroupInfoById = nullptr,
    .getGroupInfo = nullptr,
    .getJoinedGroups = nullptr,
    .getRelatedGroups = getRelatedGroups,
    .getDeviceInfoById = nullptr,
    .getTrustedDevices = nullptr,
    .isDeviceInGroup = nullptr,
    .destroyInfo = nullptr
};

const DeviceGroupManager* GetGmInstance(void)
{
    return &g_deviceGroupManager;
}
}