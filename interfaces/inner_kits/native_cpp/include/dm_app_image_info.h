/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_APP_IMAGE_INFO_H
#define OHOS_DM_APP_IMAGE_INFO_H

#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
class DmAppImageInfo {
public:
    DmAppImageInfo() = default;
    /**
     * @tc.name: DmAppImageInfo::DmAppImageInfo
     * @tc.desc: Dm App Image Info Save Data
     * @tc.type: FUNC
     */
    explicit DmAppImageInfo(uint8_t *appIcon, int32_t appIconLen, uint8_t *appThumbnail, int32_t appThumbnailLen)
    {
        SaveData(appIcon, appIconLen, appThumbnail, appThumbnailLen);
    }
    /**
     * @tc.name: DmAppImageInfo::Reset
     * @tc.desc: Dm App Image Info Reset
     * @tc.type: FUNC
     */
    void Reset(uint8_t *appIcon, int32_t appIconLen, uint8_t *appThumbnail, int32_t appThumbnailLen)
    {
        SaveData(appIcon, appIconLen, appThumbnail, appThumbnailLen);
    }
    /**
     * @tc.name: DmAppImageInfo::ResetIcon
     * @tc.desc: Dm App Image Info ResetIcon
     * @tc.type: FUNC
     */
    void ResetIcon(uint8_t *appIcon, int32_t appIconLen)
    {
        SaveIconData(appIcon, appIconLen);
    }
    /**
     * @tc.name: DmAppImageInfo::InitThumbnail
     * @tc.desc: Dm App Image Info Init Thumbnail
     * @tc.type: FUNC
     */
    void InitThumbnail(int32_t appThumbnailLen)
    {
        if (appThumbnailLen <= 0 || appThumbnailLen > THUMB_MAX_LEN) {
            appThumbnailLen_ = 0;
            appThumbnail_ = nullptr;
            return;
        }

        appThumbnail_ = new (std::nothrow) uint8_t[appThumbnailLen] { 0 };
        if (appThumbnail_ != nullptr) {
            appThumbnailLen_ = appThumbnailLen;
        }
    }
    /**
     * @tc.name: DmAppImageInfo::SetThumbnailData
     * @tc.desc: Dm App Image Info Init Set Data of Thumbnail
     * @tc.type: FUNC
     */
    int32_t SetThumbnailData(uint8_t *srcBuffer, int32_t srcBufferLen, int32_t copyIndex, int32_t copyLen)
    {
        if (srcBuffer == nullptr || srcBufferLen <= 0 || copyLen > srcBufferLen || copyIndex < 0) {
            return -1;
        }

        if ((copyIndex + copyLen) > appThumbnailLen_) {
            return -1;
        }

        if (appThumbnail_ == nullptr) {
            return -1;
        }

        if (memcpy_s(appThumbnail_ + copyIndex, appThumbnailLen_ - copyIndex, srcBuffer,
                     static_cast<uint32_t>(copyLen)) != 0) {
            return -1;
        }

        return 0;
    }
    /**
     * @tc.name: DmAppImageInfo::~DmAppImageInfo
     * @tc.desc: Dm App Image Info destructor
     * @tc.type: FUNC
     */
    ~DmAppImageInfo()
    {
        if (appIcon_ != nullptr) {
            delete[] appIcon_;
            appIcon_ = nullptr;
        }
        if (appThumbnail_ != nullptr) {
            delete[] appThumbnail_;
            appThumbnail_ = nullptr;
        }
    }
    /**
     * @tc.name: DmAppImageInfo::DmAppImageInfo
     * @tc.desc: Dm App Image Info Constructor
     * @tc.type: FUNC
     */
    DmAppImageInfo(const DmAppImageInfo &other)
    {
        if (this != &other) {
            *this = other;
        }
    }

    DmAppImageInfo &operator=(const DmAppImageInfo &other)
    {
        if (this != &other) {
            SaveData(other.GetAppIcon(), other.GetAppIconLen(), other.GetAppThumbnail(), other.GetAppThumbnailLen());
        }
        return *this;
    }

    DmAppImageInfo(DmAppImageInfo &&) = delete;
    DmAppImageInfo &operator=(DmAppImageInfo &&) = delete;
    /**
     * @tc.name: DmAppImageInfo::GetAppIconLen
     * @tc.desc: Dm App Image Info Get App Icon Len
     * @tc.type: FUNC
     */
    int32_t GetAppIconLen() const
    {
        return appIconLen_;
    }

    const uint8_t *GetAppIcon() const
    {
        return appIcon_;
    }
    /**
     * @tc.name: DmAppImageInfo::GetAppThumbnailLen
     * @tc.desc: Dm App Image Info Get App ThumbnailLen
     * @tc.type: FUNC
     */
    int32_t GetAppThumbnailLen() const
    {
        return appThumbnailLen_;
    }
    /**
     * @tc.name: DmAppImageInfo::GetAppThumbnail
     * @tc.desc: Dm App Image Info Get App Thumbnail
     * @tc.type: FUNC
     */
    const uint8_t *GetAppThumbnail() const
    {
        return appThumbnail_;
    }

private:
    void SaveData(const uint8_t *appIcon, int32_t appIconLen, const uint8_t *appThumbnail, int32_t appThumbnailLen)
    {
        SaveIconData(appIcon, appIconLen);
        SaveThumbnailData(appThumbnail, appThumbnailLen);
    }

    void SaveIconData(const uint8_t *appIcon, int32_t appIconLen)
    {
        if (appIconLen > 0 && appIconLen < ICON_MAX_LEN && appIcon != nullptr) {
            if (appIconLen_ < appIconLen) {
                if (appIcon_ != nullptr && appIconLen_ > 0) {
                    delete[] appIcon_;
                    appIcon_ = nullptr;
                    appIconLen_ = 0;
                }
                appIcon_ = new (std::nothrow) uint8_t[appIconLen] { 0 };
            }
            if (appIcon_ != nullptr) {
                appIconLen_ = appIconLen;
                if (memcpy_s(appIcon_, static_cast<uint32_t>(appIconLen_), appIcon, appIconLen) != 0) {
                    return;
                }
            }
        }
    }

    void SaveThumbnailData(const uint8_t *appThumbnail, int32_t appThumbnailLen)
    {
        if (appThumbnailLen > 0 && appThumbnailLen < THUMB_MAX_LEN && appThumbnail != nullptr) {
            if (appThumbnailLen_ < appThumbnailLen) {
                if (appThumbnail_ != nullptr && appThumbnailLen_ > 0) {
                    delete[] appThumbnail_;
                    appThumbnail_ = nullptr;
                    appThumbnailLen_ = 0;
                }
                appThumbnail_ = new (std::nothrow) uint8_t[appThumbnailLen] { 0 };
            }
            if (appThumbnail_ != nullptr) {
                appThumbnailLen_ = appThumbnailLen;
                if (memcpy_s(appThumbnail_, static_cast<uint32_t>(appThumbnailLen_), appThumbnail,
                             appThumbnailLen) != 0) {
                    return;
                }
            }
        }
    }

private:
    int32_t appIconLen_ { 0 };
    uint8_t *appIcon_ { nullptr };
    int32_t appThumbnailLen_ { 0 };
    uint8_t *appThumbnail_ { nullptr };
    const int32_t ICON_MAX_LEN = 32 * 1024;
    const int32_t THUMB_MAX_LEN = 153 * 1024;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_APP_IMAGE_INFO_H
