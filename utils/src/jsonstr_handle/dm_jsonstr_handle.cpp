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

#include "dm_jsonstr_handle.h"

#include "json_object.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"

const char* TAG_PROXY = "proxy";
const char* TAG_PEER_BUNDLE_NAME = "peerBundleName";
const char* TAG_PEER_TOKENID = "peerTokenId";

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(JsonStrHandle);

DM_EXPORT void JsonStrHandle::GetPeerAppInfoParseExtra(const std::string &extra,
    uint64_t &peerTokenId, std::string &peerBundleName)
{
    JsonObject extraInfoJson(extra);
    if (extraInfoJson.IsDiscarded()) {
        LOGE("ParseExtra extraInfoJson error");
        return;
    }
    if (!IsString(extraInfoJson, TAG_PEER_BUNDLE_NAME) || !IsInt64(extraInfoJson, TAG_PEER_TOKENID)) {
        LOGE("ParseExtra TAG_PEER_BUNDLE_NAME or TAG_PEER_TOKENID error");
        return;
    }
    peerTokenId = extraInfoJson[TAG_PEER_TOKENID].Get<uint64_t>();
    peerBundleName = extraInfoJson[TAG_PEER_BUNDLE_NAME].Get<std::string>();
}

DM_EXPORT std::vector<int64_t> JsonStrHandle::GetProxyTokenIdByExtra(const std::string &extraInfo)
{
    std::vector<int64_t> tokenIdVec;
    JsonObject extraInfoJson(extraInfo);
    if (extraInfoJson.IsDiscarded() || !IsString(extraInfoJson, TAG_PROXY)) {
        LOGE("GetProxyTokenIdByExtra extraInfoJson error");
        return tokenIdVec;
    }
    std::string proxyListStr = extraInfoJson[TAG_PROXY].Get<std::string>();
    JsonObject proxyList;
    if (!proxyList.Parse(proxyListStr)) {
        LOGE("GetProxyTokenIdByExtra proxyListStr Parse error");
        return tokenIdVec;
    }
    for (auto &item : proxyList.Items()) {
        if (item.IsNumber()) {
            tokenIdVec.push_back(item.Get<int64_t>());
        }
    }
    return tokenIdVec;
}
} // namespace DistributedHardware
} // namespace OHOS
