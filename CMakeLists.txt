cmake_minimum_required(VERSION 3.16)
project(geometrydrawer LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6依赖
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGLWidgets REQUIRED)

# 手动配置Coin3D、OCCT和Quarter的包含路径
# 需要通过CMake命令行参数指定路径
set(COIN3D_INCLUDE_PATH "" CACHE PATH "Coin3D include directory")
set(OCCT_INCLUDE_PATH "" CACHE PATH "OCCT include directory")
set(QUARTER_INCLUDE_PATH "" CACHE PATH "Quarter include directory")

# 手动配置库路径
set(COIN3D_LIB_PATH "" CACHE PATH "Coin3D library directory")
set(OCCT_LIB_PATH "" CACHE PATH "OCCT library directory")
set(QUARTER_LIB_PATH "" CACHE PATH "Quarter library directory")

# 验证必要路径是否已设置
if("${COIN3D_INCLUDE_PATH}" STREQUAL "" OR "${OCCT_INCLUDE_PATH}" STREQUAL "" OR "${QUARTER_INCLUDE_PATH}" STREQUAL "" OR
   "${COIN3D_LIB_PATH}" STREQUAL "" OR "${OCCT_LIB_PATH}" STREQUAL "" OR "${QUARTER_LIB_PATH}" STREQUAL "")
    message(FATAL_ERROR "请通过CMake命令行参数指定所有必要的依赖路径，例如：
    cmake -DCOIN3D_INCLUDE_PATH=path/to/coin/include \
          -DOCCT_INCLUDE_PATH=path/to/occt/include \
          -DQUARTER_INCLUDE_PATH=path/to/quarter/include \
          -DCOIN3D_LIB_PATH=path/to/coin/lib \
          -DOCCT_LIB_PATH=path/to/occt/lib \
          -DQUARTER_LIB_PATH=path/to/quarter/lib \
          ..")
endif()

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COIN3D_INCLUDE_PATH}
    ${OCCT_INCLUDE_PATH}
    ${QUARTER_INCLUDE_PATH}
)

# 链接库目录
link_directories(
    ${COIN3D_LIB_PATH}
    ${OCCT_LIB_PATH}
    ${QUARTER_LIB_PATH}
)

# 收集源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/geometrydrawer.cpp
    src/occtgeometry.cpp
    src/quarterviewer.cpp
)

# 收集头文件
set(HEADERS
    include/mainwindow.h
    include/geometrydrawer.h
    include/occtgeometry.h
    include/quarterviewer.h
)

# 不再使用外部UI和资源文件，所有界面在代码中实现

# 添加可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 启用Qt自动处理（moc、uic、rcc）
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Qt6不再需要qt_use_modules，直接在target_link_libraries中指定

# 链接Coin3D库（使用正确的库名称）
target_link_libraries(${PROJECT_NAME} Coin4)

# 链接Quarter库
target_link_libraries(${PROJECT_NAME} Quarter1)

# 设置编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE COIN_DLL QUARTER_DLL)

# 链接OCCT库
target_link_libraries(${PROJECT_NAME}
    TKBin TKBinL TKBinTObj TKBinXCAF TKBO TKBool TKBRep TKCAF TKCDF TKDCAF TKDE TKDECascade TKDEIGES TKDEOBJ TKDEPLY TKDESTEP TKDESTL TKDEVRML TKDraw TKernel TKExpress TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHelix TKHLR TKLCAF TKMath TKMesh TKMeshVS TKOffset TKOpenGl TKOpenGlTest TKPrim TKQADraw TKRWMesh TKService TKShHealing TKStd TKStdL TKTObj TKTObjDRAW TKTopAlgo TKTopTest TKV3d TKVCAF TKViewerTest TKXCAF TKXDEDRAW TKXMesh TKXml TKXmlL TKXmlTObj TKXmlXCAF TKXSBase TKXSDRAW TKXSDRAWDE TKXSDRAWIGES TKXSDRAWOBJ TKXSDRAWPLY TKXSDRAWSTEP TKXSDRAWSTL TKXSDRAWVRML
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets
)

# 设置输出目录
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# 平台特定配置
if(WIN32)
    # Windows特定设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装配置
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 添加构建子目录
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug or Release)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
