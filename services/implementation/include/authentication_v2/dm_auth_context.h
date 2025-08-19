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

#ifndef OHOS_DM_AUTH_CONTEXT_V2_H
#define OHOS_DM_AUTH_CONTEXT_V2_H
#include <map>
#include <string>

#include "access_control_profile.h"
#include "authentication.h"
#include "auth_manager.h"
#include "auth_ui_state_manager.h"
#include "dm_ability_manager.h"
#include "dm_anonymous.h"
#include "dm_auth_message_processor.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_timer.h"
#include "hichain_auth_connector.h"
#include "hichain_connector.h"
#include "softbus_connector.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {

class DmAuthStateMachine;
class DmAuthMessageProcessor;

using CleanNotifyCallback = std::function<void(uint64_t)>;

// PIN Code Authentication Type
enum DmAuthType : int32_t {
    AUTH_TYPE_CRE = 0,
    AUTH_TYPE_PIN,              // Input PIN code
    AUTH_TYPE_QR_CODE,          // Not used in the new protocol
    AUTH_TYPE_NFC,
    AUTH_TYPE_NO_INTER_ACTION,  // Not used in the new protocol
    AUTH_TYPE_IMPORT_AUTH_CODE, // Import PIN code
    AUTH_TYPE_PIN_ULTRASONIC,   // Ultrasonic PIN code
    AUTH_TYPE_UNKNOW,           // Not used in the new protocol
};

enum DmAuthDirection {
    DM_AUTH_SOURCE = 0,
    DM_AUTH_SINK,
};

enum DmBindType {
    DM_AUTH_USERID = 1,
    DM_AUTH_SYSTEM_SERVICE,
    DM_AUTH_APP_SERVICE,
    DM_AUTH_DEVICEID,
};

enum DmAuthSide {
    DM_AUTH_LOCAL_SIDE = 0,
    DM_AUTH_REMOTE_SIDE,
};

enum DmAuthScope {
    DM_AUTH_SCOPE_INVALID = 0,
    DM_AUTH_SCOPE_DEVICE,
    DM_AUTH_SCOPE_USER,
    DM_AUTH_SCOPE_APP,
    DM_AUTH_SCOPE_LNN,
    DM_AUTH_SCOPE_MAX,
};

enum {
    DM_UNKNOWN_TYPE = 0,
    DM_SAME_ACCOUNT_TYPE = 1,
    DM_SHARE_TYPE = 2,
    DM_POINT_TO_POINT_TYPE = 256,
    DM_SAME_GROUP_TYPE = 3,
};

enum DmUltrasonicInfo {
    // Forward means the pincode is generated at sink side, then send by ultrasonic from sink to source side.
    DM_Ultrasonic_Forward = 0,
    // Reverse means the pincode is generated at source side, then send by ultrasonic from source to sink side.
    DM_Ultrasonic_Reverse = 1,
    DM_Ultrasonic_Invalid = 2,
};

typedef struct DmProxyAccess {
    std::string pkgName;
    std::string pkgLabel;
    std::string bundleInfo;
    std::string bundleName;
    std::string peerBundleName;
    int64_t tokenId;
    std::string tokenIdHash;
    int32_t bindLevel;
    std::string transmitCredentialId;
    std::string transmitPublicKey;
    std::string publicKey;
    int32_t sessionKeyId;
    int32_t transmitSessionKeyId;
    int64_t transmitSkTimeStamp;
    int64_t skTimeStamp;        // Used for aging, time is 2 days
    bool isAuthed;
    std::string aclTypeList;        // Trust relationship list, used for data aging, KV format
    std::string credTypeList;
    std::string aclStrList;
    std::map<int32_t, std::string> credentialInfos;
    std::map<int32_t, DistributedDeviceProfile::AccessControlProfile> aclProfiles;
    std::string credInfoJson;
    std::string aclTypeJson;
} DmProxyAccess;

struct DmProxyAuthContext {
    std::string proxyContextId;
    std::string customData;
    std::string pkgLabel;
    bool needBind{true};
    bool needAgreeCredential{true};
    bool needAuth{true};
    bool IsNeedSetProxyRelationShip{false};
    DmProxyAccess proxyAccesser;
    DmProxyAccess proxyAccessee;
    bool operator==(const DmProxyAuthContext &other) const
    {
        return (proxyContextId == other.proxyContextId);
    }

