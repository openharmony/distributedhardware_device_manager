/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_DEVICE_INFO_H
#define OHOS_DM_DEVICE_INFO_H

#include <map>
#include <string>

#include "dm_app_image_info.h"
#include "dm_subscribe_info.h"

#define DM_MAX_DEVICE_ID_LEN (97)
#define DM_MAX_DEVICE_NAME_LEN (129)
#define DM_MAX_PIN_CODE_LEN (1025)
#define DM_MAX_META_TOKEN_LEN (9)

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {
/**
 * @brief Device manager event notify.
 */
typedef enum DmNotifyEvent {
    /**
     * Device manager start event.
     */
    DM_NOTIFY_EVENT_START = 0,
    /**
     * Device manager on ready event.
     */
    DM_NOTIFY_EVENT_ONDEVICEREADY,
    /**
     * Device manager on pin holder event.
     */
    DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT,
    // Add event here
    DM_NOTIFY_EVENT_BUTT,
} DmNotifyEvent;

/**
 * @brief Device Type definitions.
 */
typedef enum DmDeviceType {
    /**
     * Indicates an unknown device type.
     */
    DEVICE_TYPE_UNKNOWN = 0x00,
    /**
     * Indicates a smart camera.
     */
    DEVICE_TYPE_WIFI_CAMERA = 0x08,
    /**
     * Indicates a smart speaker.
     */
    DEVICE_TYPE_AUDIO = 0x0A,
    /**
     * Indicates a smart pc.
     */
    DEVICE_TYPE_PC = 0x0C,
    /**
     * Indicates a smart phone.
     */
    DEVICE_TYPE_PHONE = 0x0E,
    /**
     * Indicates a smart pad.
     */
    DEVICE_TYPE_PAD = 0x11,
    /**
     * Indicates a smart watch.
     */
    DEVICE_TYPE_WATCH = 0x6D,
    /**
     * Indicates a car.
     */
    DEVICE_TYPE_CAR = 0x83,
    /**
     * Indicates a smart TV.
     */
    DEVICE_TYPE_TV = 0x9C,
    /**
     * Indicates smart display
    */
    DEVICE_TYPE_SMART_DISPLAY = 0xA02,
    /**
     * Indicates 2in1
    */
    DEVICE_TYPE_2IN1 = 0xA2F,
    THIRD_TV = 0x2E,
    /**
     *  Indicates ai glasses.
     */
    DEVICE_TYPE_GLASSES = 0xA31,
} DmDeviceType;

/**
 * @brief Device state change event definition.
 */
typedef enum DmDeviceState {
    /**
     * Device status is unknown.
     */
    DEVICE_STATE_UNKNOWN = -1,
    /**
     * Device online action, which indicates the device is physically online.
     */
    DEVICE_STATE_ONLINE = 0,
    /**
     * Device ready action, which indicates the information between devices has
     * been synchronized in the Distributed Data Service (DDS) module,
     * and the device is ready for running distributed services.
     */
    DEVICE_INFO_READY = 1,
    /**
     * Device offline action, which Indicates the device is physically offline.
     */
    DEVICE_STATE_OFFLINE = 2,
    /**
     * Device change action, which Indicates the device is physically change.
     */
    DEVICE_INFO_CHANGED = 3,
} DmDeviceState;

/**
 * @brief Device authentication form.
 */
typedef enum DmAuthForm {
    /**
     * Device Auth invalid.
     */
    INVALID_TYPE = -1,
    /**
     * Peer To Peer Device auth.
     */
    PEER_TO_PEER = 0,
    /**
     * Identical Account Device auth.
     */
    IDENTICAL_ACCOUNT = 1,
    /**
     * Across Account Device auth.
     */
    ACROSS_ACCOUNT = 2,
    /**
     * Share.
     */
    SHARE = 3,
} DmAuthForm;

/**
 * @brief Device Information.
 */
typedef struct DmDeviceInfo {
    /**
     * Device Id of the device.
     */
    char deviceId[DM_MAX_DEVICE_ID_LEN] = {0};
    /**
     * Device name of the device.
     */
    char deviceName[DM_MAX_DEVICE_NAME_LEN] = {0};
    /**
     * Device type of the device.
     */
    uint16_t deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    /**
     * NetworkId of the device.
     */
    char networkId[DM_MAX_DEVICE_ID_LEN] = {0};
    /**
     * The distance of discovered device, in centimeter(cm).
     */
    int32_t range = 0;
    /**
     * NetworkType of the device.
     */
    int32_t networkType = 0;
    /**
     * Device authentication form.
     */
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    /**
     * Extra data of the device.
     * include json keys: "CONN_ADDR_TYPE", "BR_MAC_", "BLE_MAC", "WIFI_IP", "WIFI_PORT", "CUSTOM_DATA"
     */
    std::string extraData = "";
} DmDeviceInfo;

/**
 * @brief Device Basic Information.
 */
typedef struct
DmDeviceBasicInfo {
    /**
     * Device Id of the device.
     */
    char deviceId[DM_MAX_DEVICE_ID_LEN] = {0};
    /**
     * Device name of the device.
     */
    char deviceName[DM_MAX_DEVICE_NAME_LEN] = {0};
    /**
     * Device type of the device.
     */
    uint16_t deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    /**
     * NetworkId of the device.
     */
    char networkId[DM_MAX_DEVICE_ID_LEN] = {0};
    /**
     * Extra data of the device.
     * include json keys: "CUSTOM_DATA"
     */
    std::string extraData = "";
} DmDeviceBasicInfo;

/**
 * @brief Device Authentication param.
 */
typedef struct DmAuthParam {
    /**
     * the token used for this authentication.
     */
    std::string authToken;
    /**
     * the package name used for this authentication.
     */
    std::string packageName;
    /**
     * the app name used for this authentication.
     */
    std::string appName;
    /**
     * the app description used for this authentication.
     */
    std::string appDescription;
    /**
     * the auth type used for this authentication.
     */
    int32_t authType;
    /**
     * the business used for this authentication.
     */
    int32_t business;
    /**
     * the pin code used for this authentication.
     */
    int32_t pincode;
    /**
     * the direction used for this authentication.
     */
    int32_t direction;
    /**
     * the pin token used for this authentication.
     */
    int32_t pinToken;
    /**
     * the app image info used for this authentication.
     */
    DmAppImageInfo imageinfo;
} DmAuthParam;

/**
 * @brief Peer target id Information.
 */
typedef struct PeerTargetId {
    /**
     * device id.
     */
    std::string deviceId;
    /**
     * br mac address.
     */
    std::string brMac;
    /**
     * ble mac address.
     */
    std::string bleMac;
    /**
     * wlan ip address.
     */
    std::string wifiIp;
    /**
     * wlan ip port.
     */
    uint16_t wifiPort = 0;
    /**
     * service id.
     */
    int64_t serviceId = 0;

    bool operator==(const PeerTargetId &other) const
    {
        return (deviceId == other.deviceId) && (brMac == other.brMac) &&
            (bleMac == other.bleMac) && (wifiIp == other.wifiIp) && (wifiPort == other.wifiPort) &&
            (serviceId == other.serviceId);
    }

    bool operator<(const PeerTargetId &other) const
    {
        return (deviceId < other.deviceId) ||
            (deviceId == other.deviceId && brMac < other.brMac) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac < other.bleMac) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac == other.bleMac && wifiIp < other.wifiIp) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac == other.bleMac && wifiIp == other.wifiIp &&
                wifiPort < other.wifiPort) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac == other.bleMac && wifiIp == other.wifiIp &&
                wifiPort == other.wifiPort && serviceId < other.serviceId);
    }
} PeerTargetId;

