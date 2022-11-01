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
Converter.exe --format <FORMAT> --input <INPUT> --output <OUTPUT> [OPTIONS] 
```

## 示例命令

```sh
# from osgb dataset
Converter.exe --format osgb -input <OSGB Path> --output <Out Path>  --yUpAxis true

# from single shp file
Converter.exe --format gdal --input <Shapefile Path> --output <DIR>  --field height --layer <Shapefile Name>

# from gdb file
Converter.exe --format gdal --input <GDB Path> --output <DIR>  --field height --layer <Layer Name>
```

## 参数说明
- `--format <FORMAT>` 输入数据格式。

  `FORMAT` 可选：OSGB, GDAL

  `可选：OSGB` 为倾斜摄影格式数据, `GDAL` 为GDAL支持的面(Polygon)数据

- `--input <PATH>` 输入数据的目录，osgb数据截止到 `<DIR>/Data` 目录的上一级，GDAL参考GDAL数据格式。

- `--output <DIR>` 输出目录。OSGB转换的3DTiles输出的数据文件位于 `<DIR>/Data` 目录, GDAL转换的3DTiles输出的数据文件位于`<DIR>/Tile`目录，`tileset.json`位于`<DIR>`根目录。

- `--field` 高度字段。指定GDAL数据集中的高度属性字段，此项为转换 GDAL 时的必须参数。

- `--offset` OSGB高度偏移字段。
- `--thread` 处理线程数量（仅对OSGB生效）
- `--yUpAxis` 是否将gltf模型转为y轴朝上

  `yUpAxis` 可选： true, false  
  如果是用`Cesium for Unreal`加载数据，需要启用yUpAxis

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
1. vcpkg install "osg" "osg[plugins]"
2. 如果没有Qt5.15，可以通过以下两种方式安装：
   1. 通过Qt官网安装
   2. vcpkg install "qt5[all]"
3. Visual Studio选择CMakeLists.txt，导入工程，编译
4. 将OSGPlugins-${Version}文件夹复制到编译目录下

# Reference
1. 3dtiles [https://github.com/fanvanzh/3dtiles](https://github.com/fanvanzh/3dtiles)
2. 3dtiles specification [https://github.com/CesiumGS/3d-tiles](https://github.com/CesiumGS/3d-tiles)