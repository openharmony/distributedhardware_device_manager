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

#ifndef OHOS_DM_DIALOG_MANAGER_H
#define OHOS_DM_DIALOG_MANAGER_H

#include <semaphore.h>
#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"

#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DmDialogManager {
public:
    static DmDialogManager &GetInstance();
    static void ConnectExtension();
    void ShowConfirmDialog(const std::string param);
    void ShowPinDialog(const std::string param);
    void ShowInputDialog(const std::string param);
    static std::string GetBundleName()
    {
        return bundleName_;
    }

    static std::string GetAbilityName()
    {
        return abilityName_;
    }

    static std::string GetPinCode()
    {
        return pinCode_;
    }

    static std::string GetDeviceName()
    {
        return deviceName_;
    }

    static std::string GetAppOperationStr()
    {
        return appOperationStr_;
    }

    static std::string GetCustomDescriptionStr()
    {
        return customDescriptionStr_;
    }

    static int32_t GetDeviceType()
    {
        return deviceType_;
    }

    static std::string GetTargetDeviceName()
    {
        return targetDeviceName_;
    }

    static std::string GetHostPkgLabel()
    {
        return hostPkgLabel_;
    }
    static bool GetIsProxyBind()
    {
        return isProxyBind_;
    }
    static std::string GetAppUserData()
    {
        return appUserData_;
    }
private:
    DmDialogManager();
    ~DmDialogManager();
    class DialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        void OnAbilityConnectDone(
            const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

    private:
        std::mutex mutex_;
    };

    static std::string bundleName_;
    static std::string abilityName_;
    static std::string deviceName_;
    static std::string targetDeviceName_;
    static std::string appOperationStr_;
    static std::string customDescriptionStr_;
    static std::string pinCode_;
    static std::string hostPkgLabel_;
    static int32_t deviceType_;
    static std::atomic<bool> isConnectSystemUI_;
    static sptr<OHOS::AAFwk::IAbilityConnection> dialogConnectionCallback_;
    static DmDialogManager dialogMgr_;
    static bool isProxyBind_;
    static std::string appUserData_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
