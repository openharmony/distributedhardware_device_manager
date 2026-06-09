## 概述

Capability `binding-acl-lifetime` 为业务提供"永久可信 ACL 自定义保留时长"配置能力。业务通过 `DmAuthInfo.extraInfo` 整数 key `ACL_LIFE_CYCLE_DAYS` 声明保留天数，DM 客体侧在入口校验、绑定写入、周期清理三处协同消费该字段，并在缺失/非法时保持改前 365 天默认行为。

本 spec 列出 5 个 ADDED Requirements，每条均带可执行的 Scenario（WHEN/THEN 写法）作为最小验收单元。

## 用户故事或场景

- **US-1**：作为长生命周期 IoT 设备业务方，我希望永久可信 ACL 能保留 3 年（1095 天），以匹配设备实际生命周期；当前 365 天到期被清理会让设备意外掉信任。
- **US-2**：作为风控业务方，我希望可以将永久可信 ACL 保留期收紧到更短的天数；当前唯一选项是 365 天，缺乏调节灵活性。
- **US-3**：作为未感知该能力的旧业务方，我希望不修改任何调用即可保持原有行为（365 天）。
- **US-4**：作为 DM 维护者，我希望非法/错配的策略输入在入口被严格拒绝（而非沉默落库），便于业务尽早暴露问题。

## 验收标准

| AC ID | 验收标准 | 关联 Requirement |
|-------|---------|-----------------|
| AC-1 | 业务以整数 `ACL_LIFE_CYCLE_DAYS ∈ [1, 3650]` 调用 `ImportAuthInfo`（`authType == TRUST_ALWAYS`），校验通过，绑定后 ACL `AccessControlProfile.extraData` JSON 含同名整数 | R-1, R-3 |
| AC-2 | `authType != TRUST_ALWAYS` 时携带正数天数 → `IsImportAuthInfoValid` 返回 `ERR_DM_INPUT_PARA_INVALID` | R-2 |
| AC-3 | `extraInfo` 缺失 `ACL_LIFE_CYCLE_DAYS` 或显式置 `-1` → 校验通过 → ACL 不含该字段 → 清理 fallback 365 天 | R-1, R-3, R-5 |
| AC-4 | ACL `AccessControlProfile.extraData` 含合法整数 N → `GetAclAllowSeconds` 返回 `N * 86400`；字段缺失/非整数/超界/malformed → 返回 `MAX_ALWAYS_ALLOW_SECONDS = 31536000` | R-4 |
| AC-5 | 旧版本服务读新 ACL 自动忽略 `ACL_LIFE_CYCLE_DAYS`，新版本服务读旧 ACL 也走 365 天 fallback（双向兼容） | R-5 |

## 业务规则

| 规则 ID | 描述 |
|---------|------|
| BR-1 | `ACL_LIFE_CYCLE_DAYS` 仅对 `authType == TRUST_ALWAYS (6)` 生效；其它 authType 必须不携带或携带哨兵 `-1` |
| BR-2 | 合法天数范围：`[1, 3650]`（含两端）。`0` 或 `> 3650` 视为非法 |
| BR-3 | 仅持久化输入参数 `ACL_LIFE_CYCLE_DAYS` 一个权威字段；过期时刻由清理路径运行时由 `aclCreateTime + days*86400` 推算（不冗余写入 `EXPIRE_TIMESTAMP`） |
| BR-4 | 该字段属于**关系级属性**，仅写入 `AccessControlProfile.extraData`，不写入 `accesser.extraData` 或 `accessee.extraData`（这两表只装实体身份信息） |
| BR-5 | 业务想重置已建 ACL 的保留期需走解绑 + 重新绑定流程；本 change 不支持批量回填 |

## 异常或边界规则

| 异常/边界 | 处理 |
|----------|------|
| `extraInfo` 不是合法 JSON | `IsImportAuthInfoValid` 入口的 `JsonObject::Parse + IsDiscarded` 短路 → 拒绝 |
| `ACL_LIFE_CYCLE_DAYS` value 是字符串 / bool / 浮点 | `IsNumberInteger` 为 false → 入口拒绝；清理读取处 → fallback 365 天 |
| `ACL_LIFE_CYCLE_DAYS == 0` | 不在 `[1, 3650]` 且非 `-1` 哨兵 → 拒绝 |
| `ACL_LIFE_CYCLE_DAYS == 3651` | 超界 → 拒绝；清理读取处 → fallback 365 天 |
| `ACL_LIFE_CYCLE_DAYS == -1` | 视为"业务未配置"哨兵 → 校验通过 + 绑定不写入 + 清理按 365 天 |
| `AccessControlProfile.extraData` 本身是 malformed JSON | writer 用 `Parse + IsDiscarded` 短路并退化为空 JSON 重新写入；reader 同样 fallback 365 天，不触发 `-fno-exceptions` abort |
| 非永久可信路径 (`confirmOperation != ALLOW_AUTH_ALWAYS`) | 不写入 ACL；`DeleteAlwaysAllowTimeOut` 不进入该分支 |

