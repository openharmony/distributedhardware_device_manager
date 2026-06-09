# OpenHarmony DeviceManager 核心概念与术语

> **版本**: v2.0
> **日期**: 2026-05-19

---

## 1. 信任关系类型 (DmAuthForm)

OpenHarmony DeviceManager 支持多种设备间信任关系，通过枚举 `DmAuthForm` 定义：

```cpp
typedef enum DmAuthForm {
    INVALID_TYPE = -1,      // 无效类型
    PEER_TO_PEER = 0,       // 点对点信任
    IDENTICAL_ACCOUNT = 1,  // 同账号信任
    ACROSS_ACCOUNT = 2,     // 跨账号信任
    SHARE = 3,              // 分享信任
} DmAuthForm;
```

### 1.1 PEER_TO_PEER (点对点信任)

**业务含义**: 设备间直接建立的对等信任关系，无需账号关联。  
**适用场景**: 
- 临时设备协作（如投屏、文件传输）
- 无账号场景的设备配对
- 用户手动确认的设备间信任

**HiChain 组类型映射**: `GROUP_TYPE_PEER_TO_PEER_GROUP = 256`

### 1.2 IDENTICAL_ACCOUNT (同账号信任)

**业务含义**: 同一华为账号登录的设备间自动建立的信任关系。  
**适用场景**:
- 多设备同步（联系人、短信、照片等）
- 跨设备任务接续
- 自动发现的设备间认证

**HiChain 组类型映射**: `GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1`

### 1.3 ACROSS_ACCOUNT (跨账号信任)

**业务含义**: 不同华为账号设备间经授权建立的信任关系。  
**适用场景**:
- 家庭共享设备（如家庭成员间共享）
- 授权的设备访问

**HiChain 组类型映射**: `GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 1282`

### 1.4 SHARE (分享信任)

**业务含义**: 基于分享场景建立的临时信任关系。  
**适用场景**: 
- 设备功能分享（如共享相机、音箱）
- 临时访问授权

---

## 2. 认证方式 (DmAuthType)

设备认证支持多种交互方式，通过 `DMLocalServiceInfoPinExchangeType` 定义：

```cpp
enum class DMLocalServiceInfoPinExchangeType : int32_t {
    PINBOX = 1,            // PIN 码输入框（从内存获取）
    QR_FROMDP = 2,         // 二维码扫描（从设备 Profile 获取）
    FROMDP = 3,            // NFC（从设备 Profile 获取）
    IMPORT_AUTH_CODE = 5,  // 导入认证码（从设备 Profile 获取）
    ULTRASOUND = 6,        // 超声波（从内存获取）
    MAX = 7
};
```

### 2.1 PIN 码认证 (PINBOX)

**机制**: 用户手动输入 PIN 码完成认证。  
**用户交互**: 
- 认证发起端显示 PIN 码
- 被认证端输入相同 PIN 码

**适用信任类型**: PEER_TO_PEER

### 2.2 二维码认证 (QR_FROMDP)

**机制**: 扫描二维码完成认证，二维码包含设备信息。  
**用户交互**: 
- 被认证端显示二维码
- 认证发起端扫描二维码

**适用信任类型**: PEER_TO_PEER、IDENTICAL_ACCOUNT

### 2.3 NFC 认证 (FROMDP)

**机制**: 近场通信触碰完成认证。  
**用户交互**: 设备靠近触碰

**适用信任类型**: PEER_TO_PEER

### 2.4 免交互认证 (NO_INTERACTION)

**机制**: 基于同账号或已有信任关系自动完成认证。  
**用户交互**: 无需用户操作

**适用信任类型**: IDENTICAL_ACCOUNT

### 2.5 认证码导入 (IMPORT_AUTH_CODE)

**机制**: 通过预先生成的认证码完成认证。  
**用户交互**: 输入或导入认证码

**适用信任类型**: PEER_TO_PEER、ACROSS_ACCOUNT

### 2.6 超声波认证 (ULTRASOUND)

**机制**: 通过超声波信号传输认证信息。  
**用户交互**: 设备靠近

**适用信任类型**: PEER_TO_PEER

### 2.7 凭证认证 (CRE)

**机制**: 基于已有凭证的认证方式。

---

## 3. 设备身份体系

DeviceManager 使用三种设备标识符，各用于不同场景：

