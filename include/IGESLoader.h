#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <TopoDS_Shape.hxx>
#include <IGESControl_Reader.hxx>

class IGESLoader : public QObject
{
    Q_OBJECT

public:
    explicit IGESLoader(QObject* parent = nullptr);
    bool loadIGESFile(const QString& filePath, bool enableFix = false);
    void setEnableFix(bool enable) { m_enableFix = enable; }
    bool isEnableFix() const { return m_enableFix; }
    const std::vector<TopoDS_Shape>& getShapes() const { return m_shapes; }

    signals:
        void fileLoaded(bool success, const QString& message);

private:
    TopoDS_Shape fixShape(const TopoDS_Shape& shape);
    IGESControl_Reader m_reader;
    std::vector<TopoDS_Shape> m_shapes;
    bool m_enableFix; ///< 是否启用形状修复工具
};
