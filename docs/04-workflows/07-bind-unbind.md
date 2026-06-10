# 绑定与解绑流程

## 1. 角色边界

绑定涉及 source 与 sink 两端。改动任一端状态、参数、ACL 写入或回调时，必须同步确认另一端行为是否受影响。

## 2. 协议边界

`authentication_v2` 与老协议 `services/implementation/src/authentication/` 互不交叉；默认不改老协议。兼容 fallback、migration、deprecated 路径改动前需要升级确认。

## 3. PIN 免弹框绑定

自动化测试或安全场景需要跳过 PIN 弹框时，走 `ExportAuthInfo` / `ImportAuthInfo`，不要试图截图识别 SecComp 安全页面。详见 `08-import-pin-no-popup-bind.md`。

## 4. ACL 生命周期

- 永久可信关系生命周期是每条 ACL 的关系级属性，不是设备属性。
- `ACL_LIFE_CYCLE_DAYS` 必须写入 `AccessControlProfile.extraData`。
- 清理永久可信关系必须按单条 ACL 的生命周期判断，不得用全局默认值覆盖指定生命周期。
- 端到端验证 ACL 持久化时，按 `05-interfaces/04-dp-permission-and-storage.md` 和 `06-debugging.md` 拉取 RDB 主库、`-wal`、`-shm` 到 PC 本地只读解析。
