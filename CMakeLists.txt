cmake_minimum_required(VERSION 3.16)
project(geometrydrawer LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 dependencies
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGLWidgets Concurrent  REQUIRED)

# Manually configure include paths for Coin3D, OCCT and Quarter
# Paths need to be specified via CMake command line arguments
set(COIN3D_INCLUDE_PATH "" CACHE PATH "Coin3D include directory")
set(OCCT_INCLUDE_PATH "" CACHE PATH "OCCT include directory")
set(QUARTER_INCLUDE_PATH "" CACHE PATH "Quarter include directory")

# Manually configure library paths
set(COIN3D_LIB_PATH "" CACHE PATH "Coin3D library directory")
set(OCCT_LIB_PATH "" CACHE PATH "OCCT library directory")
set(QUARTER_LIB_PATH "" CACHE PATH "Quarter library directory")

# Verify necessary paths are set
if("${COIN3D_INCLUDE_PATH}" STREQUAL "" OR "${OCCT_INCLUDE_PATH}" STREQUAL "" OR "${QUARTER_INCLUDE_PATH}" STREQUAL "" OR
   "${COIN3D_LIB_PATH}" STREQUAL "" OR "${OCCT_LIB_PATH}" STREQUAL "" OR "${QUARTER_LIB_PATH}" STREQUAL "")
    message(FATAL_ERROR "Please specify all necessary dependency paths via CMake command line arguments, for example:
    cmake -DCOIN3D_INCLUDE_PATH=path/to/coin/include \
          -DOCCT_INCLUDE_PATH=path/to/occt/include \
          -DQUARTER_INCLUDE_PATH=path/to/quarter/include \
          -DCOIN3D_LIB_PATH=path/to/coin/lib \
          -DOCCT_LIB_PATH=path/to/occt/lib \
          -DQUARTER_LIB_PATH=path/to/quarter/lib \
          ..")
endif()

# Include header directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COIN3D_INCLUDE_PATH}
    ${OCCT_INCLUDE_PATH}
    ${QUARTER_INCLUDE_PATH}
)

# Link library directories
link_directories(
    ${COIN3D_LIB_PATH}
    ${OCCT_LIB_PATH}
    ${QUARTER_LIB_PATH}
)

# Collect source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/geometrydrawer.cpp
    src/occtgeometry.cpp
    src/quarterviewer.cpp
    src/AnotherMainWindow.cpp
    src/OccWidget.cpp
    src/STEPLoader.cpp
    src/CurveExtractor.cpp
)

# Collect header files
set(HEADERS
    include/mainwindow.h
    include/geometrydrawer.h
    include/occtgeometry.h
    include/quarterviewer.h
    include/AnotherMainWindow.h
    include/OccWidget.h
    include/STEPLoader.h
    include/CurveExtractor.h
)

# No external UI and resource files used, all interfaces implemented in code

# Add executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Enable Qt automatic processing (moc, uic, rcc)
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Qt6 no longer needs qt_use_modules, specify directly in target_link_libraries

# Link Coin3D library (use correct library name)
target_link_libraries(${PROJECT_NAME} Coin4)

# Link Quarter library
target_link_libraries(${PROJECT_NAME} Quarter1)

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE COIN_DLL QUARTER_DLL)

# Link OCCT libraries
target_link_libraries(${PROJECT_NAME}
    TKBin TKBinL TKBinTObj TKBinXCAF TKBO TKBool TKBRep TKCAF TKCDF TKDCAF TKDE TKDECascade TKDEIGES TKDEOBJ TKDEPLY TKDESTEP TKDESTL TKDEVRML TKDraw TKernel TKExpress TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHelix TKHLR TKLCAF TKMath TKMesh TKMeshVS TKOffset TKOpenGl TKOpenGlTest TKPrim TKQADraw TKRWMesh TKService TKShHealing TKStd TKStdL TKTObj TKTObjDRAW TKTopAlgo TKTopTest TKV3d TKVCAF TKViewerTest TKXCAF TKXDEDRAW TKXMesh TKXml TKXmlL TKXmlTObj TKXmlXCAF TKXSBase TKXSDRAW TKXSDRAWDE TKXSDRAWIGES TKXSDRAWOBJ TKXSDRAWPLY TKXSDRAWSTEP TKXSDRAWSTL TKXSDRAWVRML
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets Qt6::Concurrent
)

# Set output directory
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# Platform specific configuration
if(WIN32)
    # Windows specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation configuration
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Add Step2Stl dynamic library
# Collect Step2Stl source files
set(STEP2STL_SOURCES
    Step2Stl/src/Step2Stl.cpp
)

set(STEP2STL_HEADERS
    Step2Stl/include/Step2Stl.h
)

# Create shared library target
add_library(Step2Stl SHARED
    ${STEP2STL_SOURCES}
    ${STEP2STL_HEADERS}
)

# Set include directories for Step2Stl
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Step2Stl/include
)

# Set compile definitions for Step2Stl
set_target_properties(Step2Stl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
    DEFINE_SYMBOL STEP2STL_EXPORTS
)

# Link OCCT libraries for Step2Stl
# Use the same OCCT library names as the main application
# Only link necessary OCCT libraries for STEP to STL conversion
target_link_libraries(Step2Stl
    TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep TKTopAlgo TKPrim TKBO TKShHealing TKHLR TKDESTEP TKDEIGES TKDEOBJ TKDESTL TKXSBase TKService TKMesh TKCDF TKOpenGl
)

# Set output directory for Step2Stl library
set_target_properties(Step2Stl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# Installation configuration for Step2Stl
install(TARGETS Step2Stl
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${STEP2STL_HEADERS} DESTINATION include/Step2Stl)

# Add build subdirectory
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug or Release)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
