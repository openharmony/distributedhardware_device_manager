/*
    Copyright (c) 2022 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

import router from '@ohos.router';

var inputValue;
var isShow = false;
var isTimes = 3;
var EVENT_CONFIRM = "EVENT_CONFIRM";
var EVENT_CANCEL = "EVENT_CANCEL";
var EVENT_INIT = "EVENT_INIT";
var EVENT_CONFIRM_CODE = "0";
var EVENT_CANCEL_CODE = "1";
var EVENT_INIT_CODE = "2";
export default {
    data: {
        isShow:  router.getParams().verifyFailed,
        inputValue: "",
    },
    onInit() {
        callNativeHandler(EVENT_INIT, EVENT_INIT_CODE);
    },
    onChange(e){ 
        inputValue = e.value;
    }, 
    onConfirm() {
        isTimes--;
        callNativeHandler(EVENT_CONFIRM, inputValue);
        inputValue = "";
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler(EVENT_CANCEL, EVENT_CANCEL_CODE);
    }
}