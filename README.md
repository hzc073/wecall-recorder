# wecall-recorder – 自动拾音工具

![C++](https://img.shields.io/badge/c++-17-blue.svg) ![Windows](https://img.shields.io/badge/platform-Windows-0078D6.svg) ![License](https://img.shields.io/badge/license-MIT-green.svg)

**wecall-recorder** 是一款智能的 Windows 桌面端录音工具，旨在自动检测并录制任何使用麦克风的应用程序（如网络会议、在线通话、语音聊天等）。它能静默运行在后台，一旦检测到麦克风被占用，便会自动开始录制，并在通话结束后自动停止，为您无缝保留每一段重要对话。

## ✨ 主要功能

🎤 智能检测：实时监测系统麦克风使用情况，全自动开始/停止录音，无需任何手动操作。 

🎶 双通道录制**：同步捕获“您的麦克风声音”和“对方的扬声器声音”，确保通话双方的音频都被清晰录制。 

🎧 自动混音**：录音结束后，自动将双通道音频混音并编码为高质量的 MP3 文件。 

**⚙️ 高度可定制**：
设备自由选：自由选择要录制的输入（麦克风）和输出（扬声器）设备。 
独立增益：独立调节麦克风和扬声器的录音音量，平衡双方声音。 
实时电平：提供可视化电平监测，方便测试设备是否正常工作。 

🛡️ 进程黑名单：可添加不希望触发录音的应用程序（如游戏、浏览器），当这些程序使用麦克风时，录音不会启动。 

📂 文件管理：
 自定义路径：自由设定录音文件的保存位置。 
自动归档：录音文件会根据使用录音的应用名称自动创建文件夹，并以精确到秒的时间戳命名。 

🚀 便捷体验：支持开机自启动和最小化到系统托盘，实现真正的“一次设置，永久省心”。 

📄 详细日志：输出完整的运行日志，便于排查和定位潜在的 WASAPI 或 FFmpeg 相关错误。 

## 🛠️ 技术原理

本工具的核心工作流程分为三个阶段：

1. 麦克风占用检测：通过 Windows 核心音频 API (`IAudioSessionManager2`)，程序可以枚举当前所有正在使用音频会话（尤其是麦克风采集 `eCapture`）的进程。通过比对进程ID，可以判断是否有第三方应用正在使用麦克风。 

2.  双通道音频捕获 (WASAPI)：当检测到目标应用开始使用麦克风时，程序会启动两个独立的录音线程 ：
    - 麦克风输入：通过 WASAPI 的 `eCapture` 数据流捕获麦克风的音频。 
    - 扬声器输出：通过 WASAPI 的环回（Loopback）模式捕获系统正在播放的声音（即对方的声音）。 
  

3.  后期混音与转码 (FFmpeg)：当应用释放麦克风，录音停止后，程序会调用**外部的 `ffmpeg.exe` 命令行工具**。 它使用 `amix` 滤镜将两个临时 WAV 文件合并，并根据用户在设置中调节的音量增益进行调整，最终编码成一个 MP3 文件后，删除临时文件。 

## ⚠️ 重要：环境要求

在使用本软件前，请务必确保满足以下要求：

- **操作系统**：Windows Vista 或更高版本。
- **FFmpeg**：
    - 本程序**自身不包含 FFmpeg**，它依赖一个外部的 `ffmpeg.exe` 程序来完成最后的混音和编码工作。
    - 您必须**自行下载 FFmpeg**，并将其放置于本程序 `wecall-recorder.exe` 的**相同目录下**，或者将其路径添加到系统的**环境变量 `Path`** 中。
    - **关键**：您下载的 FFmpeg 版本**必须包含 `libmp3lame` MP3 编码器**。由于许可证原因，某些官方构建可能不含此编码器。
    - **推荐下载地址**：
        - **[gyan.dev](https://gyan.dev/ffmpeg/builds/)** (推荐下载 `full` 或 `essentials` 构建)
        - **[BtbN's builds](https://github.com/BtbN/FFmpeg-Builds/releases)**
    - 建议使用较新的 FFmpeg 版本（如 4.x 或更高），以确保所有命令行参数和滤镜都能被正确支持。

## 🚀 如何使用

1.  下载最新版本的 `wecall-recorder.exe`。
2.  按照[环境要求](#️-重要环境要求)配置好 `ffmpeg.exe`。
3.  直接运行 `wecall-recorder.exe`。
4.  在主界面点击 **【设置】** 按钮，进行初次配置：
    - **【设备】** 页面：选择您常用的麦克风和扬声器设备。可以通过“测试”按钮检查工作是否正常。
    - **【路径】** 页面：设置您希望保存录音文件的根目录。
    - **【常规】** 页面：根据需要开启“开机自启动”和“最小化到托盘”。
    - **【黑名单】** 页面：添加您不希望录音的程序名，例如 `chrome.exe`。
5.  返回主界面，点击 **【开始检测】**。程序将开始在后台守护您的麦克风。
6.  现在，您可以将主窗口关闭（根据设置会最小化到托盘或直接关闭UI），录音将在后台自动进行。

## ⚖️ 使用须知

- 本工具的初衷是仅供个人学习、备份个人参与的会议记录或通话内容。 
- **在您录制任何通话之前，请务必遵守您所在国家和地区的相关法律法规，并征得所有通话参与者的明确同意。** 
- **请勿将此工具用于任何侵犯他人隐私或进行非法活动的用途。开发者对因使用本软件而导致的任何法律纠纷不承担任何责任。** 

## 💻 技术栈

- **C++17**
- **Win32 API**
- **Windows Audio Session API (WASAPI)**
- **FFmpeg** (作为命令行工具调用)
- **libmp3lame** (通过 FFmpeg)

