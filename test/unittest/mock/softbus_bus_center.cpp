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

#include "softbus_connector.h"

#include "softbus_bus_center_mock.h"

namespace {
int32_t g_mockPublishLnnRet = 0;
int32_t g_mockStopPublishLnnRet = 0;
int32_t g_mockPublishLnnCallCount = 0;
int32_t g_mockStopPublishLnnCallCount = 0;
int32_t g_mockPublishLnnLastPublishId = -1;
int32_t g_mockStopPublishLnnLastPublishId = -1;
}

namespace OHOS {
namespace DistributedHardware {
void ResetSoftbusBusCenterMock()
{
    g_mockPublishLnnRet = 0;
    g_mockStopPublishLnnRet = 0;
    g_mockPublishLnnCallCount = 0;
    g_mockStopPublishLnnCallCount = 0;
    g_mockPublishLnnLastPublishId = -1;
    g_mockStopPublishLnnLastPublishId = -1;
}

void SetPublishLnnMockRet(int32_t ret)
{
    g_mockPublishLnnRet = ret;
}

void SetStopPublishLnnMockRet(int32_t ret)
{
    g_mockStopPublishLnnRet = ret;
}

int32_t GetPublishLnnMockCallCount()
{
    return g_mockPublishLnnCallCount;
}

int32_t GetStopPublishLnnMockCallCount()
{
    return g_mockStopPublishLnnCallCount;
}

int32_t GetPublishLnnMockLastPublishId()
{
    return g_mockPublishLnnLastPublishId;
}

int32_t GetStopPublishLnnMockLastPublishId()
{
    return g_mockStopPublishLnnLastPublishId;
}
} // namespace DistributedHardware
} // namespace OHOS

int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, NodeDeviceInfoKey key, uint8_t *info,
                       int32_t infoLen)
{
    (void)pkgName;
    (void)networkId;
    (void)key;
    (void)info;
    (void)infoLen;
    return 0;
}

int32_t RegNodeDeviceStateCb(const char *pkgName, INodeStateCb *callback)
{
    (void)pkgName;
    (void)callback;
    return 0;
};

int32_t GetAllNodeDeviceInfo(const char *pkgName, NodeBasicInfo **info, int32_t *infoNum)
{
    (void)pkgName;
    (void)info;
    (void)infoNum;
    return 0;
}

int32_t PublishLNN(const char *pkgName, const PublishInfo *info, const IPublishCb *cb)
{
    (void)pkgName;
    (void)cb;
    g_mockPublishLnnCallCount++;
    g_mockPublishLnnLastPublishId = (info == nullptr) ? -1 : info->publishId;
    return g_mockPublishLnnRet;
}

int32_t StopPublishLNN(const char *pkgName, int32_t publishId)
{
    (void)pkgName;
    g_mockStopPublishLnnCallCount++;
    g_mockStopPublishLnnLastPublishId = publishId;
    return g_mockStopPublishLnnRet;
}

int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    (void)pkgName;
    (void)sessionName;
    (void)listener;
    return 0;
}

