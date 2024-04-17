/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
let TAG = '[DeviceManagerUI:BluetoothDialog]==>'

const MSG_CANCEL_PIN_CODE_SHOW: number = 2
@Entry
@Component
struct dialogPlusPage {
  @State isShow: boolean = true

  aboutToAppear() {
    this.initStatue()
    console.log('aboutToAppear execute');
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear executed')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange')
        dmClass.release()
      } catch (error) {
        console.log('dmClass release failed')
      }
      dmClass = null
    }
  }

  initStatue() {
    let globalWindowNum: number = AppStorage.get('bluetoothDialogWindowNum') as number
    console.log('initStatue' + 'bluetoothDialogWindowNum:' + globalWindowNum)
    if (dmClass) {
      console.log('deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.bluetoothDialog',
      (err: Error, dm: deviceManager.DeviceManager) => {
      if (err) {
        console.log('createDeviceManager err:' + JSON.stringify(err) + '  --fail:' + JSON.stringify(dm))
        return
      }
      dmClass = dm
      dmClass.on('uiStateChange', (data: Record<string, string>) => {
        console.log('uiStateChange executed, dialog closed' + JSON.stringify(data))
        let tmpStr: Record<string, number> = JSON.parse(data.param)
        let msg: number = tmpStr.uiStateMsg as number
        if (msg === MSG_CANCEL_PIN_CODE_SHOW) {
          this.destruction()
        }
      })
    });
  }

  setUserOperation(operation: number) {
    console.log('setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra')
    } catch (error) {
      console.log('dmClass setUserOperation failed')
    }
  }

  destruction() {
    let temporaryWindow: window.Window = AppStorage.get('bluetoothDialogWin') as window.Window
    temporaryWindow.destroy()
    let context: Context.UIAbilityContext = AppStorage.get('bluetoothDialogContext') as Context.UIAbilityContext
    context.terminateSelf()
  }

  build() {
    Row() {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Column() {
          Text($r('app.string.dm_bt_not_open'))
            .fontSize(35)
            .fontWeight(FontWeight.Bold)
          Text($r('app.string.dm_open_bt'))
            .fontSize(30)
            .fontWeight(FontWeight.Medium)
          Row() {}.height('2%')
          Button($r('app.string.dm_close'))
            .fontSize(25)
            .fontColor(0xffffff)
            .border({ width: 1.5, color: (0xe7e7e7), radius: 50 })
            .fontWeight(FontWeight.Normal)
            .onClick(() => {
              this.destruction()
            })
        }.width('100%')
      }.width('100%')
      .height('100%')
    }.width('100%')
  }
}