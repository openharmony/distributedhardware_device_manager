/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <fuzzer/FuzzedDataProvider.h>
#include <random>
#include <string>

#include "deviceprofile_connector.h"
#include "device_profile_connector_fuzzer.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 256;

void DeleteAclForAccountLogOutFuzzTest(FuzzedDataProvider &fdp)
{
    DMAclQuadInfo info;
    info.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.localUserId = fdp.ConsumeIntegral<int32_t>();
    info.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.peerUserId = fdp.ConsumeIntegral<int32_t>();

    std::string accountId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(info, accountId, offlineParam, serviceInfos);
}

void DeleteAclByActhashFuzzTest(FuzzedDataProvider &fdp)
{
    DMAclQuadInfo info;
    info.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.localUserId = fdp.ConsumeIntegral<int32_t>();
    info.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.peerUserId = fdp.ConsumeIntegral<int32_t>();

    std::string accountIdHash = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().DeleteAclByActhash(info, accountIdHash, offlineParam, serviceInfos);
}

void CacheOfflineParamFuzzTest(FuzzedDataProvider &fdp)
{
    DmCacheOfflineInputParam inputParam;
    inputParam.info.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    inputParam.info.localUserId = fdp.ConsumeIntegral<int32_t>();
    inputParam.info.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    inputParam.info.peerUserId = fdp.ConsumeIntegral<int32_t>();
    inputParam.accountIdHash = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    inputParam.profile.status = fdp.ConsumeIntegral<int32_t>();
    inputParam.profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    inputParam.profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    inputParam.profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    inputParam.profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    DmOfflineParam offlineParam;
    bool notifyOffline = fdp.ConsumeBool();
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().CacheOfflineParam(inputParam, offlineParam, notifyOffline, serviceInfos);
}

void ProcessLocalToPeerFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.status = fdp.ConsumeIntegral<int32_t>();
    profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    DMAclQuadInfo info;
    info.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.localUserId = fdp.ConsumeIntegral<int32_t>();
    info.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.peerUserId = fdp.ConsumeIntegral<int32_t>();

    std::string accountIdHash = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmOfflineParam offlineParam;
    bool notifyOffline = fdp.ConsumeBool();
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().ProcessLocalToPeer(profile, info,
            accountIdHash, offlineParam, notifyOffline, serviceInfos);
}

void ProcessPeerToLocalFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.status = fdp.ConsumeIntegral<int32_t>();
    profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    DMAclQuadInfo info;
    info.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.localUserId = fdp.ConsumeIntegral<int32_t>();
    info.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.peerUserId = fdp.ConsumeIntegral<int32_t>();

    std::string accountIdHash = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmOfflineParam offlineParam;
    bool notifyOffline = fdp.ConsumeBool();
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().ProcessPeerToLocal(profile, info,
            accountIdHash, offlineParam, notifyOffline, serviceInfos);
}

void DeleteAclForUserRemovedFuzzTest(FuzzedDataProvider &fdp)
{
    DmLocalUserRemovedInfo userRemovedInfo;
    userRemovedInfo.localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    userRemovedInfo.preUserId = fdp.ConsumeIntegral<int32_t>();

    size_t peerUdidCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < peerUdidCount; ++i) {
        userRemovedInfo.peerUdids.push_back(fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH));
    }

    std::multimap<std::string, int32_t> peerUserIdMap;
    size_t mapCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < mapCount; ++i) {
        peerUserIdMap.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH), fdp.ConsumeIntegral<int32_t>()});
    }

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(userRemovedInfo,
            peerUserIdMap, offlineParam, serviceInfos);
}

void DeleteAclForRemoteUserRemovedFuzzTest(FuzzedDataProvider &fdp)
{
    DmRemoteUserRemovedInfo userRemovedInfo;
    userRemovedInfo.peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    userRemovedInfo.peerUserId = fdp.ConsumeIntegral<int32_t>();

    size_t localUserIdCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < localUserIdCount; ++i) {
        userRemovedInfo.localUserIds.push_back(fdp.ConsumeIntegral<int32_t>());
    }

    DmOfflineParam offlineParam;
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().DeleteAclForRemoteUserRemoved(userRemovedInfo, offlineParam, serviceInfos);
}

void DeleteAccessControlListByUdidFuzzTest(FuzzedDataProvider &fdp)
{
    std::string udid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(udid);
}

void HandleSyncForegroundUserIdEventFuzzTest(FuzzedDataProvider &fdp)
{
    std::vector<int32_t> remoteUserIds;
    size_t remoteUserIdCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < remoteUserIdCount; ++i) {
        remoteUserIds.push_back(fdp.ConsumeIntegral<int32_t>());
    }

    std::string remoteUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::vector<int32_t> localUserIds;
    size_t localUserIdCount = fdp.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < localUserIdCount; ++i) {
        localUserIds.push_back(fdp.ConsumeIntegral<int32_t>());
    }

    std::string localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<DmUserRemovedServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(remoteUserIds,
            remoteUdid, localUserIds, localUdid, serviceInfos);
}

void FillDmUserRemovedServiceInfoLocalFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.status = fdp.ConsumeIntegral<int32_t>();
    profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().FillDmUserRemovedServiceInfoLocal(profile, serviceInfos);
}

