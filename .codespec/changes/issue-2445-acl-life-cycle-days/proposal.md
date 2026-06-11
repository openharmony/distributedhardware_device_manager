---
target_release: OpenHarmony-6.0-Release
---

# Proposal: 业务可配置的永久可信 ACL 保留时长

## 背景与问题

DM 组件在客体侧绑定流程中，"永久可信"关系（`authType = TRUST_ALWAYS = 6`，即 `DMLocalServiceInfoAuthType::TRUST_ALWAYS`）所产生的 ACL 在客体侧周期清理中按硬编码常量保留：

- 常量：`MAX_ALWAYS_ALLOW_SECONDS = 31536000`（365 天）
- 落点：`services/implementation/src/device_manager_service_impl.cpp` 中 `DeleteAlwaysAllowTimeOut`

业务方无法按自身安全策略调整该保留期：

- 部分业务诉求保留 3 年（1095 天）以支撑长生命周期 IoT 设备
- 部分业务诉求收紧到更短的保留期以满足风控要求

需要在客体侧打通"业务声明保留时长 → 入口校验 → 绑定流转 → ACL 持久化 → 周期清理"的端到端链路，并保持完全向后兼容。

## 目标

- 业务可通过 `DmAuthInfo.extraInfo` JSON 中的整数 key `ACL_LIFE_CYCLE_DAYS`（合法范围 `[1, 3650]`）配置永久可信 ACL 自定义保留时长
- 在 `IsImportAuthInfoValid` 入口对 `authType × ACL_LIFE_CYCLE_DAYS` 做交叉校验，非永久可信路径不允许配置正数保留天数
- 客体侧 `authentication_v2` 在最终判定为永久可信时把该天数写入 ACL `AccessControlProfile.extraData`
- 客体侧 `DeleteAlwaysAllowTimeOut` 按 ACL 自带的天数判过期；缺失/非法时 fallback 365 天
- 未配置时永久可信 ACL 保留时长仍为 365 天，行为完全等价于改前

## 非目标

- 不修改 SDK 接口签名（沿用 `ImportAuthInfo`）
- 不修改 DP 数据库 schema
- 不修改主端（source side）逻辑
- 不修改老协议路径（`authentication/dm_auth_manager.cpp`）
- 不修改 3rd 栈（`3rd/`）
- 不实现"已有 ACL 回填"——业务改规则后只影响后续新建的 ACL
- 不纳入相邻扩展项：PIN 码多次绑定（`PINCODE_POLICY_REUSABLE`）、`ClearAuthInfo` 策略清理接口、组合策略需另立 change 处理

## 范围

涉及子系统：`distributedhardware`

涉及仓库：`foundation/distributedhardware/device_manager`

涉及模块与改动类型：

| 模块 | 改动类型 |
|------|---------|
| `common/include/dm_constants.h` + `common/src/dm_constants.cpp` | 新增 4 个常量 |
| `services/service/src/device_manager_service.cpp` | `IsImportAuthInfoValid` 扩展交叉校验 |
| `services/implementation/src/authentication_v2/auth_stages/auth_confirm.cpp` | `DmAuthContext` 新增字段 + extraInfo 解析 |
| `services/implementation/src/authentication_v2/dm_auth_message_processor.cpp` | ACL 写入流程写入 `AccessControlProfile.extraData` |
| `services/implementation/src/device_manager_service_impl.cpp` | 新增 `GetAclAllowSeconds` helper，替换 `DeleteAlwaysAllowTimeOut` 硬编码 |
| `test/unittest/` | 新增 26 个 GoogleTest 用例 |

## 组件层影响

| 层级 | 是否变更 | 说明 |
|------|---------|------|
| 业务应用层 | 否 | 复用现有 `ImportAuthInfo`，仅在 `DmAuthInfo.extraInfo` 中新增整数 key |
| DM 服务层 | 是 | 入口校验、authentication_v2 sink 端 context 解析与 ACL 写入、`DeleteAlwaysAllowTimeOut` 清理读取 |
| DP 服务层 | 否 | 复用 `AccessControlProfile.extraData` JSON 字段承载策略快照，不修改 DP schema |
| dsoftbus 层 | 否 | 设备发现、连接、传输无变化 |

## 验收基线

- AC-1：业务在 `extraInfo` 中以整数声明 `ACL_LIFE_CYCLE_DAYS ∈ [1, 3650]` 且 `authType == TRUST_ALWAYS`，`ImportAuthInfo` 校验通过、绑定完成后 ACL `AccessControlProfile.extraData` JSON 含同名整数 key
- AC-2：业务在 `authType != TRUST_ALWAYS` 时声明正数保留天数，`ImportAuthInfo` 返回 `ERR_DM_INPUT_PARA_INVALID`
- AC-3：业务在 `extraInfo` 中未配置 `ACL_LIFE_CYCLE_DAYS` 或显式置 `-1`，绑定后 ACL `AccessControlProfile.extraData` 不含该字段，清理仍按 365 天判过期
- AC-4：永久可信 ACL `AccessControlProfile.extraData` 含合法整数 `ACL_LIFE_CYCLE_DAYS = N`，`DeleteAlwaysAllowTimeOut` 按 `N * 86400` 秒判过期；字段缺失/非整数/超界/malformed JSON 时 fallback 到 `MAX_ALWAYS_ALLOW_SECONDS = 31536000`
- AC-5：旧版本服务读到由新版本写入、含 `ACL_LIFE_CYCLE_DAYS` 的 ACL 时忽略该字段并按 365 天清理；新版本服务读到不含该字段的旧 ACL 时也按 365 天清理（双向兼容）

## 不涉及项确认

OpenHarmony 8 维 N/A 确认：

| 维度 | 是否涉及 | 说明 |
|------|---------|------|
| API/SDK 签名 | 否 | 不改 `DmAuthInfo` 结构体；`extraInfo` JSON key 为新增 |
| IPC 接口/codes | 否 | 不改 `common/include/device_manager_ipc_interface_code.h` |
| 持久化 schema | 否 | 不改 DP 数据库 schema；复用 `AccessControlProfile.extraData` JSON 扩展字段 |
| 权限 | 否 | 不改 `permission/dm_permission.json` |
| SA 注册/init | 否 | 不改 `sa_profile/device_manager.cfg` |
| HiSysEvent | 否 | 不新增事件，不改 `hisysevent.yaml` |
| 构建组件依赖 | 否 | 不改 `bundle.json` inner_kits / `deps.components` |
| 跨子系统调用 | 否 | DP 通过 `AccessControlProfile.extraData` 扩展字段承载，无新 inner-kit 引用 |

## 关联

- Issue：[#2445](https://gitcode.com/openharmony/distributedhardware_device_manager/issues/2445)
- Capability：`binding-acl-lifetime`（首次引入）
- 设计：见 `design.md`
- 规约：见 `spec.md`
- 执行计划：见 `execution-plan.md`
