#include "IGESLoader.h"
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRep_Builder.hxx>
#include <Precision.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TopoDS.hxx>

IGESLoader::IGESLoader(QObject* parent)
    : QObject(parent),
      m_enableFix(false)
{
}

bool IGESLoader::loadIGESFile(const QString& filePath, bool enableFix)
{
    m_shapes.clear();
    m_enableFix = enableFix;

    const char* filePathCStr = filePath.toLocal8Bit().constData();

    // read IGES file
    IFSelect_ReturnStatus status = m_reader.ReadFile(filePathCStr);

    if (status != IFSelect_RetDone) {
        emit fileLoaded(false, "fail");
        return false;
    }

    Standard_Boolean failsonly = Standard_False;
    m_reader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

    int nbRootsForTransfer =  m_reader.NbRootsForTransfer();
    QString statusMessage = QString("Read file done, %1 roots for transfer").arg(nbRootsForTransfer);

    // Set transfer mode to include all entity types (solids, curves, lines, etc.)
    //m_reader.SetTransferMode(IGESControl_AsIs);
    
    // transfer roots to shapes
    bool transferOk = m_reader.TransferRoots();
    m_reader.PrintCheckTransfer(failsonly, IFSelect_ItemsByEntity);

    if (!transferOk) {
        emit fileLoaded(false, "success");
        return false;
    }

    // get number of shapes
    int nbShapes = m_reader.NbShapes();
    int shapesLoaded = 0;

    for (int i = 1; i <= nbShapes; i++) {
        TopoDS_Shape shape = m_reader.Shape(i);
        if (!shape.IsNull()) {
            // 如果启用了修复工具，修复形状
            if (m_enableFix) {
                shape = fixShape(shape);
            }
            m_shapes.push_back(shape);
            shapesLoaded++;
        }
    }

    QString message = QString("success load %1 shapes").arg(shapesLoaded);
    emit fileLoaded(true, message);
    return true;
}

TopoDS_Shape IGESLoader::fixShape(const TopoDS_Shape& shape)
{
    // 复制原始形状
    TopoDS_Shape fixedShape = shape;
    
    // 修复无限曲线问题
    // 创建临时形状用于修改
    TopoDS_Shape tempShape = shape;
    
    // 遍历所有边
    for (TopExp_Explorer exp(tempShape, TopAbs_EDGE); exp.More(); exp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(exp.Current());
        
        // 检查曲线是否是无限直线
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        
        if (curve->IsKind(STANDARD_TYPE(Geom_Line))) {
            // 重新裁剪曲线到合理范围（使用现有边界）
            Handle(Geom_TrimmedCurve) trimmed = 
                new Geom_TrimmedCurve(curve, first, last);
            // 更新边的几何
            BRep_Builder B;
            B.UpdateEdge(edge, trimmed, Precision::Confusion());
        }
    }
    
    // 使用ShapeFix_Shape修复整个形状
    Handle(ShapeFix_Shape) fixer = new ShapeFix_Shape(fixedShape);
    fixer->SetPrecision(1.0e-3);
    fixer->SetMaxTolerance(1.0e-2);
    fixer->Perform();
    fixedShape = fixer->Shape();
    
    return fixedShape;
}
