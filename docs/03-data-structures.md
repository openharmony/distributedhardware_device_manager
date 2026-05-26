# OpenHarmony DeviceManager 数据结构文档

**版本**: v2.0  
**日期**: 2026-05-19

本文档详细描述 OpenHarmony DeviceManager 子系统中的核心数据结构，包括字段说明、取值范围、生命周期和存储位置。

---

## 1. DmDeviceInfo - 设备信息结构

```cpp
typedef struct DmDeviceInfo {
    char deviceId[DM_MAX_DEVICE_ID_LEN] = {0};        // DM_MAX_DEVICE_ID_LEN = 97
    char deviceName[DM_MAX_DEVICE_NAME_LEN] = {0};    // DM_MAX_DEVICE_NAME_LEN = 129
    uint16_t deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    char networkId[DM_MAX_DEVICE_ID_LEN] = {0};
    int32_t range = 0;
    int32_t networkType = 0;
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::string extraData = "";
} DmDeviceInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| deviceId | char[97] | 设备唯一标识符 | 设备的 UDID（Unique Device ID） |
| deviceName | char[129] | 设备名称 | 用户可读的设备名称，UTF-8 编码 |
| deviceTypeId | uint16_t | 设备类型枚举值 | 参见 DmDeviceType 枚举定义 |
| networkId | char[97] | 网络 ID | 用于设备间通信的网络标识符 |
| range | int32_t | 设备距离 | 单位：厘米 (cm)，0 表示未知或本地设备 |
| networkType | int32_t | 网络类型 | 参见 DmNetworkType 枚举定义 |
| authForm | DmAuthForm | 认证形式 | 参见 DmAuthForm 枚举定义 |
| extraData | std::string | 扩展数据 | JSON 格式字符串，包含连接地址等额外信息 |

**extraData JSON 字段**:
- `"CONN_ADDR_TYPE"`: 连接地址类型
- `"BR_MAC_"`: 蓝牙 BR MAC 地址
- `"BLE_MAC"`: 蓝牙 LE MAC 地址
- `"WIFI_IP"`: Wi-Fi IP 地址
- `"WIFI_PORT"`: Wi-Fi 端口
- `"CUSTOM_DATA"`: 自定义数据

### 生命周期
- **创建时机**: 设备发现、认证完成后、设备状态变化时
- **销毁时机**: 进程退出、设备解除信任、设备下线
- **存储位置**: 
  - 内存中: 设备列表缓存
  - 持久化: 设备配置信息存储在分布式数据库

### 使用场景
- `GetTrustedDeviceList()`: 获取已信任设备列表
- `GetDeviceInfo()`: 根据网络ID获取设备信息
- `OnDeviceOnline()`: 设备上线回调
- `OnDeviceOffline()`: 设备下线回调
- `OnDeviceReady()`: 设备就绪回调

---

## 2. DmDeviceBasicInfo - 设备基本信息结构

```cpp
typedef struct DmDeviceBasicInfo {
    char deviceId[DM_MAX_DEVICE_ID_LEN] = {0};
    char deviceName[DM_MAX_DEVICE_NAME_LEN] = {0};
    uint16_t deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    char networkId[DM_MAX_DEVICE_ID_LEN] = {0};
    std::string extraData = "";
} DmDeviceBasicInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| deviceId | char[97] | 设备唯一标识符 | 设备的 UDID |
| deviceName | char[129] | 设备名称 | 用户可读的设备名称 |
| deviceTypeId | uint16_t | 设备类型枚举值 | 参见 DmDeviceType 枚举定义 |
| networkId | char[97] | 网络 ID | 用于设备间通信的网络标识符 |
| extraData | std::string | 扩展数据 | JSON 格式字符串，包含 `"CUSTOM_DATA"` |

### 生命周期
- **创建时机**: 设备发现时、获取可用设备列表时
- **销毁时机**: 进程退出、设备下线
- **存储位置**: 内存中临时存储

### 使用场景
- `GetAvailableDeviceList()`: 获取可用设备列表
- `DeviceStatusCallback`: 设备状态变化的回调接口

---

## 3. DmAuthParam - 认证参数结构

```cpp
typedef struct DmAuthParam {
    std::string authToken;          // 认证令牌
    std::string packageName;        // 包名
    std::string appName;            // 应用名称
    std::string appDescription;     // 应用描述
    int32_t authType;               // 认证类型
    int32_t business;               // 业务类型
    int32_t pincode;                // PIN 码
    int32_t direction;              // 认证方向
    int32_t pinToken;               // PIN 令牌
    DmAppImageInfo imageinfo;       // 应用图像信息
} DmAuthParam;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| authToken | std::string | 认证令牌 | 用于标识本次认证会话的唯一令牌 |
| packageName | std::string | 包名 | 发起认证的应用包名 |
| appName | std::string | 应用名称 | 应用显示名称 |
| appDescription | std::string | 应用描述 | 应用功能描述信息 |
| authType | int32_t | 认证类型 | 参见 DmAuthType 枚举定义 |
| business | int32_t | 业务类型 | 业务标识符 |
| pincode | int32_t | PIN 码 | 数字形式的 PIN 码 |
| direction | int32_t | 认证方向 | 0: 主动认证, 1: 被动认证 |
| pinToken | int32_t | PIN 令牌 | PIN 码对应的令牌 |
| imageinfo | DmAppImageInfo | 应用图像信息 | 包含应用图标和缩略图 |

### 生命周期
- **创建时机**: 发起设备认证时
- **销毁时机**: 认证流程结束
- **存储位置**: 内存中临时存储，认证完成后释放

### 使用场景
- `AuthenticateDevice()`: 设备认证接口
- `GetFaParam()`: 获取 FA 参数（已废弃）

---

## 4. PeerTargetId - 对端目标标识结构

```cpp
typedef struct PeerTargetId {
    std::string deviceId;    // 设备 ID
    std::string brMac;       // 蓝牙 BR MAC 地址
    std::string bleMac;      // 蓝牙 LE MAC 地址
    std::string wifiIp;      // Wi-Fi IP 地址
    uint16_t wifiPort = 0;   // Wi-Fi 端口
    int64_t serviceId = 0;   // 服务 ID
} PeerTargetId;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| deviceId | std::string | 设备 ID | 对端设备的唯一标识符 |
| brMac | std::string | 蓝牙 BR MAC | 经典蓝牙 MAC 地址格式 "XX:XX:XX:XX:XX:XX" |
| bleMac | std::string | 蓝牙 LE MAC | 低功耗蓝牙 MAC 地址 |
| wifiIp | std::string | Wi-Fi IP | IPv4 或 IPv6 地址 |
| wifiPort | uint16_t | Wi-Fi 端口 | 端口号范围 0-65535 |
| serviceId | int64_t | 服务 ID | 服务标识符，用于服务绑定场景 |

