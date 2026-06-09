# 架构设计决策记录 (ADR)

> **版本**: v2.0
> **更新日期**: 2026-05-19

---

本文档记录 DeviceManager 的关键架构决策，包括每个决策的背景、内容和影响。

---

## ADR-001: 认证架构双版本（v1/v2）并存

### 元信息
- **日期**: 2024-2025（v2 引入）
- **状态**: 现行策略

### 背景
- **问题**: v1 认证架构存在状态机复杂、扩展困难、代码耦合度高等问题
- **驱动因素**:
  - 新增认证方式（凭证认证、PIN 码认证）需要更好的扩展性
  - 状态转换逻辑复杂导致维护成本高
  - 代码复用率低，多种认证方式存在大量重复代码
- **技术约束**:
  - v1 已有大量设备存量，无法直接替换
  - 需要保证向后兼容性

### 决策
采用 v1/v2 双版本并存架构：

```
3rd/services/implementation/
├── authentication_v1/          # v1 认证架构
│   ├── auth_manager.cpp
│   ├── dm_auth_state_machine.cpp
│   └── ...
└── authentication_v2/          # v2 认证架构
    ├── authdevicewithacl/      # ACL 认证
    ├── authcredential/         # 凭证认证
    ├── authpincode/           # PIN 码认证
    └── commonbase/            # 公共基类
```

**v2 架构特点**：
- 每种认证方式独立目录（`authdevicewithacl/`、`authcredential/`、`authpincode/`）
- 状态机通过模板化设计提高复用性
- 上下文和消息处理器独立封装

### 影响
**正面影响**：
- ✅ 新认证方式扩展更容易（新增目录即可）
- ✅ 代码复用率提高（公共基类 `dm_auth_manager_base_3rd`）
- ✅ 状态机逻辑更清晰（模板化状态转换）

**负面影响**：
- ❌ 维护成本增加（同时维护两套代码）
- ❌ 代码库体积增大（约 2 倍）
- ❌ 新人学习曲线陡峭

**迁移路径**：
- 逐步将 v1 认证方式迁移到 v2 架构
- 新功能仅在 v2 实现，v1 保持稳定

---

## ADR-002: 同账号免交互认证引入

### 元信息
- **日期**: 2023-2024
- **状态**: 已实现

### 背景
- **问题**: 同账号设备间认证仍需用户手动确认，体验不佳
- **需求场景**:
  - 用户登录同一华为账号的多设备应自动信任
  - 减少用户手动操作，提升跨设备体验流畅性
- **安全考量**:
  - 账号级别的身份验证已由账号系统保证
  - 设备间信任可基于账号关系自动建立

### 决策
引入 `IDENTICAL_ACCOUNT` 信任类型，支持免交互认证：

```cpp
// 信任类型定义
typedef enum DmAuthForm {
    PEER_TO_PEER = 0,       // 点对点信任（需手动确认）
    IDENTICAL_ACCOUNT = 1,  // 同账号信任（免交互）
    ACROSS_ACCOUNT = 2,     // 跨账号信任
    SHARE = 3,              // 分享信任
} DmAuthForm;
```

**实现机制**：
- 认证发起时检查双方 `AccountId` 是否一致
- 一致时自动选择 `IDENTICAL_ACCOUNT` 类型
- 跳过用户确认步骤，直接完成认证

### 影响
**正面影响**：
- ✅ 用户体验显著提升（无需手动确认）
- ✅ 跨设备协同更流畅（自动发现并认证）
- ✅ 降低认证失败率（减少用户操作失误）

**负面影响**：
- ❌ 安全风险略有增加（依赖账号系统安全性）
- ❌ 设备丢失后需远程解绑（否则仍可认证）

**安全缓解措施**：
- 设备数量上限（同账号最多 128 台设备）
- 异常登录检测（账号系统层）
- 支持远程解绑（云端设备管理）

---

## ADR-003: ACL 三阶段老化设计

### 元信息
- **日期**: 2025（5.1.0 版本引入）
- **状态**: 已实现

### 背景
- **问题**: SessionKey 生命周期管理僵化，无法平衡安全性与用户体验
- **原有问题**:
  - 立即删除：用户体验差（频繁需重新认证）
  - 永久保留：安全风险高（密钥泄露风险）
  - 固定期限：无法适应不同使用频率

### 决策
采用三阶段老化机制：

```
Active (活跃期) → Transition (过渡期) → Cache (缓存期) → 删除
```

**各阶段特点**：

