# 架构设计决策记录 (ADR)

## ADR-001: 认证架构双版本并存

为兼容存量设备与新认证能力，DM 保持老协议与 `authentication_v2` 并存。新功能默认不改老协议；涉及 fallback、migration、deprecated 路径时需升级确认。

## ADR-002: ACL 关系属性写入 AccessControlProfile.extraData

生命周期、`IsLnnAcl`、`serviceId` 等描述“这条可信关系”的属性属于 ACL 关系级属性，必须写入 `AccessControlProfile.extraData`。

不得写入 `accesser/accessee.extraData`，因为同一实体可能参与多条 ACL，实体级 extraData 会导致关系属性互相覆盖。

## ADR-003: 永久可信关系按单条 ACL 生命周期清理

永久可信关系生命周期不是设备级默认值。清理时必须逐条 ACL 读取指定生命周期，不能用全局默认值覆盖。

## ADR-004: 开源/闭源版本编译宏隔离

同一份源码需支持开源与闭源产品形态。新增代码若涉及 vendor resident so、闭源能力、单/双设备兼容适配或开源 fallback，必须同时验证两种路径。详细规则见 `09-opensource-closedsource-isolation.md`。

## ADR-005: DP 持久化验证方式

ACL / DP 持久化验证不能在板上直接 sqlite 查询或修改；必须拉取 RDB 主库、`-wal`、`-shm` 到 PC 本地只读解析。详见 `05-interfaces/04-dp-permission-and-storage.md` 和 `06-debugging.md`。
