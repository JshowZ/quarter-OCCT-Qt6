#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <STEPControl_Reader.hxx>

class STEPLoader : public QObject
{
    Q_OBJECT

public:
    explicit STEPLoader(QObject* parent = nullptr);
    bool loadSTEPFile(const QString& filePath);
    const std::vector<TopoDS_Shape>& getShapes() const { return m_shapes; }

signals:
    void fileLoaded(bool success, const QString& message);

private:
    STEPControl_Reader m_reader;
    std::vector<TopoDS_Shape> m_shapes;
};