| 标识符 | 长度 | 用途 | 生命周期 | 获取方式 |
|--------|------|------|----------|----------|
| **deviceId** | 97 字节 | 设备唯一标识，用于 API 调用 | 设备终身不变 | GetLocalDeviceId() |
| **networkId** | 97 字节 | 组网后的网络标识，用于通信 | 组网期间有效 | GetLocalDeviceNetWorkId() |
| **udid** | 65 字节 | 硬件唯一标识，用于内部认证 | 设备终身不变 | GetUdidByNetworkId() |

### 3.1 使用场景对比

- **deviceId**: 
  - 应用层 API 调用
  - 设备绑定/解绑操作
  - 用户可见的设备标识

- **networkId**: 
  - 设备间通信（SoftBus 传输）
  - 分布式数据同步
  - 服务发现与发布

- **udid**: 
  - 认证凭证管理
  - ACL 访问控制
  - 内部安全校验

### 3.2 设备类型分类

```cpp
typedef enum DmDeviceType {
    DEVICE_TYPE_UNKNOWN = 0x00,      // 未知设备
    DEVICE_TYPE_WIFI_CAMERA = 0x08,  // 智能摄像头
    DEVICE_TYPE_AUDIO = 0x0A,        // 智能音箱
    DEVICE_TYPE_PC = 0x0C,           // 电脑
    DEVICE_TYPE_PHONE = 0x0E,        // 手机
    DEVICE_TYPE_PAD = 0x11,          // 平板
    DEVICE_TYPE_WATCH = 0x6D,        // 手表
    DEVICE_TYPE_CAR = 0x83,          // 车机
    DEVICE_TYPE_TV = 0x9C,           // 电视
    DEVICE_TYPE_SMART_DISPLAY = 0xA02, // 智能显示
    DEVICE_TYPE_2IN1 = 0xA2F,        // 二合一设备
    DEVICE_TYPE_GLASSES = 0xA31,     // AR 眼镜
} DmDeviceType;
```

---

## 4. 凭证组模型

HiChain 凭证组是设备信任关系的基础存储单元：

### 4.1 组类型 (GroupType)

```cpp
const int32_t GROUP_TYPE_INVALID_GROUP = 0;           // 无效组
const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1; // 同账号组
const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;    // 点对点组
const int32_t GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 1282; // 跨账号组
```

### 4.2 组可见性 (GroupVisibility)

- **PUBLIC (0)**: 公开组，可被同账号其他设备发现
- **PRIVATE (非 0)**: 私有组，仅组内设备可见

### 4.3 凭证组与信任类型映射关系

| DmAuthForm | GroupType | GroupVisibility | 典型场景 |
|------------|-----------|-----------------|----------|
| PEER_TO_PEER | 256 | PRIVATE | 临时配对 |
| IDENTICAL_ACCOUNT | 1 | PUBLIC | 多设备同步 |
| ACROSS_ACCOUNT | 1282 | PRIVATE | 家庭共享 |
| SHARE | 256 | PRIVATE | 功能分享 |

### 4.4 凭证组操作

- **创建**: 认证成功后自动创建对应类型的凭证组
- **查询**: 通过 HiChainConnector::GetGroupType() 获取设备信任类型
- **删除**: 解绑操作触发凭证组删除
- **老化**: 5.1.0 版本支持三阶段老化机制

---

## 5. 设备状态 (DmDeviceState)

```cpp
typedef enum DmDeviceState {
    DEVICE_STATE_UNKNOWN = -1,    // 未知状态
    DEVICE_STATE_ONLINE = 0,      // 物理上线
    DEVICE_INFO_READY = 1,        // 信息就绪（逻辑上线）
    DEVICE_STATE_OFFLINE = 2,     // 物理下线
    DEVICE_INFO_CHANGED = 3,      // 信息变更
} DmDeviceState;
```

### 5.1 物理上线/下线

- **物理上线 (DEVICE_STATE_ONLINE)**: 
  - 设备网络连接建立
  - SoftBus 层检测到设备在线
  - 触发 `OnDeviceOnline()` 事件

- **物理下线 (DEVICE_STATE_OFFLINE)**:
  - 设备网络连接断开
  - SoftBus 层检测到设备离线
  - 触发 `OnDeviceOffline()` 事件
  - 启动 300 秒倒计时删除凭证

### 5.2 逻辑上线/下线

