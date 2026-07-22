# EUI-NEO Demo

基于 [EUI-NEO](https://github.com/sudoevolve/EUI-NEO) 制作的原生 C++ 桌面演示程序。

## 功能展示

- 响应式侧边导航和顶部区域
- 深色、浅色主题切换
- 指标卡、进度条与任务列表
- 输入框、复选框、开关、滑块、分段选择和标签页
- 基于 `eui::Signal` 的交互状态
- Windows x64 自动构建和 ZIP 打包

## Windows 构建

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

构建产物位于：

```text
build/Release/EUI-NEO-Demo.exe
```

## Linux 构建

安装 CMake、C++17 编译器以及 OpenGL、GLFW 所需的开发依赖后执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## 发布

合并到 `main` 后，GitHub Actions 会自动创建 `v0.1.0` Release，并上传：

```text
EUI-NEO-Demo-windows-x64.zip
```
