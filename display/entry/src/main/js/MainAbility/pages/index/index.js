/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import router from '@system.router';
import deviceManager from '@ohos.distributedHardware.deviceManager';
function uint8ArrayToBase64(array) {
    array = new Uint8Array(array);
    let table = ['A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
    ,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2'
    ,'3','4','5','6','7','8','9','+','/'],
        base64Str = '', length = array.byteLength, i = 0;
    for(i = 0; length - i >= 3; i += 3) {
        let num1 = array[i], num2 = array[i + 1], num3 = array[i + 2];
        base64Str += table[num1 >>> 2] + table[((num1 & 0b11) << 4) | (num2 >>> 4)] + table[((num2 & 0b1111) << 2) | 
            (num3 >>> 6)] + table[num3 & 0b111111];
    }
    const lastByte = length - i;
    if (lastByte === 1) {
        const lastNum1 = array[i];
        base64Str += table[lastNum1 >>> 2] + table[((lastNum1 & 0b11) << 4)] + '==';
    } else if (lastByte === 2) {
        const lastNum1 = array[i];
        const lastNum2 = array[i + 1];
        base64Str += table[lastNum1 >>> 2] + table[((lastNum1 & 0b11) << 4) | (lastNum2 >>> 4)] + table[(lastNum2 & 
            0b1111) << 2] + '=';
    }
    return 'data:image/png;base64,' + base64Str;
}
const TAG = "DeviceManagerUI:";

const AUTH_TYPE_PIN = 1
const AUTH_TYPE_WIFI = 2

const LOWER_CASE_KEYS = {
    line1 : ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0'],
    line2 : ['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'],
    line3 : [',', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '.'],
    line4 : [';', '*', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '-', '_'],
}
const UPPER_CASE_KEYS = {
    line1 : ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0'],
    line2 : ['Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'],
    line3 : [',', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '.'],
    line4 : [';', '*', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '-', '_'],
}

const CURRENT_FOCUS_SSID = 0
const CURRENT_FOCUS_KEY = 1

const USER_WIFI_CANCEL_CODE = 5

const ACTIVE_COLOR = '#fff'
const INACTIVE_COLOR = '#ddd'

const WIFI_CONFIGURE_SUCCESS = 0
const WIFI_CONFIGURE_FAILED = 1
const WIFI_CONFIGURE_DOING = 2
const WIFI_CONFIGURE_DEFAULT = -1

let dmClass;

