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


#include "multiple_user_connector.h"
#include "dm_log.h"
#include "dm_container.h"

int32_t DmShellGetCurrentAccountUserId(void)
{
    return 0;
}

int32_t DmShellQueryActiveOsAccountIds(int32_t* userId)
{
    if (userId == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    *userId = 0;
    return 0;
}

DmString DmShellGetOhosAccountId(void)
{
    return DmStringCreateEmpty();
}

DmString DmShellGetOhosAccountIdByUserId(int32_t userId)
{
    (void)userId;
    return DmStringCreateEmpty();
}

DmString DmShellGetOhosAccountNameByUserId(int32_t userId)
{
    (void)userId;
    return DmStringCreateEmpty();
}

DmString DmShellGetOhosAccountName(void)
{
    return DmStringCreateEmpty();
}

void DmShellGetCallingTokenId(uint32_t* tokenId)
{
    if (tokenId != NULL) {
        *tokenId = 0;
    }
}

void DmShellGetCallerUserId(int32_t* userId)
{
    if (userId != NULL) {
        *userId = 0;
    }
}

int32_t DmShellGetBackgroundUserIds(DmVec_int* userIdVec)
{
    if (userIdVec == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return 0;
}

int32_t DmShellGetAllUserIds(DmVec_int* userIdVec)
{
    if (userIdVec == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    (void)DmVec_int_Push(userIdVec, 0);
    return 0;
}

DmString DmShellGetAccountNickName(int32_t userId)
{
    (void)userId;
    return DmStringCreateEmpty();
}

bool DmShellIsUserUnlocked(int32_t userId)
{
    (void)userId;
    return true;
}

void DmShellGetForegroundOsAccountLocalIdByDisplayId(int32_t displayId, int32_t* userId)
{
    (void)displayId;
    if (userId != NULL) {
        *userId = 0;
    }
}

int32_t DmShellCheckOsAccountConstraintEnabled(int32_t userId, const char* constraint, bool* isEnabled)
{
    (void)userId;
    (void)constraint;
    if (isEnabled == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    *isEnabled = false;
    return 0;
}
