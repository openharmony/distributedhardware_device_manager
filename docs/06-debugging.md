# DeviceManager 调试与故障排查指南

**版本：v2.0**  
**更新日期：2026-05-19**

---

## 1. 概述

本文档提供 OpenHarmony DeviceManager 的调试方法、工具使用和常见故障排查流程。DeviceManager 作为分布式设备管理的核心服务，涉及设备发现、认证、信任关系管理等多个复杂流程，因此掌握系统的调试方法对于快速定位和解决问题至关重要。


### 2.4 关键日志格式说明

DeviceManager 的日志采用统一格式：

```
[级别] [时间] [PID:tid] [标签] [模块][函数]: 消息内容
```

**示例**：
```
05-19 10:23:45.123  1234  5678 I 01234/DHDM: [DH_DM_SERVER][Init]: "DeviceManager service initializing..."
05-19 10:23:45.124  1234  5678 I 01234/DHDM: [DH_DM_SOFTBUS][RegisterLnn]: "SoftBus LNN callback registered"
05-19 10:23:45.125  1234  5678 E 01234/DHDM: [DH_DM_AUTH][CreateGroup]: "Failed to create HiChain group: -20026"
```

**关键字段**：
- `[模块]`：标识子模块，如 `DH_DM_AUTH`
- `[函数]`：当前函数名
- `%{public}s`：Hilog 格式化占位符，表示公开字符串（可被日志工具读取）
- `%{private}s`：私有字符串（会被脱敏处理）

**敏感信息脱敏**：
```cpp
// UDID 等敏感信息会被脱敏
LOGI("Local device: %{public}s", GetAnonyString(localUdid).c_str());
// 输出: Local device: ABC***123
```

---


### 5.2 验证 PR / 本地 DM 改动时：必须先推 .so 再跑测试

**Why**：本仓 UT 与独立测试归档仓中的 sink/source/probe demo 都通过 IPC 与 DM SA 通信。它们的 stub/proxy 行为是由**板子镜像里 `/system/lib/` 下安装的 DM .so 决定的**，而不是测试二进制自己编进去的——`devicemanagersdk` 在板子上是动态库，测试二进制只是 dlopen 调用方。

如果板子镜像里的 DM .so 仍是上版本，而你的 PR 改了 DM 行为，你跑测试看到的是**旧 .so 的行为**，得到的结论无效（PASS/FAIL 都不能信）。

**Rule of thumb**：凡是 PR 改了 `services/service` / `services/implementation` / `common` / `interfaces/inner_kits` 下面的 cpp/h，验证前必须按下面流程把对应 .so 推到板子并重启。

#### 5.2.1 识别 PR 影响的 .so

```bash
# 先列出 PR 改动的源文件（排除 test/docs）
cd foundation/distributedhardware/device_manager
git diff --name-only <base>..HEAD -- '*.cpp' '*.h' \
    | grep -v test/ | grep -v openspec/ | grep -v docs/

# 然后通过 ninja 文件反查每个源文件被编进哪个 .so
find out/<product>/src -name '*.ninja' \
    | xargs grep -nr "source_file_part = <修改文件名>"
```

#### 5.2.2 DM 仓四个核心 .so 与对应板上路径

| .so | 由什么源生成 | 板上路径 |
|---|---|---|
| `libdevicemanagerservice.z.so` | `services/service/src/*.cpp` | `/system/lib/libdevicemanagerservice.z.so` |
| `libdevicemanagerserviceimpl.z.so` | `services/implementation/src/**/*.cpp` | `/system/lib/libdevicemanagerserviceimpl.z.so` |
| `libdevicemanagersdk.z.so` | `interfaces/inner_kits/native_cpp/src/*.cpp` + `common/src/dm_constants.cpp` | `/system/lib/platformsdk/libdevicemanagersdk.z.so` |
| `libdevicemanagerutils.z.so` | `utils/src/**/*.cpp` | `/system/lib/libdevicemanagerutils.z.so` |

> SDK lib 在 `/system/lib/platformsdk/` 子目录，不在 `/system/lib/` 顶层——`find /system -name <lib>` 一次性确认。

#### 5.2.3 推 .so + 重启 + 跑测试（完整流程）

