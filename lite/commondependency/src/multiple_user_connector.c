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


#include "multiple_user_connector.h"
#include "dm_error_type.h"
#include "dm_log.h"

DM_MAP_IMPL(int_DmAccountInfo, int, DmAccountInfo, DmCmpInt)
#include "dm_constants.h"
#include "dm_shell_functions.h"
#include <unistd.h>
#include <time.h>

static int32_t g_dm_old_user_id = -1;
static DmString g_dm_account_id;
static DmString g_dm_account_name;
static DmMutex g_dm_lock = { .mtx = PTHREAD_MUTEX_INITIALIZER };
static DmMap_int_DmAccountInfo g_dm_account_info_map;
static DmMutex g_dm_account_info_map_lock = { .mtx = PTHREAD_MUTEX_INITIALIZER };
static DmMutex g_dm_current_foreground_user_id_lock = { .mtx = PTHREAD_MUTEX_INITIALIZER };
static int32_t g_dm_current_foreground_user_id = -1;

static const char* DM_MDM_CONSTRAINT = "constraint.distributed.transmission.outgoing";
static const int32_t DM_GET_USERID_MAX_NUM = 3;
static const int32_t DM_USLEEP_TIME_US_50000 = 50000;

void DmAccountInfoInit(DmAccountInfo* info)
{
    info->accountId = DmStringCreateEmpty();
    info->accountName = DmStringCreateEmpty();
}

void DmAccountInfoDestroy(DmAccountInfo* info)
{
    DmStringDestroy(&info->accountId);
    DmStringDestroy(&info->accountName);
}

int32_t DmMultipleUserGetCurrentAccountUserId(void)
{
    return DmShellGetCurrentAccountUserId();
}

int32_t DmMultipleUserTryGetCurrentAccountUserId(void)
{
    int32_t retryNum = 0;
    int32_t ret = 0;
    int32_t userId = -1;
    while (retryNum < DM_GET_USERID_MAX_NUM) {
        ret = DmShellQueryActiveOsAccountIds(&userId);
        if (ret == 0) {
            break;
        }
        struct timespec ts = {0, DM_USLEEP_TIME_US_50000 * 1000};
        nanosleep(&ts, NULL);
        retryNum++;
    }
    if (ret != 0) {
        LOGE("error ret: %{public}d", ret);
        return -1;
    }
    return userId;
}

bool DmMultipleUserCheckMdmControl(void)
{
    bool isMDMControl = false;
    int32_t activeAccountId = DmMultipleUserGetCurrentAccountUserId();
    int32_t ret = DmShellCheckOsAccountConstraintEnabled(activeAccountId,
        DM_MDM_CONSTRAINT, &isMDMControl);
    if (ret != 0) {
        LOGE("CheckOsAccountConstraintEnabled failed, ret %{public}d.", ret);
        return false;
    }
    LOGI("CheckOsAccountConstraintEnabled success, isMDMControl %{public}d.", isMDMControl);
    return isMDMControl;
}

DmString DmMultipleUserGetOhosAccountId(void)
{
    DmString result;
    result = DmStringCreateEmpty();
    DmString shellResult = DmShellGetOhosAccountId();
    if (!DmStringEmpty(&shellResult)) {
        DmStringDestroy(&result);
        result = shellResult;
    } else {
        DmStringDestroy(&shellResult);
    }
    return result;
}

DmString DmMultipleUserGetOhosAccountIdByUserId(int32_t userId)
{
    DmString result;
    result = DmStringCreateEmpty();
    DmString shellResult = DmShellGetOhosAccountIdByUserId(userId);
    if (!DmStringEmpty(&shellResult)) {
        DmStringDestroy(&result);
        result = shellResult;
    } else {
        DmStringDestroy(&shellResult);
    }
    return result;
}

DmString DmMultipleUserGetOhosAccountNameByUserId(int32_t userId)
{
    DmString result;
    result = DmStringCreateEmpty();
    DmString shellResult = DmShellGetOhosAccountNameByUserId(userId);
    if (!DmStringEmpty(&shellResult)) {
        DmStringDestroy(&result);
        result = shellResult;
    } else {
        DmStringDestroy(&shellResult);
    }
    return result;
}

