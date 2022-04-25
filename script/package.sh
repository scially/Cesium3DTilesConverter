#! /bin/bash

cd ../
linuxdeploy-x86_64.AppImage --appdir AppDir -e ./binary/linux/release/Converter -i ./script/Converter.png --desktop-file ./script/Converter.desktop -o appimage

cp -r ./gdal_data ./proj_data ./AppDir/usr/bin
cp -r /usr/lib/x86_64-linux-gnu/osgPlugins-3.6.5 ./AppDir/usr/bin
