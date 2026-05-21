# DM 依赖的外部模块 API

版本 v2.0
更新日期 2026-05-19

## 1. 概述

设备管理器（Device Manager，DM）作为 OpenHarmony 分布式设备管理的核心组件，需要与多个外部模块进行交互，主要包括：

- **SoftBus**：提供设备发现、网络管理、数据传输能力
- **DP（DeviceProfile）**：提供设备配置信息存储、访问控制列表（ACL）管理、会话密钥管理能力
- **HiChain**：提供设备认证、群组管理、凭据管理能力

本文档详细描述 DM 对这些外部模块的 API 依赖清单、调用约束与实际调用模式。

## 2. 依赖关系全景

```mermaid
graph TB
    subgraph Device Manager (DM)
        DM[Device Manager]
        SC[SoftbusConnector]
        SL[SoftbusListener]
        DPC[DeviceProfileConnector]
        HC[HiChainConnector]
        HCA[HiChainAuthConnector]
    end
    subgraph SoftBus
        S1[JoinLNN]
        S2[LeaveLNN]
        S3[RefreshLNN]
        S4[StartDiscovery]
        S5[PublishLNN]
        S6[GetUdidByNetworkId]
        S7[GetConnectAddr]
        S8[CheckIsOnline]
        S9[ShiftLNNGear]
    end
    subgraph DeviceProfile (DP)
        DP1[PutAccessControlProfile]
        DP2[DeleteAccessControlProfile]
        DP3[GetAccessControlProfile]
        DP4[SubscribeDeviceProfile]
        DP5[PutSessionKey]
        DP6[GetSessionKey]
        DP7[PutServiceInfo]
        DP8[GetAllAccessControlProfile]
        DP9[GetServiceInfosByUdid]
    end
    subgraph HiChain
        HC1[CreateGroup]
        HC2[AddMember]
        HC3[DeleteGroup]
        HC4[GetGroupInfo]
        HC5[AuthDevice]
        HC6[GenerateCredential]
        HC7[ImportCredential]
        HC8[DeleteCredential]
    end
    DM --> S1
    DM --> S2
    DM --> S3
    DM --> S4
    DM --> S5
    DM --> S6
    DM --> S7
    DM --> S8
    DM --> S9
    DM --> DP1
    DM --> DP2
    DM --> DP3
    DM --> DP4
    DM --> DP5
    DM --> DP6
    DM --> DP7
    DM --> DP9
    DM --> HC1
    DM --> HC2
    DM --> HC3
    DM --> HC4
    DM --> HC5
    DM --> HC6
    DM --> HC7
    DM --> HC8
    SC -.->|事件回调| SL
    DPC -.->|配置变更通知| DM
    HC -.->|认证结果| DM
    HCA -.->|凭据事件| DM
    SoftBus -.-> DP8
    SoftBus -.-> DP6
```

## 3. DM → SoftBus 接口

### 3.1 JoinLNN

**功能**: 将设备加入逻辑网络（Logical Network）

**调用场景**: 
- 设备认证完成后，将设备加入 LNN 以启用分布式能力
- 通过 HML（Hardware Markup Language）会话加入 LNN
- 通过会话密钥 ID 加入 LNN

**接口签名**:
```cpp
void JoinLnn(const std::string &deviceId, bool isForceJoin = false);
void JoinLnnByHml(int32_t sessionId, int32_t sessionKeyId, int32_t remoteSessionKeyId, bool isForceJoin = false);
void JoinLNNBySkId(int32_t sessionId, int32_t sessionKeyId, int32_t remoteSessionKeyId,
                   std::string udid, std::string udidHash, bool isForceJoin = false);
```

**参数详解**:
- `deviceId`: 目标设备 ID（UDID 或 NetworkId）
- `isForceJoin`: 是否强制加入（用于重连场景）
- `sessionId`: 会话 ID
- `sessionKeyId`: 本地会话密钥 ID
- `remoteSessionKeyId`: 对端会话密钥 ID
- `udid`: 设备 UDID
- `udidHash`: UDID 哈希值

**返回值与错误码**:
- 通过回调 `OnSoftbusJoinLNNResult(ConnectionAddr *addr, const char *networkId, int32_t result)` 返回结果
- `result = 0`: 成功
- `result != 0`: 失败（SoftBus 错误码）

**调用约束**:
- **必须**在设备认证完成后调用
- **必须**确保设备已通过 SoftBus 发现
- 强制加入模式仅用于设备已离线后重新加入场景

