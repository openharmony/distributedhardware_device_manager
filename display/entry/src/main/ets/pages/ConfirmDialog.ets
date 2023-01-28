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
@Entry
@Component
struct dialogPlusPage {
  @State message: string = '是否允许对端连接本机'
  @State allow: string = '允许'
  @State seconds: number = 60
  @State cancel: string = '取消({{seconds}}秒)'
  @State times: number = 0
  @State win: any = ""
  @State ACTION_ALLOW_AUTH: number = 0
  @State ACTION_CANCEL_AUTH: number = 1
  @State ACTION_AUTH_CONFIRM_TIMEOUT: number = 2

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
    console.log("aalow")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("onAllow execute" + JSON.stringify(globalThis.abilityWant.parameters))
    console.log("aalow" + this.ACTION_ALLOW_AUTH)
    this.setUserOperation(this.ACTION_ALLOW_AUTH)
    this.destruction()
  }

  onCancel() {
    console.log("cancle")
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log("cancle" + this.ACTION_CANCEL_AUTH)
    this.setUserOperation(this.ACTION_CANCEL_AUTH);
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
          clearInterval(that.times);
          that.times = 0;
          that.setUserOperation(that.ACTION_AUTH_CONFIRM_TIMEOUT)
          that.destruction();
          console.info('click cancel times run out');
      }
  }

  onPageShow(){
    this.initStatue()
    this.win = globalThis.extensionWin
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
    this.win.destroy()
    globalThis.windowNum --
    console.info("windowNum:" + globalThis.windowNum)
    if(globalThis.windowNum == 0) {
      globalThis.extensionContext.terminateSelf()
    }
  }


  build() {
    Row() {
        Column() {
            Text(this.message)
              .fontSize(35)
              .fontWeight(FontWeight.Bold)
            Row() {}.height('2%')
            Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
              Row() {
                Button("取消" + "(" + this.seconds + "秒)")
                  .fontSize(25)
                  .backgroundColor(0xffffff)
                  .fontColor(0x0000ff)
                  .margin({left: '5%'})
                  .height(50)
                  .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
                  .fontWeight(FontWeight.Normal)
                  .onClick(() => {
                    this.onCancel()
                  }).width('55%')
                Button(this.allow)
                  .fontSize(25)
                  .height(50)
                  .margin({left: '5%'})
                  .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
                  .fontWeight(FontWeight.Normal)
                  .onClick(() => {
                    this.onAllow()
                  }).width('30%')
              }.width('100%')
            }.width('100%')
        }.width('100%')
      }.width('100%')
    .height('100%')
  }
}