```bash
# 1) 编译要推的 .so（用 hb 单 target 编最快）
hb build --ccache \
    -T devicemanagerservice \
    -T devicemanagerserviceimpl \
    -T devicemanagersdk

# 2) 挂载 /system 可写
hdc -t <sn> target mount

# 3) 替换 .so（按实际 PR 影响的 .so 列表来）
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagerservice.z.so /system/lib/libdevicemanagerservice.z.so
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagerserviceimpl.z.so /system/lib/libdevicemanagerserviceimpl.z.so
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/libdevicemanagersdk.z.so /system/lib/platformsdk/libdevicemanagersdk.z.so

# 4) 重启板子让 SA 重新加载新 .so
hdc -t <sn> shell sync
hdc -t <sn> shell reboot
hdc -t <sn> wait-for-device
# 等 BOOT_COMPLETED（SA start-on-demand 在引导完成后才能被拉起）
sleep 30

# 5) 推测试二进制
hdc -t <sn> file send out/<product>/distributedhardware/device_manager/<your_test_binary> /data/local/tmp/
hdc -t <sn> shell chmod 755 /data/local/tmp/<your_test_binary>

# 6) 跑测试
hdc -t <sn> shell /data/local/tmp/<your_test_binary>
```

#### 5.2.4 检查 SA 是否健康（区分镜像问题 vs PR 问题）

如果测试报 `ERR_DM_INIT_FAILED (96929747)` 或 `proxy init failed`，先看是不是 DM SA 自己挂了，而不是你的 PR 出问题：

```bash
# DM SA 进程是否在
hdc -t <sn> shell "ps -ef | grep device_manager | grep -v grep"

# 最近是否有 DM SA cppcrash
hdc -t <sn> shell "ls -lt /data/log/faultlog/faultlogger/ | grep device_manager | head -5"

# 看最新一份 cppcrash 的 stack（Reason / 顶部 5 帧）
hdc -t <sn> shell "head -40 /data/log/faultlog/faultlogger/cppcrash-device_manager-*.log" \
    | tail -45
```

如果 stack 里没有你 PR 改动的源文件，就是**板子镜像本身的 bug**（典型如 `libdistributeddata_inner` UBSan abort），需要换/刷镜像，不要怀疑你的 PR。

#### 5.2.5 一次推完两台板子（分布式测试场景）

```bash
for sn in <sn1> <sn2>; do
    hdc -t $sn target mount
    for so_pair in \
        "libdevicemanagerservice.z.so:/system/lib" \
        "libdevicemanagerserviceimpl.z.so:/system/lib" \
        "libdevicemanagersdk.z.so:/system/lib/platformsdk"; do
        lib="${so_pair%:*}"; dir="${so_pair##*:}"
        hdc -t $sn file send out/<product>/distributedhardware/device_manager/$lib "$dir/$lib"
    done
    hdc -t $sn shell sync
    hdc -t $sn shell reboot
done
# 两台同步等 BOOT_COMPLETED
for sn in <sn1> <sn2>; do
    hdc -t $sn wait-for-device
done
sleep 30
```

#### 5.2.6 常见坑

- **只推 SDK 不重启**：SDK 是 dlopen 库，重启后才生效；SA 是常驻进程，**必须重启**
- **少推 utils**：PR 没改 utils 时不需要，但 `common/include/*.h` 内联到 utils 时要重新编 utils（看 ninja 反查为准）
- **推完忘 sync**：写入未落盘就 reboot，等于没推
- **板子状态不一致**：两台板子若一台推过、一台没推，"是否复现" 这个判断就会带误差——同步推同步重启
- **分布式用例必须双端亮屏（不要求解锁）**：DM 的设备发布依赖屏幕事件，`PublishCommonEventCallback` 只有在 Wi-Fi 可用且屏幕亮起时才会发布设备；任一端灭屏都可能导致 source `StartDiscovering` 30s 超时。跑 sink/source demo 前先让两块 RK 都保持亮屏，必要时执行 `power-shell wakeup`

#### 5.2.7 demo / 自研可执行：native_token 必须用动态库（`*_shared`）才能链通 `Restorecon`

