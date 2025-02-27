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
    (void)info;
    (void)cb;
    return 0;
}

int32_t StopPublishLNN(const char *pkgName, int32_t publishId)
{
    (void)pkgName;
    (void)publishId;
    return 0;
}

int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    (void)pkgName;
    (void)sessionName;
    (void)listener;
    return 0;
}

