https://github.com/fanvanzh/3dtiles

# About Project
1、向作者致敬，市面上唯一的开源好用的3DTILES转换工具，在学习过程中，从中收获很多，对3DTiles、OSGB、GLTF等数据格式有了进一步了解。
2、原工程基于C++、C和Rust，本人本身对Rust不了解，但是学习过程中难免涉及到调试，也看了整个代码，个人理解作者采用Rust和C++混编应该是两个问题：
    1、OpenSceneGraph没有除了C++之外其他版本，而且我们在用这个工具时，最主要的就是OSGB转3DTILES
    2、本身C++可以跨平台，但是C++对字符串、文件系统支持不够好，Rust可以很好满足这一点，并且Rust也便于和C++交互
    3、Rust也是号称无GC性能堪比C++语言，原作者本身可能也对Rust很熟悉
    综合下来，原作者使用了Rust和C++来编写，一是保证跨平台，二是保证了性能。
本人本身是对Qt比较熟悉的，就萌生了这个想法，用C++和Qt，从新梳理整个框架，刚好今年春节一个人，秉着学习想法，使用纯C++（加了Qt）重写了该工程，新的工程基于C++ 17标准重写，使用了Qt5.15，主要作者本身写C++大部分
时候都是基于Qt，对Qt也稍微了解，同时也借助Qt很好的跨平台特性，性能没有变化，并且也便于调试学习。
3、整个工程开源，但是鉴于Qt一些限制，该程序采用LGPL协议。
4、后续会与作者同步保持更新，后期希望可以加入自己一些特色功能。

# 简介

3D-Tile 转换工具集，高效快速的 3D-Tiles 生产工具，极度节省你的处理时间。

这是一个基于c++ 17 和 Qt5.15 项目。

提供了如下的子工具：

- `OSGB(OpenSceneGraph Binary)` 转 `3DTiles`
- `Esri Shapefile` 转 `3DTiles`

# 用法说明

##  命令行格式

```sh
3dtile.exe [FLAGS] [OPTIONS] --format <FORMAT> --input <PATH> --output <DIR>
```

## ②示例命令

```sh
# from osgb dataset
3dtile.exe -f osgb -i E:\osgb_path -o E:\out_path
3dtile.exe -f osgb -i E:\osgb_path -o E:\out_path -c "{\"offset\": 0}"
# use pbr-texture
3dtile.exe -f osgb -i E:\osgb_path -o E:\out_path -c "{\"pbr\": true}"

# from single shp file
3dtile.exe -f shape -i E:\Data\aa.shp -o E:\Data\aa --height height

# from single osgb file to glb file
3dtile.exe -f gltf -i E:\Data\TT\001.osgb -o E:\Data\TT\001.glb

# from single obj file to glb file
3dtile.exe -f gltf -i E:\Data\TT\001.obj -o E:\Data\TT\001.glb

# convert single b3dm file to glb file
3dtile.exe -f b3dm -i E:\Data\aa.b3dm -o E:\Data\aa.glb
```

## 参数说明
- `-f, --format <FORMAT>` 输入数据格式。

  `FORMAT` 可选：osgb, shape

  `osgb` 为倾斜摄影格式数据, `shape` 为 Shapefile 面(Polygon)数据

- `--input <PATH>` 输入数据的目录，osgb数据截止到 `<DIR>/Data` 目录的上一级，其他格式具体到文件名。

- `--output <DIR>` 输出目录。输出的数据文件位于 `<DIR>/Data` 目录。

- `--field` 高度字段。指定shapefile中的高度属性字段，此项为转换 shp 时的必须参数。



# 数据要求及说明

### 倾斜摄影数据

倾斜摄影数据仅支持 smart3d 格式的 osgb 组织方式：

- 数据目录必须有一个 `“Data”` 目录的总入口；
- `“Data”` 目录同级放置一个 `metadata.xml` 文件用来记录模型的位置信息；
- 每个瓦片目录下，必须有个和目录名同名的 osgb 文件，否则无法识别根节点；

正确的目录结构示意：

```
- Your-data-folder
  ├ metadata.xml
  └ Data\Tile_000_000\Tile_000_000.osgb
```

### Shapefile

目前仅支持 Shapefile 的面数据，可用于建筑物轮廓批量生成 3D-Tiles.

Shapefile 中需要有字段来表示高度信息。


# How To Build
1、vcpkg install "osg" "osg[plugins]"
2、如果没有Qt5.15
  1. 通过Qt官网安装
  2. vcpkg install "qt5[all]"
3. mkdir build
4. cd build 
5. cmake .. && make 
6. 把OSGPlugins文件夹复制到build目录下
# About Author