**实际调用位置**:
- `softbus_connector.cpp:JoinLnn()` - 主要实现
- 认证完成后通过 `HiChainConnector` 回调触发

---

### 3.2 LeaveLNN

**功能**: 将设备从逻辑网络移除

**调用场景**:
- 设备解绑（Unbind）
- 设备下线清理
- 用户登出导致的设备信任关系变更

**接口签名**:
```cpp
int32_t LeaveLNN(const std::string &pkgName, const std::string &networkId);
static void OnLeaveLNNResult(const char *networkId, int32_t retCode);
```

**参数详解**:
- `pkgName`: 调用者包名（如 "com.huawei.devicemanager"）
- `networkId`: SoftBus 网络设备 ID

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败（SoftBus 错误码）

**调用约束**:
- **必须**先删除 ACL（访问控制列表）
- **必须**先删除会话密钥
- **应该**在设备离线通知前调用

**实际调用位置**:
- `softbus_connector.cpp:LeaveLNN()` - 主要实现
- 设备解绑流程中调用

---

### 3.3 RefreshLNN / StartDiscovery

**功能**: 刷新 LNN 信息或启动设备发现

**调用场景**:
- 主动发现周边设备
- 刷新设备列表
- 获取最新设备状态

**接口签名**:
```cpp
int32_t RefreshSoftbusLNN(const char *pkgName, const DmSubscribeInfo &dmSubInfo, const std::string &customData);
int32_t StopRefreshSoftbusLNN(uint16_t subscribeId);
```

**参数详解**:
- `pkgName`: 订阅者包名
- `dmSubInfo`: 订阅信息（包含订阅模式、频率等）
- `customData`: 自定义数据（用于过滤或扩展）
- `subscribeId`: 订阅 ID（用于停止订阅）

**返回值与错误码**:
- `0`: 成功启动/停止发现
- 非 0: 失败

**调用约束**:
- **必须**配对调用 Refresh 和 StopRefresh
- **必须**处理发现超时
- 同一包名多次订阅需使用不同 subscribeId

**实际调用位置**:
- `softbus_listener.cpp:RefreshSoftbusLNN()` - 主要实现

---

### 3.4 PublishLNN

**功能**: 发布本设备 LNN 信息，使其他设备可发现

**调用场景**:
- 设备启动后发布本设备信息
- 设备状态变更时更新发布信息

**接口签名**:
```cpp
int32_t PublishSoftbusLNN(const DmPublishInfo &dmPubInfo, const std::string &capability, const std::string &customData);
int32_t StopPublishSoftbusLNN(int32_t publishId);
static void OnSoftbusPublishResult(int publishId, PublishResult result);
```

**参数详解**:
- `dmPubInfo`: 发布信息（包含设备名、类型等）
- `capability`: 能力标识（如 "ddmpCapability"）
- `customData`: 自定义数据
- `publishId`: 发布 ID
- `result`: 发布结果（PUBLISH_SUCCESS 或 PUBLISH_FAIL）

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在设备管理服务初始化后调用
- **应该**持续监听发布结果
- 停止发布时使用对应的 publishId

**实际调用位置**:
- `softbus_publish.cpp:PublishSoftbusLNN()` - 主要实现

---

### 3.5 GetUdidByNetworkId

**功能**: 通过 SoftBus NetworkId 获取设备 UDID

**调用场景**:
- 设备上线时获取真实 UDID
- 需要持久化设备 ID 时转换
- ACL 匹配时验证设备身份

**接口签名**:
```cpp
static int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);
static int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);
```

**参数详解**:
- `networkId`: SoftBus 网络设备 ID（临时 ID）
- `udid`: 输出参数，设备唯一标识（持久 ID）
- `uuid`: 输出参数，设备 UUID

**返回值与错误码**:
- `0`: 成功
- `ERR_DM_FAILED`: 查询失败

**调用约束**:
- **必须**在设备在线后调用
- 返回的 udid 可能需要进一步哈希处理
- NetworkId 在设备重启后会变更，UDID 不变

**实际调用位置**:
- `softbus_connector.cpp:GetUdidByNetworkId()` - 主要实现
- 设备上线事件处理中高频调用

---

### 3.6 GetConnectAddr

**功能**: 获取设备连接地址（IP、MAC 等）

**调用场景**:
- 建立数据传输通道前获取连接地址
- 设备连接状态检查
- 多种连接方式切换（WiFi 转蓝牙等）

**接口签名**:
```cpp
static std::shared_ptr<ConnectionAddr> GetConnectAddr(const std::string &deviceId, std::string &connectAddr);
```

