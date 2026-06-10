# 内部接口约束

## 1. 记录原则

内部类关系、函数签名、调用链以源码为准，不在文档中复制快照。

## 2. 修改边界

- `authentication_v2` 与老协议目录互不交叉；默认不改老协议。
- sink/source 改动需保持职责清晰，跨角色改动需同步说明影响。
- ACL / `extraInfo` 只持久化输入参数，不冗余写派生快照。
- 新增字段优先走 JSON `extraData`，不得改变 DP schema。

## 3. JSON 与异常约束

全仓构建带 `-fno-exceptions`：JSON 解析先判 `IsDiscarded`，`JsonObject` 复制用 `Duplicate()`，禁止 `std::stoi` 和 try-catch。
