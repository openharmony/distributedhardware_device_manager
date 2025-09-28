/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "mine_softbus_listener.h"

#include <dlfcn.h>
#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <thread>
#include <unistd.h>
#include <condition_variable>
#include <list>

#include "device_manager_service.h"
#include "dm_common_util.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "softbus_listener.h"
#include "json_object.h"
#include "dm_crypto.h"
#include "openssl/sha.h"
#include "openssl/evp.h"

namespace OHOS {
namespace DistributedHardware {
enum PulishStatus {
    STATUS_UNKNOWN = 0,
    ALLOW_BE_DISCOVERY = 1,
    NOT_ALLOW_BE_DISCOVERY = 2,
};
constexpr uint32_t DM_MAX_SCOPE_TLV_NUM = 3;
constexpr uint32_t DM_MAX_VERTEX_TLV_NUM = 6;
constexpr int32_t SHA256_OUT_DATA_LEN = 32;
constexpr int32_t MAX_RETRY_TIMES = 30;
constexpr int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t DM_MAX_DEVICE_ALIAS_LEN = 65;
constexpr int32_t DM_MAX_DEVICE_UDID_LEN = 65;
constexpr int32_t DM_INVALID_DEVICE_NUMBER = -1;
constexpr int32_t DM_TLV_VERTEX_DATA_OFFSET = 2;
constexpr int32_t DM_TLV_SCOPE_DATA_OFFSET = 4;
constexpr int32_t MAX_SOFTBUS_DELAY_TIME = 10;
#if (defined(MINE_HARMONY))
constexpr int32_t DM_SEARCH_BROADCAST_MIN_LEN = 18;
#endif
constexpr const char* FIELD_DEVICE_MODE = "findDeviceMode";
constexpr const char* FIELD_TRUST_OPTIONS = "tructOptions";
constexpr const char* FIELD_FILTER_OPTIONS = "filterOptions";
constexpr const char* DEVICE_ALIAS = "persist.devicealias";
constexpr const char* DEVICE_NUMBER = "persist.devicenumber";
constexpr char BROADCAST_VERSION = 1;
constexpr char FIND_ALL_DEVICE = 1;
constexpr char FIND_SCOPE_DEVICE = 2;
constexpr char FIND_VERTEX_DEVICE = 3;
constexpr char FIND_TRUST_DEVICE = 3;
constexpr char DEVICE_ALIAS_NUMBER = 1;
constexpr char DEVICE_TYPE_TYPE = 1;
constexpr char DEVICE_SN_TYPE = 2;
constexpr char DEVICE_UDID_TYPE = 3;
constexpr char FIND_NOTRUST_DEVICE = 2;
constexpr uint32_t EVP_OK = 1;

static std::mutex g_matchWaitDeviceLock;
static std::mutex g_publishLnnLock;
static std::list<DeviceInfo> g_matchQueue;
static std::vector<std::string> pkgNameVec_ = {};
bool g_publishLnnFlag = false;
bool g_matchDealFlag = false;
std::condition_variable g_matchDealNotify;
std::condition_variable g_publishLnnNotify;

static IPublishCb publishLNNCallback_ = {
    .OnPublishResult = MineSoftbusListener::OnPublishResult,
#if (defined(MINE_HARMONY))
    .OndeviceFound = MineSoftbusListener::OnPublishDeviceFound,
    .onRePublish = MineSoftbusListener::OnRePublish
#endif
};

void FromJson(const JsonItemObject &object, VertexOptionInfo &optionInfo)
{
    if (!object.Contains("type") || !object["type"].IsString()) {
        LOGE("OptionInfo type json key is not exist or type error.");
        return;
    }
    if (!object.Contains("value") || !object["value"].IsString()) {
        LOGE("OptionInfo value json key is not exist or type error.");
        return;
    }
    object["type"].GetTo(optionInfo.type);
    object["value"].GetTo(optionInfo.value);
}

void FromJson(const JsonItemObject &object, ScopeOptionInfo &optionInfo)
{
    if (!object.Contains("deviceAlias") || !object["deviceAlias"].IsString()) {
        LOGE("OptionInfo deviceAlias json key is not exist or error.");
        return;
    }
    if (!object.Contains("startNumber") || !object["startNumber"].IsNumberInteger()) {
        LOGE("OptionInfo startNumber json key is not exist or error.");
        return;
    }
    if (!object.Contains("endNumber") || !object["endNumber"].IsNumberInteger()) {
        LOGE("OptionInfo endNumber json key is not exist or error.");
        return;
    }
    object["deviceAlias"].GetTo(optionInfo.deviceAlias);
    object["startNumber"].GetTo(optionInfo.startNumber);
    object["endNumber"].GetTo(optionInfo.endNumber);
}

MineSoftbusListener::MineSoftbusListener()
{
#if (defined(MINE_HARMONY))
    if (PublishDeviceDiscovery() != DM_OK) {
        LOGE("failed to publish device sn sha256 hash to softbus");
    }
    {
        std::lock_guard<std::mutex> autoLock(g_matchWaitDeviceLock);
        g_matchDealFlag = true;
        std::thread([]() { MatchSearchDealTask(); }).detach();
    }
#endif
    LOGI("MineSoftbusListener constructor");
}

MineSoftbusListener::~MineSoftbusListener()
{
#if (defined(MINE_HARMONY))
    if (StopPublishLNN(DM_PKG_NAME.c_str(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) != DM_OK) {
        LOGI("fail to unregister service public callback");
    }
    {
        std::lock_guard<std::mutex> autoLock(g_matchWaitDeviceLock);
        g_matchDealFlag = false;
    }
#endif
    LOGI("SoftbusConnector destructor");
}

int32_t MineSoftbusListener::RefreshSoftbusLNN(const string &pkgName, const string &searchJson,
    const DmSubscribeInfo &dmSubscribeInfo)
{
    LOGI("start to start discovery device with pkgName: %{public}s", pkgName.c_str());
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    if (ParseSearchJson(pkgName, searchJson, output, &outLen) != DM_OK) {
        LOGE("failed to parse searchJson with pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_JSON_PARSE_STRING;
    }
    SubscribeInfo subscribeInfo;
    SetSubscribeInfo(dmSubscribeInfo, subscribeInfo);
    if (SendBroadcastInfo(pkgName, subscribeInfo, output, outLen) != DM_OK) {
        LOGE("failed to start quick discovery beause sending broadcast info.");
        return ERR_DM_SOFTBUS_SEND_BROADCAST;
    }
    LOGI("start discovery device successfully with pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t MineSoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
    int retValue = StopRefreshLNN(DM_PKG_NAME, subscribeId);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to stop discovery device with ret: %{public}d", retValue);
        return retValue;
    }
    return DM_OK;
}

void MineSoftbusListener::OnPublishResult(int publishId, PublishResult reason)
{
    std::unique_lock<std::mutex> locker(g_publishLnnLock);
    if (reason == PUBLISH_LNN_SUCCESS) {
        g_publishLnnFlag = true;
        LOGI("publishLNN successfully with publishId: %{public}d.", publishId);
    } else {
        g_publishLnnFlag = false;
        LOGE("failed to publishLNN with publishId: %{public}d, reason: %{public}d.", publishId, (int)reason);
    }
    g_publishLnnNotify.notify_one();
}

void MineSoftbusListener::OnPublishDeviceFound(const DeviceInfo *deviceInfo)
{
    if (deviceInfo == nullptr) {
        LOGE("deviceInfo is nullptr.");
        return;
    }
#if (defined(MINE_HARMONY))
    if (deviceInfo->businessDataLen >= DISC_MAX_CUST_DATA_LEN ||
        deviceInfo->businessDataLen < DM_SEARCH_BROADCAST_MIN_LEN) {
        LOGE("deviceInfo data is too long or to short with dataLen: %{public}u", deviceInfo->businessDataLen);
        return;
    }
    LOGI("broadcast data is received with DataLen: %{public}u", deviceInfo->businessDataLen);
#endif
    std::unique_lock<std::mutex> autoLock(g_matchWaitDeviceLock);
    g_matchQueue.push_back(*deviceInfo);
    g_matchDealNotify.notify_one();
}

void MineSoftbusListener::OnRePublish(void)
{
    LOGI("try to rePublishLNN");
    int32_t retryTimes = 0;
    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_PASSIVE;
    publishInfo.medium = ExchangeMedium::COAP;
    publishInfo.freq = ExchangeFreq::LOW;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.capabilityData = nullptr;
    publishInfo.dataLen = 0;
    retryTimes = 0;
    while (PublishLNN(DM_PKG_NAME, &publishInfo, &publishLNNCallback_) != SOFTBUS_OK &&
        retryTimes <= MAX_RETRY_TIMES) {
        retryTimes++;
        LOGW("failed to rePublishLNN with retryTimes: %{public}d", retryTimes);
        usleep(SOFTBUS_CHECK_INTERVAL);
    }
    LOGI("rePublishLNN finish");
}

int32_t MineSoftbusListener::ParseSearchJson(const string &pkgName, const string &searchJson, char *output,
    size_t *outLen)
{
    JsonObject object(searchJson);
    if (object.IsDiscarded()) {
        LOGE("failed to parse filter options string.");
        return ERR_DM_INVALID_JSON_STRING;
    }
    int32_t retValue = DM_OK;
    uint32_t findMode = 0;
    if (IsUint32(object, FIELD_DEVICE_MODE)) {
        findMode = object[FIELD_DEVICE_MODE].Get<uint32_t>();
    }
    LOGI("quick search device mode is: %{public}u", findMode);
    switch (findMode) {
        case FIND_ALL_DEVICE:
            retValue = ParseSearchAllDevice(object, pkgName, output, outLen);
            break;
        case FIND_SCOPE_DEVICE:
            retValue = ParseSearchScopeDevice(object, pkgName, output, outLen);
            break;
        case FIND_VERTEX_DEVICE:
            retValue = ParseSearchVertexDevice(object, pkgName, output, outLen);
            break;
        default:
            LOGE("key type is not match key: %{public}s.", FIELD_DEVICE_MODE);
    }
    if (retValue != DM_OK) {
        LOGE("fail to parse search find device with ret: %{public}d.", retValue);
        return retValue;
    }
    LOGI("parse search json successfully with pkgName: %{public}s, outLen: %{public}zu,", pkgName.c_str(), *outLen);
    return DM_OK;
}

int32_t MineSoftbusListener::ParseSearchAllDevice(const JsonObject &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    broadcastHead.tlvDataLen = 0;
    broadcastHead.findMode = FIND_ALL_DEVICE;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = sizeof(BroadcastHead);
    return DM_OK;
}

int32_t MineSoftbusListener::ParseSearchScopeDevice(const JsonObject &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    if (!object.Contains(FIELD_FILTER_OPTIONS) || !object[FIELD_FILTER_OPTIONS].IsArray()) {
        LOGE("failed to get %{public}s scope cjson object or is not array.", FIELD_FILTER_OPTIONS);
        return ERR_DM_FAILED;
    }
    std::vector<ScopeOptionInfo> optionInfoVec;
    object[FIELD_FILTER_OPTIONS].Get(optionInfoVec);
    size_t optionInfoVecSize = optionInfoVec.size();
    if (optionInfoVecSize == 0 || optionInfoVecSize > DM_MAX_SCOPE_TLV_NUM) {
        LOGE("failed to get search josn array lenght.");
        return ERR_DM_INVALID_JSON_STRING;
    }
    LOGI("start to parse scope search array json with size:%{public}zu.", optionInfoVecSize);
    if (ParseScopeDeviceJsonArray(optionInfoVec, output + sizeof(BroadcastHead), outLen) != DM_OK) {
        LOGE("failed to parse scope json array.");
        return ERR_DM_FAILED;
    }

    broadcastHead.findMode = FIND_SCOPE_DEVICE;
    broadcastHead.tlvDataLen = *outLen;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = *outLen + sizeof(BroadcastHead);
    return DM_OK;
}

int32_t MineSoftbusListener::ParseSearchVertexDevice(const JsonObject &object, const string &pkgName, char *output,
    size_t *outLen)
{
    BroadcastHead broadcastHead;
    if (SetBroadcastHead(object, pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set broadcast head.");
        return ERR_DM_FAILED;
    }
    if (!object.Contains(FIELD_FILTER_OPTIONS) || !object[FIELD_FILTER_OPTIONS].IsArray()) {
        LOGE("failed to get %{public}s vertex cjson object or is not array.", FIELD_FILTER_OPTIONS);
        return ERR_DM_FAILED;
    }
    std::vector<VertexOptionInfo> optionInfoVec;
    object[FIELD_FILTER_OPTIONS].Get(optionInfoVec);
    size_t optionInfoVecSize = optionInfoVec.size();
    if (optionInfoVecSize == 0 || optionInfoVecSize > DM_MAX_VERTEX_TLV_NUM) {
        LOGE("failed to get search josn array lenght.");
        return ERR_DM_FAILED;
    }
    LOGI("start to parse vertex search array json with size: %{public}zu.", optionInfoVecSize);
    if (ParseVertexDeviceJsonArray(optionInfoVec, output + sizeof(BroadcastHead), outLen) != DM_OK) {
        LOGE("failed to parse vertex json array.");
        return ERR_DM_FAILED;
    }

    broadcastHead.findMode = FIND_VERTEX_DEVICE;
    broadcastHead.tlvDataLen = *outLen;
    AddHeadToBroadcast(broadcastHead, output);
    *outLen = *outLen + sizeof(BroadcastHead);
    return DM_OK;
}

int32_t MineSoftbusListener::SetBroadcastHead(const JsonObject &object, const string &pkgName,
    BroadcastHead &broadcastHead)
{
    broadcastHead.version = BROADCAST_VERSION;
    broadcastHead.headDataLen = sizeof(BroadcastHead);
    broadcastHead.tlvDataLen = 0;
    broadcastHead.findMode = 0;
    if (SetBroadcastTrustOptions(object, broadcastHead) != DM_OK) {
        LOGE("fail to set trust options to search broadcast.");
        return ERR_DM_FAILED;
    }
    if (SetBroadcastPkgname(pkgName, broadcastHead) != DM_OK) {
        LOGE("fail to set pkgname to search broadcast.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void MineSoftbusListener::AddHeadToBroadcast(const BroadcastHead &broadcastHead, char *output)
{
    size_t startPos = 0;
    output[startPos++] = broadcastHead.version;
    output[startPos++] = broadcastHead.headDataLen;
    output[startPos++] = broadcastHead.tlvDataLen;
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[startPos++] = broadcastHead.pkgNameHash[i];
    }
    output[startPos++] = broadcastHead.findMode;
    output[startPos++] = broadcastHead.trustFilter;
    LOGI("find device info with version: %{public}d, findMode: %{public}d, HeadLen: %{public}d, tlvDataLen: %{public}d,"
        "trustFilter: %{public}d", (int)(broadcastHead.version), (int)(broadcastHead.findMode),
        (int)(broadcastHead.headDataLen), (int)(broadcastHead.tlvDataLen), (int)(broadcastHead.trustFilter));
}

int32_t MineSoftbusListener::ParseScopeDeviceJsonArray(const vector<ScopeOptionInfo> &optionInfo,
    char *output, size_t *outLen)
{
    errno_t retValue = EOK;
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    size_t arraySize = optionInfo.size();

    for (size_t i = 0; i < arraySize; i++) {
        if (GetSha256Hash(optionInfo[i].deviceAlias.c_str(),
                          optionInfo[i].deviceAlias.size(), sha256Out) != DM_OK) {
            LOGE("failed to get sha256 hash with index: %{public}zu, value: %{public}s.", i,
                optionInfo[i].deviceAlias.c_str());
            return ERR_DM_FAILED;
        }
        output[(*outLen)++] = DEVICE_ALIAS_NUMBER;
        output[(*outLen)++] = DM_HASH_DATA_LEN;
        output[(*outLen)++] = DM_DEVICE_NUMBER_LEN;
        output[(*outLen)++] = DM_DEVICE_NUMBER_LEN;
        for (size_t j = 0; j < DM_HASH_DATA_LEN; j++) {
            output[(*outLen)++] = sha256Out[j];
        }
        retValue = sprintf_s(&output[*outLen], DM_DEVICE_NUMBER_LEN, "%010d", optionInfo[i].startNumber);
        if (retValue <= 0) {
            LOGE("fail to add device number to data buffer");
            return ERR_DM_FAILED;
        }
        *outLen = *outLen + DM_DEVICE_NUMBER_LEN;
        retValue = sprintf_s(&output[*outLen], DM_DEVICE_NUMBER_LEN, "%010d", optionInfo[i].endNumber);
        if (retValue <= 0) {
            LOGE("fail to add device number to data buffer");
            return ERR_DM_FAILED;
        }
        *outLen = *outLen + DM_DEVICE_NUMBER_LEN;
    }
    return DM_OK;
}

int32_t MineSoftbusListener::ParseVertexDeviceJsonArray(const std::vector<VertexOptionInfo> &optionInfo,
    char *output, size_t *outLen)
{
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    size_t arraySize = optionInfo.size();

    for (size_t i = 0; i < arraySize; i++) {
        if (optionInfo[i].type.empty() || optionInfo[i].value.empty()) {
            LOGE("failed to get type or value cjosn object with index: %{public}zu", i);
            continue;
        }
        if (optionInfo[i].type == "deviceUdid") {
            output[(*outLen)++] = DEVICE_UDID_TYPE;
        } else if (optionInfo[i].type == "deviceType") {
            output[(*outLen)++] = DEVICE_TYPE_TYPE;
        } else if (optionInfo[i].type == "deviceSn") {
            output[(*outLen)++] = DEVICE_SN_TYPE;
        } else {
            LOGE("type:%{public}s is not allowed with index: %{public}zu.", optionInfo[i].type.c_str(), i);
            return ERR_DM_FAILED;
        }
        output[(*outLen)++] = DM_HASH_DATA_LEN;
        if (GetSha256Hash((const char *) optionInfo[i].value.data(), optionInfo[i].value.size(),
                          sha256Out) != DM_OK) {
            LOGE("failed to get value sha256 hash with index: %{public}zu", i);
            return ERR_DM_GET_DATA_SHA256_HASH;
        }
        for (size_t j = 0; j < DM_HASH_DATA_LEN; j++) {
            output[(*outLen)++] = sha256Out[j];
        }
    }
    return DM_OK;
}

int32_t MineSoftbusListener::GetSha256Hash(const char *data, size_t len, char *output)
{
    if (data == nullptr || output == nullptr || len == 0) {
        LOGE("Input param invalied.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final((unsigned char *)output, &ctx);
    return DM_OK;
}

int32_t MineSoftbusListener::SetBroadcastTrustOptions(const JsonObject &object, BroadcastHead &broadcastHead)
{
    if (!object.Contains(FIELD_TRUST_OPTIONS)) {
        broadcastHead.trustFilter = 0;
        return DM_OK;
    } else if (object[FIELD_TRUST_OPTIONS].IsBoolean() && object[FIELD_TRUST_OPTIONS].Get<bool>()) {
        broadcastHead.trustFilter = FIND_TRUST_DEVICE;
        return DM_OK;
    } else if (object[FIELD_TRUST_OPTIONS].IsBoolean() && !object[FIELD_TRUST_OPTIONS].Get<bool>()) {
        broadcastHead.trustFilter = FIND_NOTRUST_DEVICE;
        return DM_OK;
    }
    LOGE("key type is error with key: %{public}s", FIELD_TRUST_OPTIONS);
    return ERR_DM_FAILED;
}

int32_t MineSoftbusListener::SetBroadcastPkgname(const string &pkgName, BroadcastHead &broadcastHead)
{
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char *)pkgName.c_str(), pkgName.size(), sha256Out) != DM_OK) {
        LOGE("failed to get search pkgName sha256 hash while search all device.");
        return ERR_DM_FAILED;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        broadcastHead.pkgNameHash[i] = sha256Out[i];
    }
    return DM_OK;
}

void MineSoftbusListener::SetSubscribeInfo(const DmSubscribeInfo &dmSubscribeInfo, SubscribeInfo &subscribeInfo)
{
    subscribeInfo.subscribeId = dmSubscribeInfo.subscribeId;
    subscribeInfo.mode = (DiscoverMode)dmSubscribeInfo.mode;
    subscribeInfo.medium = (ExchangeMedium)dmSubscribeInfo.medium;
    subscribeInfo.freq = (ExchangeFreq)dmSubscribeInfo.freq;
    subscribeInfo.isSameAccount = dmSubscribeInfo.isSameAccount;
    subscribeInfo.isWakeRemote = dmSubscribeInfo.isWakeRemote;
    subscribeInfo.capability = dmSubscribeInfo.capability;
    subscribeInfo.capabilityData = nullptr;
    subscribeInfo.dataLen = 0;
}

int32_t MineSoftbusListener::SendBroadcastInfo(const string &pkgName, SubscribeInfo &subscribeInfo, char *output,
    size_t outputLen)
{
    size_t base64OutLen = 0;
    int retValue;
    char base64Out[DISC_MAX_CUST_DATA_LEN] = {0};
    retValue = DmBase64Encode(base64Out, DISC_MAX_CUST_DATA_LEN, output, outputLen, base64OutLen);
    if (retValue != 0) {
        LOGE("failed to get search data base64 encode type data with ret: %{public}d.", retValue);
        return ERR_DM_FAILED;
    }
#if (defined(MINE_HARMONY))
    subscribeInfo.custData = base64Out;
    subscribeInfo.custDataLen = base64OutLen;
    IRefreshCallback softbusRefreshCallback_ = SoftbusListener::GetSoftbusRefreshCb();
    retValue = RefreshLNN(DM_PKG_NAME, &subscribeInfo, &softbusRefreshCallback_);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to start to refresh quick discovery with ret: %{public}d.", retValue);
        return ERR_DM_FAILED;
    }
#endif
    LOGI("send search broadcast info by softbus successfully with dataLen: %{public}zu, pkgName: %{public}s.",
        base64OutLen, pkgName.c_str());
    return DM_OK;
}

int32_t MineSoftbusListener::DmBase64Encode(char *output, size_t outputLen, const char *input,
    size_t inputLen, size_t &base64OutLen)
{
    LOGI("MineSoftbusListener::DmBase64Encode");
    if (output == nullptr || input == nullptr || outputLen == 0 || inputLen == 0) {
        LOGE("Input param invalied.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t outLen = 0;
    base64OutLen = 0;
    EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
    if (ctx == nullptr) {
        LOGE("create ctx failed.");
        EVP_ENCODE_CTX_free(ctx);
        return ERR_DM_FAILED;
    }
    EVP_EncodeInit(ctx);
    if (EVP_EncodeUpdate(ctx, (unsigned char *)output, &outLen, (const unsigned char *)input, inputLen) != EVP_OK) {
        LOGE("EVP_EncodeUpdate failed.");
        EVP_ENCODE_CTX_free(ctx);
        return ERR_DM_FAILED;
    }
    base64OutLen += static_cast<size_t>(outLen);
    EVP_EncodeFinal(ctx, (unsigned char *)(output + outLen), &outLen);
    base64OutLen += static_cast<size_t>(outLen);
    EVP_ENCODE_CTX_free(ctx);
    return DM_OK;
}

int32_t MineSoftbusListener::DmBase64Decode(char *output, size_t outputLen, const char *input,
    size_t inputLen, size_t &base64OutLen)
{
    LOGI("MineSoftbusListener::DmBase64Decode");
    if (output == nullptr || outputLen == 0 || input == nullptr || inputLen == 0) {
        LOGE("Input param invalied.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    base64OutLen = 0;
    int32_t outLen = 0;
    EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
    if (ctx == nullptr) {
        LOGE("create ctx failed.");
        EVP_ENCODE_CTX_free(ctx);
        return ERR_DM_FAILED;
    }
    EVP_DecodeInit(ctx);
    if (EVP_DecodeUpdate(ctx, (unsigned char *)output, &outLen, (const unsigned char *)input, inputLen) != EVP_OK) {
        LOGE("EVP_DecodeUpdate failed.");
        EVP_ENCODE_CTX_free(ctx);
        return ERR_DM_FAILED;
    }
    base64OutLen += static_cast<size_t>(outLen);
    if (EVP_DecodeFinal(ctx, (unsigned char *)(output + outLen), &outLen) != EVP_OK) {
        LOGE("EVP_DecodeFinal failed.");
        EVP_ENCODE_CTX_free(ctx);
        return ERR_DM_FAILED;
    }
    base64OutLen += static_cast<size_t>(outLen);
    EVP_ENCODE_CTX_free(ctx);
    return DM_OK;
}

int32_t MineSoftbusListener::PublishDeviceDiscovery(void)
{
    PublishInfo publishInfo;
    publishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    publishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    publishInfo.medium = ExchangeMedium::AUTO;
    publishInfo.freq = ExchangeFreq::LOW;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.capabilityData = nullptr;
    publishInfo.dataLen = 0;
    int retValue = PublishLNN(DM_PKG_NAME, &publishInfo, &publishLNNCallback_);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to call softbus publishLNN function with ret: %{public}d.", retValue);
        return retValue;
    }
    std::chrono::seconds timeout = std::chrono::seconds(MAX_SOFTBUS_DELAY_TIME);
    std::unique_lock<std::mutex> locker(g_publishLnnLock);
    if (!g_publishLnnNotify.wait_for(locker, timeout, [] { return g_publishLnnFlag; })) {
        g_publishLnnFlag = false;
        return ERR_DM_SOFTBUS_PUBLISH_SERVICE;
    }
    g_publishLnnFlag = false;
    return DM_OK;
}

void MineSoftbusListener::MatchSearchDealTask(void)
{
    LOGI("the match deal task has started to run.");
#if (defined(MINE_HARMONY))
    DeviceInfo tempDeviceInfo;
    while (true) {
        {
            std::unique_lock<std::mutex> autoLock(g_matchWaitDeviceLock);
            if (!g_matchDealFlag) {
                LOGI("the match deal task will stop to run.");
                return;
            }
            g_matchDealNotify.wait(autoLock, [] { return !g_matchQueue.empty(); });
            tempDeviceInfo = g_matchQueue.front();
            g_matchQueue.pop_front();
        }
        if (ParseBroadcastInfo(tempDeviceInfo) != DM_OK) {
            LOGE("failed to parse broadcast info.");
        }
    }
#endif
}

int32_t MineSoftbusListener::ParseBroadcastInfo(DeviceInfo &deviceInfo)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    if (!GetBroadcastData(deviceInfo, output, DISC_MAX_CUST_DATA_LEN)) {
        LOGE("fail to get broadcast data");
        return ERR_DM_FAILED;
    }
    DevicePolicyInfo devicePolicyInfo;
    Action matchResult = BUSINESS_EXACT_NOT_MATCH;
    BroadcastHead broadcastHead = *(BroadcastHead *)output;
    LOGI("parse device info with version: %{public}d, findMode: %{public}d, HeadLen: %{public}d, tlvDataLen:"
        "%{public}d, trustFilter: %{public}d", (int)(broadcastHead.version), (int)(broadcastHead.findMode),
        (int)(broadcastHead.headDataLen), (int)(broadcastHead.tlvDataLen), (int)broadcastHead.trustFilter);

    char findMode = broadcastHead.findMode;
    switch (findMode) {
        case FIND_ALL_DEVICE:
            matchResult = MatchSearchAllDevice(deviceInfo, broadcastHead);
            break;
        case FIND_SCOPE_DEVICE:
            GetScopeDevicePolicyInfo(devicePolicyInfo);
            matchResult = MatchSearchScopeDevice(deviceInfo, output + sizeof(BroadcastHead),
                devicePolicyInfo, broadcastHead);
            break;
        case FIND_VERTEX_DEVICE:
            GetVertexDevicePolicyInfo(devicePolicyInfo);
            matchResult = MatchSearchVertexDevice(deviceInfo, output + sizeof(BroadcastHead),
                devicePolicyInfo, broadcastHead);
            break;
        default:
            LOGE("key type is not match key: %{public}s.", FIELD_DEVICE_MODE);
            return ERR_DM_FAILED;
    }
    LOGI("parse broadcast info matchResult: %{public}d.", (int)matchResult);
    if (matchResult == BUSINESS_EXACT_MATCH) {
        return SendReturnwave(deviceInfo, broadcastHead, matchResult);
    }
    return DM_OK;
}

bool MineSoftbusListener::GetBroadcastData(DeviceInfo &deviceInfo, char *output, size_t outLen)
{
    size_t base64OutLen = 0;
#if (defined(MINE_HARMONY))
    size_t dataLen = deviceInfo.businessDataLen;
    unsigned char *data = (unsigned char *)deviceInfo.businessData;
    int retValue = DmBase64Decode(output, outLen, data, dataLen, base64OutLen);
    if (retValue != 0) {
        LOGE("failed to with ret: %{public}d.", retValue);
        return false;
    }
    if (base64OutLen < DM_SEARCH_BROADCAST_MIN_LEN) {
        LOGE("data length too short with outLen: %{public}zu.", base64OutLen);
        return false;
    }
#endif
    BroadcastHead *broadcastHead = (BroadcastHead *)output;
    size_t hDataLen = broadcastHead->headDataLen;
    size_t tlvDataLen = broadcastHead->tlvDataLen;
    if (hDataLen >= DISC_MAX_CUST_DATA_LEN || tlvDataLen >= DISC_MAX_CUST_DATA_LEN ||
        (hDataLen + tlvDataLen) != base64OutLen) {
        LOGE("data lenght is not valid with: headDataLen: %{public}zu, tlvDataLen: %{public}zu, base64OutLen:"
            "%{public}zu.", hDataLen, tlvDataLen, base64OutLen);
        return false;
    }
    return true;
}

Action MineSoftbusListener::MatchSearchAllDevice(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead)
{
    if (broadcastHead.trustFilter == 0) {
        return BUSINESS_EXACT_MATCH;
    }
    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_MATCH;
    } else if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_NOTRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (!(deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else {
        return BUSINESS_EXACT_MATCH;
    }
}

void MineSoftbusListener::GetScopeDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo)
{
    devicePolicyInfo.numberValid = false;
    devicePolicyInfo.aliasHashValid = false;
    if (GetDeviceAliasHash(devicePolicyInfo.aliasHash)) {
        devicePolicyInfo.aliasHashValid = true;
    }
    if (GetDeviceNumber(devicePolicyInfo.number)) {
        devicePolicyInfo.numberValid = true;
    }
}

Action MineSoftbusListener::MatchSearchScopeDevice(DeviceInfo &deviceInfo, char *output,
    const DevicePolicyInfo &devicePolicyInfo, const BroadcastHead &broadcastHead)
{
    vector<int> matchItemNum(DM_MAX_SCOPE_TLV_NUM, 0);
    vector<int> matchItemResult(DM_MAX_SCOPE_TLV_NUM, 0);

    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_NOTRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (!(deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    }

    size_t tlvLen = broadcastHead.tlvDataLen;
    const size_t ONE_TLV_DATA_LEN = DM_TLV_SCOPE_DATA_OFFSET + DM_HASH_DATA_LEN +
        DM_DEVICE_NUMBER_LEN + DM_DEVICE_NUMBER_LEN;
    for (size_t i = 0; (i + ONE_TLV_DATA_LEN) <= tlvLen; i += ONE_TLV_DATA_LEN) {
        if (output[i] == DEVICE_ALIAS_NUMBER) {
            size_t dataPosition = i + DM_TLV_SCOPE_DATA_OFFSET;
            size_t startNumberPosition = dataPosition + DM_HASH_DATA_LEN;
            int startNumber = atoi(&output[startNumberPosition]);
            if (startNumber == 0) {
                LOGE("the value of start device number is not allowed");
                continue;
            }
            size_t endNumberPosition = startNumberPosition + DM_DEVICE_NUMBER_LEN;
            int endNumber = atoi(&output[endNumberPosition]);
            if (endNumber == 0) {
                LOGE("the value of end device number is not allowed.");
                continue;
            }
            matchItemNum[DEVICE_ALIAS_NUMBER] = 1;
            if (CheckDeviceAliasMatch(devicePolicyInfo, &output[dataPosition]) &&
                CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber)) {
                matchItemResult[DEVICE_ALIAS_NUMBER] = 1;
            }
        } else {
            LOGE("the value of type is not allowed with type: %{public}u.", output[i]);
            continue;
        }
    }
    return GetMatchResult(matchItemNum, matchItemResult);
}

void MineSoftbusListener::GetVertexDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo)
{
    devicePolicyInfo.snHashValid = false;
    devicePolicyInfo.typeHashValid = false;
    devicePolicyInfo.udidHashValid = false;
    if (GetDeviceSnHash(devicePolicyInfo.snHash)) {
        devicePolicyInfo.snHashValid = true;
    }
    if (GetDeviceUdidHash(devicePolicyInfo.udidHash)) {
        devicePolicyInfo.udidHashValid = true;
    }
    if (GetDeviceTypeHash(devicePolicyInfo.typeHash)) {
        devicePolicyInfo.typeHashValid = true;
    }
}

Action MineSoftbusListener::MatchSearchVertexDevice(DeviceInfo &deviceInfo, char *output,
    const DevicePolicyInfo &devicePolicyInfo, const BroadcastHead &broadcastHead)
{
    vector<int> matchItemNum(DM_MAX_VERTEX_TLV_NUM, 0);
    vector<int> matchItemResult(DM_MAX_VERTEX_TLV_NUM, 0);

    if ((deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_NOTRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (!(deviceInfo.isOnline) && (broadcastHead.trustFilter == FIND_TRUST_DEVICE)) {
        return BUSINESS_EXACT_NOT_MATCH;
    }

    size_t tlvLen = broadcastHead.tlvDataLen;
    const size_t ONE_TLV_DATA_LEN = DM_TLV_VERTEX_DATA_OFFSET + DM_HASH_DATA_LEN;
    for (size_t i = 0; (i + ONE_TLV_DATA_LEN) <= tlvLen; i += ONE_TLV_DATA_LEN) {
        size_t dataPosition = 0;
        if (output[i] == DEVICE_TYPE_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_TYPE_TYPE] = 1;
            if (CheckDeviceTypeMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_TYPE_TYPE] = 1;
            }
        } else if (output[i] == DEVICE_SN_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_SN_TYPE] = 1;
            if (CheckDeviceSnMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_SN_TYPE] = 1;
            }
        } else if (output[i] == DEVICE_UDID_TYPE) {
            dataPosition = i + DM_TLV_VERTEX_DATA_OFFSET;
            matchItemNum[DEVICE_UDID_TYPE] = 1;
            if (CheckDeviceUdidMatch(devicePolicyInfo, &output[dataPosition])) {
                matchItemResult[DEVICE_UDID_TYPE] = 1;
            }
        } else {
            LOGE("the value of type is not allowed with type: %{public}u.", output[i]);
        }
    }
    return GetMatchResult(matchItemNum, matchItemResult);
}

int32_t MineSoftbusListener::SendReturnwave(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead,
    Action matchResult)
{
    size_t outLen = 0;
    unsigned char outData[DISC_MAX_CUST_DATA_LEN] = {0};
    outData[outLen++] = BROADCAST_VERSION;
    outData[outLen++] = sizeof(ReturnwaveHead);
    size_t base64OutLen = 0;
#if (defined(MINE_HARMONY))
    if (HiChainConnector::IsCredentialExist()) {
        outData[outLen++] = true;
    } else {
        outData[outLen++] = false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        outData[outLen++] = broadcastHead.pkgNameHash[i];
    }
    int retValue;

    int retValue = DmBase64Encode((char *)(deviceInfo.businessData), DISC_MAX_CUST_DATA_LEN,
        outData, outLen, base64OutLen);
    if (retValue != 0) {
        LOGE("failed to get search data base64 encode type data with ret: %{public}d.", retValue);
        return ERR_DM_FAILED;
    }
    deviceInfo.businessData[base64OutLen] = '\0';
    retValue = SetDiscoveryPolicy(DM_PKG_NAME, &deviceInfo, (int32_t)matchResult);
    if (retValue != SOFTBUS_OK) {
        LOGE("failed to set discovery policy with ret: %{public}d.", retValue);
    }
#endif
    LOGI("set discovery policy successfully with dataLen: %{public}zu.", base64OutLen);
    return DM_OK;
}

bool MineSoftbusListener::GetDeviceAliasHash(char *output)
{
    char deviceAlias[DM_MAX_DEVICE_ALIAS_LEN + 1] = {0};
    int32_t retValue = GetParameter(DEVICE_ALIAS, "not exist", deviceAlias, DM_MAX_DEVICE_ALIAS_LEN);
    if (retValue < 0 || strcmp((const char *)deviceAlias, "not exist") == 0) {
        LOGE("failed to get device alias from system parameter with ret: %{public}d.", retValue);
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char *)deviceAlias, strlen(deviceAlias), sha256Out) != DM_OK) {
        LOGE("failed to generated device alias sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool MineSoftbusListener::GetDeviceSnHash(char *output)
{
    const char *deviceSn = GetSerial();
    if (deviceSn == NULL) {
        LOGE("failed to get device sn from system parameter.");
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char *)deviceSn, strlen(deviceSn), sha256Out) != DM_OK) {
        LOGE("failed to generated device sn sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool MineSoftbusListener::GetDeviceUdidHash(char *output)
{
    char deviceUdid[DM_MAX_DEVICE_UDID_LEN + 1] = {0};
    int32_t retValue = GetDevUdid(deviceUdid, DM_MAX_DEVICE_UDID_LEN);
    if (retValue != 0) {
        LOGE("failed to get local device udid with ret: %{public}d.", retValue);
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char *)deviceUdid, strlen(deviceUdid), sha256Out) != DM_OK) {
        LOGE("failed to generated device udid sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool MineSoftbusListener::GetDeviceTypeHash(char *output)
{
    const char *deviceType = GetDeviceType();
    if (deviceType == NULL) {
        LOGE("failed to get device type from system parameter.");
        return false;
    }
    char sha256Out[SHA256_OUT_DATA_LEN] = {0};
    if (GetSha256Hash((const char *)deviceType, strlen(deviceType), sha256Out) != DM_OK) {
        LOGE("failed to generated device type sha256 hash.");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        output[i] = sha256Out[i];
    }
    return true;
}

bool MineSoftbusListener::GetDeviceNumber(char *output)
{
    char deviceNumber[DM_DEVICE_NUMBER_LEN + 1] = {0};
    int32_t retValue = GetParameter(DEVICE_NUMBER, "not exist", deviceNumber, DM_DEVICE_NUMBER_LEN);
    if (retValue < 0 || strcmp((const char *)deviceNumber, "not exist") == 0) {
        LOGE("failed to get device number from system parameter with ret: %{public}d.", retValue);
        return false;
    }
    for (size_t i = 0; i < DM_DEVICE_NUMBER_LEN; i++) {
        output[i] = deviceNumber[i];
    }
    return true;
}

bool MineSoftbusListener::CheckDeviceAliasMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.aliasHashValid) {
        LOGE("device alias is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.aliasHash[i]) {
            LOGI("device alias is not match.");
            return false;
        }
    }
    LOGI("device alias is match.");
    return true;
}

bool MineSoftbusListener::CheckDeviceNumberMatch(const DevicePolicyInfo &devicePolicyInfo,
    int32_t startNumber, int32_t endNumber)
{
    if (!devicePolicyInfo.numberValid) {
        LOGE("device number is not valid");
        return false;
    }
    if (startNumber <= DM_INVALID_DEVICE_NUMBER || endNumber <= DM_INVALID_DEVICE_NUMBER) {
        LOGI("device number is match");
        return true;
    }
    int deviceNumber = atoi((const char *)devicePolicyInfo.number);
    if (deviceNumber < startNumber || deviceNumber > endNumber) {
        LOGI("device number is not match.");
        return false;
    }
    LOGI("device number is match.");
    return true;
}

bool MineSoftbusListener::CheckDeviceSnMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.snHashValid) {
        LOGE("device sn is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.snHash[i]) {
            LOGI("device sn is not match.");
            return false;
        }
    }
    LOGI("device sn is match.");
    return true;
}

bool MineSoftbusListener::CheckDeviceTypeMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.typeHashValid) {
        LOGE("device type is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.typeHash[i]) {
            LOGI("device type is not match.");
            return false;
        }
    }
    LOGI("device type is match.");
    return true;
}

bool MineSoftbusListener::CheckDeviceUdidMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data)
{
    if (!devicePolicyInfo.udidHashValid) {
        LOGE("device udid is not valid");
        return false;
    }
    for (size_t i = 0; i < DM_HASH_DATA_LEN; i++) {
        if (data[i] != devicePolicyInfo.udidHash[i]) {
            LOGI("device udid is not match.");
            return false;
        }
    }
    LOGI("device udid is match.");
    return true;
}

Action MineSoftbusListener::GetMatchResult(const vector<int> &matchItemNum, const vector<int> &matchItemResult)
{
    int matchItemSum = 0;
    int matchResultSum = 0;
    size_t matchItemNumLen = matchItemNum.size();
    size_t matchItemResultLen = matchItemResult.size();
    size_t minLen = (matchItemNumLen >= matchItemResultLen ? matchItemResultLen : matchItemNumLen);
    for (size_t i = 0; i < minLen; i++) {
        matchResultSum += matchItemResult[i];
        matchItemSum += matchItemNum[i];
    }
    if (matchResultSum == 0) {
        return BUSINESS_EXACT_NOT_MATCH;
    } else if (matchItemSum == matchResultSum) {
        return BUSINESS_EXACT_MATCH;
    } else {
        return BUSINESS_PARTIAL_MATCH;
    }
}
} // namespace DistributedHardware
} // namespace OHOS