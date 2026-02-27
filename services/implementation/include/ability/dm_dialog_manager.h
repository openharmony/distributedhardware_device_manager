/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

#include "dm_single_instance.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
class DmDialogManager {
    DM_DECLARE_SINGLE_INSTANCE_BASE(DmDialogManager);
public:
    void ConnectExtension();
    void ShowConfirmDialog(const std::string param);
    void ShowServiceBindConfirmDialog(const std::string param);
    void ShowPinDialog(const std::string param);
    void ShowInputDialog(const std::string param);
    void CloseDialog();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode);
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode);
private:
    DmDialogManager();
    ~DmDialogManager();
    void SendMsgRequest(const sptr<IRemoteObject>& remoteObject);
    void ConfigLocalInfo(JsonObject& jsonObject);
    class DialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        void OnAbilityConnectDone(
            const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;
    };

    std::string bundleName_;
    std::string abilityName_;
    std::string deviceName_;
    std::string targetDeviceName_;
    std::string appOperationStr_;
    std::string customDescriptionStr_;
    std::string pinCode_;
    std::string serviceType_;
    std::string hostPkgLabel_;
    int32_t deviceType_ = -1;
    bool isProxyBind_ = false;
    bool isServiceBind_ = false;
    std::string appUserData_;
    std::string serviceUserData_;
    std::string title_;
    int32_t userId_ = -1;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::mutex mutex_;
#else
    std::mutex mutex_;
#endif
    sptr<OHOS::AAFwk::IAbilityConnection> dialogConnectionCallback_ = nullptr;
    sptr<IRemoteObject> g_remoteObject = nullptr;
    std::atomic<bool> isConnectSystemUI_{false};
    std::atomic<bool> isCloseDialog_{false};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
