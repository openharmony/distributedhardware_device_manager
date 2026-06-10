# 分布式设备管理（device_manager）Agent 知识库

> 本文件适用于全仓；子目录如有更近的 AGENTS.md，以子规则优先。

## 1. 记录原则

- 只记录 AI 不能从代码稳定推断、但执行任务时必须遵守的规则。
- 不记录目录职责、普通调用链、实现摘要、可搜索到的类/函数位置或测试清单。
- 可从代码读出的事实优先让 AI 通过搜索和阅读确认；知识库只给触发条件、红线和验证要求。
- 新增知识必须能回答：代码是否无法稳定推断、是否能防止高风险错误、是否有明确触发条件。

## 2. 知识路由

细节先读 `docs/`，不要把架构、流程或接口内容复制进 AGENTS。

| 触发条件 | 先读 |
|---|---|
| 架构、术语、数据结构、ABI、公开出口、JSON 约束 | `docs/01-core-rules.md` |
| 发布、发现、绑定、解绑、Import PIN、权限 | `docs/02-workflows.md` |
| DP / ACL 持久化、推包、板端、WSL、调试验证 | `docs/03-verification.md` |
| 构建、测试、minimum checks、完成定义 | `docs/04-build-test.md` |

| 任务类型或术语 | 先读 |
|---|---|
| PIN 认证 | `ext/pin_auth/AGENTS.md` |
| 3rd 栈共享语义 | `3rd/AGENTS.md` |
| ACL 写入 / 清理 / 永久可信关系生命周期 | `services/implementation/AGENTS.md` |

## 3. 需先升级确认的改动

改以下内容前先升级到 CODEOWNERS 或 issue owner：IPC code；`bundle.json` 的 `inner_kits` / `deps.components`；`sa_profile/` 或 init 脚本；`permission/dm_permission.json`；hisysevent 事件；顶层 `BUILD.gn` group 或 `device_manager.gni` feature flag；兼容 fallback / migration / deprecated 路径；第三方组件依赖；公开 SDK 头签名、错误码或生命周期语义；老协议对外行为。

## 4. 构建、测试与 PR

- 构建、测试、minimum checks 详见 `docs/04-build-test.md`。
- PR 必须填写安全自检、TDD 结果、XTS 结果；涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需特别说明。
- 新增需求需要符合 codespec 规范，设计文档归档到 `.codespec/changes/issue-{issue No.}-{issue description}`，包含 `proposal.md`、`spec.md`、`design.md`、`execution-plan.md`；有条件时输出功能验证 `REPORT.md`。
