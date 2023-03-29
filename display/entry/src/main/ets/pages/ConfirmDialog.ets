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
var that;
let dmClass;

const ACTION_ALLOW_AUTH: number = 0
const ACTION_CANCEL_AUTH: number = 1
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2
const ACTION_ALLOW_AUTH_FOREVER: number = 6

@Entry
@Component
struct dialogPlusPage {
  @State message: string = '是否允许对端连接本机'
  @State allow: string = '仅本次信任'
  @State allowForever: string = '始终信任'
  @State seconds: number = 60
  @State times: number = 0

  initStatue() {
    console.log('initStatue' + "windowNum:" + globalThis.windowNum)
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui', (err, dm) => {
      if (err) {
        console.log("createDeviceManager err:" + JSON.stringify(err) + '  --fail:' + JSON.stringify(dm))
        return
      }
      dmClass = dm
    })
  }

  onAllow() {
    console.log("allow")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("onAllow execute" + JSON.stringify(globalThis.abilityWant.parameters))
    console.log("allow" + ACTION_ALLOW_AUTH)
    this.setUserOperation(ACTION_ALLOW_AUTH)
    this.destruction()
  }

  onAllowForever() {
    console.log("allow forever")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("onAllowForever execute" + JSON.stringify(globalThis.abilityWant.parameters))
    console.log("allow forever" + ACTION_ALLOW_AUTH_FOREVER)
    this.setUserOperation(ACTION_ALLOW_AUTH_FOREVER)
    this.destruction()
  }

  onCancel() {
    console.log("cancel")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("cancel" + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH);
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
    console.info('devicemanagerui confirmdialog run seconds:' + that.seconds )
    that.seconds--;
    if (that.seconds == 0) {
      clearInterval(that.times)
      that.times = 0
      that.setUserOperation(ACTION_AUTH_CONFIRM_TIMEOUT)
      that.destruction()
      console.info('click cancel times run out')
    }
  }

  onPageShow(){
    this.initStatue()
    that = this
    if(this.times)
       return;
    this.times = setInterval(this.run,1000)
  }

  destruction() {
    if (dmClass != null) {
      dmClass.release()
      dmClass = null
    }
    globalThis.extensionWin.destroy()
    globalThis.windowNum --
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
      Button(this.allowForever)
        .margin({top: 8, bottom: 8})
        .height(40)
        .onClick(() => {
          this.onAllowForever()
        })
      Button(this.allow)
        .margin({top: 8, bottom: 8})
        .height(40)
        .onClick(() => {
          this.onAllow()
        })
      Button("取消(" + this.seconds + "秒)")
        .margin({top: 8, bottom: 8})
        .backgroundColor(0xffffff)
        .fontColor(0x0000ff)
        .height(40)
        .onClick(() => {
          this.onCancel()
        })
    }
    .padding(12)
    .height('100%')
    .width('100%')
  }
}