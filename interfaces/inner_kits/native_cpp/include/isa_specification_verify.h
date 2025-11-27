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

#ifndef OHOS_DM_ISA_SPECIFICATION_VERIFY_H
#define OHOS_DM_ISA_SPECIFICATION_VERIFY_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
struct WhiteListInfo {
    int32_t objectSaUid = 0;
    int32_t subjectSaUid = 0;
    int32_t allowCompatible = 0;
    std::string objectProcessName = "";
    std::string subjectProcessName = "";
    std::vector<int32_t> trustLevel;
    std::vector<int32_t> objectDeviceSecLevel;
    std::vector<int32_t> subjectDeviceSecLevel;
    std::vector<int32_t> userLevelBindTypes;
};

typedef struct DmSaInfo {
    int32_t saUid = 0;
    int32_t securityLevel = 0;
    std::string processName = "";
} DmSaInfo;

typedef struct DmSaCaller {
    DmSaInfo callerSaInfo;
    DmAccessCaller accessCaller;
    std::string udid = "";
} DmSaCaller;

typedef struct DmSaCallee {
    DmSaInfo calleeSaInfo;
    DmAccessCallee accessCallee;
    std::string udid = "";
} DmSaCallee;

using WhiteListMap = std::unordered_map<std::string, std::unordered_map<std::string, WhiteListInfo>>;
class ISaSpecificationVerify {
public:
    virtual ~ISaSpecificationVerify() = default;
    virtual int32_t Initialize() = 0;
    virtual int32_t LoadSaWhiteList() = 0;
    virtual int32_t LoadRpcWhiteList() = 0;
    virtual bool CheckBindTypesInWhitelist(const WhiteListInfo &whiteListInfo,
        const std::vector<int32_t> &bindTypes) = 0;
    virtual bool CheckBuildLink(const DmSaCaller &caller,
        const DmSaCallee &callee, bool isRpc, bool isSrc) = 0;
    virtual bool CheckWhiteListSystemSA(const std::string &pkgName) = 0;
    virtual bool CheckWhiteListInfo(const DmSaCaller &caller, const DmSaCallee &callee, bool isRpc, bool isSrc) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_ISA_SPECIFICATION_VERIFY_H