/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../common/constant';
import common from '@ohos.app.ability.common';
import display from '@ohos.display';
import mediaQuery from '@ohos.mediaquery';
import i18n from '@ohos.i18n';
import { KeyCode } from '@ohos.multimodalInput.keyCode';
import window from '@ohos.window';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:BuildConfirmDialog]==>';
const USER_OPERATION_TYPE_BUILD_ALLOW: number = 7;
const USER_OPERATION_TYPE_BUILD_REJECT: number = 8;
const MSG_CANCEL_CONFIRM_SHOW: number = 5;
const DEVICE_TYPE_2IN1: number = 0xA2F;
const DEVICE_TYPE_PC: number = 0x0C;
let metaType: number = 11;

interface AppData {
  hostPkgLabel: string;
  bundleName: string;
  bundleInfo: string;
}

interface AppSendData {
  bundleName: string;
}

@CustomDialog
struct BuildConfirmCustomDialog {
  @State peerAppOperation: string = '';
  @State peerCustomDescription: string = '';
  @State peerDeviceName: string = '';
  @State peerDeviceType: number = 0;
  @State secondsNum: number = 30;
  @State times: number = 0;
  @State isAvailableType: boolean = false;
  @State btnColor: ResourceColor = Color.Transparent;
  @State title: string = '';
  @State isProxy: boolean = false;
  @State appDataList: AppData[] = [];
  @State selectedAppDataList: AppSendData[] = [];
  @State isFoldable: boolean = false;
  @State isFolded: display.FoldStatus = display.FoldStatus.FOLD_STATUS_UNKNOWN;
  @State mLocalHeight: number = 1;
  @State currentOrientation: window.Orientation = window.Orientation.UNSPECIFIED;
  @State marginValue: number = 0;
  @State fontSizeScale: number = AppStorage.get('fontSizeScale') as number;
  @State isPhone: boolean = false;
  @State isPC: boolean = false;
  @State isTablet: boolean = false;
  @State isCar: boolean = false;
  @State fontSize: string = '';
  @State textHeight: number = 0;
  @State scrollHeight: number = 0;
  @State buttonHeight: number = 0;
  @State columnHeight: number = 0;
  @State symbolHeight: number = 0;
  @State extraInfo: string = '';

  controller ?: CustomDialogController;
  listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(orientation: landscape)');

  private updateOrientation() {
    let isPortrait: boolean = mediaQuery.matchMediaSync('(orientation: portrait)').matches;
    this.currentOrientation = isPortrait ? window.Orientation.PORTRAIT : window.Orientation.LANDSCAPE;
    console.info(`currentOrientation: ${this.currentOrientation}`);
  }

  private updateFoldState() {
    try {
      this.isFolded = display.getFoldStatus();
      console.log('fold status:', this.isFolded);
    } catch (err) {
      console.error(`status update failed: ${err.message}`);
    }
  }

  aboutToAppear() {
    try {
      this.isFoldable = display.isFoldable();
    } catch (err) {
      console.error('Failed to get isFoldable:', err);
    }

    try {
      this.isFolded = display.getFoldStatus();
    } catch (err) {
      console.error('Failed to get FoldStatus:', err);
    }

    try {
      this.mLocalHeight = display.getDefaultDisplaySync().height;
    } catch (err) {
      console.error('Failed to get display height:', err);
    }

    this.updateOrientation();
    this.getDeviceType();

    if (AppStorage.get('title') != null) {
      this.title = AppStorage.get('title') as string;
      console.log('title is ' + this.title);
    }

    console.log(TAG + 'aboutToAppear execute PinCustomDialog')
    let context = getContext() as common.UIAbilityContext;

    this.fontSizeScale = context.config ?.fontSizeScale ?? 1;
    console.log('=-= fontScale:' + this.fontSizeScale);
    this.setMarginValue();

    if (AppStorage.get('deviceName') != null) {
      this.peerDeviceName = AppStorage.get('deviceName') as string;
    }
    let customDescriptionStr: string = AppStorage.get('customDescriptionStr') as string;
    let hostPkgLabel: string = AppStorage.get('hostPkgLabel') as string;

    let titleFirst: string =
      context.resourceManager.getStringSync($r('app.string.dm_connect_device').id, this.peerDeviceName);
    this.title =
      context.resourceManager.getStringSync($r('app.string.dm_is_trust_device').id, titleFirst);
    this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_intention').id);

    this.times = setInterval(() => {
      console.info('devicemanagerui confirm dialog run seconds:' + this.secondsNum);
      this.secondsNum--;
      if (this.secondsNum === 0) {
        clearInterval(this.times);
        this.times = 0;
        this.setUserOperation(USER_OPERATION_TYPE_BUILD_REJECT);
        this.destruction();
        console.info('click cancel times run out');
      }
    }, 1000)
    console.log(TAG + 'deviceInfo.deviceType:' + deviceInfo.deviceType);

    this.listener.on('change', () => {
      this.updateOrientation();
      try {
        this.mLocalHeight = display.getDefaultDisplaySync().height;
      } catch (err) {
        console.error('Failed to get display height:', err);
      }
    });
    try {
      display.on('foldDisplayModeChange', () => {
        this.updateFoldState();
      });
    } catch (err) {
      console.error('Failed to register event listener:', err);
    }
  }

  aboutToDisappear() {
    if (this.listener) {
      this.listener.off('change');
    }
    try {
      display.off('foldDisplayModeChange');
    } catch (err) {
      console.error('Failed to remove foldDisplayModeChange listener:', err);
    }
    clearInterval(this.times);
  }