### 生命周期
- **创建时机**: 发起目标绑定时、服务发现时
- **销毁时机**: 绑定流程结束、进程退出
- **存储位置**: 内存中临时存储

### 使用场景
- `BindTarget()`: 绑定指定目标
- `UnbindTarget()`: 解绑指定目标
- `CreatePinHolder()`: 创建 PIN 持有者
- `DestroyPinHolder()`: 销毁 PIN 持有者

---

## 5. DmSubscribeInfo - 设备发现订阅信息

```cpp
typedef enum DmDiscoverMode {
    DM_DISCOVER_MODE_PASSIVE = 0x55,  // 被动发现模式
    DM_DISCOVER_MODE_ACTIVE = 0xAA     // 主动发现模式
} DmDiscoverMode;

typedef enum DmExchangeMedium {
    DM_AUTO = 0,   // 自动选择媒介
    DM_BLE = 1,    // 蓝牙
    DM_COAP = 2,   // Wi-Fi
    DM_USB = 3,    // USB
    DM_MEDIUM_BUTT
} DmExchangeMedium;

typedef enum DmExchangeFreq {
    DM_LOW = 0,           // 低频率
    DM_MID = 1,           // 中频率
    DM_HIGH = 2,          // 高频率
    DM_SUPER_HIGH = 3,    // 超高频率
    DM_EXTREME_HIGH = 4,  // 极高频率
    DM_FREQ_BUTT
} DmExchangeFreq;

typedef struct DmSubscribeInfo {
    uint16_t subscribeId;                           // 订阅 ID
    DmDiscoverMode mode;                            // 发现模式
    DmExchangeMedium medium;                        // 订阅媒介
    DmExchangeFreq freq;                            // 订阅频率
    bool isSameAccount;                             // 是否仅发现同账号设备
    bool isWakeRemote;                              // 是否唤醒远程设备
    char capability[DM_MAX_DEVICE_CAPABILITY_LEN];  // DM_MAX_DEVICE_CAPABILITY_LEN = 65
} DmSubscribeInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| subscribeId | uint16_t | 订阅 ID | 唯一标识一次订阅操作，范围 0-65535 |
| mode | DmDiscoverMode | 发现模式 | PASSIVE(0x55): 被动, ACTIVE(0xAA): 主动 |
| medium | DmExchangeMedium | 订阅媒介 | AUTO, BLE, COAP, USB |
| freq | DmExchangeFreq | 订阅频率 | LOW, MID, HIGH, SUPER_HIGH, EXTREME_HIGH |
| isSameAccount | bool | 同账号过滤 | true: 仅发现同账号设备, false: 发现所有设备 |
| isWakeRemote | bool | 唤醒远程 | true: 唤醒睡眠设备, false: 不唤醒 |
| capability | char[65] | 订阅能力 | 指定要发现的设备能力类型 |

### 生命周期
- **创建时机**: 调用 `StartDeviceDiscovery()` 时
- **销毁时机**: 调用 `StopDeviceDiscovery()` 后
- **存储位置**: 内存中存储，用于匹配发现的设备

### 使用场景
- `StartDeviceDiscovery()`: 启动设备发现
- `StopDeviceDiscovery()`: 停止设备发现
- `DiscoveryCallback::OnDeviceFound()`: 设备发现回调

---

## 6. DmPublishInfo - 设备发布信息

```cpp
typedef struct {
    int32_t publishId;        // 发布 ID
    DmDiscoverMode mode;      // 发现模式
    DmExchangeFreq freq;      // 发布频率
    bool ranging;             // 是否支持测距
    DmExchangeMedium medium;  // 发布媒介
} DmPublishInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| publishId | int32_t | 发布 ID | 唯一标识一次发布操作 |
| mode | DmDiscoverMode | 发现模式 | PASSIVE(0x55): 被动, ACTIVE(0xAA): 主动 |
| freq | DmExchangeFreq | 发布频率 | LOW, MID, HIGH, SUPER_HIGH, EXTREME_HIGH |
| ranging | bool | 测距功能 | true: 支持距离测量, false: 不支持 |
| medium | DmExchangeMedium | 发布媒介 | AUTO, BLE, COAP, USB |

### 生命周期
- **创建时机**: 调用 `PublishDeviceDiscovery()` 时
- **销毁时机**: 调用 `UnPublishDeviceDiscovery()` 后
- **存储位置**: 内存中存储，用于设备发布

### 使用场景
- `PublishDeviceDiscovery()`: 发布设备发现
- `UnPublishDeviceDiscovery()`: 取消发布设备发现
- `PublishCallback::OnPublishResult()`: 发布结果回调

---

## 7. DmDeviceType - 设备类型枚举

```cpp
typedef enum DmDeviceType {
    DEVICE_TYPE_UNKNOWN = 0x00,      // 未知设备
    DEVICE_TYPE_WIFI_CAMERA = 0x08,  // 智能摄像头
    DEVICE_TYPE_AUDIO = 0x0A,        // 智能音箱
    DEVICE_TYPE_PC = 0x0C,           // 个人电脑
    DEVICE_TYPE_PHONE = 0x0E,        // 智能手机
    DEVICE_TYPE_PAD = 0x11,          // 平板电脑
    DEVICE_TYPE_WATCH = 0x6D,        // 智能手表
    DEVICE_TYPE_CAR = 0x83,          // 车载设备
    DEVICE_TYPE_TV = 0x9C,           // 智能电视
    DEVICE_TYPE_SMART_DISPLAY = 0xA02,  // 智能显示屏
    DEVICE_TYPE_2IN1 = 0xA2F,        // 二合一设备
    DEVICE_TYPE_GLASSES = 0xA31,     // AR/VR 眼镜
    THIRD_TV = 0x2E,                 // 第三方电视
} DmDeviceType;
```

### 取值说明