| 阶段 | 时长 | 密钥状态 | 认证行为 | 恢复方式 |
|------|------|----------|----------|----------|
| **Active** | 无固定时长 | 有效 | 正常使用 | 无需恢复 |
| **Transition** | 默认 2 天 | 缓存到 UKV | 免交互认证 | 重认证回到 Active |
| **Cache** | 到期后清理 | 仅凭证 | 需重认证 | 完整认证流程 |

**代码实现**（`deviceprofile_connector.cpp`）：
```cpp
const int64_t ACTIVE_TIMEOUT = 48 * 3600;        // 48小时
const int64_t TRANSITION_TIMEOUT = 120 * 3600;   // 120秒（用于过渡期超时判断）

// 老化判断逻辑
if (keyAge > ACTIVE_TIMEOUT && profile.getSessionKeyId() > 0) {
    // Phase 2: Transition → UKV 缓存
    InsertUserKeyToUKCache(profile);
} else if (keyAge > TRANSITION_TIMEOUT) {
    // Phase 3: Cache → 清理
    DeleteSessionKey(profile);
}
```

### 影响
**正面影响**：
- ✅ 安全性与体验平衡（活跃期安全，过渡期便捷）
- ✅ 高频使用设备免重复认证（过渡期内免交互）
- ✅ 低频设备自动清理（释放资源）

**负面影响**：
- ❌ 逻辑复杂度增加（三阶段状态管理）
- ❌ 需维护时间戳（SessionKey 创建时间）
- ❌ 调试难度增加（老化问题难以复现）

---

## ADR-004: 第三方认证插件框架

### 元信息
- **日期**: 2023-2024
- **状态**: 已实现

### 背景
- **问题**: 认证方式硬编码，扩展性差
- **需求**:
  - 允许第三方实现自定义认证方式
  - 核心代码与具体认证逻辑解耦
- **参考模式**: Strategy Pattern（策略模式）

### 决策
采用插件化架构，第三方认证方式独立实现：

```
3rd/services/implementation/
├── authdevicewithacl/    # ACL 认证（默认）
├── authcredential/       # 凭证认证（第三方扩展）
└── authpincode/         # PIN 码认证（第三方扩展）
```

**公共基类**（`dm_auth_manager_base_3rd.h`）：
```cpp
class AuthManagerBase3rd {
public:
    virtual int32_t AuthDevice3rd(
        const PeerTargetId3rd &targetId,
        const std::map<std::string, std::string> &authParam,
        int32_t sessionId, uint64_t logicalSessionId) = 0;
    
    virtual void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) = 0;
    virtual void OnDataReceived(int32_t sessionId, const std::string &message) = 0;
    // ... 其他虚函数
};
```

**插件实现示例**（凭证认证）：
```cpp
class AuthManagerCred : public AuthManagerBase3rd {
public:
    int32_t AuthDevice3rd(...) override;
    void OnSessionOpened(...) override;
    // 实现凭证认证特定逻辑
};
```

### 影响
**正面影响**：
- ✅ 扩展性强（新增认证方式无需修改核心代码）
- ✅ 代码隔离（不同认证方式互不影响）
- ✅ 第三方友好（可独立开发和测试）

**负面影响**：
- ❌ 接口设计复杂（需考虑所有认证方式需求）
- ❌ 调试困难（跨多个目录和类）
- ❌ 性能开销（虚函数调用）

---

## ADR-005: 双层上下线模型

### 元信息
- **日期**: 2022-2023
- **状态**: 已实现

### 背景
- **问题**: 物理连接不等同于逻辑可用
- **场景**:
  - 设备网络已连接，但分布式服务未就绪
  - 物理离线后，逻辑状态仍需短暂保留
- **依赖关系**:
  - SoftBus 负责物理连接管理
  - DM 负责逻辑设备状态管理

### 决策
采用物理/逻辑双层模型：

```
SoftBus (物理层)              DM (逻辑层)
════════════════════════════════════════
WiFi/BLE 连接              ONLINE
       ↓                    ↓
  Session 建立          READY
       ↓                    ↓
  Session 断开          OFFLINE
```

**状态定义**（`dm_app_manager.cpp`）：
```cpp
typedef enum {
    DEVICE_STATE_UNKNOWN = -1,   // 未知状态
    DEVICE_STATE_ONLINE = 0,     // 设备物理在线（但 DDS 未同步）
    DEVICE_INFO_READY = 1,       // 设备就绪（DDS 已同步，可用）
    DEVICE_STATE_OFFLINE = 2,    // 设备离线
} DeviceState;
```