- **逻辑上线 (DEVICE_INFO_READY)**:
  - 设备信息在分布式数据库（DDS）同步完成
  - 设备可正常提供分布式服务
  - 触发 `OnDeviceReady()` 事件

- **逻辑下线**: 
  - 设备信息从分布式数据库移除
  - 不再提供分布式服务

### 5.3 状态转换规则

```
物理上线 → 信息就绪 → [提供服务]
            ↓
        信息变更 → [更新服务]
            ↓
        物理下线 → [启动老化倒计时]
```

### 5.4 设备信息变更

- **触发场景**: 
  - 设备名称更改
  - 设备类型变更
  - 信任关系类型变化

- **处理流程**:
  1. 接收 `DEVICE_INFO_CHANGED` 事件
  2. 更新本地设备信息缓存
  3. 通知已注册的监听器

---

## 6. 发现与发布模型

### 6.1 订阅信息 (DmSubscribeInfo)

```cpp
typedef struct DmSubscribeInfo {
    uint16_t subscribeId;              // 订阅 ID
    DmDiscoverMode mode;               // 发现模式
    DmExchangeMedium medium;           // 发现媒介
    DmExchangeFreq freq;               // 发现频率
    bool isSameAccount;                // 是否仅同账号设备
    bool isWakeRemote;                 // 是否唤醒休眠设备
    char capability[DM_MAX_DEVICE_CAPABILITY_LEN]; // 能力过滤
} DmSubscribeInfo;
```

### 6.2 发布信息 (DmPublishInfo)

```cpp
typedef struct DmPublishInfo {
    int32_t publishId;                 // 发布 ID
    DmDiscoverMode mode;               // 发布模式
    DmExchangeFreq freq;               // 发布频率
    bool ranging;                      // 是否测距
    DmExchangeMedium medium;           // 发布媒介
} DmPublishInfo;
```

### 6.3 发现模式

```cpp
typedef enum DmDiscoverMode {
    DM_DISCOVER_MODE_PASSIVE = 0x55,  // 被动发现（监听）
    DM_DISCOVER_MODE_ACTIVE = 0xAA    // 主动发现（扫描）
} DmDiscoverMode;
```

- **被动发现**: 设备广播自身信息，等待其他设备发现
- **主动发现**: 设备扫描周围设备广播信息

### 6.4 发现媒介

```cpp
typedef enum DmExchangeMedium {
    DM_AUTO = 0,   // 自动选择
    DM_BLE = 1,    // 蓝牙 BLE
    DM_COAP = 2,   // WiFi (CoAP)
    DM_USB = 3,    // USB
    DM_MEDIUM_BUTT
} DmExchangeMedium;
```

### 6.5 发现频率

```cpp
typedef enum DmExchangeFreq {
    DM_LOW = 0,         // 低频
    DM_MID = 1,         // 中频
    DM_HIGH = 2,        // 高频
    DM_SUPER_HIGH = 3,  // 超高频
    DM_EXTREME_HIGH = 4 // 极高频
} DmExchangeFreq;
```

**注意**: 频率参数仅对蓝牙有效，WiFi 发现不支持频率配置。

### 6.6 过滤模型

设备发现支持以下过滤条件：

- **设备类型过滤**: `FILTER_PARA_DEVICE_TYPE`
- **距离过滤**: `FILTER_PARA_RANGE`（单位：厘米）
- **信任状态过滤**: `FILTER_PARA_INCLUDE_TRUST`
- **能力过滤**: `capability` 字段（如 "AAC", "HDCP"）

---

## 7. ACL 访问控制列表

### 7.1 ACL 概念

ACL (Access Control List) 是 DeviceManager 实现跨设备访问控制的核心机制，用于控制设备间服务访问权限。

### 7.2 ACL 角色定义

- **Accesser (访问者)**: 发起访问请求的设备/应用
  - 包含信息: `accountId`, `networkId`, `pkgName`, `userId`, `tokenId`

- **Accessee (被访问者)**: 提供服务的设备/应用
  - 包含信息: `accountId`, `networkId`, `peerId`, `pkgName`, `userId`, `tokenId`

### 7.2.1 字段归属约束（关系属性 vs 实体属性）

DP 持久化模型里三张表语义严格分层，**不可混用**：

| 表 | 语义 | extraData 该装什么 |
|---|---|---|
| `access_control_table` (`AccessControlProfile`) | **一条可信关系**本身 | **关系级属性**：`ACL_LIFE_CYCLE_DAYS` (关系生命周期天数)、`IsLnnAcl` (关系是否 LNN 内置)、`serviceId` (关系绑定的服务标识)、关系级 flag |
| `accesser_table` (`Accesser`) | 关系**主体侧实体**身份补充 | **实体级身份信息**：`dmVersion`（实体所在设备的 DM 版本号）等 |
| `accessee_table` (`Accessee`) | 关系**客体侧实体**身份补充 | 同上，仅装实体自身信息 |

**判定规则**（一句话）：如果删除/修改这个值会改变"两实体之间这条信任关系的属性"，就属于关系级，落 `AccessControlProfile.extraData`；如果只改变某一侧实体本身的描述（如版本号、设备型号），才落 `accesser/accessee` 的 extraData。

**反例（曾出现，已修正）**：把 `ACL_LIFE_CYCLE_DAYS` 写到 `accessee.extraData`。错误原因——生命周期是"这条关系到期日"，不是"客体设备的属性"。同一客体设备可与同一主体建立多条不同 bindType 的 ACL，每条 ACL 的生命周期独立；如果挂在 `accessee`，多条 ACL 共用一份 `accessee` 行时会相互覆盖。

**典型字段归属对照**：

| 字段 | 归属 | 理由 |
|---|---|---|
| `ACL_LIFE_CYCLE_DAYS` | `AccessControlProfile.extraData` | 关系到期天数 |
| `IsLnnAcl` | `AccessControlProfile.extraData` | 关系类型 flag |
| `serviceId` | `AccessControlProfile.extraData` | 关系所属服务 |
| `bindType` / `bindLevel` / `authenticationType` / `status` | `AccessControlProfile` 顶层字段（**有专列**） | 关系核心列 |
| `dmVersion` | `accesser/accessee.extraData` | 实体所在设备 DM 版本 |
| `accesseeCredentialIdStr` / `accesseeSessionKeyId` | `Accessee` 顶层字段（**有专列**） | 实体的凭证/会话密钥 |

**写入侧契约**：写 ACL 时按下面分支调用：

```cpp
// 关系级：进 ACL extraData
JsonObject aclExtra;
aclExtra[ACL_IS_LNN_ACL_KEY] = ACL_IS_LNN_ACL_VAL_FALSE;
aclExtra[TAG_SERVICE_ID] = access.serviceId;
aclExtra[ACL_LIFE_CYCLE_DAYS] = context->aclLifeCycleDays;  // 关系生命周期
profile.SetExtraData(aclExtra.Dump());

// 实体级：进 accesser/accessee extraData（仅装实体身份补充）
accesser.SetAccesserExtraData(context->accesser.extraInfo);   // 含 dmVersion
accessee.SetAccesseeExtraData(context->accessee.extraInfo);   // 含 dmVersion
```

**读取侧契约**：消费关系级属性必须从 `profile.GetExtraData()` 读，不允许从 `accesser/accessee.extraData` 读关系级字段。

### 7.3 ACL 全生命周期

#### 阶段 1: Active (活动状态)

- **创建时机**: 设备绑定成功
- **状态特征**: 
  - ACL 存储在 DP (Device Profile) 数据库
  - 设备可正常访问服务
  - 定期刷新避免过期

- **转换条件**: 
  - 设备物理下线 → Transition
  - 设备解绑 → 直接删除

#### 阶段 2: Transition (过渡状态)

- **进入时机**: 设备物理下线
- **状态特征**:
  - ACL 标记为过渡状态
  - 启动 300 秒老化倒计时
  - 允许短暂重新上线恢复

- **转换条件**:
  - 倒计时结束 → Cache
  - 设备重新上线 → Active

#### 阶段 3: Cache (缓存状态)

- **进入时机**: 老化倒计时结束
- **状态特征**:
  - ACL 从 DP 移除，存储到本地缓存
  - 删除关联的 SessionKey 和凭证
  - 仅保留基本信息

- **转换条件**:
  - 用户手动清理 → 删除
  - 缓存空间不足 → 删除

### 7.4 ACL 版本管理 (5.1.0+)

```cpp
// 支持老化的版本列表
const std::vector<std::string> DM_SUPPORT_ACL_AGING_VERSIONS = {
    DM_VERSION_STR_5_1_0
};
```

- ACL 按版本存储哈希值
- 版本升级时自动迁移
- 不支持老化的版本保持原有行为

---

## 8. SessionKey 会话密钥

### 8.1 SessionKey 概念

SessionKey 是设备间加密通信的会话密钥，由 HiChain 在认证过程中生成，用于保护设备间数据传输安全。

### 8.2 存储位置

| 存储位置 | 用途 | 生命周期 |
|----------|------|----------|
| **DP (设备配置文件)** | 主要存储，持久化保存 | ACL Active 状态 |
| **UKCache (用户密钥缓存)** | 备份存储，快速恢复 | ACL Transition 状态 |
| **内存 (CryptoMgr)** | 运行时使用，加解密操作 | 应用运行期间 |

### 8.3 生命周期阶段

#### 阶段 1: 生成

- **触发时机**: 设备认证成功
- **生成方式**: HiChain 协商生成 256 位密钥
- **回调接口**: `onSessionKeyReturned(requestId, sessionKey, length)`

#### 阶段 2: 使用

- **存储**: CryptoMgr 加密保存到内存
- **用途**: 
  - 设备间通信加密
  - 敏感信息加解密
  - 数据传输完整性校验

#### 阶段 3: 持久化

- **DP 存储**: 
  - 关联 ACL ID
  - 加密存储在设备配置文件
  - 设备重启后恢复

- **UKCache 备份**:
  - 防止 DP 数据丢失
  - 快速恢复机制

#### 阶段 4: 销毁

- **触发条件**:
  - 设备解绑
  - ACL 老化完成 (Transition → Cache)
  - 用户手动清理

- **销毁流程**:
  1. `ClearSessionKey()` 清除内存
  2. DP 中删除关联记录
  3. UKCache 中删除备份

### 8.4 密钥管理接口

```cpp
// 加密数据
int32_t Encrypt(const std::vector<uint8_t> &input, std::vector<uint8_t> &output);

// 解密数据
int32_t Decrypt(const std::vector<uint8_t> &input, std::vector<uint8_t> &output);

// 获取 SessionKey
std::vector<unsigned char> GetSessionKey();

// 清除 SessionKey
void ClearSessionKey();
```

---

## 9. 易混淆概念对比

### 9.1 物理上下线 vs 逻辑上下线

| 对比项 | 物理上下线 | 逻辑上下线 |
|--------|------------|------------|
| **定义** | 设备网络连接状态变化 | 设备信息在 DDS 同步状态变化 |
| **检测方式** | SoftBus 层检测 | DDS 数据库同步完成 |
| **状态值** | ONLINE / OFFLINE | READY / CHANGED |
| **触发条件** | WiFi/BLE 连接/断开 | 设备信息写入/移除 DDS |
| **业务影响** | 设备可达性变化 | 设备服务可用性变化 |
| **老化影响** | 触发老化倒计时 | 不触发老化 |

### 9.2 deviceId vs networkId vs udid

| 对比项 | deviceId | networkId | udid |
|--------|----------|-----------|------|
| **长度** | 97 字节 | 97 字节 | 65 字节 |
| **生成方式** | 设备启动时生成 | 组网时分配 | 硬件标识计算 |
| **变化性** | 固定不变 | 组网后变化 | 固定不变 |
| **可见性** | 应用层可见 | 应用层可见 | 系统内部使用 |
| **使用场景** | API 调用 | 通信传输 | 认证/ACL |
| **安全性** | 中 | 中 | 高（敏感信息） |
| **获取接口** | GetLocalDeviceId | GetLocalDeviceNetWorkId | GetUdidByNetworkId |

### 9.3 认证 vs 绑定 vs 组网

| 对比项 | 认证 (Authentication) | 绑定 (Binding) | 组网 (Networking/JoinLNN) |
|--------|----------------------|----------------|---------------------------|
| **目的** | 建立设备间信任关系 | 建立业务层关联 | 建立通信网络 |
| **执行顺序** | 第一 | 第二 | 第三 |
| **涉及模块** | HiChain | DM Service | SoftBus |
| **产出物** | 凭证组 | ACL + SessionKey | networkId |
| **用户交互** | 需要（PIN/二维码等） | 可选 | 自动 |
| **时效性** | 长期有效 | 长期有效 | 动态变化 |
| **典型接口** | AuthenticateDevice | BindDevice | JoinLNN |

