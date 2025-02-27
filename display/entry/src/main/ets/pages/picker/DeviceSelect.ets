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
import distributedDeviceManager from '@ohos.distributedDeviceManager';
import { AsyncCallback, BusinessError, Callback, ErrorCallback } from '@ohos.base';
import abilityAccessCtrl, { Context } from '@ohos.abilityAccessCtrl';
import promptAction from '@ohos.promptAction';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import mediaquery from '@ohos.mediaquery';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../../common/constant';
import accessibility from '@ohos.accessibility';
import common from '@ohos.app.ability.common';
let anmDuration: number = 800;
let TAG = '[DeviceManagerUI:DeviceSelect]==>';

enum StatusType {
  Unbound = 0,
  TrustBinding = 1,
  Binding = 2
}

let customData: Record<string, string | number> = {
  'operation': '',
  'object': '',
  'qResp': 1
};

let discoverParam: Record<string, Object> = {
  'discoverTargetType': 1,
  'CUSTOM_DATA': JSON.stringify(customData),
  'DISC_CAPABILITY': ''
};
let filterOptions: Record<string, number> = {
  'availableStatus': 0
};

class Data {
  public status?: StatusType = StatusType.Unbound
  public device: distributedDeviceManager.DeviceBasicInfo = {
    deviceId: '',
    deviceName: '',
    deviceType: '',
    networkId: '',
    extraData: '',
  };
}

class DeviceStateData {
  public action: distributedDeviceManager.DeviceStateChange = 0;
  public device: distributedDeviceManager.DeviceBasicInfo = {
    deviceId: '',
    deviceName: '',
    deviceType: '',
    networkId: '',
    extraData: '',
  };
}

class Device {
  public device: distributedDeviceManager.DeviceBasicInfo = {
    deviceId: '',
    deviceName: '',
    deviceType: '',
    networkId: '',
    extraData: '',
  }
}

class CustomData {
  public operation: string = '';
  public object: string = '';
  public displayName: string = '';
  public policyCode: number = 0;
  public actionId: number = 0;
}

class ExtraData {
  public CUSTOM_DATA: string = ''
}

class DeviceCallBack {
  public deviceId: string = '';
  public deviceName: string = '';
  public bundlename: string = '';
  public moudlename: string = '';
  public ablityname: string = '';
}

class BindTargetData {
  public deviceId: string = '';
}

@Entry
@Component
export struct Index {
  @State showPhone: Visibility = Visibility.Visible;
  @State showFlag: Visibility = Visibility.Visible;
  @State deviceSelectSession : UIExtensionContentSession | undefined = undefined;
  @State isAutoCancel: boolean = false;
  @State deviceList: Data[] = [];
  @State trustDeviceList: Data[] = [];
  @State centerX: number = 0;
  @State centerY: number = 0;
  @State radius: number = 0;
  @State deviceStateStatus: number = -1;
  @State currentDevice: Data | undefined = undefined;
  @State title: string | Resource = '';
  @State operation: string = '';
  @State customDescription: string | Resource = '';
  @State bundleName: string = '';
  @State moduleName: string = '';
  @State abilityName: string = '';
  @State isLandscape: boolean = true;
  @State isListNum: number = 5;
  @State discCapability: string = 'oop';
  @State networIdMatchLen: number = -1;
  listener = mediaquery.matchMediaSync('(orientation: landscape)');
  dmInstance: distributedDeviceManager.DeviceManager | undefined = undefined;
  context = getContext(this) as common.UIAbilityContext
  bindParam: Record<string, string | number | Context | Resource> = {};
  scroller: Scroller = new Scroller()

  @Builder
  CustomBuilder() {
    Column() {
      Stack() {
        Image($r('app.media.iocn_phone_self')).width(54).draggable(false)
        Image($r('app.media.phone')).width(30).draggable(false)
      }
      .width(54)
      .height(54)
      .borderRadius(54)
    }
    .width(68)
    .height(92)
  }

