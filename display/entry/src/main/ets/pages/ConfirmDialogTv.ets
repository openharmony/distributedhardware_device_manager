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
import i18n from '@ohos.i18n';
import { KeyCode } from '@ohos.multimodalInput.keyCode';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:ConfirmDialog]==>';
const ACTION_ALLOW_AUTH_ONCE: number = 0;
const ACTION_CANCEL_AUTH: number = 1;
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2;
const ACTION_ALLOW_AUTH_ALWAYS: number = 6;
const MSG_CANCEL_CONFIRM_SHOW: number = 5;
const DEVICE_TYPE_2IN1: number = 0xA2F;
const DEVICE_TYPE_PC: number = 0x0C;
const CAST_PKG_NAME: string = 'CastEngineService';

@CustomDialog
struct ConfirmCustomDialog {
  @State peerAppOperation: string = '';
  @State peerCustomDescription: string = '';
  @State peerDeviceName: string = '';
  @State peerDeviceType: number = 0;
  @State secondsNum: number = 30;
  @State times: number = 0;
  @State firstButtonWidth: number = 1;
  @State firstButtonHeight: number = 1;
  @State secondButtonWidth: number = 1;
  @State secondButtonHeight: number = 1;
  @State thirdButtonWidth: number = 1;
  @State thirdButtonHeight: number = 1;
  @State isAvailableType: boolean = false;
  @State btnColor: ResourceColor = Color.Transparent;
  @State title: string = '';
  @State mLocalWidth: number = display.getDefaultDisplaySync().width;
  controller?: CustomDialogController;
  isPC: boolean = false;
  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog')
    console.log(this.mLocalWidth + 'mLocalWidth')
    let context = getContext() as common.UIAbilityContext;

    if (AppStorage.get('deviceName') != null) {
      this.peerDeviceName = AppStorage.get('deviceName') as string + ' ';
      console.log('peerDeviceName is ' + this.peerDeviceName);
    }
    let customDescriptionStr: string = AppStorage.get('customDescriptionStr') as string;
    let hostPkgLabel: string = AppStorage.get('hostPkgLabel') as string;
    if (hostPkgLabel === CAST_PKG_NAME) {
      this.title =
        context.resourceManager.getStringSync($r('app.string.dm_confirm_title_cast').id, this.peerDeviceName);
    } else if (hostPkgLabel != null) {
      this.title = context.resourceManager.getStringSync($r('app.string.dm_confirm_title_hap').id, hostPkgLabel,
        this.peerDeviceName);
      this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_intention').id);
      if (customDescriptionStr != undefined && customDescriptionStr != '') {
        this.peerCustomDescription = this.peerDeviceName + customDescriptionStr;
      }
    } else {
      let titleFirst: string =
        context.resourceManager.getStringSync($r('app.string.dm_connect_device').id, this.peerDeviceName);
      this.title =
        context.resourceManager.getStringSync($r('app.string.dm_is_trust_device').id, titleFirst);
      this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_intention').id);
    }

    if (AppStorage.get('deviceType') != null) {
      this.peerDeviceType = AppStorage.get('deviceType') as number;
      console.log('peerDeviceType is ' + this.peerDeviceType);
    }

