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

#include "account_boot_listener.h"
#include "common_event_support.h"
#include "parameter.h"

#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
    const char * const BOOTEVENT_ACCOUNT_READY = "bootevent.account.ready";
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    const char * const ACCOUNT_BOOT_EVENT = "account_boot_event";
#endif
}

std::mutex AccountBootListener::depSaStatelock_;

static void AccountBootCb(const char *key, const char *value, void *context)
{
    if (key == nullptr || value == nullptr || context == nullptr) {
        LOGE("key or value or context is null, param is error!");
        return;
    }
    if (strcmp(key, BOOTEVENT_ACCOUNT_READY) != 0 || strcmp(value, "true") != 0) {
        return;
    }
    AccountBootListener *accountBootListener = static_cast<AccountBootListener *>(context);

    if (accountBootListener == nullptr) {
        LOGE("accountBootListener is null");
        return;
    }
    LOGI("Trigger");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { accountBootListener->DoAccountBootProc(); });
#else
    std::thread dealThread([=]() {
        accountBootListener->DoAccountBootProc();
    });
    int32_t ret = pthread_setname_np(dealThread.native_handle(), ACCOUNT_BOOT_EVENT);
    if (ret != DM_OK) {
        LOGE("dealThread setname failed.");
    }
    dealThread.detach();
#endif
}

AccountBootListener::AccountBootListener() : isRegAccountBootCb_(false),
    localDeviceMgr_(std::make_shared<LocalDeviceNameMgr>()), isDmSaReady_(false), isDataShareReady_(false),
    dataShareCommonEventManager_(std::make_shared<DmDataShareCommonEventManager>())
{
    LOGI("Ctor AccountBootListener");
}

AccountBootListener::~AccountBootListener()
{
    LOGI("Dtor AccountBootListener");
}

void AccountBootListener::RegisterAccountBootCb()
{
    if (isRegAccountBootCb_) {
        return;
    }
    LOGI("start");
    int32_t ret = WatchParameter(BOOTEVENT_ACCOUNT_READY, AccountBootCb, this);
    if (ret != 0) {
        LOGE("watch account boot event fail");
    }
    isRegAccountBootCb_ = true;
}

void AccountBootListener::DoAccountBootProc()
{
    LOGI("start");
    if (localDeviceMgr_ == nullptr) {
        LOGE("localDeviceMgr_ is null");
        return;
    }
    localDeviceMgr_->RegisterDeviceNameChangeCb();
    localDeviceMgr_->RegisterDisplayNameChangeCb();
    localDeviceMgr_->QueryLocalDisplayName();
    localDeviceMgr_->QueryLocalDeviceName();
}

void AccountBootListener::SetSaTriggerFlag(SaTriggerFlag triggerFlag)
{
    LOGI("start");
    std::lock_guard<std::mutex> lock(depSaStatelock_);
    switch (triggerFlag) {
        case SaTriggerFlag::DM_SA_READY:
            isDmSaReady_ = true;
            LOGI("DM SA ready!");
            break;
        case SaTriggerFlag::DATA_SHARE_SA_REDDY:
            LOGI("DATA SHARE SA ready!");
            this->InitDataShareEvent();
            break;
        default:
            break;
    }
    LOGI("isDmSaReady_: %{public}d, isDataShareReady_: %{public}d",
        std::atomic_load(&isDmSaReady_), std::atomic_load(&isDataShareReady_));
    if (isDmSaReady_ && isDataShareReady_) {
        LOGI("dm and data_share is ready!");
        this->RegisterAccountBootCb();
    }
}

void AccountBootListener::InitDataShareEvent()
{
    LOGI("Start");
    if (dataShareCommonEventManager_ == nullptr) {
        dataShareCommonEventManager_ = std::make_shared<DmDataShareCommonEventManager>();
    }
    DataShareEventCallback callback = [=]() {
        this->DataShareCallback();
    };
    std::vector<std::string> dataShareEventVec;
    dataShareEventVec.emplace_back(EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    if (dataShareCommonEventManager_->SubscribeDataShareCommonEvent(dataShareEventVec, callback)) {
        LOGI("Success");
    }
    return;
}

void AccountBootListener::DataShareCallback()
{
    LOGI("Start");
    std::lock_guard<std::mutex> lock(depSaStatelock_);
    isDataShareReady_ = true;
    LOGI("isDmSaReady_: %{public}d, isDataShareReady_: %{public}d",
        std::atomic_load(&isDmSaReady_), std::atomic_load(&isDataShareReady_));
    if (isDmSaReady_ && isDataShareReady_) {
        LOGI("dm and data_share is ready!");
        this->RegisterAccountBootCb();
    }
}

std::string AccountBootListener::GetLocalDisplayName() const
{
    if (localDeviceMgr_ == nullptr) {
        LOGE("logcalDeviceMgr_ is null");
        return "";
    }
    return localDeviceMgr_->GetLocalDisplayName();
}

std::string AccountBootListener::GetLocalDeviceName() const
{
    if (localDeviceMgr_ == nullptr) {
        LOGE("logcalDeviceMgr_ is null");
        return "";
    }
    return localDeviceMgr_->GetLocalDeviceName();
}
} // namespace DistributedHardware
} // namespace OHOS