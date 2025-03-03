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

#ifndef OHOS_DM_MINE_SOFTBUS_LISTENER_H
#define OHOS_DM_MINE_SOFTBUS_LISTENER_H

#include "softbus_listener.h"

using std::vector;
using std::string;
using json = nlohmann::json;

namespace OHOS {
namespace DistributedHardware {

#define DM_DEVICE_NUMBER_LEN 11
#define DM_HASH_DATA_LEN 16

typedef struct {
    string type;
    string value;
} VertexOptionInfo;

typedef struct {
    char version;
    char headDataLen;
    char tlvDataLen;
    char pkgNameHash[DM_HASH_DATA_LEN];
    char findMode;
    char trustFilter;
} BroadcastHead;

typedef struct {
    string deviceAlias;
    int32_t startNumber;
    int32_t endNumber;
} ScopeOptionInfo;

typedef struct {
    char version;
    char dataLen;
    bool isExistCredential;
    char pkgNameHash[DM_HASH_DATA_LEN];
} ReturnwaveHead;

typedef struct {
    bool snHashValid;
    bool typeHashValid;
    bool udidHashValid;
    bool aliasHashValid;
    bool numberValid;
    char snHash[DM_HASH_DATA_LEN];
    char typeHash[DM_HASH_DATA_LEN];
    char udidHash[DM_HASH_DATA_LEN];
    char aliasHash[DM_HASH_DATA_LEN];
    char number[DM_DEVICE_NUMBER_LEN];
} DevicePolicyInfo;

typedef enum {
    BUSINESS_EXACT_MATCH = 0x01,
    BUSINESS_PARTIAL_MATCH,
    BUSINESS_EXACT_NOT_MATCH,
} Action;

class MineSoftbusListener {
public:
    MineSoftbusListener();
    ~MineSoftbusListener();

    int32_t StopRefreshSoftbusLNN(uint16_t subscribeId);
    int32_t RefreshSoftbusLNN(const string &pkgName, const string &searchJson, const DmSubscribeInfo &subscribeInfo);
    int32_t RegisterSoftbusLnnOpsCbk(const std::string &pkgName,
        const std::shared_ptr<ISoftbusDiscoveringCallback> callback);
    int32_t UnRegisterSoftbusLnnOpsCbk(const std::string &pkgName);
    static void OnPublishResult(int publishId, PublishResult reason);
    static void OnPublishDeviceFound(const DeviceInfo *deviceInfo);
    static void OnRePublish(void);
    static IDmRadarHelper* GetDmRadarHelperObj();
    static bool IsDmRadarHelperReady();
    static bool CloseDmRadarHelperObj(std::string name);
private:
    int32_t InitSoftPublishLNN();
    int32_t ParseSearchJson(const string &pkgName, const string &searchJsonStr, char *output, size_t *outLen);
    int32_t ParseSearchAllDevice(const json &object, const string &pkgName, char *output, size_t *outLen);
    int32_t ParseSearchScopeDevice(const json &object, const string &pkgName, char *output, size_t *outLen);
    int32_t ParseSearchVertexDevice(const json &object, const string &pkgName, char *output, size_t *outLen);
    int32_t SetBroadcastHead(const json &object, const string &pkgName, BroadcastHead &broadcastHead);
    void AddHeadToBroadcast(const BroadcastHead &broadcastHead, char *output);
    int32_t ParseScopeDeviceJsonArray(const std::vector<ScopeOptionInfo> &optionInfo, char *output, size_t *outLen);
    int32_t ParseVertexDeviceJsonArray(const std::vector<VertexOptionInfo> &optionInfo, char *output, size_t *outLen);
    static int32_t GetSha256Hash(const char *data, size_t len, char *output);
    int32_t SetBroadcastTrustOptions(const json &object, BroadcastHead &broadcastHead);
    int32_t SetBroadcastPkgname(const string &pkgName, BroadcastHead &broadcastHead);
    int32_t SendBroadcastInfo(const string &pkgName, SubscribeInfo &subscribeInfo, char *output, size_t outputLen);
    void SetSubscribeInfo(const DmSubscribeInfo &dmSubscribeInfo, SubscribeInfo &subscribeInfo);
    int32_t PublishDeviceDiscovery(void);
    static void MatchSearchDealTask(void);
    static int32_t ParseBroadcastInfo(DeviceInfo &deviceInfo);
    static bool GetBroadcastData(DeviceInfo &deviceInfo, char *output, size_t outLen);
    static Action MatchSearchAllDevice(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead);
    static void GetScopeDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo);
    static Action MatchSearchScopeDevice(DeviceInfo &deviceInfo, char *output,
        const DevicePolicyInfo &devicePolicyInfo, const BroadcastHead &broadcastHead);
    static void GetVertexDevicePolicyInfo(DevicePolicyInfo &devicePolicyInfo);
    static Action MatchSearchVertexDevice(DeviceInfo &deviceInfo, char *output,
        const DevicePolicyInfo &devicePolicyInfo, const BroadcastHead &broadcastHead);
    static int32_t SendReturnwave(DeviceInfo &deviceInfo, const BroadcastHead &broadcastHead, Action matchResult);
    static bool GetDeviceAliasHash(char *output);
    static bool GetDeviceSnHash(char *output);
    static bool GetDeviceUdidHash(char *output);
    static bool GetDeviceTypeHash(char *output);
    static bool GetDeviceNumber(char *output);
    static bool CheckDeviceAliasMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static bool CheckDeviceNumberMatch(const DevicePolicyInfo &devicePolicyInfo,
        int32_t startNumber, int32_t endNumber);
    static bool CheckDeviceSnMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static bool CheckDeviceTypeMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static bool CheckDeviceUdidMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static Action GetMatchResult(const vector<int> &matchItemNum, const vector<int> &matchItemResult);
    int32_t DmBase64Encode(char *output, size_t outputLen, const char *input, size_t inputLen, size_t &base64OutLen);
    int32_t DmBase64Decode(char *output, size_t outputLen, const char *input, size_t inputLen, size_t &base64OutLen);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_MINE_SOFTBUS_LISTENER_H
