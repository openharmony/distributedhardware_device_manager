# OpenHarmony DeviceManager 外部 API 参考文档

版本 v2.0
更新日期 2026-05-19

---

## 1. 概述

DeviceManager（设备管理器）是 OpenHarmony 分布式硬件子系统中的核心组件，提供设备发现、认证、绑定和状态管理等能力。DM 支持跨账户的分布式设备组网，提供一套完整的设备生命周期管理 API。

DM 提供三套外部接口：

1. **JS/NAPI 接口**：面向应用开发者的主要接口，支持异步和同步调用方式
2. **NDK C 接口**：面向 C/C++ 原生开发者的 Native 接口
3. **Cangjie FFI 接口**：面向 Cangjie 语言开发者的外部函数接口

本文档以 JS/NAPI 接口为主，详细说明每个接口的使用方法、参数、返回值和错误码。NDK 和 Cangjie 接口提供对照表，方便不同语言开发者参考。

### 1.1 核心概念

- **可信设备**：已完成认证和绑定的设备，可以进行分布式数据同步和服务调用
- **本地设备**：当前运行的设备
- **远端设备**：网络中发现的其他设备
- **设备发现**：通过蓝牙、Wi-Fi 等介质发现周边的设备
- **设备认证**：建立设备间的信任关系，包括 PIN 码、二维码等方式
- **设备绑定**：将设备添加到可信列表，建立长期信任关系

### 1.2 架构说明

DeviceManager 采用客户端-服务端架构：
- **DM Client**：运行在应用进程，提供 API 接口
- **DM Service**：系统级服务，负责设备管理核心逻辑
- **DM UI Service**：提供认证交互界面（PIN 显示/输入）

### 1.3 技术规格

- **支持设备类型**：手机、平板、手表、耳机、PC、车机、TV、相机等
- **通信介质**：BLE、Wi-Fi、BR、USB
- **认证方式**：PIN 码、扫码、声波、NFC
- **安全级别**：支持多种认证形式（点对点、同账号、跨账号）

---

## 2. 接口级别说明

DeviceManager 提供的 API 按访问级别分为两类：

### 2.1 System API（系统接口）

仅系统应用可调用，需要以下权限之一：
- `ohos.permission.ACCESS_SERVICE_DM`：访问 DeviceManager 服务的权限
- `ohos.permission.DISTRIBUTED_DEVICE_AUTH_SERVICE`：设备认证服务权限

**系统接口包括**：
- 设备发现相关接口
- 设备认证和绑定接口
- 设备状态监听接口
- 设备发布接口

### 2.2 Public API（公共接口）

所有应用可调用，无特殊权限要求。

**公共接口包括**：
- `createDeviceManager()`：创建 DM 实例
- `release()`：释放 DM 实例
- `getTrustedDeviceListSync()`：获取可信设备列表
- `getLocalDeviceInfoSync()`：获取本地设备信息
- `getDeviceInfo()`：获取指定设备信息

### 2.3 权限说明表

| 权限名称 | 权限级别 | 说明 |
|---------|---------|------|
| ohos.permission.ACCESS_SERVICE_DM | system | 访问 DM 服务核心接口 |
| ohos.permission.DISTRIBUTED_DEVICE_AUTH_SERVICE | system | 设备认证服务 |
| ohos.permission.DISCOVER_DEVICE | system | 设备发现 |
| ohos.permission.MANAGE_TRUSTED_DEVICES | system | 管理可信设备 |

---

## 3. 生命周期管理接口

### 3.1 createDeviceManager

#### 接口签名
```typescript
createDeviceManager(bundleName: string, callback: AsyncCallback<DeviceManager>): void
```

#### 接口级别
**Public API** - 所有应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
创建一个 DeviceManager 实例，这是使用所有其他 DM 接口的前提。每个应用进程应该只创建一个 DM 实例，创建后可以重复使用。

#### 使用场景
- 应用启动时初始化
- 需要使用设备管理功能前的准备步骤

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| bundleName | string | 是 | 应用包名，用于标识调用者身份，通常使用 `context.bundleName` |
| callback | AsyncCallback<DeviceManager> | 是 | 异步回调，成功时返回 DeviceManager 实例对象 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误（bundleName 为空或格式错误） |
| 11600101 | 创建 DM 实例失败 |
| 11600102 | 获取 DM 服务失败 |

#### 示例代码
```typescript
import deviceManager from '@ohos.distributedHardware.deviceManager';

// 在应用启动时创建 DM 实例
let dmInstance: deviceManager.DeviceManager | null = null;

function initDeviceManager() {
    try {
        deviceManager.createDeviceManager('com.example.myapp', (err, dm) => {
            if (err) {
                console.error('创建 DeviceManager 失败:', JSON.stringify(err));
                return;
            }
            console.info('创建 DeviceManager 成功');
            dmInstance = dm;
            
            // 注册服务死亡监听
            dmInstance.on('serviceDie', () => {
                console.warn('DeviceManager 服务已停止');
                dmInstance = null;
            });
        });
    } catch (e) {
        console.error('创建 DeviceManager 异常:', e);
    }
}
```

---

### 3.2 release

#### 接口签名
```typescript
release(): void
```

#### 接口级别
**Public API** - 所有应用可调用

#### 所需权限
无

#### 功能描述
释放 DeviceManager 实例，释放相关资源。在应用退出或不再需要设备管理功能时调用。

#### 使用场景
- 应用退出时清理资源
- 切换用户时重置 DM 状态
- 长时间不使用 DM 功能时释放资源

#### 参数详解
无参数

#### 错误码
无错误码返回，调用后直接释放资源

#### 示例代码
```typescript
function cleanupDeviceManager() {
    if (dmInstance != null) {
        try {
            // 先取消所有监听
            dmInstance.off('deviceStateChange');
            dmInstance.off('deviceFound');
            dmInstance.off('serviceDie');
            
            // 释放实例
            dmInstance.release();
            dmInstance = null;
            console.info('DeviceManager 实例已释放');
        } catch (e) {
            console.error('释放 DeviceManager 失败:', e);
        }
    }
}
```

---

## 4. 设备发现接口

### 4.1 startDeviceDiscovery

#### 接口签名
```typescript
startDeviceDiscovery(subscribeInfo: SubscribeInfo): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`
- `ohos.permission.DISCOVER_DEVICE`

#### 功能描述
启动设备发现，扫描周边的可用设备。发现设备后会通过 `deviceFound` 事件回调返回设备信息。

#### 使用场景
- 添加新设备前的设备扫描
- 查找周边可用的分布式设备
- 建立临时连接前的设备发现

#### 参数详解
**SubscribeInfo 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| subscribeId | number | 是 | 订阅 ID，用于标识本次发现操作，应用内应唯一，范围 1000-65535 |
| mode | DiscoverMode | 是 | 发现模式：0x55（被动模式）/ 0xAA（主动模式） |
| medium | ExchangeMedium | 是 | 发现介质：0（自动）/ 1（BLE）/ 2（Wi-Fi）/ 3（USB） |
| freq | ExchangeFreq | 是 | 发现频率：0（低）/ 1（中）/ 2（高）/ 3（超高）/ 4（极高） |
| isSameAccount | boolean | 否 | 是否只发现同账号设备，默认 false |
| isWakeRemote | boolean | 否 | 是否唤醒休眠设备，默认 true |
| capability | string | 否 | 能力标签，用于过滤特定类型的设备 |

