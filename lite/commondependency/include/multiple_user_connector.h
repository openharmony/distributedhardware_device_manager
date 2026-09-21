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


#ifndef DM_MULTIPLE_USER_CONNECTOR_H
#define DM_MULTIPLE_USER_CONNECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"
#include "dm_thread.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef struct {
    DmString accountId;
    DmString accountName;
} DmAccountInfo;

void DmAccountInfoInit(DmAccountInfo* info);
void DmAccountInfoDestroy(DmAccountInfo* info);

DM_MAP_DEFINE(int_DmAccountInfo, int, DmAccountInfo);

DM_EXPORT int32_t DmMultipleUserGetCurrentAccountUserId(void);
DM_EXPORT void DmMultipleUserSetSwitchOldUserId(int32_t userId);
int32_t DmMultipleUserGetSwitchOldUserId(void);
DM_EXPORT DmString DmMultipleUserGetOhosAccountId(void);
DM_EXPORT DmString DmMultipleUserGetOhosAccountIdByUserId(int32_t userId);
DM_EXPORT DmString DmMultipleUserGetOhosAccountNameByUserId(int32_t userId);
DM_EXPORT void DmMultipleUserSetSwitchOldAccountId(DmString accountId);
DmString DmMultipleUserGetSwitchOldAccountId(void);
void DmMultipleUserSetSwitchOldAccountName(DmString accountName);
DmString DmMultipleUserGetSwitchOldAccountName(void);
DM_EXPORT DmString DmMultipleUserGetOhosAccountName(void);
DM_EXPORT void DmMultipleUserSetAccountInfo(int32_t userId, DmAccountInfo dmAccountInfo);
DM_EXPORT DmAccountInfo DmMultipleUserGetAccountInfoByUserId(int32_t userId);
DM_EXPORT void DmMultipleUserDeleteAccountInfoByUserId(int32_t userId);
DM_EXPORT void DmMultipleUserGetTokenId(uint32_t* tokenId);
DM_EXPORT void DmMultipleUserGetTokenIdAndForegroundUserId(uint32_t* tokenId, int32_t* userId);
DM_EXPORT void DmMultipleUserGetCallerUserId(int32_t* userId);
DM_EXPORT int32_t DmMultipleUserGetForegroundUserIds(DmVec_int* userVec);
DM_EXPORT int32_t DmMultipleUserGetFirstForegroundUserId(void);
DM_EXPORT int32_t DmMultipleUserGetBackgroundUserIds(DmVec_int* userIdVec);
int32_t DmMultipleUserGetAllUserIds(DmVec_int* userIdVec);
DM_EXPORT DmString DmMultipleUserGetAccountNickName(int32_t userId);
DM_EXPORT bool DmMultipleUserIsUserUnlocked(int32_t userId);
DM_EXPORT void DmMultipleUserClearLockedUser(DmVec_int* foregroundUserVec);
DM_EXPORT void DmMultipleUserClearLockedUserBoth(DmVec_int* foregroundUserVec, DmVec_int* backgroundUserVec);
DM_EXPORT DmAccountInfo DmMultipleUserGetCurrentDmAccountInfo(void);
DM_EXPORT void DmMultipleUserGetCallingTokenId(uint32_t* tokenId);
DM_EXPORT int32_t DmMultipleUserGetUserIdByDisplayId(int32_t displayId);
DM_EXPORT bool DmMultipleUserCheckMdmControl(void);
DM_EXPORT void DmMultipleUserUpdateForegroundUserId(void);
DM_EXPORT int32_t DmMultipleUserGetForegroundUserId(void);
DM_EXPORT int32_t DmMultipleUserTryGetCurrentAccountUserId(void);

#endif