| 设备类型 | 值 | 说明 |
|----------|-----|------|
| DEVICE_TYPE_UNKNOWN | 0x00 | 无法识别的设备类型 |
| DEVICE_TYPE_WIFI_CAMERA | 0x08 | 支持 Wi-Fi 连接的智能摄像头 |
| DEVICE_TYPE_AUDIO | 0x0A | 智能音箱、音频设备 |
| DEVICE_TYPE_PC | 0x0C | 个人电脑、笔记本 |
| DEVICE_TYPE_PHONE | 0x0E | 智能手机 |
| DEVICE_TYPE_PAD | 0x11 | 平板电脑 |
| DEVICE_TYPE_WATCH | 0x6D | 智能手表、手环 |
| DEVICE_TYPE_CAR | 0x83 | 车载设备、车机系统 |
| DEVICE_TYPE_TV | 0x9C | 智能电视 |
| DEVICE_TYPE_SMART_DISPLAY | 0xA02 | 智能显示屏 |
| DEVICE_TYPE_2IN1 | 0xA2F | 二合一设备（平板+键盘） |
| DEVICE_TYPE_GLASSES | 0xA31 | AR/VR 眼镜 |
| THIRD_TV | 0x2E | 第三方电视设备 |

---

## 8. DmDeviceState - 设备状态枚举

```cpp
typedef enum DmDeviceState {
    DEVICE_STATE_UNKNOWN = -1,   // 未知状态
    DEVICE_STATE_ONLINE = 0,     // 设备在线（物理在线）
    DEVICE_INFO_READY = 1,       // 设备就绪（信息已同步）
    DEVICE_STATE_OFFLINE = 2,    // 设备离线
    DEVICE_INFO_CHANGED = 3,     // 设备信息变更
} DmDeviceState;
```

### 状态说明

| 状态 | 值 | 说明 |
|------|-----|------|
| DEVICE_STATE_UNKNOWN | -1 | 设备状态未知 |
| DEVICE_STATE_ONLINE | 0 | 设备物理上线，但分布式数据服务（DDS）信息尚未同步 |
| DEVICE_INFO_READY | 1 | 设备信息已在 DDS 模块同步完成，可运行分布式服务 |
| DEVICE_STATE_OFFLINE | 2 | 设备物理下线 |
| DEVICE_INFO_CHANGED | 3 | 设备信息发生变化 |

### 状态转换
```
                    ┌─────────────┐
                    │   UNKNOWN   │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   ONLINE    │ ◄────┐
                    └──────┬──────┘      │
                           │             │
                    ┌──────▼──────┐      │
                    │    READY    │ ─────┤
                    └──────┬──────┘      │
                           │             │
                    ┌──────▼──────┐      │
                    │   OFFLINE   │ ─────┘
                    └─────────────┘
```

### 使用场景
- `DeviceStateCallback::OnDeviceOnline()`: 设备上线回调
- `DeviceStateCallback::OnDeviceReady()`: 设备就绪回调
- `DeviceStateCallback::OnDeviceOffline()`: 设备下线回调
- `DeviceStateCallback::OnDeviceChanged()`: 设备变化回调

---

## 9. DmAuthForm - 认证形式枚举

```cpp
typedef enum DmAuthForm {
    INVALID_TYPE = -1,        // 无效类型
    PEER_TO_PEER = 0,         // 点对点认证
    IDENTICAL_ACCOUNT = 1,    // 同账号认证
    ACROSS_ACCOUNT = 2,       // 跨账号认证
    SHARE = 3,                // 分享认证
} DmAuthForm;
```

### 形式说明

| 认证形式 | 值 | 说明 |
|----------|-----|------|
| INVALID_TYPE | -1 | 无效的认证形式 |
| PEER_TO_PEER | 0 | 点对点认证，两个设备直接建立信任关系 |
| IDENTICAL_ACCOUNT | 1 | 同账号认证，设备属于同一华为账号 |
| ACROSS_ACCOUNT | 2 | 跨账号认证，设备属于不同华为账号 |
| SHARE | 3 | 分享认证，通过分享方式建立的认证关系 |

---

## 10. DmAuthType - 认证类型枚举

```cpp
typedef enum DmAuthType {
    AUTH_TYPE_CRE = 0,            // 凭证认证
    AUTH_TYPE_PIN,                // PIN 码认证
    AUTH_TYPE_QR_CODE,            // 二维码认证
    AUTH_TYPE_NFC,                // NFC 认证
    AUTH_TYPE_NO_INTER_ACTION,    // 无感认证
    AUTH_TYPE_IMPORT_AUTH_CODE,   // 导入认证码
    AUTH_TYPE_UNKNOW,             // 未知认证类型
} DmAuthType;
```

### 类型说明

| 认证类型 | 值 | 说明 |
|----------|-----|------|
| AUTH_TYPE_CRE | 0 | 基于凭证的认证 |
| AUTH_TYPE_PIN | 1 | 基于 PIN 码的认证 |
| AUTH_TYPE_QR_CODE | 2 | 基于二维码的认证 |
| AUTH_TYPE_NFC | 3 | 基于 NFC 的认证 |
| AUTH_TYPE_NO_INTER_ACTION | 4 | 无感认证（自动认证） |
| AUTH_TYPE_IMPORT_AUTH_CODE | 5 | 通过导入认证码进行认证 |
| AUTH_TYPE_UNKNOW | 6 | 未知认证类型 |

---

## 11. DmPinType - PIN 码类型枚举

```cpp
typedef enum {
    NUMBER_PIN_CODE = 0,  // 数字 PIN 码
    QR_CODE,              // 二维码
    VISIBLE_LLIGHT,       // 可见光
    SUPER_SONIC,          // 超声波
} DmPinType;
```

### 类型说明

| PIN 类型 | 值 | 说明 |
|----------|-----|------|
| NUMBER_PIN_CODE | 0 | 数字形式的 PIN 码 |
| QR_CODE | 1 | 二维码形式的 PIN 码 |
| VISIBLE_LLIGHT | 2 | 可见光通信 |
| SUPER_SONIC | 3 | 超声波通信 |

---

## 12. DmNetworkType - 网络类型枚举

```cpp
typedef enum {
    BIT_NETWORK_TYPE_UNKNOWN = 0,  // 未知网络
    BIT_NETWORK_TYPE_WIFI,         // Wi-Fi 网络
    BIT_NETWORK_TYPE_BLE,          // 蓝牙低功耗网络
    BIT_NETWORK_TYPE_BR,           // 蓝牙经典网络
    BIT_NETWORK_TYPE_P2P,          // P2P 网络
    BIT_NETWORK_TYPE_USB = 8,      // USB 网络
    BIT_NETWORK_TYPE_COUNT,        // 网络类型数量
} DmNetworkType;
```

---

