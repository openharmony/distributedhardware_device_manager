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

#ifndef OHOS_DM_LANGUAGE_MANAGER_H
#define OHOS_DM_LANGUAGE_MANAGER_H

#include <set>
#include <string>

#include "dm_cJSON.h"
#include "dm_single_instance.h"
namespace OHOS {
namespace DistributedHardware {
using namespace DmJson;
class DmLanguageManager {
    DM_DECLARE_SINGLE_INSTANCE_BASE(DmLanguageManager);

public:
    DmLanguageManager() {}
    ~DmLanguageManager() {}
    std::string GetSystemParam(const std::string &key);
    void GetLocaleByLanguage(const std::string &language, std::set<std::string> &localeSet);
    std::string GetTextBySystemLocale(const cJSON *const textObj, const std::set<std::string> &localeSet);
    std::string GetTextBySystemLanguage(const std::string &text);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_LANGUAGE_MANAGER_H
