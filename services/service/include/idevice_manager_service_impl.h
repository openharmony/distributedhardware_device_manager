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

#ifndef OHOS_I_DM_SERVICE_IMPL_H
#define OHOS_I_DM_SERVICE_IMPL_H

#include <string>
#include <vector>

#include "socket.h"

#include "idevice_manager_service_listener.h"
#include "dm_device_info.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
class IDeviceManagerServiceImpl {
public:
    virtual ~IDeviceManagerServiceImpl() {}

    /**
     * @tc.name: IDeviceManagerServiceImpl::Initialize
     * @tc.desc: Initialize the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::Release
     * @tc.desc: Release the device manager service impl
     * @tc.type: FUNC
     */
    virtual void Release() = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::UnAuthenticateDevice
     * @tc.desc: UnAuthenticate Device of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::UnBindDevice
     * @tc.desc: UnBindDevice Device of the device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel) = 0;

    virtual int32_t UnBindDevice(const std::string &pkgName, const std::string &udid,
        int32_t bindLevel, const std::string &extra) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::SetUserOperation
     * @tc.desc: Se tUser Operation of device manager service impl
     * @tc.type: FUNC
     */
    virtual int32_t SetUserOperation(std::string &pkgName, int32_t action, const std::string &params) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::HandleDeviceStatusChange
     * @tc.desc: Handle Device Status Event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::OnSessionOpened
     * @tc.desc: Send Session Opened event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual int OnSessionOpened(int sessionId, int result) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::OnSessionClosed
     * @tc.desc: Send Session Closed event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void OnSessionClosed(int sessionId) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::OnBytesReceived
     * @tc.desc: Send Bytes Received event to the device manager service impl
     * @tc.type: FUNC
     */
    virtual void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::RequestCredential
     * @tc.desc: RequestCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::ImportCredential
     * @tc.desc: ImportCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::DeleteCredential
     * @tc.desc: DeleteCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::MineRequestCredential
     * @tc.desc: MineRequestCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::CheckCredential
     * @tc.desc: CheckCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::ImportCredential
     * @tc.desc: ImportCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::DeleteCredential
     * @tc.desc: DeleteCredential of the Device Manager Service
     * @tc.type: FUNC
     */
    virtual int32_t DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::RegisterCredentialCallback
     * @tc.desc: RegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterCredentialCallback(const std::string &pkgName) = 0;
    /**
     * @tc.name: IDeviceManagerServiceImpl::UnRegisterCredentialCallback
     * @tc.desc: UnRegisterCredentialCallback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterCredentialCallback(const std::string &pkgName) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::NotifyEvent
     * @tc.desc: NotifyEvent
     * @tc.type: FUNC
     */
    virtual int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::GetGroupType
     * @tc.desc: GetGroupType
     * @tc.type: FUNC
     */
    virtual int32_t GetGroupType(std::vector<DmDeviceInfo> &deviceList) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::GetUdidHashByNetWorkId
     * @tc.desc: GetUdidHashByNetWorkId
     * @tc.type: FUNC
     */
    virtual int32_t GetUdidHashByNetWorkId(const char *networkId, std::string &deviceId) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::ImportAuthCode
     * @tc.desc: ImportAuthCode
     * @tc.type: FUNC
     */
    virtual int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::ExportAuthCode
     * @tc.desc: ExportAuthCode
     * @tc.type: FUNC
     */
    virtual int32_t ExportAuthCode(std::string &authCode) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::BindTarget
     * @tc.desc: BindTarget
     * @tc.type: FUNC
     */
    virtual int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::RegisterUiStateCallback
     * @tc.desc: RegisterUiStateCallback
     * @tc.type: FUNC
     */
    virtual int32_t RegisterUiStateCallback(const std::string &pkgName) = 0;

    /**
     * @tc.name: IDeviceManagerServiceImpl::UnRegisterUiStateCallback
     * @tc.desc: UnRegisterUiStateCallback
     * @tc.type: FUNC
     */
    virtual int32_t UnRegisterUiStateCallback(const std::string &pkgName) = 0;

    virtual std::unordered_map<std::string, DmAuthForm> GetAppTrustDeviceIdList(std::string pkgname) = 0;
    virtual int32_t DpAclAdd(const std::string &udid) = 0;
    virtual void DeleteAlwaysAllowTimeOut() = 0;
    virtual void CheckDeleteCredential(const std::string &remoteUdid, int32_t remoteUserId) = 0;
    virtual int32_t IsSameAccount(const std::string &udid) = 0;
    virtual uint64_t GetTokenIdByNameAndDeviceId(std::string extra, std::string requestDeviceId) = 0;
    virtual void ScreenCommonEventCallback(std::string commonEventType) = 0;
    virtual bool CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual bool CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual void HandleDeviceNotTrust(const std::string &udid) = 0;
    virtual std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t userId,
        const std::string &accountId) = 0;
    virtual void HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid) = 0;
    virtual void HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId) = 0;
    virtual void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId) = 0;
    virtual void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        int32_t tokenId, int32_t peerTokenId) = 0;
    virtual int32_t GetBindLevel(const std::string &pkgName, const std::string &localUdid,
        const std::string &udid, uint64_t &tokenId) = 0;
    virtual void HandleIdentAccountLogout(const DMAclQuadInfo &info, const std::string &accountId) = 0;
    virtual void HandleUserRemoved(std::vector<std::string> peerUdids, int32_t preUserId) = 0;
    virtual void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo) = 0;
    virtual void HandleUserSwitched(const std::vector<std::string> &deviceVec, int32_t currentUserId,
        int32_t beforeUserId) = 0;
    virtual int32_t StopAuthenticateDevice(const std::string &pkgName) = 0;
    virtual void HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId,
                                            int32_t errcode) = 0;
    virtual int32_t SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string remoteAclList, bool isDelImmediately) = 0;
    virtual int32_t GetAclListHash(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string &aclList) = 0;
    virtual int32_t ProcessAppUnintall(const std::string &appId, int32_t accessTokenId) = 0;
    virtual int32_t ProcessAppUninstall(int32_t userId, int32_t accessTokenId) = 0;
    virtual void ProcessUnBindApp(int32_t userId, int32_t accessTokenId, const std::string &extra,
        const std::string &udid) = 0;

    virtual void HandleSyncUserIdEvent(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid, bool isCheckUserStatus) = 0;
    virtual void HandleRemoteUserRemoved(int32_t userId, const std::string &remoteUdid) = 0;
    virtual std::map<std::string, int32_t> GetDeviceIdAndBindLevel(int32_t userId) = 0;
    virtual std::vector<std::string> GetDeviceIdByUserIdAndTokenId(int32_t userId, int32_t tokenId) = 0;
    virtual std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t localUserId) = 0;
    virtual int32_t SaveOnlineDeviceInfo(const std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual void HandleDeviceUnBind(int32_t bindType, const std::string &peerUdid,
        const std::string &localUdid, int32_t localUserId, const std::string &localAccountId) = 0;
    virtual int32_t RegisterAuthenticationType(int32_t authenticationType) = 0;
    virtual void HandleCredentialDeleted(const char *credId, const char *credInfo, const std::string &localUdid,
        std::string &remoteUdid, bool &isSendBroadCast) = 0;
    virtual void HandleShareUnbindBroadCast(const std::string &credId, const int32_t &userId,
        const std::string &localUdid) = 0;
    virtual int32_t CheckDeviceInfoPermission(const std::string &localUdid, const std::string &peerDeviceId) = 0;
    virtual void HandleServiceUnBindEvent(int32_t userId, const std::string &remoteUdid,
        int32_t remoteTokenId) = 0;
    virtual void HandleCommonEventBroadCast(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid) = 0;
    virtual bool CheckSrcAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual bool CheckSinkAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual bool CheckSrcIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual bool CheckSinkIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid) = 0;
    virtual void DeleteHoDevice(const std::string &peerUdid, const std::vector<int32_t> &foreGroundUserIds,
        const std::vector<int32_t> &backGroundUserIds) = 0;
    virtual int32_t BindServiceTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam) = 0;
    virtual int32_t UnbindServiceTarget(const std::string &pkgName, int64_t serviceId) = 0;
    virtual void InitTaskOfDelTimeOutAcl(const std::string &peerUdid, int32_t peerUserId, int32_t localUserId) = 0;
    virtual void GetNotifyEventInfos(std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual int32_t LeaveLNN(const std::string &pkgName, const std::string &networkId) = 0;
    virtual int32_t ExportAuthInfo(DmAuthInfo &dmAuthInfo, uint32_t pinLength) = 0;
    virtual int32_t ImportAuthInfo(const DmAuthInfo &dmAuthInfo) = 0;
};

using CreateDMServiceFuncPtr = IDeviceManagerServiceImpl *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_H
