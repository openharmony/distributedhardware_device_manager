import router from '@ohos.router'

export default {
    data: {
        pincode: router.getParams().pinCode,
        EVENT_CONFIRM:"EVENT_CONFIRM",
        EVENT_CANCEL:"EVENT_CANCEL",
        EVENT_INIT:"EVENT_INIT",
        EVENT_CONFIRM_CODE:"0",
        EVENT_CANCEL_CODE:"1",
        EVENT_INIT_CODE:"2",
    },
    onInit() {
        callNativeHandler(EVENT_INIT, EVENT_INIT_CODE);
    },
    onCancel() {
        console.info('click onCancel');
        callNativeHandler(EVENT_CANCEL, EVENT_CANCEL_CODE);
    }
}