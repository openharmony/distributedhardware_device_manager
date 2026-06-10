# 分布式设备管理（device_manager）Agent 知识库

## 1. 知识路由

| 触发条件 | 先读 |
|---|---|
| 敏感设备标识、ACL 关系属性、JSON / C++ 约束、ABI / 公开出口、协议角色边界 | `docs/01-core-rules.md` |
| 发布发现、绑定解绑、Import PIN 免弹框绑定、权限白名单 / 黑名单 / 系统应用判断 | `docs/02-workflows.md` |
| DP / ACL 持久化验证、ACL 环境清理、推包验证、WSL / 双板、云端镜像 / 凭据 | `docs/03-verification.md` |
| 构建入口、minimum checks、完成定义、无法验证时的说明 | `docs/04-build-test.md` |

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
- 老协议对外行为

## 3. 构建、测试与 PR

- 构建、测试、minimum checks 详见 `docs/04-build-test.md`。
- PR 必须填写安全自检、TDD 结果、XTS 结果；涉及 IPC、SDK 头、`bundle.json`、SA、权限、hisysevent 的改动需特别说明。
