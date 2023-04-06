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
  @State message: string = '是否允许对端连接本机'
  @State allowOnce: string = '仅本次信任'
  @State allowAlways: string = '始终信任'
  @State seconds: number = 60
  @State times: number = 0
  @State selectIndex: number = 0

  initStatue() {
    console.log('initStatue' + "windowNum:" + globalThis.windowNum)
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui', (err, dm) => {
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
    dmClass.setUserOperation(operation, "extra")
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
    this.initStatue()
    gThis = this
    if(this.times) {
      return;
    }
    this.times = setInterval(this.run, 1000)
  }

  destruction() {
    if (dmClass != null) {
      dmClass.release()
      dmClass = null
    }
    globalThis.extensionWin.destroy()
    globalThis.windowNum--
    console.info("windowNum:" + globalThis.windowNum)
    if(globalThis.windowNum == 0) {
      globalThis.extensionContext.terminateSelf()
    }
  }

  build() {
    Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center, direction: FlexDirection.Column }) {
      Text(this.message)
        .fontSize(24)
        .height(50)
      Button(this.allowAlways)
        .margin({top: 4, bottom: 4})
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
        .height(40)
        .width('100%')
      Button(this.allowOnce)
        .margin({top: 4, bottom: 4, left: 16, right: 16})
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
        .height(40)
        .width('100%')
      Button("不信任(" + this.seconds + "秒)")
        .margin({top: 4, bottom: 4, left: 16, right: 16})
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
        .height(40)
        .width('100%')
    }
    .padding(24)
    .height('100%')
    .width('100%')
    .borderRadius(24)
  }
}