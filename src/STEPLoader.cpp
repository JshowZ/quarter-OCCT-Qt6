#include "STEPLoader.h"
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

	// transfer roots to shapes
    bool transferOk = m_reader.TransferRoots();
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