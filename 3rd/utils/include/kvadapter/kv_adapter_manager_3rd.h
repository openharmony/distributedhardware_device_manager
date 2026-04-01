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

#ifndef OHOS_DM_KV_ADAPTER_MANAGER_3RD_H
#define OHOS_DM_KV_ADAPTER_MANAGER_3RD_H

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "dm_single_instance_3rd.h"
#include "ffrt.h"
#include "kv_adapter_3rd.h"

namespace OHOS {
namespace DistributedHardware {

class KVAdapterManager3rd {
    DM_DECLARE_SINGLE_INSTANCE_3RD_BASE_3RD(KVAdapterManager3rd);
public:
    int32_t Init();
    void UnInit();
    void ReInit();
    int32_t PutByKey(const std::string &key, const std::string &value);
    int32_t Get(const std::string &key, std::string &value);
    int32_t GetAllByPrefix(const std::string &prefix, std::map<std::string, std::string> &acls);
    int32_t DeleteByKey(const std::string &key);

private:
    KVAdapterManager3rd() = default;
    ~KVAdapterManager3rd() = default;

private:
    std::shared_ptr<DistributedKv::KvStoreDeathRecipient> deathRecipient_ = nullptr;
    ffrt::mutex kvAdapterMtx_;
    std::shared_ptr<KVAdapter3rd> kvAdapter_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_KV_ADAPTER_MANAGER_3RD_H