typedef enum {
    BIT_NETWORK_TYPE_UNKNOWN = 0,  /**< Unknown network type */
    BIT_NETWORK_TYPE_WIFI,         /**< WIFI network type */
    BIT_NETWORK_TYPE_BLE,          /**< BLE network type */
    BIT_NETWORK_TYPE_BR,           /**< BR network type */
    BIT_NETWORK_TYPE_P2P,          /**< P2P network type */
    BIT_NETWORK_TYPE_USB = 8,          /**< USB network type */
    BIT_NETWORK_TYPE_COUNT,        /**< Invalid type */
} DmNetworkType;

typedef enum {
    NUMBER_PIN_CODE = 0,
    QR_CODE,
    VISIBLE_LLIGHT,
    SUPER_SONIC,
} DmPinType;

typedef enum {
    CREATE = 0,
    CREATE_RESULT,
    DESTROY,
    DESTROY_RESULT,
    PIN_TYPE_CHANGE,
    PIN_TYPE_CHANGE_RESULT,
} DmPinHolderEvent;

typedef enum {
    STATUS_DM_AUTH_DEFAULT = 0,
    STATUS_DM_AUTH_FINISH = 7,
    STATUS_DM_SHOW_AUTHORIZE_UI = 8,
    STATUS_DM_CLOSE_AUTHORIZE_UI = 9,
    STATUS_DM_SHOW_PIN_DISPLAY_UI = 10,
    STATUS_DM_CLOSE_PIN_DISPLAY_UI = 11,
    STATUS_DM_SHOW_PIN_INPUT_UI = 12,
    STATUS_DM_CLOSE_PIN_INPUT_UI = 13,
    STATUS_DM_SINK_AUTH_FINISH = 25,
} DmAuthStatus;

