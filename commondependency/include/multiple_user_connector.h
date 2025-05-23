/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_MULTIPLE_USER_CONNECTOR_H
#define OHOS_DM_MULTIPLE_USER_CONNECTOR_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {
typedef struct DMAccountInfo {
    std::string accountId;
    std::string accountName;
} DMAccountInfo;
class MultipleUserConnector {
public:
    /**
     * @tc.name: MultipleUserConnector::GetCurrentAccountUserID
     * @tc.desc: Get Current Account UserID of the Multiple User Connector
     * @tc.type: FUNC
     */
    DM_EXPORT static int32_t GetCurrentAccountUserID(void);

    /**
     * @tc.name: MultipleUserConnector::SetSwitchOldUserId
     * @tc.desc: Set Switch Old UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    DM_EXPORT static void SetSwitchOldUserId(int32_t userId);

    /**
     * @tc.name: MultipleUserConnector::GetSwitchOldUserId
     * @tc.desc: Get Switc hOld UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static int32_t GetSwitchOldUserId(void);

    /**
     * @tc.name: MultipleUserConnector::GetOhosAccountId
     * @tc.desc: Get Current AccountId of current user
     * @tc.type: FUNC
     */
    DM_EXPORT static std::string GetOhosAccountId(void);

    /**
     * @brief Get the Ohos Account Id By Userid
     *
     * @param userId the user id in which account login
     * @return std::string the account id
     */
    DM_EXPORT static std::string GetOhosAccountIdByUserId(int32_t userId);

    /**
     * @brief Get the Ohos Account Name By Userid
     *
     * @param userId the user id in which account login
     * @return std::string the account name
     */
    DM_EXPORT static std::string GetOhosAccountNameByUserId(int32_t userId);

    /**
     * @tc.name: MultipleUserConnector::SetSwitchOldAccountId
     * @tc.desc: Set Switch Old UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    DM_EXPORT static void SetSwitchOldAccountId(std::string accountId);

    /**
     * @tc.name: MultipleUserConnector::GetSwitchOldAccountId
     * @tc.desc: Get Switc hOld UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static std::string GetSwitchOldAccountId(void);

    /**
     * @tc.name: MultipleUserConnector::SetSwitchOldAccountName
     * @tc.desc: Set Switch Old AccountName of the Multiple User Connector
     * @tc.type: FUNC
     */
    static void SetSwitchOldAccountName(std::string accountName);

    /**
     * @tc.name: MultipleUserConnector::GetOhosAccountName
     * @tc.desc: Get Current AccountName of the Multiple User Connector
     * @tc.type: FUNC
     */
    DM_EXPORT static std::string GetOhosAccountName(void);

    /**
     * @tc.name: MultipleUserConnector::GetSwitchOldAccountName
     * @tc.desc: Get Switc Old AccountName of the Multiple User Connector
     * @tc.type: FUNC
     */
    static std::string GetSwitchOldAccountName(void);
    DM_EXPORT static void SetAccountInfo(int32_t userId, DMAccountInfo dmAccountInfo);
    DM_EXPORT static DMAccountInfo GetAccountInfoByUserId(int32_t userId);
    DM_EXPORT static void DeleteAccountInfoByUserId(int32_t userId);
    DM_EXPORT static void GetTokenId(uint32_t &tokenId);
    DM_EXPORT static void GetTokenIdAndForegroundUserId(uint32_t &tokenId,
        int32_t &userId);
    DM_EXPORT static void GetCallerUserId(int32_t &userId);
    DM_EXPORT static int32_t GetForegroundUserIds(std::vector<int32_t> &userVec);
    DM_EXPORT static int32_t GetFirstForegroundUserId(void);
    DM_EXPORT static int32_t GetBackgroundUserIds(std::vector<int32_t> &userIdVec);
    static int32_t GetAllUserIds(std::vector<int32_t> &userIdVec);
    DM_EXPORT static std::string GetAccountNickName(int32_t userId);
    DM_EXPORT static bool IsUserUnlocked(int32_t userId);
    DM_EXPORT static void ClearLockedUser(std::vector<int32_t> &foregroundUserVec);
    DM_EXPORT static void ClearLockedUser(std::vector<int32_t> &foregroundUserVec,
        std::vector<int32_t> &backgroundUserVec);
    DM_EXPORT static DMAccountInfo GetCurrentDMAccountInfo();
    DM_EXPORT static void GetCallingTokenId(uint32_t &tokenId);
    DM_EXPORT static int32_t GetUserIdByDisplayId(uint64_t displayId);
private:
    static int32_t oldUserId_;
    static std::string accountId_;
    static std::string accountName_;
    static std::mutex lock_;
    static std::map<int32_t, DMAccountInfo> dmAccountInfoMap_;
    static std::mutex dmAccountInfoMaplock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_MULTIPLE_USER_CONNECTOR_H