**参数详解**:
- `deviceId`: 目标设备 ID
- `connectAddr`: 输出参数，连接地址 JSON 字符串

**返回值与错误码**:
- 非 nullptr: 成功，返回连接地址对象
- nullptr: 失败

**调用约束**:
- **必须**在设备在线后调用
- **必须**处理多种连接类型（WIFI_IP、BLE_MAC、BR_MAC 等）
- 返回地址可能包含多个连接方式

**实际调用位置**:
- `softbus_connector.cpp:GetConnectAddr()` - 主要实现

---

### 3.7 CheckIsOnline

**功能**: 检查设备在线状态

**调用场景**:
- 设备操作前验证设备在线
- 设备状态监控
- 重连决策

**接口签名**:
```cpp
bool CheckIsOnline(const std::string &targetDeviceId);
bool CheckIsOnline(const std::string &targetDeviceIdHash, bool isHash);
```

**参数详解**:
- `targetDeviceId`: 目标设备 ID（UDID）
- `targetDeviceIdHash`: 设备 ID 哈希值
- `isHash`: 是否为哈希值

**返回值与错误码**:
- `true`: 设备在线
- `false`: 设备离线

**调用约束**:
- 高频调用，**必须**使用缓存优化
- **应该**定期同步 SoftBus 真实状态

**实际调用位置**:
- `softbus_connector.cpp:CheckIsOnline()` - 主要实现

---

### 3.8 ShiftLNNGear（BLE 心跳控制）

**功能**: 调整 LNN 心跳策略，平衡功耗与实时性

**调用场景**:
- 设备唤醒时提高心跳频率
- 设备休眠时降低心跳频率
- 应用请求特定心跳级别

**接口签名**:
```cpp
int32_t ShiftLNNGear(bool isWakeUp, const std::string &callerId);
```

**参数详解**:
- `isWakeUp`: true=唤醒模式（高频心跳），false=休眠模式（低频心跳）
- `callerId`: 调用者标识（如包名）

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在设备已加入 LNN 后调用
- **应该**按需调用，避免过度功耗
- **应该**在状态恢复时调用

**实际调用位置**:
- `softbus_listener.cpp:ShiftLNNGear()` - 主要实现

---

## 4. DM → DP 接口

### 4.1 PutAccessControlProfile

**功能**: 存储访问控制配置（ACL），建立设备间信任关系

**调用场景**:
- 设备认证成功后创建 ACL
- 应用授权创建跨应用 ACL
- 服务绑定创建服务级 ACL

**接口签名**:
```cpp
int32_t PutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser, DmAccessee dmAccessee);
```

**参数详解**:
- `aclInfo`: ACL 信息
  - `sessionKey`: 会话密钥
  - `bindType`: 绑定类型（点对点/同账号/跨账号）
  - `state`: 状态（ACTIVE/INACTIVE）
  - `bindLevel`: 绑定级别（设备级/应用级/服务级）
- `dmAccesser`: 访问发起者信息
  - `requestTokenId`: 发起者 TokenId
  - `requestBundleName`: 发起者包名
  - `requestDeviceId`: 发起者设备 ID
- `dmAccessee`: 被访问者信息
  - `trustTokenId`: 被访问者 TokenId
  - `trustBundleName`: 被访问者包名
  - `trustDeviceId`: 被访问者设备 ID

**返回值与错误码**:
- `0`: 成功
- `ERR_DM_FAILED`: 失败

**调用约束**:
- **必须**在认证成功后调用
- **必须**先存储会话密钥（PutSessionKey）
- 设备级 ACL **必须**在应用级/服务级 ACL 之前创建
- **应该**处理重复创建场景

**实际调用位置**:
- `deviceprofile_connector.cpp:PutAccessControlList()` - 主要实现
- 认证流程 `auth_confirm.cpp`、`auth_acl.cpp` 中调用

---

### 4.2 DeleteAccessControlProfile

**功能**: 删除访问控制配置

**调用场景**:
- 设备解绑
- 应用卸载
- 服务解绑
- 用户登出

**接口签名**:
```cpp
DmOfflineParam DeleteAccessControlList(const std::string &pkgName, const std::string &localDeviceId,
                                       const std::string &remoteDeviceId, int32_t bindLevel, const std::string &extra);
void DeleteAccessControlList(const std::string &udid);
void DeleteAccessControlById(int64_t accessControlId);
```

