import router from '@ohos.router'

var EVENT_CONFIRM = "EVENT_CONFIRM";
var EVENT_CANCEL = "EVENT_CANCEL";
var EVENT_INIT = "EVENT_INIT";
var EVENT_CONFIRM_CODE = "0";
var EVENT_CANCEL_CODE = "1";
var EVENT_INIT_CODE = "2";
export default {
    data: {
        pincode: router.getParams().pinCode,
    },
    onInit() {
        callNativeHandler(EVENT_INIT, EVENT_INIT_CODE);
    },
    onCancel() {
        console.info('click onCancel');
        callNativeHandler(EVENT_CANCEL, EVENT_CANCEL_CODE);
    }
}