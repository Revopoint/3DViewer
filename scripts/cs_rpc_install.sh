#!/bin/sh

# copy files
sudo cp ../thirdparty/3DCamera/mac/x64/com.revopoint3d.CameraHelper.plist /Library/LaunchDaemons/
sudo cp ../thirdparty/3DCamera/mac/x64/rpcService /Library/PrivilegedHelperTools/
sudo cp ../thirdparty/3DCamera/mac/x64/lib3DCamera.dylib /Library/PrivilegedHelperTools/

# chown
sudo chown root /Library/LaunchDaemons/com.revopoint3d.CameraHelper.plist
sudo install_name_tool -add_rpath /Library/PrivilegedHelperTools /Library/PrivilegedHelperTools/rpcService

# launch
sudo launchctl load /Library/LaunchDaemons/com.revopoint3d.CameraHelper.plist