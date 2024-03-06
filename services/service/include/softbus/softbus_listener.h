/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_LISTENER_H
#define OHOS_DM_SOFTBUS_LISTENER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_radar_helper.h"
#include "dm_timer.h"
#include "dm_subscribe_info.h"
#include "i_softbus_discovering_callback.h"
#include "inner_session.h"
#include "session.h"
#include "socket.h"
#include "nlohmann/json.hpp"

using std::vector;
using std::string;
using json = nlohmann::json;

namespace OHOS {
namespace DistributedHardware {
    constexpr uint32_t DM_HASH_DATA_LEN = 16;
constexpr uint32_t DM_DEVICE_NUMBER_LEN = 11;
constexpr uint32_t DM_DE_LEN = 16;


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
}  ScopeOptionInfo;

typedef struct {
    char version;
    char dataLen;
    bool IsExistCredential;
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

class SoftbusListener {
public:
    SoftbusListener();
    ~SoftbusListener();

    static void OnSoftbusDeviceFound(const DeviceInfo *device);
    static void OnSoftbusDiscoveryResult(int subscribeId, RefreshResult result);
    static void OnSoftbusDeviceOnline(NodeBasicInfo *info);
    static void OnSoftbusDeviceOffline(NodeBasicInfo *info);
    static void OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);
    static void OnParameterChgCallback(const char *key, const char *value, void *context);
    static void OnSoftbusPublishResult(int publishId, PublishResult result);
    static void DeviceOnLine(DmDeviceInfo deviceInfo);
    static void DeviceOffLine(DmDeviceInfo deviceInfo);
    static void DeviceNameChange(DmDeviceInfo deviceInfo);

    static void CacheDiscoveredDevice(const DeviceInfo *device);
    static void ClearDiscoveredDevice();
    static void ConvertDeviceInfoToDmDevice(const DeviceInfo &device, DmDeviceInfo &dmDevice);
    static int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);
    static int32_t GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
        ConnectionAddrType &addrType);
    static int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceInfo &devInfo);
    static int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeInfo, DmDeviceBasicInfo &devInfo);
    static std::string ConvertBytesToUpperCaseHexString(const uint8_t arr[], const size_t size);

    int32_t InitSoftbusListener();
    int32_t GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList);
    int32_t GetAvailableDeviceList(std::vector<DmDeviceBasicInfo> &deviceBasicInfoList);
    int32_t GetLocalDeviceInfo(DmDeviceInfo &deviceInfo);
    int32_t GetLocalDeviceNetworkId(std::string &networkId);
    int32_t GetLocalDeviceName(std::string &deviceName);
    int32_t GetLocalDeviceType(int32_t &deviceType);
    int32_t GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info);
    int32_t ShiftLNNGear();
    int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);
    int32_t GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel);
    int32_t GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType);
    int32_t RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo, const std::string &customData);
    int32_t StopRefreshSoftbusLNN(uint16_t subscribeId);
    int32_t PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability,
        const std::string &customData);
    int32_t StopPublishSoftbusLNN(int32_t publishId);
    int32_t StartDiscovery(const string &pkgName, const string &searchJson, const DmSubscribeInfo &subscribeInfo);
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
    static int32_t ParseVertexDeviceJsonArray(const std::vector<VertexOptionInfo> &optionInfo, char *output,
                                              size_t *outLen);
    static int32_t GetSha256Hash(const char *data, size_t len, char *output);
    int32_t SetBroadcastTrustOptions(const json &object, BroadcastHead &broadcastHead);
    int32_t SetBroadcastPkgname(const string &pkgName, BroadcastHead &broadcastHead);
    int32_t SendBroadcastInfo(const string &pkgName, SubscribeInfo &subscribeInfo, char *input, size_t *inputLen);
    void SetSubscribeInfo(const DmSubscribeInfo &dmSubscribeInfo, SubscribeInfo &subscribeInfo);
#if (defined(MINE_HARMONY))
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
    static bool ChecKDeviceSnMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static bool ChecKDeviceTypeMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static bool ChecKDeviceUdidMatch(const DevicePolicyInfo &devicePolicyInfo, const char *data);
    static Action GetMatchResult(const vector<int> &matchItemNum, const vector<int> &matchItemResult);
    static int32_t DmBase64Encode(char *output, size_t outputLen, const char *input,
        size_t inputLen, size_t &base64OutLen);
    static int32_t DmBase64Decode(char *output, size_t outputLen, const unsigned char *input,
        size_t inputLen, size_t &base64OutLen);
#endif

private:
    static bool isRadarSoLoad_;
    static IDmRadarHelper *dmRadarHelper_;
    static std::shared_ptr<DmTimer> timer_;
    static void *radarHandle_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_LISTENER_H
