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
var that;
let dmClass;
let TAG = "[DeviceManagerUI:InputPinDialog]==>"
@Entry
@Component
struct dialogPlusPage {
  @State messageTitle: string = 'PIN码连接'
  @State messageTips: string = '请输入另一个设备显示的PIN码进行验证'
  @State isTimes: number = 3
  @State isShow: boolean = true
  @State errorTips: string = 'PIN码输入错误，请重新输入(3次：还有' + this.isTimes + '次机会)'
  @State errorTipsVisible: number = Visibility.Hidden
  @State inputPinCode: string = ''
  @State ok: string = '确定'
  @State cancel: string = '取消'
  @State win: any = ""
  @State ACTION_CANCEL_PINCODE_INPUT: number = 4
  @State ACTION_DONE_PINCODE_INPUT: number = 5

  aboutToAppear(){
    console.log(TAG + 'aboutToDisappear aboutToDisappear')
    that = this
    this.initStatue()
    this.win = globalThis.extensionWin
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear aboutToDisappear')
    if (dmClass != null) {
      dmClass.off('uiStateChange')
      dmClass.release()
      dmClass = null
    }
  }

  initStatue() {
    console.log('initStatue')
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui', (err, dm) => {
      if (err) {
        console.log("createDeviceManager err:" + JSON.stringify(err) + '  --fail:' + '${dm}')
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
        } else {
          this.isTimes--
          this.errorTips = 'PIN码输入错误，请重新输入(3次：还有' + this.isTimes + '次机会)'
          this.inputPinCode = ''
          this.errorTipsVisible = Visibility.Visible
        }
    })
    });

  }

  onDone() {
    console.log("Done")
    if(this.inputPinCode == null || this.inputPinCode == "") {
      return;
    }
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("Done" + JSON.stringify(this.ACTION_DONE_PINCODE_INPUT))
    this.setUserOperation(this.ACTION_DONE_PINCODE_INPUT, this.inputPinCode)
  }

  onCancel() {
    console.log("cancle")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("cancle" + this.ACTION_CANCEL_PINCODE_INPUT)
    this.setUserOperation(this.ACTION_CANCEL_PINCODE_INPUT, "extra")
    this.destruction()
  }

  setUserOperation(operation, extra) {
    console.log('setUserOperation: ' + operation + 'inputPinCode'+ extra)
    if(dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    dmClass.setUserOperation(operation, extra)
  }

  destruction() {
    this.win.destroy()
    globalThis.extensionContext.terminateSelf()
  }

  build() {
    Row() {
      Column() {
          Text(this.messageTitle)
            .fontSize(35)
            .fontWeight(FontWeight.Bold)
            .height('25%')
          Text(this.messageTips)
            .fontSize(25)
            .fontWeight(FontWeight.Medium)
            .height('20%')
          Row() {}.height('2%')
          TextInput({text: this.inputPinCode})
            .fontSize(25)
            .fontWeight(FontWeight.Bold)
            .maxLength(6)
            .height('20%')
            .type(InputType.Number)
            .onChange((value) => {
              this.inputPinCode = value
              console.info(this.inputPinCode)
            })
          Text(this.errorTips)
            .fontSize(15)
            .fontWeight(FontWeight.Medium)
            .fontColor(0xff0000)
            .visibility(this.errorTipsVisible)
            .height('10%')
          Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
            Row() {
              Button(this.cancel)
                .fontSize(25)
                .backgroundColor(0xffffff)
                .fontColor(0x0000ff)
                .height(50)
                .margin({left: '10%'})
                .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
                .fontWeight(FontWeight.Normal)
                .onClick(() => {
                  this.onCancel()
                }).width('30%')
              Button(this.ok)
                .fontSize(25)
                .fontColor(0xffffff)
                .height(50)
                .margin({left: '20%'})
                .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
                .fontWeight(FontWeight.Normal)
                .onClick(() => {
                  this.onDone()
                }).width('30%')
            }.width('100%')
          }.width('100%').height('20%')
          Row() {}.height('3%')
        }.width('100%')
      }.width('100%')
      .height('100%')
  }
}