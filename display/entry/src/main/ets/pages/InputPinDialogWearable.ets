/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:InputPinDialog]==>';
const ACTION_CANCEL_PINCODE_INPUT: number = 4;
const ACTION_DONE_PINCODE_INPUT: number = 5;
const MSG_PIN_CODE_ERROR: number = 0;
const MSG_CANCEL_PIN_CODE_INPUT: number = 3;
const MSG_DOING_AUTH: number = 4;
const BUTTON_BG_COLOR = 'rgba(255, 255, 255, 0.15)';
const TOAST_DURATION_TIME_MS = 1500;
const MAX_PINCODE_LENGTH = 6;

@Entry
@Component
struct Index {
  @State isTimes: number = 3;
  @State passwordCircle: string[] = ['', '', '', '', '', ''];
  @State errorTips: Resource = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
  @State @Watch('onChangeInput') input: string = '';
  private keyboards: string[][] = [
    ['1', '2', '3'],
    ['4', '5', '6'],
    ['7', '8', '9'],
    ['', '0', 'x'],
  ];
  @State isUserInput: boolean = false;
  @State isUserOperate: boolean = false;

  dialogController: CustomDialogController = new CustomDialogController({
    builder: errTips({attemptTimes: this.isTimes, errTips: this.errorTips}),
    autoCancel: false,
    alignment: DialogAlignment.Center,
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin'),
  });