## 兼容性声明

- **向后兼容**：业务未配置（缺失或 `-1`）→ 全链路行为与改前 100% 等价，永久可信 ACL 保留 365 天
- **跨版本兼容**：旧版本服务读到新版本写入、含 `ACL_LIFE_CYCLE_DAYS` 的 ACL → 旧版本忽略该字段并按 `MAX_ALWAYS_ALLOW_SECONDS` 清理（早 fallback 行为安全，无数据损坏）；新版本服务读到不含该字段的旧 ACL → 同样 fallback 365 天
- **数据/Schema 兼容**：不修改 DP 数据库 schema；只在已有 `AccessControlProfile.extraData` JSON 中新增一个可选整数 key
- **API/SDK 兼容**：不修改 `DmAuthInfo` 结构体；不修改 `ImportAuthInfo` 接口签名；不修改 IPC code

## 验证映射

下方 5 个 ADDED Requirements 中的每个 Scenario 即为最小验证单元；对应的 GoogleTest 用例集见 `execution-plan.md` 的"代码范围映射"与"AC 到 Task 追溯"。



### Requirement: 业务可通过 extraInfo 配置永久可信 ACL 自定义保留时长

DM 服务 SHALL 允许业务在客体侧通过 `ImportAuthInfo` 接口预置策略时，在 `DmAuthInfo.extraInfo` JSON 中以整数 key `ACL_LIFE_CYCLE_DAYS` 声明永久可信 ACL 的自定义保留时长（天）。合法取值范围为 `[1, 3650]`；哨兵值 `-1` 表示"业务未配置"。

#### Scenario: 永久可信类型 + 合法天数通过校验
- **WHEN** 业务调用 `ImportAuthInfo`，`authType == TRUST_ALWAYS (6)` 且 `extraInfo` 含整数 `"ACL_LIFE_CYCLE_DAYS": 1095`
- **THEN** `IsImportAuthInfoValid` 返回 `DM_OK`，策略被持久化到 DP `LocalServiceInfo`

#### Scenario: 永久可信类型 + 哨兵值通过校验
- **WHEN** 业务调用 `ImportAuthInfo`，`authType == TRUST_ALWAYS` 且 `extraInfo` 含整数 `"ACL_LIFE_CYCLE_DAYS": -1`
- **THEN** `IsImportAuthInfoValid` 返回 `DM_OK`，且该 import 在绑定时不写入 ACL（等同未配置）

#### Scenario: 永久可信类型 + 缺失 key 通过校验
- **WHEN** 业务调用 `ImportAuthInfo`，`authType == TRUST_ALWAYS` 且 `extraInfo` 不含 `ACL_LIFE_CYCLE_DAYS` key
- **THEN** `IsImportAuthInfoValid` 返回 `DM_OK`，绑定后 ACL 保留 365 天默认值

### Requirement: authType 与 ACL_LIFE_CYCLE_DAYS 的交叉校验

DM 服务 SHALL 在 `IsImportAuthInfoValid` 中按下表强制 `authType × ACL_LIFE_CYCLE_DAYS` 交叉约束。非永久可信路径不允许配置正数保留天数。

| authType | ACL_LIFE_CYCLE_DAYS | 校验结果 |
|----------|---------------------|---------|
| `TRUST_ALWAYS (6)` | 整数 ∈ [1, 3650] | 通过 |
| `TRUST_ALWAYS (6)` | -1 / 缺失 | 通过（fallback 默认 365 天） |
| 非 6 | 整数 ∈ [1, 3650] | **拒绝**（`ERR_DM_INPUT_PARA_INVALID`） |
| 非 6 | -1 / 缺失 | 通过 |
| 任何 | 0 / 3651 / 字符串 / malformed JSON | **拒绝**（`ERR_DM_INPUT_PARA_INVALID`） |