typedef enum {
    MIN = 0,
    REG_DEVICE_STATE = 1,
    UN_REG_DEVICE_STATE = 2,
    REG_DEVICE_SCREEN_STATE = 3,
    UN_REG_DEVICE_SCREEN_STATE = 4,
    REG_REMOTE_DEVICE_TRUST_CHANGE = 5,
    UN_REG_REMOTE_DEVICE_TRUST_CHANGE = 6,
    REG_CREDENTIAL_AUTH_STATUS_NOTIFY = 7,
    UN_REG_CREDENTIAL_AUTH_STATUS_NOTIFY = 8,
    REG_AUTH_CODE_INVALID = 9,
    UN_REG_AUTH_CODE_INVALID = 10,
    REG_SERVICE_STATE = 11,
    UN_REG_SERVICE_STATE = 12,

    MAX = 13,
} DmCommonNotifyEvent;

DM_EXPORT extern const char* DEVICE_TYPE_UNKNOWN_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PHONE_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PAD_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_TV_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_CAR_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_WATCH_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_WIFICAMERA_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_PC_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_SMART_DISPLAY_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_2IN1_STRING;
DM_EXPORT extern const char* DEVICE_TYPE_GLASSES_STRING;

typedef struct DmAccessCaller {
    std::string accountId;
    std::string pkgName;
    std::string networkId;
    int32_t userId;
    uint64_t tokenId = 0;
    std::string extra;
} DmAccessCaller;

typedef struct DmAccessCallee {
    std::string accountId;
    std::string networkId;
    std::string peerId;
    std::string pkgName;
    int32_t userId;
    uint64_t tokenId = 0;
    std::string extra;
} DmAccessCallee;

typedef struct ProcessInfo {
    int32_t userId;
    std::string pkgName;

    bool operator==(const ProcessInfo &other) const
    {
        return (userId == other.userId) && (pkgName == other.pkgName);
    }

    bool operator<(const ProcessInfo &other) const
    {
        return (userId < other.userId) ||
            (userId == other.userId && pkgName < other.pkgName);
    }
} ProcessInfo;

typedef struct DmNotifyKey {
    int32_t processUserId;
    std::string processPkgName;
    int32_t notifyUserId;
    std::string udid;

    bool operator==(const DmNotifyKey &other) const
    {
        return (processUserId == other.processUserId) && (processPkgName == other.processPkgName) &&
            (notifyUserId == other.notifyUserId) && (udid == other.udid);
    }

    bool operator<(const DmNotifyKey &other) const
    {
        return (processUserId < other.processUserId) ||
        (processUserId == other.processUserId && processPkgName < other.processPkgName) ||
        (processUserId == other.processUserId && processPkgName == other.processPkgName &&
            notifyUserId < other.notifyUserId) ||
        (processUserId == other.processUserId && processPkgName == other.processPkgName &&
            notifyUserId == other.notifyUserId && udid < other.udid);
    }
} DmNotifyKey;

enum class DMLocalServiceInfoAuthType : int32_t {
    TRUST_ONETIME = 0,
    TRUST_ALWAYS = 6,
    CANCEL = 1,
    MAX = 7
};

enum class DMLocalServiceInfoPinExchangeType : int32_t {
    PINBOX = 1,           /** pin(from memory) */
    QR_FROMDP = 2,        /** qr(from dp) */
    FROMDP = 3,           /** nfc(from dp) */
    IMPORT_AUTH_CODE = 5, /** import(from dp)  */
    ULTRASOUND = 6,       /** ultrasound(from memory) */
    MAX = 7
};

enum class DMLocalServiceInfoAuthBoxType : int32_t {
    STATE3 = 1,
    SKIP_CONFIRM = 2,
    TWO_IN1 = 3,
    MAX = 4
};

typedef struct DMLocalServiceInfo {
    std::string bundleName;
    int32_t authBoxType = 0;
    int32_t authType = 0;
    int32_t pinExchangeType = 0;
    std::string pinCode;
    std::string description;
    std::string extraInfo;
} DMLocalServiceInfo;