- **现象**：独立测试归档仓中的 `ohos_executable` demo（例如 `issue-2445-acl-life-cycle-days/sink/source/p2_probe`）链接报：
  ```
  ld.lld: error: undefined symbol: Restorecon
  >>> referenced by nativetoken.c:283 (../../base/security/access_token/interfaces/innerkits/nativetoken/src/nativetoken.c:283)
  >>>               lto.tmp:(ClearOrCreateCfgFile)
  >>> referenced by nativetoken.c:802 (../../base/security/access_token/interfaces/innerkits/nativetoken/src/nativetoken.c:802)
  >>>               lto.tmp:(GetAccessTokenId)
  ```
- **根因**：
  - `Restorecon` 由 `base/security/selinux_adapter:librestorecon` 提供（声明在 `interfaces/policycoreutils/include/policycoreutils.h`）
  - `access_token:libnativetoken` 是**静态库**（`libnativetoken.a`），它把对 `Restorecon` 的引用打包但**不携带 selinux_adapter 链接**，需要使用方自己显式带上
  - `access_token:libtoken_setproc` 同样是静态库（`libtoken_setproc.a`），有同类问题
- **修复**：把 external_deps 改成 `_shared` 后缀的动态版本——
  ```gn
  external_deps = [
    "access_token:libaccesstoken_sdk",
    "access_token:libnativetoken_shared",    # 不要用 libnativetoken（静态）
    "access_token:libtokensetproc_shared",   # 不要用 libtoken_setproc（静态）
    ...
  ]
  ```
- **参考样板**：`foundation/distributedhardware/distributed_hardware_fwk/interfaces/inner_kits/test/demo/BUILD.gn` 是同子系统中同时用 `libnativetoken_shared` + `libtokensetproc_shared` 的 demo；新增可执行 demo 直接 copy 它的 external_deps 模板可少踩这个坑
- **何时复现**：`ohos_executable` 类型 + 自己调 `GetAccessTokenId` / `SetSelfTokenID` 的二进制；UT（`ohos_unittest`）通常通过 fuzz/gtest 框架间接拉 selinux_adapter，不一定触发
- **不要的"修复"**：不要给 demo 加 `selinux_adapter:librestorecon` 来"绕"——这只是补一个间接依赖，正确方向是让 native_token 走动态库版本，由动态库内部自带 selinux 解析

#### 5.2.8 板镜像必须与"编 dm so 用的 code base 同期"，否则 `__cfi_check_fail` SA 必崩

- **现象**：把云端编出来的 `libdevicemanagerservice.z.so` / `libdevicemanagerserviceimpl.z.so` 推到 RK3568 板、reboot 后 SA 拉起正常，但**调用任何走 BundleMgr 的 IPC 入口（如 `BindTarget`）时 SA 立即 `SIGABRT`**，回栈：
  ```
  Signal:SIGABRT(SI_TKILL)
  #02 __cfi_check_fail            libappexecfwk_core.z.so
  #03 __cfi_check                  libappexecfwk_core.z.so
  #04 DeviceManagerServiceImpl::GetBundleLable
  #05 DeviceManagerServiceImpl::GetBindCallerInfo
  #08 IpcCmdProcessBIND_TARGET
  ```
- **根因**：dm 编译时带 `sanitize.cfi = true` + `cfi_cross_dso = true`（见 `services/implementation/BUILD.gn`），跨 DSO 函数指针调用走 CFI 校验。云端代码与板镜像不同期 ⇒ 双方对 `IBundleMgr::GetApplicationInfoV9` 等签名生成的 CFI bitset 不一致 ⇒ 跳转时 type id 不匹配 ⇒ `__cfi_check_fail` 直接 `abort()`
- **判别**：
  - 看 `/data/log/faultlog/faultlogger/cppcrash-device_manager-*.log` 栈顶是否包含 `__cfi_check_fail` / `__cfi_check`
  - 是 → ABI 不对齐，本质是版本错配，不是 PR bug
  - 不是 → 才考虑 PR 自身逻辑问题
