# 分布式设备管理（device_manager）Agent 知识库

## 1. 知识路由

| 触发条件 | 先读 |
|---|---|
| 发布发现、绑定解绑、Import PIN 免弹框绑定、权限白名单 / 黑名单 / 系统应用判断 | `docs/01-workflows.md` |
| DP / ACL 持久化验证、ACL 环境清理、推包验证、WSL / 双板、云端镜像 / 凭据 | `docs/02-verification.md` |
| 构建入口、minimum checks、完成定义、无法验证时的说明 | `docs/03-build-test.md` |

| 任务类型或术语 | 先读 |
|---|---|
| PIN 认证 | `ext/pin_auth/AGENTS.md` |
| 3rd 栈共享语义 | `3rd/AGENTS.md` |
| ACL 写入 / 清理 / 永久可信关系生命周期 | `services/implementation/AGENTS.md` |

## 2. 需先升级确认的改动

修改以下内容前先让开发者人工确认：

- IPC code
- `bundle.json` 的 `inner_kits` / `deps.components`
- `sa_profile/` 或 init 脚本
- `permission/dm_permission.json`
- hisysevent 事件
- 顶层 `BUILD.gn` group 或 `device_manager.gni` feature flag
- 兼容 fallback / migration / deprecated 路径
- 第三方组件依赖
- 公开 SDK 头签名、错误码或生命周期语义
- 老协议的对外行为，协议路径：\services\implementation\src\authentication
- 新协议的对外行为，协议路径：\services\implementation\src\authentication_v2

## 3. 构建、测试与 PR

- 构建、测试、minimum checks 详见 `docs/04-build-test.md`。
- PR 必须填写安全自检、TDD 结果、XTS 结果；涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需要在文档中说明。

## 4. DeviceManager 核心规则

### 1. 记录原则

本文只记录 AI 不能从源码稳定推断、但执行任务时必须遵守的规则。接口签名、目录职责、调用链、枚举值和路径清单以源码为准。

### 2. 设备标识与敏感信息

`networkId`、`udid`、`uuid`、`deviceId`、PIN、位置信息都按敏感信息处理，不得明文写入日志、事件、PR、issue。

### 3. ACL 关系属性

- `ACL_LIFE_CYCLE_DAYS` 是单条可信关系的生命周期，不是设备属性。
- 关系级属性只写 `AccessControlProfile.extraData`，不得写 `accesser/accessee.extraData`。
- 永久可信关系清理必须逐条 ACL 读取生命周期，不得用全局默认值覆盖指定生命周期。
- ACL / `extraInfo` 只持久化输入参数，不冗余写派生快照；DP schema 不变，新增字段优先走 JSON `extraData`。

### 4. JSON 与 C++ 约束

- `LocalServiceInfo`、`ImportAuthInfo`、`extraInfo` 等 JSON 输入解析后先判 `IsDiscarded`。
- 全仓构建带 `-fno-exceptions`：禁止 try-catch、`std::stoi`。
- `JsonObject` 复制使用 `Duplicate()`。
- 不要绕过 `json/json_object.h` 直接 include nlohmann/cJSON。

### 5. ABI 与公开出口

- IPC code 数值是 ABI，不得修改已有值。
- SDK 头、NAPI、NDK、CJ 等公开出口必须经 `bundle.json` 的 `inner_kits` 显式登记。
- 改公开 SDK 头签名、错误码或生命周期语义前需升级确认。

### 6. 协议与角色边界

- sink/source 职责必须清晰；跨角色改动必须说明双方影响。
- `authentication_v2` 与老协议 `services/implementation/src/authentication/` 互不交叉；默认不改老协议。
- 兼容 fallback、migration、deprecated 路径改动前需升级确认。
