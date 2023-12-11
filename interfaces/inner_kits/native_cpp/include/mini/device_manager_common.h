/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DEVICE_MANAGER_COMMON_H
#define OHOS_DEVICE_MANAGER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM_MAX_REG_PKG_NUMBER          3
#define DM_SHA256_OUT_DATA_LEN         32
#define DM_HASH_DATA_LEN               16
#define DM_MAX_DEVICE_UDID_LEN         65
#define DM_MAX_DEVICE_NETWORKID_LEN    65
#define DM_MAX_DEVICE_NAME_LEN         128
#define DM_MAX_PKG_NAME_LEN            65
#define DM_MAX_DEVICE_ID_LEN           8
#define DM_MAX_USER_ID_LEN             128

#define DMLOGI(fmt, ...) printf("[devicemanager][I][%s] " fmt "\r\n", __func__, ##__VA_ARGS__)
#define DMLOGE(fmt, ...) printf("[devicemanager][E][%s] " fmt "\r\n", __func__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // OHOS_DEVICE_MANAGER_COMMON_H
