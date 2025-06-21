/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import deviceInfo from '@ohos.deviceInfo';

export default class Constant {
  static readonly DIALOG_HEIGHT_PROPORTION = 0.8;
  static readonly HALF = 0.5;
  static readonly TOP_OFFSET_PROPORTION = 6;
  static readonly SHARE_WIDTH_PHONE = 360;
  static readonly SHARE_HEIGHT_PHONE = 355;
  static readonly SHARE_WIDTH_PAD = 519;
  static readonly SHARE_HEIGHT_PAD = 355;
  static readonly SHARE_RADIUS = 24;
  static readonly DIALOG_TITLE_FONT_SIZE = 20;
  static readonly DIALOG_TIPS_FONT_SIZE = 14;
  static readonly TIBETAN_LANGUAGES: Array<string> = ['bo'];
  static isPhone(): boolean {
    return (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default');
  }
  static isPC(): boolean {
    return (deviceInfo.deviceType === 'pc' || deviceInfo.deviceType === '2in1');
  }
  static isCar(): boolean {
    return (deviceInfo.deviceType === 'car');
  }
  static isTablet(): boolean {
    return (deviceInfo.deviceType === 'tablet');
  }
};