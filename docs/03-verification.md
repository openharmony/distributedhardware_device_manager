# DeviceManager 验证与调试规则

## 1. DP / ACL 持久化验证

DP 服务端按调用方实际 process name 做白名单校验。自动化 demo 即使命中 DM 白名单，也通常不在 DP 白名单内，不能直接调用 `GetAllAccessControlProfile` 验证 ACL。

正确做法：端到端验证 DP / ACL 持久化时，把 DP RDB 主库、`-wal`、`-shm` 拉到 PC 本地只读解析；不要在板上直接 sqlite 查询或修改。

要点：

- ACL RDB 在 `/data/service/el1/public/database/distributed_device_profile_service/acl_db/`。
- LocalServiceInfo / ServiceInfo 等 DP RDB 在同一 DP 服务数据库目录下；具体库名和表名可能随 DP 演进，测试脚本应动态发现表和列，不要硬编码。
- SQLite WAL 模式下必须同时拉 `.db`、`.db-wal`、`.db-shm`，否则可能读不到最新写入。
- 拉取 `dp_rdb.db` 前不要先 kill DP SA；提前 kill 可能导致 WAL 未 checkpoint，引入假阴性。

## 2. ACL 验证环境清理

修 ACL 写入、读取、清理逻辑后，端到端复测前要避免旧 DP / DM / hichain 持久化数据污染结论。

- 首次验证或怀疑历史数据污染时，清 DP / DM / hichain 相关持久化目录后 reboot，让服务重建空库。
- 多用例连续跑只需在用例间清 DP ACL / profile / LocalServiceInfo / kvdb 目录，并重启 DM / DP SA；通常不需要清 hichain，也不需要 reboot。
- 不要删除 `/data/misc_ce/<userId>/`、`/data/app/`、`/data/accounts/`。

## 3. 推包验证

本仓 UT 和独立 sink/source/probe demo 通过 IPC 调 DM SA；行为由板上已安装的 DM `.so` 决定。改 `services/service`、`services/implementation`、`common`、`interfaces/inner_kits` 后，板端验证前必须推对应 `.so` 并重启，不能只跑新测试二进制。

分布式验证两板必须同步推包、同步重启、等待 BOOT_COMPLETED，并确认两端均亮屏、同 Wi-Fi。

## 4. WSL / 设备注意事项

WSL2 下开发板 reboot 后 USB 会重新枚举，Windows 侧会收回设备，需重新 `usbipd attach` 后 `hdc list targets` 才能看到板子。双板验证前必须确认两个 SN 都在线。

## 5. 云端镜像和凭据

- 云端产物推板验证前，板镜像需与编译 DM `.so` 的 code base 同期，否则可能因跨 DSO CFI bitset 不一致导致 `__cfi_check_fail`。
- 远程主机、端口、用户、路径、密码等凭据不得写入 docs、脚本、PR 或 issue；运行时向用户询问并只保留在会话中。
