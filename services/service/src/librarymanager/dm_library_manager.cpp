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

#include "dm_library_manager.h"
#include <dlfcn.h>
#include <functional>

namespace OHOS {
namespace DistributedHardware {
std::unique_ptr<DMLibraryManager> DMLibraryManager::instance_ = nullptr;
std::once_flag DMLibraryManager::initFlag_;

DMLibraryManager::DMLibraryManager()
    : segments_(NUM_SEGMENTS), segmentMutexes_(NUM_SEGMENTS) {}

DMLibraryManager& DMLibraryManager::GetInstance()
{
    std::call_once(initFlag_, []() {
        instance_ = std::make_unique<DMLibraryManager>();
    });
    return *instance_;
}

size_t DMLibraryManager::GetSegmentIndex(const std::string& libraryPath) const
{
    return std::hash<std::string>{}(libraryPath) % NUM_SEGMENTS;
}

std::shared_ptr<DMLibraryManager::LibraryInfo> DMLibraryManager::GetOrCreateLibrary(const std::string& libraryPath)
{
    size_t segmentIdx = GetSegmentIndex(libraryPath);

    {
        std::shared_lock<std::shared_mutex> segmentLock(segmentMutexes_[segmentIdx]);
        auto it = segments_[segmentIdx].find(libraryPath);
        if (it != segments_[segmentIdx].end()) {
            return it->second;
        }
    }

    std::unique_lock<std::shared_mutex> segmentLock(segmentMutexes_[segmentIdx]);
    auto it = segments_[segmentIdx].find(libraryPath);
    if (it != segments_[segmentIdx].end()) {
        return it->second;
    }

    auto newLibInfo = std::make_shared<LibraryInfo>();
    segments_[segmentIdx][libraryPath] = newLibInfo;
    return newLibInfo;
}

std::shared_ptr<DMLibraryManager::LibraryInfo> DMLibraryManager::GetLibraryInfo(const std::string& libraryPath)
{
    size_t segmentIdx = GetSegmentIndex(libraryPath);

    std::shared_lock<std::shared_mutex> segmentLock(segmentMutexes_[segmentIdx]);
    auto it = segments_[segmentIdx].find(libraryPath);
    if (it != segments_[segmentIdx].end()) {
        return it->second;
    }

    return nullptr;
}

void DMLibraryManager::DoUnloadLib(std::string& libraryPath)
{
    auto libInfo = GetLibraryInfo(libraryPath);
    if (libInfo == nullptr) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(libInfo->mutex);
    int32_t currentRefs = libInfo->refCount.load();
    auto now = std::chrono::steady_clock::now();
    auto nowSec = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
    int32_t freeTimeSpan = static_cast<int32_t>(nowSec - libInfo->lastUsed);

    if ((currentRefs == 0 && freeTimeSpan >= LIB_UNLOAD_TRIGGER_FREE_TIMESPAN) ||
        (currentRefs != 0 && freeTimeSpan >= LIB_UNLOAD_TRIGGER_MAX_FREE_TIMESPAN)) {
        if (libInfo->handle != nullptr) {
            LOGI("Do unload lib: %{public}s", libraryPath.c_str());
            dlclose(libInfo->handle);
            libInfo->handle = nullptr;
            libInfo->isLibTimerBegin = false;
            libInfo->refCount.store(0, std::memory_order_relaxed);
        }
        libInfo->libTimer->DeleteTimer(libraryPath);
    } else {
        LOGI("Can not unload lib: %{public}s, ref: %{public}d, free time(sec): %{public}d",
            libraryPath.c_str(), currentRefs, freeTimeSpan);
        libInfo->libTimer->DeleteTimer(libraryPath);
        libInfo->libTimer->StartTimer(libraryPath, LIB_UNLOAD_TRIGGER_FREE_TIMESPAN, [this] (std::string libPath) {
            DMLibraryManager::DoUnloadLib(libPath);
        });
    }
}

void DMLibraryManager::Release(const std::string& libraryPath)
{
    auto libInfo = GetLibraryInfo(libraryPath);
    if (!libInfo) {
        return;
    }

    int32_t currentRefs = libInfo->refCount.fetch_sub(1, std::memory_order_acq_rel);
    if (currentRefs <= 0) {
        LOGE("Reference count underflow for %{public}s", libraryPath.c_str());
        libInfo->refCount.store(0, std::memory_order_relaxed);
    }
    LOGI("the lib:%{public}s current ref: %{public}d", libraryPath.c_str(), currentRefs - 1);
}
} // namespace DistributedHardware
} // namespace OHOS