**参数详解**:
- `pkgName`: 应用包名
- `localDeviceId`: 本地设备 ID
- `remoteDeviceId`: 对端设备 ID
- `bindLevel`: 绑定级别（过滤条件）
- `extra`: 扩展信息（如 TokenId）
- `udid`: 设备 UDID
- `accessControlId`: ACL ID

**返回值与错误码**:
- `DmOfflineParam`: 返回删除操作的详细结果
  - `processVec`: 需要通知下线的进程列表
  - `leftAclNumber`: 剩余 ACL 数量

**调用约束**:
- **必须**先删除会话密钥（DeleteSessionKey）
- **必须**先删除 HiChain 凭据（DeleteCredential）
- **应该**先检查是否为最后一个 ACL（决定是否通知设备下线）
- **应该**按依赖关系删除（先应用级，后设备级）

**实际调用位置**:
- `deviceprofile_connector.cpp:DeleteAccessControlList()` - 主要实现
- `dm_device_state_manager.cpp` 中设备下线处理

---

### 4.3 GetAccessControlProfile

**功能**: 查询访问控制配置

**调用场景**:
- 设备权限检查
- 认证状态查询
- ACL 同步验证

**接口签名**:
```cpp
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAccessControlProfile();
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAccessControlProfileByUserId(int32_t userId);
DistributedDeviceProfile::AccessControlProfile GetAccessControlProfileByAccessControlId(int64_t accessControlId);
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAclList(const std::string localUdid, int32_t localUserId,
                                                                        const std::string remoteUdid, int32_t remoteUserId);
```

**参数详解**:
- `userId`: 用户 ID
- `accessControlId`: ACL ID
- `localUdid`: 本地设备 UDID
- `remoteUdid`: 对端设备 UDID

**返回值与错误码**:
- 返回 ACL 列表或单个 ACL 对象

**调用约束**:
- 高频查询，**应该**使用缓存
- **应该**处理空结果场景

**实际调用位置**:
- `deviceprofile_connector.cpp:GetAccessControlProfile()` - 主要实现
- 权限检查 `CheckAccessControl()` 中调用

---

### 4.4 SubscribeDeviceProfile

**功能**: 订阅设备配置变更通知

**调用场景**:
- 订阅远程设备 ACL 变更
- 订阅远程设备服务信息变更
- 订阅设备上线/下线事件

**接口签名**:
```cpp
int32_t SubscribeDeviceProfileInited(sptr<DistributedDeviceProfile::IDpInitedCallback> dpInitedCallback);
int32_t UnSubscribeDeviceProfileInited();
```

**参数详解**:
- `dpInitedCallback`: DP 初始化回调接口

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在 DM 服务初始化时订阅
- **必须**在服务停止时取消订阅
- **应该**异步处理通知

**实际调用位置**:
- `deviceprofile_connector.cpp:SubscribeDeviceProfileInited()` - 主要实现
- DM 服务初始化流程中调用

---

### 4.5 PutSessionKey

**功能**: 存储会话密钥

**调用场景**:
- 认证成功后存储会话密钥
- 会话密钥更新

**接口签名**:
```cpp
int32_t PutSessionKey(int32_t userId, const std::vector<unsigned char> &sessionKeyArray, int32_t &sessionKeyId);
```

**参数详解**:
- `userId`: 用户 ID
- `sessionKeyArray`: 会话密钥字节数组
- `sessionKeyId`: 输出参数，返回的会话密钥 ID

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在创建 ACL 前调用
- **必须**确保密钥长度合法
- **应该**妥善保管返回的 sessionKeyId

**实际调用位置**:
- `deviceprofile_connector.cpp:PutSessionKey()` - 主要实现
- 认证流程 `auth_credential.cpp` 中调用

---

### 4.6 GetSessionKey

**功能**: 获取已存储的会话密钥

**调用场景**:
- 建立加密通道
- 会话密钥同步验证
- ACL 哈希校验

**接口签名**:
```cpp
int32_t GetSessionKey(int32_t userId, int32_t sessionKeyId, std::vector<unsigned char> &sessionKeyArray);
```

**参数详解**:
- `userId`: 用户 ID
- `sessionKeyId`: 会话密钥 ID
- `sessionKeyArray`: 输出参数，会话密钥字节数组

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败（如密钥不存在）

**调用约束**:
- **必须**谨慎处理密钥数据，避免泄露
- **应该**验证密钥有效性

**实际调用位置**:
- `deviceprofile_connector.cpp:GetSessionKey()` - 主要实现

---

