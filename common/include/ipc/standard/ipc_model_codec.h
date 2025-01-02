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

#include "dm_device_profile_info.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
class IpcModelCodec {
public:
    static void DecodeDmProductInfo(MessageParcel &parcel, DmProductInfo &prodInfo);
    static void DecodeDmServiceProfileInfo(MessageParcel &parcel, DmServiceProfileInfo &svrInfo);
    static void DecodeDmServiceProfileInfos(MessageParcel &parcel, std::vector<DmServiceProfileInfo> &svrInfos);
    static void DecodeDmDeviceProfileInfo(MessageParcel &parcel, DmDeviceProfileInfo &devInfo);
    static bool EncodeDmProductInfo(const DmProductInfo &prodInfo, MessageParcel &parcel);
    static bool EncodeDmServiceProfileInfo(const DmServiceProfileInfo &svrInfo, MessageParcel &parcel);
    static bool EncodeDmServiceProfileInfos(const std::vector<DmServiceProfileInfo> &svrInfos, MessageParcel &parcel);
    static bool EncodeDmDeviceProfileInfo(const DmDeviceProfileInfo &devInfo, MessageParcel &parcel);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_MODEL_CODEC_H
