/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SERVICE_IMPL_3RD_H
#define OHOS_DM_SERVICE_IMPL_3RD_H


#include <set>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <semaphore>
#include "ffrt.h"

#include "softbus_common.h"
#include "parameter.h"

#include "auth_manager_3rd.h"
#include "dm_single_instance_3rd.h"
#include "dm_log_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "idevice_manager_service_impl_3rd.h"
#include "idevice_manager_service_listener_3rd.h"
#include "dm_anonymous_3rd.h"
#include "softbus_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

typedef struct DmAuthCallerInfo3rd {
    uint32_t uid = 0;
    int32_t userId = -1;
    uint32_t tokenId = 0;
    std::string processName = "";
    bool isSystemSA = false;
    int32_t bindLevel = -1;
} DmAuthCallerInfo3rd;

class Session {
public:
    Session(int sessionId, std::string deviceId);
    int sessionId_;
    std::string deviceId_;
    std::string version_{""};
    std::atomic<bool> flag_{false}; // Only one session is allowed
    std::set<uint64_t> logicalSessionSet_;  // Logical Session Set
    std::atomic<int> logicalSessionCnt_{0};
};

struct Config {
    std::string businessName;
    std::string authCode;
};

class DeviceManagerServiceImpl3rd : public IDeviceManagerServiceImpl3rd {
public:
    DeviceManagerServiceImpl3rd();
    virtual ~DeviceManagerServiceImpl3rd();

    int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener3rd> &listener);
    void Release();
    int32_t ImportPinCode3rd(const std::string &businessName, const std::string &pinCode);
    int32_t AuthPincode(const PeerTargetId3rd &targetId, std::map<std::string, std::string> &authparam);

    int OnAuth3rdAclSessionOpened(int sessionId, int result);
    void OnAuth3rdAclSessionClosed(int sessionId);
    void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnAuth3rdSessionOpened(int sessionId, int result);
    void OnAuth3rdSessionClosed(int sessionId);
    void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen);

private:
    bool IsInvalidPeerTargetId(const PeerTargetId3rd &targetId);
    void GetBindCallerInfo(DmAuthCallerInfo3rd &authCallerInfo3rd, ProcessInfo3rd &processInfo);
    void SetBindCallerInfoToAuthParam(const std::map<std::string, std::string> &authParam,
        std::map<std::string, std::string> &authParamTmp, const DmAuthCallerInfo3rd &authCallerInfo3rd,
        ProcessInfo3rd &processInfo);
    void AuthPincodeImpl(const PeerTargetId3rd &targetId, const PinCodeInfo pinCodeInfo,
        const std::map<std::string, std::string> &authParamTmp, const ProcessInfo3rd processInfo3rd);
    void AuthDeviceAclImpl(const PeerTargetId3rd &targetId, const PinCodeInfo pinCodeInfo,
        const std::map<std::string, std::string> &authParamTmp, const ProcessInfo3rd processInfo3rd);

private:
    ffrt::mutex pinCodeLock_;
    std::map<ProcessInfo3rd, PinCodeInfo> pinCodeMap_;
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener_;
    PeerTargetId3rd targetId3rd_;
    std::map<std::string, std::string> authParam_;
    int32_t sessionId_ = -1;

private:
    int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam);
    std::shared_ptr<AuthManagerBase3rd> GetAuthMgrByTokenId(uint32_t tokenId);
    std::shared_ptr<AuthManagerBase3rd> GetAuthMgrByMessage(int32_t msgType,
        uint64_t logicalSessionId, const JsonObject &jsonObject);
    int32_t InitAuthMgr(bool isSrcSide, uint32_t tokenId, uint64_t logicalSessionId,
        ProcessInfo3rd processInfo3rd);
    int32_t AddAuthMgr(uint32_t tokenId, std::shared_ptr<AuthManagerBase3rd> authMgr);
    void ImportAuthCodeAndUidFromCache(std::shared_ptr<AuthManagerBase3rd> authMgr,
        const ProcessInfo3rd processInfo3rd);
    void NotifyCleanEvent(uint64_t logicalSessionId, int32_t connDelayCloseTime);
    std::shared_ptr<AuthManagerBase3rd> GetAuthMgr();
    void CleanAuthMgrByLogicalSessionId(uint64_t logicalSessionId, int32_t connDelayCloseTime);
    void EraseAuthMgr(uint32_t tokenId);
    ffrt::mutex authLock_;
    bool isAuthFlowRunning_ = false;
    std::string runningAuthDeviceId_;
    std::shared_ptr<SoftbusSession3rd> softbusSession_;
    std::shared_ptr<SoftbusConnector3rd> softbusConnector_;
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector_;
    std::map<uint32_t, std::shared_ptr<AuthManagerBase3rd>> authMgrMap_;
    ffrt::mutex authMgrMapLock_;
    ffrt::mutex logicalSessionId2TokenIdMapMtx_;
    std::map<uint64_t, uint32_t> logicalSessionId2TokenIdMap_;
    ffrt::mutex authMgrMapMtx_;
    ffrt::mutex tokenIdSessionIdMapMtx_;
    std::map<uint32_t, int> tokenIdSessionIdMap_;  // New protocol sharing
    ffrt::mutex mapMutex_;  // sessionsMap_ lock
    std::map<int, bool> sessionEnableCvReadyMap_;  // Condition variable ready flag
    std::map<int, ffrt::condition_variable> sessionEnableCvMap_;  // Condition variable corresponding to the session
    std::map<int, ffrt::mutex> sessionEnableMutexMap_;
};

using CreateDMServiceImpl3rdFuncPtr = IDeviceManagerServiceImpl3rd *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_IMPL_3RD_H