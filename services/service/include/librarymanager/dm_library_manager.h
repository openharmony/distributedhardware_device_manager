/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_LIBRARY_MANAGER_H
#define OHOS_DM_LIBRARY_MANAGER_H

#include <atomic>
#include <chrono>
#include <dlfcn.h>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <shared_mutex>

#include "dm_log.h"
#include "dm_timer.h"

namespace OHOS {
namespace DistributedHardware {
// if a lib not used more than 60 seconds, trigger unload it.
constexpr int32_t LIB_UNLOAD_TRGIIGER_FREE_TIMESPAN = 60;
class DMLibraryManager : public std::enable_shared_from_this<DMLibraryManager> {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    DMLibraryManager();

    static DMLibraryManager& GetInstance();

    template<typename FuncType>
    FuncType GetFunction(const std::string& libraryPath, const std::string& functionName);

    void Release(const std::string& libraryPath);
    void RequestUnload(const std::string& libraryPath);
    bool IsLoaded(const std::string& libraryPath);

private:
    struct LibraryInfo {
        void* handle = nullptr;
        std::atomic<int> refCount{0};
        std::atomic<bool> isLibTimerBegin{false};
        mutable std::shared_mutex mutex;
        TimePoint lastUsed;
        std::unique_ptr<DmTimer> libTimer;

        LibraryInfo()
        {
            lastUsed = std::chrono::steady_clock::now();
            libTimer = std::make_unique<DmTimer>();
        }
    };

    static const size_t NUM_SEGMENTS = 8;
    std::vector<std::unordered_map<std::string, std::shared_ptr<LibraryInfo>>> segments_;
    std::vector<std::shared_mutex> segmentMutexes_;

    size_t GetSegmentIndex(const std::string& libraryPath) const;

    static std::unique_ptr<DMLibraryManager> instance_;
    static std::once_flag initFlag_;

private:
    std::shared_ptr<LibraryInfo> GetOrCreateLibrary(const std::string& libraryPath);
    std::shared_ptr<LibraryInfo> GetLibraryInfo(const std::string& libraryPath);
    void DoUnloadLib(std::string& libraryPath);
};

template<typename FuncType>
FuncType DMLibraryManager::GetFunction(const std::string& libraryPath, const std::string& functionName)
{
    auto libInfo = GetOrCreateLibrary(libraryPath);
    {
        std::shared_lock<std::shared_mutex> readLock(libInfo->mutex);
        libInfo->refCount++;

        if (!libInfo->handle) {
            readLock.unlock();
            std::unique_lock<std::shared_mutex> writeLock(libInfo->mutex);
            if (!libInfo->handle) {
                libInfo->handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
                if (!libInfo->handle) {
                    libInfo->refCount--;
                    LOGE("dlopen failed for %{public}s: %{public}s", libraryPath.c_str(), dlerror());
                    return nullptr;
                }
            }
        }
    }

    void* sym = dlsym(libInfo->handle, functionName.c_str());
    if (!sym) {
        libInfo->refCount--;
        LOGE("dlopen failed for %{public}s: %{public}s", libraryPath.c_str(), dlerror());
        return nullptr;
    }
    libInfo->lastUsed = std::chrono::steady_clock::now();
    if (libInfo->isLibTimerBegin == false) {
        auto weakSelf = weak_from_this();
        libInfo->libTimer->StartTimer(libraryPath, LIB_UNLOAD_TRGIIGER_FREE_TIMESPAN, [weakSelf] (std::string libPath) {
            if (auto self = weakSelf.lock()) {
                self->DoUnloadLib(libPath);
            }
        });
        libInfo->isLibTimerBegin = true;
    }
    LOGI("Do load lib: %{public}s", libraryPath.c_str());
    return reinterpret_cast<FuncType>(sym);
}

inline DMLibraryManager& GetLibraryManager()
{
    return DMLibraryManager::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_LIBRARY_MANAGER_H