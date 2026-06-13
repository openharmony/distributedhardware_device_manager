# DeviceManager 工作流规则

## 1. 发布与发现

DM 发布/发现依赖屏幕事件与 Wi-Fi/蓝牙状态，需要保持双端设备屏幕亮屏，并且wifi/蓝牙状态开启，任一端灭屏或者wifi/蓝牙关闭都可能导致发现/发布功能失败。

## 2. 绑定与解绑

- 绑定涉及 source 与 sink 两端：source是主动发起端，sink是被动接受端。当一端状态、参数、ACL 写入修改时，需同步确认修改对端行为，保持双端行为一致。
- 新增认证能力默认走 `authentication_v2`，不要顺手改老协议`authentication`。
- 永久可信关系生命周期按单条 ACL 关系级属性处理，清理时逐条判断。

## 3. Import PIN 免弹框绑定

适用场景：自动化测试或安全场景需要跳过 PIN 弹框 UI。PIN 弹框是 SecComp 安全页面，外部进程不能截图识别或注入输入，必须走 `ExportAuthInfo` / `ImportAuthInfo`。

关键不变式：

- 调用方实际 process name 必须命中 DM auth code 白名单；demo 可用 `SetSelfTokenID` 覆写为白名单进程名，二进制文件名本身无效。
- `Init(pkgName)`、`pinConsumerPkgName`、`bizSrcPkgName`、`bizSinkPkgName` 与 self token process name 保持一致，避免白名单和兼容分支不一致。
- sink/source 的 `pinConsumerPkgName` 与 `pinExchangeType` 必须完全一致。
- `extraInfo` 携带 `ACL_LIFE_CYCLE_DAYS` 时，`authType` 必须是 `TRUST_ALWAYS`，否则入口校验拒绝或字段不落盘。
- PIN 单次有效；bind 完成或 PIN timer 到期后，重跑必须重新 Export + Import。
- 新接口路径下 `ImportAuthInfo` 会写 LocalServiceInfo 并注册 PIN，不需要再额外调用 `RegisterLocalServiceInfo`。

## 4. 权限

- `ImportAuthInfo` / `ExportAuthInfo` 校验的是 IPC 元信息中的实际 process name，不是 pkgName。
- 白名单、黑名单、系统应用判断不要用硬编码绕过。
- 修改 `permission/dm_permission.json`、`sa_profile/` 或 init 脚本前需升级确认。
