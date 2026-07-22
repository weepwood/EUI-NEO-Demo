# EUI-NEO Demo

基于 [EUI-NEO](https://github.com/sudoevolve/EUI-NEO) 制作的原生 C++17 桌面演示程序。

## 演示内容

- 响应式侧边导航
- 深色与浅色主题切换
- 指标卡、任务列表和进度状态
- 输入框、分段选择、开关和滑块
- 本地状态绑定与交互反馈
- Windows x64 自动构建与 ZIP 打包

## Windows 构建

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
.\build\Release\EUI-NEO-Demo.exe
```

## Linux 构建

先安装 EUI-NEO 所需的 X11、OpenGL 与 libcurl 开发依赖，然后执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/EUI-NEO-Demo
```

EUI-NEO 依赖固定到 0.5.2 对应提交，避免上游主分支变化导致构建结果漂移。