## 13. DmAccessCaller - 访问控制调用者信息

```cpp
typedef struct DmAccessCaller {
    std::string accountId;   // 账号 ID
    std::string pkgName;     // 包名
    std::string networkId;   // 网络 ID
    int32_t userId;          // 用户 ID
    uint64_t tokenId = 0;    // 令牌 ID
    std::string extra;       // 扩展信息
} DmAccessCaller;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| accountId | std::string | 账号 ID | 华为账号唯一标识符 |
| pkgName | std::string | 包名 | 调用者应用包名 |
| networkId | std::string | 网络 ID | 调用者所在设备的网络 ID |
| userId | int32_t | 用户 ID | 系统用户 ID |
| tokenId | uint64_t | 令牌 ID | 访问令牌标识符 |
| extra | std::string | 扩展信息 | 额外的调用者信息 |

### 使用场景
- `CheckAccessControl()`: 检查访问权限
- `CheckIsSameAccount()`: 检查是否同账号

---

## 14. DmAccessCallee - 访问控制被调用者信息

```cpp
typedef struct DmAccessCallee {
    std::string accountId;   // 账号 ID
    std::string networkId;   // 网络 ID
    std::string peerId;      // 对端 ID
    std::string pkgName;     // 包名
    int32_t userId;          // 用户 ID
    uint64_t tokenId = 0;    // 令牌 ID
    std::string extra;       // 扩展信息
} DmAccessCallee;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| accountId | std::string | 账号 ID | 被访问设备的账号 ID |
| networkId | std::string | 网络 ID | 被访问设备的网络 ID |
| peerId | std::string | 对端 ID | 被访问设备的对端标识符 |
| pkgName | std::string | 包名 | 被访问应用的包名 |
| userId | int32_t | 用户 ID | 被访问设备的用户 ID |
| tokenId | uint64_t | 令牌 ID | 被访问应用的访问令牌 |
| extra | std::string | 扩展信息 | 额外的被调用者信息 |

---

## 15. ProcessInfo - 进程信息结构

```cpp
typedef struct ProcessInfo {
    int32_t userId;         // 用户 ID
    std::string pkgName;    // 包名
    uint32_t tokenId = 0;   // 令牌 ID
} ProcessInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| userId | int32_t | 用户 ID | 系统用户 ID，多用户场景下区分不同用户 |
| pkgName | std::string | 包名 | 应用进程的包名 |
| tokenId | uint32_t | 令牌 ID | 进程访问令牌标识符 |

### 使用场景
- 标识发起 DeviceManager 操作的调用进程
- 权限验证和访问控制
- 多用户隔离

---

## 16. DmAuthInfo - 认证信息结构

```cpp
enum class DMLocalServiceInfoAuthType : int32_t {
    TRUST_ONETIME = 0,  // 仅本次信任
    TRUST_ALWAYS = 6,   // 总是信任
    CANCEL = 1,         // 取消
    MAX = 7
};

enum class DMLocalServiceInfoPinExchangeType : int32_t {
    PINBOX = 1,             // PIN 盒（从内存获取 PIN）
    QR_FROMDP = 2,          // 二维码（从设备_profile 获取）
    FROMDP = 3,             // NFC（从设备_profile 获取）
    IMPORT_AUTH_CODE = 5,   // 导入认证码
    ULTRASOUND = 6,         // 超声波
    MAX = 7
};

enum class DMLocalServiceInfoAuthBoxType : int32_t {
    STATE3 = 1,        // 三步确认
    SKIP_CONFIRM = 2,  // 跳过确认
    TWO_IN1 = 3,       // 二合一设备
    MAX = 4
};

typedef struct DmAuthInfo {
    int32_t userId = 0;                             // 用户 ID
    uint64_t pinConsumerTokenId = 0;                // PIN 消费者令牌 ID
    char pinCode[DM_MAX_PIN_CODE_LEN] = {0};        // DM_MAX_PIN_CODE_LEN = 1025
    char metaToken[DM_MAX_META_TOKEN_LEN] = {0};    // DM_MAX_META_TOKEN_LEN = 9
    std::string pinConsumerPkgName;                 // PIN 消费者包名
    std::string bizSrcPkgName;                      // 业务源包名
    std::string bizSinkPkgName;                     // 业务目标包名
    DMLocalServiceInfoAuthType authType;            // 信任类型
    DMLocalServiceInfoAuthBoxType authBoxType;      // 认证框类型
    DMLocalServiceInfoPinExchangeType pinExchangeType; // PIN 交换类型
    std::string description;                        // 描述信息
    std::string extraInfo;                          // 扩展信息
    std::string regPkgName;                         // 注册包名
} DmAuthInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| userId | int32_t | 用户 ID | 当前调用者的用户 ID |
| pinConsumerTokenId | uint64_t | PIN 消费者令牌 | 当前调用者使用的 PIN 对应的令牌 ID |
| pinCode | char[1025] | PIN 码 | 认证使用的 PIN 码 |
| metaToken | char[9] | 元令牌 | 双设备认证使用的元令牌 |
| pinConsumerPkgName | std::string | PIN 消费者包名 | 当前调用者使用的 PIN 对应的包名 |
| bizSrcPkgName | std::string | 业务源包名 | 用于双设备协同（代理入口/出口场景） |
| bizSinkPkgName | std::string | 业务目标包名 | 代理对应的目标包名 |
| authType | DMLocalServiceInfoAuthType | 信任类型 | 0: 仅本次, 6: 总是, 1: 取消 |
| authBoxType | DMLocalServiceInfoAuthBoxType | 认证框类型 | 1: 确认, 2: 跳过确认, 3: 二合一 |
| pinExchangeType | DMLocalServiceInfoPinExchangeType | PIN 交换类型 | 1: PIN 盒, 2: 二维码, 3: NFC, 5: 导入, 6: 超声波 |
| description | std::string | 描述信息 | 三方接口上的描述信息 |
| extraInfo | std::string | 扩展信息 | 扩展配置信息 |
| regPkgName | std::string | 注册包名 | 当前注册服务的包名 |

---

## 17. DmServiceInfo - 服务信息结构

```cpp
typedef struct DmServiceInfo {
    int32_t userId = -1;                     // 用户 ID
    int64_t serviceId = -1;                  // 服务 ID
    int64_t displayId = -1;                  // 显示 ID
    int8_t publishState = -1;                // 发布状态
    std::string deviceId = "";               // 设备 ID
    std::string networkId = "";              // 网络 ID
    DmAuthForm authform;                     // 认证形式
    uint64_t serviceOwnerTokenId = 0;        // 服务拥有者令牌 ID
    std::string serviceOwnerPkgName = "";    // 服务拥有者包名
    uint64_t serviceRegisterTokenId = 0;     // 服务注册令牌 ID
    std::string serviceType = "";            // 服务类型
    std::string serviceName = "";            // 服务名称
    std::string serviceDisplayName = "";     // 服务显示名称
    std::string serviceCode = "";            // 服务代码
    std::string customData = "";             // 自定义数据
    uint32_t dataLen = 0;                    // 数据长度
    int64_t timeStamp = -1;                  // 时间戳
    std::string description = "";            // 描述信息
} DmServiceInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| userId | int32_t | 用户 ID | 服务所属用户 ID |
| serviceId | int64_t | 服务 ID | 服务唯一标识符 |
| displayId | int64_t | 显示 ID | 显示屏 ID |
| publishState | int8_t | 发布状态 | 0: 未发布, 1: 已发布 |
| deviceId | std::string | 设备 ID | 服务所在设备 ID |
| networkId | std::string | 网络 ID | 服务所在设备网络 ID |
| authform | DmAuthForm | 认证形式 | 服务认证形式 |
| serviceOwnerTokenId | uint64_t | 服务拥有者令牌 | 服务拥有者的访问令牌 |
| serviceOwnerPkgName | std::string | 服务拥有者包名 | 服务拥有者的包名 |
| serviceRegisterTokenId | uint64_t | 服务注册令牌 | 服务注册时的令牌 |
| serviceType | std::string | 服务类型 | 服务类型标识 |
| serviceName | std::string | 服务名称 | 服务名称 |
| serviceDisplayName | std::string | 服务显示名称 | 服务显示的友好名称 |
| serviceCode | std::string | 服务代码 | 服务代码 |
| customData | std::string | 自定义数据 | 自定义业务数据 |
| dataLen | uint32_t | 数据长度 | 自定义数据长度 |
| timeStamp | int64_t | 时间戳 | 服务注册/更新时间戳 |
| description | std::string | 描述信息 | 服务描述 |

### 使用场景
- `RegisterServiceInfo()`: 注册服务信息
- `UnRegisterServiceInfo()`: 注销服务信息
- `StartPublishService()`: 启动服务发布
- `StopPublishService()`: 停止服务发布
- `ServiceInfoStateCallback`: 服务状态回调

---

## 18. ServiceInfo - 基础服务信息结构

```cpp
typedef struct ServiceInfo {
    int64_t serviceId = 0;               // 服务 ID
    std::string serviceType;             // 服务类型
    std::string serviceName;             // 服务名称
    std::string serviceDisplayName;      // 服务显示名称
} ServiceInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| serviceId | int64_t | 服务 ID | 服务唯一标识符 |
| serviceType | std::string | 服务类型 | 服务类型字符串 |
| serviceName | std::string | 服务名称 | 服务名称 |
| serviceDisplayName | std::string | 服务显示名称 | 服务显示的友好名称 |

---

## 19. DmDeviceProfileInfo - 设备配置信息结构

```cpp
typedef struct DmDeviceProfileInfo {
    std::string deviceId = "";              // 设备 ID
    std::string deviceSn = "";              // 设备序列号
    std::string mac = "";                   // MAC 地址
    std::string model = "";                 // 设备型号
    std::string internalModel = "";         // 内部型号
    std::string deviceType = "";            // 设备类型
    std::string manufacturer = "";          // 制造商
    std::string deviceName = "";            // 设备名称
    std::string productName = "";           // 产品名称
    std::string productId = "";             // 产品 ID
    std::string subProductId = "";          // 子产品 ID
    std::string sdkVersion = "";            // SDK 版本
    std::string bleMac = "";                // 蓝牙 LE MAC 地址
    std::string brMac = "";                 // 蓝牙 BR MAC 地址
    std::string sleMac = "";                // SLE MAC 地址
    std::string firmwareVersion = "";       // 固件版本
    std::string hardwareVersion = "";       // 硬件版本
    std::string softwareVersion = "";       // 软件版本
    int32_t protocolType = 0;               // 协议类型
    int32_t setupType = 0;                  // 配置类型
    std::string wiseDeviceId = "";          // Wise 设备 ID
    std::string wiseUserId = "";            // Wise 用户 ID
    std::string registerTime = "";          // 注册时间
    std::string modifyTime = "";            // 修改时间
    std::string shareTime = "";             // 分享时间
    bool isLocalDevice = false;             // 是否本地设备
    std::vector<DmServiceProfileInfo> services = {};  // 服务列表
} DmDeviceProfileInfo;

typedef struct DmServiceProfileInfo {
    std::string deviceId = "";      // 设备 ID
    std::string serviceId = "";     // 服务 ID
    std::string serviceType = "";   // 服务类型
    std::map<std::string, std::string> data = {};  // 服务数据
} DmServiceProfileInfo;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| deviceId | std::string | 设备 ID | 设备唯一标识符 |
| deviceSn | std::string | 设备序列号 | 制造商分配的序列号 |
| mac | std::string | MAC 地址 | 主要网络接口 MAC 地址 |
| model | std::string | 设备型号 | 市场型号名称 |
| internalModel | std::string | 内部型号 | 内部型号代号 |
| deviceType | std::string | 设备类型 | 设备类型字符串 |
| manufacturer | std::string | 制造商 | 设备制造商 |
| deviceName | std::string | 设备名称 | 用户设置的设备名称 |
| productName | std::string | 产品名称 | 产品名称 |
| productId | std::string | 产品 ID | 产品标识符 |
| subProductId | std::string | 子产品 ID | 子产品标识符 |
| sdkVersion | std::string | SDK 版本 | 设备 SDK 版本 |
| bleMac | std::string | 蓝牙 LE MAC | 低功耗蓝牙 MAC 地址 |
| brMac | std::string | 蓝牙 BR MAC | 经典蓝牙 MAC 地址 |
| sleMac | std::string | SLE MAC | SLE（低功耗音频）MAC 地址 |
| firmwareVersion | std::string | 固件版本 | 设备固件版本 |
| hardwareVersion | std::string | 硬件版本 | 设备硬件版本 |
| softwareVersion | std::string | 软件版本 | 设备软件版本 |
| protocolType | int32_t | 协议类型 | 设备支持的协议类型 |
| setupType | int32_t | 配置类型 | 设备配置类型 |
| wiseDeviceId | std::string | Wise 设备 ID | Wise 平台设备 ID |
| wiseUserId | std::string | Wise 用户 ID | Wise 平台用户 ID |
| registerTime | std::string | 注册时间 | 设备注册时间戳 |
| modifyTime | std::string | 修改时间 | 信息最后修改时间 |
| shareTime | std::string | 分享时间 | 设备分享时间 |
| isLocalDevice | bool | 是否本地设备 | true: 本地设备, false: 远程设备 |
| services | vector | 服务列表 | 设备提供的服务列表 |

### 使用场景
- `GetDeviceProfileInfoList()`: 获取设备配置信息列表
- `PutDeviceProfileInfoList()`: 上报设备配置信息列表

---

## 20. DmAppImageInfo - 应用图像信息类

```cpp
class DmAppImageInfo {
public:
    DmAppImageInfo();
    explicit DmAppImageInfo(uint8_t *appIcon, int32_t appIconLen, 
                           uint8_t *appThumbnail, int32_t appThumbnailLen);
    
