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

#ifndef OHOS_DM_CALLBACK_H
#define OHOS_DM_CALLBACK_H

#include "dm_device_info.h"
#include "dm_device_profile_info.h"

namespace OHOS {
namespace DistributedHardware {
class DmInitCallback {
public:
    virtual ~DmInitCallback()
    {
    }
    virtual void OnRemoteDied() = 0;
};

class DeviceStateCallback {
public:
    virtual ~DeviceStateCallback()
    {
    }
    virtual void OnDeviceOnline(const DmDeviceInfo &deviceInfo) = 0;
    virtual void OnDeviceOffline(const DmDeviceInfo &deviceInfo) = 0;
    virtual void OnDeviceChanged(const DmDeviceInfo &deviceInfo) = 0;
    virtual void OnDeviceReady(const DmDeviceInfo &deviceInfo) = 0;
};

class DeviceStatusCallback {
public:
    virtual ~DeviceStatusCallback()
    {
    }
    virtual void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) = 0;
    virtual void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) = 0;
    virtual void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo) = 0;
    virtual void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) = 0;
};

class DiscoveryCallback {
public:
    virtual ~DiscoveryCallback()
    {
    }
    virtual void OnDiscoverySuccess(uint16_t subscribeId) = 0;
    virtual void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) = 0;
    virtual void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) {};
    virtual void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo) {};
};

class PublishCallback {
public:
    virtual ~PublishCallback()
    {
    }
    virtual void OnPublishResult(int32_t publishId, int32_t publishResult) = 0;
};

class AuthenticateCallback {
public:
    virtual ~AuthenticateCallback()
    {
    }
    virtual void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
        int32_t reason) = 0;
};

class BindTargetCallback {
public:
    virtual ~BindTargetCallback()
    {
    }
    virtual void OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status, std::string content) = 0;
};

class UnbindTargetCallback {
public:
    virtual ~UnbindTargetCallback()
    {
    }
    virtual void OnUnbindResult(const PeerTargetId &targetId, int32_t result, std::string content) = 0;
};

class VerifyAuthCallback {
public:
    virtual ~VerifyAuthCallback()
    {
    }
    virtual void OnVerifyAuthResult(const std::string &deviceId, int32_t resultCode, int32_t flag) = 0;
};

class DeviceManagerUiCallback {
public:
    virtual ~DeviceManagerUiCallback()
    {
    }
    virtual void OnCall(const std::string &paramJson) = 0;
};

class GetDeviceProfileInfoListCallback {
public:
    virtual ~GetDeviceProfileInfoListCallback()
    {}
    virtual void OnResult(const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code) = 0;
};

class GetDeviceIconInfoCallback {
public:
    virtual ~GetDeviceIconInfoCallback() {}
    virtual void OnResult(const DmDeviceIconInfo &deviceIconInfo, int32_t code) = 0;
};

class SetLocalDeviceNameCallback {
public:
    virtual ~SetLocalDeviceNameCallback() {}
    virtual void OnResult(int32_t code) = 0;
};

class SetRemoteDeviceNameCallback {
public:
    virtual ~SetRemoteDeviceNameCallback() {}
    virtual void OnResult(int32_t code) = 0;
};

class CredentialCallback {
public:
    virtual ~CredentialCallback()
    {
    }
    virtual void OnCredentialResult(int32_t &action, const std::string &credentialResult) = 0;
};

class PinHolderCallback {
public:
    virtual ~PinHolderCallback()
    {
    }
    virtual void OnPinHolderCreate(const std::string &deviceId, DmPinType pinType, const std::string &payload) = 0;
    virtual void OnPinHolderDestroy(DmPinType pinType, const std::string &payload) = 0;
    virtual void OnCreateResult(int32_t result) = 0;
    virtual void OnDestroyResult(int32_t result) = 0;
    virtual void OnPinHolderEvent(DmPinHolderEvent event, int32_t result, const std::string &content) = 0;
};

class DevTrustChangeCallback {
public:
    virtual ~DevTrustChangeCallback()
    {
    }
    virtual void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) = 0;
};

class DeviceScreenStatusCallback {
public:
    virtual ~DeviceScreenStatusCallback()
    {
    }
    virtual void OnDeviceScreenStatus(const DmDeviceInfo &deviceInfo) = 0;
};

class CredentialAuthStatusCallback {
public:
    virtual ~CredentialAuthStatusCallback()
    {
    }
    virtual void OnCredentialAuthStatus(const std::string &proofInfo, uint16_t deviceTypeId, int32_t errcode) = 0;
};

class ServiceDiscoveryCallback {
public:
    virtual ~ServiceDiscoveryCallback()
    {
    }
    virtual void OnServiceFound(const DiscoveryServiceInfo &service) = 0;
    virtual void OnServiceDiscoveryResult(int32_t resReason) = 0;
};

class ServiceInfoStateCallback {
public:
    virtual ~ServiceInfoStateCallback()
    {
    }
    virtual void OnServiceOnline(int64_t serviceId) = 0;
    virtual void OnServiceOffline(int64_t serviceId) = 0;
};

class ServicePublishCallback {
public:
    virtual ~ServicePublishCallback()
    {
    }
    virtual void OnServicePublishResult(int64_t serviceId, int32_t reason) = 0;
};

class LeaveLNNCallback {
public:
    virtual ~LeaveLNNCallback()
    {
    }
    virtual void OnLeaveLNNCallback(const std::string &networkId, int32_t retCode) = 0;
};

class AuthCodeInvalidCallback {
public:
    virtual ~AuthCodeInvalidCallback()
    {
    }
    virtual void OnAuthCodeInvalid() = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CALLBACK_H
