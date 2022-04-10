# About Project
1. 整体参考[https://github.com/fanvanzh/3dtiles](https://github.com/fanvanzh/3dtiles),  向作者致敬，市面上唯一的开源好用的3DTILES转换工具，在学习过程中，从中收获很多，对3DTiles、OSGB、GLTF等数据格式有了进一步了解。
2. 原工程基于C++、C和Rust，本人本身对Rust不了解，但是学习过程中难免涉及到调试，也看了整个代码，个人理解作者采用Rust和C++混编应该是两个问题：
   1. OpenSceneGraph没有除了C++之外其他版本，而且我们在用这个工具时，最主要的就是OSGB转3DTILES
   2. 本身C++可以跨平台，但是C++对字符串、文件系统支持不够好，Rust可以很好满足这一点，并且Rust也便于和C++交互
   3. 综合下来，原作者使用了Rust和C++来编写，一是保证跨平台，二是保证了性能。
   
3. 本人本身是对Qt比较熟悉的，就萌生了这个想法，用C++和Qt，从新梳理整个框架，刚好今年春节一个人，秉着学习想法，使用纯C++（加了Qt）重写了该工程，新的工程基于C++ 17标准重写，使用了Qt5.15，主要作者本身写C++大部分时候都是基于Qt，对Qt也稍微了解，同时也借助Qt很好的跨平台特性，性能没有变化，并且也便于调试学习。
   1. 整个工程开源，但是鉴于Qt一些限制，该程序采用LGPL协议。
   2. 后续会与作者同步保持更新，后期希望可以加入自己一些特色功能。
   
# 简介

3DTiles 转换工具集, 快速将OSGB、Shp、GDB等格式转为Cesium 3DTiles, 方便搭建三维平台。

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

# How To Build (Windows)
1. vcpkg install "osg" "osg[plugins]"
2. 如果没有Qt5.15，可以通过以下两种方式安装：
   1. 通过Qt官网安装
   2. vcpkg install "qt5[all]"
3. VS2019选择CMakeLists.txt，导入工程，编译
4. 将OSGPlugins-${Version}文件夹复制到编译目录下