  onAllow() {
    console.log('allow');
    if (dmClass == null) {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('allow' + USER_OPERATION_TYPE_BUILD_ALLOW);
    this.setUserOperation(USER_OPERATION_TYPE_BUILD_ALLOW);
    this.destruction();
  }

  onCancel() {
    console.log('cancel');
    if (dmClass == null) {
      console.log('createDeviceManager is null');
      return;
    }

    console.log('cancel' + USER_OPERATION_TYPE_BUILD_REJECT);
    this.setUserOperation(USER_OPERATION_TYPE_BUILD_REJECT);
    this.destruction();
  }

  getDeviceType(): void {
    this.isPhone = Constant.isPhone();
    this.isPC = Constant.isPC();
    this.isTablet = Constant.isTablet();
    this.isCar = Constant.isCar();
  }

  // fontSizeScale means senior mode display magnification scale
  setMarginValue(): void {
    if ((this.fontSizeScale >= 1) && (this.fontSizeScale < 1.75)) {
      if (this.fontSizeScale === 1) {
        this.fontSize = '20vp';
      } else if (this.fontSizeScale === 1.15) {
        this.fontSize = '23vp';
      } else if (this.fontSizeScale === 1.3) {
        this.fontSize = '26vp';
      } else if (this.fontSizeScale === 1.45) {
        this.fontSize = '29vp';
      }
      this.marginValue = 0;
    } else if (this.fontSizeScale === 1.75) {
      this.fontSize = '35vp';
      this.marginValue = 5;
    } else if (this.fontSizeScale === 2) {
      this.fontSize = '40vp';
      this.marginValue = 9;
    } else if (this.fontSizeScale === 3.2) {
      this.fontSize = '40vp';
      this.marginValue = 13;
    }
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ')
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      const param = {
          'META_TYPE': metaType
        };
      const jsonStr = JSON.stringify(param);
      dmClass.setUserOperation(operation, jsonStr);
    } catch (err) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('BuildConfirmSession');
    if (session) {
      session.terminateSelf();
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
        Column() {
          Column() {
            Text(this.title)
              .textAlign(TextAlign.Center)
              .fontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
              .fontWeight(FontWeight.Bold)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .minFontSize(11)
              .maxFontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
              .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
              .lineHeight(this.isTibetanLanguages() ? 22 : 0)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .width('auto')
              .maxLines(2)
          }
          .margin({
            top: this.isAvailableType ? 0 : 24,
            bottom: 15, left: 24, right: 24
          })
          .width('100%')
          Column() {
            Text(this.peerCustomDescription)
              .textAlign(TextAlign.Start)
              .fontColor($r('sys.color.ohos_id_color_text_secondary'))
              .fontWeight(FontWeight.Regular)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .fontSize($r('sys.float.ohos_id_text_size_body2'))
              .maxLines(2)
              .width('auto')
              .lineHeight(this.isTibetanLanguages() ? 22 : 0)
              .margin({ top: 8 })
              .visibility(this.peerCustomDescription === '' ? Visibility.None : Visibility.Visible)
          }.margin({
            top: this.isAvailableType ? 0 : 24,
            bottom: 16, left: 24, right: 24
          })

          Column() {
            Button($r('app.string.dm_allow_always'))
              .margin({ bottom: 4 })
              .padding({ top: 8, bottom: 8 })
              .onClick(() => {
                this.onAllow();
              })
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
              .width('100%')
              .backgroundColor(this.btnColor)
              .onHover((isHover ?: boolean, event ?: HoverEvent): void => {
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
            Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
              .margin({ left: 16, right: 16 })
              .padding({ top: 8, bottom: 8 })
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .defaultFocus(true)
              .onKeyEvent((event ?: KeyEvent) => {
                if (event && event ?.keyCode === KeyCode.KEYCODE_HOME && event ?.type === KeyType.Down) {
                  console.log(TAG + 'onKeyEvent eventType: ' + event ?.type)
                  return;
                }
                if (event && event ?.keyCode === KeyCode.KEYCODE_HOME && event ?.type === KeyType.Up) {
                  console.log(TAG + 'onKeyEvent eventType: ' + event ?.type)
                  this.onCancel();
                }
              })
              .onClick(() => {
                this.onCancel();
              })
              .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
              .width('100%')
              .backgroundColor(this.btnColor)
              .onHover((isHover ?: boolean, event ?: HoverEvent): void => {
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
          }
          .margin({
            left: 16,
            right: 16,
            bottom: this.isPC ? 24 : 8
          })
        }
        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .margin({ left: $r('sys.float.ohos_id_dialog_margin_start'), right: $r('sys.float.ohos_id_dialog_margin_end') })
      }
    }
    .constraintSize({ maxHeight: '90%' })
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: BuildConfirmCustomDialog(),
    autoCancel: false,
    onWillDismiss: () => {
      this.onWillDismiss()
    },
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: 0 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
  });

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist')
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
          if (msg === MSG_CANCEL_CONFIRM_SHOW) {
            console.log('cancel confirm show.')
            this.destruction()
            return
          }
        })
      })
  }

  onWillDismiss() {
    console.log(TAG + 'onWillDismiss: ' + USER_OPERATION_TYPE_BUILD_REJECT)
    this.setUserOperation(USER_OPERATION_TYPE_BUILD_REJECT);
    this.destruction();
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      const param = {
          'META_TYPE': metaType
        };
      const jsonStr = JSON.stringify(param);
      dmClass.setUserOperation(operation, jsonStr);
    } catch (err) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  onPageShow() {
    console.log('onPageShow')
    this.initStatue()
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('BuildConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear aboutToDisappear')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange');
        dmClass.release();
      } catch (err) {
        console.log('dmClass release failed')
      }
      dmClass = null
    }
  }

  build() {
    Column(this.dialogController.open())
  }
}
