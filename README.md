# About Project

基于C++17、Qt5的3DTiles 转换工具集。

# 简介

3DTiles 转换工具集, 快速将OSGB、Shp、GDB等格式转为Cesium 3DTiles。

这是一个基于c++ 17 和 Qt5.15 项目。

提供了如下的子工具：

- `OSGB(OpenSceneGraph Binary)` 转 `3DTiles`
- `GDAL数据集` 转 `3DTiles`

# 用法说明

##  命令行格式

```sh
Converter -f <FORMAT> [OPTIONS] <INPUT> <OUTPUT> 
```

## 示例命令

```sh
# from osgb dataset
Converter -f OSGB --yUpAxis  <OSGB> <OUTPUT> 

# from shp file
Converter -f GDAL --field height <Shapefile> <OUTPUT>

# from gdb file
Converter -f GDAL --field height --layer <Layer> <GDB> <OUTPUT>
```

## 参数说明
```
Options:
  -?, -h, --help           Displays help on commandline options.
  
  -f, --format <format>    OSGB or GDAL(required), OSGB 为倾斜摄影格式数据, GDAL 为GDAL支持的面(Polygon)数据
  
  -l, --level <level>      OSGB max level, OSGB处理到的最大级别
  
  -F, --field <field>      height field name(required for gdal), 指定GDAL数据集中的高度属性字段，此项为转换 GDAL 时的必须参数。
  
  -L, --layer <layer>      layer name(required when input isn't shapefile)
  
  -t, --thread <thread>    thread number, 处理线程数量
  
  -y, --yUpAxis            y up axis, 是否将gltf模型转为y轴朝上（仅对OSGB生效）,如果是用Cesium for Unreal，需要指定yUpAxis
  
  -H, --height <height>    height offset(default value 0), OSGB高度偏移字段
  
  <INPUT> 输入数据的目录，OSGB数据截止到 `<DIR>/Data` 目录的上一级，GDAL参考GDAL数据格式。

  <OUTPUT> 输出目录。OSGB转换的3DTiles输出的数据文件位于 <DIR>/Data`目录, GDAL转换的3DTiles输出的数据文件位于<DIR>/Tile目录，tileset.json位于<DIR>根目录。
```

# 数据要求及说明

### 倾斜摄影数据

倾斜摄影数据仅支持 smart3d 格式的 osgb 组织方式：

- 数据目录必须有一个 `Data` 目录的总入口；
- `Data` 目录同级放置一个 `metadata.xml` 文件用来记录模型的位置信息；
- 每个瓦片目录下，必须有个和目录名同名的 osgb 文件，否则无法识别根节点；

正确的目录结构示意：

```
- Your-data-folder
  ├ metadata.xml
  └ Data/Tile_000_000/Tile_000_000.osgb
```

### GDAL

目前仅支持GDAL格式的面数据，可用于建筑物轮廓批量生成3DTiles.

图层中需要有字段来表示高度信息。

# How To Build
1. vcpkg install "gdal" "osg" "osg[plugins]"
2. 如果没有Qt5.15，可以通过以下两种方式安装：
   1. 通过Qt官网安装
   2. vcpkg install "qt5[all]"
3. Visual Studio选择CMakeLists.txt，导入工程，编译
4. 将OSGPlugins-${Version}文件夹复制到编译目录下

# Reference
1. 3dtiles [https://github.com/fanvanzh/3dtiles](https://github.com/fanvanzh/3dtiles)
2. 3dtiles specification [https://github.com/CesiumGS/3d-tiles](https://github.com/CesiumGS/3d-tiles)