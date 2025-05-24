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

#include "dm_auth_attest_common.h"

#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

const int32_t MAX_CERT_COUNT = 1024;
constexpr int32_t HEX_TO_UINT8 = 2;

std::string AuthAttestCommon::SerializeDmCertChain(const DmCertChain *chain)
{
    if (chain == nullptr || chain->cert == nullptr || chain->certCount == 0) {
        return "{}";
    }
    JsonObject jsonObject;
    jsonObject[TAG_CERT_COUNT] = chain->certCount;
    JsonObject jsonArrayObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (uint32_t i = 0; i < chain->certCount; ++i) {
        const DmBlob &blob = chain->cert[i];
        if (blob.data == nullptr || blob.size == 0) {
            return "{}";
        }
        const uint32_t hexLen = blob.size * HEX_TO_UINT8 + 1; // 2*blob.size + 1
        char *hexBuffer = new char[hexLen]{0};
        if (hexBuffer == nullptr) {
            return "{}";
        }
        int32_t ret = Crypto::ConvertBytesToHexString(hexBuffer, hexLen, blob.data, blob.size);
        if (ret != DM_OK) {
            delete[] hexBuffer;
            return "{}";
        }
        jsonArrayObj.PushBack(std::string(hexBuffer));
        delete[] hexBuffer;
    }
    jsonObject.Insert(TAG_CERT, jsonArrayObj);
    return jsonObject.Dump();
}

bool ValidateInputJson(const std::string &data)
{
    JsonObject jsonObject;
    jsonObject.Parse(data);
    if (!IsUint32(jsonObject, TAG_CERT_COUNT) || !jsonObject.Contains(TAG_CERT)) {
        LOGE("DeserializeDmCertChain: Missing required fields 'certCount' or 'cert'");
        return false;
    }
    const uint32_t certCount = jsonObject[TAG_CERT_COUNT].Get<uint32_t>();
    if (certCount == 0 || certCount > MAX_CERT_COUNT) {
        LOGE("DeserializeDmCertChain: Invalid certCount value %{public}u", certCount);
        return false;
    }
    return true;
}

 bool ProcessCertItem(const JsonItemObject &item, DmBlob &cert, uint32_t processedIndex)
 {
    std::string hexStr = item.Get<std::string>();
    const size_t hexLen = hexStr.length();
    if (hexLen == 0 || hexLen % HEX_TO_UINT8 != 0) {
        LOGE("DeserializeDmCertChain: Invalid HEX length %{public}zu at index %{public}u", hexLen, processedIndex);
        return false;
    }
    const uint32_t binSize = hexLen / HEX_TO_UINT8;
    cert.data = new uint8_t[binSize];
    if (cert.data == nullptr) {
        LOGE("DeserializeDmCertChain: Data allocation failed at index %{public}u", processedIndex);
        return false;
    }
    int32_t ret = Crypto::ConvertHexStringToBytes(cert.data, binSize, hexStr.c_str(), hexLen);
    if (ret != DM_OK) {
        LOGE("DeserializeDmCertChain: HEX conversion failed at index %{public}u, ret = %{public}d",
            processedIndex, ret);
        delete[] cert.data;
        cert.data = nullptr;
        return false;
    }
    cert.size = binSize;
    return true;
}

bool AuthAttestCommon::DeserializeDmCertChain(const std::string &data, DmCertChain *outChain)
{
    if (outChain == nullptr || data.empty() || !ValidateInputJson(data)) {
        LOGE("Invalid input");
        return false;
    }
    JsonObject jsonObject;
    jsonObject.Parse(data);
    const uint32_t certCount = jsonObject[TAG_CERT_COUNT].Get<uint32_t>();
    JsonObject jsonArrayObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    jsonArrayObj.Parse(jsonObject[TAG_CERT].Dump());
    DmBlob *certs = new DmBlob[certCount]{0};
    if (certs == nullptr) {
        LOGE("DeserializeDmCertChain: Memory allocation failed for certs array");
        return false;
    }
    bool success = true;
    uint32_t processedIndex = 0;
    for (const auto &item : jsonArrayObj.Items()) {
        if (!ProcessCertItem(item, certs[processedIndex], processedIndex)) {
            success = false;
            break;
        }
        processedIndex++;
    }
    if (!success) {
        for (uint32_t i = 0; i < processedIndex; ++i) {
            delete[] certs[i].data;
        }
        delete[] certs;
        return false;
    }
    if (outChain->cert != nullptr) {
        for (uint32_t i = 0; i < outChain->certCount; ++i) {
            delete[] outChain->cert[i].data;
        }
        delete[] outChain->cert;
    }
    outChain->cert = certs;
    outChain->certCount = certCount;
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS