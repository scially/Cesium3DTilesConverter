set_project("Cesium3DTilesConverter")
set_version("2.0.0")
set_runtimes("MD")

add_requires("gdal")
add_requires("openscenegraph", { configs = {shared = true }})
add_requires("spdlog", { configs = { fmt_external = true }})

if is_plat("windows") then
    add_defines("NOMINMAX")
    add_defines("WIN32")
end

target("Converter")
    set_languages("cxx17")
    
    add_rules("mode.debug", "mode.release")
    add_rules("qt.console")
    add_includedirs("include")
    add_headerfiles("include/*")
    add_files("src/*.cpp")
    add_files("include/*.h")

    add_packages("gdal")
    add_packages("openscenegraph")
    add_packages("spdlog")
    add_frameworks("QtXml", "QtTest", "QtCore")

    -- link osgdb_osg plugins for osg format read
    before_link(function (target)
        local osg_pkg = target:pkg("openscenegraph")
    
        local osg_basepath = osg_pkg:installdir() .. "/lib/osgPlugins-" .. osg_pkg:version()
        os.cp(osg_pkg:installdir() .. "/bin/osgPlugins-" .. osg_pkg:version(),  "osgPlugins")
    end)

    after_build(function (target)
        os.cp("$(projectdir)/gdal_data", target:targetdir() .. "/GDAL_DATA")
        os.cp("$(projectdir)/proj_data", target:targetdir() .. "/PROJ_LIB")

        os.cp("osgPlugins", target:targetdir() .. "osgPlugins")
    end)
    
    -- link gdal plugins for gdal format read
    after_install(function(target)
        os.cp("$(projectdir)/gdal_data", target:installdir() .. "/bin/GDAL_DATA")
        os.cp("$(projectdir)/proj_data", target:installdir() .. "/bin/PROJ_LIB")

        os.cp("osgPlugins", target:installdir() .. "osgPlugins")
    end)