#### Scenario: 一次性可信携带正数天数被拒
- **WHEN** 业务调用 `ImportAuthInfo`，`authType == TRUST_ONETIME` 且 `extraInfo` 含 `"ACL_LIFE_CYCLE_DAYS": 1095`
- **THEN** `IsImportAuthInfoValid` 返回 `ERR_DM_INPUT_PARA_INVALID`

#### Scenario: 超界值被拒
- **WHEN** 业务调用 `ImportAuthInfo`，`authType == TRUST_ALWAYS` 且 `extraInfo` 含 `"ACL_LIFE_CYCLE_DAYS": 3651`
- **THEN** `IsImportAuthInfoValid` 返回 `ERR_DM_INPUT_PARA_INVALID`

#### Scenario: 字符串类型被拒
- **WHEN** 业务调用 `ImportAuthInfo`，`extraInfo` 含字符串 `"ACL_LIFE_CYCLE_DAYS": "1095"`（非整数）
- **THEN** `IsImportAuthInfoValid` 返回 `ERR_DM_INPUT_PARA_INVALID`（`IsNumberInteger` 为 false）

#### Scenario: 零值被拒
- **WHEN** 业务调用 `ImportAuthInfo`，`extraInfo` 含 `"ACL_LIFE_CYCLE_DAYS": 0`
- **THEN** `IsImportAuthInfoValid` 返回 `ERR_DM_INPUT_PARA_INVALID`（不在 [1, 3650] 且非 -1 哨兵）

### Requirement: 永久可信绑定时将保留时长写入 ACL AccessControlProfile.extraData

DM 客体侧 `authentication_v2` 在 ACL 写入阶段，当本次绑定最终判定为永久可信（`context->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS`）且 `context->aclLifeCycleDays ∈ [1, 3650]` 时，SHALL 将该整数以 key `ACL_LIFE_CYCLE_DAYS` 写入 ACL `AccessControlProfile.extraData` JSON。不得写入 `accesser` 侧。

#### Scenario: 永久可信 + 合法天数写入 AccessControlProfile.extraData
- **WHEN** `confirmOperation == ALLOW_AUTH_ALWAYS` 且 `aclLifeCycleDays == 1095`
- **THEN** 最终 ACL `AccessControlProfile.extraData` JSON 包含整数 `"ACL_LIFE_CYCLE_DAYS": 1095`；`accesser.extraData` 与 `accessee.extraData` 均不含该 key

#### Scenario: 哨兵值不触发写入
- **WHEN** `confirmOperation == ALLOW_AUTH_ALWAYS` 且 `aclLifeCycleDays == -1`（即业务未配置）
- **THEN** 最终 ACL `AccessControlProfile.extraData` 不含 `ACL_LIFE_CYCLE_DAYS` key

#### Scenario: 非永久可信路径不写入
- **WHEN** `confirmOperation != ALLOW_AUTH_ALWAYS`（例如 `ALLOW_AUTH` 一次性可信、`CANCEL` 取消）
- **THEN** 无论 `aclLifeCycleDays` 取何值，ACL `AccessControlProfile.extraData` 不含 `ACL_LIFE_CYCLE_DAYS` key

#### Scenario: ACL 写入不解析既有 malformed ExtraData
- **WHEN** 本次绑定需要写入 `ACL_LIFE_CYCLE_DAYS`
- **THEN** writer 使用本次绑定流程构造的 `AccessControlProfile.extraData` JSON 写入关系级策略，不解析旧 `profile.GetExtraData()`，不触发 `-fno-exceptions` abort

### Requirement: 客体侧周期清理按 ACL 自带保留时长判过期

DM 客体侧 `DeleteAlwaysAllowTimeOut` 在判定永久可信 ACL 是否过期时，SHALL 通过 helper `GetAclAllowSeconds(const AccessControlProfile&)` 读取 ACL `AccessControlProfile.extraData` 中的 `ACL_LIFE_CYCLE_DAYS` 整数字段并按 `days * 86400` 秒计算保留时长。当字段缺失、非整数、超界（`< 1 || > 3650`）、或 base JSON 解析失败时，helper SHALL fallback 到 `MAX_ALWAYS_ALLOW_SECONDS = 31536000` 秒（365 天）。

#### Scenario: 合法天数返回 days*86400 秒
- **WHEN** ACL `AccessControlProfile.extraData` 含整数 `"ACL_LIFE_CYCLE_DAYS": 1095`
- **THEN** `GetAclAllowSeconds` 返回 `1095 * 86400 = 94608000`

