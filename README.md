# VertiX
VertiX – 防晕3D视觉稳定器
中文说明
简介
VertiX 是一款免费的桌面覆盖层工具，通过在屏幕中心显示可自定义的坐标轴与十字准星，为您的视觉提供一个稳定的参考点，有效减轻3D游戏、VR应用或其他动态画面引起的眩晕（晕动症）。所有参数实时调整，无延迟，无广告，完全免费。

重要提示：本工具仅适用于窗口模式或无边框窗口模式（Borderless Window）的全屏应用。在传统的独占全屏（Fullscreen Exclusive）模式下，覆盖层可能无法正常显示或被游戏覆盖。请确保您的游戏或应用设置为窗口化或无边窗口模式，以获得最佳体验。

为什么能缓解晕3D？
晕动症常因视觉感知与内耳前庭系统不匹配引发。

屏幕中心的固定参考系（准星与轴线）让大脑获得稳定的空间锚点，降低视觉震荡引起的冲突感。

研究表明，视觉参考点能显著缓解动态场景下的不适。

主要功能
十字准星：可独立开关，大小（1-1000像素）、粗细（1-20像素）、透明度（0-255）和颜色（红/绿/蓝）实时调节。

坐标轴：始终显示（无箭头），水平/垂直长度可分别设为0-100%屏幕宽度/高度（从边缘向中心延伸），粗细、透明度、颜色同样实时可调。

即时反馈：所有修改立即生效，无需点击“应用”按钮。

透明穿透：覆盖层只绘制图形，鼠标可直接穿透，不影响游戏操作。

运行环境
Windows 7/8/10/11（32或64位）

系统自带 GDI+，无需额外安装

使用方法
下载并运行 VertiX.exe，将弹出两个窗口：

覆盖窗口：全屏置顶，显示坐标轴和准星。

配置窗口：调整所有参数。

在配置窗口中修改数值、切换显示、选择颜色，覆盖层会同步更新。

点击配置窗口底部的 Close 按钮或关闭配置窗口即可退出。

编译（开发者）
使用 MinGW-w64：

bash
g++ -mwindows -o VertiX.exe overlay.cpp -lgdi32 -lgdiplus -lm
或 Visual Studio：

bash
cl overlay.cpp user32.lib gdi32.lib gdiplus.lib
注意事项
覆盖层以黑色（RGB 0,0,0）为透明键，请勿绘制纯黑图形。

若覆盖层未置顶，尝试调整窗口模式或无边框模式。

部分全屏独占应用无法叠加，请切换至窗口或无边框模式。

协议
MIT License – 免费使用、修改、分发，保留版权声明即可。

English Version
Introduction
VertiX is a free desktop overlay tool that displays a customizable crosshair and coordinate axes at the screen center, providing a stable visual reference point to alleviate motion sickness (cybersickness) caused by 3D games, VR applications, or any dynamic visuals. All parameters update in real-time – no latency, no ads, completely free.

Important: This tool works only in Windowed Mode or Borderless Window Mode. It may not appear over traditional exclusive fullscreen applications. Please set your game or app to windowed or borderless window mode for the overlay to function properly.

Why It Helps with Motion Sickness
Motion sickness often arises from a mismatch between visual perception and inner ear vestibular signals.

A fixed central reference (crosshair and axes) provides a stable spatial anchor for the brain, reducing disorientation and visual conflicts.

Studies show that visual reference points can significantly reduce discomfort in dynamic scenes.

Key Features
Crosshair: Independently toggle on/off; adjust size (1-1000 px), thickness (1-20 px), alpha (0-255), and color (red/green/blue) in real time.

Axes: Always visible (no arrows); horizontal/vertical length separately adjustable as a percentage (0-100%) of screen width/height (extending from edges toward center); thickness, alpha, and color also adjustable instantly.

Instant Feedback: All changes take effect immediately – no "Apply" button needed.

Transparent & Click-Through: The overlay displays only graphics; mouse clicks pass through, so game controls remain unaffected.

System Requirements
Windows 7/8/10/11 (32-bit or 64-bit)

GDI+ (built-in, no extra install)

How to Use
Run VertiX.exe. Two windows appear:

Overlay window: Full-screen, always-on-top, showing axes and crosshair.

Configuration window: Adjust all parameters.

Change values, toggle checkboxes, or select colors – the overlay updates immediately.

Click the Close button at the bottom of the configuration window or close it via the title bar ‘X’ to exit.

Building from Source (Developers)
With MinGW-w64:

bash
g++ -mwindows -o VertiX.exe overlay.cpp -lgdi32 -lgdiplus -lm
With Visual Studio:

bash
cl overlay.cpp user32.lib gdi32.lib gdiplus.lib
Important Notes
The overlay uses black (RGB 0,0,0) as the transparency key; do not draw pure black elements.

If the overlay doesn't appear on top, try adjusting the window mode or borderless mode.

Exclusive fullscreen applications may block the overlay; switch to windowed or borderless window mode.

License
MIT License – free to use, modify, and distribute, provided that the copyright notice is retained.
