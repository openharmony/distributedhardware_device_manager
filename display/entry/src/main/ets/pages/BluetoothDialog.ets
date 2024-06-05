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
import { BusinessError } from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../common/constant';

let TAG = '[DeviceManagerUI:BluetoothDialog]==>';

@CustomDialog
struct BluetoothCustomDialog {
  @State btnColor: ResourceColor = Color.Transparent;
  @State isPC: boolean = false;
  controller?: CustomDialogController

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute BluetoothCustomDialog');
    this.isPC = Constant.isPC();
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 8, md: this.isPC ? 24 : 12 },
      gutter: { x: 4 },
      breakpoints: { value: ['600vp', '840vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: this.isPC ? 6 : 4 }, offset: { sm: 2, md: this.isPC ? 9 : 4 } }) {
        Column() {
          Flex({ justifyContent: FlexAlign.Center }) {
            Text($r('app.string.dm_bluetooth_dialog_content'))
              .fontSize($r('sys.float.ohos_id_text_size_body1'))
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .fontWeight(FontWeight.Regular)
              .margin({
                left: 24,
                right: 24
              })
              .flexBasis('auto')
              .width('auto')
          }
          .margin({
            bottom: this.isPC ? 16 : 8,
            top: this.isPC ? 32 : 24
          })

          Flex({ justifyContent: FlexAlign.Center }) {
            Button($r('app.string.dm_bluetooth_dialog_close'))
              .fontSize($r('sys.float.ohos_id_text_size_button1'))
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .height(40)
              .width('100%')
              .backgroundColor(this.btnColor)
              .onClick(() => {
                if (this.controller) {
                  this.controller.close();
                }
                let session = AppStorage.get<UIExtensionContentSession>('bluetoothSession');
                if (session) {
                  session.terminateSelf();
                }
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
            left: this.isPC ? 72 : 16,
            right: this.isPC ? 72 : 16,
            bottom: this.isPC ? 24 : 16 })
        }
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
    builder: BluetoothCustomDialog(),
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
  });

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute')
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear executed')
  }

  build() {
    Column(this.dialogController.open())
  }
}