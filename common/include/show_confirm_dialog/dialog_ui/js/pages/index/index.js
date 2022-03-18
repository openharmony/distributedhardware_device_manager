import router from '@ohos.router'

var timel = null;
var EVENT_CONFIRM = "EVENT_CONFIRM";
var EVENT_CANCEL = "EVENT_CANCEL";
var EVENT_INIT = "EVENT_INIT";
var EVENT_CONFIRM_CODE = "0";
var EVENT_CANCEL_CODE = "1";
var EVENT_INIT_CODE = "2";
export default {
    data: {
        seconds:60,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
        callNativeHandler(EVENT_INIT, EVENT_INIT_CODE);
    },
    onShow() {
        timel = setInterval(this.run,1000)
    },
    run() {
        this.seconds--;
        if (this.seconds == 0) {
            clearInterval(timel);
            timel = null;
            console.info('click cancel');
            callNativeHandler(EVENT_CANCEL, EVENT_CANCEL_CODE);
        }
    },
    onConfirm() {
        console.info('click confirm');
        callNativeHandler(EVENT_CONFIRM, EVENT_CONFIRM_CODE);
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler(EVENT_CANCEL, EVENT_CANCEL_CODE);
    }
}