    bool operator<(const DmProxyAuthContext &other) const
    {
        return proxyContextId < other.proxyContextId;
    }
};

// Used for one-touch pairing
struct DmPeerTargetAddress {
    // directly establish a Bluetooth connection
    std::string peerBrMacAddress;
    std::string peerBleMacAddress;
    std::string peerWifiMacAddress;
    std::string peerActionMacAddress;

    std::string peerWifiChannel;
    std::string peerWifiIp;
    uint16_t peerWifiPort;
};

struct DmPeerTarget {
    DmBindType peerType;
    std::string peerDeviceId;
    int64_t peerServiceId;
    int64_t peerSaTokenId;
    std::string peerBundleName;
    DmPeerTargetAddress peerTargetAddress;
};

struct DmAccess {
    std::string deviceName;
    int32_t deviceType;         // Device types such as PC, mobile, watch, large screen, etc.
    std::string deviceId;
    std::string deviceIdHash;
    std::string addr;
    int32_t userId{-1};
    int32_t displayId{-1};       // Logical screen ID, used for query userId
    std::string accountId;
    std::string accountIdHash;
    int64_t tokenId;
    std::string tokenIdHash;
    std::string networkId;
    std::string bundleName;     // Stores the bundleName, HAP -> bundleName in BMS, SA -> process name
    std::string pkgName;    // store pkgName set by client
    std::string language;
    int64_t serviceId;          // Reserved field, to be used in HM 6.0
    std::string accesserHapSignature;
    int32_t bindLevel;
    int32_t lnnBindType;
    int32_t transmitBindType;
    std::string lnnCredentialId; // User-level credential ID
    std::string transmitCredentialId; // Application-level credential ID
    std::string lnnPublicKey;   // User-level public key
    std::string transmitPublicKey;   // Application-level public key
    std::vector<int32_t> bindType;  // such as DM_AUTH_CREDENTIAL_ACCOUNT_RELATED
    std::string publicKey;
    int32_t status;             // Indicates whether the service is in the foreground or background
    int32_t sessionKeyId;       // Used as key delivery material, retrieves the SK from the bus
    int32_t transmitSessionKeyId; // Permanent application SKID on this end, returned by DP for ACL updates and aging
    int32_t lnnSessionKeyId{0};    // Permanent user SKID on this end, returned by DP for ACL updates and aging
    int32_t oldBindLevel;
    int64_t transmitSkTimeStamp; // Used for aging, time is 2 days, application-level credential timestamp
    int64_t lnnSkTimeStamp{0};     // Used for aging, time is 2 days, user-level credential timestamp
    int64_t skTimeStamp;        // Used for aging, time is 2 days
    uint64_t certRandom{0}; // Used for cert generate
    bool isAuthed;
    bool isUserLevelAuthed;
    bool isOnline;
    bool isGenerateLnnCredential{true};
    bool isPutLnnAcl{true};
    std::string dmVersion;
    std::string edition;        // Used for compatibility before version 5.1.0, assists in version negotiation
    std::string aclTypeList;        // Trust relationship list, used for data aging, KV format
    std::string credTypeList;
    std::string aclStrList;
    std::map<int32_t, std::string> credentialInfos;         // map: <credType, cred>, cred is string tranformed by json
    std::map<int32_t, DistributedDeviceProfile::AccessControlProfile> aclProfiles;
    // construct for old version compatible start
    // In the scenario of compatibility between the new and old protocols, it is necessary to
    // send both the new and old protocol messages simultaneously in the 80 message when
    // authType == import. Therefore, it is required to define these fields for compatibility processing.
    std::string accountGroupIdHash;
    std::string oldBundleName;
    // construct for old version compatible end
    std::string extraInfo;      // Expandable field, JSON format, KV structure
    std::string cert;
    bool isCommonFlag{false};
    bool isGeneratedLnnCredThisBind{false};
    bool isGeneratedTransmitThisBind{false};
};

