/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICE_MANAGER_SERVICE_IMPL_MOCK_H
#define OHOS_DEVICE_MANAGER_SERVICE_IMPL_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "device_manager_service_impl.h"
#include "ipc_start_publish_service_req.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceManagerServiceImpl {
public:
    virtual ~DmDeviceManagerServiceImpl() = default;
public:
    virtual int32_t StopAuthenticateDevice(const std::string &pkgName) = 0;
    virtual int32_t GetBindLevel(const std::string &pkgName, const std::string &localUdid,
        const std::string &udid, uint64_t &tokenId) = 0;
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel) = 0;
    virtual std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t userId, const std::string &accountId) = 0;
    virtual std::map<std::string, int32_t> GetDeviceIdAndBindLevel(int32_t userId) = 0;
    virtual std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t localUserId) = 0;
    virtual std::unordered_map<std::string, DmAuthForm> GetAppTrustDeviceIdList(std::string pkgname) = 0;
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel) = 0;
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &udid,
        int32_t bindLevel, const std::string &extra) = 0;
    virtual bool CheckSharePeerSrc(const std::string &peerUdid, const std::string &localUdid) = 0;
    virtual void HandleCredentialDeleted(const char *credId, const char *credInfo, const std::string &localUdid,
        std::string &remoteUdid, bool &isSendBroadCast) = 0;
    virtual void HandleShareUnbindBroadCast(const std::string &credId, const int32_t &userId,
        const std::string &localUdid) = 0;
    virtual int32_t CheckDeviceInfoPermission(const std::string &localUdid, const std::string &peerDeviceId) = 0;
    virtual void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId) = 0;
    virtual void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        int32_t tokenId, int32_t peerTokenId) = 0;
    virtual bool CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual bool CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual int32_t DeleteAclExtraDataServiceId(int64_t serviceId, int64_t tokenIdCaller,
        std::string &udid, int32_t &bindLevel) = 0;
    virtual int32_t ExportAuthInfo(DmAuthInfo &dmAuthInfo, uint32_t pinLength) = 0;
    virtual int32_t ImportAuthInfo(const DmAuthInfo &dmAuthInfo) = 0;
public:
    static inline std::shared_ptr<DmDeviceManagerServiceImpl> dmDeviceManagerServiceImpl = nullptr;
};

class DeviceManagerServiceImplMock : public DmDeviceManagerServiceImpl {
public:
    MOCK_METHOD(int32_t, StopAuthenticateDevice, (const std::string &));
    MOCK_METHOD(int32_t, GetBindLevel, (const std::string &, const std::string &, const std::string &, uint64_t &));
    MOCK_METHOD(int32_t, UnBindDevice, (const std::string &, const std::string &, int32_t));
    MOCK_METHOD((std::multimap<std::string, int32_t>), GetDeviceIdAndUserId, (int32_t, const std::string &));
    MOCK_METHOD((std::multimap<std::string, int32_t>), GetDeviceIdAndUserId, (int32_t));
    MOCK_METHOD((std::map<std::string, int32_t>), GetDeviceIdAndBindLevel, (int32_t));
    MOCK_METHOD((std::unordered_map<std::string, DmAuthForm>), GetAppTrustDeviceIdList, (std::string));
    MOCK_METHOD(int32_t, UnAuthenticateDevice, (const std::string &, const std::string &, int32_t));
    MOCK_METHOD(int32_t, UnBindDevice, (const std::string &, const std::string &, int32_t, const std::string &));
    MOCK_METHOD(bool, CheckSharePeerSrc, (const std::string &peerUdid, const std::string &localUdid));
    MOCK_METHOD(void, HandleCredentialDeleted, (const char *, const char *, const std::string &,
        std::string &, bool &));
    MOCK_METHOD(void, HandleShareUnbindBroadCast, (const std::string &, const int32_t &, const std::string &));
    MOCK_METHOD(int32_t, CheckDeviceInfoPermission, (const std::string &, const std::string &));
    MOCK_METHOD(void, HandleAppUnBindEvent, (int32_t, const std::string &, int32_t));
    MOCK_METHOD(void, HandleAppUnBindEvent, (int32_t, const std::string &, int32_t, int32_t));
    MOCK_METHOD(bool, CheckAccessControl, (const DmAccessCaller &, const std::string &,
        const DmAccessCallee &, const std::string &));
    MOCK_METHOD(bool, CheckIsSameAccount, (const DmAccessCaller &, const std::string &,
        const DmAccessCallee &, const std::string &));
    MOCK_METHOD(int32_t, DeleteAclExtraDataServiceId, (int64_t, int64_t,
        std::string &, int32_t &), (override));
    MOCK_METHOD(int32_t, ExportAuthInfo, (DmAuthInfo &, uint32_t));
    MOCK_METHOD(int32_t, ImportAuthInfo, (const DmAuthInfo &));
};
}
}
#endif
