## 支持平台<div id="1"/>

- Windows 10（64位）或更高版本
- Ubuntu Linux 18.04 （64位）或更高版本
- 在Windows平台下编译 

## 编译3DViewer<div id="2"/>

依赖环境：

- Qt 5.10.1 或更高版本，Qt下载地址见：[https://download.qt.io/](https://download.qt.io/)
- Windows
  - Visual Studio 2015或者更高版本
- MacOS
  - 最新Xcode
- Linux
  - g++ 7.5.0 或更高版本

依赖库：

- 3DCamera
- OpenSceneGraph 3.6.5 或更高版本
- zlib
- libpng
- quazip
- yaml-cpp

### 在Windows平台下编译<div id="3"/>

1. 安装 [Qt5](https://download.qt.io/)、[Visual Studio 2015 或更高版本（社区版）](https://visualstudio.microsoft.com)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads)（安装 CMake 和 Git 时将安装路径添加到 windows 系统环境变量中）

2. 使用 Git 下载代码

   - 新建代码存放文件夹：例如：***D:\3DViewer-git***
   - 使用 git clone 命令下载代码，按 ***windows + R*** 快捷键打开命令提示符窗口，输入 ***cmd.exe***，然后输入以下命令：

   ```
   cd /D D:\3DViewer-git
   git clone https://github.com/Revopoint/3DViewer.git
   ```

3. 执行 CMake 命令新建 Visual Studio 工程（以 Qt 5.10.1 + Visual Studio 2015 为例）

   - 在下载好代码根目录新建build文件夹，然后打开命令提示符，进入build目录：

     ```
     cd D:\3DViewer-git\3DViewer\build
     ```

   - 执行 CMake，需要修改 ***-DQt5_DIR=*** 后面携带的 Qt 安装路径，***Qt5_DIR*** 指向 ***Qt5Config.cmake*** 存放位置，***-G"Visual Studio 14 2015 Win64"*** 指明生成Visual Studio 2015 工程。

     ```
     cmake -DQt5_DIR=c:\Qt5.10.1\5.10.1\msvc2015_64\lib\cmake\Qt5 -G"Visual Studio 14 2015 Win64" ..\src\
     ```

4. 使用 Visual Studio 2015 打开 build 目录下生成的 ***3DViewer.sln*** 解决方案文件

5. 打开工程后，按 ***Ctrl + Shift + B*** 进行编译，或者按 ***F5*** 直接运行程序

### 在MacOS平台下编译<div id="4"/>

1. 安装[Qt5](https://download.qt.io/)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads)、Xcode

2. 打开终端，新建代码存放文件夹，例如：***~/3DViewer-git***，执行 git clone 下载代码

   ```
   mkdir ~/3DViewer-git
   cd ~/3DViewer-git
   git clone https://github.com/Revopoint/3DViewer.git
   ```

3. 在代码根目录新建build目录，进入该目录执行CMake，***-DQt5_DIR=*** 后面携带Qt安装目录，指向 ***Qt5Config.cmake*** 存放位置

   ```
   mkdir 3DViewer/build
   cd 3DViewer/build
   cmake -DQt5_DIR=~/Qt5.10.1/5.10.1/clang_64/lib/cmake/Qt5 ../src/
   ```

4. 在build目录下执行make，完成项目编译

   ```
   make
   ```

5. 编译完成后在访达中鼠标双击 ***./bin/3DViewer.app*** 运行程序

   （USB连接相机时，需要先运行代码根目录下 ***scripts/cs_rpc_install.sh*** 脚本）

### 在Linux（Ubuntu）平台下编译<div id="5"/>

1. 安装[Qt5](https://download.qt.io/)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads)、g++ （Qt 安装参照 Qt 官网教程）

   ```
   sudo apt -y install cmake g++ git 
   ```

   - 安装Qt（以Qt 5.10.1为例）

     ```
     wget https://download.qt.io/new_archive/qt/5.10/5.10.1/qt-opensource-linux-x64-5.10.1.run
     chmod +x qt-opensource-linux-x64-5.10.1.run
     ./qt-opensource-linux-x64-5.10.1.run
     ```

   - 安装OpenGL依赖库

     ```
     sudo apt install mesa-common-dev
     ```

2. 打开终端，新建代码存放文件夹，例如：***~/3DViewer-git***，执行 ***git clone*** 下载代码

   ```
   mkdir ~/3DViewer-git
   cd ~/3DViewer-git
   git clone https://github.com/Revopoint/3DViewer.git
   ```

3. 在代码根目录新建 ***build*** 目录，进入该目录执行 CMake，***-DQt5_DIR=*** 后面携带Qt安装目录，指向 ***Qt5Config.cmake*** 存放位置

   ```
   mkdir 3DViewer/build
   cd 3DViewer/build
   cmake -DQt5_DIR=~/Qt5.10.1/5.10.1/clang_64/lib/cmake/Qt5 ../src/
   ```

4. 在build目录下执行make，完成项目编译

   ```
   make
   ```

5. 编译完成后执行***./bin/3DViewer***运行程序

   ```
   ./bin/3DViewer
   ```

   （USB连接相机时，需要先运行代码根目录下 ***scripts/cs_uvc_config.sh*** 脚本）

