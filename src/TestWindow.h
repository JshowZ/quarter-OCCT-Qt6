#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <Quarter/QuarterWidget.h>
#include <Quarter/Quarter.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoMaterial.h>

using namespace SIM::Coin3D::Quarter;

class TestWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow();
    
    // Initialize the window and QuarterWidget
    void initialize();
    
    // Draw 3D text at specified position
    void draw3DText(const QString &text, float x, float y, float z, float size = 1.0f);
    
    // Clear all 3D text
    void clearAllText();
    
private:
    QuarterWidget *m_quarterWidget;
    QVBoxLayout *m_layout;
    SoSeparator *m_rootNode;
    SoSeparator *m_textRoot;
};
