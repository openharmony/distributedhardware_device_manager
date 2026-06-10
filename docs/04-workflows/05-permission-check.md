# DeviceManager 权限校验机制

## 1. 记录原则

权限配置、接口权限清单、白名单内容以 `permission/`、`sa_profile/`、服务端校验代码为准；本文只记录修改红线。

## 2. 修改红线

- 修改 `permission/dm_permission.json`、`sa_profile/` 或 init 脚本前需要升级确认。
- 对外、绑定、导入 PIN、可信关系管理接口必须校验调用方身份与权限。
- 白名单、黑名单、系统应用判断不要用硬编码绕过。
- 测试或 demo 如需模拟系统 native 调用，必须显式设置 token，并避免把真实敏感标识写入日志。

## 3. 敏感信息

`networkId`、`udid`、`uuid`、`deviceId`、PIN、位置信息禁止明文落日志、事件、PR、issue。