  aboutToAppear() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist');
      return;
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.input',
      (err: Error, dm: deviceManager.DeviceManager) => {
      if (err) {
        console.log('createDeviceManager err:' + JSON.stringify(err) + '  --fail:' + '${dm}');
        return;
      }
      dmClass = dm;
      dmClass.on('uiStateChange', (data: Record<string, string>) => {
        console.log('uiStateChange executed, dialog closed' + JSON.stringify(data));
        let tmpStr: Record<string, number> = JSON.parse(data.param);
        let msg: number = tmpStr.uiStateMsg as number;
        if (msg === MSG_DOING_AUTH) {
          this.errorTips = $r('app.string.dm_authenticating');
          return;
        }
        if (msg === MSG_CANCEL_PIN_CODE_INPUT) {
          this.destruction();
          return;
        }
        if (msg === MSG_PIN_CODE_ERROR) {
          this.isTimes--;
          this.errorTips = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
          this.input = '';
          this.passwordCircle = ['', '', '', '', '', ''];
          this.showToast();
        }
      });
    });
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear begin');
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange');
        dmClass.release();
      } catch (error) {
        console.log('dmClass release failed');
      }
      dmClass = null;
    }
  }

  showToast() {
    if (this.dialogController) {
      this.dialogController.open();
      setTimeout(() => {
        this.dialogController.close();
      }, TOAST_DURATION_TIME_MS);
    }
  }

  onPageHide() {
    console.log('onPageHide');
    if (this.isUserOperate) {
      console.log('user operate');
      return;
    }
    this.cancel();
  }

  destruction() {
    console.log(TAG + 'destruction begin');
    let session = AppStorage.get<UIExtensionContentSession>('inputSession');
    if (session) {
      session.terminateSelf();
    }
  }

  setUserOperation(operation: number, extra: string) {
    console.log('setUserOperation: ' + operation);
    if (dmClass === null) {
      console.log('setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      this.isUserOperate = true;
      dmClass.setUserOperation(operation, extra);
    } catch (error) {
      console.log('dmClass setUserOperation failed');
    }
  }

  cancel() {
    console.log('cancle');
    if (dmClass) {
      console.log('deviceManager exist');
    } else {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('cancle' + ACTION_CANCEL_PINCODE_INPUT);
    this.setUserOperation(ACTION_CANCEL_PINCODE_INPUT, 'extra');
    this.destruction();
  }

  confirm() {
    console.log('confirm');
    if (this.input === null || this.input === '') {
      return;
    }
    if (dmClass) {
      console.log('deviceManager exist');
    } else {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('confirm' + JSON.stringify(ACTION_DONE_PINCODE_INPUT));
    this.setUserOperation(ACTION_DONE_PINCODE_INPUT, this.input);
  }

  build() {
    Column() {
      this.titleBuilder();
      this.keyboardMocker();
    }
    .backgroundColor(Color.Black)
    .height('100%')
    .width('100%')
  }

  @Builder
  keyBuilder(key: string) {
    GridItem() {
      Button(key)
        .backgroundColor(BUTTON_BG_COLOR)
        .fontColor('#FFFFFF')
        .fontSize($r('sys.float.ohos_id_text_size_headline6'))
        .onClick(() => {
          if (this.input.length >= MAX_PINCODE_LENGTH) {
            return;
          }
          this.input += key.toString();
        })
        .size({ width: 60, height: 48 })
        .margin({ left: 3, top: 2 })
        .visibility(key === '' ? Visibility.None : Visibility.Visible)
    }
  }

  @Builder
  processKey(key: string) {
    if (key === 'x') {
      this.SymbolDelete();
    } else {
      this.keyBuilder(key);
    }
  }

  @Builder
  keyboardMocker() {
    Grid() {
      ForEach(this.keyboards, (keysArr: string[]) => {
        ForEach(keysArr, (key: string) => {
          this.processKey(key);
        })
      })
    }
    .width(189)
    .height(200)
    .margin({ top: 2 })
  }

  @Builder
  SymbolDelete() {
    SymbolGlyph($r('sys.symbol.delete_left_fill'))
      .fontSize('28vp')
      .renderingStrategy(SymbolRenderingStrategy.MULTIPLE_OPACITY)
      .fontColor(['#FFFFFF'])
      .margin({ left: 10, top: 5 })
      .onClick(() => {
        this.input = this.input.slice(0, -1);
      })
      .visibility(this.isUserInput ? Visibility.Visible : Visibility.None)
  }

  @Builder
  PinCode() {
    Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      ForEach(this.passwordCircle, (item:string) => {
        Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text(item)
            .fontSize($r('sys.float.ohos_id_text_size_sub_title2'))
            .fontColor('#FFFFFF')
            .fontWeight(FontWeight.Medium)
        }.width('7%')
        .visibility(item === '' ? Visibility.None : Visibility.Visible)
      })
      ForEach(this.passwordCircle, (item: string) => {
        Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Column()
            .width(9)
            .height(9)
            .border({ width: 1, color: '#FFFFFF', radius: 9})
            .alignItems(HorizontalAlign.Center)
        }.width('7%')
        .visibility(item === '' ? Visibility.Visible : Visibility.None)
      })
    }
  }

  @Builder
  titleBuilder() {
    Text($r('app.string.dm_enter_peer_connect_code'))
      .fontColor('#FFFFFF')
      .fontSize($r('sys.float.ohos_id_text_size_body2'))
      .margin({ top: 10 })
      .textOverflow({ overflow: TextOverflow.MARQUEE })
      .visibility(this.isUserInput ? Visibility.None : Visibility.Visible)
      .height(19)
    Stack() {
      List() {
        ListItem() {
          this.PinCode();
        }
      }
    }
    .visibility(this.isUserInput ? Visibility.Visible : Visibility.None)
    .margin({ top: 9 })
    .height(20)
  }

  onChangeInput(changedPropertyName: string) {
    let length = this.input.length;
    for (let i = 0; i < MAX_PINCODE_LENGTH; ++i) {
      if (i < length) {
        this.passwordCircle[i] = this.input[i];
      } else {
        this.passwordCircle[i] = '';
      }
    }
    if (length === MAX_PINCODE_LENGTH) {
      setTimeout(() => {
        this.setUserOperation(ACTION_DONE_PINCODE_INPUT, this.input);
      }, 50);
    }
    this.isUserInput = true;
  }
}

@CustomDialog
struct errTips {
  controller?: CustomDialogController;
  @Link attemptTimes: number;
  @Link errTips: Resource;
  build() {
    Column() {
      Text(this.errTips)
        .fontSize(15)
        .fontColor('#FFFFFF')
        .textAlign(TextAlign.Center)
        .backgroundColor('#F2404040')
    }
    .justifyContent(FlexAlign.Center)
    .width(205)
    .padding({left: 12, right: 12, top: 10, bottom: 10 })
    .backgroundColor('#F2404040')
    .borderRadius(19)
  }
}