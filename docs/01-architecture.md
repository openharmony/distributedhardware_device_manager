# DeviceManager 架构概述

## 1. 定位

DeviceManager 是分布式硬件设备管理的业务编排层，协调设备发现、认证、可信关系、上下线状态和对外通知。

## 2. 边界约束

- DM 不直接替代 SoftBus、DeviceProfile、HiChain 的职责：SoftBus 管连接与发现，DeviceProfile 管持久化档案与 ACL，HiChain 管认证与凭证。
- sink/source 职责必须清晰；跨角色改动需要同时说明两端影响。
- `authentication_v2` 与老协议 `services/implementation/src/authentication/` 互不交叉；默认不改老协议。
- 新增架构信息优先放设计文档；可从代码搜索到的类、目录、调用链不写进知识库。

## 3. 关联文档

- 术语与关系属性：`02-core-concepts.md`
- ACL / DP 持久化：`05-interfaces/04-dp-permission-and-storage.md`
- 开闭源隔离：`09-opensource-closedsource-isolation.md`
- 构建测试验证：`10-build-test-verify.md`
