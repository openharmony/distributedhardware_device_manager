# 3rd PIN / sessionKey 临时内存清理

本次修改覆盖 3rd 服务端的 `GeneratePinCode`、`ImportPinCode3rd` Stub，以及 `QueryTrustRelation` / `QuerySessionKey` 中的临时敏感数据。

- PIN 直接填充一次确定长度的输出字符串，移除 `to_string`、字符串追加、`substr` 和 `generatedPinCode` 中间副本；保留长度范围、纯数字、首位非零规则。
- Generate 在写入 reply 后清零本地 PIN，写结果失败时同样清理；Import 在服务调用结束后清零 Stub 的本地 PIN。
- Query 清零当前条目的字符串、字节数组和原始 key；写结果、列表长度或任意条目失败时，调用已有 `FreeDeviceInfos` 清零并释放尚未处理的 key。
- `QuerySessionKey` 在读取失败、长度超限、分配失败及复制完成时清零临时字节数组。

IPC code、字段顺序、`ReadString` / `WriteString` 格式、公开 SDK 签名及错误码保持不变。清零本地副本不会清除正在传输的 Parcel 内容，也不构成 IPC 加密。Import 后仍供认证使用的缓存不在本次临时副本修改范围内。

已对照主栈 `utils/src/dm_random.cpp` 和 `services/service/src/ipc/standard/ipc_cmd_parser.cpp`。本次仅修改问题单指定的 3rd 路径；主栈实现未同步修改，双方的 PIN 格式及接口使用规则不变。

验证结果见本次交付说明。本仓为组件子仓，完整编译和板端验证需在对应 OpenHarmony 产品环境执行；独立故障注入验证不能替代产品编译和真实 IPC 测试。
