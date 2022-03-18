import router from '@ohos.router'

var timel = null;
export default {
    data: {
        seconds:60,
        EVENT_CONFIRM:"EVENT_CONFIRM",
        EVENT_CANCEL:"EVENT_CANCEL",
        EVENT_INIT:"EVENT_INIT",
        EVENT_CONFIRM_CODE:"0",
        EVENT_CANCEL_CODE:"1",
        EVENT_INIT_CODE:"2",
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