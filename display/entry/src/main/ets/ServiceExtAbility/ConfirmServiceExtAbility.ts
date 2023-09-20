/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../common/constant';
import type Want from '@ohos.app.ability.Want';

const TAG = '[DeviceManagerUI:Confirm]==>';

export default class ServiceExtAbility extends extension {
  onCreate(want: Want): void {
    globalThis.confirmContext = this.context;
    globalThis.confirmWindowNum = 0;
    this.getShareStyle();
  }

  onRequest(want: Want, startId: number): void {
    console.log(TAG + 'onRequest execute' + JSON.stringify(want.parameters));
    if (globalThis.confirmWindowNum !== 0) {
      console.log(TAG + 'onRequest window number is not zero.');
      return;
    }
    globalThis.abilityWant = want;
    display.getDefaultDisplay().then((dis: display.Display) => {
      let density: number = dis.densityPixels;
      let dialogRect: { left: number; top: number; width: number; height: number; } = {
        left: (dis.width - (globalThis.style.shareWidth * density)) * Constant.HALF,
        top: (dis.height - (globalThis.style.shareHeight * density)) * Constant.HALF -
          (globalThis.style.shareHeight * density) / Constant.TOP_OFFSET_PROPORTION,
        width: globalThis.style.shareWidth * density,
        height: globalThis.style.shareHeight * density * Constant.DIALOG_HEIGHT_PROPORTION
      };
      this.createWindow('picker Dialog:' + startId, window.WindowType.TYPE_FLOAT, dialogRect);
    });
  }

  getShareStyle(): void {
    globalThis.style = {};
    if (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default') {
      globalThis.style.shareWidth = Constant.SHARE_WIDTH_PHONE;
      globalThis.style.shareHeight = Constant.SHARE_HEIGHT_PHONE;
    } else {
      globalThis.style.shareWidth = Constant.SHARE_WIDTH_PAD;
      globalThis.style.shareHeight = Constant.SHARE_HEIGHT_PAD;
    }
  }

  onDestroy(): void {
    console.log(TAG + 'ServiceExtAbility destroyed');
  }

  private async createWindow(name: string, windowType: window.WindowType,
    rect: { left: number; top: number; width: number; height: number; }): Promise<void> {
    console.log(TAG + 'createWindow execute');
    try {
      const win: window.Window = await window.create(this.context, name, windowType);
      globalThis.confirmWin = win;
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.setCornerRadius(Constant.SHARE_RADIUS);
      await win.loadContent('pages/ConfirmDialog');
      await win.show();
      globalThis.confirmWindowNum++;
      console.log(TAG + 'window create successfully');
    } catch {
      console.info(TAG + 'window create failed');
    }
  }
};