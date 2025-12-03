#include "STEPLoader.h"
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Shape.hxx>

STEPLoader::STEPLoader(QObject* parent)
    : QObject(parent)
{
}

bool STEPLoader::loadSTEPFile(const QString& filePath)
{
    m_shapes.clear();

    const char* filePathCStr = filePath.toLocal8Bit().constData();

    // read STEP file
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
    //m_reader.SetTransferMode(STEPControl_AsIs);
    
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
            m_shapes.push_back(shape);
            shapesLoaded++;
        }
    }

    QString message = QString("success load %1 shapes").arg(shapesLoaded);
    emit fileLoaded(true, message);
    return true;
}