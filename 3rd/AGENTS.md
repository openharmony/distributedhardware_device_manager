# 3rd Agent 知识库

> 本文件适用于 `3rd/`。根目录 `AGENTS.md` 仍是总入口，本文件只补充第三方平行栈不可从代码稳定推断的规则。

## 1. 触发条件

| 触发条件 | 先读 |
|---|---|
| 改 3rd SDK 接口 | 主栈对应接口 + `docs/05-interfaces/01-external-api.md` |
| 改 3rd 服务实现 | 主栈对应实现 + `docs/05-interfaces/02-internal-api.md` |
| 改绑定、解绑、认证、生命周期语义 | `docs/04-workflows/07-bind-unbind.md` |
| 改导入 PIN / 代理绑定语义 | `docs/04-workflows/08-import-pin-no-popup-bind.md` |
| 改开源 / 闭源差异处理 | `docs/09-opensource-closedsource-isolation.md` |

## 2. 红线

- `3rd/` 不是独立产品线；共享协议、字段、错误码、SDK 入参语义、认证/绑定行为、生命周期语义应与主栈一致。
- 只改 `3rd/` 而不同步主栈，或只改主栈而不同步 `3rd/`，必须在最终回复说明原因。
- 不新增主栈没有评估过的第三方依赖。
- 不明文打印 `networkId`、`udid`、`uuid`、`deviceId`、PIN 或位置信息。
- JSON 解析必须使用仓内封装，先判断 `IsDiscarded`。
- 不使用 `std::stoi`、try-catch 或依赖异常的控制流。
- 改公开头或 SDK 语义时，按根目录升级确认规则处理。

## 3. 验证

- 共享概念改动要同时检查主栈 diff，最终回复列出是否同步及原因。
- 接口语义改动要说明兼容性影响和下游 include 路径影响。