### 4.7 PutServiceInfo

**功能**: 存储服务信息（服务级 ACL）

**调用场景**:
- 服务绑定
- 服务信息更新

**接口签名**:
```cpp
int32_t PutServiceInfo(const DistributedDeviceProfile::ServiceInfo &dpServiceInfo);
```

**参数详解**:
- `dpServiceInfo`: 服务信息
  - `serviceName`: 服务名称
  - `serviceType`: 服务类型
  - `serviceId`: 服务 ID

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**先创建设备级或应用级 ACL
- **必须**确保 serviceId 唯一

**实际调用位置**:
- `deviceprofile_connector.cpp:PutServiceInfo()` - 主要实现

---

### 4.8 GetAllAccessControlProfile

**功能**: 获取所有 ACL（包括 LNN ACL）

**调用场景**:
- ACL 同步
- 设备状态查询
- ACL 校验和计算

**接口签名**:
```cpp
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAccessControlProfile();
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAclIncludeLnnAcl();
```

**返回值与错误码**:
- 返回所有 ACL 配置列表

**调用约束**:
- 大数据量查询，**应该**异步处理
- **应该**过滤无效 ACL

**实际调用位置**:
- `deviceprofile_connector.cpp:GetAllAccessControlProfile()` - 主要实现
- SoftBus 查询 ACL 时调用

---

### 4.9 GetServiceInfosByUdid

**功能**: 获取设备的服务列表

**调用场景**:
- 服务绑定验证
- 服务代理查询

**接口签名**:
```cpp
int32_t GetServiceInfosByUdid(const std::string &udid, std::vector<DistributedDeviceProfile::ServiceInfo> &serviceInfos);
int32_t GetServiceInfosByUdidAndUserId(const std::string &udid, int32_t userId,
                                        std::vector<DistributedDeviceProfile::ServiceInfo> &serviceInfos);
```

**参数详解**:
- `udid`: 设备 UDID
- `userId`: 用户 ID
- `serviceInfos`: 输出参数，服务信息列表

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**确保设备 UDID 有效

**实际调用位置**:
- `deviceprofile_connector.cpp:GetServiceInfosByUdid()` - 主要实现

---

## 5. DM → HiChain 接口

### 5.1 CreateGroup

**功能**: 创建 HiChain 群组

**调用场景**:
- 点对点认证创建 P2P 群组
- 同账号认证创建同账号群组
- 跨账号认证创建跨账号群组

**接口签名**:
```cpp
int32_t CreateGroup(int64_t requestId, const std::string &groupName);
int32_t CreateGroup(int64_t requestId, int32_t authType, const std::string &userId, JsonObject &jsonOutObj);
```

**参数详解**:
- `requestId`: 请求 ID（用于回调匹配）
- `groupName`: 群组名称
- `authType`: 认证类型（点对点/同账号/跨账号）
- `userId`: 用户 ID
- `jsonOutObj`: 输出 JSON 对象

**返回值与错误码**:
- `0`: 成功发起创建请求
- `ERR_DM_INPUT_PARA_INVALID`: 参数错误

**调用约束**:
- **必须**先注册 HiChain 回调
- **应该**处理群组已存在场景
- **应该**通过回调处理异步结果

**实际调用位置**:
- `hichain_connector.cpp:CreateGroup()` - 主要实现
- 认证流程 `auth_credential.cpp` 中调用

---

### 5.2 AddMember

**功能**: 向 HiChain 群组添加成员

**调用场景**:
- 将对端设备加入已创建的群组
- 群组扩展

**接口签名**:
```cpp
int32_t AddMember(const std::string &deviceId, const std::string &connectInfo);
int32_t addMultiMembers(const int32_t groupType, const std::string &userId, const JsonObject &jsonDeviceList);
```

**参数详解**:
- `deviceId`: 目标设备 ID
- `connectInfo`: 连接信息（JSON 格式）
- `groupType`: 群组类型
- `jsonDeviceList`: 设备列表 JSON

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**确保群组已创建
- **应该**处理设备已在群组中场景

**实际调用位置**:
- `hichain_connector.cpp:AddMember()` - 主要实现

---

### 5.3 DeleteGroup

**功能**: 删除 HiChain 群组

**调用场景**:
- 设备解绑删除 P2P 群组
- 清理过期群组
- 用户登出删除群组

**接口签名**:
```cpp
int32_t DeleteGroup(std::string &groupId);
int32_t DeleteGroup(const int32_t userId, std::string &groupId);
int32_t DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType);
```

