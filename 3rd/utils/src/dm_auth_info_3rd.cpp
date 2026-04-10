/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See License for the specific language governing permissions and
 * limitations under License.
 */

#include "dm_auth_info_3rd.h"

#include <cstring>
#include <algorithm>

#include "json_object.h"
#include "dm_anonymous_3rd.h"

namespace OHOS {
namespace DistributedHardware {

void ToJson(JsonItemObject &itemObject, const Access3rd &access)
{
    itemObject["deviceId"] = access.deviceId;
    itemObject["userId"] = access.userId;
    itemObject["accountId"] = access.accountId;
    itemObject["tokenId"] = access.tokenId;
    itemObject["processName"] = access.processName;
    itemObject["uid"] = access.uid;
    itemObject["businessName"] = access.businessName;
    itemObject["version"] = access.version;
}

void FromJson(const JsonItemObject &itemObject, Access3rd &access)
{
    if (itemObject.Contains("deviceId") && itemObject["deviceId"].IsString()) {
        access.deviceId = itemObject["deviceId"].Get<std::string>();
    }
    if (itemObject.Contains("userId") && itemObject["userId"].IsNumberInteger()) {
        access.userId = itemObject["userId"].Get<int32_t>();
    }
    if (itemObject.Contains("accountId") && itemObject["accountId"].IsString()) {
        access.accountId = itemObject["accountId"].Get<std::string>();
    }
    if (itemObject.Contains("tokenId") && itemObject["tokenId"].IsNumberInteger()) {
        access.tokenId = itemObject["tokenId"].Get<uint32_t>();
    }
    if (itemObject.Contains("processName") && itemObject["processName"].IsString()) {
        access.processName = itemObject["processName"].Get<std::string>();
    }
    if (itemObject.Contains("uid") && itemObject["uid"].IsNumberInteger()) {
        access.uid = itemObject["uid"].Get<int32_t>();
    }
    if (itemObject.Contains("businessName") && itemObject["businessName"].IsString()) {
        access.businessName = itemObject["businessName"].Get<std::string>();
    }
    if (itemObject.Contains("version") && itemObject["version"].IsString()) {
        access.version = itemObject["version"].Get<std::string>();
    }
}

void ToJson(JsonItemObject &itemObject, const AccessControl3rd &accessControl)
{
    itemObject["trustDeviceId"] = accessControl.trustDeviceId;
    itemObject["sessionKeyId"] = accessControl.sessionKeyId;
    itemObject["createTime"] = accessControl.createTime;
    itemObject["accesser"] = accessControl.accesser;
    itemObject["accessee"] = accessControl.accessee;
    itemObject["extra"] = accessControl.extra;
    itemObject["bindLevel"] = accessControl.bindLevel;
    itemObject["bindType"] = accessControl.bindType;
}

void FromJson(const JsonItemObject &itemObject, AccessControl3rd &accessControl)
{
    if (IsString(itemObject, "trustDeviceId")) {
        accessControl.trustDeviceId = itemObject["trustDeviceId"].Get<std::string>();
    }
    if (IsInt32(itemObject, "sessionKeyId")) {
        accessControl.sessionKeyId = itemObject["sessionKeyId"].Get<int32_t>();
    }
    if (IsInt64(itemObject, "createTime")) {
        accessControl.createTime = itemObject["createTime"].Get<int64_t>();
    }
    if (itemObject.Contains("accesser") && itemObject["accesser"].IsObject()) {
        FromJson(itemObject["accesser"], accessControl.accesser);
    }
    if (itemObject.Contains("accessee") && itemObject["accessee"].IsObject()) {
        FromJson(itemObject["accessee"], accessControl.accessee);
    }
    if (IsString(itemObject, "extra")) {
        accessControl.extra = itemObject["extra"].Get<std::string>();
    }
    if (IsInt32(itemObject, "bindLevel")) {
        accessControl.bindLevel = itemObject["bindLevel"].Get<int32_t>();
    }
    if (IsInt32(itemObject, "bindType")) {
        accessControl.bindType = itemObject["bindType"].Get<int32_t>();
    }
}

void ToJson(JsonItemObject &itemObject, const TrustDeviceInfo3rd &trustDeviceInfo)
{
    itemObject["trustDeviceId"] = trustDeviceInfo.trustDeviceId;
    itemObject["sessionKeyId"] = trustDeviceInfo.sessionKeyId;
    itemObject["createTime"] = trustDeviceInfo.createTime;
    itemObject["userId"] = trustDeviceInfo.userId;
    itemObject["extra"] = trustDeviceInfo.extra;
    itemObject["bindLevel"] = trustDeviceInfo.bindLevel;
    itemObject["bindType"] = trustDeviceInfo.bindType;
}

void FromJson(const JsonItemObject &itemObject, TrustDeviceInfo3rd &trustDeviceInfo)
{
    if (IsString(itemObject, "trustDeviceId")) {
        trustDeviceInfo.trustDeviceId = itemObject["trustDeviceId"].Get<std::string>();
    }
    if (IsInt32(itemObject, "sessionKeyId")) {
        trustDeviceInfo.sessionKeyId = itemObject["sessionKeyId"].Get<int32_t>();
    }
    if (IsInt64(itemObject, "createTime")) {
        trustDeviceInfo.createTime = itemObject["createTime"].Get<int64_t>();
    }
    if (IsInt32(itemObject, "userId")) {
        trustDeviceInfo.userId = itemObject["userId"].Get<int32_t>();
    }
    if (IsString(itemObject, "extra")) {
        trustDeviceInfo.extra = itemObject["extra"].Get<std::string>();
    }
    if (IsInt32(itemObject, "bindLevel")) {
        trustDeviceInfo.bindLevel = itemObject["bindLevel"].Get<int32_t>();
    }
    if (IsInt32(itemObject, "bindType")) {
        trustDeviceInfo.bindType = itemObject["bindType"].Get<int32_t>();
    }
}
} // namespace DistributedHardware
} // namespace OHOS