**状态转换条件**：
- `UNKNOWN → ONLINE`: SoftBus 设备上线事件
- `ONLINE → READY`: DDS 设备信息同步完成
- `READY → OFFLINE`: SoftBus 设备下线事件（延迟 60 秒）
- `ONLINE → OFFLINE`: 快速下线（未完成初始化）

### 影响
**正面影响**：
- ✅ 状态更准确（区分物理在线和逻辑可用）
- ✅ 用户体验更好（避免过早显示不可用设备）
- ✅ 容错性强（处理 DDS 同步延迟）

**负面影响**：
- ❌ 复杂度增加（两层状态协调）
- ❌ 时序问题（SoftBus 和 DM 状态可能不一致）
- ❌ 调试困难（需同时监控两层状态）

---

## ADR-006: 认证超时分级设计

### 元信息
- **日期**: 2023-2024
- **状态**: 已实现

### 背景
- **问题**: 单一超时时间无法适应不同认证阶段
- **需求**:
  - 快速失败（避免用户长时间等待）
  - 充分时间（避免网络波动导致失败）

### 决策
采用超时分级设计（`dm_auth_manager_base_3rd.cpp`）：

```cpp
// 认证各阶段超时定义
const int32_t AUTHENTICATE_TIMEOUT = 120;      // 完整认证超时：120秒
const int32_t CONFIRM_TIMEOUT = 60;            // 用户确认超时：60秒
const int32_t NEGOTIATE_TIMEOUT = 10;          // 协商阶段超时：10秒
const int32_t INPUT_TIMEOUT = 60;              // 输入超时：60秒
const int32_t ADD_TIMEOUT = 10;                // 添加设备超时：10秒
const int32_t WAIT_NEGOTIATE_TIMEOUT = 10;     // 等待协商超时：10秒

// Session 相关超时
const int32_t OPEN_AUTH_SESSION_TIMEOUT = 15000;  // Session 开启超时：15秒
```

**超时任务命名**：
- `AUTHENTICATE_TIMEOUT_TASK`: 完整认证流程超时
- `CONFIRM_TIMEOUT_TASK`: 用户确认超时
- `NEGOTIATE_TIMEOUT_TASK`: 协商阶段超时
- `INPUT_TIMEOUT_TASK`: PIN 码输入超时
- `SESSION_HEARTBEAT_TIMEOUT_TASK`: Session 心跳超时

### 影响
**正面影响**：
- ✅ 快速失败（避免无效等待）
- ✅ 用户体验好（超时提示清晰）
- ✅ 资源及时释放（超时后清理 Session）

**负面影响**：
- ❌ 超时时间难以调优（网络环境差异）
- ❌ 假阳性风险（网络慢时误判失败）

---

## ADR-007: PIN 码超时机制

### 元信息
- **日期**: 2023-2024
- **状态**: 已实现

### 背景
- **问题**: PIN 码展示后无超时，存在安全风险
- **风险**:
  - PIN 码长期有效，可能被他人使用
  - 设备未及时认证，PIN 码仍可输入

### 决策
实现 PIN 码超时机制：

**超时设置**（`dm_auth_manager_base_3rd.cpp`）：
```cpp
const int32_t INPUT_TIMEOUT = 60;  // PIN 码输入超时：60秒
const char* INPUT_TIMEOUT_TASK = "deviceManagerTimer:input";
```

**超时处理**：
- 认证发起端展示 PIN 码后启动 60 秒倒计时
- 超时后自动取消认证流程
- 用户需重新发起认证

### 影响
**正面影响**：
- ✅ 安全性提升（降低 PIN 码泄露风险）
- ✅ 用户体验优化（及时反馈）

**负面影响**：
- ❌ 可能导致认证失败（用户操作慢）

---

## ADR-008: 代理认证支持

### 元信息
- **日期**: 2025
- **状态**: 已实现

### 背景
- **问题**: 系统应用需要代表普通应用执行认证
- **场景**:
  - 系统应用统一管理设备认证
  - 普通应用无权限直接调用 DM 服务

### 决策
引入代理认证机制：

**相关常量**（`dm_constants_3rd.h`）：
```cpp
const char* TAG_PROXY = "proxy";
const char* TAG_PROXY_TOKEN_ID = "proxyTokenId";
const char* TAG_PROXY_PROCESS_NAME = "proxyProcessName";
const char* TAG_IS_PROXY_DELETE = "isProxyDelete";
const char* PARAM_KEY_IS_PROXY_BIND = "isProxyBind";
const char* PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT = "isCallingProxyAsSubject";
```

