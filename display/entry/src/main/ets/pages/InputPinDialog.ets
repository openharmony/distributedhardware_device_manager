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
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import mediaquery from '@ohos.mediaquery';
import deviceInfo from '@ohos.deviceInfo';
import display from '@ohos.display';
import inputMethod from '@ohos.inputMethod';
import Constant from '../common/constant';
import accessibility from '@ohos.accessibility';
import common from '@ohos.app.ability.common';
import i18n from '@ohos.i18n';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:InputPinDialog]==>'
const ACTION_CANCEL_PINCODE_INPUT: number = 4
const ACTION_DONE_PINCODE_INPUT: number = 5
const MSG_PIN_CODE_ERROR: number = 0
const MSG_CANCEL_PIN_CODE_INPUT: number = 3
const MSG_DOING_AUTH: number = 4
const MODEL_PIN: string = 'pin';
const MODEL_PASSWORD: string = 'password';

@CustomDialog
struct InputCustomDialog {
  @State password: string = '';
  @State passwordCircle: string[] = ['', '', '', '', '', ''];
  @State isTimes: number = 3;
  @State errorTips: Resource = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
  @State errorTipsVisible: Visibility = Visibility.None;
  @State heightNum: number = 600;
  @State targetDeviceName: string = '';
  @State model: string = MODEL_PIN;
  @State isPC: boolean = false;
  @State isPhone: boolean = false;
  @State btnColor: ResourceColor = Color.Transparent;
  @State mLocalHeight: number = display.getDefaultDisplaySync().height;
  listener: mediaquery.MediaQueryListener = mediaquery.matchMediaSync('(orientation: landscape)');
  controller?: CustomDialogController;
  private scroller: Scroller = new Scroller();

  onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
    this.mLocalHeight = display.getDefaultDisplaySync().height;
    let heightRatio = px2vp(this.mLocalHeight) * 0.2;
    if (mediaQueryResult.matches as boolean) {
        if (this.isPhone) {
            if (display.isFoldable() &&
                display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_MAIN) {
                this.heightNum = heightRatio;
            } else if (!display.isFoldable()) {
                this.heightNum = heightRatio;
            }
        } else {
            this.heightNum = 300;
        }
    } else {
        this.heightNum = 800;
    }
  }

  aboutToDisappear() {
    console.info(TAG + 'InputCustomDialog aboutToDisappear');
    let ims = inputMethod.getSetting();
    ims.off('imeShow');
  }

  aboutToAppear() {
    console.info(TAG + 'InputCustomDialog aboutToAppear');
    this.isPC = Constant.isPC();
    this.isPhone = Constant.isPhone();
    let ims = inputMethod.getSetting();
    ims.on('imeShow', (info: Array<inputMethod.InputWindowInfo>) => {
      this.scroller.scrollTo({yOffset: 72, xOffset: 0});
    });
    if (AppStorage.get('mediaQueryResult') != null && AppStorage.get('mediaQueryResult') as boolean && this.isPhone) {
        let heightRatio = px2vp(this.mLocalHeight) * 0.2;
        if (display.isFoldable() && display.getFoldDisplayMode() === display.FoldDisplayMode.FOLD_DISPLAY_MODE_MAIN) {
            this.heightNum = heightRatio;
        } else if (!display.isFoldable()) {
            this.heightNum = heightRatio;
        }
    }
    if (AppStorage.get('targetDeviceName') != null) {
      this.targetDeviceName = AppStorage.get('targetDeviceName') as string;
      console.log('targetDeviceName is ' + this.targetDeviceName);
    }
    if (AppStorage.get('model') != null) {
      this.model = AppStorage.get('model') as string;
      console.log('model is ' + this.model);
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
          this.errorTipsVisible = Visibility.Visible;
          return;
        }
        if (msg === MSG_CANCEL_PIN_CODE_INPUT) {
          this.destruction();
          return;
        }
        if (msg === MSG_PIN_CODE_ERROR) {
          this.inputCodeError();
        }
      })
    });
    this.listener.on('change', (mediaQueryResult: mediaquery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    });
  }

  sendAccessibilityEvent(times: number) {
    console.log(TAG + 'sendAccessibilityEvent in');
    let context = getContext(this) as common.UIAbilityContext;
    let str = context.resourceManager.getPluralStringValueSync($r('app.plural.dm_incorrect_code').id, times)
    let eventInfo: accessibility.EventInfo = ({
      type: 'announceForAccessibility',
      bundleName: 'com.ohos.devicemanagerui',
      triggerAction: 'common',
      textAnnouncedForAccessibility: str
    })

    try {
      accessibility.sendAccessibilityEvent(eventInfo).then(()=>{
        console.info(`${TAG} Succeeded in send event, eventInfo is ${JSON.stringify(eventInfo)}`);
      });
    } catch (error) {
      console.info(`${TAG} Failed in send event, error.message is ${error.message}`);
    }
  }

  inputCodeError() {
    console.log(TAG + 'inputCodeError in');
    if (this.model == MODEL_PASSWORD) {
      this.errorTips = $r('app.string.dm_password_error');
    } else {
      this.isTimes--;
      this.errorTips = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
      this.sendAccessibilityEvent(this.isTimes);
    }
    this.password = '';
    this.errorTipsVisible = Visibility.Visible;
    this.passwordCircle = ['', '', '', '', '', ''];
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
    if (this.password == null || this.password == '') {
      return;
    }
    if (dmClass) {
      console.log('deviceManager exist');
    } else {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('confirm' + JSON.stringify(ACTION_DONE_PINCODE_INPUT));
    this.setUserOperation(ACTION_DONE_PINCODE_INPUT, this.password);
  }

  setUserOperation(operation: number, extra: string) {
    console.log('setUserOperation: ' + operation);
    if (dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      dmClass.setUserOperation(operation, extra);
    } catch (error) {
      console.log('dmClass setUserOperation failed');
    }
  }

  destruction() {
    console.info(TAG + 'destruction');
    let inputMethodController = inputMethod.getController();
    inputMethodController.hideTextInput();
    let session = AppStorage.get<UIExtensionContentSession>('inputSession');
    if (session) {
      console.info(TAG + 'terminateSelf');
      session.terminateSelf();
    }
  }

  isNumberSix(str: string): boolean {
    console.info(TAG + 'isNumber6 in');
    const reg: RegExp = new RegExp('^[0-9]{6}$');
    return reg.test(str);
  }

  passwordOnChange(value: string) {
    console.info(TAG + 'passwordOnChange in');
    if (this.isNumberSix(value)) {
      this.confirm();
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
            Column() {
              Text($r('app.string.dm_connect', this.targetDeviceName))
                .fontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
                .fontWeight(FontWeight.Bold)
                .fontColor($r('sys.color.ohos_id_color_text_primary'))
                .margin({ top: 12, bottom: 3 })
                .width('auto')
                .textAlign(TextAlign.Center)
                .maxLines(2)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
                .minFontSize(12)
                .maxFontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
                .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                .lineHeight(this.isTibetanLanguages() ? 31 : 0)

              Text($r('app.string.dm_enter_connect_code'))
                .fontSize($r('sys.float.ohos_id_text_size_body2'))
                .fontWeight(FontWeight.Regular)
                .fontColor($r('sys.color.ohos_id_color_text_secondary'))
                .margin({ bottom: 8 })
                .width('auto')
                .maxLines(2)
                .textAlign(TextAlign.Center)
                .textOverflow({ overflow: TextOverflow.Ellipsis })
                .minFontSize(12)
                .maxFontSize($r('sys.float.ohos_id_text_size_body2'))
                .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
                .lineHeight(this.isTibetanLanguages() ? 22 : 0)
            }
            .margin({ left: 24, right: 24 })
            .constraintSize({ minHeight: 72 })
            .justifyContent(FlexAlign.Center)

            Stack() {
              List() {
                ListItem() {
                  Flex({ justifyContent: FlexAlign.Center }) {
                    ForEach(this.passwordCircle, (item:string) => {
                      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                        Text(item)
                          .fontSize($r('sys.float.ohos_id_text_size_headline7'))
                          .fontColor($r('sys.color.ohos_id_color_text_primary'))
                          .fontWeight(FontWeight.Medium)
                      }.width('10%')
                      .height('100%')
                      .visibility(item === '' ? Visibility.None : Visibility.Visible)
                    })
                    ForEach(this.passwordCircle, (item: string) => {
                      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                        Column()
                          .width(12)
                          .height(12)
                          .border({ width: 2, color: $r('sys.color.ohos_id_color_primary'), radius: 12})
                      }.width('10%')
                      .height('100%')
                      .visibility(item === '' ? Visibility.Visible : Visibility.None)
                    })
                  }
                }
              }
              TextInput({ placeholder: '', text: this.password})
                .defaultFocus(true)
                .onAppear(() => {
                  focusControl.requestFocus('inputpin')
                })
                .id('inputpin')
                .type(8)
                .height(60)
                .opacity(0)
                .fontColor(('rgba(0,0,0,0)'))
                .backgroundColor(('rgba(0,0,0,0)'))
                .caretColor(('rgba(0,0,0,0)'))
                .maxLength(6)
                .margin({ bottom: 8 })
                .width('100%')
                .onChange((value: string) => {
                  this.password = value;
                  if (value.length > 6) {
                    return;
                  }
                  let length = value.length;
                  for (let i = 0; i < 6; i++) {
                    if (i < length) {
                      this.passwordCircle[i] = value[i];
                    } else {
                      this.passwordCircle[i] = '';
                    }
                  }
                  let gThis = this;
                  setTimeout(()=> {
                    gThis.passwordOnChange(value);
                  }, 50)
                })
            }.height(48)
            .margin({ top: 12, bottom: 16})

            Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
              Text(this.errorTips)
                  .fontSize($r('sys.float.ohos_id_text_size_body2'))
                  .fontWeight(FontWeight.Medium)
                  .fontColor($r('sys.color.ohos_id_color_warning'))
                  .lineHeight(this.isTibetanLanguages() ? 22 : 0)
            }.visibility(this.errorTipsVisible)
            .margin({ bottom: 16,
                      left: $r('sys.float.ohos_id_corner_radius_dialog'),
                      right: $r('sys.float.ohos_id_corner_radius_dialog') })

            Flex({ justifyContent: FlexAlign.Center }) {
              Button($r('app.string.dm_cancel'))
                .constraintSize({ minHeight: 40 })
                .fontSize($r('sys.float.ohos_id_text_size_button1'))
                .onClick(() => {
                  if (this.controller) {
                    this.controller.close();
                  }
                  this.cancel();
                })
                .width('100%')
                .backgroundColor(this.btnColor)
                .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
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
              bottom: this.isPC ? 24 : 16 })
          }
        }
        .scrollable(ScrollDirection.Vertical)
        .scrollBar(BarState.On)
        .constraintSize({ maxHeight: `${this.heightNum}`})
        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .margin({
          left: $r('sys.float.ohos_id_dialog_margin_bottom'),
          right: $r('sys.float.ohos_id_dialog_margin_bottom')
        })
      }
    }.margin({top: 8, bottom: 20})
    .constraintSize({
      maxHeight: '40%'
    })
  }
}

@Entry
@Component
struct dialogPlusPage {
  mediaQueryListener: mediaquery.MediaQueryListener = mediaquery.matchMediaSync('(orientation: landscape)');
  dialogController: CustomDialogController = new CustomDialogController({
    builder: InputCustomDialog(),
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: 0 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
  });

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear aboutToAppear');
    this.mediaQueryListener.on('change', this.onPortrait.bind(this));
  }

  onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
    AppStorage.setOrCreate('mediaQueryResult', mediaQueryResult.matches as boolean);
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear aboutToDisappear')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange');
        dmClass.release();
      } catch (error) {
        console.log('dmClass release failed');
      }
      dmClass = null
    }
  }

  build() {
    Column(this.dialogController.open())
  }
}