**参数详解**:
- `groupId`: 群组 ID
- `userId`: 用户 ID
- `authType`: 认证类型

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**先删除所有成员凭据
- **应该**在删除 ACL 后调用

**实际调用位置**:
- `hichain_connector.cpp:DeleteGroup()` - 主要实现
- 设备解绑流程中调用

---

### 5.4 GetGroupInfo

**功能**: 查询 HiChain 群组信息

**调用场景**:
- 获取设备所属群组
- 认证类型查询
- 群组状态验证

**接口签名**:
```cpp
bool GetGroupInfo(const std::string &queryParams, std::vector<GroupInfo> &groupList);
bool GetGroupInfo(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);
bool GetGroupInfoExt(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);
```

**参数详解**:
- `queryParams`: 查询参数 JSON（如 `{"groupId": "xxx"}`）
- `userId`: 用户 ID
- `groupList`: 输出参数，群组信息列表

**返回值与错误码**:
- `true`: 查询成功
- `false`: 查询失败

**调用约束**:
- 高频查询，**应该**使用缓存

**实际调用位置**:
- `hichain_connector.cpp:GetGroupInfo()` - 主要实现

---

### 5.5 AuthDevice

**功能**: 发起设备认证

**调用场景**:
- 点对点认证
- PIN 码认证
- 凭据认证

**接口签名**:
```cpp
int32_t AuthDevice(const std::string &pinCode, int32_t osAccountId, int64_t requestId);
int32_t AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId, const std::string &pinCode);
int32_t AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode);
```

**参数详解**:
- `pinCode`: PIN 码
- `osAccountId`: OS 账户 ID
- `requestId`: 认证请求 ID
- `credId`: 凭据 ID

**返回值与错误码**:
- `0`: 成功发起认证
- 非 0: 失败

**调用约束**:
- **必须**先注册认证回调
- **必须**通过回调处理认证结果
- **应该**处理认证超时

**实际调用位置**:
- `hichain_auth_connector.cpp:AuthDevice()` - 主要实现
- 认证流程 `auth_pin_auth.cpp` 中调用

---

### 5.6 GenerateCredential

**功能**: 生成凭据

**调用场景**:
- 创建本地凭据
- 认证前置准备

**接口签名**:
```cpp
int32_t GenerateCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey);
```

**参数详解**:
- `localUdid`: 本地设备 UDID
- `osAccountId`: OS 账户 ID
- `publicKey`: 输出参数，公钥

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在创建群组前调用

**实际调用位置**:
- `hichain_auth_connector.cpp:GenerateCredential()` - 主要实现

---

### 5.7 ImportCredential

**功能**: 导入对端凭据

**调用场景**:
- 认证成功后导入对端凭据
- 凭据同步

**接口签名**:
```cpp
int32_t ImportCredential(int32_t osAccountId, int32_t peerOsAccountId, std::string deviceId, std::string publicKey);
```

**参数详解**:
- `osAccountId`: 本地 OS 账户 ID
- `peerOsAccountId`: 对端 OS 账户 ID
- `deviceId`: 设备 ID
- `publicKey`: 公钥

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在对端认证通过后调用

**实际调用位置**:
- `hichain_auth_connector.cpp:ImportCredential()` - 主要实现

---

### 5.8 DeleteCredential

**功能**: 删除凭据

**调用场景**:
- 设备解绑删除对端凭据
- 清理过期凭据

**接口签名**:
```cpp
int32_t DeleteCredential(const std::string &deviceId, int32_t userId, int32_t peerUserId);
int32_t DeleteCredential(int32_t osAccountId, const std::string &credId);
```

**参数详解**:
- `deviceId`: 设备 ID
- `userId`: 本地用户 ID
- `peerUserId`: 对端用户 ID
- `credId`: 凭据 ID
- `osAccountId`: OS 账户 ID

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **必须**在删除群组前调用
- **应该**在删除 ACL 后调用

**实际调用位置**:
- `hichain_auth_connector.cpp:DeleteCredential()` - 主要实现
- `softbus_connector.cpp:DeleteCredential()` - 间接调用

---

### 5.9 GetRelatedGroups

**功能**: 获取设备相关的所有群组

**调用场景**:
- 查询设备认证类型
- 设备解绑清理
- 群组同步

**接口签名**:
```cpp
int32_t GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList);
int32_t GetRelatedGroups(int32_t userId, const std::string &deviceId, std::vector<GroupInfo> &groupList);
int32_t GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList);
```