- **修复（按推荐度排序）**：
  1. **刷与云端 code base 同期的 OH 日构建到板上**（最干净）：从 [https://dcp.openharmony.cn/workbench/cicd/dailybuild/detail/component](https://dcp.openharmony.cn/workbench/cicd/dailybuild/detail/component) 下载对应日期的 RK3568 整机包，用 RKDevTool 刷机。例：云端 base 是 `openharmony_master_default_20260530...`，板上就刷 5/30 日构建
  2. **依赖 so 整套对齐**：除 dm so 外，把 `libappexecfwk_core.z.so` 等 CFI 校验失败涉及的依赖 so 一起从云端 `out/` 推到板上（容易漏，不推荐）
  3. **关掉 dm CFI 重新编**（仅作快速验证、不要提交）：临时把 `sanitize.cfi = false` 改进 dm BUILD.gn，重编推板。**绝对不要**把这种改动合到 PR
- **预防**：每次"云端编 → 推板验证"前先核对：板上 `param get const.ohos.version.security_patch` / 日构建时间戳 与 云端 code 的 `openharmony_master_default_<日期>...` 是否同日

#### 5.2.9 云上拉镜像：先云端打 tar.gz，再下载，再本地解压（不要直接 rsync 散文件）

> **凭据/物理环境隔离原则**：本节命令里的 `<HOST>` / `<PORT>` / `<USER>` / `<REMOTE_WORKSPACE>` / 任何密码都**不允许写入本知识库**或任何 docs / .codespec / 公共脚本。每次执行前由 agent 向当前会话用户询问，得到后只保留在会话上下文里（不落盘、不进 commit）。如遇到 PR/MR/issue 评论里贴了真实地址或密码，agent 必须当场提示作者撤回。

- **现象**：`out/rk3568/packages/phone/images/` 下散文件总量 ~2.4GB（system.img / vendor.img / userdata.img / sys_prod.img / ...），直接 `rsync` 走 ssh 单连接，带宽利用率低、易被 20 分钟 tool 超时截断、断点续传只能恢复"单个 img"，已传一半的大 img 经常要重来。
- **正确流程（节省带宽 + 可断点续传 + 不超时）**：
  1. **云端先打 tar.gz**（一次性顺序读盘、压缩比 ~70%）：
     ```bash
     # 示例占位，真实 <HOST>/<PORT>/<USER>/<REMOTE_WORKSPACE> 运行时问用户
     ssh -p <PORT> <USER>@<HOST> \
       "cd <REMOTE_WORKSPACE>/code/out/rk3568/packages/phone && \
        tar -czf rk_images.tar.gz images/"
     ```
     2.4GB 散文件 → 单个 ~700MB tar.gz。
  2. **rsync 单文件下载**（`--partial --inplace --progress` 保证断点续传）：
     ```bash
     rsync -av --partial --inplace --progress \
       --rsh="ssh -p <PORT> -o StrictHostKeyChecking=no" \
       <USER>@<HOST>:<REMOTE_WORKSPACE>/code/out/rk3568/packages/phone/rk_images.tar.gz \
       <LOCAL_DIR>/rk_images.tar.gz
     ```
     如必须用密码登录，优先 ssh-agent / `~/.ssh/config` 中配 key；万不得已用 `sshpass` 时，密码用 `SSHPASS` 环境变量读入，**绝不 `-p '<明文>'` 写进命令历史 / 脚本 / 日志**。
  3. **本地解压**：`tar -xzf <LOCAL_DIR>/rk_images.tar.gz -C <LOCAL_DIR>/`
- **反例**：散文件 rsync 跑 `system.img` 卡住 → 20 分钟 tool 超时 → 终端看似挂死、实际后台 rsync 还在跑/已挂但进程残留，需要 `pkill -f "rsync .*images"` 才能清干净。
- **额外好处**：tar.gz 单文件可直接给同事走 IM/网盘分发；散 img 包没 manifest 容易漏 `MiniLoaderAll.bin` 之类启动件。
- **检查清单**：
  - [ ] **本节命令里的 `<HOST>/<PORT>/<USER>/<REMOTE_WORKSPACE>/密码` 没有任何一项被写进 docs / .codespec / commit / PR 描述**
  - [ ] 云端打包前 `du -sh images/` 记录原始大小
  - [ ] 打包后 `ls -lh rk_images.tar.gz` 记录压缩后大小，写入验证报告（不带主机名）
  - [ ] 下载完 `tar -tzf rk_images.tar.gz | wc -l` 与云端 `ls images/ | wc -l` 对齐
  - [ ] 解压后核对 `MiniLoaderAll.bin / parameter.txt / system.img / vendor.img / userdata.img / sys_prod.img / chip_prod.img / resource.img / boot_linux.img / updater.img` 齐全

#### 5.2.10 验证可信关系（ACL）相关改动前，先清 DP RDB 历史数据再重测，避免旧记录污染结论

- **现象**：同一组板子反复跑 sink/source demo 验证 `access_control_table.extraData` 字段是否被新代码正确写入时，看到的可能是上一轮（旧 so 或旧逻辑）落下的 ACL 行，新 so 实际没写进去也会"看似 PASS"或"看似 FAIL"，掩盖真实行为。
- **典型坑**：
  - 旧 so 把 `ACL_LIFE_CYCLE_DAYS` 错写到 `accessee.extraData`，新 so 改写到 `access_control_table.extraData`，但 sink 板上旧那行 accessee 残留，新 so 因 `hasAcl=true` 走 update 分支只补 `access_control_table` 不会重置 accessee，结论混乱
  - hichain credential 也是持久化的，旧 credential 残留会让 source `BindTarget` 走"已认证 fast path"直接 OK，跳过新逻辑落 ACL，DB 里看不到新行
- **正确做法（推 so 验证前清干净）**：
  ```bash
  hdc -t <DEV> target mount
  hdc -t <DEV> shell "\
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/acl_db/* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/profile_data* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/local_service_info_store* ; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/kvdb/ ; \
    rm -rf /data/service/el1/public/database/distributed_device_manager_service/ ; \
    rm -rf /data/service/el1/public/hichainsdk/ ; \
    rm -f /data/local/tmp/issue2445_pin.txt /data/local/tmp/sink.log /data/local/tmp/source.log ; \
    sync"
  # 推新 so 后 reboot，DP / DM / hichain 进程拉起时各自重建空库
  hdc -t <DEV> shell reboot
  ```
- **为什么可以直接删**：DP（`distributed_device_profile_service`）/ DM（`distributed_device_manager_service`）/ hichain（`hichainsdk`）的持久化目录都是各自服务的私有 SQLite/KV 库，启动时检测到目录缺失/空表会自动重建 schema 和空表，**对运行时没有破坏性**。本质等同"业务侧 factory reset"。
- **判定 ACL 是否"由本次 demo 新落"**：
  ```sh
  # 删库 + reboot 后，库里应当只有空 schema、零行
  python3 -c "import pysqlite3; c=pysqlite3.connect('/tmp/dp_rdb_sink.db'); \
    print('rows:', c.execute('SELECT COUNT(*) FROM access_control_table').fetchone()[0])"
  # 跑完一轮 demo 再查，看到的每一行都来自本次 bind，能直接对比代码改动效果
  ```
- **不要清的内容**：板子的 `/data/misc_ce/<userId>/`、`/data/app/`、`/data/accounts/` 不要乱删，会破坏系统账户/应用沙箱，导致 dm SA 起不来。
- **检查清单**：
  - [ ] 改 ACL 写入/读取相关代码后，验证前先按上面命令清两板的 DP / DM / hichain 数据
  - [ ] reboot 后等 `bootevent.boot.completed=true` 再跑 demo
  - [ ] 跑前 `sqlite3` 看 `access_control_table` 行数应为 0
  - [ ] 跑后再查 DB，看到的所有 ACL 行均为本轮产物，可直接断言

#### 5.2.11 多用例连续跑：用例之间快速重置可信关系，不需要 reboot 也不需要清 hichain

- **场景**：同一对板子上要跑 N 个 ACL 行为用例（例如 `ACL_LIFE_CYCLE_DAYS = 1095 / -1 / 1 / 3650` 各跑一次），每个用例都要在"干净环境"下验证 `access_control_table.extraData` 的写入结果。逐个 reboot 太慢（每次都要重新 `usbipd attach`），全程跳过清理又会让上一轮 ACL 行污染下一轮（DM 看 `hasAcl=true` 走 update 分支，新行不写入）。
- **正确做法（轻量 reset，无需 reboot）**：每个用例跑完后，**只删 DP 的 ACL 数据库目录，再 kill DM 与 DP SA 让它们自动拉起**，hichain credential 不用动：
  ```bash
  # 在两板上各跑一次
  hdc -t <DEV> shell "\
    killall CollaborationFwk_source CollaborationFwk_sink 2>/dev/null; \
    kill -9 \$(pidof device_manager) \$(pidof deviceprofile) 2>/dev/null; \
    sleep 1; \
    rm -rf /data/service/el1/public/database/distributed_device_profile_service/{acl_db,profile_data*,local_service_info_store*,kvdb}/ 2>/dev/null"
  # 触发一次 DM/DP lazy load（任何 DM IPC 调用都行）
  hdc -t <DEV> shell "/data/local/tmp/dm_issue2445_p2_probe > /dev/null"
  hdc -t <DEV> shell "power-shell wakeup; power-shell setmode 602"
  ```
- **为什么不用清 hichain credential**：DM 判定"可信关系是否存在"**只看 DP `access_control_table`，不直接查 hichain credential**。DP ACL 被清空后，DM 视为"全新关系"，下一次 bind 走完整流程并重新落 ACL。hichain credential 的残留只影响 hichain 内部 session key 复用，不会让 DM 跳过 ACL 写入。这与 §5.2.10 中"首次验证或修了 DM ACL 代码后整库清"的场景不同——后者要确保**所有持久层**都从零开始排除历史影响，前者只需让"ACL 落盘行为"可复测。
- **为什么不用 reboot**：DM SA / DP SA 是 `device_manager.cfg` / `device_profile.cfg` 注册的按需 SA，kill 后下一次 IPC 调用即被 SAMgr 自动拉起，重建空 schema 与 case 1 reboot 等效。reboot 在 WSL 下还会断 USB，需要人工 `usbipd attach`，连续跑用例时成本太高。
- **DB 拉取时机的强约束**：用例跑完后**拉 `dp_rdb.db` 前不要 kill DP SA**——DP 写入走 WAL，DP 进程在世时 WAL pages 才完整可读；提前 kill DP 可能丢失未 checkpoint 的 WAL 数据，导致 `access_control_table` 查到 0 行的假阴性。正确顺序：
  1. sink/source bind 完成（source 拿到 `bindResult:0`）
  2. `sleep 3` 让 DP 把数据写完 WAL
  3. `hdc file recv` 拉 `dp_rdb.db` + `dp_rdb.db-wal` + `dp_rdb.db-shm`（**全套**，python sqlite3 自动合并 WAL）
  4. 本地 `python3 -c "import sqlite3; ..."` 解析
  5. 验证完了，再进入"清 DP DB + kill DM/DP SA"做下一个用例
- **多用例运行模板**（伪代码）：
  ```bash
  for case in case1 case2 case3 ...; do
    reset_env $DEV_A $DEV_B            # kill demo / DM / DP + rm DP db
    sink_run --life-cycle-days $val    # 在 sink 板跑 sink demo
    pin_ferry                          # PC 端转移 pin 文件 sink -> source
    source_run --life-cycle-days $val  # source 板跑 source demo
    sleep 3                            # 让 DP 写完 WAL
    pull_dp_rdb_and_verify             # 全套拉 db/-wal/-shm 后 python 解析
  done
  ```
- **检查清单**：
  - [ ] 每个用例之间只清 `/data/service/el1/public/database/distributed_device_profile_service/` 下的 ACL/profile/local_service_info/kvdb 目录
  - [ ] hichain credential 与 `/data/service/el1/public/database/distributed_device_manager_service/` 可不动（DM 不依赖）
  - [ ] kill DM SA + DP SA 后用 `dm_issue2445_p2_probe` 或任意 DM IPC 触发 lazy load
  - [ ] 拉 DP RDB 时 DP SA 必须在运行，且要拉全套（.db + -wal + -shm）
  - [ ] 用例间不需要 reboot，节省 WSL 重新 attach 时间

### 5.3 验证流程闭环 checklist

每次跑 DM 相关验证（UT / 独立归档 demo / xdevice 端到端）前过一遍：

- [ ] PR 改动的源文件已识别完整
- [ ] 对应 .so 已用 `hb build -T <target>` 重编
- [ ] 所有目标板已 `hdc target mount` + 推完 .so + `sync` + reboot + `wait-for-device` + 等 30s BOOT_COMPLETED
- [ ] 测试二进制已推 `/data/local/tmp/` 且 `chmod 755`
- [ ] 跑测试前先确认 `ps -ef | grep device_manager` 看到 SA 在，且 `/data/log/faultlog/` 没有刚生成的 DM cppcrash
- [ ] 验证日志里出现 PR 引入的关键字（如 `ACL_LIFE_CYCLE_DAYS`），证明 .so 确实生效
- [ ] **若开发主机是 WSL2**：板子 reboot 后 USB 设备会断链，必须由 Windows 侧 `usbipd attach` 重新挂载到 WSL（见 §5.4）

### 5.4 WSL2 + usbipd：板子 reboot 后必须重新 attach

WSL2 通过 `usbipd-win` 把 Windows 主机上的 USB 设备透传到 WSL 用户态。RK3568 等开发板 `reboot` 后 USB 链路重新枚举，**Windows 侧自动收回设备所有权**，WSL 内 `hdc list targets` 会返回 `[Empty]` 直到重新 attach。

**症状**

```bash
hdc list targets
# [Empty]

# Windows 侧 (在 WSL 内调用 .exe)
'/mnt/c/Program Files/usbipd-win/usbipd.exe' list
# BUSID  VID:PID    DEVICE         STATE
# 1-1    2207:5000  "HDC Device"   Shared        <- 已收回，未 attach
# 2-3    2207:5000  "HDC Device"   Shared        <- 同上
```

**根因**

- `usbipd` 的 `Shared` 状态表示设备**允许**被 WSL attach，但**当前未** attach
- `reboot` → USB 重新枚举 → `usbipd` 检测到新设备 → 自动 detach → 回到 `Shared` 状态
- 在 WSL 内调 `hdc list targets` 看不到设备直到下一次 attach

**恢复流程**

1. 查 Windows 侧目前看到几块板子：

   ```bash
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' list | grep -E "HDC|2207"
   ```

2. 对每个 `Shared` 状态的 HDC busid 重新 attach：

   ```bash
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' attach --wsl --busid 1-1
   '/mnt/c/Program Files/usbipd-win/usbipd.exe' attach --wsl --busid 2-3
   ```

3. 等几秒让 `hdcd` 在板子侧重启完，确认两板可见：

   ```bash
   sleep 5 && hdc list targets
   # 150100424a5444345209d94ebed7b900
   # 450100445634303332011a0f0a2eba00
   ```

4. 若仍 `[Empty]`，重启 hdc 守护：

   ```bash
   hdc kill && sleep 2 && hdc start -r && sleep 5 && hdc list targets
   ```

**自动化建议**

把 attach 步骤封装成脚本（如 `~/.opencode/skills/distributed-ai-auto-test/scripts/attach-boards.sh`），在 xdevice runner 之前先调一次，避免每次 reboot 后手忙脚乱：

```bash
#!/bin/bash
# attach-boards.sh: re-attach all "Shared" HDC devices to WSL
USBIPD='/mnt/c/Program Files/usbipd-win/usbipd.exe'
"$USBIPD" list | awk '/2207:5000.*Shared$/ {print $1}' | while read busid; do
    "$USBIPD" attach --wsl --busid "$busid"
done
sleep 5
hdc list targets
```

**常见坑**

- **只 attach 一块板子**：双板验证场景必须确认 `hdc list targets` 返回两个 SN，否则后续 xdevice 自动选板会拿错
- **`Persisted` vs `Shared`**：第一次用某块板时先在 Windows 管理员 PowerShell 里 `usbipd bind --busid <X>`，让设备进入 `Shared` 列表，否则 attach 会报权限错
- **板子 BOOT_COMPLETED 之前 attach**：可能 attach 到内核 USB 阶段，hdc 仍连不上；建议等 ~30 秒板子完全起来再 attach；或先 attach、再 `hdc wait-for-device`
- **WSL Distro 名不对**：`usbipd attach --wsl` 默认挂到第一个 WSL2 distro；用 `--wsl <DistroName>` 显式指定避免误挂

---

### 10.2 相关文档

- 《DeviceManager 架构设计文档》
- 《DeviceManager API 参考手册》
- 《SoftBus 开发指南》
- 《HiChain 开发指南》
- 《OpenHarmony HiLog 使用指南》
- 《OpenHarmony HiSysEvent 开发指南》

---

**文档结束**