    int32_t GetAppIconLen() const;
    const uint8_t *GetAppIcon() const;
    int32_t GetAppThumbnailLen() const;
    const uint8_t *GetAppThumbnail() const;

private:
    int32_t appIconLen_ { 0 };              // 应用图标长度
    uint8_t *appIcon_ { nullptr };          // 应用图标数据
    int32_t appThumbnailLen_ { 0 };         // 应用缩略图长度
    uint8_t *appThumbnail_ { nullptr };     // 应用缩略图数据
    const int32_t ICON_MAX_LEN = 32 * 1024;    // 图标最大长度 32KB
    const int32_t THUMB_MAX_LEN = 153 * 1024;  // 缩略图最大长度 153KB
};
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| appIconLen_ | int32_t | 应用图标长度 | 最大 32KB (32 * 1024 字节) |
| appIcon_ | uint8_t* | 应用图标数据 | 图像二进制数据 |
| appThumbnailLen_ | int32_t | 应用缩略图长度 | 最大 153KB (153 * 1024 字节) |
| appThumbnail_ | uint8_t* | 应用缩略图数据 | 缩略图二进制数据 |

### 生命周期
- **创建时机**: 构造时传入图标和缩略图数据
- **销毁时机**: 析构函数自动释放内存
- **存储位置**: 堆内存，由类管理生命周期

### 使用场景
- `DmAuthParam`: 认证参数中携带应用图标信息
- UI 展示认证应用时显示图标和缩略图

---

## 21. CredentialData - 凭证数据结构

```cpp
typedef struct {
    int32_t credentialType;        // 凭证类型
    std::string credentialId;      // 凭证 ID
    std::string serverPk;          // 服务器公钥
    std::string pkInfoSignature;   // 公钥信息签名
    std::string pkInfo;            // 公钥信息
    std::string authCode;          // 认证码
    std::string peerDeviceId;      // 对端设备 ID
} CredentialData;
```

### 字段说明

| 字段 | 类型 | 含义 | 取值范围/说明 |
|------|------|------|--------------|
| credentialType | int32_t | 凭证类型 | 凭证类型标识符 |
| credentialId | std::string | 凭证 ID | 凭证唯一标识符 |
| serverPk | std::string | 服务器公钥 | Base64 编码的公钥 |
| pkInfoSignature | std::string | 公钥信息签名 | 公钥信息的数字签名 |
| pkInfo | std::string | 公钥信息 | 公钥相关信息 |
| authCode | std::string | 认证码 | 用于设备认证的授权码 |
| peerDeviceId | std::string | 对端设备 ID | 对端设备的 UDID |

---

## 22. AuthState - 认证状态枚举

```cpp
typedef enum AuthState {
    AUTH_REQUEST_INIT = 1,              // 认证请求初始化
    AUTH_REQUEST_NEGOTIATE,             // 认证请求协商
    AUTH_REQUEST_NEGOTIATE_DONE,        // 认证请求协商完成
    AUTH_REQUEST_REPLY,                 // 认证请求回复
    AUTH_REQUEST_JOIN,                  // 认证请求加入
    AUTH_REQUEST_NETWORK,               // 认证请求组网
    AUTH_REQUEST_FINISH,                // 认证请求完成
    AUTH_REQUEST_CREDENTIAL,            // 认证请求凭证
    AUTH_REQUEST_CREDENTIAL_DONE,       // 认证请求凭证完成
    AUTH_REQUEST_AUTH_FINISH,           // 认证请求认证完成
    AUTH_REQUEST_RECHECK_MSG,           // 认证请求重检消息
    AUTH_REQUEST_RECHECK_MSG_DONE,      // 认证请求重检消息完成
    
    AUTH_RESPONSE_INIT = 20,            // 认证响应初始化
    AUTH_RESPONSE_NEGOTIATE,            // 认证响应协商
    AUTH_RESPONSE_CONFIRM,              // 认证响应确认
    AUTH_RESPONSE_GROUP,                // 认证响应组
    AUTH_RESPONSE_SHOW,                 // 认证响应显示
    AUTH_RESPONSE_FINISH,               // 认证响应完成
    AUTH_RESPONSE_CREDENTIAL,           // 认证响应凭证
    AUTH_RESPONSE_AUTH_FINISH,          // 认证响应认证完成
    AUTH_RESPONSE_RECHECK_MSG,          // 认证响应重检消息
} AuthState;
```

### 状态说明

**认证请求方状态 (1-19)**:
| 状态 | 值 | 说明 |
|------|-----|------|
| AUTH_REQUEST_INIT | 1 | 初始化认证请求 |
| AUTH_REQUEST_NEGOTIATE | 2 | 协商认证参数 |
| AUTH_REQUEST_NEGOTIATE_DONE | 3 | 协商完成 |
| AUTH_REQUEST_REPLY | 4 | 等待对方回复 |
| AUTH_REQUEST_JOIN | 5 | 加入设备组 |
| AUTH_REQUEST_NETWORK | 6 | 建立网络连接 |
| AUTH_REQUEST_FINISH | 7 | 认证完成 |
| AUTH_REQUEST_CREDENTIAL | 8 | 处理凭证 |
| AUTH_REQUEST_CREDENTIAL_DONE | 9 | 凭证处理完成 |
| AUTH_REQUEST_AUTH_FINISH | 10 | 认证流程结束 |
| AUTH_REQUEST_RECHECK_MSG | 11 | 重新检查消息 |
| AUTH_REQUEST_RECHECK_MSG_DONE | 12 | 消息重检完成 |

