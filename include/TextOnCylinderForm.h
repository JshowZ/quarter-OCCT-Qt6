#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <string>

#include<TopoDS_Shape.hxx>


// Forward declarations for Coin3D and Quarter
class SoNode;
class SoSeparator;
class SoPerspectiveCamera;
namespace SIM {
namespace Coin3D {
namespace Quarter {
    class QuarterWidget;
}
}
}

class QSlider;
class QPushButton;
class QLineEdit;
class QDoubleSpinBox;


/**
 * @brief The TextOnCylinderForm class provides a Qt window for creating a cylinder, text, and performing text projection/engraving
 * 
 * This class implements the functionality to:
 * - Create a cylinder
 * - Create text
 * - Project text onto the cylinder
 * - Engrave text onto the cylinder
 */
class TextOnCylinderForm : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit TextOnCylinderForm(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~TextOnCylinderForm() override;
    
    /**
     * @brief Clear the current scene
     */
    void clearScene();
    
    /**
     * @brief Zoom to fit the entire scene
     */
    void zoomAll();

protected:
    /**
     * @brief Setup the UI components
     */
    void setupUI();
    
    /**
     * @brief Create a cylinder shape
     * @param radius Cylinder radius
     * @param height Cylinder height
     * @return OCCT cylinder shape
     */
    TopoDS_Shape createCylinder(double radius, double height);
    
    /**
     * @brief Create text shape
     * @param text Text string
     * @param height Text height
     * @return OCCT text shape
     */
    TopoDS_Shape createText(const std::string& text, double height);
    
    /**
     * @brief Project text onto cylinder
     * @param cylinder Cylinder shape
     * @param text Text shape
     * @return Combined shape with text projected
     */
    TopoDS_Shape projectTextOntoCylinder(const TopoDS_Shape& cylinder, const TopoDS_Shape& text);
    
    /**
     * @brief Engrave text onto cylinder
     * @param cylinder Cylinder shape
     * @param text Text shape
     * @param depth Engraving depth
     * @return Combined shape with text engraved
     */
    TopoDS_Shape engraveTextOntoCylinder(const TopoDS_Shape& cylinder, const TopoDS_Shape& text, double depth);
    
    /**
     * @brief Convert OCCT TopoDS_Shape to Coin3D SoNode
     * @param shape The OCCT shape to convert
     * @return Converted Coin3D node, or nullptr if conversion failed
     */
    SoNode* convertShapeToCoin3D(const TopoDS_Shape& shape);
    
    /**
     * @brief Recursively convert OCCT TopoDS_Shape to Coin3D SoNode
     * @param shape The OCCT shape to convert
     * @param deviation Deviation for mesh generation
     * @param angularDeflection Angular deflection for mesh generation
     * @return Converted Coin3D node, or nullptr if conversion failed
     */
    SoNode* convertShapeRecursive(TopoDS_Shape shape, double deviation, double angularDeflection);
    
    /**
     * @brief Display shape in the viewer
     * @param shape The OCCT shape to display
     * @param clearExisting Whether to clear existing models (default: true)
     * @return true if successful, false otherwise
     */
    bool displayShape(const TopoDS_Shape& shape, bool clearExisting = false);

private slots:
    /**
     * @brief Slot for create cylinder button
     */
    void onCreateCylinder();
    
    /**
     * @brief Slot for create text button
     */
    void onCreateText();
    
    /**
     * @brief Slot for project text button
     */
    void onProjectText();
    
    /**
     * @brief Slot for engrave text button
     */
    void onEngraveText();
    
    /**
     * @brief Slot for clear button
     */
    void onClear();
    
    /**
     * @brief Slot for zoom all button
     */
    void onZoomAll();

private:
    QWidget* m_centralWidget;            // Central widget
    QVBoxLayout* m_mainLayout;            // Main layout
    QWidget* m_controlPanel;              // Control panel
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget; // Quarter widget for Coin3D rendering
    SoSeparator* m_root;                  // Root of the Coin3D scene graph
    SoSeparator* m_modelRoot;             // Root for the model geometry
    SoPerspectiveCamera* m_camera;        // Camera for the scene
    
    // UI controls
    QLineEdit* m_textInput;               // Text input
    QDoubleSpinBox* m_cylinderRadius;     // Cylinder radius
    QDoubleSpinBox* m_cylinderHeight;     // Cylinder height
    QDoubleSpinBox* m_textHeight;         // Text height
    QDoubleSpinBox* m_engravingDepth;     // Engraving depth
    QPushButton* m_createCylinderBtn;     // Create cylinder button
    QPushButton* m_createTextBtn;         // Create text button
    QPushButton* m_projectTextBtn;        // Project text button
    QPushButton* m_engraveTextBtn;        // Engrave text button
    QPushButton* m_clearBtn;              // Clear button
    QPushButton* m_zoomAllBtn;            // Zoom all button
    
    // Shapes
    TopoDS_Shape m_cylinder;              // Cylinder shape
    TopoDS_Shape m_text;                  // Text shape
    bool m_cylinderCreated;               // Flag indicating if cylinder is created
    bool m_textCreated;                   // Flag indicating if text is created
};