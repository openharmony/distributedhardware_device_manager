/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "device_manager_service_impl_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DeviceManagerServiceImpl::StopAuthenticateDevice(const std::string &pkgName)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->StopAuthenticateDevice(pkgName);
}

int32_t DeviceManagerServiceImpl::GetBindLevel(const std::string &pkgName, const std::string &localUdid,
    const std::string &udid, uint64_t &tokenId)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetBindLevel(pkgName, localUdid, udid, tokenId);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->UnBindDevice(pkgName, udid, bindLevel);
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t userId,
    const std::string &accountId)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetDeviceIdAndUserId(userId, accountId);
}

std::multimap<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndUserId(int32_t userId)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetDeviceIdAndUserId(userId);
}
std::map<std::string, int32_t> DeviceManagerServiceImpl::GetDeviceIdAndBindLevel(int32_t localUserId)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetDeviceIdAndBindLevel(localUserId);
}

std::unordered_map<std::string, DmAuthForm> DeviceManagerServiceImpl::GetAppTrustDeviceIdList(std::string pkgname)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->GetAppTrustDeviceIdList(pkgname);
}

int32_t DeviceManagerServiceImpl::UnAuthenticateDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->UnAuthenticateDevice(pkgName, udid, bindLevel);
}

int32_t DeviceManagerServiceImpl::UnBindDevice(const std::string &pkgName, const std::string &udid,
    int32_t bindLevel, const std::string &extra)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->UnBindDevice(pkgName, udid, bindLevel, extra);
}
bool DeviceManagerServiceImpl::CheckSharePeerSrc(const std::string &peerUdid, const std::string &localUdid)
{
    return DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->CheckSharePeerSrc(peerUdid, localUdid);
}

void DeviceManagerServiceImpl::HandleCredentialDeleted(const char *credId, 
    const char *credInfo, const std::string &localUdid, std::string &remoteUdid)
{
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->HandleCredentialDeleted(credId, 
        credInfo, localUdid, remoteUdid);
}
void DeviceManagerServiceImpl::HandleShareUnbindBroadCast(const std::string &credId, 
    const int32_t &userId, const std::string &localUdid)
{
    DmDeviceManagerServiceImpl::dmDeviceManagerServiceImpl->HandleShareUnbindBroadCast(credId, userId, localUdid);
}
} // namespace DistributedHardware
} // namespace OHOS