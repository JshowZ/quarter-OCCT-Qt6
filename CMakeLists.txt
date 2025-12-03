cmake_minimum_required(VERSION 3.16)
project(geometrydrawer LANGUAGES CXX)

# Set project version
set(PROJECT_VERSION "1.0.0")
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Build type options
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Find Qt6 dependencies
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGLWidgets Concurrent REQUIRED)

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

# Set output directory
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# -----------------------------------------------------------------------------
# Main Application Configuration
# -----------------------------------------------------------------------------

# Collect source files for the main application
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")
file(GLOB_RECURSE APP_HEADERS "include/*.h")

# Add executable
add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_HEADERS}
)

# Enable Qt automatic processing (moc, uic, rcc)
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    WIN32_EXECUTABLE $<NOT:$<BOOL:${ENABLE_CONSOLE_OUTPUT}>>
)

# Set include directories for the main application
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COIN3D_INCLUDE_PATH}
    ${OCCT_INCLUDE_PATH}
    ${QUARTER_INCLUDE_PATH}
)

# Set link directories for the main application
target_link_directories(${PROJECT_NAME} PRIVATE
    ${COIN3D_LIB_PATH}
    ${OCCT_LIB_PATH}
    ${QUARTER_LIB_PATH}
)

# Set compile definitions for the main application
target_compile_definitions(${PROJECT_NAME} PRIVATE
    COIN_DLL
    QUARTER_DLL
    $<$<BOOL:${ENABLE_CONSOLE_OUTPUT}>:ENABLE_CONSOLE_OUTPUT>
)

# Define OCCT libraries
set(OCCT_CORE_LIBS
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
    TKPrim
    TKBO
    TKShHealing
    TKHLR
    TKService
    TKMesh
    TKCDF
    TKOpenGl
)

set(OCCT_DATA_EXCHANGE_LIBS
    TKDESTEP
    TKDEIGES
    TKDEOBJ
    TKDESTL
    TKDEPLY
    TKDEVRML
    TKDESTL
    TKXSBase
    TKDE
    TKDECascade
    TKDESTL
    TKXSDRAW
    TKXSDRAWDE
    TKXSDRAWIGES
    TKXSDRAWOBJ
    TKXSDRAWPLY
    TKXSDRAWSTEP
    TKXSDRAWSTL
    TKXSDRAWVRML
)

set(OCCT_ADDITIONAL_LIBS
    TKBin
    TKBinL
    TKBinTObj
    TKBinXCAF
    TKBool
    TKCAF
    TKDCAF
    TKDraw
    TKExpress
    TKFeat
    TKFillet
    TKHelix
    TKLCAF
    TKMeshVS
    TKOffset
    TKOpenGlTest
    TKQADraw
    TKRWMesh
    TKStd
    TKStdL
    TKTObj
    TKTObjDRAW
    TKTopTest
    TKV3d
    TKVCAF
    TKViewerTest
    TKXCAF
    TKXDEDRAW
    TKXMesh
    TKXml
    TKXmlL
    TKXmlTObj
    TKXmlXCAF
)

# Link libraries for the main application
target_link_libraries(${PROJECT_NAME} PRIVATE
    # Coin3D and Quarter libraries
    Coin4
    Quarter1
    
    # OCCT libraries
    ${OCCT_CORE_LIBS}
    ${OCCT_DATA_EXCHANGE_LIBS}
    ${OCCT_ADDITIONAL_LIBS}
    
    # Qt libraries
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Concurrent
)

# -----------------------------------------------------------------------------
# Step2Stl Library Configuration
# -----------------------------------------------------------------------------

# Option to build Step2Stl library
option(BUILD_STEP2STL_LIBRARY "Build Step2Stl dynamic library" ON)

# Option to enable console output
option(ENABLE_CONSOLE_OUTPUT "Enable console output for the application" OFF)

if(BUILD_STEP2STL_LIBRARY)
    # Collect Step2Stl source files
    file(GLOB_RECURSE STEP2STL_SOURCES "Step2Stl/src/*.cpp")
    file(GLOB_RECURSE STEP2STL_HEADERS "Step2Stl/include/*.h")
    
    # Create shared library target
    add_library(Step2Stl SHARED
        ${STEP2STL_SOURCES}
        ${STEP2STL_HEADERS}
    )
    
    # Set Step2Stl library properties
    set_target_properties(Step2Stl PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        PREFIX ""
        DEFINE_SYMBOL STEP2STL_EXPORTS
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    
    # Set include directories for Step2Stl
    target_include_directories(Step2Stl PRIVATE
        ${OCCT_INCLUDE_PATH}
    )
    
    # Set public include directory for Step2Stl
    target_include_directories(Step2Stl PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Step2Stl/include>
        $<INSTALL_INTERFACE:include/Step2Stl>
    )
    
    # Set link directories for Step2Stl
    target_link_directories(Step2Stl PRIVATE
        ${OCCT_LIB_PATH}
    )
    
    # Link necessary OCCT libraries for Step2Stl
    target_link_libraries(Step2Stl PRIVATE
        ${OCCT_CORE_LIBS}
        ${OCCT_DATA_EXCHANGE_LIBS}
    )
    
    # Installation configuration for Step2Stl
    install(TARGETS Step2Stl
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
    )
    
    install(FILES ${STEP2STL_HEADERS} DESTINATION include/Step2Stl)
endif()

# -----------------------------------------------------------------------------
# Installation Configuration
# -----------------------------------------------------------------------------

# Install the main application
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# -----------------------------------------------------------------------------
# Build Information
# -----------------------------------------------------------------------------

message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Output Directory: ${OUTPUT_DIR}")
message(STATUS "Build Step2Stl Library: ${BUILD_STEP2STL_LIBRARY}")
message(STATUS "Enable Console Output: ${ENABLE_CONSOLE_OUTPUT}")
message(STATUS "")