#### Scenario: 缺失 key fallback 365 天
- **WHEN** ACL `AccessControlProfile.extraData` 不含 `ACL_LIFE_CYCLE_DAYS` key
- **THEN** `GetAclAllowSeconds` 返回 `MAX_ALWAYS_ALLOW_SECONDS = 31536000`（365 天）

#### Scenario: 非整数类型 fallback 365 天
- **WHEN** ACL `AccessControlProfile.extraData` 含字符串 `"ACL_LIFE_CYCLE_DAYS": "1095"`
- **THEN** `GetAclAllowSeconds` 返回 `31536000`（`IsNumberInteger` 为 false 触发 fallback）

#### Scenario: 超界值 fallback 365 天
- **WHEN** ACL `AccessControlProfile.extraData` 含整数 `"ACL_LIFE_CYCLE_DAYS": 3651`
- **THEN** `GetAclAllowSeconds` 返回 `31536000`

#### Scenario: malformed JSON fallback 365 天
- **WHEN** ACL `AccessControlProfile.extraData` 为非法 JSON 串
- **THEN** `GetAclAllowSeconds` 返回 `31536000`，记录 INFO 日志，不崩溃

#### Scenario: 非同账号 ACL 使用统一清理 helper
- **WHEN** ACL `bindType` 不是 `DM_IDENTICAL_ACCOUNT`
- **THEN** `DeleteAlwaysAllowTimeOut` 调用 `GetAclAllowSeconds` 读取 ACL 关系级策略；未配置或非法值 fallback 到 365 天

### Requirement: 未配置 ACL_LIFE_CYCLE_DAYS 时保持向后兼容

当业务未在 `extraInfo` 中配置 `ACL_LIFE_CYCLE_DAYS`（缺失或值为 `-1`）时，DM 服务 SHALL 保持改前完全等价的行为：永久可信 ACL 保留时长为 365 天。

#### Scenario: 旧业务零修改正常工作
- **WHEN** 旧版本业务调用 `ImportAuthInfo` 且 `extraInfo` 不含 `ACL_LIFE_CYCLE_DAYS`
- **THEN** 入口校验通过 → 绑定流程不写入 `AccessControlProfile.extraData` → 周期清理 fallback 365 天 → 最终行为与改前一致

#### Scenario: 旧版本服务读新 ACL 兼容
- **WHEN** 旧版本服务读到由新版本写入、含 `ACL_LIFE_CYCLE_DAYS` 的 ACL
- **THEN** 旧版本忽略该字段，仍按 `MAX_ALWAYS_ALLOW_SECONDS` 清理（行为安全，无数据破坏）

## 代码映射

| Spec Item | Code Ref (file:symbol) | Verification |
|-----------|------------------------|--------------|
| R-1 业务声明字段 | `common/include/dm_constants.h::ACL_LIFE_CYCLE_DAYS` / `ACL_LIFE_CYCLE_DAYS_NOT_CONFIGURED` / `_MIN` / `_MAX` | 编译期常量；UT 间接覆盖 |
| R-2 入口交叉校验 | `services/service/src/device_manager_service.cpp::IsImportAuthInfoValid` / `IsAclLifeCycleDaysValid` | `UTTest_device_manager_service_three` 10 用例 |
| R-3a context 解析 | `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp::ProcessImportAuthInfo` + 匿名 `ExtractAclLifeCycleDays` | `UTTest_auth_confirm` 5 用例 |
| R-3b ACL 写入 | `services/implementation/src/authentication_v2/dm_auth_message_processor.cpp::PutNonLnnAclProfile`（在已有 `extraData[ACL_IS_LNN_ACL_KEY]=...` 之后追加 `extraData[ACL_LIFE_CYCLE_DAYS]=context->aclLifeCycleDays`，最终 `profile.SetExtraData(extraData.Dump())`） | `UTTest_dm_auth_message_processor` 5 用例 |
| R-4 清理读取 | `services/implementation/src/device_manager_service_impl.cpp::DeleteAlwaysAllowTimeOut` + `GetAclAllowSeconds` | `UTTest_device_manager_service_impl_first` 6 用例 |
| R-5 向后兼容 | 由 R-2/R-3/R-4 的 fallback 路径共同保证 | 上述 4 个 UT 套件中专项 fallback 用例 |

详细的 AC ↔ Task ↔ 代码范围 ↔ 验证命令对照见 `execution-plan.md`。