    this.times = setInterval(() => {
      console.info('devicemanagerui confirm dialog run seconds:' + this.secondsNum);
      this.secondsNum--;
      if (this.secondsNum === 0) {
        clearInterval(this.times);
        this.times = 0;
        this.setUserOperation(ACTION_AUTH_CONFIRM_TIMEOUT);
        this.destruction();
        console.info('click cancel times run out');
      }
    }, 1000)
    console.log(TAG + 'deviceInfo.deviceType:' + deviceInfo.deviceType);
    this.isPC = Constant.isPC();
  }

  onAllowOnce() {
    console.log('allow once')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('allow once' + ACTION_ALLOW_AUTH_ONCE)
    this.setUserOperation(ACTION_ALLOW_AUTH_ONCE)
    this.destruction()
  }

  onAllowAlways() {
    console.log('allow always')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('allow always' + ACTION_ALLOW_AUTH_ALWAYS)
    this.setUserOperation(ACTION_ALLOW_AUTH_ALWAYS)
    this.destruction()
  }

  onCancel() {
    console.log('cancel')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('cancel' + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH)
    this.destruction()
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  getImages(peerdeviceType: number): Resource {
    console.info('peerdeviceType is ' + peerdeviceType);
    if (peerdeviceType === deviceManager.DeviceType.SPEAKER) {
      this.isAvailableType = true;
      return $r('sys.symbol.soundai_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.PHONE) {
      this.isAvailableType = true;
      return $r('sys.symbol.phone_fill_1');
    } else if (peerdeviceType === deviceManager.DeviceType.TABLET) {
      this.isAvailableType = true;
      return $r('sys.symbol.pad_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.WEARABLE) {
      this.isAvailableType = true;
      return $r('sys.symbol.earphone_case_16896');
    } else if (peerdeviceType === deviceManager.DeviceType.CAR) {
      this.isAvailableType = true;
      return $r('sys.symbol.car_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.TV) {
      this.isAvailableType = true;
      return $r('sys.symbol.smartscreen_fill');
    } else if (peerdeviceType === DEVICE_TYPE_PC) {
      this.isAvailableType = true;
      return $r('sys.symbol.matebook_fill');
    } else if (peerdeviceType === DEVICE_TYPE_2IN1) {
      this.isAvailableType = true;
      return $r('sys.symbol.matebook_fill');
    } else {
      this.isAvailableType = false;
      return $r('sys.symbol.unknown_device_fill');
    }
  }

  @Builder
  Symbol() {
    Shape() {
      Column() {
        SymbolGlyph(this.getImages(this.peerDeviceType))
          .fontSize('70vp')
          .renderingStrategy(SymbolRenderingStrategy.MULTIPLE_OPACITY)
          .fontColor(['#E5FFFFFF'])
      }
    }
    .visibility(this.isAvailableType ? Visibility.Visible : Visibility.None)
    .margin({ bottom: '24vp', top: '24vp' })
  }

  private isTibetanLanguages(): boolean {
    console.info(`${TAG} isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    console.info(`${TAG} isTibetanLanguages: ${locale}`);
    return Constant.TIBETAN_LANGUAGES.includes(locale);
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 8, md: 24 },
      gutter: { x: 4 },
      breakpoints: { value: ['600vp', '840vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: 6 }, offset: { sm: 2, md: 9 } }) {
        Column() {
          this.Symbol();
          Column() {
            Text(this.title)
              .textAlign(TextAlign.Center)
              .fontSize('16fp')
              .fontWeight(FontWeight.Regular)
              .fontColor('#E5FFFFFF')
              .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
              .lineHeight(this.isTibetanLanguages() ? '22vp' : 0)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .width('auto')
              .maxLines(2)
            Text(this.peerCustomDescription)
              .textAlign(TextAlign.Start)
              .fontColor('#E5FFFFFF')
              .fontWeight(FontWeight.Regular)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .fontSize('16vp')
              .maxLines(2)
              .width('auto')
              .lineHeight(this.isTibetanLanguages() ? '22vp' : 0)
              .margin({ top: '8vp' })
              .visibility(this.peerCustomDescription === '' ? Visibility.None : Visibility.Visible)
          }.margin({
            top: this.isAvailableType ? 0 : '24vp',
            bottom: '24vp', left: '24vp', right: '24vp' })

          Column() {
            Button($r('app.string.dm_allow_always'))
              .margin({ bottom: '12vp' })
              .onClick(() => {
                this.onAllowAlways();
              })
              .fontSize('18vp')
              .fontColor('#E5FFFFFF')
              .fontWeight(FontWeight.Medium)
              .height(this.isTibetanLanguages() ? 'auto' : '40vp')
              .width('100%')
              .backgroundColor(this.btnColor)
              .scale({x: this.firstButtonWidth, y: this.firstButtonHeight})
              .focusable(true)
              .border({
                width: '0.5vp',
                color: '#26ffffff'
              })
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = '#4DFFFFFF';
                  this.firstButtonWidth = 1.05;
                  this.firstButtonHeight = 1.05;
                } else {
                  this.btnColor = '#33F1F3F5';
                  this.firstButtonWidth = 1;
                  this.firstButtonHeight = 1;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor('#26FFFFFF')
                },
                focused: {
                  .scale({x: 1.05, y: 1.05})
                  .backgroundColor('#4DFFFFFF')
                },
                normal: {
                  .backgroundColor('#33F1F3F5')
                }
              })
            Button($r('app.string.dm_allow_this_time'))
              .margin({ bottom: '12vp' })
              .onClick(() => {
                this.onAllowOnce();
              })
              .fontSize('18vp')
              .fontColor('#E5FFFFFF')
              .fontWeight(FontWeight.Medium)
              .focusable(true)
              .height(this.isTibetanLanguages() ? 'auto' : '40vp')
              .width('100%')
              .backgroundColor(this.btnColor)
              .scale({x: this.secondButtonWidth, y: this.secondButtonHeight})
              .border({
                width: '0.5vp',
                color: '#26ffffff'
              })
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = '#4DFFFFFF';
                  this.secondButtonWidth = 1.05;
                  this.secondButtonHeight = 1.05;
                } else {
                  this.btnColor = '#33F1F3F5';
                  this.secondButtonWidth = 1;
                  this.secondButtonHeight = 1;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor('#26FFFFFF')
                },
                focused: {
                  .scale({x: 1.05, y: 1.05})
                  .backgroundColor('#4DFFFFFF')
                },
                normal: {
                  .backgroundColor('#33F1F3F5')
                }
              })
            Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
              .margin({ left: '16vp', right: '16vp' })
              .fontSize('18vp')
              .fontColor('#E5FFFFFF')
              .fontWeight(FontWeight.Medium)
              .focusable(true)
              .onKeyEvent((event?: KeyEvent) => {
                if (event && event?.keyCode === KeyCode.KEYCODE_HOME && event?.type === KeyType.Down) {
                  console.log(TAG + 'onKeyEvent KEYCODE_HOME:' + event?.type)
                  return;
                }
                if (event?.keyCode == KeyCode.KEYCODE_HOME) {
                  console.log(TAG + 'onKeyEvent KEYCODE_HOME:' + event?.type)
                  this.onCancel();
                }
              })
              .onClick(() => {
                this.onCancel();
              })
              .height(this.isTibetanLanguages() ? 'auto' : '40vp')
              .width('100%')
              .backgroundColor(this.btnColor)
              .scale({x: this.thirdButtonWidth, y: this.thirdButtonHeight})
              .border({
                width: '0.5vp',
                color: '#26ffffff'
              })
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = '#4DFFFFFF';
                  this.thirdButtonWidth = 1.05;
                  this.thirdButtonHeight = 1.05;
                } else {
                  this.btnColor = '#33F1F3F5';
                  this.thirdButtonWidth = 1;
                  this.thirdButtonHeight = 1;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor('#26FFFFFF')
                },
                focused: {
                  .scale({x: 1.05, y: 1.05})
                  .backgroundColor('#4DFFFFFF')
                },
                normal: {
                  .backgroundColor('#33F1F3F5')
                }
              })
          }
          .margin({
            left: '24vp',
            right: '24vp',
            bottom: '24vp'
          })
        }
        .backgroundColor('#3C3C3C')
        .borderRadius(16)
        .border({
          width: '0.5',
          color: '#26ffffff',
          radius: '16vp'
        })
        .margin({ left: $r('sys.float.ohos_id_dialog_margin_start'), right: $r('sys.float.ohos_id_dialog_margin_end') })
        .width(px2vp(this.mLocalWidth) * 0.36)
      }
      .onKeyPreIme((event:KeyEvent) => {
        if (event?.keyCode === KeyCode.KEYCODE_MENU && event?.type === KeyType.Up) {
          console.log(TAG + 'onKeyEvent KEYCODE_MENU:' + event?.type)
          return true;
        }
        return false;
      })
      .onKeyEvent((event?: KeyEvent) => {
        if (event?.keyCode == KeyCode.KEYCODE_HOME) {
          console.log(TAG + 'onKeyEvent KEYCODE_HOME:' + event?.type)
          this.onCancel();
        }
      })
    }
    .constraintSize({ maxHeight: '80%' })
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: ConfirmCustomDialog(),
    autoCancel: false,
    onWillDismiss: ()=>{
      this.onWillDismiss()
    },
    alignment: DialogAlignment.Center,
    customStyle: true,
    maskColor: ('#CC000000')
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
    console.log(TAG + 'onWillDismiss: ' + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH);
    this.destruction();
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  onPageShow() {
    console.log('onPageShow')
    this.initStatue()
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
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
      } catch (error) {
        console.log('dmClass release failed')
      }
      dmClass = null
    }
  }

  build() {
    Column(this.dialogController.open())
  }
}