struct DmAuthContext {
    bool isOnline;
    uint64_t logicalSessionId;
    DmMessageType msgType;
    int32_t sessionId;
    int64_t requestId;          // HiChain authentication ID
    DMLocalServiceInfoAuthBoxType authBoxType{DMLocalServiceInfoAuthBoxType::STATE3}; // Authentication box type
    UiAction pinInputResult;
    // Authorization result (using 0, 1, 6, representing single use, cancel, and always trust, enum UiAction)
    UiAction confirmOperation{UiAction::USER_OPERATION_TYPE_ALLOW_AUTH};
    DmAuthType authType{DmAuthType::AUTH_TYPE_PIN};  // PIN code, ultrasonic PIN code, imported PIN code
    std::vector<DmAuthType> authTypeList;
    uint32_t currentAuthTypeIdx{0};
    int32_t inputPinAuthFailTimes{0}; // Number of failed PIN authentication attempts, exceeding 3 results in failure
    std::string pinCode{""};
    bool serviceInfoFound{false};
    // Link delay release time, does not automatically disconnect after
    // authorization (used for specific business needs), reserved field
    int32_t connDelayCloseTime;
    int32_t reason{DM_OK};
    int32_t reply;
    std::string userOperationParam;
    int32_t state;
    int32_t hmlActionId = 0;
    int64_t remainingFrozenTime = 0;
    bool authenticating;        // Indicator whether authentication is in progress
    bool successFinished{false};
    bool isAppCredentialVerified{false};        // Whether the application credential has been verified
    bool hmlEnable160M{false};
    std::string businessId;
    std::string pkgName;    // Business-provided identifier, custom-defined by business, carries risk of spoofing
    std::string pkgLabel;
    std::string importCodeBundleName;           // Bundle name for imported PIN code
    std::string appThumbnail;   // Application thumbnail
    // Description of the operation this binding is used for, displayed in authorization dialog
    std::string appOperation;
    // Custom business field, provides detailed information to the user about this binding operation
    std::string customData;
    std::string connSessionType;
    std::string extraInfo;      // Expandable field, key-value structure
    DmAuthDirection direction;  // Indicator of authentication direction
    ProcessInfo processInfo;
    DmPeerTarget peerTarget;
    DmUltrasonicInfo ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Forward; // Ultrasonic information
    DmAccess accesser;
    DmAccess accessee;
    std::multimap<DmAccess, DmAccess> proxy;    // Multimap where the key is the accessor and the value is the accesssee
    bool isNeedJoinLnn{true};
    bool IsProxyBind{false};
    bool IsCallingProxyAsSubject{true};
    bool IsNeedSetProxy{false};
    bool isNeedAuthenticate{true}; // apply for skip authenticate
    std::vector<DmProxyAuthContext> subjectProxyOnes;
    std::string reUseCreId;
    std::string title;
    std::string srvExtarInfo;

    std::shared_ptr<DmAuthStateMachine> authStateMachine;
    std::shared_ptr<AuthUiStateManager> authUiStateMgr;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<DmAuthMessageProcessor> authMessageProcessor;
    std::shared_ptr<SoftbusConnector> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<IAuthentication> authPtr;   // Pointer to authentication interface
    std::shared_ptr<DmTimer> timer;
    std::string transmitData;                   // Data returned from onTransmit function
    std::string importPkgName = "";
    std::string importAuthCode = "";
    std::map<int32_t, std::shared_ptr<IAuthentication>> authenticationMap;
    PeerTargetId peerTargetId;
    bool pinNegotiateStarted{false};
    bool isAuthenticateDevice{false};           // Whether device authentication is in progress
    bool needBind{true};
    bool needAgreeCredential{true};
    bool needAuth{true};
    std::mutex certMtx_; // cert lock
    std::mutex certCVMtx_; // cert cv lock
    std::condition_variable certCV_; // cert cv
    CleanNotifyCallback cleanNotifyCallback{nullptr};

    std::string GetDeviceId(DmAuthSide side);
    int32_t GetUserId(DmAuthSide side);
    std::string GetCredentialId(DmAuthSide side, DmAuthScope authorizedScope);
    std::string GetPublicKey(DmAuthSide side, DmAuthScope authorizedScope);
    void SetCredentialId(DmAuthSide side, DmAuthScope authorizedScope, const std::string &credentialId);
    void SetPublicKey(DmAuthSide side, DmAuthScope authorizedScope, const std::string &publicKey);
    std::string GetAccountId(DmAuthSide side);
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_CONTEXT_V2_H