**参数详解**:
- `deviceId`: 设备 ID
- `userId`: 用户 ID
- `groupList`: 输出参数，群组列表

**返回值与错误码**:
- `0`: 成功
- 非 0: 失败

**调用约束**:
- **应该**处理空结果场景

**实际调用位置**:
- `hichain_connector.cpp:GetRelatedGroups()` - 主要实现

---

## 6. SoftBus → DP 接口（DM 侧需要了解）

### 6.1 GetAllAccessControlProfile

**功能**: SoftBus 查询所有 ACL

**调用场景**:
- SoftBus 设备上线验证权限
- SoftBus 建立传输通道前检查 ACL

**接口签名**:
```cpp
std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAccessControlProfile();
```

**DM 处理**:
- DM 提供 `DeviceProfileConnector::GetAllAccessControlProfile()` 接口
- SoftBus 通过 DP 框架调用

---

### 6.2 GetSessionKey

**功能**: SoftBus 获取会话密钥

**调用场景**:
- SoftBus 建立加密传输通道

**接口签名**:
```cpp
int32_t GetSessionKey(int32_t userId, int32_t sessionKeyId, std::vector<unsigned char> &sessionKeyArray);
```

**DM 处理**:
- DM 提供 `DeviceProfileConnector::GetSessionKey()` 接口
- SoftBus 通过 DP 框架调用

---

### 6.3 AuthDeviceProfileListener

**功能**: SoftBus 设备认证监听器

**调用场景**:
- SoftBus 设备认证回调

**DM 处理**:
- DM 实现认证监听器接口
- 处理 SoftBus 触发的认证事件

---

### 6.4 DpAclAdd

**功能**: DP ACL 添加通知

**调用场景**:
- DM 添加 ACL 后通知 SoftBus

**DM 处理**:
- DM 通过 DP 框架通知 SoftBus
- SoftBus 更新本地 ACL 缓存

---

## 7. DP 通知机制

### 7.1 OnTrustDeviceProfileAdd

**功能**: 信任设备配置添加通知

**调用场景**:
- DP 检测到新设备配置添加

**接口签名**:
```cpp
void OnTrustDeviceProfileAdd(const DistributedDeviceProfile::TrustDeviceProfile &profile);
```

**DM 处理**:
- 触发设备上线处理
- 更新设备状态缓存
- 通知应用设备变更

---

### 7.2 OnTrustDeviceProfileUpdate

**功能**: 信任设备配置更新通知

**调用场景**:
- DP 检测到设备配置更新（如设备名变更）

**接口签名**:
```cpp
void OnTrustDeviceProfileUpdate(const DistributedDeviceProfile::TrustDeviceProfile &profile);
```

**DM 处理**:
- 更新设备缓存信息
- 通知应用配置变更

---

### 7.3 OnTrustDeviceProfileDelete

**功能**: 信任设备配置删除通知

**调用场景**:
- DP 检测到设备配置删除

**接口签名**:
```cpp
void OnTrustDeviceProfileDelete(const std::string &deviceId);
```

**DM 处理**:
- 清理设备缓存
- 通知应用设备删除

---

### 7.4 OnTrustDeviceProfileActive

**功能**: 信任设备配置激活通知

**调用场景**:
- DP 检测到设备配置激活（ACL 状态变为 ACTIVE）

**接口签名**:
```cpp
void OnTrustDeviceProfileActive(const std::string &deviceId);
```

**DM 处理**:
- 更新设备在线状态
- 恢复设备服务

---

### 7.5 OnTrustDeviceProfileInactive

**功能**: 信任设备配置去激活通知

**调用场景**:
- DP 检测到设备配置去激活（ACL 状态变为 INACTIVE）

**接口签名**:
```cpp
void OnTrustDeviceProfileInactive(const std::string &deviceId);
```

**DM 处理**:
- 暂停设备服务
- 更新设备状态

---

## 8. 跨模块调用约束与注意事项

### 8.1 调用顺序约束

1. **设备认证流程**:
   ```
   HiChain::CreateGroup → HiChain::AddMember → HiChain::AuthDevice 
   → HiChain::GenerateCredential → HiChain::ImportCredential 
   → DP::PutSessionKey → DP::PutAccessControlProfile 
   → SoftBus::JoinLNN
   ```

2. **设备解绑流程**:
   ```
   DP::DeleteAccessControlProfile → DP::DeleteSessionKey 
   → HiChain::DeleteCredential → HiChain::DeleteGroup 
   → SoftBus::LeaveLNN
   ```

