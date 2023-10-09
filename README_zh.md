# **DeviceManager部件**

## 简介

DeviceManager组件在OpenHarmony上提供账号无关的分布式设备的认证组网能力，并为开发者提供了一套用于分布式设备间监听、发现和认证的接口。

其组成及依赖如下所示：

![](figures/devicemanager_zh.png)

dsoftbus提供能力：
    提供设备上下线通知及设备信息，设备认证通道和设备发现能力。
deviceauth提供能力：
    提供设备群组管理和群组认证能力。


## 目录

```
foundation/distributedhardware/distributedhardware_device_manager
├── common                        # 公共能力头文件存放目录
│   ├── include
│   │   ├── dfx                   # dfx功能头文件存放目录
│   │   └── ipc                   # ipc功能头文件存放目录
│   │       ├── lite              # small
│   │       ├── model             # ipc功能模块头文件存放目录
│   │       └── standard          # standard
│   └── src
│       ├── dfx                   # dfx功能实现相关代码
│       └── ipc                   # ipc公共功能代码
│           ├── lite              # small
│           └── standard          # standard
├── display                       # DM显示hap代码
│   └── entry
│       └── src
│           └── main
│               ├── js            # DM PIN码显示ServiceExtensionAbility相关JS代码
│               └── resources     # DM PIN码显示ServiceExtensionAbility相关资源配置文件目录
├── figures
├── interfaces
│   ├── inner_kits                # 内部接口及实现存放目录
│   │   └── native_cpp            # 内部native接口及实现存放目录
│   │       ├── include
│   │       │   ├── ipc           # ipc头文件存放目录
│   │       │   │   ├── lite      # small
│   │       │   │   └── standard  # standard
│   │       │   └── notify        # ipc回调通知头文件目录
│   │       └── src
│   │           ├── ipc           # ipc功能代码
│   │           │   ├── lite      # small
│   │           │   └── standard  # standard
│   │           └── notify        # ipc回调通知功能代码
│   └── kits                      # 外部接口及实现存放目录
│       ├── js                    # 外部JS接口及实现存放目录
│       │   ├── include           # 外部JS接口及实现头文件存放目录
│       │   └── src               # 外部JS接口及实现代码
|       └── js4.0                 # 从Openharmony 4.0.9.2版本开始新增外部JS接口及实现存放目录
|           ├── include           # 新增外部JS接口及实现头文件存放目录
|           └── src               # 新增外部JS接口及实现代码
|
├── sa_profile                    # SA进程配置相关文件存放目录
├── services
│   ├── implementation            # devicemanagerservice服务实现核心代码
│   │   ├── include
│   │   │   ├── ability           # 与PIN码显示ServiceExtensionAbility拉起管理相关头文件
│   │   │   ├── adapter           # DM适配扩展功能相关头文件
│   │   │   ├── authentication    # device_auth交互相关头文件
│   │   │   ├── config            # DM适配扩展相关配置功能头文件
│   │   │   ├── credential        # 凭据管理功能相关头文件
│   │   │   ├── dependency        # 外部依赖模块相关头文件
│   │   │   │   ├── commonevent   # 事件处理功能相关头文件
│   │   │   │   ├── hichain       # 与hichain功能交互相关头文件
│   │   │   │   ├── mini          # mini
│   │   │   │   ├── multipleuser  # 多用户功能相关头文件
│   │   │   │   ├── softbus       # 与软总线功能交互相关头文件
│   │   │   │   └── timer         # 定时器处理相关头文件
│   │   │   ├── devicestate       # 设备状态管理功能相关头文件
│   │   │   ├── discovery         # 设备发现功能相关头文件
│   │   │   ├── dispath           # L0上功能实现相关头文件
│   │   │   └── publish           # 设备主动发现功能相关头文件
│   │   └── src
│   │       ├── ability           # 与PIN码显示ServiceExtensionAbility拉起管理相关功能代码
│   │       ├── adapter           # DM适配扩展功能代码
│   │       ├── authentication    # device_auth交互相关核心代码
│   │       ├── config            # DM适配扩展相关配置功能代码
│   │       ├── credential        # 凭据管理功能代码
│   │       ├── dependency        # 外部依赖模块功能代码
│   │       │   ├── commonevent   # 事件处理功能代码
│   │       │   ├── hichain       # 与hichain功能交互代码
│   │       │   ├── mini          # mini
│   │       │   ├── multipleuser  # 多用户功能代码
│   │       │   ├── softbus       # 与软总线功能交互代码
│   │       │   └── timer         # 定时器处理相关功能代码
│   │       ├── devicestate       # 设备状态管理功能代码
│   │       ├── discovery         # 设备发现功能代码
│   │       ├── dispath           # mini上功能实现代码
│   │       └── publish           # 设备主动发现功能
│   └── service                   # devicemanagerservice服务实现核心代码
│       ├── include
│       │   ├── ipc               # 进程间通信相关头文件
│       │   │   ├── lite          # small
│       │   │   └── standard      # standard
│       │   └── softbus           # 软总线相关头文件
│       └── src
│           ├── ipc               # 进程间通信相关功能代码
│           │   ├── lite          # small
│           │   └── standard      # standard
│           └── softbus           # 通道建立功能核心代码
└── utils                         # 公共能力头文件存放目录
    ├── include
    │   ├── fwkload               # 拉起其他sa功能相关头文件
    │   └── permission            # 权限校验相关头文件存放目录
    └── src
        ├── fwkload               # 拉起其他sa功能相关功能代码
        └── permission            # 权限校验相关功能代码
```

