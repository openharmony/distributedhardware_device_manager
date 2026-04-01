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
#include "kv_adapter_manager_3rd.h"

#include <mutex>
#include <unistd.h>

#include "datetime_ex.h"
#include "string_ex.h"

#include "dm_anonymous_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* DB_KEY_DELIMITER = "###";
}

DM_IMPLEMENT_SINGLE_INSTANCE_3RD(KVAdapterManager3rd);
int32_t KVAdapterManager3rd::Init()
{
    LOGI("Init Kv-Adapter manager");
    int32_t ret = DM_OK;
    {
        std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
        if (kvAdapter_ == nullptr) {
            kvAdapter_ = std::make_shared<KVAdapter3rd>();
            ret = kvAdapter_->Init();
        }
    }
    return ret;
}

void KVAdapterManager3rd::UnInit()
{
    LOGI("Uninit Kv-Adapter manager");
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->UnInit();
    kvAdapter_ = nullptr;
}

void KVAdapterManager3rd::ReInit()
{
    LOGI("Re init kv adapter manager");
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->ReInit();
}

int32_t KVAdapterManager3rd::PutByKey(const std::string &key, const std::string &value)
{
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(key, value) != DM_OK) {
        LOGE("Insert value to DB for dmKey failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapterManager3rd::Get(const std::string &key, std::string &value)
{
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);

    if (kvAdapter_->Get(key, value) != DM_OK) {
        LOGE("Get kv value failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapterManager3rd::GetAllByPrefix(const std::string &prefix, std::map<std::string, std::string> &acls)
{
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);

    if (kvAdapter_->GetAllByPrefix(prefix, acls) != DM_OK) {
        LOGE("Get kv value failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapterManager3rd::DeleteByKey(const std::string &key)
{
    LOGI("start");
    std::lock_guard<ffrt::mutex> kvAdapterLck(kvAdapterMtx_);
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Delete(key) != DM_OK) {
        LOGE("delete data failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
