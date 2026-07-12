# MHY_Scanner

### **版本 - v1.1.16**

## 说明
本项目为免费开源项目，用于学习和研究，禁止商业化用途。

## 功能和特性
- 从屏幕自动获取二维码登录，适用于大部分登录情景，不适用于在竞争激烈时抢码。
- 从直播流获取二维码登录，适用于抢码登录情景。
- 可选启动后自动开始识别屏幕和识别完成后自动退出，无需登录后手动切窗口关闭。
- 表格化管理多账号，方便切换游戏账号。
 
## 目前可用的平台

| 渠道 \\ 游戏 | 崩坏3 | 原神 | 星穹铁道 | 绝区零 |
| :---------: | :---: | :---: | :-----: | :---: |
|    官服     |  ✅   |  ✅   |   ✅    |  ✅   |
|  Bilibili   |  ✅   |       |         |       |

> 说明：上表为各游戏已支持的**登录渠道**。除崩坏3 额外支持 Bilibili 服外，原神 / 星穹铁道 / 绝区零 目前仅支持官服。
> 注意：以上渠道为代码层面已支持的范围，**仅原神经过实际测试**，其余游戏/渠道尚未充分验证，欢迎提 Issues 反馈。

**直播流监控平台**（监视直播间时支持的来源）：

| 平台 | 直播间链接示例 |
| :--- | :--- |
| B 站 | `https://live.bilibili.com/<RID>` |
| 抖音 | `https://live.douyin.com/<RID>` |

> 直播流监控与「游戏登录渠道」相互独立：上述任意平台直播间的二维码均可用于登录上表中的任意游戏官服 / 崩坏3 Bilibili 服。
> 注意：直播流监控**仅 B 站经过实际测试**，抖音直播流尚未充分验证，欢迎提 Issues 反馈。

## 使用说明

