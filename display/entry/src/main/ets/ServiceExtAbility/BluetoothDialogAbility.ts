/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

const TAG = '[DeviceManagerUI:BluetoothDialog]==>';
const dmCallerUid: number = 3062;

export default class ServiceExtAbility extends extension {
  isCreatingWindow: boolean = false;
  onCreate(want: Want): void {
    AppStorage.SetOrCreate('bluetoothDialogContext', this.context);
    AppStorage.SetOrCreate('bluetoothDialogWindowNum', 0);
    this.getShareStyle();
  }

  onRequest(want: Want, startId: number): void {
    if (want.parameters['ohos.aafwk.param.callerUid'] as number !== dmCallerUid) {
      console.log(TAG + 'device manager callerUid ' + want.parameters['ohos.aafwk.param.callerUid']);
      return;
    }
    console.log(TAG + 'onRequest execute' + JSON.stringify(want.parameters));
    let bluetoothDialogWindowNum: number = AppStorage.get('bluetoothDialogWindowNum');
    if (bluetoothDialogWindowNum !== 0 || this.isCreatingWindow) {
      console.log(TAG + 'onRequest window number is not zero or creating window.');
      return;
    }
    this.isCreatingWindow = true;
    AppStorage.SetOrCreate('abilityWant', want);
    let globalWant: Want = AppStorage.get('abilityWant') as Want;
    console.log(TAG + 'onRequest execute' + JSON.stringify(globalWant.parameters));

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
      AppStorage.SetOrCreate('bluetoothDialogWin', win);
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.loadContent('pages/BluetoothDialog');
      await win.show();
      let windowNum: number = AppStorage.get('bluetoothDialogWindowNum') as number;
      windowNum++;
      AppStorage.SetOrCreate('bluetoothDialogWindowNum', windowNum);
      this.isCreatingWindow = false;
      console.log(TAG + 'window create successfully');
    } catch {
      this.isCreatingWindow = false;
      console.info(TAG + 'window create failed');
    }
  }
};