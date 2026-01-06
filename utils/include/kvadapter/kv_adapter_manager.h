/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_KV_ADAPTER_MANAGER_H
#define OHOS_DM_KV_ADAPTER_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "ffrt.h"

#include "dm_kv_info.h"
#include "ikv_adapter.h"
#include "dm_single_instance.h"
#include "dm_timer.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {

class KVAdapterManager {
    DM_DECLARE_SINGLE_INSTANCE_BASE(KVAdapterManager);
public:
    DM_EXPORT int32_t Init();
    DM_EXPORT void UnInit();
    DM_EXPORT void ReInit();
    DM_EXPORT int32_t PutByAnoyDeviceId(const std::string &key, const DmKVValue &value);
    DM_EXPORT int32_t Get(const std::string &key, DmKVValue &value);
    DM_EXPORT int32_t DeleteAgedEntry();
    DM_EXPORT int32_t AppUninstall(const std::string &appId);
    DM_EXPORT int32_t GetFreezeData(const std::string &key, std::string &value);
    DM_EXPORT int32_t PutFreezeData(const std::string &key, std::string &value);
    DM_EXPORT int32_t DeleteFreezeData(const std::string &key);
    DM_EXPORT int32_t GetAllOstypeData(std::vector<std::string> &values);
    DM_EXPORT int32_t PutOstypeData(const std::string &key, const std::string &value);
    DM_EXPORT int32_t DeleteOstypeData(const std::string &key);
    DM_EXPORT int32_t GetLocalUserIdData(const std::string &key, std::string &value);
    DM_EXPORT int32_t PutLocalUserIdData(const std::string &key, const std::string &value);
    DM_EXPORT int32_t GetOsTypeCount(int32_t &count);

private:
    KVAdapterManager() = default;
    ~KVAdapterManager() = default;

    struct KVAdapterDeleter {
        explicit KVAdapterDeleter(KVAdapterManager& manager) : manager(manager) {}
        void operator()(IKVAdapter* adapter) const
        {
            if (adapter != nullptr) {
                manager.AfterUseKvAdapter();
            }
        }
        KVAdapterManager& manager;
    };
    using KVAdapterPtr = std::unique_ptr<IKVAdapter, KVAdapterDeleter>;

    KVAdapterPtr GetKvAdapter();
    void AfterUseKvAdapter();
    std::unique_ptr<DmTimer> libTimer_;
    inline bool IsTimeOut(int64_t sourceTime, int64_t targetTime, int64_t timeOut);

private:
    ffrt::mutex kvAdapterMtx_;
    IKVAdapter *kvAdapter_ = nullptr;
    ffrt::mutex idCacheMapMtx_;
    std::map<std::string, DmKVValue> idCacheMap_;

    std::atomic<int32_t> refCount_{0};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_KV_ADAPTER_MANAGER_H
