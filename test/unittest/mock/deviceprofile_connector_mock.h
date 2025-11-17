/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICEPROFILE_CONNECTOR_MOCK_H
#define OHOS_DEVICEPROFILE_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "deviceprofile_connector.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceProfileConnector {
public:
    virtual ~DmDeviceProfileConnector() = default;
public:
    virtual std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAccessControlProfile() = 0;
    virtual int32_t HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        const std::string &localUdid, DmOfflineParam &offlineParam, int32_t tokenId) = 0;
    virtual int32_t HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid, const std::string &localUdid) = 0;
    virtual uint32_t CheckBindType(std::string trustDeviceId, std::string requestDeviceId) = 0;
    virtual std::map<int32_t, int32_t> GetUserIdAndBindLevel(const std::string &localUdid,
        const std::string &peerUdid) = 0;
    virtual bool DeleteAclForAccountLogOut(const DMAclQuadInfo &info, const std::string &accountId,
        DmOfflineParam &offlineParam) = 0;
    virtual DmOfflineParam HandleAppUnBindEvent(int32_t remoteUserId,
        const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid) = 0;
    virtual std::multimap<std::string, int32_t> GetDevIdAndUserIdByActHash(const std::string &localUdid,
        const std::string &peerUdid, int32_t peerUserId, const std::string &peerAccountHash) = 0;
    virtual std::vector<int32_t> SyncAclByBindType(std::string pkgName, std::vector<int32_t> bindTypeVec,
        std::string localDeviceId, std::string targetDeviceId) = 0;
    virtual std::vector<int32_t> GetBindTypeByPkgName(std::string pkgName, std::string requestDeviceId,
        std::string trustUdid) = 0;
    virtual DmOfflineParam DeleteAccessControlList(const std::string &pkgName, const std::string &localDeviceId,
        const std::string &remoteDeviceId, int32_t bindLevel, const std::string &extra) = 0;
    virtual void DeleteAclForRemoteUserRemoved(std::string peerUdid, int32_t peerUserId,
        std::vector<int32_t> &userIds, DmOfflineParam &offlineParam) = 0;
    virtual DmOfflineParam HandleAppUnBindEvent(int32_t remoteUserId,
        const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid, int32_t peerTokenId) = 0;
    virtual std::unordered_map<std::string, DmAuthForm> GetAppTrustDeviceList(const std::string &pkgName,
        const std::string &deviceId) = 0;
    virtual int32_t CheckDeviceInfoPermission(const std::string &localUdid, const std::string &peerDeviceId) = 0;
    virtual int32_t GetLocalServiceInfoByBundleNameAndPinExchangeType(const std::string& bundleName,
        int32_t pinExchangeType, DistributedDeviceProfile::LocalServiceInfo &serviceInfo) = 0;
    virtual int32_t PutSessionKey(int32_t userId, const std::vector<unsigned char> &sessionKeyArray,
        int32_t &sessionKeyId) = 0;
    virtual int32_t PutLocalServiceInfo(const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo) = 0;
    virtual int32_t DeleteLocalServiceInfo(const std::string &bundleName, int32_t pinExchangeType) = 0;
    virtual int32_t UpdateLocalServiceInfo(const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo) = 0;
    virtual bool CheckAclStatusAndForegroundNotMatch(const std::string &localUdid,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds) = 0;
    virtual std::map<std::string, int32_t> GetDeviceIdAndBindLevel(std::vector<int32_t> userIds,
        const std::string &localUdid) = 0;
    virtual void DeleteAccessControlById(int64_t& accessControlId) = 0;
    virtual std::vector<ProcessInfo> GetProcessInfoFromAclByUserId(const std::string &localDeviceId,
        const std::string &targetDeviceId, int32_t userId) = 0;
    virtual std::vector<DistributedDeviceProfile::AccessControlProfile> GetAccessControlProfile() = 0;
    virtual std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAclIncludeLnnAcl() = 0;
    virtual std::vector<std::string> GetDeviceIdAndUdidListByTokenId(const std::vector<int32_t> &userIds,
        const std::string &localUdid, int32_t tokenId) = 0;
    virtual void CacheAcerAclId(const DistributedDeviceProfile::AccessControlProfile &profile,
        std::vector<DmAclIdParam> &aclInfos) = 0;
    virtual bool IsLnnAcl(const DistributedDeviceProfile::AccessControlProfile &profile) = 0;
    virtual int32_t GetServiceInfoProfileByServiceId(int64_t serviceId, ServiceInfoProfile &profile) = 0;
    virtual int32_t DeleteServiceInfoProfile(int32_t regServiceId, int32_t userId) = 0;
    virtual int32_t PutServiceInfoProfile(const ServiceInfoProfile &profile) = 0;
    virtual int32_t GetServiceInfoProfileByTokenId(int64_t tokenId, std::vector<ServiceInfoProfile> &serviceInfos) = 0;
    virtual int32_t GetServiceInfoProfileByRegServiceId(int32_t regServiceId, ServiceInfoProfile &profile) = 0;
