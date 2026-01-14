/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_HICHAIN_CONNECTOR_MOCK_H
#define OHOS_HICHAIN_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmHiChainConnector {
public:
    virtual ~DmHiChainConnector() = default;
public:
    virtual bool GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList) = 0;
    virtual bool IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice) = 0;
    virtual int32_t GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList) = 0;
    virtual int32_t GetRelatedGroups(int32_t userId, const std::string &deviceId,
        std::vector<GroupInfo> &groupList) = 0;
    virtual int32_t DeleteGroupByACL(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec,
        std::vector<int32_t> &userIdVec) = 0;
    virtual int32_t DeleteTimeOutGroup(const std::string &peerUdid, int32_t userId) = 0;
    virtual int32_t GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList) = 0;
    virtual int32_t DeleteGroupExt(int32_t userId, const std::string &groupId) = 0;
public:
    static inline std::shared_ptr<DmHiChainConnector> dmHiChainConnector = nullptr;
};

class HiChainConnectorMock : public DmHiChainConnector {
public:
    MOCK_METHOD(bool, GetGroupInfo, (const int32_t, const std::string &, std::vector<GroupInfo> &));
    MOCK_METHOD(bool, IsDevicesInP2PGroup, (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, GetRelatedGroups, (const std::string &, std::vector<GroupInfo> &));
    MOCK_METHOD(int32_t, GetRelatedGroups, (int32_t, const std::string &, std::vector<GroupInfo> &));
    MOCK_METHOD(int32_t, DeleteGroupByACL, ((std::vector<std::pair<int32_t, std::string>> &),
        (std::vector<int32_t> &)));
    MOCK_METHOD(int32_t, DeleteTimeOutGroup, (const std::string &, int32_t));
    MOCK_METHOD(int32_t, GetRelatedGroupsExt, (const std::string &, std::vector<GroupInfo> &));
    MOCK_METHOD(int32_t, DeleteGroupExt, (int32_t, const std::string &));
};
}
}
#endif
