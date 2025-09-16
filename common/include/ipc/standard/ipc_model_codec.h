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

#ifndef OHOS_DM_IPC_MODEL_CODEC_H
#define OHOS_DM_IPC_MODEL_CODEC_H

#include "dm_device_info.h"
#include "dm_device_profile_info.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
class IpcModelCodec {
public:
    static void DecodeDmDeviceBasicInfo(MessageParcel &parcel, DmDeviceBasicInfo &devInfo);
    static bool EncodePeerTargetId(const PeerTargetId &targetId, MessageParcel &parcel);
    static void DecodePeerTargetId(MessageParcel &parcel, PeerTargetId &targetId);
    static bool EncodeDmAccessCaller(const DmAccessCaller &caller, MessageParcel &parcel);
    static bool EncodeDmAccessCallee(const DmAccessCallee &callee, MessageParcel &parcel);
    static int32_t DecodeDmDeviceProfileInfoFilterOptions(MessageParcel &parcel,
        DmDeviceProfileInfoFilterOptions &filterOptions);
    static bool EncodeDmDeviceProfileInfoFilterOptions(const DmDeviceProfileInfoFilterOptions &filterOptions,
        MessageParcel &parcel);
    static void DecodeDmServiceProfileInfo(MessageParcel &parcel, DmServiceProfileInfo &svrInfo);
    static void DecodeDmServiceProfileInfos(MessageParcel &parcel, std::vector<DmServiceProfileInfo> &svrInfos);
    static void DecodeDmDeviceProfileInfo(MessageParcel &parcel, DmDeviceProfileInfo &devInfo);
    static bool EncodeDmServiceProfileInfo(const DmServiceProfileInfo &svrInfo, MessageParcel &parcel);
    static bool EncodeDmServiceProfileInfos(const std::vector<DmServiceProfileInfo> &svrInfos, MessageParcel &parcel);
    static bool EncodeDmDeviceProfileInfo(const DmDeviceProfileInfo &devInfo, MessageParcel &parcel);
    static std::string GetDeviceIconInfoUniqueKey(const DmDeviceIconInfoFilterOptions &iconFiter);
    static std::string GetDeviceIconInfoUniqueKey(const DmDeviceIconInfo &iconInfo);
    static void DecodeDmDeviceIconInfo(MessageParcel &parcel, DmDeviceIconInfo &deviceIconInfo);
    static bool EncodeDmDeviceIconInfo(const DmDeviceIconInfo &deviceIconInfo, MessageParcel &parcel);
    static void DecodeDmDeviceIconInfoFilterOptions(MessageParcel &parcel,
        DmDeviceIconInfoFilterOptions &filterOptions);
    static bool EncodeDmDeviceIconInfoFilterOptions(const DmDeviceIconInfoFilterOptions &filterOptions,
        MessageParcel &parcel);
    static void DecodeDmDeviceInfo(MessageParcel &parcel, DmDeviceInfo &devInfo);
    static bool EncodeNetworkIdQueryFilter(const NetworkIdQueryFilter &queryFilter, MessageParcel &parcel);
    static bool DecodeNetworkIdQueryFilter(MessageParcel &parcel, NetworkIdQueryFilter &queryFilter);
    static bool EncodeStringVector(const std::vector<std::string> &vec, MessageParcel &parcel);
    static bool DecodeStringVector(MessageParcel &parcel, std::vector<std::string> &vec);
    static bool EncodeLocalServiceInfo(const DMLocalServiceInfo &serviceInfo, MessageParcel &parcel);
    static bool EncodeLocalServiceInfos(const std::vector<DMLocalServiceInfo> &serviceInfos, MessageParcel &parcel);
    static bool DecodeLocalServiceInfo(MessageParcel &parcel, DMLocalServiceInfo &serviceInfo);
    static bool DecodeLocalServiceInfos(MessageParcel &parcel, std::vector<DMLocalServiceInfo> &serviceInfos);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_MODEL_CODEC_H