**DiscoverMode 枚举值：**
- `DISCOVER_MODE_PASSIVE = 0x55`：被动模式，设备只监听不主动广播
- `DISCOVER_MODE_ACTIVE = 0xAA`：主动模式，设备同时广播和监听

**ExchangeMedium 枚举值：**
- `AUTO = 0`：自动选择介质
- `BLE = 1`：蓝牙低功耗
- `COAP = 2`：Wi-Fi CoAP
- `USB = 3`：USB 连接

**ExchangeFreq 枚举值：**
- `LOW = 0`：低频率，功耗低
- `MID = 1`：中等频率
- `HIGH = 2`：高频率
- `SUPER_HIGH = 3`：超高频率
- `EXTREME_HIGH = 4`：极高频率

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误（subscribeId 重复或参数无效） |
| 11600101 | 启动发现失败 |
| 11600103 | 订阅 ID 已存在 |
| 11600104 | 发现服务未就绪 |

#### 示例代码
```typescript
// 启动设备发现
function startDiscovery() {
    // 生成唯一的订阅 ID
    const subscribeId = Math.floor(Math.random() * 54536) + 11000;
    
    // 配置发现参数
    const subscribeInfo: deviceManager.SubscribeInfo = {
        subscribeId: subscribeId,
        mode: deviceManager.DiscoverMode.DISCOVER_MODE_ACTIVE,
        medium: deviceManager.ExchangeMedium.AUTO,
        freq: deviceManager.ExchangeFreq.HIGH,
        isSameAccount: false,
        isWakeRemote: true,
        capability: "ddmpcapability"
    };
    
    // 可选：配置过滤选项
    const filterOptions = {
        filter_op: "OR",  // 过滤操作符：OR/AND
        filters: [
            {
                type: "range",      // 距离过滤
                value: 100         // 只发现 100cm 内的设备
            },
            {
                type: "deviceType", // 设备类型过滤
                value: 0x0E        // 只发现手机设备
            }
        ]
    };
    
    try {
        // 注册发现回调
        dmInstance.on('deviceFound', (data) => {
            console.info('发现设备:', JSON.stringify(data));
            // data.subscribeId: 订阅 ID
            // data.device: 设备信息
        });
        
        dmInstance.on('discoverFail', (data) => {
            console.error('发现失败:', JSON.stringify(data));
            // data.subscribeId: 订阅 ID
            // data.reason: 失败原因
        });
        
        // 启动发现
        dmInstance.startDeviceDiscovery(subscribeInfo, JSON.stringify(filterOptions));
        console.info('设备发现已启动, subscribeId:', subscribeId);
        
        // 保存 subscribeId 用于后续停止发现
        this.currentSubscribeId = subscribeId;
        
    } catch (e) {
        console.error('启动设备发现失败:', e);
    }
}
```

---

### 4.2 stopDeviceDiscovery

#### 接口签名
```typescript
stopDeviceDiscovery(subscribeId: number): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`
- `ohos.permission.DISCOVER_DEVICE`

#### 功能描述
停止设备发现，必须与 `startDeviceDiscovery` 配对使用，使用相同的 subscribeId。

#### 使用场景
- 完成设备扫描后停止
- 应用切后台时停止发现以节省功耗
- 发现超时后停止

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| subscribeId | number | 是 | 要停止的发现操作的订阅 ID，必须与 startDeviceDiscovery 中的 ID 一致 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误（subscribeId 无效） |
| 11600101 | 停止发现失败 |
| 11600105 | 订阅 ID 不存在 |

#### 示例代码
```typescript
function stopDiscovery() {
    if (this.currentSubscribeId == null) {
        console.warn('没有正在进行的发现操作');
        return;
    }
    
    try {
        dmInstance.stopDeviceDiscovery(this.currentSubscribeId);
        console.info('设备发现已停止, subscribeId:', this.currentSubscribeId);
        this.currentSubscribeId = null;
        
        // 取消回调
        dmInstance.off('deviceFound');
        dmInstance.off('discoverFail');
        
    } catch (e) {
        console.error('停止设备发现失败:', e);
    }
}

// 带超时的自动停止
function startDiscoveryWithTimeout() {
    startDiscovery();
    
    // 60 秒后自动停止
    setTimeout(() => {
        stopDiscovery();
    }, 60000);
}
```

---

### 4.3 publishDeviceDiscovery

#### 接口签名
```typescript
publishDeviceDiscovery(publishInfo: PublishInfo): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
发布设备发现信息，使当前设备能够被其他设备发现。通常与 `startDeviceDiscovery` 配合使用。

#### 使用场景
- 需要被其他设备发现时
- 建立临时连接时
- 设备配对时

#### 参数详解
**PublishInfo 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| publishId | number | 是 | 发布 ID，标识本次发布操作，应用内应唯一，范围 1000-65535 |
| mode | DiscoverMode | 是 | 发布模式：0x55（被动）/ 0xAA（主动） |
| freq | ExchangeFreq | 是 | 发布频率：0-4，值越大频率越高 |
| ranging | boolean | 是 | 是否支持距离测量 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误 |
| 11600101 | 发布失败 |
| 11600106 | 发布 ID 已存在 |

#### 示例代码
```typescript
function publishDevice() {
    const publishId = Math.floor(Math.random() * 54536) + 11000;
    
    const publishInfo: deviceManager.PublishInfo = {
        publishId: publishId,
        mode: deviceManager.DiscoverMode.DISCOVER_MODE_ACTIVE,
        freq: deviceManager.ExchangeFreq.HIGH,
        ranging: true  // 支持测距
    };
    
    // 注册发布结果回调
    dmInstance.on('publishSuccess', (data) => {
        console.info('设备发布成功, publishId:', data.publishId);
    });
    
    dmInstance.on('publishFail', (data) => {
        console.error('设备发布失败, publishId:', data.publishId, 
                     'reason:', data.reason);
    });
    
    try {
        dmInstance.publishDeviceDiscovery(publishInfo);
        console.info('设备发布已启动, publishId:', publishId);
        this.currentPublishId = publishId;
    } catch (e) {
        console.error('发布设备失败:', e);
    }
}
```

---

### 4.4 unPublishDeviceDiscovery

#### 接口签名
```typescript
unPublishDeviceDiscovery(publishId: number): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
停止发布设备发现信息，必须与 `publishDeviceDiscovery` 配对使用。

#### 使用场景
- 不再需要被其他设备发现时
- 完成设备配对后
- 应用进入后台时

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| publishId | number | 是 | 要停止的发布操作的发布 ID |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误 |
| 11600101 | 停止发布失败 |

#### 示例代码
```typescript
function unpublishDevice() {
    if (this.currentPublishId == null) {
        console.warn('没有正在进行的发布操作');
        return;
    }
    
    try {
        dmInstance.unPublishDeviceDiscovery(this.currentPublishId);
        console.info('设备发布已停止, publishId:', this.currentPublishId);
        this.currentPublishId = null;
        
        // 取消回调
        dmInstance.off('publishSuccess');
        dmInstance.off('publishFail');
        
    } catch (e) {
        console.error('停止设备发布失败:', e);
    }
}
```

---

## 5. 设备认证与绑定接口

### 5.1 authenticateDevice

#### 接口签名
```typescript
authenticateDevice(deviceInfo: DeviceInfo, authParam: AuthParam, 
                  callback: AsyncCallback<{deviceId: string, pinToken?: number}>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`