export default {
    data: {
        // showType:  ['main-pin','join-authorize','join-auth','join-auth-image','join-pin']
        status: "",
        // showInfo
        statusInfo: {
            deviceName: "AppName",
            appName: 'PackageName',
            appIcon: null,
            appThumbnail: null,
            pinCode: '',
            pinToken: ''
        },
        // join: join-authorize timing
        timeRemaining: 0,
        // input pinCode
        pin: ['','','','','',''],
        // input pinCode next number
        pinNumb: 0,
        wifiInfo: {
            wifiSsid: '',
            wifiPassword: ''
        },
        keyboardItem : LOWER_CASE_KEYS,
        isSSidFocus : false,
        isKeyFocus : false,
        currentSsidColor : INACTIVE_COLOR,
        currentKeyColor : INACTIVE_COLOR,
        configureResult : WIFI_CONFIGURE_DEFAULT
    },

    log(m) {
        console.info(TAG + m);
    },

    onDestroy() {
        this.log("onDestroy")
        if (dmClass != null) {
            dmClass.off('dmFaCallback');
            dmClass.off('deviceStateChange');
            dmClass.off('serviceDie');
            dmClass.release();
            dmClass = null
        }
    },

    onShow() {
        if (dmClass) {
            this.log('deviceManager exist')
            this.initStatue()
        } else {
            this.log('createDeviceManager')
            deviceManager.createDeviceManager('com.ohos.devicemanagerui', (err, dm) => {
                this.log("createDeviceManager err:" + JSON.stringify(err) + '  --success:' + JSON.stringify(dm))
                if (err) return;
                dmClass = dm;
                dmClass.on('dmFaCallback', () => {
                    this.log("dmFaCallback executed, dialog closed")
                    router.back()
                })
                this.initStatue()
            });
        }
    },

    onHide() {
        this.log('onHide')
        this.timeRemaining = 0
    },

    /**
     * Get authentication param
     */
    initStatue() {
        this.log('initStatue')
        const data = dmClass.getFaParam();
        this.log('getFaParam:' + JSON.stringify(data))
        //        const data = dmClas icationParam:' + JSON.stringify(data))
        // Authentication type, 1 for pin code.
        if (data && !data.FaType) {
            return;
        }
        if (data && data.FaType == 1) {
            this.log('initStatue:data.FaType == 1'  )
            let authParam = data.authParam
            this.log('initStatue authParam: ' +authParam )
            this.statusInfo = {
                deviceName: authParam.extraInfo.PackageName,
                appName: authParam.extraInfo.appName,
                appIcon: uint8ArrayToBase64(authParam.appIcon),
                appThumbnail: uint8ArrayToBase64(authParam.appThumbnail),
                pinCode: authParam.extraInfo.pinCode + '',
                pinToken: authParam.extraInfo.pinToken
            }
            this.log('initStatue statusInfo: ' +this.statusInfo )
            // direction: 1(main)/0(join)
            if (authParam.extraInfo.direction == 1) {
                this.log('initStatue statusInfo: authParam.extraInfo.direction == 1')
                this.mainPin()
            } else if (authParam.appIcon) {
                this.log('initStatue statusInfo:authParam.appIcon == 1')
                this.joinAuthImage()
            } else if (authParam.extraInfo.business == 0) {
                // business: 0(FA流转)/1(资源访问)
                this.log('initStatue statusInfo:authParam.extraInfo.business == 0')
                this.joinAuth()
            } else {
                this.log('initStatue join')
                this.joinAuthorize()
            }
        }
        this.log('initStatue wifi start')
        if (data && data.FaType == AUTH_TYPE_WIFI && data.ConWifiParam == WIFI_CONFIGURE_DOING) {
            this.log("show main wifi page")
            this.mainWifi()
        }

        if (data && data.FaType == AUTH_TYPE_WIFI && data.ConWifiParam != WIFI_CONFIGURE_DOING) {
            this.log("show join wifi page")
            let wifiParam = data.conWifiParam
            this.wifiInfo.wifiSsid = wifiParam.wifiSsid;
            this.configureResult = wifiParam.configureResult
            this.joinWifi()
        }
    },

    /**
     * Set user Operation from devicemanager Fa, this interface can only used by devicemanager Fa.
     *
     * @param operateAction User Operation Actions.
     * ACTION_ALLOW_AUTH = 0, allow authentication
     * ACTION_CANCEL_AUTH = 1, cancel authentication
     * ACTION_AUTH_CONFIRM_TIMEOUT = 2, user operation timeout for authentication confirm
     * ACTION_CANCEL_PINCODE_DISPLAY = 3, cancel pinCode display
     * ACTION_CANCEL_PINCODE_INPUT = 4, cancel pinCode input
     */
    setUserOperation(operation) {
        this.log('setUserOperation: ' + operation)
        if (dmClass != null) {
            var data = dmClass.setUserOperation(operation);
            this.log('setUserOperation result: ' + JSON.stringify(data))
        } else {
            this.log('deviceManagerObject not exit')
        }
    },

    /**
     * verify auth info, such as pin code.
     * @param pinCode
     * @return
     */
    verifyAuthInfo(pinCode) {
        this.log('verifyAuthInfo: ' + pinCode)
        if (dmClass != null) {
            dmClass.verifyAuthInfo({
                "authType": 1,
                "token": this.statusInfo.pinToken,
                "extraInfo": {
                    "pinCode": +pinCode
                }
            }, (err, data) => {
                if (err) {
                    this.log("verifyAuthInfo err:" + JSON.stringify(err))
                }
                this.log("verifyAuthInfo result:" + JSON.stringify(data))
                router.back()
            });
        } else {
            this.log('deviceManagerObject not exit')
        }
    },

    /**
     * Input pinCode at the main control terminal
     */
    mainPin() {
        this.status = 'main-pin'
        this.log("mainPin")
    },

    /**
     * Enter a number with the keyboard
     * @param s
     * @return
     */
    mainInputPin(s) {
        this.log('mainInputPin input: ' + s + '-' + this.pin)
        if (this.pinNumb == 6) return
        if (this.pinNumb < 6) {
            this.pin[this.pinNumb] =
            ++this.pinNumb
            this.pin = [...this.pin]
        }
        this.log('mainInputPin pin: ' + this.pin + '-' + this.pin.join(''))
        if (this.pinNumb == 6) {
            // input end
            this.log('mainInputPin end: ' + this.pin + '-' + this.pin.join(''))
            this.verifyAuthInfo(this.pin.join(''))
            router.back()
        }
        this.log("mainInputPin")
    },

    /**
     * Keyboard delete number
     */
    mainInputPinBack() {
        if (this.pinNumb > 0) {
            --this.pinNumb
            this.pin[this.pinNumb] = ''
            this.pin = [...this.pin]
        }
        this.log("mainInputPinBack")
    },

    /**
     * Cancel pinCode input
     */
    mainInputPinCancel() {
        this.setUserOperation(4)
        router.back()
        this.log("mainInputPinCancel")
    },

    /**
     * Main end wifiInfo input Page display
     */
    mainWifi() {
        this.status = 'main-wifi'
        this.log(" this.status = 'main-wifi'")
    },

    /**
     * Join end presents wifi configure result
     */
    joinWifi() {
        this.status = 'join-wifi'
        this.log(" this.status = 'join-wifi'")
    },

    /**
     * Cancel current operation
     */
    mainWifiCancel() {
        this.log(' mainWifiCancel')
        this.setUserOperation(USER_WIFI_CANCEL_CODE)
        router.back()
    },

    /**
     * main end wifi ssid and pwd confirm
     */
    mainWifiConfirm() {
        this.log(' mainWifiConfirm')
        dmClass.configureWifiInfo(this.wifiInfo, (err, data) => {
            this.log('configureWifiInfo callback err = ' + err)
            this.log('mainWifiConfirm + wifiSsid = ' + this.wifiInfo.wifiSsid)
        })
        router.back();
    },

    /**
     * Cancel result dialog
     */
    joinWifiCancel() {
        this.log("joinWifiCancel")
        router.back()
    },

    /**
     * Join end authorization, business(FA流转)/1(资源访问): 0
     */
    joinAuthorize() {
        this.status = 'join-authorize'
        this.timing(60, 'join-authorize', () => {
            this.setUserOperation(2)
            router.back()
        })
    },

    /**
     * Join end authorization, business(FA流转)/1(资源访问): 1
     */
    joinAuth() {
        this.log("joinAuth")
        this.status = 'join-auth'
        this.timing(60, 'join-auth', () => {
            this.setUserOperation(2)
            router.back()
        })
    },

    /**
     * Join end authorization, business(FA流转)/1(资源访问): 1, show application icon
     */
    joinAuthImage() {
        this.log("joinAuthImage")
        this.status = 'join-auth-image'
        this.timing(60, 'join-auth-image', () => {
            this.setUserOperation(2)
            router.back()
        })
    },

    /**
     * Display pinCode at join end
     */
    joinPin() {
        this.status = 'join-pin'
    },

    /**
     * Cancel authorization
     */
    joinAuthorizeCancel() {
        this.setUserOperation(1)
        router.back()
    },

    /**
     * Confirm authorization
     */
    joinAuthorizeOk() {
        this.setUserOperation(0)
        this.joinPin()
    },

    /**
     * Cancel authorization
     */
    joinAuthCancel() {
        this.setUserOperation(1)
        router.back()
    },

    /**
     * Confirm authorization
     */
    joinAuthOk() {
        this.setUserOperation(0)
        this.joinPin()
    },

    /**
     * Cancel authorization
     */
    joinAuthImageCancel() {
        this.setUserOperation(1)
        router.back()
    },

    /**
     * Confirm authorization
     */
    joinAuthImageOk() {
        this.setUserOperation(0)
        this.joinPin()
    },

    /**
     * Cancel authorization
     */
    joinPinCancel() {
        this.setUserOperation(3)
        router.back()
    },

    /**
     * Pure function countdown
     * @param numb second
     * @param status
     * @param callback
     * @return
     */
    timing(numb, status, callback) {
        this.log("timing")
        this.timeRemaining = numb
        const next = () => {
            if (status != this.status) return
            --this.timeRemaining
            if (this.timeRemaining > 0) {
                setTimeout(next, 1000)
            } else {
                callback()
            }
        }
        next()
    },

    /**
     *  Change keyboard focus in wifi page
     *  @param currentArea - area that user clicked
     */
    changeFocus(currentArea) {
        this.log("changeFocus" )
        if (currentArea == CURRENT_FOCUS_SSID) {
            this.isSSidFocus = true
            this.currentSsidColor = ACTIVE_COLOR

            this.isKeyFocus = false
            this.currentKeyColor = INACTIVE_COLOR
        }
        if (currentArea == CURRENT_FOCUS_KEY) {
            this.isSSidFocus = false
            this.currentSsidColor = INACTIVE_COLOR

            this.isKeyFocus = true
            this.currentKeyColor = ACTIVE_COLOR
        }
    },

    /**
     * Change input case to upper case or lower case
     */
    changeCase() {
        this.log("changeCase" )
        if (this.keyboardItem == LOWER_CASE_KEYS) {
            this.keyboardItem = UPPER_CASE_KEYS;
        } else {
            this.keyboardItem = LOWER_CASE_KEYS;
        }
    },

    /**
     * Add char to the selected input area
     * @param itemChar - the char in the button that user clicked
     */
    inputChar(itemChar) {
        this.log("inputChar ："+itemChar)
        if (this.isSSidFocus == true) {
            this.ssidString = this.ssidString + itemChar
        }
        if (this.isKeyFocus == true) {
            this.keyString = this.keyString + itemChar
        }
    },

    /**
     * Remove the first char of the selected input area
     */
    deleteChar() {
        this.log("deleteChar")
        if (this.isSSidFocus == true && this.ssidString.length > 0) {
            this.ssidString = this.ssidString.substring(0, this.ssidString.length - 1)
        }
        if (this.isKeyFocus == true && this.keyString.length > 0) {
            this.keyString = this.keyString.substring(0, this.keyString.length - 1)
        }
    },
}
