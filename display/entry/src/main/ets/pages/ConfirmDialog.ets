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
let TAG = '[DeviceManagerUI:ConfirmDialog]==>';
const ACTION_ALLOW_AUTH_ONCE: number = 0;
const ACTION_CANCEL_AUTH: number = 1;
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2;
const ACTION_ALLOW_AUTH_ALWAYS: number = 6;
const MSG_CANCEL_CONFIRM_SHOW: number = 5;
const DEVICE_TYPE_2IN1: number = 0xA2F;
const DEVICE_TYPE_PC: number = 0x0C;
const CAST_PKG_NAME: string = 'CastEngineService';

interface AppData {
  hostPkgLabel: string;
  bundleName: string;
  bundleInfo: string;
}

interface AppSendData {
  bundleName: string;
}

@CustomDialog
struct ConfirmCustomDialog {
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
    this.appDataList = this.getAppDataList();

    if (AppStorage.get('isProxyBind') != null) {
      this.isProxy = AppStorage.get('isProxyBind') as boolean;
      console.log('isProxy is ' + this.isProxy);
    }

    if (!this.isPhone && this.isProxy) {
      this.onCancel;
    }

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
      console.log('peerDeviceName is ' + this.peerDeviceName);
    }
    let customDescriptionStr: string = AppStorage.get('customDescriptionStr') as string;
    let hostPkgLabel: string = AppStorage.get('hostPkgLabel') as string;
    if (this.isProxy) {
      if (hostPkgLabel === CAST_PKG_NAME) {
        this.peerCustomDescription =
          context.resourceManager.getStringSync($r('app.string.dm_confirm_title_cast').id, this.peerDeviceName);
      } else if (hostPkgLabel != null) {
        this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_proxy').id,
          this.peerDeviceName);
      } else {
        let titleFirst: string =
          context.resourceManager.getStringSync($r('app.string.dm_connect_device').id, this.peerDeviceName);
        this.peerCustomDescription =
          context.resourceManager.getStringSync($r('app.string.dm_is_trust_device').id, titleFirst);
      }
    } else {
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

  onAllowOnce() {
    console.log('allow once')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }
    const param = {
      'appUserData': this.selectedAppDataList
    };
    const jsonStr = JSON.stringify(param);
    console.log('allow once' + ACTION_ALLOW_AUTH_ONCE)
    console.log('param:' + jsonStr)
    this.setUserAuthorization(ACTION_ALLOW_AUTH_ONCE, jsonStr)
    this.destruction()
  }

  onAllowAlways() {
    console.log('allow always')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }
    const param = {
      'appUserData': this.selectedAppDataList
    };
    const jsonStr = JSON.stringify(param);
    console.log('allow always' + ACTION_ALLOW_AUTH_ALWAYS)
    console.log('param:' + jsonStr)
    this.setUserAuthorization(ACTION_ALLOW_AUTH_ALWAYS, jsonStr)
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

  getAppDataList(): AppData[] {
    try {
      const jsonStr = AppStorage.get('appUserData') as string;
      return JSON.parse(jsonStr) as AppData[];
    } catch (err) {
      console.error('Failed to parse data:', err);
      return [];
    }
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

  setUserAuthorization(operation: number, param: string) {
    console.log(TAG + 'setUserAuthorization: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserAuthorization: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, param);
    } catch (err) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (err) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  private toggleSelection(app: AppData, index: number, checked: boolean) {
  if (checked) {
    if (!this.selectedAppDataList.some(item => item.bundleName === app.bundleName)) {
      this.selectedAppDataList = [...this.selectedAppDataList, { bundleName: app.bundleName }];
    }
  } else {
    this.selectedAppDataList = this.selectedAppDataList.filter(
      item => item.bundleName !== app.bundleName
    );
  }
  console.log('The currently selected data:', this.selectedAppDataList);
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
      Circle()
        .width(32)
        .height(32)
        .fill($r('sys.color.ohos_id_color_activated'))
      Column() {
        SymbolGlyph(this.getImages(this.peerDeviceType))
          .fontSize('20vp')
          .renderingStrategy(SymbolRenderingStrategy.MULTIPLE_OPACITY)
          .fontColor([$r('sys.color.ohos_id_color_primary_contrary')])
          .offset({ x: 6, y: 6 })
      }
        .onAreaChange((o, n) => {
          this.symbolHeight = n.height as number;
        })
    }
    .visibility(this.isAvailableType ? Visibility.Visible : Visibility.None)
    .margin({ top: 24, bottom: 15 })
  }

  private isTibetanLanguages(): boolean {
    console.info(`${TAG} isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    console.info(`${TAG} isTibetanLanguages: ${locale}`);
    return Constant.TIBETAN_LANGUAGES.includes(locale);
  }

  build() {
    if (this.isProxy && this.isPhone) {
      GridRow({
        columns: { xs: 4, sm: 8, md: this.isPC ? 24 : 12 },
        gutter: { x: 4 },
        breakpoints: { value: ['600vp', '840vp'] }
      }) {
        GridCol({ span: { xs: 4, sm: 4, md: this.isPC ? 6 : 4 }, offset: { sm: 2, md: this.isPC ? 9 : 4 } }) {
          if (this.isFoldable) {
            if (this.isFolded === display.FoldStatus.FOLD_STATUS_FOLDED) {
              if (this.currentOrientation === window.Orientation.PORTRAIT) {
                Column() {
                  this.Symbol();
                  Column() {
                    Text(this.title)
                      .textAlign(TextAlign.Center)
                      .fontSize(this.fontSize)
                      .fontWeight(FontWeight.Bold)
                      .fontColor($r('sys.color.ohos_id_color_text_primary'))
                      .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                      .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                      .textOverflow({ overflow: TextOverflow.None })
                      .width('auto')
                      .maxLines(2)
                  }
                  .onAreaChange((o, n) => {
                    this.textHeight = n.height as number;
                    console.info('**textHeight**:${this.textHeight}')
                  })
                  .margin({
                    top: this.isAvailableType ? 0 : 24,
                    bottom: 15, left: 24, right: 24
                  })
                  .accessibilityGroup(true)
                  .accessibilityLevel('yes')
                    
                  Column() {
                    Scroll(){
                      Column(){
                        Text(this.peerCustomDescription)
                          .textAlign(TextAlign.Center)
                          .fontColor($r('sys.color.font_primary'))
                          .fontSize($r('sys.float.Body_L'))
                          .width('auto')
                          .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                          .margin({
                            bottom: 12, left: 12, right: 12
                          })
                          .padding({ left: (this.isPC) ? 10 : 0, right: (this.isPC) ? 10 : 0 })
                          .accessibilityGroup(true)
                          .accessibilityLevel('yes')
    
                        Stack(){
                          List({ space: 0 }) {
                            ForEach(this.appDataList, (app: AppData, index: number) => {
                              ListItem() {
                                Row() {
                                  Column() {
                                    Column() {
                                      Text(app.hostPkgLabel)
                                        .fontSize($r('sys.float.Body_L'))
                                        .fontColor($r('sys.color.font_primary'))
                                        .textAlign(TextAlign.Start)
                                        .maxLines(2)
                                        .textOverflow({ overflow: TextOverflow.Ellipsis })
                                        .width('100%')
                                    }
                                    .margin({ top: this.marginValue, bottom: 2 })
                                    Column() {
                                      Text(app.bundleInfo)
                                        .fontSize($r('sys.float.Body_M'))
                                        .fontColor($r('sys.color.font_secondary'))
                                        .textAlign(TextAlign.Start)
                                        .width('100%')
                                    }
                                    .margin({ bottom: this.marginValue })
                                  }
                                  .layoutWeight(1)
                                  .padding({
                                    right: 8
                                  })
                                  .accessibilityGroup(true)
                                  .accessibilityLevel('yes')
                                  .margin({
                                    top: index === 0 ? 4 : 0,
                                    bottom: index === (this.appDataList.length - 1) ? 4 : 0
                                  })
    
                                  Checkbox({ name: app.hostPkgLabel, })
                                    .select(this.selectedAppDataList.some(item => item.bundleName === app.bundleName))
                                    .shape(CheckBoxShape.CIRCLE)
                                    .size({ width: 20, height: 20 })
                                    .onChange((checked: boolean) => {
                                      this.toggleSelection(app, index, checked);
                                    })
                                }
                                .width('100%')
                                .padding({ top: 11, bottom: 11, left: 12, right: 12 })
                              }
                            }, (app: AppData) => app.hostPkgLabel)
                          }
                          .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
                          .backgroundColor($r('sys.color.comp_background_list_card'))
                          .width('100%')
                          .divider({
                            strokeWidth: 1,
                            color: $r('sys.color.ohos_id_color_list_separator'),
                            startMargin: 12,
                            endMargin: 12
                          })
                          .edgeEffect(EdgeEffect.Spring)
                        }
                      }
                      .margin({ left: 12, right: 12 })
                    }
                    .height('auto')
                    .onAreaChange((o, n) => {
                      this.scrollHeight = n.height as number;
                      console.info('**scrollHeight**:${this.scrollHeight}')
                     })
                    .constraintSize({
                      maxHeight: ((px2vp(this.mLocalHeight) - 28 - 39) * 0.9 - this.symbolHeight -
                        this.buttonHeight - this.textHeight - 81)
                    })
                    .onAreaChange((o, n) => {
                      this.scrollHeight = n.height as number;
                    })
                    .scrollable(ScrollDirection.Vertical)
                    .scrollBar(BarState.On)
                  }
                  .margin({ bottom: 8})
                    
                  Column() {
                    Button($r('app.string.dm_allow_always'))
                      .margin({ top: 0, bottom: 4 })
                      .padding({ top: 8, bottom: 8 })
                      .type(ButtonType.ROUNDED_RECTANGLE)
                      .onClick(() => {
                        this.onAllowAlways();
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
  
                    Button($r('app.string.dm_allow_this_time'))
                      .margin({ bottom: 4 })
                      .padding({ top: 8, bottom: 8 })
                      .type(ButtonType.ROUNDED_RECTANGLE)
                      .onClick(() => {
                        this.onAllowOnce();
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
  
                    Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
                      .padding({ top: 8, bottom: 8 })
                      .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                      .defaultFocus(true)
                      .type(ButtonType.ROUNDED_RECTANGLE)
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
                  }
                  .onAreaChange((o, n) => {
                    this.buttonHeight = n.height as number;
                    console.info(`**buttonHeight**:${this.buttonHeight}`)
                  })
                  .margin({
                    left: 16,
                    right: 16,
                    bottom: this.isPC ? 16 : 8
                  })
                }
                .margin({
                  left: 16, right: 16,
                })
                .height('auto')
                .clip(true)
                .backgroundColor($r('sys.color.mask_fourth'))
                .borderRadius($r('sys.float.alert_container_shape'))
                .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
              } else if (this.currentOrientation === window.Orientation.LANDSCAPE) {
                Column(){
                  this.Symbol();
                  Scroll() {
                    Column() {
                      Column() {
                        Text(this.title)
                          .textAlign(TextAlign.Center)
                          .fontSize(this.fontSize)
                          .fontWeight(FontWeight.Bold)
                          .fontColor($r('sys.color.ohos_id_color_text_primary'))
                          .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                          .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                          .textOverflow({ overflow: TextOverflow.None })
                          .width('auto')
                          .maxLines(2)
                      }
                      .margin({
                        top: this.isAvailableType ? 0 : 24, left: 12, right: 12,
                        bottom: 15
                      })
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
  
                      Text(this.peerCustomDescription)
                        .textAlign(TextAlign.Center)
                        .fontColor($r('sys.color.font_primary'))
                        .fontSize($r('sys.float.Body_L'))
                        .width('auto')
                        .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                        .margin({
                          left: 12,
                          right: 12,
                          bottom: 12
                        })
                        .padding({ left: 20, right: 20 })
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
                      List({ space: 0 }) {
                        ForEach(this.appDataList, (app: AppData, index: number) => {
                          ListItem() {
                            Row() {
                              Column() {
                                Column() {
                                  Text(app.hostPkgLabel)
                                  .fontSize($r('sys.float.Body_L'))
                                    .fontColor($r('sys.color.font_primary'))
                                    .textAlign(TextAlign.Start)
                                    .maxLines(2)
                                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                                    .width('100%')
                                }
                                  .margin({ top: this.marginValue, bottom: 2 })
                                Column() {
                                  Text(app.bundleInfo)
                                    .fontSize($r('sys.float.Body_M'))
                                    .fontColor($r('sys.color.font_secondary'))
                                    .textAlign(TextAlign.Start)
                                    .width('100%')
                                }
                                  .margin({ bottom: this.marginValue })
                              }
                                .layoutWeight(1)
                                .padding({
                                  right: 12
                                })
                                .accessibilityGroup(true)
                                .accessibilityLevel('yes')
                                .margin({
                                  top: index === 0 ? 4 : 0,
                                  bottom: index === (this.appDataList.length - 1) ? 4 : 0
                                })
  
                              Checkbox({ name: app.hostPkgLabel, })
                                .select(this.selectedAppDataList.some(item => item.bundleName === app.bundleName))
                                .shape(CheckBoxShape.CIRCLE)
                                .size({ width: 20, height: 20 })
                                .onChange((checked: boolean) => {
                                  this.toggleSelection(app, index, checked);
                                  })
                            }
                            .width('100%')
                            .padding(12)
                          }
                        }, (app: AppData) => app.hostPkgLabel)
                      }
                      .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
                      .backgroundColor($r('sys.color.comp_background_list_card'))
                      .width('auto')
                      .divider({
                        strokeWidth: 1,
                        color: $r('sys.color.ohos_id_color_list_separator'),
                        startMargin: 12,
                        endMargin: 12
                      })
                      .margin({ bottom: 4 })
                      .edgeEffect(EdgeEffect.Spring)
                      .scrollBar(BarState.On)
  
                      Button($r('app.string.dm_allow_always'))
                        .margin({ top: 4, bottom: 4, right: 4, left: 4 })
                        .padding({ top: 8, bottom: 8 })
                        .type(ButtonType.ROUNDED_RECTANGLE)
                        .onClick(() => {
                          this.onAllowAlways();
                        })
                        .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                        .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
                        .width(this.isPC ? 250 : '100%')
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
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
                          
                      Button($r('app.string.dm_allow_this_time'))
                      .margin({ bottom: 4, right: 4, left: 4 })
                        .padding({ top: 8, bottom: 8 })
                        .type(ButtonType.ROUNDED_RECTANGLE)
                        .onClick(() => {
                          this.onAllowOnce();
                        })
                        .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                        .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
                        .width(this.isPC ? 250 : '100%')
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
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
  
                      Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
                        .margin({ bottom: 8, right: 4, left: 4 })
                        .padding({ top: 8, bottom: 8 })
                        .type(ButtonType.ROUNDED_RECTANGLE)
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
                        .width(this.isPC ? 250 : '100%')
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
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
                  }
                  .margin({ left: 12, right: 12 })
                }
                .border({
                  radius: { topLeft: 0, topRight: 0, bottomLeft: 32, bottomRight: 32 } 
                })
                .layoutWeight(1)
                .constraintSize({
                  maxHeight: 'auto'
                })
                .scrollable(ScrollDirection.Vertical)
                .scrollBar(BarState.On)
                }
                .width(400)
                .height(266)
                .clip(true)
                .backgroundColor($r('sys.color.mask_fourth'))
                .borderRadius($r('sys.float.alert_container_shape'))
                .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
              }
            } else if (this.isFolded === display.FoldStatus.FOLD_STATUS_EXPANDED ||
              this.isFolded === display.FoldStatus.FOLD_STATUS_HALF_FOLDED) {
              Column(){
                this.Symbol();
                Column() {
                  Text(this.title)
                    .textAlign(TextAlign.Center)
                    .fontSize(this.fontSize)
                    .fontWeight(FontWeight.Bold)
                    .fontColor($r('sys.color.ohos_id_color_text_primary'))
                    .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                    .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                    .textOverflow({ overflow: TextOverflow.None })
                    .width('auto')
                    .maxLines(2)
                }
                .onAreaChange((o, n) => {
                  this.textHeight = n.height as number;
                  console.info('**textHeight**:${this.textHeight}')
                })
                .margin({
                  top: this.isAvailableType ? 0 : (24),
                  bottom: 15, left: 24, right: 24
                })
                .accessibilityGroup(true)
                .accessibilityLevel('yes')
  
                Column() {
                  Scroll() {
                    Column() {
                      Text(this.peerCustomDescription)
                        .textAlign(TextAlign.Center)
                        .fontColor($r('sys.color.font_primary'))
                        .fontSize($r('sys.float.Body_L'))
                        .width('auto')
                        .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                        .margin({
                          bottom: 12, left: 24, right: 24
                        })
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
                      Stack() {
                        List({ space: 0 }) {
                          ForEach(this.appDataList, (app: AppData, index: number) => {
                            ListItem() {
                              Row() {
                                Column() {
                                  Column() {
                                    Text(app.hostPkgLabel)
                                      .fontSize($r('sys.float.Body_L'))
                                      .fontColor($r('sys.color.font_primary'))
                                      .textAlign(TextAlign.Start)
                                      .maxLines(2)
                                      .textOverflow({ overflow: TextOverflow.Ellipsis })
                                      .width('100%')
                                  }
                                    .margin({ top: this.marginValue, bottom: 2 })
                                  Column() {
                                    Text(app.bundleInfo)
                                      .fontSize($r('sys.float.Body_M'))
                                      .fontColor($r('sys.color.font_secondary'))
                                      .textAlign(TextAlign.Start)
                                      .width('100%')
                                  }
                                    .margin({ bottom: this.marginValue })
                                }
                                  .layoutWeight(1)
                                  .padding({
                                    right: 8
                                  })
                                  .accessibilityGroup(true)
                                  .accessibilityLevel('yes')
                                  .margin({
                                    top: index === 0 ? 4 : 0,
                                    bottom: index === (this.appDataList.length - 1) ? 4 : 0
                                  })
    
                                Checkbox({ name: app.hostPkgLabel, })
                                  .select(this.selectedAppDataList.some(item => item.bundleName === app.bundleName))
                                  .shape(CheckBoxShape.CIRCLE)
                                  .size({ width: 20, height: 20 })
                                  .onChange((checked: boolean) => {
                                    this.toggleSelection(app, index, checked);
                                  })
                              }
                                .width('100%')
                                .padding({ top: 11, bottom: 11, left: 12, right: 12 })
                            }
                          }, (app: AppData) => app.hostPkgLabel)
                        }
                        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
                        .backgroundColor($r('sys.color.comp_background_list_card'))
                        .width('100%')
                        .divider({
                          strokeWidth: 1,
                          color: $r('sys.color.ohos_id_color_list_separator'),
                          startMargin: 12,
                          endMargin: 12
                        })
                        .edgeEffect(EdgeEffect.Spring)
                      }
                    }
                    .margin({ left: 12, right: 12 })
                  }
                  .height('auto')
                  .onAreaChange((o, n) => {
                    this.scrollHeight = n.height as number;
                    console.info('**scrollHeight**:${this.scrollHeight}')
                  })
                  .constraintSize({
                    maxHeight: ((px2vp(this.mLocalHeight) - 28 - 39) * 0.9 - this.symbolHeight -
                      this.buttonHeight - this.textHeight - 81)
                  })
                  .scrollable(ScrollDirection.Vertical)
                  .scrollBar(BarState.On)
                }
                .margin({ bottom: 8})
  
                Column() {
                  Button($r('app.string.dm_allow_always'))
                    .margin({ top: 0, bottom: 4 })
                    .padding({ top: 8, bottom: 8 })
                    .type(ButtonType.ROUNDED_RECTANGLE)
                    .onClick(() => {
                      this.onAllowAlways();
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
  
                  Button($r('app.string.dm_allow_this_time'))
                    .margin({ bottom: 4 })
                    .padding({ top: 8, bottom: 8 })
                    .type(ButtonType.ROUNDED_RECTANGLE)
                    .onClick(() => {
                      this.onAllowOnce();
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
  
                  Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
                    .padding({ top: 8, bottom: 8 })
                    .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                    .defaultFocus(true)
                    .type(ButtonType.ROUNDED_RECTANGLE)
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
                }
                .onAreaChange((o, n) => {
                  this.buttonHeight = n.height as number;
                  console.info('**buttonHeight**:${this.buttonHeight}')
                })
                .margin({
                  left: 16,
                  right: 16,
                  bottom: this.isPC ? 16 : 8
                })
              }
              .width(400)
              .height('auto')
              .clip(true)
              .backgroundColor($r('sys.color.mask_fourth'))
              .borderRadius($r('sys.float.alert_container_shape'))
              .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
            }
          } else {
            if (this.currentOrientation === window.Orientation.PORTRAIT) {
              Column() {
                this.Symbol();
                Column() {
                  Text(this.title)
                    .textAlign(TextAlign.Center)
                    .fontSize(this.fontSize)
                    .fontWeight(FontWeight.Bold)
                    .fontColor($r('sys.color.ohos_id_color_text_primary'))
                    .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                    .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                    .textOverflow({ overflow: TextOverflow.None })
                    .width('auto')
                    .maxLines(2)
                }
                .onAreaChange((o, n) => {
                  this.textHeight = n.height as number;
                  console.info(`**textHeight**:${this.textHeight}`)
                })
                .margin({
                  top: this.isAvailableType ? 0 : (24),
                  bottom: 15, left: 24, right: 24
                })
                .accessibilityGroup(true)
                .accessibilityLevel('yes')
  
                Column() {
                  Scroll(){
                    Column(){
                      Text(this.peerCustomDescription)
                        .textAlign(TextAlign.Center)
                        .fontColor($r('sys.color.font_primary'))
                        .fontSize($r('sys.float.Body_L'))
                        .width('auto')
                        .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                        .margin({
                          bottom: 12, left: 12, right: 12
                        })
                        .accessibilityGroup(true)
                        .accessibilityLevel('yes')
                      
                      Stack(){
                        List({ space: 0 }) {
                          ForEach(this.appDataList, (app: AppData, index: number) => {
                            ListItem() {
                              Row() {
                                Column() {
                                  Column() {
                                    Text(app.hostPkgLabel)
                                      .fontSize($r('sys.float.Body_L'))
                                      .fontColor($r('sys.color.font_primary'))
                                      .textAlign(TextAlign.Start)
                                      .maxLines(2)
                                      .textOverflow({ overflow: TextOverflow.Ellipsis })
                                      .width('100%')
                                  }
                                  .margin({ top: this.marginValue, bottom: 2 })
                                  Column() {
                                    Text(app.bundleInfo)
                                      .fontSize($r('sys.float.Body_M'))
                                      .fontColor($r('sys.color.font_secondary'))
                                      .textAlign(TextAlign.Start)
                                      .width('100%')
                                  }
                                    .margin({ bottom: this.marginValue })
                                }
                                .layoutWeight(1)
                                .padding({
                                  right: 8
                                })
                                .accessibilityGroup(true)
                                .accessibilityLevel('yes')
                                .margin({
                                  top: index === 0 ? 4 : 0,
                                  bottom: index === (this.appDataList.length - 1) ? 4 : 0
                                })
    
                                Checkbox({ name: app.hostPkgLabel, })
                                  .select(this.selectedAppDataList.some(item => item.bundleName === app.bundleName))
                                  .shape(CheckBoxShape.CIRCLE)
                                  .size({ width: 20, height: 20 })
                                  .onChange((checked: boolean) => {
                                    this.toggleSelection(app, index, checked);
                                  })
                              }
                              .width('100%')
                              .padding({ top: 11, bottom: 11, left: 12, right: 12 })
                            }
                          }, (app: AppData) => app.hostPkgLabel)
                        }
                        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
                        .backgroundColor($r('sys.color.comp_background_list_card'))
                        .width('100%')
                        .divider({
                          strokeWidth: 1,
                          color: $r('sys.color.ohos_id_color_list_separator'),
                          startMargin: 12,
                          endMargin: 12
                        })
                        .edgeEffect(EdgeEffect.Spring)
                      }
                    }
                    .margin({ left: 12, right: 12 })
                  }
                  .height('auto')
                  .constraintSize({
                    maxHeight: ((px2vp(this.mLocalHeight) - 28 - 39) * 0.9 - this.symbolHeight -
                      this.buttonHeight - this.textHeight - 81)
                  })
                  .onAreaChange((o, n) => {
                    this.scrollHeight = n.height as number;
                    console.info(`**scrollHeight**:${this.scrollHeight}`)
                  })
                  .scrollable(ScrollDirection.Vertical)
                  .scrollBar(BarState.On)
                }
                .margin({ bottom: 8 })
  
                Column() {
                  Button($r('app.string.dm_allow_always'))
                    .margin({ top: 0, bottom: 4 })
                    .padding({ top: 8, bottom: 8 })
                    .type(ButtonType.ROUNDED_RECTANGLE)
                    .onClick(() => {
                      this.onAllowAlways();
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
  
                  Button($r('app.string.dm_allow_this_time'))
                    .margin({ bottom: 4 })
                    .padding({ top: 8, bottom: 8 })
                    .type(ButtonType.ROUNDED_RECTANGLE)
                    .onClick(() => {
                      this.onAllowOnce();
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
  
                  Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
                    .padding({ top: 8, bottom: 8 })
                    .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                    .defaultFocus(true)
                    .type(ButtonType.ROUNDED_RECTANGLE)
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
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
                  }
                .onAreaChange((o, n) => {
                  this.buttonHeight = n.height as number;
                  console.info(`**buttonHeight**:${this.buttonHeight}`)
                })
                .margin({
                  left: 16,
                  right: 16,
                  bottom: this.isPC ? 16 : 8
                })
              }
              .margin({
                left: 16, right: 16,
              })
              .height('auto')
              .clip(true)
              .backgroundColor($r('sys.color.mask_fourth'))
              .borderRadius($r('sys.float.alert_container_shape'))
              .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
            } else if (this.currentOrientation === window.Orientation.LANDSCAPE) {
              Column(){
                this.Symbol();
                Scroll() {
                  Column() {
                    Column() {
                      Text(this.title)
                        .textAlign(TextAlign.Center)
                        .fontSize(this.fontSize)
                        .fontWeight(FontWeight.Bold)
                        .fontColor($r('sys.color.ohos_id_color_text_primary'))
                        .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                        .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                        .textOverflow({ overflow: TextOverflow.None })
                        .width('auto')
                        .maxLines(2)
                    }
                    .margin({
                      top: this.isAvailableType ? 0 : 24, left: 12, right: 12,
                      bottom: 15
                    })
                    .accessibilityGroup(true)
                    .accessibilityLevel('yes')
  
                    Text(this.peerCustomDescription)
                      .textAlign(TextAlign.Center)
                      .fontColor($r('sys.color.font_primary'))
                      .fontSize($r('sys.float.Body_L'))
                      .width('auto')
                      .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                      .margin({
                        left: 12,
                        right: 12,
                        bottom: 12
                      })
                      .padding({ left: 20, right: 20 })
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
                    List({ space: 0 }) {
                      ForEach(this.appDataList, (app: AppData, index: number) => {
                        ListItem() {
                          Row() {
                            Column() {
                              Column() {
                                Text(app.hostPkgLabel)
                                  .fontSize($r('sys.float.Body_L'))
                                  .fontColor($r('sys.color.font_primary'))
                                  .textAlign(TextAlign.Start)
                                  .maxLines(2)
                                  .textOverflow({ overflow: TextOverflow.Ellipsis })
                                  .width('100%')
                              }
                              .margin({ top: this.marginValue, bottom: 2 })
                              Column() {
                                Text(app.bundleInfo)
                                  .fontSize($r('sys.float.Body_M'))
                                  .fontColor($r('sys.color.font_secondary'))
                                  .textAlign(TextAlign.Start)
                                  .width('100%')
                              }
                              .margin({ bottom: this.marginValue })
                            }
                            .layoutWeight(1)
                            .padding({
                              right: 12
                            })
                            .accessibilityGroup(true)
                            .accessibilityLevel('yes')
                            .margin({
                              top: index === 0 ? 4 : 0,
                              bottom: index === (this.appDataList.length - 1) ? 4 : 0
                            })
  
                            Checkbox({ name: app.hostPkgLabel, })
                              .select(this.selectedAppDataList.some(item => item.bundleName === app.bundleName))
                              .shape(CheckBoxShape.CIRCLE)
                              .size({ width: 20, height: 20 })
                              .onChange((checked: boolean) => {
                                this.toggleSelection(app, index, checked);
                              })
                          }
                          .width('100%')
                          .padding(12)
                        }
                      }, (app: AppData) => app.hostPkgLabel)
                    }
                    .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
                    .backgroundColor($r('sys.color.comp_background_list_card'))
                    .width('auto')
                    .divider({
                      strokeWidth: 1,
                      color: $r('sys.color.ohos_id_color_list_separator'),
                      startMargin: 12,
                      endMargin: 12
                    })
                    .margin({ bottom: 4 })
                    .edgeEffect(EdgeEffect.Spring)
                    .scrollBar(BarState.Off)
  
                    Button($r('app.string.dm_allow_always'))
                      .margin({ top: 4, bottom: 4, right: 4, left: 4 })
                      .padding({ top: 8, bottom: 8 })
                      .type(ButtonType.ROUNDED_RECTANGLE)
                      .onClick(() => {
                        this.onAllowAlways();
                      })
                      .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                      .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
                      .width(this.isPC ? 250 : '100%')
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
  
                    Button($r('app.string.dm_allow_this_time'))
                      .margin({ bottom: 4, right: 4, left: 4 })
                      .padding({ top: 8, bottom: 8 })
                      .type(ButtonType.ROUNDED_RECTANGLE)
                      .onClick(() => {
                        this.onAllowOnce();
                      })
                      .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                      .height(this.isTibetanLanguages() ? 'auto' : (this.fontSizeScale > 1 ? 56 : 40))
                      .width(this.isPC ? 250 : '100%')
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
                                          
                    Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
                      .margin({ bottom: 8, right: 4, left: 4 })
                      .padding({ top: 8, bottom: 8 })
                      .type(ButtonType.ROUNDED_RECTANGLE)
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
                      .width(this.isPC ? 250 : '100%')
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
                      .accessibilityGroup(true)
                      .accessibilityLevel('yes')
                  }
                  .margin({ left: 12, right: 12 })
                }
                .border({
                  radius: { topLeft: 0, topRight: 0, bottomLeft: 32, bottomRight: 32 }
                })
                .layoutWeight(1)
                .constraintSize({
                  maxHeight: 'auto'
                })
                .scrollable(ScrollDirection.Vertical)
                .scrollBar(BarState.On)
              }
              .width(400)
              .height(266)
              .clip(true)
              .backgroundColor($r('sys.color.mask_fourth'))
              .borderRadius($r('sys.float.alert_container_shape'))
              .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
            }
          }
        }
      }
      .onAreaChange((o, n) => {
        this.columnHeight = n.height as number;
        console.info(`**columnHeight**:${this.columnHeight}`)
      })
      .constraintSize({ maxHeight: (px2vp(this.mLocalHeight) - 28 - 39) * 0.9 })
      .height('auto')
    } else {
      GridRow({
        columns: { xs: 4, sm: 8, md: this.isPC ? 24 : 12 },
        gutter: { x: 4 },
        breakpoints: { value: ['600vp', '840vp'] }
      }) {
        GridCol({ span: { xs: 4, sm: 4, md: this.isPC ? 6 : 4 }, offset: { sm: 2, md: this.isPC ? 9 : 4 } }) {
          Column() {
            this.Symbol();
            Column() {
              Text(this.title)
                .textAlign(TextAlign.Center)
                .fontSize($r('sys.float.Title_S'))
                .fontWeight(FontWeight.Bold)
                .fontColor($r('sys.color.ohos_id_color_text_primary'))
                .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                .lineHeight(this.isTibetanLanguages() ? 22 : 0)
                .textOverflow({ overflow: TextOverflow.None })
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
                  this.onAllowAlways();
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
              Button($r('app.string.dm_allow_this_time'))
                .margin({ bottom: 4 })
                .padding({ top: 8, bottom: 8 })
                .onClick(() => {
                  this.onAllowOnce();
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
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: ConfirmCustomDialog(),
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
    } catch (err) {
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