3. **服务绑定流程**:
   ```
   DP::PutSessionKey → DP::PutAccessControlProfile (设备级) 
   → DP::PutAccessControlProfile (应用级) 
   → DP::PutServiceInfo
   ```

---

### 8.2 线程安全考虑

1. **HiChain 回调线程**:
   - HiChain 回调在独立线程执行
   - **必须**使用线程安全的数据结构（如 `ffrt::mutex`）
   - **禁止**在回调中执行耗时操作

2. **DP 操作线程**:
   - DP 接口可能在 DP 服务线程调用
   - **应该**使用异步处理机制
   - **必须**避免死锁（DP 回调 → DM → DP）

3. **SoftBus 回调线程**:
   - SoftBus 回调在 SoftBus 线程执行
   - **应该**快速返回，避免阻塞 SoftBus 线程
   - **必须**使用事件队列异步处理

---

### 8.3 错误处理模式

1. **HiChain 错误处理**:
   - 认证失败**必须**清理中间状态（如群组）
   - **应该**重试超时错误
   - **必须**记录失败原因供故障诊断

2. **DP 错误处理**:
   - 存储失败**必须**回滚相关操作
   - **应该**处理 DP 服务未就绪场景
   - **必须**验证存储结果（如读取验证）

3. **SoftBus 错误处理**:
   - 加入 LNN 失败**应该**延迟重试
   - **必须**处理 NetworkId 变更场景
   - **应该**降级处理（如 WiFi 不可用时使用 BLE）

---

### 8.4 性能优化建议

1. **缓存策略**:
   - **必须**缓存设备 UDID 映射（NetworkId → UDID）
   - **应该**缓存 ACL 信息（定期刷新）
   - **必须**缓存设备在线状态

2. **批量操作**:
   - **应该**批量删除 ACL（避免逐个删除）
   - **应该**批量查询设备信息

3. **异步处理**:
   - **必须**异步处理 DP 通知
   - **应该**异步处理 SoftBus 事件
   - **禁止**在回调中同步等待

---

### 8.5 兼容性考虑

1. **版本协商**:
   - **必须**处理 DM 版本差异（如 5.1.0 新特性）
   - **应该**降级处理旧版本设备

2. **API 变更**:
   - **必须**兼容旧版 SoftBus/DP/HiChain API
   - **应该**使用特性检测而非版本判断

3. **错误码兼容**:
   - **必须**统一处理不同模块错误码
   - **应该**提供统一的错误码映射表

---

### 8.6 安全注意事项

1. **密钥安全**:
   - **必须**保护会话密钥不被泄露
   - **禁止**日志打印密钥内容
   - **应该**及时清理过期密钥

2. **权限验证**:
   - **必须**验证调用者身份（TokenId）
   - **应该**检查操作权限（如绑定级别）

3. **数据保护**:
   - **必须**保护敏感设备信息（UDID）
   - **应该**使用匿名化处理（如日志脱敏）

---

## 9. 附录

### 9.1 错误码参考

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| `DM_OK` (0) | 成功 | - |
| `ERR_DM_FAILED` | 通用失败 | 检查日志定位具体原因 |
| `ERR_DM_INPUT_PARA_INVALID` | 参数错误 | 检查输入参数合法性 |
| `ERR_DM_NOT_FOUND` | 资源不存在 | 检查设备/ACL/群组是否存在 |
| `ERR_DM_ALREADY_EXIST` | 资源已存在 | 检查重复创建场景 |

### 9.2 数据结构定义

**GroupInfo**:
```cpp
struct GroupInfo {
    std::string groupName;      // 群组名称
    std::string groupId;        // �群组 ID
    std::string groupOwner;     // 群组拥有者
    int32_t groupType;          // 群组类型
    int32_t groupVisibility;    // 群组可见性
    std::string userId;         // 用户 ID
};
```

**DmAclInfo**:
```cpp
struct DmAclInfo {
    std::string sessionKey;     // 会话密钥
    int32_t bindType;           // 绑定类型
    int32_t state;              // 状态（ACTIVE/INACTIVE）
    std::string trustDeviceId;  // 信任设备 ID
    int32_t bindLevel;          // 绑定级别
    int32_t authenticationType; // 认证类型
    std::string deviceIdHash;   // 设备 ID 哈希
};
```

### 9.3 参考文档

- SoftBus API 文档
- DeviceProfile API 文档
- HiChain API 文档
- OpenHarmony 分布式设备管理规范

---
**文档结束**
