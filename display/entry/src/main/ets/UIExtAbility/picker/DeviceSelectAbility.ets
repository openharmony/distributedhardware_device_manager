/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility'
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import Want from '@ohos.app.ability.Want';
const TAG = '[DeviceManagerUI:DeviceSelectAbility]==>';

export default class DeviceSelectAbility extends UIExtensionAbility {
  onSessionCreate(want: Want, session: UIExtensionContentSession) {
    console.log(TAG, `UIExtAbility onSessionCreate`)
    if (want.parameters != undefined) {
      if (want.parameters.title) {
        AppStorage.setOrCreate('title', want.parameters.title);
      }
      if (want.parameters.customDescription) {
        AppStorage.setOrCreate('customDescription', want.parameters.customDescription);
      }
      if (want.parameters.operation) {
        AppStorage.setOrCreate('operation', want.parameters.operation);
      }
      if (want.parameters.bundleName) {
        AppStorage.setOrCreate('bundleName', want.parameters.bundleName);
      }
      if (want.parameters.moduleName) {
        AppStorage.setOrCreate('moduleName', want.parameters.moduleName);
      }
      if (want.parameters.abilityName) {
        AppStorage.setOrCreate('abilityName', want.parameters.abilityName);
      }
      let callerBundleName: string = want.parameters?.['ohos.aafwk.param.callerBundleName'] as string;
      AppStorage.setOrCreate('callerBundleName', callerBundleName);
    }
    let param: Record<string, UIExtensionContentSession> = {
      'session': session
    }
    let storage: LocalStorage = new LocalStorage(param);
    session.loadContent('pages/picker/DeviceSelect', storage);
    session.setWindowBackgroundColor('#00000000');
    let extensionHostWindow = session.getUIExtensionHostWindowProxy();
    extensionHostWindow.hideNonSecureWindows(true);
    session.setWindowPrivacyMode(true);
    AppStorage.setOrCreate('deviceSelectSession', session);
  }
  
  onSessionDestroy(session: UIExtensionContentSession) {
    let extensionHostWindow = session.getUIExtensionHostWindowProxy();
    extensionHostWindow.hideNonSecureWindows(false);
    console.log(TAG, `UIExtAbility onSessionDestroy`)
  }
}