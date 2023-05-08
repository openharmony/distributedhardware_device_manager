import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import deviceInfo from '@ohos.deviceInfo';
import { Constant } from '../common/constant';
import type Want from '@ohos.app.ability.Want';

const TAG = '[DeviceManagerUI:Input]==>';

export default class ServiceExtAbility extends extension {
  onCreate(want: Want): void {
    globalThis.extensionContext = this.context;
    globalThis.windowNum = 0;
    this.getShareStyle();
  }

  onRequest(want: Want, startId: number): void {
    console.log(TAG + 'onRequest execute' + JSON.stringify(want.parameters));
    // 每次startAbility拉起页面的时候, 都传递want
    globalThis.abilityWant = want;
    console.log(TAG + 'onRequest execute' + JSON.stringify(globalThis.abilityWant.parameters));

    // 每次调用startAbility时可以在这里创建窗口
    display.getDefaultDisplay().then(dis => {
      // 获取像素密度系数
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
      // 页面的默认类型是手机
      globalThis.style.shareWidth = Constant.SHARE_WIDTH_PHONE;
      globalThis.style.shareHeight = Constant.SHARE_HEIGHT_PHONE;
    } else {
      // pad类型
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
      globalThis.extensionWin = win;
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.loadContent('pages/InputPinDialog');
      await win.show();
      globalThis.windowNum ++;
      console.log(TAG + 'window create successfully');
    } catch {
      console.info(TAG + 'window create failed');
    }
  }
}