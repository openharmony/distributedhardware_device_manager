/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
import { BusinessError } from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../common/constant';
import i18n from '@ohos.i18n';
import { KeyCode } from '@ohos.multimodalInput.keyCode';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:PinDialog]==>';
let metaType: number = 11;
const ACTION_CANCEL_PINCODE_DISPLAY: number = 3;
const MSG_CANCEL_PIN_CODE_SHOW: number = 2;

@CustomDialog
struct PinCustomDialog {
  @State pinCode: string = '';
  @State pinCodeArr: Array<string> = [];
  @State btnColor: ResourceColor = Color.Transparent;
  @State isPC: boolean = false;
  controller?: CustomDialogController
  private scroller: Scroller = new Scroller();

  cancel() {
    console.log(TAG + 'destruction()');
    try {
      console.log(TAG + 'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch (err) {
      console.log(TAG + 'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog');
    if (AppStorage.get('metaType') != null) {
      metaType = AppStorage.get('metaType') as number;
    }
    this.isPC = Constant.isPC();
    // 获取pinCode
    this.pinCode = AppStorage.get('pinCode') as string;
    this.pinCodeArr = this.pinCode.split('');
  }

  setUserOperation(operation: number, param: string) {
    console.log(TAG + 'setUserOperation: ' + operation);
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      dmClass.setUserOperation(operation, param);
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed');
    }
  }

  private isTibetanLanguages(): boolean {
    console.info(`${TAG} isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    console.info(`${TAG} isTibetanLanguages: ${locale}`);
    return Constant.TIBETAN_LANGUAGES.includes(locale);
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 8, md: this.isPC ? 24 : 12 },
      gutter: { x: 4 },
      breakpoints: { value: ['600vp', '840vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: this.isPC ? 6 : 4 }, offset: { sm: 2, md: this.isPC ? 9 : 4 } }) {
        Scroll(this.scroller) {
          Column() {
            Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
              Text($r('app.string.dm_connect_code'))
                .fontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
                .fontColor($r('sys.color.ohos_id_color_text_primary'))
                .fontWeight(FontWeight.Bold)
                .lineHeight(this.isTibetanLanguages() ? 32.5 : 0)
                .margin({
                  left: 24,
                  right: 24
                })
            }
            .constraintSize({ minHeight: 56 })
            .margin({ bottom: this.isPC ? 16 : 8 })

            Row() {
              Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
                ForEach(this.pinCodeArr, (item: string) => {
                  Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                    Text(item)
                      .fontSize($r('sys.float.ohos_id_text_size_headline7'))
                      .fontColor($r('sys.color.ohos_id_color_text_primary'))
                      .fontWeight(FontWeight.Medium)
                  }.width('10%')
                  .height('100%')
                })
              }
              .height(48)
              .accessibilityText('[n1]' + this.pinCode + '[n0]')
            }
            .margin({ bottom: this.isPC ? 16 : 8 })

            Flex({ justifyContent: FlexAlign.Center }) {
              Button($r('app.string.dm_cancel'))
                .fontSize($r('sys.float.ohos_id_text_size_button1'))
                .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                .constraintSize({ minHeight: 40 })
                .width('100%')
                .height(57)
                .backgroundColor(this.btnColor)
                .defaultFocus(true)
                .onKeyEvent((event?: KeyEvent) => {
                  if (event && event?.keyCode === KeyCode.KEYCODE_HOME && event?.type === KeyType.Down) {
                    console.log(TAG + 'onKeyEvent eventType: ' + event?.type)
                    return;
                  }
                  if (event && event?.keyCode === KeyCode.KEYCODE_HOME && event?.type === KeyType.Up) {
                    console.log(TAG + 'onKeyEvent eventType: ' + event?.type)
                    if (this.controller) {
                      this.controller.close();
                    }
                    this.cancel();
                    const param = {
                      'META_TYPE': metaType
                    };
                    const jsonStr = JSON.stringify(param);
                    this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY, jsonStr);
                  }
                })
                .onClick(() => {
                  if (this.controller) {
                    this.controller.close();
                  }
                  this.cancel();
                  const param = {
                    'META_TYPE': metaType
                  };
                  const jsonStr = JSON.stringify(param);
                  this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY, jsonStr);
                })
                .onHover((isHover?: boolean, event?: HoverEvent): void => {
                  if (isHover) {
                    this.btnColor = $r('sys.color.ohos_id_color_hover');
                  } else {
                    this.btnColor = this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
                  }
                })
                .stateStyles({
                  pressed: {
                    .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                  },
                  normal: {
                    .backgroundColor(this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
                  }
                })
            }.margin({
              left: 16,
              right: 16,
              bottom: this.isPC ? 24 : 16
            })
          }
        }
        .scrollable(ScrollDirection.Vertical)
        .scrollBar(BarState.On)
        .constraintSize({ maxHeight: `${300}` })
        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .margin({ left: $r('sys.float.ohos_id_dialog_margin_start'), right: $r('sys.float.ohos_id_dialog_margin_end') })
      }
    }
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: PinCustomDialog(),
    cancel: this.onCancel,
    autoCancel: false,
    onWillDismiss: ()=>{
      this.onWillDismiss()
    },
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
  });

  onCancel() {
    this.destruction();
  }

  onWillDismiss() {
    console.log(TAG + 'onWillDismiss: ' + ACTION_CANCEL_PINCODE_DISPLAY)
    const param = {
      'META_TYPE': metaType
    };
    const jsonStr = JSON.stringify(param);
    this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY, jsonStr);
    this.destruction();
  }

  aboutToAppear() {
    this.initStatue();
    console.log(TAG + 'aboutToAppear execute')
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear executed')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange')
        try {
          dmClass.release();
        } catch (err) {
          let e: BusinessError = err as BusinessError;
          console.error(TAG + 'release device manager errCode:' + e.code + ',errMessage:' + e.message);
        }
      } catch (error) {
        console.log(TAG + 'dmClass release failed')
      }
      dmClass = null
    }
  }

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist');
      return;
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.pin',
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
          if (msg === MSG_CANCEL_PIN_CODE_SHOW) {
            this.destruction()
          }
        })
      });
  }

  setUserOperation(operation: number, param: string) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, param);
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  destruction() {
    console.log(TAG + 'destruction()');
    try {
      console.log(TAG + 'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch (err) {
      console.log(TAG + 'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  build() {
    Column(this.dialogController.open())
  }
}