# services/implementation Agent 知识库

> 本文件适用于 `services/implementation/`。根目录 `AGENTS.md` 仍是总入口，本文件只补充服务实现侧不可从代码稳定推断的规则。

## 1. 触发条件

| 触发条件 | 先读 |
|---|---|
| 改绑定、解绑、认证状态机 | `docs/04-workflows/07-bind-unbind.md` |
| 改导入 PIN 免弹窗绑定 | `docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 改 ACL 写入 / 清理 / 永久可信关系生命周期 | `docs/04-workflows/03-trust-management.md`、`docs/02-core-concepts.md` |
| 改 DP / HiChain / SoftBus 交互 | `docs/05-interfaces/03-dependency-api.md` |
| 改权限或调用方校验 | `docs/04-workflows/05-permission-check.md` |

## 2. 红线

- sink/source 分支必须职责清晰，跨角色改动要同时说明两端影响。
- 关系属性只写 `AccessControlProfile.extraData`，不要写入 `accesser/accessee.extraData`。
- 永久可信关系生命周期是单条 ACL 的关系级属性；同一设备不同 ACL 可有不同生命周期。
- 永久可信关系清理必须按 ACL 自身记录的生命周期判断，不得用全局默认生命周期覆盖指定值。
- 不改 DP schema；新增关系级字段走 ACL `extraData` JSON 扩展位。
- JSON 解析必须使用 `json/json_object.h`，先判断 `IsDiscarded`，不要直接 include nlohmann/cJSON。
- 不使用 `std::stoi`、try-catch 或依赖异常的控制流。
- `authentication_v2` 与老协议路径默认不交叉；不要把新协议 helper 抄进老协议。
- 不明文打印 `networkId`、`udid`、`uuid`、`deviceId`、PIN 或位置信息。

## 3. 验证

- 认证、ACL、永久可信关系生命周期改动还要编译 `device_manager_test`，并优先跑关联 UT。
- 无法运行验证时，最终回复必须说明原因和风险。
