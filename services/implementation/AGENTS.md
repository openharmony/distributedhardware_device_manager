# services/implementation Agent 知识库

> 本文件适用于 `services/implementation/`。根目录 `AGENTS.md` 仍是总入口，本文件只补充服务实现侧不可从代码稳定推断的规则。

## 1. 触发条件

| 触发条件 | 先读 |
|---|---|
| 改绑定、解绑、认证状态机 | `docs/02-workflows.md` |
| 改导入 PIN 免弹窗绑定 | `docs/02-workflows.md` |
| 改 ACL 写入 / 清理 / 永久可信关系生命周期 | `docs/01-core-rules.md`、`docs/03-verification.md` |
| 改 DP / HiChain / SoftBus 交互 | `docs/03-verification.md` |
| 改权限或调用方校验 | `docs/02-workflows.md` |

## 2. 红线

- 永久可信关系生命周期是单条 ACL 的关系级属性；同一设备不同 ACL 可有不同生命周期。
- 永久可信关系清理必须按 ACL 自身记录的生命周期判断，不得用全局默认生命周期覆盖指定值。
- `authentication_v2` 与老协议路径默认不交叉；不要把新协议 helper 抄进老协议。

## 3. 验证

- 认证、ACL、永久可信关系生命周期改动还要编译 `device_manager_test`，并优先跑关联 UT。
- 无法运行验证时，按根目录完成定义说明原因和风险。
