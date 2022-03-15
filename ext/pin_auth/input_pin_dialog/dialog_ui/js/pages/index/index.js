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

var numbs = 0;
var code = 0;
var inputVal = 0;
export default {
    data: {
        pincode: router.getParams().pinCode,
        isShow:false,
        isTimes:3,
    },
    onInit() {
        code = router.getParams().pinCode;
        callNativeHandler("EVENT_CONFIRM", "2");
    },
    onChange(e){ 
        inputVal = e.value; 
    }, 
    onConfirm() {
        numbs = numbs + 1;
        if(numbs <= 3){
            console.info('click confirm numbs < 3 ');
            if(code == inputVal){
                console.info('click confirm code == inputVal');
                callNativeHandler("EVENT_INPUT", "0");
            }else{
                if(numbs == 3){
                    console.info('click confirm code != inputVal and numbs == 3');
                    callNativeHandler("EVENT_CONFIRM", "1");
                }
                console.info('click confirm code != inputVal');
                this.isShow = true;
                this.isTimes = 3 - numbs;
            }
        }else{
            console.info('click confirm numbs > 3 ');
            callNativeHandler("EVENT_CONFIRM", "1");
        }       
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CONFIRM", "1");
    }
}