/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


#include "dm_jsonstr_handle.h"

#include "json_object.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"

static const char* DM_TAG_PROXY = "proxy";

DM_IMPLEMENT_SINGLE_INSTANCE(DmJsonStrHandle);

DM_EXPORT DmVecInt64_t DmJsonstrHandleGetProxyTokenIdByExtra(const DmString* extraInfo)
{
    DmVecInt64_t tokenIdVec;
    DmVecInt64_t_Init(&tokenIdVec);

    DmJsonObject extraInfoJson;
    DmJsonObjectInit(&extraInfoJson, DM_JSON_CREATE_TYPE_OBJECT);
    if (!DmJsonObjectParse(&extraInfoJson, DmStringCstr(extraInfo))) {
        LOGE("extraInfoJson error");
        DmJsonObjectDestroy(&extraInfoJson);
        return tokenIdVec;
    }
    DmString keyProxy = DmStringCreate(DM_TAG_PROXY);
    if (!DmIsString(DmJsonObjectGetItem(&extraInfoJson), &keyProxy)) {
        LOGE("extraInfoJson error");
        DmStringDestroy(&keyProxy);
        DmJsonObjectDestroy(&extraInfoJson);
        return tokenIdVec;
    }
    DmString proxyListStr = DmJsonGetString(DmJsonObjectGetByKey(&extraInfoJson, DM_TAG_PROXY));
    DmJsonObject proxyList;
    DmJsonObjectInit(&proxyList, DM_JSON_CREATE_TYPE_OBJECT);
    if (!DmJsonObjectParse(&proxyList, DmStringCstr(&proxyListStr))) {
        LOGE("proxyListStr Parse error");
        DmStringDestroy(&proxyListStr);
        DmStringDestroy(&keyProxy);
        DmJsonObjectDestroy(&extraInfoJson);
        DmJsonObjectDestroy(&proxyList);
        return tokenIdVec;
    }
    DmVec_DmJsonItemObject items = DmJsonItemObjectItems(DmJsonObjectGetItem(&proxyList));
    for (int i = 0; i < DmVec_DmJsonItemObject_Size(&items); i++) {
        DmJsonItemObject* item = DmVec_DmJsonItemObject_At(&items, i);
        if (DmJsonItemObjectIsNumber(item)) {
            int64_t val = DmJsonGetInt64(item);
            DmVecInt64_t_Push(&tokenIdVec, val);
        }
    }
    DmVec_DmJsonItemObject_Destroy(&items);
    DmStringDestroy(&proxyListStr);
    DmStringDestroy(&keyProxy);
    DmJsonObjectDestroy(&extraInfoJson);
    DmJsonObjectDestroy(&proxyList);
    return tokenIdVec;
}
