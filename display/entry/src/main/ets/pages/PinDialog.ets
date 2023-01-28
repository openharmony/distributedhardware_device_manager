/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
let TAG = "[DeviceManagerUI:PinDialog]==>"
@Entry
@Component
struct dialogPlusPage {
  @State messageTitle: string = 'PIN码连接'
  @State messageTips: string = '请在设备端输入连接码进行验证'
  @State pinCode: string = ""
  @State allow: string = '允许'
  @State cancel: string = '取消'
  @State isShow: boolean = true
  @State win: any = ""
  @State ACTION_CANCEL_PINCODE_DISPLAY: number = 3

  aboutToAppear(){
    this.initStatue()
    console.log("aboutToAppear execute")
    this.win = globalThis.extensionWin
    this.pinCode = globalThis.abilityWant.parameters["PinCode"]
  }

  aboutToDisappear() {
    console.log(TAG + "aboutToDisappear executed")
    if (dmClass != null) {
      dmClass.off('uiStateChange')
      dmClass.release()
      dmClass = null
    }
  }

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
      dmClass.on('uiStateChange', (data) => {
        console.log("uiStateChange executed, dialog closed" + JSON.stringify(data))
        var tmpStr = JSON.parse(data.param)
        this.isShow = tmpStr.verifyFailed
        console.log("uiStateChange executed, dialog closed" + this.isShow)
        if (!this.isShow) {
          this.destruction()
        }
      })
    });
  }

  setUserOperation(operation) {
    console.log('setUserOperation: ' + operation)
    if(dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    dmClass.setUserOperation(operation, "extra")
  }

  destruction() {
    this.win.destroy()
    globalThis.extensionContext.terminateSelf()
  }

  build() {
    Row() {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Column() {
          Text(this.messageTitle)
            .fontSize(35)
            .fontWeight(FontWeight.Bold)
          Text(this.messageTips)
            .fontSize(30)
            .fontWeight(FontWeight.Medium)
          Row() {}.height('2%')
          Text(this.pinCode)
            .fontSize(30)
            .fontWeight(FontWeight.Bold)
          Row() {}.height('2%')
          Button(this.cancel)
            .fontSize(25)
            .fontColor(0xffffff)
            .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
            .fontWeight(FontWeight.Normal)
            .onClick(() => {
              this.setUserOperation(this.ACTION_CANCEL_PINCODE_DISPLAY)
              this.destruction()
            })
        }.width('100%')
      }.width('100%')
      .height('100%')
    }.width('100%')
  }
}