  cancel() {
    this.dmInstance!.stopDiscovering();
    try {
      console.log(TAG + 'terminateSelf');
      this.deviceSelectSession = AppStorage.get<UIExtensionContentSession>('deviceSelectSession');
      if (this.deviceSelectSession) {
        this.deviceSelectSession.terminateSelf();
      }
    } catch (err) {
      console.log(TAG + 'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  deviceStateChange() {
    try {
      this.dmInstance!.on('deviceStateChange', (data: DeviceStateData) => {
        if (data.action == 0 || data.action == 1) {
          let deviceInfoList: distributedDeviceManager.DeviceBasicInfo[] =
            this.dmInstance!.getAvailableDeviceListSync();
          this.trustDeviceList = [];
          deviceInfoList.forEach((item) => {
            this.trustDeviceList.push({ device: item, status: StatusType.TrustBinding})
            for (let i = 0; i < this.deviceList.length; i++) {
              let deviceItem = this.deviceList[i]
              if (this.isDeviceBasicEqual(deviceItem, item)) {
                this.deviceList[i].status = StatusType.TrustBinding
                this.deviceList.push(new Data())
                this.deviceList.splice(this.deviceList.length - 1, 1)
              }
            }
          })
          let currentItem = this.currentDevice as Data
          if (currentItem && this.isDeviceEqual(currentItem, data)) {
            this.sendData(data.device.deviceId, data.device.networkId, data.device.deviceName)
          }
        }
        if (data.action == 2) {
          this.deviceList = this.deviceList.filter((item) => !this.isDeviceEqual(item, data))
        }
      });
    } catch (err) {
      let e: BusinessError = err as BusinessError;
      console.error('deviceStateChange errCode:' + e.code + ',errMessage:' + e.message);
    }
  }

  addProperty(data: Data) {
    let extraDataStr: string = data.device.extraData == undefined ? '' : data.device.extraData
    let extraData: ExtraData = JSON.parse(extraDataStr)
    let customData: CustomData = JSON.parse(extraData.CUSTOM_DATA)
    data.device.deviceName = customData.displayName
  }

  devListInfo() {
    try {
      this.deviceStateChange()
      let deviceInfoList: distributedDeviceManager.DeviceBasicInfo[] =
      this.dmInstance!.getAvailableDeviceListSync();
      deviceInfoList.forEach((item) => {
        this.trustDeviceList.push({ device: item, status: StatusType.TrustBinding })
      })

      customData = {
        'operation': this.operation,
        'object': this.bundleName,
        'qResp': 1
      }
      discoverParam = {
        'discoverTargetType': 1,
        'CUSTOM_DATA': JSON.stringify(customData),
        'DISC_CAPABILITY' : this.discCapability
      }
      this.dmInstance!.startDiscovering(discoverParam, filterOptions);
    } catch (err) {
      let e: BusinessError = err as BusinessError;
      console.error('startDiscovering errCode:' + e.code + ',errMessage:' + e.message);
    }
    this.dmInstance!.on('discoverSuccess', (data: Data) => {
      let arr: string[] = []
      let result = this.deviceList.find((item: Data) => {
        return this.isDeviceDataEqual(item, data)
      })
      if (!!result) {
        return
      }
      this.deviceList.forEach((item) => {
        arr.push(this.getMatchId(item.device.deviceId, item.device.networkId))
      })
      if (arr.indexOf(this.getMatchId(data.device.deviceId, data.device.networkId)) === -1) {
        data.status = this.getStatusType(data)
        this.deviceList.push(data)
      }
    });
  }

  getFullNetworkId(networkId: string | undefined): string {
    let networkIdStr = networkId == undefined ? '' : networkId
    let fullNetworkId = '';
    this.trustDeviceList.forEach((item) => {
      if (this.getMatchId('', item.device.networkId) == this.getMatchId('', networkIdStr)) {
        fullNetworkId = item.device.networkId;
      }
    });
    return fullNetworkId;
  }

  getStatusType(data: Data): StatusType {
    let isTrust = false;
    this.trustDeviceList.forEach((item) => {
      if (this.isDeviceDataEqual(item, data)) {
        isTrust = true
      }
    });
    if (isTrust) {
      return StatusType.TrustBinding
    }
    return StatusType.Unbound
  }

  getMatchId(deviceId: string, networkId: string | undefined): string {
    networkId = networkId == undefined ? '' : networkId;
    if (this.discCapability == 'oop') {
      this.networIdMatchLen = 8;
      if (networkId.length > this.networIdMatchLen) {
        return networkId.substring(0, this.networIdMatchLen)
      }
      return networkId
    }
    return deviceId;
  }

  isDeviceEqual(device1: Data, device2: DeviceStateData): boolean {
    return this.getMatchId(device1.device.deviceId, device1.device.networkId) ==
      this.getMatchId(device2.device.deviceId, device2.device.networkId);
  }

  isDeviceBasicEqual(device1: Data, device2: distributedDeviceManager.DeviceBasicInfo): boolean {
    return this.getMatchId(device1.device.deviceId, device1.device.networkId) ==
      this.getMatchId(device2.deviceId, device2.networkId);
  }

  isDeviceDataEqual(device1: Data, device2: Data): boolean {
    return this.getMatchId(device1.device.deviceId, device1.device.networkId) ==
      this.getMatchId(device2.device.deviceId, device2.device.networkId);
  }

  initPickParms() {
    if (AppStorage.get('title') != null) {
      this.title = AppStorage.get('title') as string;
    }
    if (AppStorage.get('customDescription') != null) {
      this.customDescription = AppStorage.get('customDescription') as string;
    }
    if (AppStorage.get('operation') != null) {
      this.operation = AppStorage.get('operation') as string;
    }
    if (AppStorage.get('bundleName') != null) {
      this.bundleName = AppStorage.get('callerBundleName') as string;
    }
    if (AppStorage.get('moduleName') != null) {
      this.moduleName = AppStorage.get('moduleName') as string;
    }
    if (AppStorage.get('abilityName') != null) {
      this.abilityName = AppStorage.get('abilityName') as string;
    }
    let bundleName = `com.ohos.devicemanagerui_pickerProxy_${this.bundleName}`
    this.dmInstance = distributedDeviceManager.createDeviceManager(bundleName);
  }

  calcPositionX(index: number): number {
    let angle = -160 + (index * 160 / this.isListNum)
    let radian = angle * Math.PI / 180
    return this.centerX + Math.cos(radian) * this.radius
  }

  calcPositionY(index: number): number {
    let angle = -160 + (index * 160 / this.isListNum)
    let radian = angle * Math.PI / 180
    return this.centerY + Math.sin(radian) * this.radius
  }

  sendData(deviceId: string, networkId: string | undefined, deviceName: string) {
    this.deviceSelectSession = AppStorage.get<UIExtensionContentSession>('deviceSelectSession');
    let callBack: DeviceCallBack = {
      deviceId: deviceId,
      deviceName: deviceName,
      bundlename: this.bundleName,
      moudlename: this.moduleName,
      ablityname: this.abilityName,
    }
    let fullNetworkId = this.getFullNetworkId(networkId)
    console.info('sendData:' + JSON.stringify(callBack));
    this.deviceSelectSession.sendData({'deviceId' : deviceId, 'networkId' : fullNetworkId, 'deviceName' : deviceName})
    this.cancel()
  }

  bindTarget(item: Data, index: number) {
    this.currentDevice = item
    if (item.status == 0) {
      try {
        this.bindParam = {
          'bindType': 1,
          'targetPkgName': 'xxxx',
          'appName': 'xxxx',
          'appOperation': 'xxxx',
          'customDescription': this.customDescription,
          'bundleName': this.bundleName,
          'context': this.context
        };
        this.dmInstance!.bindTarget(item.device.deviceId, this.bindParam,
          (err: BusinessError, data: BindTargetData) => {
            if (err) {
              console.error('======bindTarget errCode:' + err.code + ',errMessage:' + err.message);
              return;
            }
          });
      } catch (err) {
        let e: BusinessError = err as BusinessError;
        console.error('bindTarget errCode:' + e.code + ',errMessage:' + e.message);
      }
    } else {
      try {
        this.dmInstance!.unbindTarget(item.device.deviceId);
        this.deviceList[index].status = StatusType.Unbound
        this.deviceList.push(new Data())
        this.deviceList.splice(this.deviceList.length - 1, 1)
      } catch (err) {
        let e: BusinessError = err as BusinessError;
        console.error('unbindTarget errCode:' + e.code + ',errMessage:' + e.message);
      }
    }
  }

  onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
    console.info('mediaQueryResult.matches:' + mediaQueryResult.matches)
    if (mediaQueryResult.matches) {
      this.isLandscape = true
      this.isListNum = 8
    } else {
      this.isLandscape = false
      this.isListNum = 5
    }
  }

  aboutToAppear(): void {
    this.listener.on('change', this.onPortrait.bind(this))
    this.initPickParms()
    this.deviceSelectSession = AppStorage.get<UIExtensionContentSession>('deviceSelectSession');
    this.devListInfo();
  }

  aboutToDisappear(): void {
    this.listener.off('change', this.onPortrait)
  }

  build() {
    Column() {
      Row() {
        Text(this.title)
          .fontSize(24)
          .fontWeight(500)
          .onClick(() => {
            let arr: string[] = []
            this.deviceList.forEach((item) => {
              arr.push(item.device.deviceId)
              arr = Array.from(new Set(arr))
            })
          })
        Image($r('app.media.close')).width(40).height(40)
          .onClick(() => {
            this.cancel()
          })
      }.justifyContent(FlexAlign.SpaceBetween)
      .width('100%')

      if (this.deviceList.length <= this.isListNum) {
        ForEach(this.deviceList, (item: Data, index: number) => {
          Column() {
            Stack() {
              Image(item.status == StatusType.TrustBinding ? $r('app.media.icon_ball_online') :
              $r('app.media.icon_ball')).width(54)
              Image(item.device.deviceType == 'PHONE' ? $r('app.media.phone') : $r('app.media.icon_pad')).width(30)
            }
            .width(54)
            .height(54)
            .borderRadius(54)

            Text(item.device.deviceName)
            .fontSize(12)
            .textAlign(TextAlign.Center)
          }
          .width(68)
          .height(92)
          .position({
            x: this.calcPositionX(index) - 34,
            y: this.calcPositionY(index) - 46
          })
          .onClick(() => {
            if (item.status == StatusType.TrustBinding) {
              this.sendData(item.device.deviceId, item.device.networkId, item.device.deviceName)
            }
          })
          .onDrop((event: DragEvent, extraParams?: string | undefined) => {
            this.showPhone = Visibility.Visible
            this.bindTarget(item, index)
            console.info('======onDrop onDrop:' + JSON.stringify(event), JSON.stringify(extraParams));
          })
        })
        Column() {
          Stack() {
            Image($r('app.media.iocn_phone_self')).width(54).draggable(false)
            Image($r('app.media.phone')).width(30).draggable(false)
          }
          .width(54)
          .height(54)
          .borderRadius(54)

          Text($r('app.string.local_machine'))
            .fontSize(12)
            .textAlign(TextAlign.Center)
        }
        .width(68)
        .height(92)
        .position({
          x: this.centerX - 34,
          y: this.centerY - 46
        })
        .visibility(this.showPhone)
        .draggable(true)
        .onDragStart((event) => {
          this.showPhone = Visibility.Hidden
          console.info('======onDrop onDragStart:' + JSON.stringify(event));
          return this.CustomBuilder()
        })
        .onDragEnter((event: DragEvent, extraParams?: string | undefined) => {
          this.showPhone = Visibility.Hidden
          console.info('======onDrop onDragEnter:' + JSON.stringify(event), JSON.stringify(extraParams));
        })
        .onDragMove((event: DragEvent, extraParams?: string | undefined) => {
          console.info('======onDrop onDragMove:' + JSON.stringify(event), JSON.stringify(extraParams));
        })
        .onDrop((event: DragEvent, extraParams?: string | undefined) => {
          console.info('======onDrop onDrop:' + JSON.stringify(event), JSON.stringify(extraParams));
        })
        .onDragEnd((event: DragEvent, extraParams?: string | undefined) => {
          console.info('======onDrop onDragEnd:' + JSON.stringify(event), JSON.stringify(extraParams));
        })
      } else {
        Grid(this.scroller) {
          ForEach(this.deviceList, (item: Data, index: number) => {
            GridItem() {
              Column() {
                Stack() {
                  Image(item.status == StatusType.TrustBinding ? $r('app.media.icon_ball_online') :
                  $r('app.media.icon_ball')).width(54).draggable(false)
                  Image(item.device.deviceType == 'PHONE' ? $r('app.media.phone') : $r('app.media.icon_pad'))
                  .width(30)
                  .draggable(false)
                }
                .width(54)
                .height(54)
                .borderRadius(54)

                Text(item.device.deviceName)
                .fontSize(12)
                .textAlign(TextAlign.Center)
              }
              .width(68)
              .height(92)
              .onClick(() => {
                if (item.status == StatusType.TrustBinding) {
                  this.sendData(item.device.deviceId, item.device.networkId, item.device.deviceName)
                } else {
                  this.bindTarget(item, index)
                }
              })
            }
            .draggable(true)
            .onDragStart((event) => {
              console.info('======onDrop onDragStart:' + JSON.stringify(event));
            })
            .onDragEnter((event: DragEvent, extraParams?: string | undefined) => {

              console.info('======onDrop onDragEnter:' + JSON.stringify(event), JSON.stringify(extraParams));
            })
            .onDragMove((event: DragEvent, extraParams?: string | undefined) => {
              console.info('======onDrop onDragMove:' + JSON.stringify(event), JSON.stringify(extraParams));
            })
            .onDrop((event: DragEvent, extraParams?: string | undefined) => {
              console.info('======onDrop onDrop:' + JSON.stringify(event), JSON.stringify(extraParams));
            })
            .onDragEnd((event: DragEvent, extraParams?: string | undefined) => {
              console.info('======onDrop onDragEnd:' + JSON.stringify(event), JSON.stringify(extraParams));
            })
          })
        }
        .columnsTemplate('1fr 1fr 1fr 1fr 1fr')
        .columnsGap(10)
        .rowsGap(10)
        .friction(0.6)
        .enableScrollInteraction(true)
        .supportAnimation(false)
        .multiSelectable(false)
        .edgeEffect(EdgeEffect.Spring)
        .scrollBar(BarState.On)
        .scrollBarColor(Color.Grey)
        .scrollBarWidth(4)
        .width('100%')
        .layoutWeight(1)
      }
    }
    .onDrop((event: DragEvent, extraParams?: string | undefined) => {
      this.showPhone = Visibility.Visible
      console.info('======onDrop onDrop:' + JSON.stringify(event), JSON.stringify(extraParams));
    })
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
    .borderRadius({ topLeft: 20, topRight: 20 })
    .padding({ bottom: 0, left: 10, right: 10 })
    .onClick(() => {
      if (this.isAutoCancel) {
        this.cancel();
      }
    })
    .onAreaChange((oldValue, newValue) => {
      this.centerX = Number(newValue.width) / 2
      if (this.isLandscape) {
        this.radius = Number(newValue.height) - 90
        this.centerY = Number(newValue.height) - 20
      } else {
        this.radius = Math.min(Number(newValue.width), Number(newValue.height)) * 0.4
        this.centerY = this.radius + 90
      }
    })
    .visibility(this.showFlag)
    .transition(TransitionEffect.OPACITY.animation({ duration: anmDuration })
      .combine(TransitionEffect.translate({ y: 100 })))

  }
}