DmString DmMultipleUserGetOhosAccountName(void)
{
    DmString result;
    result = DmStringCreateEmpty();
    DmString shellResult = DmShellGetOhosAccountName();
    if (!DmStringEmpty(&shellResult)) {
        DmStringDestroy(&result);
        result = shellResult;
    } else {
        DmStringDestroy(&shellResult);
    }
    return result;
}

void DmMultipleUserGetTokenIdAndForegroundUserId(uint32_t* tokenId, int32_t* userId)
{
    DmShellGetCallingTokenId(tokenId);
    *userId = DmMultipleUserGetFirstForegroundUserId();
}

void DmMultipleUserGetTokenId(uint32_t* tokenId)
{
    DmShellGetCallingTokenId(tokenId);
}

void DmMultipleUserGetCallerUserId(int32_t* userId)
{
    DmShellGetCallerUserId(userId);
}

void DmMultipleUserSetSwitchOldUserId(int32_t userId)
{
    DmMutexLock(&g_dm_lock);
    g_dm_old_user_id = userId;
    DmMutexUnlock(&g_dm_lock);
}

int32_t DmMultipleUserGetSwitchOldUserId(void)
{
    DmMutexLock(&g_dm_lock);
    int32_t result = g_dm_old_user_id;
    DmMutexUnlock(&g_dm_lock);
    return result;
}

void DmMultipleUserSetSwitchOldAccountId(DmString accountId)
{
    DmMutexLock(&g_dm_lock);
    DmStringDestroy(&g_dm_account_id);
    g_dm_account_id = accountId;
    DmMutexUnlock(&g_dm_lock);
}

DmString DmMultipleUserGetSwitchOldAccountId(void)
{
    DmMutexLock(&g_dm_lock);
    DmString result = DmStringCopy(&g_dm_account_id);
    DmMutexUnlock(&g_dm_lock);
    return result;
}

void DmMultipleUserSetSwitchOldAccountName(DmString accountName)
{
    DmMutexLock(&g_dm_lock);
    DmStringDestroy(&g_dm_account_name);
    g_dm_account_name = accountName;
    DmMutexUnlock(&g_dm_lock);
}

DmString DmMultipleUserGetSwitchOldAccountName(void)
{
    DmMutexLock(&g_dm_lock);
    DmString result = DmStringCopy(&g_dm_account_name);
    DmMutexUnlock(&g_dm_lock);
    return result;
}

void DmMultipleUserSetAccountInfo(int32_t userId, DmAccountInfo dmAccountInfo)
{
    DmMutexLock(&g_dm_account_info_map_lock);
    DmMap_int_DmAccountInfo_Insert(&g_dm_account_info_map, userId, dmAccountInfo);
    DmMutexUnlock(&g_dm_account_info_map_lock);
}

DmAccountInfo DmMultipleUserGetAccountInfoByUserId(int32_t userId)
{
    DmAccountInfo dmAccountInfo;
    DmAccountInfoInit(&dmAccountInfo);
    DmMutexLock(&g_dm_account_info_map_lock);
    DmAccountInfo* found = DmMap_int_DmAccountInfo_Find(&g_dm_account_info_map, userId);
    if (found != NULL) {
        dmAccountInfo.accountId = DmStringCopy(&found->accountId);
        dmAccountInfo.accountName = DmStringCopy(&found->accountName);
    } else {
        LOGE("userId is not exist.");
    }
    DmMutexUnlock(&g_dm_account_info_map_lock);
    return dmAccountInfo;
}

void DmMultipleUserDeleteAccountInfoByUserId(int32_t userId)
{
    DmMutexLock(&g_dm_account_info_map_lock);
    LOGI("userId: %{public}d", userId);
    DmMap_int_DmAccountInfo_Erase(&g_dm_account_info_map, userId);
    DmMutexUnlock(&g_dm_account_info_map_lock);
}

int32_t DmMultipleUserGetForegroundUserIds(DmVec_int* userVec)
{
    return DmShellGetForegroundUserIds(userVec);
}