**认证响应方状态 (20-28)**:
| 状态 | 值 | 说明 |
|------|-----|------|
| AUTH_RESPONSE_INIT | 20 | 初始化认证响应 |
| AUTH_RESPONSE_NEGOTIATE | 21 | 协商认证参数 |
| AUTH_RESPONSE_CONFIRM | 22 | 确认认证 |
| AUTH_RESPONSE_GROUP | 23 | 创建设备组 |
| AUTH_RESPONSE_SHOW | 24 | 显示认证 UI |
| AUTH_RESPONSE_FINISH | 25 | 认证完成 |
| AUTH_RESPONSE_CREDENTIAL | 26 | 处理凭证 |
| AUTH_RESPONSE_AUTH_FINISH | 27 | 认证流程结束 |
| AUTH_RESPONSE_RECHECK_MSG | 28 | 重新检查消息 |

---

## 23. DmMsgType - 消息类型枚举

```cpp
enum DmMsgType : int32_t {
    MSG_TYPE_UNKNOWN = 0,                    // 未知消息
    MSG_TYPE_NEGOTIATE = 80,                 // 协商消息
    MSG_TYPE_RESP_NEGOTIATE = 90,            // 协商响应
    MSG_TYPE_REQ_AUTH = 100,                 // 认证请求
    MSG_TYPE_INVITE_AUTH_INFO = 102,         // 邀请认证信息
    MSG_TYPE_REQ_AUTH_TERMINATE = 104,       // 认证终止请求
    MSG_TYPE_RESP_AUTH = 200,                // 认证响应
    MSG_TYPE_JOIN_AUTH_INFO = 201,           // 加入认证信息
    MSG_TYPE_RESP_AUTH_TERMINATE = 205,      // 认证终止响应
    MSG_TYPE_CHANNEL_CLOSED = 300,           // 通道关闭
    MSG_TYPE_SYNC_GROUP = 400,               // 同步组信息
    MSG_TYPE_AUTH_BY_PIN = 500,              // PIN 认证
    MSG_TYPE_REQ_AUTH_DEVICE_NEGOTIATE = 600,  // 设备认证协商请求
    MSG_TYPE_RESP_AUTH_DEVICE_NEGOTIATE = 700, // 设备认证协商响应
} DmMsgType;
```

---

## 24. DmAuthRequestContext - 认证请求上下文

```cpp
typedef struct DmAuthRequestContext {
    int32_t authType;                  // 认证类型
    std::string localDeviceId;         // 本地设备 ID
    std::string localDeviceName;       // 本地设备名称
    int32_t localDeviceTypeId;         // 本地设备类型 ID
    std::string deviceId;              // 对端设备 ID
    std::string deviceName;            // 对端设备名称
    std::string deviceTypeId;          // 对端设备类型 ID
    int32_t sessionId;                 // 会话 ID
    int32_t groupVisibility;           // 组可见性
    bool cryptoSupport;                // 是否支持加密
    std::string cryptoName;            // 加密算法名称
    std::string cryptoVer;             // 加密算法版本
    std::string hostPkgName;           // 主机包名
    std::string targetPkgName;         // 目标包名
    std::string peerBundleName;        // 对端包名
    std::string bundleName;            // 包名
    std::string appOperation;          // 应用操作
    std::string appDesc;               // 应用描述
    std::string appName;               // 应用名称
    std::string customDesc;            // 自定义描述
    std::string appThumbnail;          // 应用缩略图
    std::string token;                 // 令牌
    int32_t reason;                    // 原因
    std::vector<std::string> syncGroupList;  // 同步组列表
    std::string dmVersion;             // DeviceManager 版本
    int32_t localUserId;               // 本地用户 ID
    std::string localAccountId;        // 本地账号 ID
    std::vector<int32_t> bindType;     // 绑定类型列表
    bool isOnline;                     // 是否在线
    bool authed;                       // 是否已认证
    int32_t bindLevel;                 // 绑定级别
    int64_t tokenId;                   // 令牌 ID
    std::string remoteAccountId;       // 远程账号 ID
    int32_t remoteUserId;              // 远程用户 ID
    std::string addr;                  // 地址
    std::string hostPkgLabel;          // 主机包标签
    int32_t closeSessionDelaySeconds = 0;  // 关闭会话延迟秒数
    std::string remoteDeviceName;      // 远程设备名称
    std::string connSessionType;       // 连接会话类型
    int32_t hmlActionId = 0;           // HML 动作 ID
    bool hmlEnable160M = false;        // HML 是否启用 160M
} DmAuthRequestContext;
```

### 字段说明（关键字段）

| 字段 | 类型 | 含义 | 说明 |
|------|------|------|------|
| authType | int32_t | 认证类型 | 参见 DmAuthType 枚举 |
| localDeviceId | std::string | 本地设备 ID | 发起认证的设备 UDID |
| deviceId | std::string | 对端设备 ID | 被认证设备的 UDID |
| sessionId | int32_t | 会话 ID | Softbus 会话标识符 |
| cryptoSupport | bool | 加密支持 | 是否支持端到端加密 |
| hostPkgName | std::string | 主机包名 | 发起认证的应用包名 |
| bundleName | std::string | 包名 | 当前应用的包名 |
| token | std::string | 令牌 | 认证会话令牌 |
| bindType | vector<int32_t> | 绑定类型 | 支持的绑定类型列表 |
| isOnline | bool | 是否在线 | 对端设备是否在线 |
| bindLevel | int32_t | 绑定级别 | 绑定级别标识 |
| tokenId | int64_t | 令牌 ID | 访问令牌 ID |

### 生命周期
- **创建时机**: 发起认证请求时
- **销毁时机**: 认证流程结束
- **存储位置**: 认证管理器内存中

---

## 25. DmAuthResponseContext - 认证响应上下文

