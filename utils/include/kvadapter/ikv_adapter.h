/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IKV_ADAPTER_H
#define OHOS_DM_IKV_ADAPTER_H

#include <string>
#include <vector>
#include "dm_kv_info.h"

namespace OHOS {
namespace DistributedHardware {

class IKVAdapter {
public:
    virtual ~IKVAdapter() = default;
    
    virtual int32_t Init() = 0;
    virtual void UnInit() = 0;
    virtual int32_t ReInit() = 0;
    virtual int32_t Put(const std::string &key, const std::string &value) = 0;
    virtual int32_t Get(const std::string &key, std::string &value) = 0;
    virtual int32_t DeleteKvStore() = 0;
    virtual int32_t DeleteByAppId(const std::string &appId, const std::string &prefix) = 0;
    virtual int32_t DeleteBatch(const std::vector<std::string> &keys) = 0;
    virtual int32_t Delete(const std::string& key) = 0;
    virtual int32_t GetAllOstypeData(const std::string &key, std::vector<std::string> &values) = 0;
    virtual int32_t GetOstypeCountByPrefix(const std::string &prefix, int32_t &count) = 0;
};

// 工厂函数声明
using CreateKVAdapterFunc = IKVAdapter* (*)();
using DestroyKVAdapterFunc = void (*)(IKVAdapter*);

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IKV_ADAPTER_H