int32_t DmMultipleUserGetFirstForegroundUserId(void)
{
    DmVec_int userVec;
    DmVec_int_Init(&userVec);
    int32_t ret = DmMultipleUserGetForegroundUserIds(&userVec);
    if (ret != 0 || DmVec_int_Size(&userVec) == 0) {
        LOGE("get userid error ret: %{public}d.", ret);
        DmVec_int_Destroy(&userVec);
        return -1;
    }
    int32_t result = *DmVec_int_At(&userVec, 0);
    DmVec_int_Destroy(&userVec);
    return result;
}

int32_t DmMultipleUserGetBackgroundUserIds(DmVec_int* userIdVec)
{
    return DmShellGetBackgroundUserIds(userIdVec);
}

int32_t DmMultipleUserGetAllUserIds(DmVec_int* userIdVec)
{
    return DmShellGetAllUserIds(userIdVec);
}

DmString DmMultipleUserGetAccountNickName(int32_t userId)
{
    return DmShellGetAccountNickName(userId);
}

bool DmMultipleUserIsUserUnlocked(int32_t userId)
{
    return DmShellIsUserUnlocked(userId);
}

void DmMultipleUserClearLockedUser(DmVec_int* foregroundUserVec)
{
    for (int i = 0; i < DmVec_int_Size(foregroundUserVec); i++) {
        int32_t* val = DmVec_int_At(foregroundUserVec, i);
        if (val != NULL && !DmMultipleUserIsUserUnlocked(*val)) {
            DmVec_int_Erase(foregroundUserVec, i);
            i--;
        }
    }
}

void DmMultipleUserClearLockedUserBoth(DmVec_int* foregroundUserVec, DmVec_int* backgroundUserVec)
{
    for (int i = 0; i < DmVec_int_Size(foregroundUserVec); i++) {
        int32_t* val = DmVec_int_At(foregroundUserVec, i);
        if (val != NULL && !DmMultipleUserIsUserUnlocked(*val)) {
            DmVec_int_Push(backgroundUserVec, *val);
            DmVec_int_Erase(foregroundUserVec, i);
            i--;
        }
    }
}

DmAccountInfo DmMultipleUserGetCurrentDmAccountInfo(void)
{
    DmAccountInfo dmAccountInfo;
    DmAccountInfoInit(&dmAccountInfo);
    dmAccountInfo.accountId = DmMultipleUserGetOhosAccountId();
    dmAccountInfo.accountName = DmMultipleUserGetOhosAccountName();
    return dmAccountInfo;
}

void DmMultipleUserGetCallingTokenId(uint32_t* tokenId)
{
    DmShellGetCallingTokenId(tokenId);
}

int32_t DmMultipleUserGetUserIdByDisplayId(int32_t displayId)
{
    LOGI("displayId %{public}d", displayId);
    int32_t userId = -1;
    if (displayId == -1) {
        userId = DmMultipleUserGetFirstForegroundUserId();
        return userId;
    }
    DmShellGetForegroundOsAccountLocalIdByDisplayId(displayId, &userId);
    return userId;
}

void DmMultipleUserUpdateForegroundUserId(void)
{
    int32_t userId = DmMultipleUserGetCurrentAccountUserId();
    DmMutexLock(&g_dm_current_foreground_user_id_lock);
    g_dm_current_foreground_user_id = userId;
    DmMutexUnlock(&g_dm_current_foreground_user_id_lock);
}

int32_t DmMultipleUserGetForegroundUserId(void)
{
    int32_t userId = -1;
    DmMutexLock(&g_dm_current_foreground_user_id_lock);
    userId = g_dm_current_foreground_user_id;
    DmMutexUnlock(&g_dm_current_foreground_user_id_lock);
    if (userId == -1) {
        userId = DmShellGetCurrentAccountUserId();
        if (userId != -1) {
            DmMutexLock(&g_dm_current_foreground_user_id_lock);
            g_dm_current_foreground_user_id = userId;
            DmMutexUnlock(&g_dm_current_foreground_user_id_lock);
        }
    }
    return userId;
}
