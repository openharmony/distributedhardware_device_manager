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
import Context from '@ohos.app.ability.common';
import Want from '@ohos.app.ability.Want';
import window from '@ohos.window';

let dmClass: deviceManager.DeviceManager | null;
let gThis: dialogPlusPage;

const ACTION_ALLOW_AUTH_ONCE: number = 0
const ACTION_CANCEL_AUTH: number = 1
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2
const ACTION_ALLOW_AUTH_ALWAYS: number = 6
const MSG_CANCEL_CONFIRM: number = 5

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
    let globalWindowNum: number = AppStorage.get('confirmWindowNum') as number
    console.log('initStatue' + 'windowNum:' + globalWindowNum)
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.confirm',
      (err: Error, dm: deviceManager.DeviceManager) => {
      if (err) {
        console.log('createDeviceManager err:' + JSON.stringify(err) + ' --fail:' + JSON.stringify(dm))
        return
      }
      dmClass = dm
      dmClass.on('uiStateChange', (data: Record<string, string>) => {
        console.log('uiStateChange executed, dialog closed' + JSON.stringify(data))
        let tmpStr: Record<string, number> = JSON.parse(data.param)
        let msg: number = tmpStr.uiStateMsg as number
        if (msg === MSG_CANCEL_CONFIRM) {
          this.destruction()
        }
      })
    })
  }

  onAllowOnce() {
    console.log('allow once')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    let globalWant: Want = AppStorage.get('abilityWant') as Want
    console.log('onAllowOnce execute' + JSON.stringify(globalWant.parameters))
    console.log('allow once' + ACTION_ALLOW_AUTH_ONCE)
    this.setUserOperation(ACTION_ALLOW_AUTH_ONCE)
    this.destruction()
  }

  onAllowAlways() {
    console.log('allow always')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    let globalWant: Want = AppStorage.get('abilityWant') as Want
    console.log('onAllowAlways execute' + JSON.stringify(globalWant.parameters))
    console.log('allow always' + ACTION_ALLOW_AUTH_ALWAYS)
    this.setUserOperation(ACTION_ALLOW_AUTH_ALWAYS)
    this.destruction()
  }

  onCancel() {
    console.log('cancel')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('cancel' + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH)
    this.destruction()
  }

  setUserOperation(operation: number) {
    console.log('setUserOperation: ' + operation)
    if(dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra')
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
    console.log('onPageShow')

    let globalWant: Want = AppStorage.get('abilityWant') as Want
    if (globalWant != null && globalWant.parameters != undefined && globalWant.parameters.deviceName != null) {
      this.peerDeviceName = globalWant.parameters.deviceName as string
      console.log('peerDeviceName is ' + this.peerDeviceName)
    }

    if (globalWant != null && globalWant.parameters != undefined && globalWant.parameters.deviceType != null) {
      this.peerDeviceType = globalWant.parameters.deviceType as number
      console.log('peerDeviceType is ' + this.peerDeviceType)
    }

    if (globalWant != null && globalWant.parameters != undefined && globalWant.parameters.appOperation != null) {
      this.peerAppOperation = globalWant.parameters.appOperation as string
      console.log('peerAppOperation is ' + this.peerAppOperation)
    }

    if (globalWant != null && globalWant.parameters != undefined && globalWant.parameters.customDescription != null) {
      this.peerCustomDescription = globalWant.parameters.customDescription as string
      console.log('peerCustomDescription is ' + this.peerCustomDescription)
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
    let temporaryWindow: window.Window = AppStorage.get('confirmWin') as window.Window
    temporaryWindow.destroy()
    let globalWindowNum: number = AppStorage.get('confirmWindowNum') as number
    globalWindowNum--
    AppStorage.SetOrCreate('confirmWindowNum', globalWindowNum)
    console.info('confirmWindowNum:' + globalWindowNum)
    if(globalWindowNum == 0) {
      let context: Context.UIAbilityContext = AppStorage.get('confirmContext') as Context.UIAbilityContext
      context.terminateSelf()
    }
  }

  getImages(peerdeviceType: number): Resource {
    console.info('peerdeviceType is ' + peerdeviceType)
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
        .onTouch((event?: TouchEvent | undefined) => {
          if (event != undefined && event.type === TouchType.Down) {
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
        .onTouch((event?: TouchEvent | undefined) => {
          if (event != undefined && event.type === TouchType.Down) {
            this.selectIndex = 1
          }
        })
        .backgroundColor(this.selectIndex == 1 ? 0x0000ff : 0xffffff)
        .fontColor(this.selectIndex == 1 ? 0xffffff : 0x0000ff)
        .height(30)
        .width('100%')
      Button('不信任(' + this.seconds + '秒)')
        .margin({left: 16, right: 16})
        .backgroundColor(this.selectIndex == 2 ? 0x0000ff : 0xffffff)
        .fontColor(this.selectIndex == 2 ? 0xffffff : 0x0000ff)
        .onClick(() => {
          this.onCancel()
        })
        .onTouch((event?: TouchEvent | undefined) => {
          if (event != undefined && event.type === TouchType.Down) {
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