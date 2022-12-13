## Support platform<div id="1"/>

- Windows 10 (64 bit) or later
- Ubuntu Linux 18.04 (64 bit) or higher
- MacOS 10.15 (64 bit) or higher

## Compile 3DViewer<div id="2"/>

Dependent environment:

- Qt5 (Qt 5.10.1 or higher),  Qt download address is shown in: [https://download.qt.io/](https://download.qt.io/)
- Windows
  - Visual Studio 2015 or later
- MacOS
  - Latest Xcode
- Linux
  - g++ 7.5.0 or higher
- Dependent libraries:
  - 3DCamera
  - OpenSceneGraph 3.6.5 or higher
  - zlib
  - libpng
  - quazip
  - yaml-cpp

### Compile under Windows platform<div id="3"/>

1. Installation [Qt5](https://download.qt.io/)、[Visual Studio 2015 or later (Community Edition)](https://visualstudio.microsoft.com)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads) (Add the running path to the windows system environment variable when installing CMake and Git)

2. Use Git to download code

   - Create a new code storage folder, for example: ***D:\3DViewer-git***

   - Use the Git clone command to download the code, press the ***windows + R*** shortcut key to open a command prompt window, enter ***cmd.exe***，and then enter the following command:

     ```
     cd /D D:\3DViewer-git
     git clone https://github.com/Revopoint/3DViewer.git
     ```

3. Create a new Visual Studio project by CMake (take QT 5.10.1 + Visual Studio 2015 as an example).

   - Create a new build folder in the root directory of downloaded code, open a command prompt, and enter the build directory:

     ```
     cd D:\3DViewer-git\3DViewer\build
     ```

   - To execute CMake, you need to modify ***-DQt5_Dir =‘……’*** which points to ***Qt5Config.cmake*** storage location, and ***-G "Visual Studio 14 2015 Win64"*** indicates to generate a Visual Studio 2015 project.

     ```
     cmake -DQt5_DIR=c:\Qt5.10.1\5.10.1\msvc2015_64\lib\cmake\Qt5 -G"Visual Studio 14 2015 Win64" ..\src\
     ```

4. Use Visual Studio 2015 to open the 3DViewer.sln solution file generated in the build directory.

5. After opening the project, press ***Ctrl+Shift+B*** to compile, or press ***F5*** to run the program directly.

### Compile under MacOS platform<div id="4"/>

1. Installation [Qt5](https://download.qt.io/)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads)、Xcode.

2. Open the terminal, create a new code storage folder, for example: ***~/3DViewer-git***. Then execute git clone to download the code.

   ```
   mkdir ~/3DViewer-git
   cd ~/3DViewer-git
   git clone https://github.com/Revopoint/3DViewer.git
   ```

3. Create a new build directory in the code root directory, enter the directory and execute CMake command.  ***-DQt5_ Dir =‘……’*** points to Qt5Config.cmake storage location.

   ```
   mkdir 3DViewer/build
   cd 3DViewer/build
   cmake -DQt5_DIR=~/Qt5.10.1/5.10.1/clang_64/lib/cmake/Qt5 ../src/
   ```

4. Execute make in the build directory to complete the project compilation.

   ```
   make
   ```

5. After compilation, double-click ***bin/3DViewer.app*** to run the program.

   (When connecting the camera via USB, you need to run the ***scripts/cs_rpc_install.sh*** script under the code root directory first.)

### Compile on Linux (Ubuntu)platform<div id="5"/>

1. Installation  [Qt5](https://download.qt.io/)、[CMake](https://cmake.org/download/)、[Git](https://git-scm.com/downloads)、g++ (take Qt 5.10.1 as an example).

   ```
   sudo apt -y install cmake g++ git 
   ```

   - Install Qt5 (take Qt 5.10.1 as an example)

     ```
     wget https://download.qt.io/new_archive/qt/5.10/5.10.1/qt-opensource-linux-x64-5.10.1.run
     chmod +x qt-opensource-linux-x64-5.10.1.run
     ./qt-opensource-linux-x64-5.10.1.run
     ```

   - Install OpenGL dependencies

     ```
     sudo apt install mesa-common-dev
     ```

2. Open the terminal, create a new code storage folder, for example: ***~/3DViewer-git***. Then execute git clone to download the code.

   ```
   mkdir ~/3DViewer-git
   cd ~/3DViewer-git
   git clone https://github.com/Revopoint/3DViewer.git
   ```

3. Create a new build directory in the code root directory, enter the directory and execute CMake command.  ***-DQt5_ Dir =‘……’*** points to the storage location of ***Qt5config.cmake***.

   ```
   mkdir 3DViewer/build
   cd 3DViewer/build
   cmake -DQt5_DIR=~/Qt5.10.1/5.10.1/gcc_64/lib/cmake/Qt5 ../src/
   ```

4. Execute make in the build directory to complete the project compilation.

   ```
   make
   ```

5. After compilation, execute the ***./bin/3DViewer*** to run the program.

   ```
   ./bin/3DViewer 
   ```

   (When connecting the camera via USB, you need to run the ***scripts/cs_uvc_config. sh*** script in the code root directory first.)