- `ohos.permission.DISTRIBUTED_DEVICE_AUTH_SERVICE`

#### 功能描述
对发现的设备进行认证，建立设备间的信任关系。认证过程可能涉及用户交互（如输入 PIN 码）。

#### 使用场景
- 将发现的设备添加为可信设备
- 建立长期设备信任关系
- 跨账号设备认证

#### 参数详解
**DeviceInfo 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| deviceId | string | 是 | 设备 ID，从 deviceFound 事件获取 |
| deviceName | string | 否 | 设备名称 |
| deviceType | number | 是 | 设备类型，见设备类型枚举 |

**AuthParam 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| authType | number | 是 | 认证类型：1（PIN 码）/ 2（扫码）/ 3（声波） |
| extraInfo | object | 否 | 扩展信息，包含应用自定义参数 |

**extraInfo 扩展信息：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| appOperation | string | 应用操作标识，用户自定义 |
| customDescription | string | 自定义描述信息 |
| displayOwner | number | UI 显示所有者：0（系统）/ 1（应用） |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误 |
| 11600101 | 认证失败 |
| 11600107 | 设备不支持该认证方式 |
| 11600108 | 认证超时 |
| 11600109 | 用户取消认证 |

#### 示例代码
```typescript
function authenticateTargetDevice(deviceInfo: deviceManager.DeviceInfo) {
    // 准备认证参数
    const authParam: deviceManager.AuthParam = {
        authType: 1,  // 1 表示 PIN 码认证
        extraInfo: {
            appOperation: 'add_device',
            customDescription: '添加设备到家庭组',
            displayOwner: 0  // 由系统显示 UI
        }
    };
    
    // 注册 UI 状态监听
    dmInstance.on('uiStateChange', (data) => {
        console.info('UI 状态变化:', JSON.stringify(data));
        const params = JSON.parse(data.param);
        if (params.verifyFailed) {
            console.error('认证失败，UI 已关闭');
        }
    });
    
    // 执行认证
    dmInstance.authenticateDevice(deviceInfo, authParam, (err, data) => {
        if (err) {
            console.error('设备认证失败:', JSON.stringify(err));
            return;
        }
        
        console.info('设备认证成功:', JSON.stringify(data));
        // data.deviceId: 认证成功的设备 ID
        // data.pinToken: PIN 令牌（可选）
        
        // 认证成功后，设备会被自动添加到可信列表
        refreshTrustedDeviceList();
    });
}
```

---

### 5.2 bindTarget