void FillDmUserRemovedServiceInfoRemoteFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.status = fdp.ConsumeIntegral<int32_t>();
    profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    DeviceProfileConnector::GetInstance().FillDmUserRemovedServiceInfoRemote(profile, serviceInfos);
}

void GetServiceInfoByUdidAndServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string udid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DistributedDeviceProfile::ServiceInfo serviceInfo;

    DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(udid, serviceId, serviceInfo);
}

void GetServiceInfosByUdidAndUserIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string udid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::vector<DistributedDeviceProfile::ServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().GetServiceInfosByUdidAndUserId(udid, userId, serviceInfos);
}

void PutServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::ServiceInfo serviceInfo;
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DeviceProfileConnector::GetInstance().PutServiceInfo(serviceInfo);
}

void DeleteServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::ServiceInfo serviceInfo;
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DeviceProfileConnector::GetInstance().DeleteServiceInfo(serviceInfo);
}

void GetServiceInfosByUdidFuzzTest(FuzzedDataProvider &fdp)
{
    std::string udid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<DistributedDeviceProfile::ServiceInfo> serviceInfos;

    DeviceProfileConnector::GetInstance().GetServiceInfosByUdid(udid, serviceInfos);
}

void GetPeerTokenIdForServiceProxyUnbindFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    uint64_t localTokenId = fdp.ConsumeIntegral<uint64_t>();
    std::string peerUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::vector<uint64_t> peerTokenId;

    DeviceProfileConnector::GetInstance().GetPeerTokenIdForServiceProxyUnbind(userId,
            localTokenId, peerUdid, serviceId, peerTokenId);
}

void HasServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.status = fdp.ConsumeIntegral<int32_t>();
    profile.accessee.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accessee.userId = fdp.ConsumeIntegral<int32_t>();
    profile.accesser.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    profile.accesser.userId = fdp.ConsumeIntegral<int32_t>();

    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceProfileConnector::GetInstance().HasServiceId(profile, serviceId);
}

void DeviceProfileConnectorFuzzTest(FuzzedDataProvider &fdp)
{
    std::string trustDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string requestDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string trustUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string localDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string targetDeviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string requestAccountId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string deviceIdHash = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string trustBundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmAclInfo aclInfo;
    aclInfo.bindType = fdp.ConsumeIntegral<int32_t>();
    aclInfo.bindLevel = fdp.ConsumeIntegral<int32_t>();
    aclInfo.trustDeviceId = trustDeviceId;
    aclInfo.deviceIdHash = deviceIdHash;
    aclInfo.authenticationType = fdp.ConsumeIntegral<int32_t>();
    DmAccesser dmAccesser;
    dmAccesser.requestDeviceId = requestDeviceId;
    dmAccesser.requestUserId = fdp.ConsumeIntegral<int32_t>();
    dmAccesser.requestAccountId = requestAccountId;
    DmAccessee dmAccessee;
    dmAccessee.trustDeviceId = trustDeviceId;
    dmAccessee.trustUserId = fdp.ConsumeIntegral<int32_t>();
    dmAccessee.trustBundleName = trustBundleName;
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t peerUserId = fdp.ConsumeIntegral<int32_t>();
    std::string accountId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmOfflineParam offlineParam;
    offlineParam.bindType = fdp.ConsumeIntegral<uint32_t>();
    DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, userId);
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, accountId, accountId);
    DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(pkgName, localDeviceId);
    DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(trustUdid, peerUserId, userId, offlineParam);
    DeviceProfileConnector::GetInstance().GetTrustNumber(localDeviceId);
}

}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int32_t) + sizeof(int32_t) + sizeof(bool);
    if ((data == nullptr) || (size < minSize)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    /* Run your code on data */
    OHOS::DistributedHardware::DeleteAclForAccountLogOutFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteAclByActhashFuzzTest(fdp);
    OHOS::DistributedHardware::CacheOfflineParamFuzzTest(fdp);
    OHOS::DistributedHardware::ProcessLocalToPeerFuzzTest(fdp);
    OHOS::DistributedHardware::ProcessPeerToLocalFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteAclForUserRemovedFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteAclForRemoteUserRemovedFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteAccessControlListByUdidFuzzTest(fdp);
    OHOS::DistributedHardware::HandleSyncForegroundUserIdEventFuzzTest(fdp);
    OHOS::DistributedHardware::FillDmUserRemovedServiceInfoLocalFuzzTest(fdp);
    OHOS::DistributedHardware::FillDmUserRemovedServiceInfoRemoteFuzzTest(fdp);
    OHOS::DistributedHardware::GetServiceInfoByUdidAndServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::GetServiceInfosByUdidAndUserIdFuzzTest(fdp);
    OHOS::DistributedHardware::PutServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::GetServiceInfosByUdidFuzzTest(fdp);
    OHOS::DistributedHardware::GetPeerTokenIdForServiceProxyUnbindFuzzTest(fdp);
    OHOS::DistributedHardware::HasServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::DeviceProfileConnectorFuzzTest(fdp);
    return 0;
}
