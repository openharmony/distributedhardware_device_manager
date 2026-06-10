# 依赖接口约束

## 1. Source of truth

DM 依赖 SoftBus、DeviceProfile、HiChain 等模块。具体 API 清单、签名和调用路径以对应模块源码及接口头文件为准，不在本文复制。

## 2. DeviceProfile 注意事项

DP 服务端按调用方进程名做白名单校验。板上 ACL、ServiceInfo、SessionKey RDB 文件位置和自动化验证方法见 `04-dp-permission-and-storage.md`。

## 3. 跨模块红线

- 不要把关系级属性写到实体级 extraData。
- 端到端验证 DP / ACL 持久化时，拉 RDB 主库、`-wal`、`-shm` 到 PC 本地只读解析。
- 不要在板上直接 sqlite 查询或修改 DP 数据库。
