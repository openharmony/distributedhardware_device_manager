# 分布式设备管理（device_manager）Agent 知识库

> 本文件适用于全仓；子目录如有更近的 AGENTS.md，以子规则优先。
> 子规则：`3rd/AGENTS.md`、`ext/pin_auth/AGENTS.md`、`services/implementation/AGENTS.md`。

## 1. 记录原则

- 只记录 AI 不能从代码稳定推断、但执行任务时必须遵守的规则。
- 不记录目录职责、普通调用链、实现摘要、可搜索到的类/函数位置或测试清单。
- 可从代码读出的事实优先让 AI 通过搜索和阅读确认；知识库只给触发条件、红线和验证要求。
- 新增知识必须能回答：代码是否无法稳定推断、是否能防止高风险错误、是否有明确触发条件。

## 2. 知识路由

细节先读 `docs/`，不要把架构、流程或接口内容复制进 AGENTS。

| 触发条件 | 先读 |
|---|---|
| 架构、术语、数据结构 | `docs/01-architecture.md`、`docs/02-core-concepts.md`、`docs/03-data-structures.md` |
| 发布、发现、查询、监听、信任、用户、权限、上下线 | `docs/04-workflows/01-*` 至 `06-*` |
| 绑定、解绑、Import PIN 免弹窗绑定 | `docs/04-workflows/07-bind-unbind.md`、`docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 对外、内部、依赖、DP 接口 | `docs/05-interfaces/01-*` 至 `04-*` |
| 调试、架构决策、FAQ、开闭源隔离、构建测试 | `docs/06-debugging.md`、`docs/07-design-decisions.md`、`docs/08-faq.md`、`docs/09-opensource-closedsource-isolation.md`、`docs/10-build-test-verify.md` |

| 任务类型或术语 | 红线 |
|---|---|
| IPC code | 数值即 ABI，不得修改已有值 |
| SDK 头 / NAPI / NDK / CJ | 公开出口必须经 `bundle.json` `inner_kits` 显式登记 |
| PIN 认证 | 先读 `ext/pin_auth/AGENTS.md`；PIN 值禁止明文打印 |
| 3rd 栈共享语义 | 先读 `3rd/AGENTS.md`；必要时主栈/3rd 双栈同改 |
| ACL 写入 / 清理 / 永久可信关系生命周期 | 先读 `services/implementation/AGENTS.md` |
| `AccessControlProfile.extraData` | 关系属性只写这里，不写 `accesser/accessee.extraData` |
| `networkId` / `udid` / `uuid` / `deviceId` / PIN / 位置信息 | 禁止明文落日志、事件、PR、issue |
| `LocalServiceInfo` / `ImportAuthInfo` / `extraInfo` | JSON 解析先判 `IsDiscarded` |

## 3. 硬约束边界

- sink/source 职责必须清晰；跨角色改动必须说明双方影响。
- `authentication_v2` 与老协议 `services/implementation/src/authentication/` 互不交叉；默认不改老协议。
- ACL / `extraInfo` 只持久化输入参数，不冗余写派生快照；DP schema 不变，新增字段优先走 JSON extraData。
- 永久可信关系的生命周期是每条 ACL 的关系级属性，不是设备属性；不得写入 `accesser/accessee.extraData`。
- 永久可信关系清理必须按单条 ACL 的生命周期判断，不得用全局默认值覆盖指定生命周期。
- 全仓构建带 `-fno-exceptions`：JSON 解析先判 `IsDiscarded`；`JsonObject` 复制用 `Duplicate()`；禁止 `std::stoi` 和 try-catch。
- 端到端验证 DP / ACL 持久化时，拉取 RDB 主库、`-wal`、`-shm` 到 PC 本地只读解析；不要在板上直接 sqlite 查询或修改。
- 不要绕过 `json/json_object.h` 直接 include nlohmann/cJSON。
- 不要为通过测试删除日志、错误码、hisysevent 或诊断信息。
- 不要新增 `OAT.xml` 协议头之外的 license 头；C++ 单行不超过 120 字符；新增阈值、超时、状态码等用具名常量。

## 4. 需先升级确认的改动

改以下内容前先升级到 CODEOWNERS 或 issue owner：IPC code；`bundle.json` 的 `inner_kits` / `deps.components`；`sa_profile/` 或 init 脚本；`permission/dm_permission.json`；hisysevent 事件；顶层 `BUILD.gn` group 或 `device_manager.gni` feature flag；兼容 fallback / migration / deprecated 路径；第三方组件依赖；公开 SDK 头签名、错误码或生命周期语义；老协议对外行为。

## 5. 构建、测试与 PR

- 构建、测试、minimum checks 详见 `docs/10-build-test-verify.md`；构建命令在 OpenHarmony 源码根目录执行，本仓没有 npm、cargo、Makefile。
- C++ 改动至少编译受影响顶层 target：`device_manager` / `device_manager_fwk` / `device_manager_test`；测试改动必须跑改动 UT。
- 无法运行验证时，最终回复必须说明原因和风险。
- 默认 remote 是 `gitcode`（`.gitcode/oh-gc-config.json`）；PR 模板在 `.gitee/PULL_REQUEST_TEMPLATE.zh-CN.md`。
- PR 必须填写安全自检、TDD 结果、XTS 结果；涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需特别说明。
- 新增需求需要符合 codespec 规范，设计文档归档到 `.codespec/changes/issue-{issue No.}-{issue description}`，包含 `proposal.md`、`spec.md`、`design.md`、`execution-plan.md`；有条件时输出功能验证 `REPORT.md`。
