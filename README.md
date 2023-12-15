# Cesium3DTilesConveter:Overview
[简体中文](README_zh_CN.md)  [English](README.md)

The Cesium3DTilesConveter based on C++17 and Qt supports the conversion of oblique photography model and vector data in multiple coordinate systems. It is also the only open source tool in the github that supports the oblique photography model pyramid reconstruction.

# Introduction

1. Oblique Photography Model(OpenSceneGraph Binary) to 3DTiles: convert huge of osgb file to 3DTiles.

2. Vector to 3DTiles: convert vector data which gdal supported to 3D-Tiles.

# Usage

Simplify the input parameters from the CLI, adopt GLTF2.0 format by default (y-axis upward), and multi threaded parallel processing.

##  CLI Format

```sh
Converter -f <FORMAT> [OPTIONS] <INPUT> <OUTPUT> 
```

## Example

```sh
# from osgb dataset
Converter -f OSGB -m true -i <OSGB> -o <OUTPUT> 
```

## Parameter Description
```
Options:
  -?, -h, --help           displays help on commandline options.
  -f, --format <format>    OSGB or Vector(required)
  -m, --merge <true/false> Top-level rebuild merge switch options
  -i, --input  <INPUT>     Input path. for OSGB format, the path ends at the upper level of the `<INPUT>/Data` directory. for GDAL, please refers to the GDAL data format.
  -o, --ouput  <OUTPUT>    Output path。for OSGB format, the data files output are located in the <OUTPUT>/Data` directory. for GDAL format, the data files output are located in the <DIR>/Tile directory, and tileset.json is located in the <OUTPUT> root directory.
```

# Data Requirements

### Oblique Photography Model

Oblique photography model only supports the osgb organization method of smart3d format:

- the data directory must have a main entry to the `Data` directory;
- a `metadata.xml` file at the same level of the `Data` directory to record the location information of the model;
- each tile directory must have an osgb file with the same name as the directory name, otherwise the root node cannot be recognized;

Correct directory structure diagram:

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
2. If you do not have Qt5.15, you can install it in the following two ways:
    1. Install through Qt official website
    2. vcpkg install "qt5[all]"
3. Open Visual Studio, selects CMakeLists.txt, imports the project and compiles
4. Copy the OSGPlugins-${Version} folder to the compilation out directory

# TODO
1. Currently, only support OSGB, and the GDAL format conversion code will be migrated later.

2. The root node merging has been initially implemented. But only Vertex simplification has been achieved, and texture simplification will be added later.

# Reference
1. 3dtiles specification [https://github.com/CesiumGS/3d-tiles](https://github.com/CesiumGS/3d-tiles)
