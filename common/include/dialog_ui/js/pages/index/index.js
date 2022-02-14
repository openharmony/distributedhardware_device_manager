import router from '@ohos.router'

var timel = null;
export default {
    data: {
        message: router.getParams().targetPkgName,
        seconds:60,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
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
            callNativeHandler("EVENT_CONFIRM", "1");
        }
    },
    onConfirm() {
        console.info('click confirm');
        callNativeHandler("EVENT_CONFIRM", "0");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CANCEL", "1");
    }
}