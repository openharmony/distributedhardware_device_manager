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
import deviceInfo from '@ohos.deviceInfo';
import { Configuration } from '@ohos.app.ability.Configuration';
const TAG: string = '[DeviceManagerUI:BuildConfirm]==>'

export default class BuildConfirmUIExtAbility extends UIExtensionAbility {
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        console.log(TAG, `UIExtAbility onSessionCreate`)
        if (want.parameters == undefined) {
            return;
        }
        if (want.parameters.deviceName) {
            AppStorage.setOrCreate('deviceName', want.parameters.deviceName);
        }
        if (want.parameters.processName) {
            AppStorage.setOrCreate('processName', want.parameters.processName);
        }

        let param: Record<string, UIExtensionContentSession> = {
            'session': session
        }
        let storage: LocalStorage = new LocalStorage(param);
        if (deviceInfo.deviceType === 'wearable') {
            session.loadContent('pages/ConfirmDialogWearable', storage);
        } else if (deviceInfo.deviceType === 'tv') {
            session.loadContent('pages/ConfirmDialogTv', storage);
        } else {
            session.loadContent('pages/BuildConfirmDialog', storage);
        }
        session.setWindowBackgroundColor('#00000000');
        let extensionHostWindow = session.getUIExtensionHostWindowProxy();
        extensionHostWindow.hideNonSecureWindows(true);
        session.setWindowPrivacyMode(true);
        AppStorage.setOrCreate('BuildConfirmSession', session);
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        let extensionHostWindow = session.getUIExtensionHostWindowProxy();
        extensionHostWindow.hideNonSecureWindows(false);
        console.log(TAG, `UIExtAbility onSessionDestroy`);
    }

    onConfigurationUpdate(newConfig: Configuration): void {
        AppStorage.setOrCreate('fontSizeScale', newConfig.fontSizeScale);
        console.info('test_log', `onConfigurationUpdate fontSizeScale: ${newConfig.fontSizeScale}`);
    }
}