> ⚠️ **本 fork 不提供打包后的程序（Release 资源）**。为避免被少数人拿去二次打包盈利/商业化，本仓库**不发布任何预编译好的可执行文件或安装包**。
> 如果你确实需要可用的程序，有以下途径：
> 1. **自行编译运行**：按下方「编译」一节，自己安装 Visual Studio、配置好依赖环境，手动编译、运行与调试（本 fork 采用手动依赖方式，详见「编译」）。
> 2. **联系我**：通过邮箱 `<xiaomaimainp@qq.com>` 或其他渠道联系我获取帮助。
> 3. **等待源作者更新**：关注源作者仓库 [Theresa-0328/MHY_Scanner](https://github.com/Theresa-0328/MHY_Scanner) 的 Releases，等待其发布修复了相关问题的包体。
>
> 上方的 Releases 链接指向**源作者**仓库；本 fork 自身不提供可直接下载运行的包。

[点击Releases](https://github.com/Theresa-0328/MHY_Scanner/releases) 选择最新版本下载解压

[点击下载安装Visual C++ 运行时库](https://aka.ms/vs/17/release/vc_redist.x64.exe)，  详细解释查看[Microsoft官方文档](https://learn.microsoft.com/zh-cn/cpp/windows/latest-supported-vc-redist?view=msvc-170)。


运行 MHY_Scanner.exe

点击菜单栏 **账号管理->添加账号**，添加你的账号。

双击账号对应的备注单元格可以添加自定义备注。

> 🔒 **账号与 cookie 安全提示**
> - 软件会在本地保存账号信息（含 cookie / 登录态），**请勿将自己的账号信息（cookie、stoken 等）通过软件打包、压缩或其他任何途径发送给他人**。
> - 若不慎泄露，请立即登录[米哈游通行证](https://user.mihoyo.com/?lang=zh-cn#/login) **修改密码**，随后重新登录以作废旧登录态。
> - 请妥善保管个人财产与账号，避免因信息泄露造成损失。

登陆后点击 **监视屏幕** 就可以自动识别任意显示在屏幕上的二维码并自动登录。

选择你需要的直播平台,在当前直播间输入框输入`RID`，点击 **监视直播间** 就可以自动识别该直播间显示的二维码并自动登录。

正在执行的任务的按钮会高亮显示，再次点击会停止。

`RID`是纯数字，一般从直播间链接中获得。

|                平台                |           `<RID>` 位置            |
| :--------------------------------: | :-------------------------------: |
| [B 站](https://live.bilibili.com/) | `https://live.bilibili.com/<RID>` |
|  [抖音](https://live.douyin.com/)  |  `https://live.douyin.com/<RID>`  |

<span style="color:red">**目前没有进行大量测试，如果有任何建议和问题欢迎提 Issues。**</span>

## 编译
本仓库提供两种编译方式：

1. **源作者方式（vcpkg）**：仓库中的 `CMakeLists.txt` 与 `vcpkg.json` 保持源作者原始配置（vcpkg manifest 模式），按 CI/CD 工作流使用 vcpkg 自动拉取依赖即可。
2. **本 fork 维护者方式（手动依赖）**：**未使用 vcpkg**。所有依赖库（OpenCV、FFmpeg、Qt、nlohmann/json、libcurl 等）均**手动下载并固定本地路径**完成编译；相关本地路径属于个人构建环境，不提交到仓库（第三方库目录与构建产物已由 `.gitignore` 忽略）。如需自行编译，可沿用源作者的 vcpkg 流程，或参照上述手动方式配置本地库路径。

## 变更记录（v1.1.15 → v1.1.16）

本次更新为一次综合重构，涵盖米游社登录/扫码接口升级、直播流链接获取增强、扫码性能优化（核心改动）；构建方面本 fork 保持源作者 `CMakeLists.txt` / `vcpkg.json` 不变，采用手动依赖方式编译（详见下文「四、构建说明」）。

### 一、扫码核心优化（本次重点）
- **`src/Core/QRScanner.cpp` / `QRScanner.h`**：`decodeSingle` 新增经典 `cv::QRCodeDetector` 作为**快速优先路径**（纯 CV、无神经网络，单帧约 1~5ms），仅在快速路径失败时回退到 `WeChatQRCode`（深度学习，更鲁棒但慢）。大幅提升清晰、正向二维码的解码速度。
- **`src/UI/QRCodeForStream.cpp` / `QRCodeForStream.h`**：
  1. **背压模型（改动二）**：直播流从原来的"线程池 `tryStart`（线程满静默丢帧）"改为"**最新帧单槽位 + 独立解码线程**"。读帧循环只把最新一帧写入单槽位（旧帧被覆盖），解码线程持续解最新帧，保证二维码所在帧一定被解到、不会因帧洪流丢帧。
  2. **① 低延迟拉流参数**：`setUrl` 增加 `fflags=nobuffer`、`analyzeduration=500000`、`tcp_nodelay=1`；`init` 解码器开启前加 `AV_CODEC_FLAG_LOW_DELAY`，减少启动缓冲与解码延迟。
  3. **② 启动缓冲排空**：新增 `drainStartupBuffer()`，连上直播后先用独立的 `AVPacket`/`AVFrame` 快速丢弃 CDN 灌下的积压旧帧（最多 2s），追上直播点后再启动解码线程，避免从几秒前的旧画面开始扫。
- 以上扫码优化均为无调试日志的"干净形态"。

### 二、米游社登录与扫码 API 升级
- **`src/Core/MhyApi.hpp`**：二维码登录接口从旧的 `hk4e::qrcode_fetch/query` 迁移到米游社 passport 新版 `createQRLogin` / `queryQRLoginStatus` / `scanQRLogin` / `confirmQRLogin`，并携带 `x-rpc-app_id` / `x-rpc-device_id` 请求头；`GetLoginQrcodeUrl` / `GetQRCodeState` 返回值扩展（新增 `ticket`）；新增 `WriteScannerLog` 写入 `Config/scanner.log`；默认登录游戏改为星穹铁道。
- **`src/Core/ApiDefs.hpp`**：新增 `passport` 命名空间下各二维码接口，以及 `hkrpg` / `nap` / `takumi` 的补充接口。
- **`src/Core/ScannerBase.hpp`**：游戏类型识别从旧的 magic key（`8F3`/`9E&`/`8F%`/`%BA`）改为米游社 biz_key（`bh3_cn`/`hk4e_cn`/`hkrpg_cn`/`nap_cn`），新增 `setGameTypeByAppId`；新增 `lastQrCode`、`mid` 字段；屏幕与直播扫码统一走 `parseOfficialQRCode` 解析 `ticket`。
- **`src/Core/UrlQuery.hpp`（新增）**：URL 参数解析工具类，被 `MhyApi` / `ScannerBase` 引用。

### 三、直播流链接获取增强（`src/Core/LiveStreamLink.cpp`）
- 新增 B 站直播间 RID 解析：`NormalizeBiliRoomID` 支持纯数字、`live.bilibili.com/<RID>`、`blanc/` 子路径、`?room_id=` 等多种形式。
- 新增 `BiliHeaders`（带 `User-Agent`/`Referer`/`Origin`）与 `BuildBiliStreamUrl`（从 codec 信息拼装真实流地址），提升直播间地址获取的鲁棒性。

### 四、构建说明（保持源作者构建文件不变）
- **`CMakeLists.txt` / `vcpkg.json`**：本 fork **保持源作者原始文件不变**（vcpkg manifest 模式），未做修改、未删除。
- **本 fork 实际编译方式**：维护者**未使用 vcpkg**，而是手动下载全部依赖库（OpenCV、FFmpeg、Qt、nlohmann/json、libcurl 等）并固定本地路径完成编译；相关本地路径属于个人构建环境，不提交到仓库。构建产物与第三方库目录已由 `.gitignore` 忽略（如 `/3rdparty/ffmpeg-*`、`/3rdparty/nlohmann/`、`/3rdparty/libcurl-*` 等）。
- **`cmake/install_ffmpeg.cmake`**：FFmpeg 安装/构建脚本调整（仅在需要自动拉取 FFmpeg 时用到）。
- **`.gitignore`**：忽略项调整，适配手动依赖目录与构建输出。

### 五、UI 与交互
- **`src/UI/WindowMain.cpp` / `WindowMain.ui` / `WindowMain.h`**：账号管理表格新增"状态"列（5 列 → 6 列）；界面布局调整（复选框均分、列宽等）；Issues 链接改到 fork 仓库 `xiaomaimainp/MHY_Scanner`。
- **`src/UI/QRCodeForScreen.cpp` / `QRCodeForScreen.h`**：屏幕扫码适配新版 `QRScanner` 与 passport 登录流程（`parseOfficialQRCode` + `PandaScanQRCode`/`scanCheck` + `continueLastLogin`），去掉旧的按字符串偏移截取游戏类型的写法；新增 `setLoginInfo1`（带 `stoken`/`mid`）。
- **`src/UI/WindowAbout.cpp` / `WindowLogin.cpp`**：配套适配与交互微调。
- **`src/Resources/MHY_Scanner.rc`**：版本资源号更新。

### 六、其他清理
- 去除多个源文件的 UTF-8 BOM 头（统一为无 BOM），并做配套头文件依赖调整（`CookieParser.hpp`、`CreateUUID.hpp`、`CryptoKit.cpp/.h`、`ScreenScan.cpp/.h`、`ScreenShotDXGI.hpp`、`TimeStamp.hpp`、`UtilString.hpp`、`compile_string.hpp`、`UtilMat.hpp`、`WindowGeeTest.cpp/.h`、`BSGameSDK.hpp`、`ConfigDate.cpp/.h` 等）。
- `src/UI/main.cpp`、`src/UI/WindowGeeTest.*` 等做相应适配。

### 改动文件清单（相对 v1.1.15）
| 文件 | 主要改动 |
| :--- | :--- |
| `CMakeLists.txt` / `vcpkg.json` | **保持源作者原始文件不变**（未修改、未删除） |
| `cmake/install_ffmpeg.cmake` | FFmpeg 脚本调整 |
| `.gitignore` | 忽略项调整（适配手动依赖目录与构建输出） |
| `src/Core/MhyApi.hpp` | passport 新版二维码登录 API + 日志文件 |
| `src/Core/ApiDefs.hpp` | 新增 passport / hkrpg / nap 接口 |
| `src/Core/ScannerBase.hpp` | biz_key 游戏类型识别 + `setGameTypeByAppId` |
| `src/Core/UrlQuery.hpp` | 新增：URL 参数解析工具 |
| `src/Core/QRScanner.cpp` / `.h` | 快速解码器优先路径（改动一） |
| `src/Core/LiveStreamLink.cpp` / `.h` | B 站直播间 RID/流地址解析增强 |
| `src/UI/QRCodeForStream.cpp` / `.h` | 背压模型 + 低延迟参数 + 启动排空（改动二/①/②） |
| `src/UI/QRCodeForScreen.cpp` / `.h` | 适配新版扫码与登录流程 |
| `src/UI/WindowMain.cpp` / `.h` / `.ui` | 账号表"状态"列、布局、Issues 链接 |
| `src/UI/WindowAbout.cpp` | 配套微调 |
| `src/UI/WindowLogin.cpp` | 登录交互微调 |
| `src/Resources/MHY_Scanner.rc` | 版本资源更新 |
| 其余 `src/Core/*`、`src/UI/*` 小文件 | BOM 清理与依赖头配套调整 |
## 相关项目
- [KuRo_Scanner](https://github.com/Theresa-0328/KuRo_Scanner) – 鸣潮扫码器
  
- [xiaomaimainp/MHY_Scanner_py](https://github.com/xiaomaimainp/MHY_Scanner_py)
  
## 参考和感谢
- [MR-LIYA/MHY_Scanner](https://github.com/MR-LIYA/MHY_Scanner)

- [DSVVA/MHY_Scanner](https://github.com/DSVVA/MHY_Scanner)

- [loqwe/MHY_Scanner2](https://github.com/loqwe/MHY_Scanner2)