#ifndef TEXTDEMOWIDGET_H
#define TEXTDEMOWIDGET_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>

namespace SIM {
    namespace Coin3D {
        namespace Quarter {
            class QuarterWidget;
        }
    }
}

// Coin3D and Quarter forward declarations
class SoSeparator;
class SoPerspectiveCamera;
class SoText3;
class SoFont;
class SoNode;

// OCCT forward declarations
class TopoDS_Shape;

class TextDemoWidget : public QMainWindow
{
    Q_OBJECT

public:
    TextDemoWidget(QWidget *parent = nullptr);
    ~TextDemoWidget();

private slots:
    void onCreateText();
    void onClear();
    void onZoomAll();

private:
    // UI components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QWidget* m_controlPanel;
    QLineEdit* m_textInput;
    QComboBox* m_fontComboBox;
    QDoubleSpinBox* m_textHeight;
    QPushButton* m_createTextBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_zoomAllBtn;
    
    // Coin3D components
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget;
    SoSeparator* m_root;
    SoSeparator* m_modelRoot;
    SoPerspectiveCamera* m_camera;
    
    // Private methods
    void setupUI();
    void createText(const std::string& text, double height, const std::string& font);
    bool displayShape(const TopoDS_Shape& shape, bool clearExisting = true);
    void clearScene();
    void zoomAll();
};

#endif // TEXTDEMOWIDGET_H