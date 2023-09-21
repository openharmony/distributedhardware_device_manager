/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
import deviceManager from '@ohos.distributedHardware.deviceManager';

let dmClass;
var gThis;

const ACTION_ALLOW_AUTH_ONCE: number = 0
const ACTION_CANCEL_AUTH: number = 1
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2
const ACTION_ALLOW_AUTH_ALWAYS: number = 6

@Entry
@Component
struct dialogPlusPage {
  @State message: string = '是否信任此设备？'
  @State allowOnce: string = '仅本次信任'
  @State allowAlways: string = '始终信任'
  @State peerAppOperation: string = '想要连接本机。'
  @State peerCustomDescription: string = ''
  @State peerDeviceName: string = ''
  @State peerDeviceType: number = 0
  @State seconds: number = 60
  @State times: number = 0
  @State selectIndex: number = 0
  @State isAvailableType: boolean = false

  initStatue() {
    console.log('initStatue' + "windowNum:" + globalThis.confirmWindowNum)
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.confirm', (err, dm) => {
      if (err) {
        console.log("createDeviceManager err:" + JSON.stringify(err) + ' --fail:' + JSON.stringify(dm))
        return
      }
      dmClass = dm
    })
  }

  onAllowOnce() {
    console.log("allow once")
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log("onAllowOnce execute" + JSON.stringify(globalThis.abilityWant.parameters))
    console.log("allow once" + ACTION_ALLOW_AUTH_ONCE)
    this.setUserOperation(ACTION_ALLOW_AUTH_ONCE)
    this.destruction()
  }

  onAllowAlways() {
    console.log("allow always")
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log("onAllowAlways execute" + JSON.stringify(globalThis.abilityWant.parameters))
    console.log("allow always" + ACTION_ALLOW_AUTH_ALWAYS)
    this.setUserOperation(ACTION_ALLOW_AUTH_ALWAYS)
    this.destruction()
  }

  onCancel() {
    console.log("cancel")
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log("cancel" + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH)
    this.destruction()
  }

  setUserOperation(operation) {
    console.log('setUserOperation: ' + operation)
    if(dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, "extra")
    } catch (error) {
      console.log('dmClass setUserOperation failed')
    }
  }

  run() {
    console.info('devicemanagerui confirm dialog run seconds:' + gThis.seconds)
    gThis.seconds--;
    if (gThis.seconds == 0) {
      clearInterval(gThis.times)
      gThis.times = 0
      gThis.setUserOperation(ACTION_AUTH_CONFIRM_TIMEOUT)
      gThis.destruction()
      console.info('click cancel times run out')
    }
  }

  onPageShow() {
    console.log("onPageShow")

    if (globalThis.abilityWant.parameters.deviceName != null) {
      this.peerDeviceName = globalThis.abilityWant.parameters.deviceName
      console.log("peerDeviceName is " + this.peerDeviceName)
    }

    if (globalThis.abilityWant.parameters.deviceType != null) {
        this.peerDeviceType = globalThis.abilityWant.parameters.deviceType
        console.log("peerDeviceType is " + this.peerDeviceType)
    }

    if (globalThis.abilityWant.parameters.appOperation != null) {
      this.peerAppOperation = globalThis.abilityWant.parameters.appOperation
      console.log("peerAppOperation is " + this.peerAppOperation)
    }

    if (globalThis.abilityWant.parameters.customDescription != null) {
      this.peerCustomDescription = globalThis.abilityWant.parameters.customDescription
      console.log("peerCustomDescription is " + this.peerCustomDescription)
    }

    this.initStatue()
    gThis = this
    if(this.times) {
      return;
    }
    this.times = setInterval(this.run, 1000)
  }

  destruction() {
    if (dmClass != null) {
      try {
        dmClass.release()
        dmClass = null
      } catch (error) {
        console.log('dmClass release failed')
      }
    }
    globalThis.confirmWin.destroy()
    globalThis.confirmWindowNum--
    console.info("windowNum:" + globalThis.confirmWindowNum)
    if(globalThis.confirmWindowNum == 0) {
      globalThis.confirmContext.terminateSelf()
    }
  }

  getImages(peerdeviceType): Resource {
    console.info("peerdeviceType is " + peerdeviceType)
    if (peerdeviceType == deviceManager.DeviceType.SPEAKER) {
      this.isAvailableType = true
      return $r('app.media.ic_device_soundx')
    } else if (peerdeviceType == deviceManager.DeviceType.PHONE) {
      this.isAvailableType = true
      return $r('app.media.ic_public_devices_phone')
    } else if (peerdeviceType == deviceManager.DeviceType.TABLET) {
      this.isAvailableType = true
      return $r('app.media.ic_device_pad')
    } else if (peerdeviceType == deviceManager.DeviceType.WEARABLE) {
      this.isAvailableType = true
      return $r('app.media.ic_device_watch')
    } else if (peerdeviceType == deviceManager.DeviceType.CAR) {
      this.isAvailableType = true
      return $r('app.media.ic_public_car')
    } else if (peerdeviceType == deviceManager.DeviceType.TV) {
      this.isAvailableType = true
      return $r('app.media.ic_device_smartscreen')
    } else {
      this.isAvailableType = false
      return $r('app.media.icon')
    }
  }

  build() {
    Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center, direction: FlexDirection.Column }) {
      Image(this.getImages(this.peerDeviceType))
      .width(24)
      .height(24)
      .margin({bottom: 16})
      .visibility(this.isAvailableType == false? Visibility.None : Visibility.Visible)
      Text(this.peerDeviceName + this.peerAppOperation + this.message)
        .textAlign(TextAlign.Start)
        .fontSize(16)
        .maxLines(3)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .width('100%')
        .opacity(0.9)
      Text(this.peerCustomDescription)
        .textAlign(TextAlign.Start)
        .textOverflow({ overflow: TextOverflow.Ellipsis })
        .fontSize(14)
        .maxLines(3)
        .constraintSize({ maxHeight: 45 })
        .margin({bottom: 16})
        .width('100%')
        .opacity(0.6)
      Button(this.allowAlways)
        .margin({bottom: 2})
        .onClick(() => {
          this.onAllowAlways()
        })
        .onTouch((event: TouchEvent) => {
          if (event.type === TouchType.Down) {
            this.selectIndex = 0
          }
        })
        .backgroundColor(this.selectIndex == 0 ? 0x0000ff : 0xffffff)
        .fontColor(this.selectIndex == 0 ? 0xffffff : 0x0000ff)
        .height(30)
        .width('100%')
      Button(this.allowOnce)
        .margin({bottom: 2, left: 16, right: 16})
        .onClick(() => {
          this.onAllowOnce()
        })
        .onTouch((event: TouchEvent) => {
          if (event.type === TouchType.Down) {
            this.selectIndex = 1
          }
        })
        .backgroundColor(this.selectIndex == 1 ? 0x0000ff : 0xffffff)
        .fontColor(this.selectIndex == 1 ? 0xffffff : 0x0000ff)
        .height(30)
        .width('100%')
      Button("不信任(" + this.seconds + "秒)")
        .margin({left: 16, right: 16})
        .backgroundColor(this.selectIndex == 2 ? 0x0000ff : 0xffffff)
        .fontColor(this.selectIndex == 2 ? 0xffffff : 0x0000ff)
        .onClick(() => {
          this.onCancel()
        })
        .onTouch((event: TouchEvent) => {
          if (event.type === TouchType.Down) {
            this.selectIndex = 2
          }
        })
        .height(30)
        .width('100%')
    }
    .padding({top:12, bottom:12, left: 16, right: 16})
    .height('100%')
    .width('100%')
    .borderRadius(24)
  }
}