**流程关系**:
```
认证成功 → 创建凭证组 → 绑定设备 → 创建 ACL → 组网通信
```

### 9.4 ACL Active vs Transition vs Cache

| 对比项 | Active | Transition | Cache |
|--------|--------|------------|-------|
| **设备状态** | 物理在线 | 物理离线 | 长期离线 |
| **存储位置** | DP | DP | 本地缓存 |
| **访问权限** | 正常访问 | 受限访问 | 禁止访问 |
| **老化状态** | 正常 | 倒计时中 | 已老化 |
| **恢复机制** | - | 重新上线恢复 | 需重新认证 |
| **删除触发** | 解绑操作 | 倒计时结束 | 手动清理 |
| **SessionKey** | 存在 | 存在但过期 | 已删除 |

### 9.5 同账号认证 vs 跨账号认证 vs P2P 认证

| 对比项 | 同账号认证 | 跨账号认证 | P2P 认证 |
|--------|------------|------------|----------|
| **账号要求** | 同一华为账号 | 不同华为账号 | 无账号要求 |
| **GroupType** | IDENTICAL_ACCOUNT (1) | ACROSS_ACCOUNT (1282) | PEER_TO_PEER (256) |
| **用户交互** | 免交互或简单确认 | 需要授权 | 需要确认 |
| **发现方式** | 自动发现 | 手动添加 | 手动发现 |
| **信任等级** | 高 | 中 | 中 |
| **典型场景** | 多设备同步 | 家庭共享 | 临时协作 |
| **ACL 可见性** | PUBLIC | PRIVATE | PRIVATE |
| **过期机制** | 不轻易过期 | 较易过期 | 易过期 |

---

## 10. 附录：常量定义

### 10.1 版本常量

```cpp
extern const char* DM_VERSION_5_0_1;  // "5.0.1"
extern const char* DM_VERSION_5_1_0;  // "5.1.0"
extern const char* DM_CURRENT_VERSION; // 当前最新版本
extern const char* DM_ACL_AGING_VERSION; // 支持 ACL 老化的版本 (5.1.0)
```

### 10.2 长度限制

```cpp
constexpr int32_t DEVICE_UUID_LENGTH = 65;              // UUID 长度
constexpr int32_t DEVICE_NETWORKID_LENGTH = 65;         // networkId 长度
constexpr int32_t DEVICEID_LEN = 65;                    // deviceId 长度
constexpr int32_t DM_MAX_DEVICE_ID_LEN = 97;            // 设备 ID 最大长度
constexpr int32_t DM_MAX_DEVICE_NAME_LEN = 129;         // 设备名称最大长度
constexpr int32_t DM_MAX_PIN_CODE_LEN = 1025;           // PIN 码最大长度
constexpr int32_t DM_MAX_META_TOKEN_LEN = 9;            // Meta token 最大长度
```

### 10.3 时间常量

```cpp
#define OFFLINE_TIMEOUT 300  // 设备下线老化超时时间（秒）
```

### 10.4 网络类型

```cpp
typedef enum {
    BIT_NETWORK_TYPE_UNKNOWN = 0,  // 未知网络
    BIT_NETWORK_TYPE_WIFI,         // WiFi
    BIT_NETWORK_TYPE_BLE,          // 蓝牙 BLE
    BIT_NETWORK_TYPE_BR,           // 蓝牙 BR
    BIT_NETWORK_TYPE_P2P,          // P2P
    BIT_NETWORK_TYPE_USB = 8,      // USB
    BIT_NETWORK_TYPE_COUNT,        // 无效类型
} DmNetworkType;
```

---

## 11. 参考文档

- [01-architecture.md](./01-architecture.md) - 架构设计文档
- [03-api-reference.md](./03-api-reference.md) - API 接口参考
- [04-workflows/](./04-workflows/) - 核心工作流程文档
- OpenHarmony DeviceManager 官方文档: https://gitee.com/openharmony/distributedhardware_device_manager

---

**文档更新记录**:

| 版本 | 日期 | 更新内容 | 作者 |
|------|------|----------|------|
| v1.0 | 2025-01-01 | 初始版本 | [待填写] |
| v2.0 | 2026-05-19 | 基于源码重构，补充 5.1.0 特性 | [待填写] |