```cpp
typedef struct DmAuthResponseContext {
    int32_t authType;                  // 认证类型
    std::string deviceId;              // 对端设备 ID
    std::string localDeviceId;         // 本地设备 ID
    std::string deviceName;            // 设备名称
    int32_t deviceTypeId;              // 设备类型 ID
    int32_t msgType;                   // 消息类型
    int32_t sessionId;                 // 会话 ID
    bool cryptoSupport;                // 是否支持加密
    bool isIdenticalAccount;           // 是否同账号
    bool isAuthCodeReady;              // 认证码是否就绪
    bool isShowDialog;                 // 是否显示对话框
    std::string cryptoName;            // 加密算法名称
    std::string cryptoVer;             // 加密算法版本
    int32_t reply;                     // 回复
    std::string networkId;             // 网络 ID
    std::string groupId;               // 组 ID
    std::string groupName;             // 组名称
    std::string hostPkgName;           // 主机包名
    std::string targetPkgName;         // 目标包名
    std::string bundleName;            // 包名
    std::string peerBundleName;        // 对端包名
    std::string appOperation;          // 应用操作
    std::string appDesc;               // 应用描述
    std::string customDesc;            // 自定义描述
    std::string appIcon;               // 应用图标
    std::string appThumbnail;          // 应用缩略图
    std::string token;                 // 令牌
    std::string authToken;             // 认证令牌
    int32_t pageId;                    // 页面 ID
    int64_t requestId;                 // 请求 ID
    std::string code = "";             // 代码
    int32_t state;                     // 状态
    std::vector<std::string> syncGroupList;  // 同步组列表
    std::string accountGroupIdHash;    // 账号组 ID 哈希
    std::string publicKey;             // 公钥
    bool isOnline;                     // 是否在线
    int32_t bindLevel;                 // 绑定级别
    bool haveCredential;               // 是否有凭证
    int32_t confirmOperation;          // 确认操作
    std::string localAccountId;        // 本地账号 ID
    int32_t localUserId;               // 本地用户 ID
    int64_t tokenId;                   // 令牌 ID
    int64_t remoteTokenId;             // 远程令牌 ID
    bool authed;                       // 是否已认证
    std::string dmVersion;             // DeviceManager 版本
    std::vector<int32_t> bindType;     // 绑定类型列表
    std::string remoteAccountId;       // 远程账号 ID
    int32_t remoteUserId;              // 远程用户 ID
    std::string targetDeviceName;      // 目标设备名称
    std::string importAuthCode;        // 导入的认证码
    std::string hostPkgLabel;          // 主机包标签
    bool isFinish = false;             // 是否完成
    std::string edition;               // 版本
    int32_t localBindLevel;            // 本地绑定级别
    std::string remoteDeviceName;      // 远程设备名称
    int32_t localSessionKeyId = 0;     // 本地会话密钥 ID
    int32_t remoteSessionKeyId = 0;    // 远程会话密钥 ID
} DmAuthResponseContext;
```

### 字段说明（关键字段）

| 字段 | 类型 | 含义 | 说明 |
|------|------|------|------|
| authType | int32_t | 认证类型 | 参见 DmAuthType 枚举 |
| deviceId | std::string | 对端设备 ID | 认证发起方的设备 ID |
| localDeviceId | std::string | 本地设备 ID | 认证响应方的设备 ID |
| msgType | int32_t | 消息类型 | 参见 DmMsgType 枚举 |
| isIdenticalAccount | bool | 是否同账号 | 双方是否属于同一账号 |
| isAuthCodeReady | bool | 认证码就绪 | 认证码是否准备就绪 |
| isShowDialog | bool | 显示对话框 | 是否需要显示确认对话框 |
| reply | int32_t | 回复 | 用户确认结果 |
| networkId | std::string | 网络 ID | 设备网络 ID |
| groupId | std::string | 组 ID | 认证组 ID |
| requestId | int64_t | 请求 ID | HiChain 请求 ID |
| bindLevel | int32_t | 绑定级别 | 设备绑定级别 |
| confirmOperation | int32_t | 确认操作 | 用户确认操作类型 |
| isOnline | bool | 是否在线 | 对端设备是否在线 |
| isFinish | bool | 是否完成 | 认证是否完成 |

### 生命周期
- **创建时机**: 接收认证请求时
- **销毁时机**: 认证响应完成
- **存储位置**: 认证管理器内存中

---

## 26. DmServiceState - 服务状态枚举

```cpp
typedef enum DmServiceState {
    SERVICE_STATE_UNKNOWN = -1,     // 服务状态未知
    SERVICE_STATE_ONLINE = 0,       // 服务在线
    SERVICE_STATE_OFFLINE = 2,      // 服务离线
    SERVICE_INFO_CHANGED = 3,       // 服务信息变更
} DmServiceState;
```

---

## 27. DMPublishState - 发布状态枚举

```cpp
typedef enum DMPublishState {
    SERVICE_UNPUBLISHED_STATE = 0,  // 服务未发布
    SERVICE_PUBLISHED_STATE,        // 服务已发布
} DMPublishState;
```

---

## 28. 常量定义

### 设备 ID 长度
```cpp
#define DM_MAX_DEVICE_ID_LEN (97)        // 设备 ID 最大长度
#define DM_MAX_DEVICE_NAME_LEN (129)     // 设备名称最大长度
#define DM_MAX_PIN_CODE_LEN (1025)       // PIN 码最大长度
#define DM_MAX_META_TOKEN_LEN (9)        // 元令牌最大长度
#define DM_MAX_DEVICE_CAPABILITY_LEN (65) // 设备能力最大长度
```

### 组类型
```cpp
const int32_t GROUP_TYPE_INVALID_GROUP = -1;         // 无效组
const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 0; // 同账号组
const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 1;     // 点对点组
const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 2;   // 跨账号组
```

### 组可见性
```cpp
const int32_t GROUP_VISIBILITY_PUBLIC = 0;  // 公开组
```

---

## 总结

本文档涵盖了 OpenHarmony DeviceManager 子系统的核心数据结构，包括：

1. **设备信息相关**: DmDeviceInfo, DmDeviceBasicInfo, DmDeviceType, DmDeviceState
2. **认证相关**: DmAuthParam, DmAuthInfo, DmAuthForm, DmAuthType, DmPinType, AuthState, DmAuthRequestContext, DmAuthResponseContext
3. **发现/发布相关**: DmSubscribeInfo, DmPublishInfo, DmDiscoverMode, DmExchangeMedium, DmExchangeFreq
4. **访问控制相关**: DmAccessCaller, DmAccessCallee, ProcessInfo
5. **服务相关**: DmServiceInfo, ServiceInfo, DmServiceState, DMPublishState
6. **配置相关**: DmDeviceProfileInfo, DmDeviceIconInfo
7. **其他**: PeerTargetId, DmAppImageInfo, CredentialData, DmNetworkType

这些数据结构在 DeviceManager 的设备发现、认证、绑定、访问控制和服务管理等核心功能中发挥关键作用。正确理解和使用这些数据结构对于开发分布式设备管理应用至关重要。