typedef struct DevUserInfo {
    std::string deviceId;
    int32_t userId;
} DevUserInfo;

typedef struct DMAclQuadInfo {
    std::string localUdid;
    int32_t localUserId;
    std::string peerUdid;
    int32_t peerUserId;
} DMAclQuadInfo;

typedef struct ServiceInfo {
    int64_t serviceId = 0;
    std::string serviceType;
    std::string serviceName;
    std::string serviceDisplayName;
} ServiceInfo;

typedef enum DMSrvDiscoveryMode {
    SERVICE_PUBLISH_MODE_PASSIVE = 0x15,
    SERVICE_PUBLISH_MODE_ACTIVE = 0x25
} DMSrvDiscoveryMode;

typedef enum DMSrvMediumType {
    SERVICE_MEDIUM_TYPE_AUTO = 0,
    SERVICE_MEDIUM_TYPE_BLE,
    SERVICE_MEDIUM_TYPE_BLE_TRIGGER,
    SERVICE_MEDIUM_TYPE_MDNS,
    SERVICE_MEDIUM_TYPE_BUTT,
} DMSrvMediumType;

typedef enum DMPublishState {
    SERVICE_UNPUBLISHED_STATE = 0,
    SERVICE_PUBLISHED_STATE,
} DMPublishState;
typedef struct DmUserRemovedServiceInfo {
    int64_t localTokenId = 0;
    std::string localPkgName = "";
    int32_t bindType = 0;
    std::string peerUdid = "";
    std::vector<int64_t> serviceIds = {};
    bool isActive = false;
    int32_t peerUserId = 0;
    std::map<int32_t, std::vector<int64_t>> bindTypeToServiceIdMap;
} DmUserRemovedServiceInfo;

//delete start
typedef struct DiscoveryServiceParam {
    std::string serviceName;
    std::string serviceType;
    int32_t discoveryServiceId = 0;
    DmExchangeFreq freq;
    DMSrvMediumType medium;
    DMSrvDiscoveryMode mode;
} DiscoveryServiceParam;

typedef struct DiscoveryServiceInfo {
    ServiceInfo serviceInfo;
    std::string pkgName;
} DiscoveryServiceInfo;

typedef struct ServiceInfoProfile {
    int32_t regServiceId = 0;
    std::string deviceId;
    int32_t userId = 0;
    int64_t tokenId = 0;
    int8_t publishState = 0;
    int64_t serviceId = 0;
    std::string serviceType;
    std::string serviceName;
    std::string serviceDisplayName;
} ServiceInfoProfile;

typedef struct ServiceRegInfo {
    ServiceInfo serviceInfo;
    std::string customData;
    uint32_t dataLen = 0;
} ServiceRegInfo;

typedef struct PublishServiceParam {
    ServiceInfo serviceInfo;
    DMSrvDiscoveryMode discoverMode;
    int32_t regServiceId = 0;
    DMSrvMediumType media;
    DmExchangeFreq freq;
} PublishServiceParam;
//delete end

typedef struct DmAuthInfo {
    int32_t userId = 0;     /** User ID of the current caller */
    uint64_t pinConsumerTokenId = 0;    /**The tokenId corresponding to the PIN used by the current caller */
    char pinCode[DM_MAX_PIN_CODE_LEN] = {0};
    char metaToken[DM_MAX_META_TOKEN_LEN] = {0};    /** Meta-token for dual-device authentication */
    std::string pinConsumerPkgName;      /** The pkgName corresponding to the PIN used by the current caller */
    std::string bizSrcPkgName;      /** Used for dual-device collaboration (for proxy entry/exit scenarios*/
    std::string bizSinkPkgName;     /** The target pkgName corresponding to the proxy (for dual-device collaboration) */
    DMLocalServiceInfoAuthType authType;        /** Trustworthy period 0:Only this time; 1:cancel ; 6：Always  */
    DMLocalServiceInfoAuthBoxType authBoxType;     /** Whether to pop up a window 1:confirmation 2:Skip confirmation  */
    DMLocalServiceInfoPinExchangeType pinExchangeType; /**1:pin box;2:qr from dp; 3:nfc from DP; 5:import 6:ultrasonic*/
    std::string description;        /** Description information on the three-party interface */
    std::string extraInfo;      /** Extended configuration information */
} DmAuthInfo;

typedef struct DmRegisterServiceInfo {
    int32_t userId;
    int64_t displayId;
    uint64_t serviceOwnerTokenId;
    std::string serviceOwnerPkgName;
    uint64_t serviceRegisterTokenId;
    std::string serviceType;
    std::string serviceName;
    std::string serviceDisplayName;
    std::string customData;
    std::string serviceCode;
    uint32_t dataLen;
    int64_t timeStamp;
    std::string description;
} DmRegisterServiceInfo;

