/*
 * Copyright (c) 2024 - 2026 Huawei Device Co., Ltd.
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
const TAG: string = '[DeviceManagerUI:Confirm]==>'

export default class ConfirmUIExtAbility extends UIExtensionAbility {
    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        console.log(TAG, `UIExtAbility onSessionCreate`)
        if (want.parameters == undefined) {
            return;
        }
        if (want.parameters.deviceName) {
            AppStorage.setOrCreate('deviceName', want.parameters.deviceName);
        }
        if (want.parameters.appOperationStr) {
            AppStorage.setOrCreate('appOperationStr', want.parameters.appOperationStr);
        }
        if (want.parameters.customDescriptionStr) {
            AppStorage.setOrCreate('customDescriptionStr', want.parameters.customDescriptionStr);
        }
        if (want.parameters.deviceType) {
            AppStorage.setOrCreate('deviceType', want.parameters.deviceType);
        }
        if (want.parameters.hostPkgLabel) {
            AppStorage.setOrCreate('hostPkgLabel', want.parameters.hostPkgLabel);
        }
        if (want.parameters.isProxyBind) {
            AppStorage.setOrCreate('isProxyBind', want.parameters.isProxyBind);
        }
        if (want.parameters.appUserData) {
            AppStorage.setOrCreate('appUserData', want.parameters.appUserData);
        }
        if (want.parameters.isServiceBind) {
            AppStorage.setOrCreate('isServiceBind', want.parameters.isServiceBind);
        }
        if (want.parameters.title) {
            AppStorage.setOrCreate('title', want.parameters.title);
        }
        if(want.parameters.serviceUserData){
            AppStorage.setOrCreate('serviceUserData', want.parameters.serviceUserData);
        }
        if (want.parameters.metaType != undefined) {
            AppStorage.setOrCreate('metaType', want.parameters.metaType);
        }
        
        let param: Record<string, UIExtensionContentSession> = {
            'session': session
        }
        let storage: LocalStorage = new LocalStorage(param);
        if(AppStorage.get('isServiceBind') != null ? AppStorage.get('isServiceBind') as boolean:false) {
            session.loadContent('pages/ConfirmDialogServe', storage);
        } else if (deviceInfo.deviceType === 'wearable') {
            session.loadContent('pages/ConfirmDialogWearable', storage);
        } else if (deviceInfo.deviceType === 'tv') {
            session.loadContent('pages/ConfirmDialogTv', storage);
        } else {
            session.loadContent('pages/ConfirmDialog', storage);
        }
        session.setWindowBackgroundColor('#00000000');
        let extensionHostWindow = session.getUIExtensionHostWindowProxy();
        extensionHostWindow.hideNonSecureWindows(true);
        session.setWindowPrivacyMode(false);
        AppStorage.setOrCreate('ConfirmSession', session);
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        let extensionHostWindow = session.getUIExtensionHostWindowProxy();
        extensionHostWindow.hideNonSecureWindows(false);
        console.log(TAG, `UIExtAbility onSessionDestroy`)
    }

    onConfigurationUpdate(newConfig: Configuration): void {
        AppStorage.setOrCreate('fontSizeScale', newConfig.fontSizeScale);
        console.info('test_log', `onConfigurationUpdate fontSizeScale: ${newConfig.fontSizeScale}`);
    }
}
