QT += core gui widgets network sql printsupport
TARGET = geometrydrawer
TEMPLATE = app

CONFIG += c++17

# Include paths
INCLUDEPATH += ./include \
    $$(COIN3D_INCLUDE_PATH) \
    $$(OCCT_INCLUDE_PATH) \
    $$(QUARTER_INCLUDE_PATH)

# Library paths
LIBS += -L$$(COIN3D_LIB_PATH) \
    -L$$(OCCT_LIB_PATH) \
    -L$$(QUARTER_LIB_PATH)

# Coin3D libraries
LIBS += -lCoin

# Quarter libraries
LIBS += -lQuarter

# OCCT libraries
LIBS += -lTKernel -lTKMath -lTKG2d -lTKG3d -lTKGeomBase -lTKGeomAlgo -lTKBRep -lTKTopAlgo -lTKPrim -lTKBO -lTKShHealing -lTKHLR -lTKIGES -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKService -lTKV3d -lTKOpenGl -lTKMesh -lTKCDF -lTKXCAF -lTKLCAF -lTKXDEIGES -lTKXDESTEP -lTKXMesh -lTKXSDRAW -lTKDraw -lTKBool -lTKFeat -lTKOffset -lTKPCAF -lTKDCAF -lTKXCAFSchema -lTKXmlLCAF -lTKXmlDRAW -lTKQADraw -lTKVCAF -lTKViewerTest

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/geometrydrawer.cpp \
    src/occtgeometry.cpp \
    src/quarterviewer.cpp

# Header files
HEADERS += \
    include/mainwindow.h \
    include/geometrydrawer.h \
    include/occtgeometry.h \
    include/quarterviewer.h

# UI files
FORMS += \
    ui/mainwindow.ui

# Resource files
RESOURCES += \
    resources/resources.qrc