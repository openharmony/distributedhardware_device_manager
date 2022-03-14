import router from '@ohos.router'

export default {
    data: {
        pincode: router.getParams().pinCode,
    },
    onInit() {
        callNativeHandler("EVENT_CONFIRM", "0");
    },
    onConfirm() {
        console.info('click confirm');
        callNativeHandler("EVENT_CONFIRM", "1");
    }
}