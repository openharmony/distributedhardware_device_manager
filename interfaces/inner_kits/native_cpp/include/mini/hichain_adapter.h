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

#ifndef OHOS_DEVICE_MANAGER_HICHAIN_ADAPTE_H
#define OHOS_DEVICE_MANAGER_HICHAIN_ADAPTE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int InitHichainModle(void);
int UnInitHichainModle(void);
int GetAuthFormByDeviceId(const char *deviceId, int authForm);
bool IsHichainCredentialExist(void);
int RequestHichainCredential(char **returnJsonStr);
void FreeHichainJsonStringMemory(char **jsonStr);
int CheckHichainCredential(const char *reqJsonStr, char **returnJsonStr);
int ImportHichainCredential(const char *reqJsonStr, char **returnJsonStr);
int DeleteHichainCredential(const char *reqJsonStr, char **returnJsonStr);

#ifdef __cplusplus
}
#endif

#endif // OHOS_DEVICE_MANAGER_HICHAIN_ADAPTE_H