**实现逻辑**：
- 系统应用在认证参数中标记 `isProxyBind = true`
- DM 服务根据 `proxyTokenId` 和 `proxyProcessName` 验证权限
- 认证完成后将设备信息授权给普通应用

### 影响
**正面影响**：
- ✅ 权限管理更灵活（系统统一管理）
- ✅ 普通应用开发简化（无需处理认证）

**负面影响**：
- ❌ 复杂度增加（代理上下文管理）
- ❌ 安全风险（权限验证逻辑复杂）

---

## ADR-009: 开源/闭源版本编译宏隔离

### 元信息
- **日期**: 2024（`device_manager_common` 引入）
- **状态**: 现行策略

### 背景
- **问题**: 同一份 DM 源码需同时支持开源版本（如 RK3568 标准镜像）和闭源版本（含 vendor `distributed_hardware_adapter` 组件 + `libdevicemanagerresident.z.so`），两者能力差异大
- **驱动因素**:
  - 闭源版有 vendor 私有的单/双设备兼容适配、服务注册/发现扩展、设备图标等能力
  - 开源版没有 `libdevicemanagerresident.z.so`，调用相关接口会失败
  - 不能强制开源用户分发闭源 so，也不能在源码里 fork 两份维护
- **技术约束**:
  - 构建系统是 GN，可在 `defined(global_parts_info.<component>)` 上做条件
  - 同一份 .cpp/.h 需在两种产品形态下都能编译通过

### 决策
引入 GN 变量 `device_manager_common` 与编译宏 `DEVICE_MANAGER_COMMON_FLAG`，配套使用：

```gni
# device_manager.gni
if (defined(global_parts_info) &&
    defined(global_parts_info.distributedhardware_distributed_hardware_adapter)) {
  device_manager_common = false   # 闭源
} else {
  device_manager_common = true    # 开源
}
```

```gni
# services/service/BUILD.gn / services/implementation/BUILD.gn
if (device_manager_common) {
  defines += [ "DEVICE_MANAGER_COMMON_FLAG" ]
}
```

**约定**：
- 宏存在 = 开源版本（跳过闭源专属代码、提供开源 fallback）
- 宏不存在 = 闭源版本（含 vendor adapter resident so）

### 影响

**正面影响**：
- ✅ 同一份代码支持两种产品形态
- ✅ 开源用户拿到的源码在 RK3568 等标准镜像上可直接编译运行
- ✅ 闭源 vendor 能力不向开源用户暴露

**负面影响**：
- ❌ 增加代码可读性负担（多处 `#ifdef`/`#ifndef`）
- ❌ 需同时维护两套等价行为路径
- ❌ 新增代码必须主动判断"是否涉及闭源资源"，遗漏会导致开源版编译失败或运行时挂死

**详细机制**：见 [docs/09-opensource-closedsource-isolation.md](09-opensource-closedsource-isolation.md)，覆盖：
- 所有 `DEVICE_MANAGER_COMMON_FLAG` 使用点的完整清单
- `ExportAuthInfo`/`ImportAuthInfo` 两段式执行与 IPC stub 不回传 `DmAuthInfo` 的关键约束
- 新增代码时的判断规则

---

## 总结

| ADR | 决策要点 | 关键权衡 |
|-----|---------|---------|
| ADR-001 | v1/v2 双版本并存 | 扩展性 vs 维护成本 |
| ADR-002 | 同账号免交互认证 | 用户体验 vs 安全风险 |
| ADR-003 | ACL 三阶段老化 | 安全性 vs 便捷性 |
| ADR-004 | 第三方认证插件框架 | 扩展性 vs 复杂度 |
| ADR-005 | 双层上下线模型 | 状态准确性 vs 实现复杂度 |
| ADR-006 | 认证超时分级设计 | 快速失败 vs 容错性 |
| ADR-007 | PIN 码超时机制 | 安全性 vs 用户体验 |
| ADR-008 | 代理认证支持 | 权限灵活性 vs 安全复杂度 |
| ADR-009 | 开源/闭源版本编译宏隔离 | 源码统一 vs 可读性 |

**设计原则总结**：
1. **向后兼容**: v1/v2 并存保证平滑升级
2. **扩展优先**: 插件化架构支持未来演进
3. **安全平衡**: 在安全性和体验间寻找平衡点
4. **状态精确**: 双层模型准确反映设备状态
5. **源码统一**: 通过编译宏在同一份源码中兼顾开源与闭源两种产品形态