public:
    static inline std::shared_ptr<DmDeviceProfileConnector> dmDeviceProfileConnector = nullptr;
};

class DeviceProfileConnectorMock : public DmDeviceProfileConnector {
public:
    MOCK_METHOD(std::vector<DistributedDeviceProfile::AccessControlProfile>, GetAllAccessControlProfile, ());
    MOCK_METHOD(int32_t, HandleDevUnBindEvent, (int32_t, const std::string &, const std::string &, DmOfflineParam &,
        int32_t));
    MOCK_METHOD(int32_t, HandleAccountLogoutEvent, (int32_t, const std::string &, const std::string &,
        const std::string &));
    MOCK_METHOD(uint32_t, CheckBindType, (std::string, std::string));
    MOCK_METHOD((std::map<int32_t, int32_t>), GetUserIdAndBindLevel, (const std::string &, const std::string &));
    MOCK_METHOD(bool, DeleteAclForAccountLogOut, (const DMAclQuadInfo &, const std::string &,
        DmOfflineParam &));
    MOCK_METHOD((DmOfflineParam), HandleAppUnBindEvent, (int32_t, const std::string &,
        int32_t, const std::string &));
    MOCK_METHOD((std::multimap<std::string, int32_t>), GetDevIdAndUserIdByActHash, (const std::string &,
        const std::string &, int32_t, const std::string &));
    MOCK_METHOD((std::vector<int32_t>), SyncAclByBindType, (std::string, std::vector<int32_t>, std::string,
        std::string));
    MOCK_METHOD((std::vector<int32_t>), GetBindTypeByPkgName, (std::string, std::string, std::string));
    MOCK_METHOD(DmOfflineParam, DeleteAccessControlList, (const std::string &, const std::string &, const std::string &,
        int32_t, const std::string &));
    MOCK_METHOD(void, DeleteAclForRemoteUserRemoved, (std::string, int32_t, std::vector<int32_t> &, DmOfflineParam &));
    MOCK_METHOD((DmOfflineParam), HandleAppUnBindEvent, (int32_t, const std::string &,
        int32_t, const std::string &, int32_t));
    MOCK_METHOD((std::unordered_map<std::string, DmAuthForm>), GetAppTrustDeviceList,
        (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, CheckDeviceInfoPermission, (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, GetLocalServiceInfoByBundleNameAndPinExchangeType, (const std::string&, int32_t,
        DistributedDeviceProfile::LocalServiceInfo &serviceInfo));
    MOCK_METHOD(int32_t, PutSessionKey, (int32_t, const std::vector<unsigned char> &, int32_t&));
    MOCK_METHOD(int32_t, PutLocalServiceInfo, (const DistributedDeviceProfile::LocalServiceInfo &));
    MOCK_METHOD(int32_t, DeleteLocalServiceInfo, (const std::string &, int32_t));
    MOCK_METHOD(int32_t, UpdateLocalServiceInfo, (const DistributedDeviceProfile::LocalServiceInfo &));
    MOCK_METHOD(bool, CheckAclStatusAndForegroundNotMatch,
        (const std::string &, (const std::vector<int32_t> &), (const std::vector<int32_t> &)));
    MOCK_METHOD((std::map<std::string, int32_t>), GetDeviceIdAndBindLevel,
        ((std::vector<int32_t>), const std::string &));
    MOCK_METHOD(void, DeleteAccessControlById, (int64_t&));
    MOCK_METHOD(std::vector<ProcessInfo>, GetProcessInfoFromAclByUserId,
        (const std::string &, const std::string &, int32_t));
    MOCK_METHOD(std::vector<DistributedDeviceProfile::AccessControlProfile>, GetAccessControlProfile, ());
    MOCK_METHOD(std::vector<DistributedDeviceProfile::AccessControlProfile>, GetAllAclIncludeLnnAcl, ());
    MOCK_METHOD(std::vector<std::string>, GetDeviceIdAndUdidListByTokenId, (const std::vector<int32_t> &,
        const std::string &, int32_t));
    MOCK_METHOD(void, CacheAcerAclId, (const DistributedDeviceProfile::AccessControlProfile &,
        std::vector<DmAclIdParam> &));
    MOCK_METHOD(bool, IsLnnAcl, (const DistributedDeviceProfile::AccessControlProfile &));
    MOCK_METHOD(int32_t, PutServiceInfoProfile, (const ServiceInfoProfile &));
    MOCK_METHOD(int32_t, GetServiceInfoProfileByServiceId, (int64_t, ServiceInfoProfile &));
    MOCK_METHOD(int32_t, DeleteServiceInfoProfile, (int32_t, int32_t));
    MOCK_METHOD(int32_t, GetServiceInfoProfileByTokenId, (int64_t, std::vector<ServiceInfoProfile> &));
    MOCK_METHOD(int32_t, GetServiceInfoProfileByRegServiceId, (int32_t, ServiceInfoProfile &));
};
}
}
#endif
