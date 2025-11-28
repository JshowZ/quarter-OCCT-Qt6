QT += core gui widgets network sql printsupport
TARGET = geometrydrawer
TEMPLATE = app

CONFIG += c++17

# 包含路径
INCLUDEPATH += ./include \
    $$(COIN3D_INCLUDE_PATH) \
    $$(OCCT_INCLUDE_PATH) \
    $$(QUARTER_INCLUDE_PATH)

# 库路径
LIBS += -L$$(COIN3D_LIB_PATH) \
    -L$$(OCCT_LIB_PATH) \
    -L$$(QUARTER_LIB_PATH)

# Coin3D库
LIBS += -lCoin

# Quarter库
LIBS += -lQuarter

# OCCT库
LIBS += -lTKernel -lTKMath -lTKG2d -lTKG3d -lTKGeomBase -lTKGeomAlgo -lTKBRep -lTKTopAlgo -lTKPrim -lTKBO -lTKShHealing -lTKHLR -lTKIGES -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKService -lTKV3d -lTKOpenGl -lTKMesh -lTKCDF -lTKXCAF -lTKLCAF -lTKXDEIGES -lTKXDESTEP -lTKXMesh -lTKXSDRAW -lTKDraw -lTKBool -lTKFeat -lTKOffset -lTKPCAF -lTKDCAF -lTKXCAFSchema -lTKXmlLCAF -lTKXmlDRAW -lTKQADraw -lTKVCAF -lTKViewerTest

# 源文件
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/geometrydrawer.cpp \
    src/occtgeometry.cpp \
    src/quarterviewer.cpp

# 头文件
HEADERS += \
    include/mainwindow.h \
    include/geometrydrawer.h \
    include/occtgeometry.h \
    include/quarterviewer.h

# UI文件
FORMS += \
    ui/mainwindow.ui

# 资源文件
RESOURCES += \
    resources/resources.qrc