#### 接口签名
```typescript
bindTarget(targetId: PeerTargetId, bindParam: Record<string, string>, 
           callback: AsyncCallback<BindResult>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
绑定目标设备，建立服务级别的绑定关系。这是比 authenticateDevice 更高级的绑定方式，支持服务绑定。

#### 使用场景
- 建立服务级绑定关系
- 多服务场景下的设备绑定
- 需要细粒度控制绑定时

#### 参数详解
**PeerTargetId 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| deviceId | string | 是 | 设备 ID |
| brMac | string | 否 | 蓝牙 MAC 地址 |
| bleMac | string | 否 | BLE MAC 地址 |
| wifiIp | string | 否 | Wi-Fi IP 地址 |
| wifiPort | number | 否 | Wi-Fi 端口 |
| serviceId | number | 否 | 服务 ID |

**bindParam 对象结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| bindType | string | 绑定类型 |
| targetPkgName | string | 目标包名 |
| appName | string | 应用名称 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 11600101 | 绑定失败 |
| 11600110 | 目标设备不存在 |
| 11600111 | 绑定关系已存在 |

#### 示例代码
```typescript
function bindTargetDevice(deviceId: string) {
    const targetId: deviceManager.PeerTargetId = {
        deviceId: deviceId,
        wifiIp: '192.168.1.100',
        wifiPort: 8080,
        serviceId: 1001
    };
    
    const bindParam: Record<string, string> = {
        bindType: '1',
        targetPkgName: 'com.example.targetapp',
        appName: '我的应用'
    };
    
    dmInstance.bindTarget(targetId, bindParam, (err, result) => {
        if (err) {
            console.error('绑定目标设备失败:', JSON.stringify(err));
            return;
        }
        console.info('绑定成功:', JSON.stringify(result));
    });
}
```

---

### 5.3 unbindTarget

#### 接口签名
```typescript
unbindTarget(targetId: PeerTargetId, unbindParam: Record<string, string>, 
             callback: AsyncCallback<number>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
解绑目标设备，解除服务级绑定关系。

#### 使用场景
- 移除不再使用的设备绑定
- 切换账号时清理绑定关系
- 设备报废时解绑

#### 参数详解
**PeerTargetId**：同 bindTarget

**unbindParam 对象结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| unbindReason | string | 解绑原因 |
| forceUnbind | string | 是否强制解绑 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 11600101 | 解绑失败 |
| 11600112 | 绑定关系不存在 |

#### 示例代码
```typescript
function unbindTargetDevice(deviceId: string) {
    const targetId: deviceManager.PeerTargetId = {
        deviceId: deviceId,
        serviceId: 1001
    };
    
    const unbindParam: Record<string, string> = {
        unbindReason: 'user_request',
        forceUnbind: 'false'
    };
    
    dmInstance.unbindTarget(targetId, unbindParam, (err, result) => {
        if (err) {
            console.error('解绑失败:', JSON.stringify(err));
            return;
        }
        console.info('解绑成功, result:', result);
    });
}
```

---

### 5.4 setUserOperation

#### 接口签名
```typescript
setUserOperation(action: number, params: string): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
设置用户操作行为，用于响应当前认证流程中的用户交互。

#### 使用场景
- 用户确认认证操作
- 用户取消认证操作
- 用户输入 PIN 码后确认

#### 参数详解
**action 枚举值：**

| 值 | 说明 |
|----|------|
| 0 | 允许认证 |
| 1 | 取消认证 |
| 2 | 用户操作超时 |
| 3 | 取消 PIN 码显示 |
| 4 | 取消 PIN 码输入 |
| 5 | 确认 PIN 码输入 |

**params**：扩展参数 JSON 字符串，可包含自定义信息

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 11600101 | 设置操作失败 |
| 11600113 | 没有正在进行的认证流程 |

#### 示例代码
```typescript
function confirmAuthentication() {
    // 用户点击"允许"按钮
    dmInstance.setUserOperation(0, JSON.stringify({
        action: 'confirm',
        timestamp: Date.now()
    }));
}

function cancelAuthentication() {
    // 用户点击"取消"按钮
    dmInstance.setUserOperation(1, JSON.stringify({
        action: 'cancel',
        reason: 'user_cancelled'
    }));
}

function confirmPinCode(pinCode: string) {
    // 用户输入 PIN 码后确认
    dmInstance.setUserOperation(5, JSON.stringify({
        pinCode: pinCode,
        action: 'confirm_pin'
    }));
}
```

---

### 5.5 verifyAuthInfo

#### 接口签名
```typescript
verifyAuthInfo(authInfo: AuthInfo, callback: AsyncCallback<{deviceId: string, level: number}>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
验证设备认证信息，检查认证的有效性和级别。

#### 使用场景
- 验证导入的认证信息
- 检查设备信任级别
- 二次确认认证状态

#### 参数详解
**AuthInfo 对象结构：**

| 字段名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| version | string | 是 | 认证信息版本 |
| userId | string | 是 | 用户 ID |
| deviceId | string | 是 | 设备 ID |
| devicePk | string | 是 | 设备公钥 |
| authCode | string | 是 | 认证码 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 11600101 | 验证失败 |
| 11600114 | 认证信息无效 |

#### 示例代码
```typescript
function verifyAuthentication(authInfo: deviceManager.AuthInfo) {
    dmInstance.verifyAuthInfo(authInfo, (err, result) => {
        if (err) {
            console.error('验证认证信息失败:', JSON.stringify(err));
            return;
        }
        
        console.info('验证成功:');
        console.info('  设备 ID:', result.deviceId);
        console.info('  信任级别:', result.level);
    });
}
```

---

### 5.6 unAuthenticateDevice

#### 接口签名
```typescript
unAuthenticateDevice(deviceInfo: DeviceInfo): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
取消正在进行的设备认证操作。

#### 使用场景
- 用户主动取消认证
- 认证超时后清理
- 切换到其他设备认证

#### 参数详解
**DeviceInfo**：设备信息对象

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 11600101 | 取消认证失败 |
| 11600115 | 没有正在进行的认证 |

#### 示例代码
```typescript
function cancelAuthentication(deviceInfo: deviceManager.DeviceInfo) {
    try {
        dmInstance.unAuthenticateDevice(deviceInfo);
        console.info('已取消设备认证');
        
        // 清理 UI 状态
        dmInstance.off('uiStateChange');
        
    } catch (e) {
        console.error('取消认证失败:', e);
    }
}
```

---

## 6. 设备信息查询接口

### 6.1 getTrustedDeviceListSync

#### 接口签名
```typescript
getTrustedDeviceListSync(): Array<DeviceInfo>
```

#### 接口级别
**Public API** - 所有应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`（或 `ohos.permission.GET_TRUSTED_DEVICES`）

#### 功能描述
同步获取已认证的可信设备列表，返回所有已建立信任关系的设备信息。

#### 使用场景
- 应用启动时加载可信设备列表
- 刷新设备列表显示
- 选择目标设备前获取可用设备

#### 参数详解
无参数

#### 返回值
`Array<DeviceInfo>`：设备信息数组

**DeviceInfo 对象结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| deviceId | string | 设备唯一标识 |
| deviceName | string | 设备名称 |
| deviceType | number | 设备类型枚举值 |
| networkId | string | 网络 ID，用于分布式通信 |
| range | number | 设备距离（厘米） |
| networkType | number | 网络类型 |
| authForm | number | 认证形式 |

#### 错误码
可能抛出异常，包含以下错误码：

| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 11600101 | 获取设备列表失败 |

#### 示例代码
```typescript
function loadTrustedDevices() {
    try {
        const deviceList = dmInstance.getTrustedDeviceListSync();
        console.info('可信设备数量:', deviceList.length);
        
        deviceList.forEach((device, index) => {
            console.info(`设备 ${index + 1}:`);
            console.info('  ID:', device.deviceId);
            console.info('  名称:', device.deviceName);
            console.info('  类型:', getDeviceTypeName(device.deviceType));
            console.info('  网络 ID:', device.networkId);
            console.info('  距离:', device.range, 'cm');
        });
        
        return deviceList;
    } catch (e) {
        console.error('获取可信设备列表失败:', e);
        return [];
    }
}

function getDeviceTypeName(type: number): string {
    const typeMap = {
        0x0E: '手机',
        0x11: '平板',
        0x6D: '手表',
        0x9C: '电视',
        0x0A: '耳机',
        0x0C: 'PC',
        0x83: '车机'
    };
    return typeMap[type] || '未知设备';
}
```

---

### 6.2 getLocalDeviceInfoSync

#### 接口签名
```typescript
getLocalDeviceInfoSync(): DeviceInfo
```

#### 接口级别
**Public API** - 所有应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
同步获取本地设备信息，返回当前运行的设备详细信息。

#### 使用场景
- 显示本地设备信息
- 初始化设备状态
- 设备信息展示

#### 参数详解
无参数

#### 返回值
`DeviceInfo`：本地设备信息对象

#### 错误码
可能抛出异常，包含以下错误码：

| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 11600101 | 获取本地设备信息失败 |

#### 示例代码
```typescript
function showLocalDeviceInfo() {
    try {
        const localInfo = dmInstance.getLocalDeviceInfoSync();
        
        console.info('本地设备信息:');
        console.info('  设备 ID:', localInfo.deviceId);
        console.info('  设备名称:', localInfo.deviceName);
        console.info('  设备类型:', getDeviceTypeName(localInfo.deviceType));
        console.info('  网络 ID:', localInfo.networkId);
        console.info('  认证形式:', localInfo.authForm);
        
        // 更新 UI 显示
        this.localDeviceName = localInfo.deviceName;
        this.localDeviceType = getDeviceTypeName(localInfo.deviceType);
        
    } catch (e) {
        console.error('获取本地设备信息失败:', e);
    }
}
```

---

### 6.3 getDeviceInfo

#### 接口签名
```typescript
getDeviceInfo(networkId: string, callback: AsyncCallback<DeviceInfo>): void
```

#### 接口级别
**Public API** - 所有应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
根据网络 ID 异步获取指定设备的信息。

#### 使用场景
- 根据网络 ID 查询设备详情
- 验证设备是否存在
- 获取设备的最新信息

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| networkId | string | 是 | 设备的网络 ID |
| callback | AsyncCallback<DeviceInfo> | 是 | 异步回调 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 401 | 参数错误（networkId 无效） |
| 11600101 | 获取设备信息失败 |
| 11600116 | 设备不存在 |

#### 示例代码
```typescript
function queryDeviceInfo(networkId: string) {
    dmInstance.getDeviceInfo(networkId, (err, deviceInfo) => {
        if (err) {
            console.error('获取设备信息失败:', JSON.stringify(err));
            return;
        }
        
        console.info('设备信息查询成功:');
        console.info('  设备 ID:', deviceInfo.deviceId);
        console.info('  设备名称:', deviceInfo.deviceName);
        console.info('  设备类型:', deviceInfo.deviceType);
        
        // 更新 UI 或执行后续操作
        this.selectedDevice = deviceInfo;
    });
}
```

---

## 7. 设备状态监听接口

### 7.1 on('deviceStateChange')

#### 接口签名
```typescript
on(type: 'deviceStateChange', 
   callback: Callback<{action: DeviceStateChangeAction, device: DeviceInfo}>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
注册设备状态变化监听，当可信设备的状态发生改变时回调。

#### 使用场景
- 监听设备上线/下线
- 监听设备信息变化
- 实时更新设备列表

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| type | 'deviceStateChange' | 是 | 事件类型，固定值 |
| callback | Callback | 是 | 状态变化回调 |

**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| action | DeviceStateChangeAction | 状态变化动作 |
| device | DeviceInfo | 发生变化的设备信息 |

**DeviceStateChangeAction 枚举值：**

| 值 | 说明 |
|----|------|
| 0 (ONLINE) | 设备上线，设备物理连接建立 |
| 1 (READY) | 设备就绪，信息同步完成，可进行分布式操作 |
| 2 (OFFLINE) | 设备下线，设备物理连接断开 |
| 3 (CHANGE) | 设备信息变化（如名称、类型等） |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 11600101 | 注册监听失败 |

#### 示例代码
```typescript
function registerDeviceStateListener() {
    dmInstance.on('deviceStateChange', (data) => {
        const { action, device } = data;
        
        console.info('设备状态变化:');
        console.info('  设备名称:', device.deviceName);
        console.info('  设备 ID:', device.deviceId);
        console.info('  动作:', getActionName(action));
        
        switch (action) {
            case 0: // ONLINE
                console.info('>>> 设备已上线');
                handleDeviceOnline(device);
                break;
            case 1: // READY
                console.info('>>> 设备已就绪');
                handleDeviceReady(device);
                break;
            case 2: // OFFLINE
                console.info('>>> 设备已下线');
                handleDeviceOffline(device);
                break;
            case 3: // CHANGE
                console.info('>>> 设备信息已变化');
                handleDeviceChange(device);
                break;
        }
    });
    
    console.info('设备状态监听已注册');
}

function getActionName(action: number): string {
    const actions = {
        0: 'ONLINE',
        1: 'READY',
        2: 'OFFLINE',
        3: 'CHANGE'
    };
    return actions[action] || 'UNKNOWN';
}

function handleDeviceOnline(device: deviceManager.DeviceInfo) {
    // 设备上线处理逻辑
    refreshDeviceList();
    showNotification(`设备 ${device.deviceName} 已上线`);
}

function handleDeviceReady(device: deviceManager.DeviceInfo) {
    // 设备就绪处理逻辑
    enableDistributedFeatures(device);
}
```

---

### 7.2 off('deviceStateChange')

#### 接口签名
```typescript
off(type: 'deviceStateChange', 
    callback?: Callback<{action: DeviceStateChangeAction, device: DeviceInfo}>): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
取消设备状态变化监听。

#### 使用场景
- 应用退出时清理监听
- 不再需要监听设备状态时
- 重新注册监听前

#### 参数详解
| 参数名 | 类型 | 必填 | 说明 |
|-------|------|------|------|
| type | 'deviceStateChange' | 是 | 事件类型 |
| callback | Callback | 否 | 指定要取消的回调，不传则取消所有 |

#### 错误码
无

#### 示例代码
```typescript
function unregisterDeviceStateListener() {
    // 取消特定回调
    // dmInstance.off('deviceStateChange', specificCallback);
    
    // 取消所有设备状态监听
    dmInstance.off('deviceStateChange');
    
    console.info('设备状态监听已取消');
}
```

---

### 7.3 on('deviceFound') / off('deviceFound')

#### 接口签名
```typescript
on(type: 'deviceFound', 
   callback: Callback<{subscribeId: number, device: DeviceInfo}>): void

off(type: 'deviceFound', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`
- `ohos.permission.DISCOVER_DEVICE`

#### 功能描述
注册/取消设备发现监听，当发现新设备时回调。

#### 使用场景
- 设备发现过程中实时显示发现的设备
- 自动连接符合条件的设备
- 设备扫描结果展示

#### 参数详解
**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| subscribeId | number | 订阅 ID，用于区分不同的发现操作 |
| device | DeviceInfo | 发现的设备信息 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |
| 11600101 | 注册监听失败 |

#### 示例代码
```typescript
function registerDeviceFoundListener() {
    dmInstance.on('deviceFound', (data) => {
        console.info('发现新设备:');
        console.info('  订阅 ID:', data.subscribeId);
        console.info('  设备 ID:', data.device.deviceId);
        console.info('  设备名称:', data.device.deviceName);
        console.info('  设备类型:', data.device.deviceType);
        
        // 过滤设备
        if (data.device.deviceType === 0x0E) { // 只处理手机设备
            handlePhoneDeviceFound(data.device);
        }
        
        // 更新 UI
        updateDiscoveredDeviceList(data.device);
    });
}

function handlePhoneDeviceFound(device: deviceManager.DeviceInfo) {
    console.info('发现手机设备，准备自动认证');
    
    // 可以在这里触发自动认证流程
    // authenticateTargetDevice(device);
}

function unregisterDeviceFoundListener() {
    dmInstance.off('deviceFound');
    console.info('设备发现监听已取消');
}
```

---

### 7.4 on('discoverFail') / off('discoverFail')

#### 接口签名
```typescript
on(type: 'discoverFail', 
   callback: Callback<{subscribeId: number, reason: number}>): void

off(type: 'discoverFail', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`
- `ohos.permission.DISCOVER_DEVICE`

#### 功能描述
注册/取消设备发现失败监听。

#### 使用场景
- 设备发现异常处理
- 错误提示和日志记录
- 重试机制触发

#### 参数详解
**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| subscribeId | number | 订阅 ID |
| reason | number | 失败原因码 |

**失败原因码：**

| 值 | 说明 |
|----|------|
| 0 | 发现成功 |
| -1 | 未知错误 |
| -2 | 蓝牙未开启 |
| -3 | Wi-Fi 未开启 |
| -4 | 权限不足 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |

#### 示例代码
```typescript
function registerDiscoverFailListener() {
    dmInstance.on('discoverFail', (data) => {
        console.error('设备发现失败:');
        console.error('  订阅 ID:', data.subscribeId);
        console.error('  失败原因:', getFailReason(data.reason));
        
        // 根据原因进行处理
        switch (data.reason) {
            case -2:
                showPrompt('蓝牙未开启，请开启蓝牙后重试');
                break;
            case -3:
                showPrompt('Wi-Fi 未开启，请开启 Wi-Fi 后重试');
                break;
            default:
                showPrompt('设备发现失败，请稍后重试');
        }
    });
}

function getFailReason(reason: number): string {
    const reasons = {
        0: '成功',
        '-1': '未知错误',
        '-2': '蓝牙未开启',
        '-3': 'Wi-Fi 未开启',
        '-4': '权限不足'
    };
    return reasons[reason] || '未知错误';
}
```

---

### 7.5 on('serviceDie') / off('serviceDie')

#### 接口签名
```typescript
on(type: 'serviceDie', callback: Callback<void>): void
off(type: 'serviceDie', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
注册/取消 DM 服务停止监听，当 DM 服务异常终止时回调。

#### 使用场景
- 服务异常检测
- 自动重新初始化
- 资源清理和状态重置

#### 参数详解
回调无参数

#### 错误码
无

#### 示例代码
```typescript
function registerServiceDieListener() {
    dmInstance.on('serviceDie', () => {
        console.error('!!! DeviceManager 服务已停止 !!!');
        
        // 清理资源
        cleanupDeviceManager();
        
        // 尝试重新初始化
        setTimeout(() => {
            console.info('尝试重新初始化 DeviceManager...');
            initDeviceManager();
        }, 3000);
    });
}

function cleanupDeviceManager() {
    // 取消所有监听
    if (dmInstance) {
        dmInstance.off('deviceStateChange');
        dmInstance.off('deviceFound');
        dmInstance.off('discoverFail');
        dmInstance.off('publishSuccess');
        dmInstance.off('publishFail');
        dmInstance.off('uiStateChange');
        dmInstance.off('serviceDie');
    }
    
    // 清空状态
    this.currentSubscribeId = null;
    this.currentPublishId = null;
    this.trustedDeviceList = [];
    
    console.info('资源清理完成');
}
```

---

### 7.6 on('uiStateChange') / off('uiStateChange')

#### 接口签名
```typescript
on(type: 'uiStateChange', callback: Callback<{param: string}>): void
off(type: 'uiStateChange', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
注册/取消认证 UI 状态变化监听。

#### 使用场景
- 监听认证对话框状态
- 响应用户认证操作
- UI 自动关闭处理

#### 参数详解
**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| param | string | JSON 字符串，包含 UI 状态信息 |

**param JSON 结构：**

```json
{
  "verifyFailed": false,  // 认证是否失败
  "uiClosed": true        // UI 是否已关闭
}
```

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |

#### 示例代码
```typescript
function registerUIStateChangeListener() {
    dmInstance.on('uiStateChange', (data) => {
        console.info('UI 状态变化:', data.param);
        
        try {
            const state = JSON.parse(data.param);
            
            if (state.uiClosed) {
                console.info('认证 UI 已关闭');
                
                if (state.verifyFailed) {
                    console.error('认证失败');
                    showErrorMessage('设备认证失败，请重试');
                } else {
                    console.info('认证成功或用户取消');
                }
                
                // 清理 UI 相关状态
                hideLoadingIndicator();
            }
            
        } catch (e) {
            console.error('解析 UI 状态失败:', e);
        }
    });
}
```

---

### 7.7 on('publishSuccess') / off('publishSuccess')

#### 接口签名
```typescript
on(type: 'publishSuccess', callback: Callback<{publishId: number}>): void
off(type: 'publishSuccess', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
注册/取消设备发布成功监听。

#### 使用场景
- 设备发布成功确认
- 启动后续流程
- 状态提示

#### 参数详解
**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| publishId | number | 发布 ID |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |

#### 示例代码
```typescript
function registerPublishSuccessListener() {
    dmInstance.on('publishSuccess', (data) => {
        console.info('设备发布成功, publishId:', data.publishId);
        
        showSuccessMessage('设备已可被其他设备发现');
        
        // 启动超时自动停止
        setTimeout(() => {
            unpublishDevice();
        }, 60000); // 60 秒后自动停止发布
    });
}
```

---

### 7.8 on('publishFail') / off('publishFail')

#### 接口签名
```typescript
on(type: 'publishFail', 
   callback: Callback<{publishId: number, reason: number}>): void
off(type: 'publishFail', callback?: Callback): void
```

#### 接口级别
**System API** - 仅系统应用可调用

#### 所需权限
- `ohos.permission.ACCESS_SERVICE_DM`

#### 功能描述
注册/取消设备发布失败监听。

#### 使用场景
- 发布失败处理
- 错误提示
- 重试机制

#### 参数详解
**回调参数结构：**

| 字段名 | 类型 | 说明 |
|-------|------|------|
| publishId | number | 发布 ID |
| reason | number | 失败原因 |

#### 错误码
| 错误码 | 说明 |
|-------|------|
| 201 | 权限验证失败 |

#### 示例代码
```typescript
function registerPublishFailListener() {
    dmInstance.on('publishFail', (data) => {
        console.error('设备发布失败:');
        console.error('  publishId:', data.publishId);
        console.error('  reason:', data.reason);
        
        showError('设备发布失败，请检查蓝牙或 Wi-Fi 是否开启');
        
        // 清理状态
        this.currentPublishId = null;
    });
}
```

---

## 8. 设备类型与枚举值

### 8.1 设备类型枚举 (DeviceType)

| 值 | 常量名 | 设备类型 | 说明 |
|----|--------|---------|------|
| 0x00 | DEVICE_TYPE_UNKNOWN | 未知设备 | 无法识别的设备类型 |
| 0x08 | DEVICE_TYPE_WIFI_CAMERA | 智能相机 | Wi-Fi 相机设备 |
| 0x0A | DEVICE_TYPE_AUDIO | 智能音箱 | 音频设备 |
| 0x0C | DEVICE_TYPE_PC | 智能电脑 | 个人电脑 |
| 0x0E | DEVICE_TYPE_PHONE | 智能手机 | 手机设备 |
| 0x11 | DEVICE_TYPE_PAD | 智能平板 | 平板电脑 |
| 0x6D | DEVICE_TYPE_WATCH | 智能手表 | 手表设备 |
| 0x83 | DEVICE_TYPE_CAR | 智能车载 | 车机设备 |
| 0x9C | DEVICE_TYPE_TV | 智能电视 | 电视设备 |
| 0xA02 | DEVICE_TYPE_SMART_DISPLAY | 智能显示屏 | 显示屏设备 |
| 0xA2F | DEVICE_TYPE_2IN1 | 二合一设备 | 2in1 设备 |
| 0xA31 | DEVICE_TYPE_GLASSES | AI 眼镜 | 智能眼镜 |
| 0x2E | THIRD_TV | 第三方电视 | 第三方电视设备 |

### 8.2 设备状态枚举 (DeviceState)

| 值 | 常量名 | 状态 | 说明 |
|----|--------|------|------|
| -1 | DEVICE_STATE_UNKNOWN | 未知状态 | 设备状态未知 |
| 0 | DEVICE_STATE_ONLINE | 在线 | 设备物理在线 |
| 1 | DEVICE_INFO_READY | 就绪 | 设备信息同步完成，可进行分布式操作 |
| 2 | DEVICE_STATE_OFFLINE | 离线 | 设备物理离线 |
| 3 | DEVICE_INFO_CHANGED | 变化 | 设备信息发生变化 |

### 8.3 认证形式枚举 (DmAuthForm)

| 值 | 常量名 | 认证形式 | 说明 |
|----|--------|---------|------|
| -1 | INVALID_TYPE | 无效 | 无效的认证形式 |
| 0 | PEER_TO_PEER | 点对点 | 点对点认证 |
| 1 | IDENTICAL_ACCOUNT | 同账号 | 相同账号认证 |
| 2 | ACROSS_ACCOUNT | 跨账号 | 跨账号认证 |
| 3 | SHARE | 共享 | 共享认证 |

### 8.4 网络类型枚举 (DmNetworkType)

| 值 | 常量名 | 网络类型 | 说明 |
|----|--------|---------|------|
| 0 | BIT_NETWORK_TYPE_UNKNOWN | 未知 | 未知网络类型 |
| 1 | BIT_NETWORK_TYPE_WIFI | Wi-Fi | Wi-Fi 网络 |
| 2 | BIT_NETWORK_TYPE_BLE | BLE | 蓝牙低功耗 |
| 3 | BIT_NETWORK_TYPE_BR | BR | 蓝牙 BR |
| 4 | BIT_NETWORK_TYPE_P2P | P2P | P2P 网络 |
| 8 | BIT_NETWORK_TYPE_USB | USB | USB 连接 |

---

## 9. NDK C 接口对照

### 9.1 NDK 接口列表

| NDK C 接口 | 对应 JS 接口 | 说明 |
|-----------|-------------|------|
| `OH_DeviceManager_GetLocalDeviceName()` | `getLocalDeviceInfoSync()` | 获取本地设备名称 |

### 9.2 NDK 接口详细说明

#### OH_DeviceManager_GetLocalDeviceName

**函数原型：**
```c
int32_t OH_DeviceManager_GetLocalDeviceName(char **localDeviceName, unsigned int *len);
```

**功能：**
获取本地设备名称

**参数：**
- `localDeviceName`：输出参数，设备名称指针，使用后需手动释放
- `len`：输出参数，设备名称长度

**返回值：**
- `ERR_OK (0)`：成功
- `DM_ERR_FAILED (11600101)`：执行失败
- `DM_ERR_OBTAIN_SERVICE (11600102)`：获取服务失败
- `DM_ERR_OBTAIN_BUNDLE_NAME (11600109)`：获取包名失败

**示例代码：**
```c
#include <ohos_device_manager.h>

void getLocalDeviceNameExample() {
    char *deviceName = NULL;
    unsigned int len = 0;
    
    int32_t result = OH_DeviceManager_GetLocalDeviceName(&deviceName, &len);
    
    if (result == ERR_OK) {
        printf("本地设备名称: %s\n", deviceName);
        printf("名称长度: %u\n", len);
        
        // 释放内存
        free(deviceName);
    } else {
        printf("获取设备名称失败, 错误码: %d\n", result);
    }
}
```

---

## 10. Cangjie FFI 接口对照

### 10.1 FFI 接口列表

| Cangjie FFI 接口 | 对应 JS 接口 | 说明 |
|-----------------|-------------|------|
| `FfiOHOSDistributedDeviceManagerCreateDeviceManager()` | `createDeviceManager()` | 创建 DM 实例 |
| `FfiOHOSDistributedDeviceManagerReleaseDeviceManager()` | `release()` | 释放 DM 实例 |
| `FfiOHOSDistributedDeviceManagerGetAvailableDeviceList()` | `getTrustedDeviceListSync()` | 获取可用设备列表 |
| `FfiOHOSDistributedDeviceManagerGetLocalDeviceNetworkId()` | `getLocalDeviceInfoSync()` | 获取本地网络 ID |
| `FfiOHOSDistributedDeviceManagerGetLocalDeviceName()` | - | 获取本地设备名称 |
| `FfiOHOSDistributedDeviceManagerGetLocalDeviceType()` | - | 获取本地设备类型 |
| `FfiOHOSDistributedDeviceManagerGetLocalDeviceId()` | - | 获取本地设备 ID |
| `FfiOHOSDistributedDeviceManagerGetDeviceName()` | `getDeviceInfo()` | 获取设备名称 |
| `FfiOHOSDistributedDeviceManagerGetDeviceType()` | - | 获取设备类型 |
| `FfiOHOSDistributedDeviceManagerStartDiscovering()` | `startDeviceDiscovery()` | 启动设备发现 |
| `FfiOHOSDistributedDeviceManagerStopDiscovering()` | `stopDeviceDiscovery()` | 停止设备发现 |
| `FfiOHOSDistributedDeviceManagerBindTarget()` | `bindTarget()` | 绑定目标设备 |
| `FfiOHOSDistributedDeviceManagerUnbindTarget()` | `unbindTarget()` | 解绑目标设备 |
| `FfiOHOSDistributedDeviceManagerOn()` | `on()` | 注册事件监听 |
| `FfiOHOSDistributedDeviceManagerOff()` | `off()` | 取消事件监听 |
| `FfiOHOSDistributedDeviceManagerFreeDeviceInfoList()` | - | 释放设备列表内存 |

### 10.2 FFI 数据结构

#### FfiDeviceBasicInfo
```c
typedef struct {
    char *deviceId;
    char *deviceName;
    uint16_t deviceType;
    char *networkId;
} FfiDeviceBasicInfo;
```

#### FfiDeviceBasicInfoArray
```c
typedef struct {
    FfiDeviceBasicInfo *head;
    int64_t size;
} FfiDeviceBasicInfoArray;
```

### 10.3 FFI 接口详细说明

#### FfiOHOSDistributedDeviceManagerCreateDeviceManager

**函数原型：**
```c
int64_t FfiOHOSDistributedDeviceManagerCreateDeviceManager(const char *name, int32_t *errCode);
```

**功能：**
创建 DeviceManager 实例

**参数：**
- `name`：应用包名
- `errCode`：输出错误码

**返回值：**
- 成功：返回 DM 实例 ID（正整数）
- 失败：返回 -1，errCode 包含具体错误

#### FfiOHOSDistributedDeviceManagerReleaseDeviceManager

**函数原型：**
```c
void FfiOHOSDistributedDeviceManagerReleaseDeviceManager(int64_t id, int32_t *errCode);
```

**功能：**
释放 DeviceManager 实例

**参数：**
- `id`：DM 实例 ID
- `errCode`：输出错误码

#### FfiOHOSDistributedDeviceManagerGetAvailableDeviceList

**函数原型：**
```c
void FfiOHOSDistributedDeviceManagerGetAvailableDeviceList(
    int64_t id, FfiDeviceBasicInfoArray *deviceInfoList, int32_t *errCode);
```

**功能：**
获取可用设备列表

**参数：**
- `id`：DM 实例 ID
- `deviceInfoList`：输出设备列表
- `errCode`：输出错误码

---

## 11. 完整应用示例

### 11.1 完整的设备管理流程

```typescript
import deviceManager from '@ohos.distributedHardware.deviceManager';

class DeviceManagerHelper {
    private dmInstance: deviceManager.DeviceManager | null = null;
    private currentSubscribeId: number | null = null;
    private currentPublishId: number | null = null;
    private trustedDevices: deviceManager.DeviceInfo[] = [];
    
    /**
     * 初始化 DeviceManager
     */
    async init(bundleName: string): Promise<boolean> {
        return new Promise((resolve) => {
            deviceManager.createDeviceManager(bundleName, (err, dm) => {
                if (err) {
                    console.error('创建 DeviceManager 失败:', JSON.stringify(err));
                    resolve(false);
                    return;
                }
                
                this.dmInstance = dm;
                console.info('DeviceManager 创建成功');
                
                // 注册服务死亡监听
                this.dmInstance.on('serviceDie', () => {
                    console.error('DeviceManager 服务已停止');
                    this.cleanup();
                });
                
                // 注册设备状态监听
                this.registerDeviceStateListener();
                
                // 加载可信设备列表
                this.loadTrustedDevices();
                
                resolve(true);
            });
        });
    }
    
    /**
     * 注册设备状态监听
     */
    private registerDeviceStateListener() {
        if (!this.dmInstance) return;
        
        this.dmInstance.on('deviceStateChange', (data) => {
            const { action, device } = data;
            console.info(`设备状态变化: ${device.deviceName}, action: ${action}`);
            
            switch (action) {
                case 0: // ONLINE
                    console.info('设备上线');
                    this.loadTrustedDevices();
                    break;
                case 1: // READY
                    console.info('设备就绪');
                    break;
                case 2: // OFFLINE
                    console.info('设备下线');
                    this.loadTrustedDevices();
                    break;
                case 3: // CHANGE
                    console.info('设备信息变化');
                    this.loadTrustedDevices();
                    break;
            }
        });
    }
    
    /**
     * 加载可信设备列表
     */
    loadTrustedDevices(): deviceManager.DeviceInfo[] {
        if (!this.dmInstance) return [];
        
        try {
            this.trustedDevices = this.dmInstance.getTrustedDeviceListSync();
            console.info(`可信设备数量: ${this.trustedDevices.length}`);
            
            this.trustedDevices.forEach((device, index) => {
                console.info(`设备 ${index + 1}: ${device.deviceName} (${device.deviceId})`);
            });
            
            return this.trustedDevices;
        } catch (e) {
            console.error('获取可信设备列表失败:', e);
            return [];
        }
    }
    
    /**
     * 启动设备发现
     */
    startDiscovery(callback?: (device: deviceManager.DeviceInfo) => void): boolean {
        if (!this.dmInstance) {
            console.error('DeviceManager 未初始化');
            return false;
        }
        
        const subscribeId = Math.floor(Math.random() * 54536) + 11000;
        
        const subscribeInfo: deviceManager.SubscribeInfo = {
            subscribeId: subscribeId,
            mode: deviceManager.DiscoverMode.DISCOVER_MODE_ACTIVE,
            medium: deviceManager.ExchangeMedium.AUTO,
            freq: deviceManager.ExchangeFreq.HIGH,
            isSameAccount: false,
            isWakeRemote: true,
            capability: "ddmpcapability"
        };
        
        const filterOptions = {
            filter_op: "OR",
            filters: [
                { type: "range", value: 100 }
            ]
        };
        
        // 注册发现回调
        this.dmInstance.on('deviceFound', (data) => {
            console.info('发现设备:', JSON.stringify(data.device));
            if (callback) {
                callback(data.device);
            }
        });
        
        this.dmInstance.on('discoverFail', (data) => {
            console.error('发现失败:', JSON.stringify(data));
        });
        
        try {
            this.dmInstance.startDeviceDiscovery(subscribeInfo, JSON.stringify(filterOptions));
            this.currentSubscribeId = subscribeId;
            console.info('设备发现已启动');
            return true;
        } catch (e) {
            console.error('启动设备发现失败:', e);
            return false;
        }
    }
    
    /**
     * 停止设备发现
     */
    stopDiscovery() {
        if (!this.dmInstance || !this.currentSubscribeId) {
            console.warn('没有正在进行的发现操作');
            return;
        }
        
        try {
            this.dmInstance.stopDeviceDiscovery(this.currentSubscribeId);
            this.dmInstance.off('deviceFound');
            this.dmInstance.off('discoverFail');
            this.currentSubscribeId = null;
            console.info('设备发现已停止');
        } catch (e) {
            console.error('停止设备发现失败:', e);
        }
    }
    
    /**
     * 认证设备
     */
    authenticateDevice(deviceInfo: deviceManager.DeviceInfo): Promise<boolean> {
        return new Promise((resolve) => {
            if (!this.dmInstance) {
                resolve(false);
                return;
            }
            
            const authParam: deviceManager.AuthParam = {
                authType: 1, // PIN 码认证
                extraInfo: {
                    appOperation: 'add_device',
                    customDescription: '添加设备',
                    displayOwner: 0
                }
            };
            
            // 注册 UI 状态监听
            this.dmInstance.on('uiStateChange', (data) => {
                const params = JSON.parse(data.param);
                if (params.verifyFailed) {
                    console.error('认证失败');
                    resolve(false);
                }
            });
            
            this.dmInstance.authenticateDevice(deviceInfo, authParam, (err, data) => {
                if (err) {
                    console.error('设备认证失败:', JSON.stringify(err));
                    resolve(false);
                    return;
                }
                
                console.info('设备认证成功:', JSON.stringify(data));
                this.loadTrustedDevices();
                resolve(true);
            });
        });
    }
    
    /**
     * 取消认证
     */
    cancelAuthentication(deviceInfo: deviceManager.DeviceInfo) {
        if (!this.dmInstance) return;
        
        try {
            this.dmInstance.unAuthenticateDevice(deviceInfo);
            this.dmInstance.off('uiStateChange');
            console.info('已取消设备认证');
        } catch (e) {
            console.error('取消认证失败:', e);
        }
    }
    
    /**
     * 获取本地设备信息
     */
    getLocalDeviceInfo(): deviceManager.DeviceInfo | null {
        if (!this.dmInstance) return null;
        
        try {
            const localInfo = this.dmInstance.getLocalDeviceInfoSync();
            console.info('本地设备信息:', JSON.stringify(localInfo));
            return localInfo;
        } catch (e) {
            console.error('获取本地设备信息失败:', e);
            return null;
        }
    }
    
    /**
     * 清理资源
     */
    cleanup() {
        if (this.currentSubscribeId) {
            this.stopDiscovery();
        }
        
        if (this.dmInstance) {
            this.dmInstance.off('deviceStateChange');
            this.dmInstance.off('serviceDie');
            this.dmInstance.release();
            this.dmInstance = null;
        }
        
        this.trustedDevices = [];
        console.info('DeviceManager 资源已清理');
    }
}

// 使用示例
const dmHelper = new DeviceManagerHelper();

// 初始化
dmHelper.init('com.example.myapp').then(success => {
    if (success) {
        // 获取本地设备信息
        const localInfo = dmHelper.getLocalDeviceInfo();
        console.info('本地设备:', localInfo?.deviceName);
        
        // 启动设备发现
        dmHelper.startDiscovery((device) => {
            console.info('发现设备:', device.deviceName);
            
            // 认证设备
            if (device.deviceType === 0x0E) { // 手机
                dmHelper.authenticateDevice(device).then(result => {
                    if (result) {
                        console.info('设备认证成功');
                    }
                });
            }
        });
        
        // 60 秒后停止发现
        setTimeout(() => {
            dmHelper.stopDiscovery();
        }, 60000);
    }
});
```

---

## 12. 错误处理与最佳实践

### 12.1 错误处理规范

1. **始终检查回调错误参数**
   ```typescript
   dmInstance.someMethod(params, (err, data) => {
       if (err) {
           // 处理错误
           return;
       }
       // 处理成功情况
   });
   ```

2. **使用 try-catch 包裹同步方法**
   ```typescript
   try {
       const devices = dmInstance.getTrustedDeviceListSync();
   } catch (e) {
       // 处理异常
   }
   ```

3. **监听 serviceDie 事件**
   ```typescript
   dmInstance.on('serviceDie', () => {
       // 服务停止，清理并重新初始化
   });
   ```

### 12.2 资源管理规范

1. **配对的接口必须成对调用**
   - `startDeviceDiscovery()` ↔ `stopDeviceDiscovery()`
   - `publishDeviceDiscovery()` ↔ `unPublishDeviceDiscovery()`
   - `on()` ↔ `off()`

2. **应用退出时清理资源**
   ```typescript
   onApplicationExit() {
       dmInstance?.off('deviceStateChange');
       dmInstance?.off('deviceFound');
       dmInstance?.release();
   }
   ```

3. **避免重复创建 DM 实例**
   ```typescript
   if (!dmInstance) {
       createDeviceManager();
   }
   ```

### 12.3 性能优化建议

1. **合理设置发现频率**
   - 主动发现使用 `HIGH` 频率
   - 被动发现使用 `LOW` 频率以节省功耗

2. **及时停止发现操作**
   ```typescript
   startDiscovery();
   setTimeout(() => stopDiscovery(), 30000); // 30秒后停止
   ```

3. **使用过滤条件减少不必要的事件**
   ```typescript
   const filterOptions = {
       filter_op: "OR",
       filters: [
           { type: "range", value: 50 },
           { type: "deviceType", value: 0x0E }
       ]
   };
   ```

---

## 13. 版本历史

| 版本 | 日期 | 说明 |
|-----|------|------|
| v2.0 | 2026-05-19 | 完整的外部 API 参考文档，包含 JS、NDK、Cangjie 接口 |
| v1.0 | 2022-01-01 | 初始版本 |

---

## 14. 参考资源

- [OpenHarmony DeviceManager 官方文档](https://gitee.com/openharmony/distributedhardware_device_manager)
- [Interface SDK JS 仓库](https://gitee.com/openharmony/interface_sdk-js)
- [OpenHarmony 分布式硬件子系统](https://gitee.com/openharmony/distributedhardware)

---

**文档结束**
