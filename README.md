# About Project

基于C++17、Qt5的3DTiles转换工具。

# 简介

3DTiles转换工具, 快速将OSGB转为Cesium 3DTiles。

这是一个基于c++ 17 和 Qt5.15 项目。

提供 `OSGB(OpenSceneGraph Binary)` 转 `3DTiles`

# 用法说明

简化命令行传入参数，默认采用GLTF2.0格式(y轴向上),多线程并行。
##  命令行格式

```sh
Converter -f <FORMAT> [OPTIONS] <INPUT> <OUTPUT> 
```

## 示例命令

```sh
# from osgb dataset
Converter -f OSGB -i <OSGB> -o <OUTPUT> 
```

## 参数说明
```
Options:
  -?, -h, --help         displays help on commandline options.
  -f, --format <format>  OSGB or Vector(required), OSGB 为倾斜摄影格式数据, Vector为GDAL支持的面(Polygon)数据
  -i, --input  <INPUT>   输入数据的目录，OSGB数据截止到 `<DIR>/Data` 目录的上一级，GDAL参考GDAL数据格式。
  -o, --ouput  <OUTPUT>  输出目录。OSGB转换的3DTiles输出的数据文件位于 <DIR>/Data`目录, GDAL转换的3DTiles输出的数据文件位于<DIR>/Tile目录，tileset.json位于<DIR>根目录。
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
# Build Version

| gdal | openscenegraph | qt |
| :----: | :----: | :----: |
| 3.7.0 | 3.6.5 | 5.15 |

# How To Build(for Windows)
1. vcpkg install "gdal" "osg" "osg[plugins]"
2. 如果没有Qt5.15，可以通过以下两种方式安装：
   1. 通过Qt官网安装
   2. vcpkg install "qt5[all]"
3. Visual Studio选择CMakeLists.txt，导入工程，编译
4. 将OSGPlugins-${Version}文件夹复制到编译目录下

# Reference
1. 3dtiles specification [https://github.com/CesiumGS/3d-tiles](https://github.com/CesiumGS/3d-tiles)