## 约束

- 开发语言：JS、C++
- 适用于Hi3516DV300单板等OpenHarmony设备


## 废弃接口说明

该部分接口暂时可继续使用，但是暂停维护，建议使用新接口进行开发。

当前版本设备管理服务不具备权限管理的能力，接口中的system api仅供系统调用，后续版本会进行严格的权限管控。
接口参见[**interface_sdk-js仓库的**](https://gitee.com/openharmony/interface_sdk-js/) *ohos.distributedHardware.deviceManager.d.ts*

- 公共接口：

  使用DeviceManager相关接口之前，需要通过createDeviceManager接口创建DeviceManager实例；

  不使用DeviceManager接口的时候需要释放对应的DeviceManager实例。

| 原型                                                         | 描述                            |
| ------------------------------------------------------------ | ------------------------------- |
| createDeviceManager(bundleName: string, callback: AsyncCallback&lt;DeviceManager&gt;): void; | 以异步方法获取DeviceManager实例 |
| release(): void;                                             | 释放DeviceManager实例           |

- 系统能力接口：

  提供可信设备列表获取、可信设备状态监听、周边设备发现、设备认证等相关接口，该部分作为系统能力接口，仅供系统应用调用。

  开始设备发现、停止发现设备接口要配对使用，使用同一个subscribeId。

| 原型                                                         | 描述                 |
| ------------------------------------------------------------ | -------------------- |
| getTrustedDeviceListSync(): Array&lt;DeviceInfo&gt;;                                                                                            | 获取信任设备列表 |
| getTrustedDeviceList(callback:AsyncCallback&lt;Array&lt;DeviceInfo&gt;&gt;): void;                                                                    | 获取信任设备列表 |
| getTrustedDeviceList(): Promise&lt;Array&lt;DeviceInfo&gt;&gt;;                                                                                       | 获取信任设备列表 |
| getLocalDeviceInfoSync(): DeviceInfo;                        | 获取本地设备信息 |
| getLocalDeviceInfo(callback:AsyncCallback&lt;DeviceInfo&gt;): void;                                                                             | 获取本地设备信息 |
| getLocalDeviceInfo(): Promise&lt;DeviceInfo&gt;;                   | 获取本地设备信息 |
| on(type: 'deviceStateChange', callback: Callback&lt;{ action: DeviceStateChangeAction, device: DeviceInfo }&gt;): void;                         | 设备状态变更回调 |
| off(type: 'deviceStateChange', callback?: Callback&lt;{ action: DeviceStateChangeAction, device: DeviceInfo }&gt;): void;                       | 取消设备状态变更回调 |
| on(type: 'serviceDie', callback: () => void): void;                                                                                       | 服务错误回调 |
| off(type: 'serviceDie', callback?: () => void): void;                                                                                     | 取消服务错误回调 |
| startDeviceDiscovery(subscribeInfo: SubscribeInfo): void;    | 开始设备发现         |
| stopDeviceDiscovery(subscribeId: number): void;              | 停止发现设备         |
| authenticateDevice(deviceInfo: DeviceInfo, authParam: AuthParam, callback: AsyncCallback&lt;{deviceId: string, pinToken ?: number}&gt;): void; | 设备认证接口         |
| unAuthenticateDevice(deviceInfo: DeviceInfo): void;          | 解除认证设备         |
| setUserOperation(operateAction: number, params: string): void;    | 设置用户ui操作行为         |
| verifyAuthInfo(authInfo: AuthInfo, callback: AsyncCallback&lt;{deviceId: string, level: number}&gt;): void; | 设备认证信息校验     |
| startDeviceDiscovery(subscribeInfo: SubscribeInfo, filterOptions?: string): void;                                                         | 发现周边设备     |
| publishDeviceDiscovery(publishInfo: PublishInfo): void;                                                                                   | 发布设备发现     |
| unPublishDeviceDiscovery(publishId: number): void;                                                                                        | 停止发布设备发现 |
| on(type: 'deviceFound', callback: Callback&lt;{ subscribeId: number, device: DeviceInfo }&gt;): void; | 发现设备列表回调     |
| off(type: 'deviceFound', callback?: Callback&lt;{ subscribeId: number, device: DeviceInfo }&gt;): void; | 取消发现设备列表回调 |
| on(type: 'discoverFail', callback: Callback&lt;{ subscribeId: number, reason: number }&gt;): void; | 发现设备失败回调     |
| off(type: 'discoverFail', callback?: Callback&lt;{ subscribeId: number, reason: number }&gt;): void; | 取消发现设备失败回调 |
| on(type: 'publishSuccess', callback: Callback&lt;{ publishId: number }&gt;): void; | 发布设备成功回调     |
| off(type: 'publishSuccess', callback?: Callback&lt;{ publishId: number }&gt;): void; | 取消发布设备成功回调 |
| on(type: 'publishFail', callback: Callback&lt;{ publishId: number, reason: number }&gt;): void; | 发布设备失败回调     |
| off(type: 'publishFail', callback?: Callback&lt;{ publishId: number, reason: number  }&gt;): void; | 取消发布设备失败回调 |
| on(type: 'uiStateChange', callback: Callback&lt;{ param: string}&gt;): void; | ui状态变更回调     |
| off(type: 'uiStateChange', callback?: Callback&lt;{ param: string}&gt;): void; | 取消ui状态变更回调     |

## 新增接口说明

  从Openharmony 4.0.9.2版本开始，新增设备管理接口。

接口参见[**interface_sdk-js仓库的**](https://gitee.com/openharmony/interface_sdk-js/) *ohos.distributedDeviceManager.d.ts*

- 公共接口：

  调用以下接口，需要申请ohos.permission.DISTRIBUTED_DATASYNC权限才能正常调用。

  使用DeviceManager相关接口之前，需要通过createDeviceManager接口创建DeviceManager实例；

  不使用DeviceManager接口的时候需要释放对应的DeviceManager实例。

  提供可信设备列表获取、可信设备状态监听、周边设备发现、设备认证等相关接口，支持三方应用调用。

| 原型                                                         | 描述                            |
| ------------------------------------------------------------ | ------------------------------- |
| createDeviceManager(bundleName: string): DeviceManager;                        | 创建一个设备管理实例。设备管理实例是分布式设备管理方法的调用入口。用于获取可信设备和本地设备的相关信息。 |
| releaseDeviceManager(deviceManager: DeviceManager): void;                      | 设备管理实例不再使用后，通过该方法释放DeviceManager实例。           |
| getAvailableDeviceListSync(): Array&lt;DeviceBasicInfo&gt;;                                                | 同步获取所有可信设备列表。 |
| getAvailableDeviceList(callback:AsyncCallback&lt;Array&lt;DeviceBasicInfo&gt;&gt;): void;                  | 获取所有可信设备列表。使用callback异步回调。 |
| getAvailableDeviceList(): Promise&lt;Array&lt;DeviceBasicInfo&gt;&gt;;                                     | 获取所有可信设备列表。使用Promise异步回调。 |
| getLocalDeviceNetworkId(): string;                                                                         | 获取本地设备网络标识。 |
| getLocalDeviceName(): string;                                                                              | 获取本地设备名称。     |
| getLocalDeviceType(): number;                                                                              | 获取本地设备类型。     |
| getLocalDeviceId(): string;                                                                                | 获取本地设备id。       |
| getDeviceName(networkId: string): string;                                                                  | 通过指定设备的网络标识获取该设备名称。    |
| getDeviceType(networkId: string): number;                                                                  | 通过指定设备的网络标识获取该设备类型。    |
| startDiscovering(discoverParam: {[key:&nbsp;string]:&nbsp;Object} , filterOptions?: {[key:&nbsp;string]:&nbsp;Object} ): void;  | 发现周边设备。发现状态持续两分钟，超过两分钟，会停止发现，最大发现数量99个。    |
| stopDiscovering(): void;                                                                                   | 停止发现周边设备。         |
| bindTarget(deviceId: string, bindParam: {[key:&nbsp;string]:&nbsp;Object} , callback: AsyncCallback&lt;{deviceId: string}>): void;          | 认证设备。                                             |
| unbindTarget(deviceId: string): void;                                                                      | 解除认证设备。         |
| on(type: 'deviceStateChange', callback: Callback&lt;{ action: DeviceStateChange, device: DeviceBasicInfo }&gt;): void;           | 注册设备状态回调。               |
| off(type: 'deviceStateChange', callback?: Callback&lt;{ action: DeviceStateChange, device: DeviceBasicInfo }&gt;): void;         | 取消注册设备状态回调。           |
| on(type: 'discoverSuccess', callback: Callback&lt;{ device: DeviceBasicInfo }&gt;): void;                  | 注册发现设备成功回调监听。        |
| off(type: 'discoverSuccess', callback?: Callback&lt;{ device: DeviceBasicInfo }&gt;): void;                | 取消注册设备发现成功回调。            |
| on(type: 'deviceNameChange', callback: Callback&lt;{ deviceName: string }&gt;): void;                      | 注册设备名称变更回调监听。        |
| off(type: 'deviceNameChange', callback?: Callback&lt;{ deviceName: string }&gt;): void;                    | 取消注册设备名称变更回调监听。    |
| on(type: 'discoverFailure', callback: Callback&lt;{ reason: number }&gt;): void;                           | 注册设备发现失败回调监听。        |
| off(type: 'discoverFailure', callback?: Callback&lt;{ reason: number }&gt;): void;                         | 取消注册设备发现失败回调。        |
| on(type: 'serviceDie', callback?: Callback&lt;{}&gt;): void;                                               | 注册设备管理服务死亡监听。        |
| off(type: 'serviceDie', callback?: Callback&lt;{}&gt;): void;                                              | 取消注册设备管理服务死亡监听。    |

- 示例如下：

```js
try {
  let dmClass = deviceManager.createDeviceManager("ohos.samples.jshelloworld");
} catch(err) {
  console.error("createDeviceManager errCode:" + err.code + ",errMessage:" + err.message);
}

try {
  // 注册设备管理服务死亡监听。
  dmClass.on("serviceDie", () => {
    console.info("serviceDie on");
  });
} catch (err) {
  console.error("serviceDie errCode:" + err.code + ",errMessage:" + err.message);
}

try {
  dmClass.on('deviceStateChange', (data) => {
    console.info("deviceStateChange on:" + JSON.stringify(data));
  });
} catch (err) {
  console.error("deviceStateChange errCode:" + err.code + ",errMessage:" + err.message);
}

// 同步获取所有可信设备列表。
try {
  let trustList = dmClass.getAvailableDeviceListSync();
  console.log("trustList:" + JSON.stringify(this.trustList));
} catch(err) {
  console.log("getAvailableDeviceList failed: " + JSON.stringify(err));
}
// 获取本地设备类型。
var data = dmClass.getLocalDeviceType();

// 获取本地设备名称。
var data = dmClass.getLocalDeviceName();

// 发现周边设备。
try {
  // 注册发现设备成功回调监听。
  dmClass.on('discoverSuccess', (data) => {
  if (data == null) {
    console.log("discoverSuccess error data = null")
    return;
  }
  console.info(TAG + "discoverSuccess:" + JSON.stringify(data));
  });
  // 注册设备发现失败回调监听。
  dmClass.on('discoverFailure', (data) => {
    console.info("discoverFailure on:" + JSON.stringify(data));
  });
  var discoverParam = {
    'discoverTargetType': 1
  };
  var filterOptions = {
    'availableStatus': 1,
    'discoverDistance': 50,
    'authenticationStatus': 0,
    'authorizationType': 0
  };
  dmClass.startDiscovering(discoverParam, filterOptions); // 当有设备发现时，通过discoverSuccess回调通知给应用程序
} catch (err) {
  console.log("startDiscovering err:" + err.code + "," + err.message);
}

// 停止发现周边设备。
try {
  dmClass.stopDiscovering();
  // 取消注册设备发现成功回调。
  dmClass.off('discoverSuccess');
  // 取消注册设备发现失败回调。
  dmClass.off('discoverFailure');
} catch (err) {
  console.log("err:" + err.code + "," + err.message);
}

// 设备认证
var deviceId ="XXXXXXXX";
let bindParam = {
  'bindType': 1,// 认证类型： 1 - 无帐号PIN码认证
  'targetPkgName': 'xxxx',
  'appName': 'xxxx',
  'appOperation': 'xxxx',
  'customDescription': 'xxxx'
}
try {
  // 设备认证
  dmClass.bindTarget(deviceId, bindParam, (err, data) => {
  if (err) {
    console.info(TAG + "bindTarget err:" + JSON.stringify(err));
    return;
  } else {
    console.info(TAG + "bindTarget result:" + JSON.stringify(data));
    let token = data.pinTone;
  }
  });
} catch (err) {
  console.log("bindTarget err:" + err.code + "," + err.message);
}
// 设备取消认证
try {
  var deviceId = "xxxxxxxx";
  let result = dmClass.unbindTarget(deviceId);
  console.log("unbindTarget last device: " + JSON.stringify(deviceId) + " and result = "
    + result);
  } catch (err) {
    console.log("unbindTarget err:" + err.code + "," + err.message);
}

try {
  // 取消注册设备管理服务死亡监听。
  dmClass.off('serviceDie');
  // 取消注册设备状态回调。
  dmClass.off('deviceStateChange', (data) => {
      console.info('deviceStateChange' + JSON.stringify(data));
    });
  // 释放DeviceManager实例。
  deviceManager.releaseDeviceManager(dmClass);
} catch (err) {
  console.log("err:" + err.code + "," + err.message);
}
```

- 系统接口：

  该部分接口仅支持系统应用调用。
  调用以下接口，需要申请ohos.permission.ACCESSS_SERVICE_DM权限才能正常调用。

| 原型                                                         | 描述                 |
| ------------------------------------------------------------ | -------------------- |
| replyUiAction(action: number, actionResult: string): void;                                                | 回复用户ui操作行为。此接口只能被devicemanager的PIN码hap使用。      |
| on(type: 'replyResult', callback: Callback&lt;{ param: string}&gt;): void;                                | 回复UI操作结果回调。          |
| off(type: 'replyResult', callback?: Callback&lt;{ param: string}&gt;): void;                              | 取消回复UI操作结果回调。       |

- 示例如下：

```js
// 设置用户ui操作行为
/*  action = 0 - 允许授权
    action = 1 - 取消授权
    action = 2 - 授权框用户操作超时
    action = 3 - 取消pin码框展示
    action = 4 - 取消pin码输入框展示
    action = 5 - pin码输入框确定操作
*/
try {
  let action = 0;
  dmClass.replyUiAction(action, "extra")
  dmClass.on('replyResult', (data) => {
    console.log("replyResult executed, dialog closed" + JSON.stringify(data))
    var tmpStr = JSON.parse(data.param)
    var isShow = tmpStr.verifyFailed
    console.log("replyResult executed, dialog closed" + isShow)
  });
} catch (err) {
  console.log("err:" + err.code + "," + err.message);
}

```

详细接口说明请参考[**API文档**](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis/js-apis-distributedDeviceManager.md)

## 系统弹框ServiceExtensionAbility

当前版本只支持PIN码认证，需要提供PIN码认证的授权提示界面、PIN码显示界面、PIN码输入界面；

当前，由于系统通过native层直接进行弹窗的能力尚不具备，这里使用ServiceExtensionAbility来进行对应界面的弹窗。

该ServiceExtensionAbility为：DeviceManager_UI.hap，作为系统应用进行预置。

- 编译运行：

  将devicemanager/display工程导入DevEco Studio 2.2 Beta1，复制display目录下的@ohos.distributedHardware.deviceManager.d.ts文件到Sdk\js\2.2.0.1\api\common目录下，进行编译构建及运行调试.

- 编译环境依赖：IDE 2.2 SDK6

- DeviceManager_UI.hap包源码存放位置：[device_manager仓库](https://gitee.com/openharmony/distributedhardware_device_manager/tree/master/display)

- UI显示：

  DeviceManager作为认证被控端，授权提示界面、PIN码显示界面由DeviceManager_UI ServiceExtensionAbility默认进行显示；

  DeviceManager作为认证发起端，PIN码输入界面可以选择由DeviceManager_UI ServiceExtensionAbility进行显示，还是由开发者自行显示。开发者如需自己定制PIN码输入界面，需要在authenticateDevice接口的认证参数AuthParam中，extraInfo属性里面指定displayOwner参数为1。

## 相关仓
****

[**interface_sdk-js**](https://gitee.com/openharmony/interface_sdk-js/)

[**applications_hap**](https://gitee.com/openharmony/applications_hap)