typedef struct DmPublishServiceParam {
    DMSrvDiscoveryMode discoverMode;
    DMSrvMediumType media;
    DmExchangeFreq freq;
} DmPublishServiceParam;

typedef struct DmDiscoveryServiceParam {
    std::string serviceType;
    std::string serviceName;
    std::string serviceDisplayName;
    DmExchangeFreq freq;
    DMSrvMediumType medium;
    DMSrvDiscoveryMode mode;
    bool operator==(const DmDiscoveryServiceParam &other) const
    {
        return (serviceType == other.serviceType) && (serviceName == other.serviceName) &&
            (serviceDisplayName == other.serviceDisplayName) && (freq == other.freq) &&
            (medium == other.medium) && (mode == other.mode);
    }
} DmDiscoveryServiceParam;

typedef struct DmServiceInfo {
    int32_t userId;
    int64_t serviceId;
    int64_t displayId;
    int8_t publishState;
    std::string deviceId;
    std::string networkId;
    DmAuthForm authform;
    uint64_t serviceOwnerTokenId;
    std::string serviceOwnerPkgName;
    uint64_t serviceRegisterTokenId;
    std::string serviceType;
    std::string serviceName;
    std::string serviceDisplayName;
    std::string serviceCode;
    std::string customData;
    uint32_t dataLen;
    int64_t timeStamp;
    std::string description;
    bool operator==(const DmServiceInfo &other) const
    {
        return (deviceId == other.deviceId) && (userId == other.userId) &&
            (serviceId == other.serviceId);
    }
    bool operator<(const DmServiceInfo &other) const
    {
        return (deviceId < other.deviceId) && (userId < other.userId) &&
            (serviceId < other.serviceId);
    }
} DmServiceInfo;

/**
 * @brief Service state change event definition.
 */
typedef enum DmServiceState {
    /**
     * Service status is unknown.
     */
    SERVICE_STATE_UNKNOWN = -1,
    /**
     * Service online action, which indicates the Service is physically online.
     */
    SERVICE_STATE_ONLINE = 0,
    /**
     * Service offline action, which Indicates the device is physically offline.
     */
    SERVICE_STATE_OFFLINE = 2,
    /**
     * Service change action, which Indicates the device is physically change.
     */
    SERVICE_INFO_CHANGED = 3,
} DmServiceState;

typedef struct DmRegisterServiceState {
    int32_t userId = 0;
    uint64_t tokenId = 0;
    std::string pkgName = "";
    int64_t serviceId = 0;

    bool operator==(const DmRegisterServiceState& other) const
    {
        return userId == other.userId &&
              tokenId == other.tokenId &&
              pkgName == other.pkgName &&
              serviceId == other.serviceId;
    }
} DmRegisterServiceState;

typedef struct ServiceStateBindParameter {
    uint64_t tokenId = 0;
    std::string pkgName = "";
    int32_t bindType = 0;
    std::string peerUdid = "";
    int32_t peerUserId = 0;
    int64_t serviceId = -1;
} ServiceStateBindParameter;

//add by zqz
typedef struct ServiceSyncInfo {
    std::string pkgName;
    int32_t localUserId = 0;
    std::string networkId;
    int64_t serviceId = 0;
    int32_t callerUserId = 0;
    uint32_t callerTokenId = 0;

    bool operator==(const ServiceSyncInfo &other) const
    {
        return (pkgName == other.pkgName) && (localUserId == other.localUserId) &&
            (networkId == other.networkId) && (serviceId == other.serviceId);
    }

    bool operator<(const ServiceSyncInfo &other) const
    {
        return (pkgName < other.pkgName) ||
            (pkgName == other.pkgName && localUserId < other.localUserId) ||
            (pkgName == other.pkgName && localUserId == other.localUserId && networkId < other.networkId) ||
            (pkgName == other.pkgName && localUserId == other.localUserId && networkId == other.networkId &&
                serviceId < other.serviceId);
    }
} ServiceSyncInfo;

typedef struct UnbindServiceProxyParam {
    int32_t userId = -1;
    uint64_t localTokenId = 0;
    std::vector<uint64_t> peerTokenId = {};
    std::string localUdid = "";
    std::string peerNetworkId = "";
    int64_t serviceId = -1;
} UnbindServiceProxyParam;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_INFO_H